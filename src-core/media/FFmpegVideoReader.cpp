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
#include "media/FFmpegCompat.h"

//#define VIDEO_EXTRALOGGING

#undef min
#include <algorithm>
#include <climits>
#include <cstdarg>
#include <cstring>
#include <filesystem>
#include <list>
#include <mutex>
#include <set>
#include <tuple>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#if __has_include(<libavdevice/avdevice.h>)
#include <libavdevice/avdevice.h>
#endif
}

#include "../utils/SpecialOptions.h"
#include "Parallel.h"
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

// FFmpeg writes its own diagnostics straight to stderr, so decoder chatter -
// most visibly a hwaccel that turns out not to handle the stream - lands on the
// console looking like a crash dump while never reaching the xLights log at all.
// Route it into spdlog: only a fatal is worth a user's attention, the rest is
// detail that the xLights_logger=debug option can turn on when diagnosing.
static void ffmpeg_log_callback(void* avcl, int level, const char* fmt, va_list vl)
{
    if (level > av_log_get_level()) {
        return;
    }
    char buf[1024];
    int prefix = 1;
    av_log_format_line2(avcl, level, fmt, vl, buf, sizeof(buf), &prefix);
    std::string msg(buf);
    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
        msg.pop_back();
    }
    if (msg.empty()) {
        return;
    }
    if (level <= AV_LOG_FATAL) {
        spdlog::error("FFmpeg: {}", msg);
    } else {
        spdlog::debug("FFmpeg: {}", msg);
    }
}

static void EnsureFFmpegLogRedirect()
{
    static std::once_flag redirectOnce;
    std::call_once(redirectOnce, []() {
        av_log_set_level(AV_LOG_WARNING);
        av_log_set_callback(ffmpeg_log_callback);
    });
}

// A hardware decoder can open cleanly and still fail once real slices arrive:
// the device exists and the codec advertises the hwaccel, but the driver has no
// config for this codec profile.  A VM is the common case - Parallels' virtio
// GPU brings up a VA-API display that decodes nothing - and FFmpeg reports it as
// AVERROR_INVALIDDATA, indistinguishable from a corrupt file.  Remembering the
// rejected combination matters as much as reporting it accurately: a render
// builds a reader per model, so otherwise every one of them repeats the doomed
// probe and its log noise.  Keyed on profile, and only ever recorded for a
// combination that has never decoded, so one damaged file cannot switch off
// hardware decoding for every other video using the same codec.
using HWStreamKey = std::tuple<int, int, int>; // device type, codec id, profile
static std::mutex s_hwStreamMutex;
static std::set<HWStreamKey> s_hwStreamRejected;
static std::set<HWStreamKey> s_hwStreamDecoded;

static HWStreamKey MakeHWStreamKey(AVHWDeviceType type, AVCodecID codec, int profile)
{
    return HWStreamKey((int)type, (int)codec, profile);
}

static bool IsHWStreamRejected(AVHWDeviceType type, AVCodecID codec, int profile)
{
    std::lock_guard<std::mutex> lock(s_hwStreamMutex);
    return s_hwStreamRejected.count(MakeHWStreamKey(type, codec, profile)) != 0;
}

static void NoteHWStreamDecoded(AVHWDeviceType type, AVCodecID codec, int profile)
{
    std::lock_guard<std::mutex> lock(s_hwStreamMutex);
    s_hwStreamDecoded.insert(MakeHWStreamKey(type, codec, profile));
}

// Returns true when this combination has never produced a hardware frame, ie the
// failure is a capability gap rather than one bad file, and it was blacklisted.
static bool NoteHWStreamFailed(AVHWDeviceType type, AVCodecID codec, int profile)
{
    std::lock_guard<std::mutex> lock(s_hwStreamMutex);
    const HWStreamKey key = MakeHWStreamKey(type, codec, profile);
    if (s_hwStreamDecoded.count(key) != 0) {
        return false;
    }
    s_hwStreamRejected.insert(key);
    return true;
}

static std::string DescribeStream(AVCodecID codec, int profile)
{
    std::string desc = avcodec_get_name(codec);
    const char* profileName = avcodec_profile_name(codec, profile);
    if (profileName != nullptr) {
        desc += " ";
        desc += profileName;
        desc += " profile";
    } else if (profile >= 0) {
        desc += " profile " + std::to_string(profile);
    }
    return desc;
}

#ifdef _WIN32
// AVI never goes to Media Foundation.
//
// It is the container people reach for when they want uncompressed or
// lossless frames, so in practice it carries codecs no GPU decodes -
// rawvideo, and H.264 in 4:4:4, which the Media Foundation H.264 decoder does
// not support at all. Media Foundation still accepts many of those files and
// then decodes them on the CPU inside its own machinery, which is slower than
// FFmpeg doing the same work and holds one of the scarce decoder sessions while
// it does it. macOS reached the same conclusion and stopped handing AVI to
// AVFoundation; this keeps the two platforms on the same decoder for the same
// file.
static bool IsAviFile(const std::string& filename)
{
    const auto dot = filename.find_last_of('.');
    if (dot == std::string::npos) {
        return false;
    }
    std::string ext = filename.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)::tolower(c); });
    return ext == "avi";
}

// The comment above names the actual disqualifying property as the codec
// profile ("H.264 in 4:4:4, which the Media Foundation H.264 decoder does not
// support at all"), but only ever tested the container extension - so an MP4
// or MOV carrying that same profile sailed straight past it. Media Foundation
// does not reject such a stream outright either: it silently substitutes a
// placeholder frame for roughly the opening 5 seconds before recovering,
// which reads as valid video (a render finishes, no error is logged) but is
// wrong - solid blue in place of the real decoded picture. Probing the
// profile is a full header parse (no decode), so it costs one open/close per
// file, same as the existing GetVideoLengthStatic probe just above.
//
// Also covers codecs Media Foundation has no decoder for at all, regardless
// of profile - ProRes chief among them (no vendor ships a ProRes MFT on
// Windows). Unlike the 4:4:4 case this fails loudly (MF_E_TOPO_CODEC_NOT_FOUND
// while negotiating the media type) rather than silently, so it was never a
// correctness bug - it was 632 of those failed negotiations logged across one
// render (one per model on the affected files). This skips them. Measured:
// that render's total time was unaffected (346s vs 357s) - whatever else
// makes a ProRes/MJPEG-heavy sequence slow dominates over this cost - so
// treat this as removing pointless failure noise and redundant per-model
// construction work, not as a render-time fix.
static bool IsUnsupportedProfile(const std::string& filename)
{
    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        return false;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        return false;
    }
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    bool unsupported = false;
    if (streamIndex >= 0) {
        AVCodecParameters* pars = formatContext->streams[streamIndex]->codecpar;
        if (pars->codec_id == AV_CODEC_ID_H264) {
            unsupported = pars->profile == AV_PROFILE_H264_HIGH_444 ||
                          pars->profile == AV_PROFILE_H264_HIGH_444_PREDICTIVE ||
                          pars->profile == AV_PROFILE_H264_HIGH_444_INTRA;
        } else if (pars->codec_id == AV_CODEC_ID_PRORES) {
            unsupported = true;
        }
    }
    avformat_close_input(&formatContext);
    return unsupported;
}
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
// Media Foundation by default on Windows: measured 2.3x faster than the FFmpeg
// path on video-heavy sequences (3.0x on a weaker box), with output closer to
// macOS. Files it cannot open - uncompressed/rawvideo, mkv - fall back to
// FFmpeg automatically, and those are cheap to decode anyway.
WINHARDWARERENDERTYPE FFmpegVideoReader::HW_ACCELERATION_TYPE = WINHARDWARERENDERTYPE::DIRECX11_API;

void FFmpegVideoReader::SetHardwareAcceleratedVideo(bool accel)
{
    HW_ACCELERATION_ENABLED = accel;
}

void FFmpegVideoReader::SetHardwareRenderType(int type)
{
    HW_ACCELERATION_TYPE = static_cast<WINHARDWARERENDERTYPE>( type );
}

void FFmpegVideoReader::InitHWAcceleration() {
    EnsureFFmpegLogRedirect();
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


// swscale defaults to BT.601 coefficients and, for a YUV->RGB conversion, does
// not produce full-range RGB unless told to. Left alone it therefore disagrees
// with both AVFoundation (macOS) and the Media Foundation reader, which do
// full-range output: measured on one h264 file, the FFmpeg path came out ~7.5%
// darker than macOS with a quarter as many exact-black pixels. Blacks that are
// not black matter here because VideoEffect's TransparentBlack keys off
// R+G+B > threshold. State the matrix and ask for full-range RGB explicitly.
// Takes the colour metadata from the CODEC CONTEXT, not the frame. A frame that
// came back from av_hwframe_transfer_data carries no colorspace/color_range -
// that call moves pixels, not properties - so reading them off the frame gives
// the stream's real values on a software-decoded frame and "unspecified" on a
// hardware-decoded one, and a reader can switch between the two at runtime when
// a failed transfer sets _abandonHardwareDecode. The codec context has the
// stream's values either way.
static void ApplySwsColorspace(SwsContext* ctx, const AVCodecContext* cc, int height)
{
    if (ctx == nullptr || cc == nullptr) {
        return;
    }
    int coefId;
    switch (cc->colorspace) {
    case AVCOL_SPC_BT709:
        coefId = SWS_CS_ITU709;
        break;
    case AVCOL_SPC_SMPTE170M:
    case AVCOL_SPC_BT470BG:
        coefId = SWS_CS_ITU601;
        break;
    default:
        // Unspecified in the bitstream (the common case) - use the SD/HD split
        // a decoder would assume.
        coefId = (height > 576) ? SWS_CS_ITU709 : SWS_CS_ITU601;
        break;
    }
    const int* coef = sws_getCoefficients(coefId);
    const int srcRange = (cc->color_range == AVCOL_RANGE_JPEG) ? 1 : 0;

    int* invTable = nullptr;
    int* table = nullptr;
    int sr = 0, dr = 0, brightness = 0, contrast = 0, saturation = 0;
    if (sws_getColorspaceDetails(ctx, &invTable, &sr, &table, &dr, &brightness, &contrast, &saturation) < 0) {
        brightness = 0;
        contrast = 1 << 16;
        saturation = 1 << 16;
    }
    if (sws_setColorspaceDetails(ctx, coef, srcRange, coef, 1 /* full-range RGB out */,
                                 brightness, contrast, saturation) < 0) {
        spdlog::debug("VideoReader: sws_setColorspaceDetails not supported for this conversion");
    }
}


// Snap uniformly-near-black pixels to exact (0,0,0), matching what the macOS
// bridge does in copyPixelBufferToFrame and what the Media Foundation reader
// now does. H.264 artifacts in dark regions decode to values like (2,0,2) -
// sum 4, one over a typical TransparentBlack threshold of 3 - which render as
// faint blotches where the user expects clean transparency, and as reveal
// halos in the "1 reveals 2" composite modes. Requires ALL THREE channels <= 4
// so deliberately dark single-channel content (e.g. RGB(0,0,10)) survives.
static void SnapNearBlack(AVFrame* frame, int width, int height, int channels)
{
    if (frame == nullptr || frame->data[0] == nullptr) {
        return;
    }
    uint8_t* base = frame->data[0];
    const int stride = frame->linesize[0];
    parallel_for(0, height, [base, stride, width, channels](int y) {
        uint8_t* row = base + (size_t)y * stride;
        for (int x = 0; x < width; ++x) {
            uint8_t* p = row + (size_t)x * channels;
            if (p[0] <= 4 && p[1] <= 4 && p[2] <= 4) {
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
            }
        }
    });
}


// swscale's bicubic is a poor choice for a large reduction: with a 10:1
// downscale it samples a handful of taps out of each 10x10 source block and
// aliases, which showed up as a systematic ~7% level difference against
// AVFoundation's vImage resampler and the Media Foundation video processor,
// both of which average properly. SWS_AREA is the correct filter for that
// regime. Only applied when the user has not chosen a specific algorithm.
static int PickScaleAlgorithm(VideoScaleAlgorithm alg, int srcW, int srcH, int dstW, int dstH)
{
    int sws = VideoScaleAlgorithmToSWS(alg);
    if (alg == VideoScaleAlgorithm::Default && dstW > 0 && dstH > 0 &&
        (srcW >= dstW * 2 || srcH >= dstH * 2)) {
        return SWS_AREA;
    }
    return sws;
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
    EnsureFFmpegLogRedirect();
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
    _usenativeresolution = usenativeresolution;
    _keepaspectratio = keepaspectratio;

    // A recent read past its deadline stands hardware decode down for a while;
    // go straight to software rather than stall this file too.
    if (HW_ACCELERATION_ENABLED && ::IsWindows8OrGreater() && HW_ACCELERATION_TYPE == WINHARDWARERENDERTYPE::DIRECX11_API &&
        !IsAviFile(filename) && !IsUnsupportedProfile(filename) &&
        !WindowsHardwareVideoReader::MediaFoundationInCooldown()) {
        _windowsHardwareVideoReader = new WindowsHardwareVideoReader(filename, _wantAlpha, usenativeresolution, keepaspectratio, maxwidth, maxheight, _pixelFmt);
        if (_windowsHardwareVideoReader->IsOk()) {
            _frames = _windowsHardwareVideoReader->GetFrames();
            _lengthMS = _windowsHardwareVideoReader->GetDuration();
            _height = _windowsHardwareVideoReader->GetHeight();
            _width = _windowsHardwareVideoReader->GetWidth();
            _frameMS = _windowsHardwareVideoReader->GetFrameMS();
            _valid = true;

            spdlog::debug("Video loaded: " + filename);
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

    OpenWithFFmpeg(filename, usenativeresolution, keepaspectratio, maxwidth, maxheight);
}

// The software decode path. Split out of the constructor so it can also be run
// after a hardware reader gives up part way through a file.
void FFmpegVideoReader::OpenWithFFmpeg(const std::string& filename, bool usenativeresolution, bool keepaspectratio, int maxwidth, int maxheight)
{
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

    // Sized in 64-bit and failed explicitly: a requested size whose byte count
    // does not fit an int wraps negative, av_malloc rejects it, and every decoded
    // frame then writes through null frame data. _valid gates every read path.
    const long long frameBytes = (long long)_width * (long long)_height * GetPixelChannels();
    if (_width <= 0 || _height <= 0 || frameBytes > (long long)INT_MAX) {
        spdlog::error("FFmpegVideoReader: refusing a {}x{} reader for {} ({} bytes per frame)", _width, _height, filename, frameBytes);
        return;
    }

    _dstFrame = av_frame_alloc();
    _dstFrame->width = _width;
    _dstFrame->height = _height;
    _dstFrame->linesize[0] = _width * GetPixelChannels();
    _dstFrame->data[0] = (uint8_t *)av_malloc((size_t)frameBytes);
    _dstFrame->format = _pixelFmt;
    _dstFrame2 = av_frame_alloc();
    _dstFrame2->width = _width;
    _dstFrame2->height = _height;
    _dstFrame2->linesize[0] = _width * GetPixelChannels();
    _dstFrame2->data[0] = (uint8_t *)av_malloc((size_t)frameBytes);
    _dstFrame2->format = _pixelFmt;
    if (_dstFrame->data[0] == nullptr || _dstFrame2->data[0] == nullptr) {
        spdlog::error("FFmpegVideoReader: could not allocate {}x{} frames for {}", _width, _height, filename);
        return;
    }
    // Zero the destinations: av_malloc does not, and sws_scale is not guaranteed
    // to write every pixel for every filter and size ratio, so anything it skips
    // would be served to the renderer as whatever was in the heap.
    memset(_dstFrame->data[0], 0x00, (size_t)frameBytes);
    memset(_dstFrame2->data[0], 0x00, (size_t)frameBytes);

    _srcFrame = av_frame_alloc();
    _srcFrame2 = av_frame_alloc();

    _packet = av_packet_alloc();
    _valid = true;

    spdlog::debug("Video loaded: " + filename);
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

#ifdef _WIN32
// The hardware reader stopped responding part way through the file. Discard it
// and reopen the same file on the software decoder so the render keeps its
// frames instead of losing the effect.
bool FFmpegVideoReader::FallBackFromHardwareReader()
{
    delete _windowsHardwareVideoReader;
    _windowsHardwareVideoReader = nullptr;

    // Everything the hardware reader published about the file is recomputed:
    // the software path can size and time frames differently.
    _valid = false;
    _atEnd = false;
    _curPos = -1000;
    _firstFramePos = -1;
    _frames = 0;
    _lengthMS = 0.0;
    _dtspersec = 1.0;
    _width = _maxwidth;
    _height = _maxheight;

    OpenWithFFmpeg(_filename, _usenativeresolution, _keepaspectratio, _maxwidth, _maxheight);
    if (!_valid) {
        spdlog::error("FFmpegVideoReader: software fallback could not open {}", _filename);
    } else {
        spdlog::info("FFmpegVideoReader: now decoding {} in software", _filename);
    }
    return _valid;
}
#endif

// Whether a hardware device of this type can actually be opened on this machine.
// Absence is machine-global, so it is remembered: a render creates dozens of
// readers and re-probing a device that isn't there costs a driver round trip
// apiece while holding the serialising lock.  A type that has opened at least
// once is never written off, because a later failure there is resource
// exhaustion under load rather than absent hardware, and that does recover.
static bool CreateHWDeviceContext(AVHWDeviceType type, AVBufferRef*& ctx, const std::string& filename)
{
    // Serialize HW device creation: concurrent CUDA/NVDEC init from many
    // render threads exhausts driver session limits and corrupts shared state.
    static std::mutex s_hwDeviceCreateMutex;
    static std::set<AVHWDeviceType> s_everOpened;
    static std::set<AVHWDeviceType> s_absent;
    std::lock_guard<std::mutex> hwLock(s_hwDeviceCreateMutex);

    if (s_absent.find(type) != s_absent.end()) {
        return false;
    }
    if (av_hwdevice_ctx_create(&ctx, type, nullptr, nullptr, 0) < 0) {
        ctx = nullptr;
        if (s_everOpened.find(type) == s_everOpened.end()) {
            spdlog::warn("VideoReader: HW device '{}' unavailable ({}) - not retrying it this session.",
                         av_hwdevice_get_type_name(type), filename);
            s_absent.insert(type);
        } else {
            spdlog::warn("VideoReader: HW device '{}' temporarily unavailable for {} - falling back to software decode for this file.",
                         av_hwdevice_get_type_name(type), filename);
        }
        return false;
    }
    s_everOpened.insert(type);
    return true;
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
    _hwDeviceType = ::AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
    _hwDecodeConfirmed = false;

    const AVCodecID streamCodecId = _videoStream != nullptr ? _videoStream->codecpar->codec_id : AV_CODEC_ID_NONE;
    const int streamProfile = _videoStream != nullptr ? _videoStream->codecpar->profile : -99;

    enum AVHWDeviceType type = ::AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
    // Set when the chosen device type is served by a standalone '<codec>_qsv'
    // decoder rather than by a hwaccel of the native decoder - see the QSV branch
    // in the candidate loop below.
    const AVCodec* qsvDecoder = nullptr;
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

        // Take the first candidate that is usable end to end: the name has to
        // resolve, this decoder has to advertise a matching hw config, and the
        // device has to actually open.  Accepting the first merely *recognised*
        // name instead meant any FFmpeg build with CUDA compiled in always chose
        // cuda and then fell back to software on non-NVIDIA machines, never
        // reaching qsv/d3d11va/vulkan.
        for (int i = 0;; i++) {
            const AVCodecHWConfig* config = avcodec_get_hw_config(_decoder, i);
            if (!config) {
                break;
            }
            spdlog::debug("VideoReader: decoder '{}' hw config {}: device_type={} pix_fmt={} methods=0x{:x}",
                          _decoder->name, i, av_hwdevice_get_type_name(config->device_type),
                          av_get_pix_fmt_name(config->pix_fmt), (unsigned)config->methods);
        }
        for (const auto& it : hwdecoders) {
            const AVHWDeviceType candidate = av_hwdevice_find_type_by_name(it.c_str());
            if (candidate == AV_HWDEVICE_TYPE_NONE) {
                spdlog::debug("VideoReader: hw candidate '{}' rejected - not built into this FFmpeg.", it);
                continue;
            }

            // QSV is not a hwaccel of the native decoder - FFmpeg ships it only
            // as standalone '<codec>_qsv' decoders - so it can never match the
            // hw-config scan below and was previously skipped on every machine,
            // however capable the GPU.  Resolve it the same way CUDA resolves
            // '<codec>_cuvid', except that it has to happen HERE rather than
            // after the loop, because there is no hw config to get it this far.
            // Given a device the _qsv decoder returns real hardware frames, so
            // they go through av_hwframe_transfer_data to NV12 and then the usual
            // scale to RGBA - the same route d3d11va already takes.
            if (candidate == AV_HWDEVICE_TYPE_QSV) {
                const std::string qsvName = std::string(_decoder->name) + "_qsv";
                const AVCodec* qsv = avcodec_find_decoder_by_name(qsvName.c_str());
                if (qsv == nullptr) {
                    spdlog::debug("VideoReader: hw candidate 'qsv' rejected - decoder '{}' not in this FFmpeg build.", qsvName);
                    continue;
                }
                if (!CreateHWDeviceContext(candidate, _hw_device_ctx, _filename)) {
                    continue;
                }
                type = candidate;
                qsvDecoder = qsv;
                // Frames arrive in system memory, so there is no hardware pixel
                // format to negotiate; leaving this NONE keeps srcIsHwBacked
                // false and routes them down the software path.
                __hw_pix_fmt = AV_PIX_FMT_NONE;
                spdlog::debug("VideoReader: QSV decoder '{}' selected", qsvName);
                break;
            }

            AVPixelFormat candidatePixFmt = AV_PIX_FMT_NONE;
            for (int i = 0;; i++) {
                const AVCodecHWConfig* config = avcodec_get_hw_config(_decoder, i);
                if (!config) {
                    break;
                }
                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                    config->device_type == candidate) {
                    candidatePixFmt = config->pix_fmt;
                    break;
                }
            }
            if (candidatePixFmt == AV_PIX_FMT_NONE) {
                // Reaching here means the device type is not a hwaccel of this
                // decoder, which is not the same as the hardware being incapable
                // - say so, because a silent skip here is indistinguishable from
                // "your GPU cannot do it" and hides a whole class of problem.
                spdlog::debug("VideoReader: hw candidate '{}' rejected - decoder '{}' advertises no hw config for it.",
                              it, _decoder->name);
                continue;
            }

            // Already proven at slice level that this device cannot decode this
            // codec/profile - skip it rather than repeat the failure per reader.
            if (IsHWStreamRejected(candidate, streamCodecId, streamProfile)) {
                continue;
            }

            if (!CreateHWDeviceContext(candidate, _hw_device_ctx, _filename)) {
                continue;
            }

            type = candidate;
            __hw_pix_fmt = candidatePixFmt;
            break;
        }
    }

    const AVCodec* decoderToUse = _decoder;
    bool usingCuvid = false;
    const bool usingQsv = (qsvDecoder != nullptr);
    if (usingQsv) {
        decoderToUse = qsvDecoder;
    }
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
    if (_hw_device_ctx != nullptr) {
        _codecContext->hw_device_ctx = av_buffer_ref(_hw_device_ctx);
        // Neither cuvid nor the _qsv decoders negotiate a hardware pixel format
        // through get_format - they take the device and hand back frames the
        // ordinary path can consume - and installing the callback for them makes
        // the codec open fail.
        if (!usingCuvid && !usingQsv) {
            _codecContext->get_format = get_hw_format;
        }
        _hwDeviceType = type;
        spdlog::debug("Hardware decoding('{}') enabled for codec '{}'", av_hwdevice_get_type_name(type), decoderToUse->long_name);
    } else {
        spdlog::debug("Software decoding enabled for codec '{}'", decoderToUse->long_name);
    }
    const bool allowVideoToolbox = HW_ACCELERATION_ENABLED && allowHWDecoder &&
                                   !IsHWStreamRejected(AV_HWDEVICE_TYPE_VIDEOTOOLBOX, streamCodecId, streamProfile);
    _videoToolboxAccelerated = AppleVideoToolboxBridge::SetupVideoToolboxAcceleration(_codecContext, allowVideoToolbox);
    if (_videoToolboxAccelerated) {
        _hwDeviceType = AV_HWDEVICE_TYPE_VIDEOTOOLBOX;
    }

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
        // Seek decodes forward to land on the frame, so it can time out too.
        if (_windowsHardwareVideoReader->HasFailed()) {
            if (!FallBackFromHardwareReader()) {
                return;
            }
            // fall through and seek the software decoder instead
        } else {
            _curPos = _windowsHardwareVideoReader->GetPos();
            if (_curPos >= (int)_windowsHardwareVideoReader->GetDuration()) {
                _atEnd = true;
            } else {
                _atEnd = false;
            }
            return;
        }
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
        // A frame arriving at all means hwaccel init succeeded, so a later
        // failure on this device/codec/profile is about the file, not the GPU.
        if (!_hwDecodeConfirmed && _hwDeviceType != AV_HWDEVICE_TYPE_NONE) {
            _hwDecodeConfirmed = true;
            NoteHWStreamDecoded(_hwDeviceType, _videoStream->codecpar->codec_id, _videoStream->codecpar->profile);
        }
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
                // Detect hw-backed frames via hw_frames_ctx OR a matching hw pixel format.
                // h264_cuvid can report format=NV12 while data is in CUDA device memory,
                // so hw_frames_ctx is the reliable indicator.
                bool srcIsHwBacked = IsHardwareAcceleratedVideo() &&
                                     _codecContext->hw_device_ctx != nullptr &&
                                     !_abandonHardwareDecode &&
                                     (_srcFrame->hw_frames_ctx != nullptr ||
                                      (_srcFrame->format == __hw_pix_fmt && __hw_pix_fmt != AV_PIX_FMT_NONE));
                if (srcIsHwBacked) {
                    bool hwscale = false;
                    if (!hwscale) {
                        if (av_hwframe_transfer_data(_srcFrame2, _srcFrame, 0) < 0) {
                            spdlog::warn("VideoReader: av_hwframe_transfer_data failed for {} — abandoning hardware decode.", _filename);
                            spdlog::default_logger()->flush();
                            _abandonHardwareDecode = true;
                            // The transfer allocates the destination before it can
                            // fail, so a failed call can still leave buffers on
                            // _srcFrame2.  Without this the frame keeps them and the
                            // next transfer reuses a frame that was never released.
                            av_frame_unref(_srcFrame2);
                            if (_swsCtx != nullptr) {
                                sws_freeContext(_swsCtx);
                                _swsCtx = nullptr;
                            }
                        } else {
                            unrefSrcFrame2 = true;
                            f = _srcFrame2;
                        }
                    }
                } else {
                    f = _srcFrame;
                }

                if (f == nullptr) {
                    spdlog::warn("VideoReader: No valid CPU frame available — skipping sws_scale for this frame.");
                }

                if (f != nullptr && _swsCtx == nullptr) {
                    if (_abandonHardwareDecode) {
                        spdlog::debug("VideoReader: Using software decode (hardware decoding unavailable for this file).");
                    }
                    if (srcIsHwBacked) {
                        spdlog::debug("Hardware format {} -> Software format {}.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame->format), av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt,
                            PickScaleAlgorithm(_scaleAlgorithm, f->width, f->height, _width, _height),
                            nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            spdlog::error("VideoReader: Error creating SWSContext");
                        } else {
                            ApplySwsColorspace(_swsCtx, _codecContext, f->height);
                            spdlog::debug("Hardware Decoding Pixel format conversion {} -> {}.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format), av_get_pix_fmt_name(_pixelFmt));
                            spdlog::debug("Size conversion {},{} -> {},{}.", f->width, f->height, _width, _height);
                        }
                    } else {
                        spdlog::debug("Software format {} -> Software format {}.", av_get_pix_fmt_name((AVPixelFormat)f->format), av_get_pix_fmt_name((AVPixelFormat)_pixelFmt));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt,
                            PickScaleAlgorithm(_scaleAlgorithm, f->width, f->height, _width, _height),
                            nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            spdlog::error("VideoReader: Error creating SWSContext");
                        } else {
                            ApplySwsColorspace(_swsCtx, _codecContext, f->height);
                            spdlog::debug("Software Decoding Pixel format conversion {} -> {}.", av_get_pix_fmt_name(_codecContext->pix_fmt), av_get_pix_fmt_name(_pixelFmt));
                            spdlog::debug("Size conversion {},{} -> {},{}.", f->width, f->height, _width, _height);
                        }
                    }
                }

                if (f != nullptr) {
                    // Guard: reject any frame that is still hardware-backed — either
                    // because hw_frames_ctx is set, or because the pixel format is a
                    // hardware-accelerated format (AV_PIX_FMT_FLAG_HWACCEL). Passing
                    // such a frame to sws_scale causes an access violation reading
                    // GPU/device memory as if it were CPU data.
                    bool isHwFrame = (f->hw_frames_ctx != nullptr);
                    if (!isHwFrame) {
                        const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get((AVPixelFormat)f->format);
                        isHwFrame = (desc != nullptr && (desc->flags & AV_PIX_FMT_FLAG_HWACCEL));
                    }
                    if (isHwFrame) {
                        const char* fmtName = av_get_pix_fmt_name((AVPixelFormat)f->format);
                        spdlog::warn("VideoReader: frame is still hardware-backed (fmt={}, hw_frames_ctx={}) — skipping sws_scale.",
                            fmtName ? fmtName : "unknown", (void*)f->hw_frames_ctx);
                        spdlog::default_logger()->flush();
                        f = nullptr;
                    }
                }
                if (f != nullptr && _swsCtx != nullptr) {
                    sws_scale(_swsCtx, f->data, f->linesize, 0,
                        f->height, _dstFrame2->data,
                        _dstFrame2->linesize);
                    SnapNearBlack(_dstFrame2, _width, _height, GetPixelChannels());
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
        if (frame == nullptr && _windowsHardwareVideoReader->HasFailed()) {
            // Hardware decode gave up on this file; retry the same request in
            // software rather than serve the effect a missing frame.
            if (!FallBackFromHardwareReader()) {
                return nullptr;
            }
            // fall through to the software path below
        } else {
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
                        const AVCodecID codecId = _videoStream->codecpar->codec_id;
                        const int profile = _videoStream->codecpar->profile;
                        const char* hwName = av_hwdevice_get_type_name(_hwDeviceType);
                        // FFmpeg cannot tell "the GPU has no config for this
                        // profile" from "this file is damaged" - both surface as
                        // AVERROR_INVALIDDATA - so report what actually differs:
                        // the format the hardware was asked to decode.
                        if (NoteHWStreamFailed(_hwDeviceType, codecId, profile)) {
                            spdlog::warn("VideoReader: Hardware video decoding ({}) does not support {} on this machine. Using software decoding for that format from now on. First seen on {}.",
                                         hwName != nullptr ? hwName : "GPU", DescribeStream(codecId, profile), _filename);
                        } else {
                            spdlog::warn("VideoReader: Hardware video decoding failed for {}. Using software decoding for this file.", _filename);
                        }
                        spdlog::debug("VideoReader: hardware decode error was '{}' ({})", errbuf, ret);
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
