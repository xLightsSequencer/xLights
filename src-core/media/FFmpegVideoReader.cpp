/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FFmpegVideoReader.h"

//#define VIDEO_EXTRALOGGING

#undef min
#include <algorithm>
#include <filesystem>
#include <list>
#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#if __has_include(<libavdevice/avdevice.h>)
#include <libavdevice/avdevice.h>
#endif
}

#include "../utils/SpecialOptions.h"
#include <log.h>

#ifdef __APPLE__
#include "media/VideoToolboxBridge.h"
#else
namespace AppleVideoToolboxBridge {
inline void InitVideoToolboxAcceleration() {}
inline bool SetupVideoToolboxAcceleration(AVCodecContext*, bool) { return false; }
inline void CleanupVideoToolbox(AVCodecContext*, void*) {}
inline bool VideoToolboxScaleImage(AVCodecContext*, AVFrame*, AVFrame*, void*&, int) { return false; }
inline bool IsVideoToolboxAcceleratedFrame(AVFrame*) { return false; }
} // namespace AppleVideoToolboxBridge
#endif

#ifdef _WIN32
#include "WindowsHardwareVideoReader.h"
#include <VersionHelpers.h>
#endif

static thread_local enum AVPixelFormat __hw_pix_fmt = ::AVPixelFormat::AV_PIX_FMT_NONE;
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
    const enum AVPixelFormat* p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == __hw_pix_fmt)
        {
            return *p;
        }
    }

    spdlog::debug("HW format negotiation: requested {} not offered by codec; codec open will fail.",
                  av_get_pix_fmt_name(__hw_pix_fmt));
#ifdef VIDEO_EXTRALOGGING
    spdlog::debug("   Looking for {} but only found:", av_get_pix_fmt_name(__hw_pix_fmt));
    for (p = pix_fmts; *p != -1; p++) {
        spdlog::debug("       {}", av_get_pix_fmt_name(*p));
    }
#endif

    return AV_PIX_FMT_NONE;
}

bool FFmpegVideoReader::HW_ACCELERATION_ENABLED = false;
WINHARDWARERENDERTYPE FFmpegVideoReader::HW_ACCELERATION_TYPE = WINHARDWARERENDERTYPE::FFMPEG_AUTO;

void FFmpegVideoReader::SetHardwareAcceleratedVideo(bool accel)
{
#ifdef __LINUX__
    HW_ACCELERATION_ENABLED = false;
#else
    HW_ACCELERATION_ENABLED = accel;
#endif
}

void FFmpegVideoReader::SetHardwareRenderType(int type)
{
    HW_ACCELERATION_TYPE = static_cast<WINHARDWARERENDERTYPE>( type );
}

void FFmpegVideoReader::InitHWAcceleration() {
    AppleVideoToolboxBridge::InitVideoToolboxAcceleration();
}

void FFmpegVideoReader::SetScaleAlgorithm(VideoScaleAlgorithm algorithm) {
    _scaleAlgorithm = algorithm;
}

static int VideoScaleAlgorithmToSWS(VideoScaleAlgorithm alg) {
    switch (alg) {
    case VideoScaleAlgorithm::Lanczos: return SWS_LANCZOS;
    case VideoScaleAlgorithm::Area:    return SWS_AREA;
    case VideoScaleAlgorithm::Point:   return SWS_POINT;
    case VideoScaleAlgorithm::Bicubic:
    case VideoScaleAlgorithm::Default:
    default:                           return SWS_BICUBIC;
    }
}

// Helper to populate the VideoFrame from an AVFrame
static void PopulateVideoFrame(VideoFrame& vf, AVFrame* avf, VideoPixelFormat fmt) {
    if (avf && avf->data[0]) {
        vf.data = avf->data[0];
        vf.linesize = avf->linesize[0];
        vf.width = avf->width;
        vf.height = avf->height;
        vf.format = fmt;
        vf.nativeHandle = nullptr;
    }
}

static VideoPixelFormat AVPixelFormatToVideoPixelFormat(AVPixelFormat fmt) {
    switch (fmt) {
        case AV_PIX_FMT_RGB24: return VideoPixelFormat::RGB24;
        case AV_PIX_FMT_BGR24: return VideoPixelFormat::BGR24;
        case AV_PIX_FMT_RGBA: return VideoPixelFormat::RGBA;
        case AV_PIX_FMT_BGRA: return VideoPixelFormat::BGRA;
        case AV_PIX_FMT_VIDEOTOOLBOX: return VideoPixelFormat::PlatformNative;
        default: return VideoPixelFormat::RGB24;
    }
}

FFmpegVideoReader::FFmpegVideoReader(const std::string& filename, int maxwidth, int maxheight, bool keepaspectratio, bool usenativeresolution,
                         bool wantAlpha, bool bgr, bool wantsHWType)
{
    _wantsHWType = wantsHWType;
    _maxwidth = maxwidth;
    _maxheight = maxheight;
    _filename = filename;
    _valid = false;
    _lengthMS = 0.0;
    _formatContext = nullptr;
    _codecContext = nullptr;
    _videoStream = nullptr;
    _dstFrame = nullptr;
    _dstFrame2 = nullptr;
    _srcFrame = nullptr;
    _curPos = -1000;
    _wantAlpha = wantAlpha;
    _videoToolboxAccelerated = false;
    if (_wantAlpha) {
        _pixelFmt = bgr ? AVPixelFormat::AV_PIX_FMT_BGRA : AVPixelFormat::AV_PIX_FMT_RGBA;
    } else {
        _pixelFmt = bgr ? AVPixelFormat::AV_PIX_FMT_BGR24 : AVPixelFormat::AV_PIX_FMT_RGB24;
    }
    _atEnd = false;
    _swsCtx = nullptr;
    _dtspersec = 1.0;
    _frames = 0;
    _width = _maxwidth;
    _height = _maxheight;

#ifdef _WIN32
    if (HW_ACCELERATION_ENABLED && ::IsWindows8OrGreater() && HW_ACCELERATION_TYPE == WINHARDWARERENDERTYPE::DIRECX11_API) {
        _windowsHardwareVideoReader = new WindowsHardwareVideoReader(filename, _wantAlpha, usenativeresolution, keepaspectratio, maxwidth, maxheight, _pixelFmt);
        if (_windowsHardwareVideoReader->IsOk()) {
            _frames = _windowsHardwareVideoReader->GetFrames();
            _lengthMS = _windowsHardwareVideoReader->GetDuration();
            _height = _windowsHardwareVideoReader->GetHeight();
            _width = _windowsHardwareVideoReader->GetWidth();
            _frameMS = _windowsHardwareVideoReader->GetFrameMS();
            _valid = true;

            spdlog::info("Video loaded: " + filename);
            spdlog::debug("      Length MS: {}", _lengthMS);
            spdlog::debug("      _frames: {}", _frames);
            spdlog::debug("      Frames per second {}", (double)_frames * 1000.0 / _lengthMS);
            spdlog::debug("      Source size: {}x{}", _windowsHardwareVideoReader->GetNativeWidth(), _windowsHardwareVideoReader->GetNativeHeight());
            spdlog::debug("      Output size: {}x{}", _width, _height);
            if (_wantAlpha)
                spdlog::debug("      Alpha: TRUE");
            spdlog::debug("      Frame ms {}", _frameMS);
            return;
        } else {
            delete _windowsHardwareVideoReader;
            _windowsHardwareVideoReader = nullptr;
        }
    }
#endif

    int res = avformat_open_input(&_formatContext, filename.c_str(), nullptr, nullptr);
    if (res != 0) {
        spdlog::error("Error opening the file " + filename);
        return;
    }

    if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        spdlog::error("VideoReader: Error finding the stream info in " + filename);
        return;
    }

    // Find the video stream
    _streamIndex = av_find_best_stream(_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &_decoder, 0);
    if (_streamIndex < 0) {
        spdlog::error("VideoReader: Could not find any video stream in " + filename);
        return;
    }

    _videoStream = _formatContext->streams[_streamIndex];
    _videoStream->discard = ::AVDiscard::AVDISCARD_NONE;

    reopenContext();

    if (_codecContext == nullptr) {
        return;
    }

    // at this point it is open and ready
    if (usenativeresolution) {
        _height = _codecContext->height;
        _width = _codecContext->width;
    } else {
        if (keepaspectratio) {
            if (_codecContext->width == 0 || _codecContext->height == 0) {
                spdlog::error("VideoReader: Invalid input reader dimensions ({},{}) {}", _codecContext->width, _codecContext->height, filename);
                return;
            }

            float shrink = std::min((float)maxwidth / (float)_codecContext->width, (float)maxheight / (float)_codecContext->height);
            _height = (int)((float)_codecContext->height * shrink);
            _width = (int)((float)_codecContext->width * shrink);
        } else {
            _height = maxheight;
            _width = maxwidth;
        }
    }

    // get the video length in MS
    _frames = (long)_videoStream->nb_frames;
    if (_videoStream->time_base.num != 0) {
        _dtspersec = (double)_videoStream->time_base.den / (double)_videoStream->time_base.num;
    } else {
        if (_frames == 0 || _videoStream->avg_frame_rate.den == 0) {
            spdlog::warn("VideoReader: dtspersec calc error _videoStream->nb_frames {} and _videoStream->avg_frame_rate.den {} cannot be zero. {}", (int)_videoStream->nb_frames, (int)_videoStream->avg_frame_rate.den, (const char *)filename.c_str());
            spdlog::warn("VideoReader: Video seeking will only work back to the start of the video.");
            _dtspersec = 1.0;
        } else {
            _dtspersec = (((double)_videoStream->duration * (double)_videoStream->avg_frame_rate.num) / ((double)_frames * (double)_videoStream->avg_frame_rate.den));
        }
    }

    if (_videoStream->time_base.num != 0 && _videoStream->duration != 0) {
        _lengthMS = ((double)_videoStream->duration * (double)_videoStream->time_base.num) / (double)_videoStream->time_base.den * 1000.0;
    } else if (_frames > 0) {
        if (_videoStream->r_frame_rate.num != 0) {
            _lengthMS = ((double)_frames * (double)_videoStream->r_frame_rate.den * 1000.0) / (double)_videoStream->r_frame_rate.num;
        } else {
            if (_videoStream->avg_frame_rate.num != 0) {
                _lengthMS = ((double)_frames * (double)_videoStream->avg_frame_rate.den * 1000.0) / (double)_videoStream->avg_frame_rate.num;
            } else {
                spdlog::info("VideoReader: _videoStream->avg_frame_rate.num = 0");
            }
        }
    }

    if (_lengthMS <= 0 || _frames <= 0) {
        if (_videoStream->avg_frame_rate.den != 0) {
            _lengthMS = (double)_formatContext->duration / 1000.0;
            _frames = (long)(_lengthMS  * (double)_videoStream->avg_frame_rate.num / (double)(_videoStream->avg_frame_rate.den) / 1000.0);
        } else {
            spdlog::info("VideoReader: _videoStream->avg_frame_rate.den = 0");
        }
    }

    if (_lengthMS <= 0 || _frames <= 0) {
        if (_videoStream->avg_frame_rate.den != 0) {
            _lengthMS = (double)_formatContext->duration / 1000.0;
            _frames = (long)(_lengthMS  * (double)_videoStream->avg_frame_rate.num / (double)(_videoStream->avg_frame_rate.den) / 1000.0);
        }
    }

    if (_lengthMS <= 0 || _frames <= 0) {
        spdlog::warn("Attempts to determine length of video have not been successful. Problems ahead.");
    }

    _keyFrameCount = _codecContext->keyint_min;

    _dstFrame = av_frame_alloc();
    _dstFrame->width = _width;
    _dstFrame->height = _height;
    _dstFrame->linesize[0] = _width * GetPixelChannels();
    _dstFrame->data[0] = (uint8_t *)av_malloc(_width * _height * GetPixelChannels() * sizeof(uint8_t));
    _dstFrame->format = _pixelFmt;
    _dstFrame2 = av_frame_alloc();
    _dstFrame2->width = _width;
    _dstFrame2->height = _height;
    _dstFrame2->linesize[0] = _width * GetPixelChannels();
    _dstFrame2->data[0] = (uint8_t *)av_malloc(_width * _height * GetPixelChannels() * sizeof(uint8_t));
    _dstFrame2->format = _pixelFmt;

    _srcFrame = av_frame_alloc();
    _srcFrame2 = av_frame_alloc();

    _packet = av_packet_alloc();
    _valid = true;

    spdlog::info("Video loaded: " + filename);
    spdlog::debug("      Length MS: {}", _lengthMS);
    spdlog::debug("      _videoStream->time_base.num: {}", _videoStream->time_base.num);
    spdlog::debug("      _videoStream->time_base.den: {}", _videoStream->time_base.den);
    spdlog::debug("      _videoStream->r_frame_rate.num: {}", _videoStream->r_frame_rate.num);
    spdlog::debug("      _videoStream->r_frame_rate.den: {}", _videoStream->r_frame_rate.den);
    spdlog::debug("      _videoStream->avg_frame_rate.num: {}", _videoStream->avg_frame_rate.num);
    spdlog::debug("      _videoStream->avg_frame_rate.den: {}", _videoStream->avg_frame_rate.den);
    spdlog::debug("      DTS per sec: {}", _dtspersec);
    spdlog::debug("      _videoStream->nb_frames: {}", _videoStream->nb_frames);
    spdlog::debug("      _frames: {}", _frames);
    spdlog::debug("      Frames per second {}", (double)_frames * 1000.0 / _lengthMS);
    spdlog::debug("      Source size: {}x{}", _codecContext->width, _codecContext->height);
    spdlog::debug("      Source coded size: {}x{}", _codecContext->coded_width, _codecContext->coded_height);
    spdlog::debug("      Output size: {}x{}", _width, _height);
    spdlog::debug("      Guessed key frame frequency: {}", _keyFrameCount);
    if (_wantAlpha)
        spdlog::debug("      Alpha: TRUE");
    if (_frames != 0) {
        spdlog::debug("      Frame ms {}", _lengthMS / (double)_frames);
        _frameMS = _lengthMS / _frames;
        spdlog::debug("      Used frame ms {}", _frameMS);
    } else {
        spdlog::warn("      Frame ms <unknown as _frames is 0>");
        _frameMS = 0;
    }

    _firstFramePos = -1;
}

void FFmpegVideoReader::reopenContext(bool allowHWDecoder) {
    spdlog::debug("VideoReader: reopenContext({}) for {}", allowHWDecoder, _filename);

    if (_codecContext != nullptr) {
        AppleVideoToolboxBridge::CleanupVideoToolbox(_codecContext, hwDecoderCache);
        hwDecoderCache = nullptr;
        avcodec_free_context(&_codecContext);
        _codecContext = nullptr;
    }
    if (_hw_device_ctx != nullptr) {
        av_buffer_unref(&_hw_device_ctx);
        _hw_device_ctx = nullptr;
    }
    enum AVHWDeviceType type = ::AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
    if (allowHWDecoder && IsHardwareAcceleratedVideo()) {
#if defined(_WIN32)
        std::list<std::string> hwdecoders = { "cuda", "qsv", "d3d11va", "vulkan" };

        switch (HW_ACCELERATION_TYPE) {
            case WINHARDWARERENDERTYPE::FFMPEG_CUDA:
                hwdecoders = { "cuda" };
                break;
            case WINHARDWARERENDERTYPE::FFMPEG_QSV:
                hwdecoders = { "qsv" };
                break;
            case WINHARDWARERENDERTYPE::FFMPEG_VULKAN:
                hwdecoders = { "vulkan" };
                break;
            case WINHARDWARERENDERTYPE::FFMPEG_AMF:
            case WINHARDWARERENDERTYPE::FFMPEG_D3D11VA:
                hwdecoders = { "d3d11va" };
                break;
            case WINHARDWARERENDERTYPE::FFMPEG_AUTO:
            case WINHARDWARERENDERTYPE::DIRECX11_API:
            default:
                break;
        }

#elif defined(__APPLE__)
        std::list<std::string> hwdecoders = { "videotoolbox" };
#else
        std::list<std::string> hwdecoders = { "vaapi", "vdpau" };
#endif

        for (const auto& it : hwdecoders) {
            type = av_hwdevice_find_type_by_name(it.c_str());
            if (type != AV_HWDEVICE_TYPE_NONE) {
                break;
            }
        }
        if (type != AV_HWDEVICE_TYPE_NONE) {
            for (int i = 0;; i++) {
                const AVCodecHWConfig* config = avcodec_get_hw_config(_decoder, i);
                if (!config) {
                    type = AV_HWDEVICE_TYPE_NONE;
                    break;
                }
                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                    config->device_type == type) {
                    __hw_pix_fmt = config->pix_fmt;
                    break;
                }
            }
        }
    }

    const AVCodec* decoderToUse = _decoder;
    bool usingCuvid = false;
#if defined(_WIN32)
    if (allowHWDecoder && type == AV_HWDEVICE_TYPE_CUDA) {
        std::string cuvidName = std::string(_decoder->name) + "_cuvid";
        const AVCodec* cuvidDecoder = avcodec_find_decoder_by_name(cuvidName.c_str());
        if (cuvidDecoder) {
            decoderToUse = cuvidDecoder;
            usingCuvid = true;
            __hw_pix_fmt = AV_PIX_FMT_CUDA;
            spdlog::debug("VideoReader: NVDEC decoder '{}' found", cuvidName.c_str());
        } else {
            spdlog::debug("VideoReader: NVDEC decoder '{}_cuvid' not in this FFmpeg build", _decoder->name);
        }
    }
#endif

    _codecContext = avcodec_alloc_context3(decoderToUse);
    if (!_codecContext) {
        spdlog::error("VideoReader: Failed to allocate codec context for {}", _filename.c_str());
        return;
    }

    _codecContext->thread_type = 0;
    _codecContext->thread_count = 1;
    _codecContext->skip_frame = AVDISCARD_NONE;
    _codecContext->skip_loop_filter = AVDISCARD_NONE;
    _codecContext->skip_idct = AVDISCARD_NONE;

    if (avcodec_parameters_to_context(_codecContext, _videoStream->codecpar) < 0) {
        spdlog::error("VideoReader: Failed to copy {} codec parameters to decoder context", _filename.c_str());
        return;
    }

    _codecContext->hwaccel_context = nullptr;
    {
        if (IsHardwareAcceleratedVideo() && type != AV_HWDEVICE_TYPE_NONE) {
            // Serialize HW device creation: concurrent CUDA/NVDEC init from many
            // render threads exhausts driver session limits and corrupts shared state.
            static std::mutex s_hwDeviceCreateMutex;
            std::lock_guard<std::mutex> hwLock(s_hwDeviceCreateMutex);
            const char* opt = nullptr;
            if (av_hwdevice_ctx_create(&_hw_device_ctx, type, opt, nullptr, 0) < 0) {
                spdlog::warn("VideoReader: Failed to create HW device '{}' for {} - falling back to software decode.", av_hwdevice_get_type_name(type), _filename.c_str());
                type = AV_HWDEVICE_TYPE_NONE;
            } else {
                _codecContext->hw_device_ctx = av_buffer_ref(_hw_device_ctx);
                if (!usingCuvid) {
                    _codecContext->get_format = get_hw_format;
                }
                const char *devName = "";
#if __has_include(<libavdevice/avdevice.h>)
                devName = av_hwdevice_get_type_name(type);
#endif
                spdlog::debug("Hardware decoding('{}') enabled for codec '{}'", devName, decoderToUse->long_name);
            }
        } else {
            spdlog::debug("Software decoding enabled for codec '{}'", decoderToUse->long_name);
        }
    }
    _videoToolboxAccelerated = AppleVideoToolboxBridge::SetupVideoToolboxAcceleration(_codecContext, HW_ACCELERATION_ENABLED && allowHWDecoder);

    AVDictionary *opts = nullptr;
    if (usingCuvid) {
        // Limit NVDEC surface pool: default is max(DPB,20) per decoder instance.
        // With many models each holding their own VideoReader this multiplies to
        // several GB.  8 covers H.264 Level 4.1 max DPB (8 ref frames) while
        // keeping per-instance VRAM well below the 20-surface default.
        av_dict_set_int(&opts, "surfaces", 8, 0);
    }
    if (avcodec_open2(_codecContext, decoderToUse, &opts) < 0) {
        av_dict_free(&opts);
        avcodec_free_context(&_codecContext);
        _codecContext = nullptr;
        if (allowHWDecoder && IsHardwareAcceleratedVideo()) {
            spdlog::warn("VideoReader: HW decoder '{}' failed to open for {}; falling back to software decode", decoderToUse->name, _filename.c_str());
            reopenContext(false);
        } else {
            spdlog::error("VideoReader: Couldn't open the context with the decoder in {}", _filename.c_str());
        }
        return;
    }
    av_dict_free(&opts);
}

static int64_t MStoDTS(int ms, double dtspersec)
{
    return (int64_t)(((double)ms * dtspersec) / 1000.0);
}

static int DTStoMS(int64_t dts , double dtspersec)
{
    if (dtspersec > 1000 && dtspersec < UINT_MAX) {
        int64_t dtsps = (int64_t)dtspersec;
        dts *= 1000;
        dts /= dtsps;
        return dts;
    }
    return (int)((1000.0 * (double)dts) / dtspersec);
}

long FFmpegVideoReader::GetVideoLengthStatic(const std::string& filename)
{
    AVFormatContext* formatContext = nullptr;
    int res = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    if (res != 0) {
        return 0;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        return 0;
    }

    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        avformat_close_input(&formatContext);
        return 0;
    }

    AVStream* videoStream = formatContext->streams[streamIndex];
    videoStream->discard = AVDISCARD_NONE;

    long frames = (long)videoStream->nb_frames;
    long lengthMS = 0;

    if (frames > 0) {
        if (videoStream->avg_frame_rate.num != 0) {
            lengthMS = ((double)frames * (double)videoStream->avg_frame_rate.den * 1000.0) / (double)videoStream->avg_frame_rate.num;
        }
    }

    if (lengthMS <= 0) {
        if (videoStream->avg_frame_rate.den != 0) {
            lengthMS = (double)formatContext->duration * (double)videoStream->avg_frame_rate.num / (double)videoStream->avg_frame_rate.den;
        }
    }

    if (lengthMS <= 0) {
        lengthMS = (double)formatContext->duration / 1000.0;
    }

    avformat_close_input(&formatContext);

    return lengthMS;
}

FFmpegVideoReader::~FFmpegVideoReader()
{
    #ifdef _WIN32
    if (_windowsHardwareVideoReader != nullptr) {
        delete _windowsHardwareVideoReader;
        _windowsHardwareVideoReader = nullptr;
    }
    #endif

    if (_packet != nullptr) {
        av_packet_free(&_packet);
        _packet = nullptr;
    }
    if (_swsCtx != nullptr) {
        sws_freeContext(_swsCtx);
        _swsCtx = nullptr;
    }
    if (_srcFrame != nullptr) {
        av_frame_free(&_srcFrame);
    }
    if (_srcFrame2 != nullptr) {
        av_frame_free(&_srcFrame2);
    }
    if (_dstFrame != nullptr) {
        if (_dstFrame->data[0] != nullptr) {
            av_free(_dstFrame->data[0]);
        }
        av_free(_dstFrame);
        _dstFrame = nullptr;
    }
    if (_dstFrame2 != nullptr) {
        if (_dstFrame2->data[0] != nullptr) {
            av_free(_dstFrame2->data[0]);
        }
        av_free(_dstFrame2);
        _dstFrame2 = nullptr;
    }
    if (_codecContext != nullptr) {
        if (_keyFrameCount != _codecContext->keyint_min) {
            spdlog::debug("Key frame count was adjusted from {} to {}.", _codecContext->keyint_min, _keyFrameCount);
        }
        AppleVideoToolboxBridge::CleanupVideoToolbox(_codecContext, hwDecoderCache);
        hwDecoderCache = nullptr;
        avcodec_free_context(&_codecContext);
        _codecContext = nullptr;
    }
    if (_formatContext != nullptr) {
        avformat_close_input(&_formatContext);
        _formatContext = nullptr;
    }
    if (_hw_device_ctx != nullptr) {
        av_buffer_unref(&_hw_device_ctx);
        _hw_device_ctx = nullptr;
    }
}

void FFmpegVideoReader::Seek(int timestampMS, bool readFrame)
{
    #ifdef _WIN32
    if (_windowsHardwareVideoReader != nullptr) {
        _windowsHardwareVideoReader->Seek(timestampMS);
        _curPos = _windowsHardwareVideoReader->GetPos();
        if (_curPos >= (int)_windowsHardwareVideoReader->GetDuration()) {
            _atEnd = true;
        } else {
            _atEnd = false;
        }
        return;
    }
    #endif

    if (_valid) {
#ifdef VIDEO_EXTRALOGGING
        spdlog::info("VideoReader: Seeking to {} ms.", timestampMS);
#endif
        if (_atEnd && !_abandonHardwareDecode && (_videoToolboxAccelerated || _hw_device_ctx)) {
            reopenContext();
        }

        if (_codecContext == nullptr) return;

        if (timestampMS < _lengthMS) {
            _atEnd = false;
        } else {
            _atEnd = true;
            avcodec_flush_buffers(_codecContext);
            av_seek_frame(_formatContext, _streamIndex, MStoDTS(_lengthMS, _dtspersec), AVSEEK_FLAG_FRAME);
            return;
        }

        avcodec_flush_buffers(_codecContext);

        if (timestampMS <= 0) {
            int f = av_seek_frame(_formatContext, _streamIndex, 0, AVSEEK_FLAG_FRAME);
            if (f != 0) {
                spdlog::info("       VideoReader: Error seeking to {}.", timestampMS);
            }
        } else {
            int f = av_seek_frame(_formatContext, _streamIndex, MStoDTS(timestampMS, _dtspersec), AVSEEK_FLAG_BACKWARD);
            if (f != 0) {
                spdlog::info("       VideoReader: Error seeking to {}.", timestampMS);
            }
        }

        _curPos = -1000;
        if (readFrame) {
            GetNextFrame(timestampMS, 0);
        }
    }
}

bool FFmpegVideoReader::readFrame(int timestampMS) {
    if (_codecContext == nullptr) return false;
    int rc = 0;
    if ((rc = avcodec_receive_frame(_codecContext, _srcFrame)) == 0) {
        if (_srcFrame->pts == (int64_t)0x8000000000000000LL) {
            _curPos = (_srcFrame->pkt_dts * _lengthMS) / _frames;
        } else {
            _curPos = DTStoMS(_srcFrame->pts, _dtspersec);
        }
        if (_firstFramePos == -1) {
            _firstFramePos = _curPos;
        }
        if (_firstFramePos > timestampMS) {
            timestampMS = _firstFramePos;
        }
        bool unrefSrcFrame2 = false;
        if ((double)_curPos / (double)_frames >= ((double)timestampMS / (double)_frames) - 2.0) {
            #ifdef VIDEO_EXTRALOGGING
            spdlog::debug("    Decoding video frame {}.", _curPos);
            #endif
            bool hardwareScaled = false;
            int scaleAlgorithm = VideoScaleAlgorithmToSWS(_scaleAlgorithm);
            if (AppleVideoToolboxBridge::IsVideoToolboxAcceleratedFrame(_srcFrame)) {
                if (_wantsHWType) {
                    hardwareScaled = true;
                    std::swap(_dstFrame2, _srcFrame);
                } else {
                    hardwareScaled = AppleVideoToolboxBridge::VideoToolboxScaleImage(_codecContext, _srcFrame, _dstFrame2, hwDecoderCache, scaleAlgorithm);
                }
            }

            if (!hardwareScaled) {
                AVFrame* f = nullptr;
                if (IsHardwareAcceleratedVideo() && _codecContext->hw_device_ctx != nullptr && _srcFrame->format == __hw_pix_fmt && !_abandonHardwareDecode) {
                    bool hwscale = false;
                    if (!hwscale) {
                        if (av_hwframe_transfer_data(_srcFrame2, _srcFrame, 0) < 0) {
                            f = _srcFrame;
                        } else {
                            unrefSrcFrame2 = true;
                            f = _srcFrame2;
                        }
                    }

                    if (_abandonHardwareDecode && _swsCtx != nullptr) {
                        spdlog::warn("VideoReader: This could get ugly ... we have abandoned hardware decode but we already had a sws Context.");
                    }
                } else {
                    f = _srcFrame;
                }

                if (f == nullptr) {
                    spdlog::warn("VideoReader: Strange f was not valid so setting it to the source frame.");
                    f = _srcFrame;
                }

                if (_swsCtx == nullptr) {
                    if (_abandonHardwareDecode) {
                        spdlog::debug("VideoReader: Using software decode (hardware decoding unavailable for this file).");
                    }
                    if (IsHardwareAcceleratedVideo() && _codecContext->hw_device_ctx != nullptr && _srcFrame->format == __hw_pix_fmt && !_abandonHardwareDecode) {
                        spdlog::debug("Hardware format {} -> Software format {}.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame->format), av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt, scaleAlgorithm, nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            spdlog::error("VideoReader: Error creating SWSContext");
                        } else {
                            spdlog::debug("Hardware Decoding Pixel format conversion {} -> {}.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format), av_get_pix_fmt_name(_pixelFmt));
                            spdlog::debug("Size conversion {},{} -> {},{}.", f->width, f->height, _width, _height);
                        }
                    } else {
                        spdlog::debug("Software format {} -> Software format {}.", av_get_pix_fmt_name((AVPixelFormat)f->format), av_get_pix_fmt_name((AVPixelFormat)_pixelFmt));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt, scaleAlgorithm, nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            spdlog::error("VideoReader: Error creating SWSContext");
                        } else {
                            spdlog::debug("Software Decoding Pixel format conversion {} -> {}.", av_get_pix_fmt_name(_codecContext->pix_fmt), av_get_pix_fmt_name(_pixelFmt));
                            spdlog::debug("Size conversion {},{} -> {},{}.", f->width, f->height, _width, _height);
                        }
                    }
                }

                if (_swsCtx != nullptr) {
                    sws_scale(_swsCtx, f->data, f->linesize, 0,
                        f->height, _dstFrame2->data,
                        _dstFrame2->linesize);
                }
            }
            std::swap(_dstFrame, _dstFrame2);
        }
        av_frame_unref(_srcFrame);
        if (unrefSrcFrame2) {
            av_frame_unref(_srcFrame2);
        }
        return true;
    } else if (rc != AVERROR(EAGAIN)) {
        spdlog::debug("avcodec_receive_frame failed {} - abandoning video read.", rc);
        _abort = true;
    }
    return false;
}

VideoFrame* FFmpegVideoReader::GetNextFrame(int timestampMS, int gracetime)
{
    if (!_valid || _frames == 0) {
        return nullptr;
    }

    if (timestampMS > _lengthMS) {
        _atEnd = true;
        return nullptr;
    }

#ifdef _WIN32
    if (_windowsHardwareVideoReader != nullptr) {
        AVFrame* frame = _windowsHardwareVideoReader->GetNextFrame(timestampMS, gracetime);
        _curPos = _windowsHardwareVideoReader->GetPos();
        if (_curPos >= (int)_windowsHardwareVideoReader->GetDuration()) {
            _atEnd = true;
            return nullptr;
        } else {
            if (frame) {
                PopulateVideoFrame(_videoFrame, frame, AVPixelFormatToVideoPixelFormat(_pixelFmt));
                return &_videoFrame;
            }
            return nullptr;
        }
    }
#endif

#ifdef VIDEO_EXTRALOGGING
    spdlog::debug("Video {} getting frame {}.", _filename, timestampMS);
#endif

    int currenttime = GetPos();
    int timeOfNextFrame = currenttime + _frameMS;
    int timeOfPrevFrame = currenttime - _frameMS;

    if (_firstFramePos >= timestampMS) {
        timestampMS = _firstFramePos;
    }

    if (timestampMS >= currenttime && timestampMS < timeOfNextFrame) {
        PopulateVideoFrame(_videoFrame, _dstFrame, AVPixelFormatToVideoPixelFormat((AVPixelFormat)_dstFrame->format));
        return &_videoFrame;
    }
    if (timestampMS >= timeOfPrevFrame - 1 && timestampMS < currenttime) {
        PopulateVideoFrame(_videoFrame, _dstFrame2, AVPixelFormatToVideoPixelFormat((AVPixelFormat)_dstFrame2->format));
        return &_videoFrame;
    }

    if (currenttime > timestampMS + gracetime || timestampMS - currenttime > 1000) {
#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("    Video {} seeking from {} to {}.", _filename, currenttime, timestampMS);
#endif
        Seek(timestampMS, false);
        currenttime = GetPos();
    }

    if (timestampMS <= _lengthMS) {
        bool firstframe = false;
        if (currenttime <= 0 && timestampMS == 0) {
            firstframe = true;
        }

        bool seekedForward = false;
        while (!_abort && (firstframe || ((currenttime + (_frameMS / 2.0)) < timestampMS)) &&
               currenttime <= _lengthMS &&
               (av_read_frame(_formatContext, _packet)) == 0)
        {
            if (_packet->stream_index == _streamIndex) {
                int decodeCount = 0;
                int ret = avcodec_send_packet(_codecContext, _packet);
                while (!_abort && ret != 0) {
                    if (ret != AVERROR(EAGAIN) && !_abandonHardwareDecode && (_videoToolboxAccelerated || _hw_device_ctx )) {
                        char errbuf[AV_ERROR_MAX_STRING_SIZE];
                        av_strerror(ret, errbuf, sizeof(errbuf));
                        spdlog::warn("VideoReader: Hardware video decoding failed for {} (error: {}). Reverting to software decoding.", (const char*)_filename.c_str(), errbuf);
                        reopenContext(false);
                        if (_codecContext == nullptr) {
                            spdlog::error("VideoReader: Failed to reopen context for {} after HW decode error; aborting render.", (const char*)_filename.c_str());
                            _valid = false;
                            av_packet_unref(_packet);
                            return nullptr;
                        }
                        Seek(timestampMS, false);
                        currenttime = GetPos();
                        ret = 0;
                    } else {
                        if (readFrame(timestampMS)) {
                            firstframe = false;
                            currenttime = _curPos;
                        } else {
                            decodeCount++;
                            if (decodeCount == 100) {
                                return nullptr;
                            }
                        }
                        ret = avcodec_send_packet(_codecContext, _packet);
                    }
                }

                if (currenttime != -1000 && currenttime < timestampMS - _frameMS * (_keyFrameCount + 2)) {
                    if (seekedForward) {
                        _keyFrameCount++;
                    } else {
                        seekedForward = true;
#ifdef VIDEO_EXTRALOGGING
                        spdlog::debug("    Video {} seeking forward from {} to {}.", (const char*)_filename.c_str(), currenttime, timestampMS);
#endif
                        Seek(timestampMS, false);
                        currenttime = GetPos();
                    }
                }
            }
            av_packet_unref(_packet);
        }
    } else {
        _atEnd = true;
        return nullptr;
    }

    if (_dstFrame->data[0] == nullptr || currenttime > _lengthMS) {
        _atEnd = true;
        return nullptr;
    } else {
        int currenttime = GetPos();
        AVFrame* resultFrame;
        if (timestampMS >= currenttime) {
            resultFrame = _dstFrame;
        } else {
            resultFrame = _dstFrame2;
        }
        PopulateVideoFrame(_videoFrame, resultFrame, AVPixelFormatToVideoPixelFormat((AVPixelFormat)resultFrame->format));
        return &_videoFrame;
    }
}
