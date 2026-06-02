/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/FFmpegVideoWriter.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
}

#include <log.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>

#ifdef min
#undef min
#endif

// Log messages from libav*
void my_av_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
{
    static char message[8192] = { 0 };

    // Create the actual message
    vsnprintf(message, sizeof(message), fmt, vargs);

    // strip off carriage return
    if (strlen(message) > 0) {
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = 0x00;
        }
    }

    spdlog::debug("WriteVideoFile: lvl: {} msg: {}.", level, static_cast<const char*>(message));
}

namespace
{
   // initialize to solid color (varies with each frame)
   bool getVideo(VideoWriterFrame& vf, unsigned frameIndex) {
      if (vf.rgbBuffer == nullptr) {
         return false;
      }
      uint8_t* ptr = vf.rgbBuffer;
      enum Color { Red, Green, Blue } color = Color( frameIndex % 3 );
      int n = vf.rgbBufferSize / 3;
      for ( int i = 0; i < n; ++i ) {
         if (color == Red) {
            *ptr++ = 0xff;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
         } else if (color == Green) {
            *ptr++ = 0x00;
            *ptr++ = 0xff;
            *ptr++ = 0x00;
         } else {
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0xff;
         }
      }
      return true;
   }

   // initialize to silence
   bool getAudio(float* leftCh, float *rightCh, int frameSize) {
      std::memset(leftCh, 0, frameSize * sizeof(float));
      std::memset(rightCh, 0, frameSize * sizeof(float));

      return true;
   }

   bool queryForCancel() {
      return false;
   }

   void progressReporter( int ) {
   }
}

FFmpegVideoWriter::FFmpegVideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly) :
    VideoWriterImpl(outPath, params, videoOnly)
{
    _inPfmt = AV_PIX_FMT_RGB24;

    // MP4/MOV has some restrictions on width... apparently it's common
    // with FFmpeg to just enforce even-number width and height
    if (_outParams.width % 2)
        ++_outParams.width;
    if (_outParams.height % 2)
        ++_outParams.height;

    // We're outputing an H.264 / AAC MP4 file; most players only support profiles with 4:2:0 compression
    _outPfmt = AV_PIX_FMT_YUV420P;

    _getVideo = getVideo;
    _getAudio = getAudio;
    _queryForCancel = queryForCancel;
    _progressReporter = progressReporter;

    for (int x = 0; x < MAX_EXPORT_BUFFER_FRAMES; x++) {
        _videoFrames[x] = nullptr;
    }

#ifdef VIDEOWRITE_DEBUG
    ::av_log_set_callback(my_av_log_callback);
#endif
}

FFmpegVideoWriter::~FFmpegVideoWriter()
{
    cleanup();

#ifdef VIDEOWRITE_DEBUG
    ::av_log_set_callback(nullptr);
#endif
}

void FFmpegVideoWriter::initialize()
{
    // Initialize video & audio
    AVOutputFormat* fmt = (AVOutputFormat*)::av_guess_format(nullptr, _path.c_str(), nullptr);
    if (fmt == nullptr) {
        std::string np = _path + ".mp4";
        fmt = (AVOutputFormat*)::av_guess_format(nullptr, _path.c_str(), nullptr);
    }
    const AVCodec* videoCodec = ::avcodec_find_encoder(fmt->video_codec);
    int status{-1};
    AVDictionary* av_opts = NULL;
    if (_outParams.videoCodec.find("H.264") != std::string::npos ||
        _outParams.videoCodec.find("H.265") != std::string::npos) {

        enum AVCodecID best = _outParams.videoCodec.find("H.264") != std::string::npos ? AV_CODEC_ID_H264 : AV_CODEC_ID_H265;
        // Note: H.264 High/High10 profiles support up to 4096x2304 (Level 5.2),
        // so we no longer force HEVC for height > 1080. The user's codec choice wins.
#ifdef _WIN32
        // Prefer GPU encoders: NVENC (NVIDIA) → AMF (AMD) → QSV (Intel) → software
        auto findWindowsEncoder = [](AVCodecID id) -> const AVCodec* {
            const AVCodec* c = ::avcodec_find_encoder_by_name((id == AV_CODEC_ID_H265) ? "hevc_nvenc" : "h264_nvenc");
            if (c == nullptr) c = ::avcodec_find_encoder_by_name((id == AV_CODEC_ID_H265) ? "hevc_amf" : "h264_amf");
            if (c == nullptr) c = ::avcodec_find_encoder_by_name((id == AV_CODEC_ID_H265) ? "hevc_qsv" : "h264_qsv");
            if (c == nullptr) c = ::avcodec_find_encoder(id);
            return c;
        };
#endif
#if defined(__APPLE__)
        // Prefer the Apple hardware encoder on macOS. avcodec_find_encoder()
        // returns whichever encoder FFmpeg registered first for the codec ID,
        // which on builds that include libx264 ends up being software — an
        // order of magnitude slower than VideoToolbox.
        const char* hwName = (best == AV_CODEC_ID_H265) ? "hevc_videotoolbox" : "h264_videotoolbox";
        videoCodec = ::avcodec_find_encoder_by_name(hwName);
        if (videoCodec == nullptr) {
            videoCodec = ::avcodec_find_encoder(best);
        }
#elif defined(_WIN32)
        videoCodec = findWindowsEncoder(best);
#else
        videoCodec = ::avcodec_find_encoder(best);
#endif
        if (videoCodec == nullptr) {
            // try flipping back/forth to h264/h265 to see if that can be loaded
            best = (best == AV_CODEC_ID_H265 ? AV_CODEC_ID_H264 : AV_CODEC_ID_H265);
#if defined(__APPLE__)
            const char* hwName2 = (best == AV_CODEC_ID_H265) ? "hevc_videotoolbox" : "h264_videotoolbox";
            videoCodec = ::avcodec_find_encoder_by_name(hwName2);
            if (videoCodec == nullptr) {
                videoCodec = ::avcodec_find_encoder(best);
            }
#elif defined(_WIN32)
            videoCodec = findWindowsEncoder(best);
#else
            videoCodec = ::avcodec_find_encoder(best);
#endif
        }
        if (videoCodec != nullptr) {
            spdlog::info("VideoWriter - selected video encoder: {}",
                         videoCodec->name ? videoCodec->name : "?");
        }
        av_dict_set(&av_opts, "brand", "mp42", 0);
        av_dict_set(&av_opts, "movflags", "faststart+disable_chpl+write_colr", 0);
        status = ::avformat_alloc_output_context2(&_formatContext, nullptr, "mp4", _path.c_str());
    } else if (_outParams.videoCodec.find("MPEG-4") != std::string::npos){//good old MPEG4
        enum AVCodecID mp4codec = AV_CODEC_ID_MPEG4;
        videoCodec = ::avcodec_find_encoder(mp4codec);
        status = ::avformat_alloc_output_context2(&_formatContext, fmt, nullptr, _path.c_str());
    } else  { //auto
        status = ::avformat_alloc_output_context2(&_formatContext, fmt, nullptr, _path.c_str());
    }

    if (videoCodec == nullptr) {
        //not working, stick with original guess (likely mpeg4)
        videoCodec = ::avcodec_find_encoder(fmt->video_codec);
        status = ::avformat_alloc_output_context2(&_formatContext, fmt, nullptr, _path.c_str());
    }

    const AVCodec* audioCodec = nullptr;
    if (!_videoOnly) {
        audioCodec = ::avcodec_find_encoder(fmt->audio_codec);
    }

    if (_formatContext == nullptr)
        throw std::runtime_error("VideoWriter - Error allocating output-context");

    if (!initializeVideo(videoCodec)) {
        // could not open the video encoder, downgrade to the original guess and try again
        ::avformat_free_context(_formatContext);
        _formatContext = nullptr;
        videoCodec = ::avcodec_find_encoder(fmt->video_codec);
        status = ::avformat_alloc_output_context2(&_formatContext, fmt, nullptr, _path.c_str());
        if (_formatContext == nullptr) {
            throw std::runtime_error("VideoWriter - Error allocating output-context");
        }
        if (!initializeVideo(videoCodec)) {
            throw std::runtime_error("VideoWriter - Error opening video codec context");
        }
    }

    if (!_videoOnly) {
        initializeAudio(audioCodec);
    }

    // Initialize frames and packets
    initializeFrames();
    initializePackets();

    // Open file for output and write header
    status = ::avio_open(&_formatContext->pb, _path.c_str(), AVIO_FLAG_WRITE);
    if (status < 0)
        throw std::runtime_error("VideoWriter - Error opening output file");

    // prepare to write... don't trust ::avformat_init_output() telling you that
    // a call to ::avformat_write_header() is unnecessary. If you don't call it,
    // the stream(s) won't be packaged in an MP4 container. Also, the stream's
    // time_base appears to be updated within this call.
    status = ::avformat_write_header(_formatContext, &av_opts);
    av_dict_free(&av_opts);
    if (status < 0)
        throw std::runtime_error("VideoWriter - Error writing file header");

    _ptsIncrement = av_rescale_q(1, _videoCodecContext->time_base, _formatContext->streams[0]->time_base);
}

bool FFmpegVideoWriter::initializeVideo(const AVCodec* codec)
{
    _videoCodecContext = ::avcodec_alloc_context3(codec);
    _videoCodecContext->time_base.num = 1000;
    _videoCodecContext->time_base.den = _outParams.fps * 1000;
    _videoCodecContext->gop_size = 40 /*12*/; // aka keyframe interval
    _videoCodecContext->max_b_frames = 0;
    _videoCodecContext->width = _outParams.width;
    _videoCodecContext->height = _outParams.height;
    _videoCodecContext->pix_fmt = static_cast<AVPixelFormat>(_outPfmt);
    _videoCodecContext->thread_count = 8;

     if (AV_CODEC_ID_MPEG4 != codec->id || _outParams.videoBitrate != 0) {
        // _outParams.videoBitrate may be 0 which would allow the encoder to
        // "choose" or flip to constant quality using the crf parameter
        _videoCodecContext->bit_rate = _outParams.videoBitrate * 1000;
        _videoCodecContext->rc_max_rate = _outParams.videoBitrate * 1000;
    }

    if (codec->id == AV_CODEC_ID_H264 || codec->id == AV_CODEC_ID_H265) {
        _videoCodecContext->color_range     = AVCOL_RANGE_JPEG;
        _videoCodecContext->color_primaries = AVCOL_PRI_BT709;
        _videoCodecContext->color_trc       = AVCOL_TRC_BT709;
        _videoCodecContext->colorspace      = AVCOL_SPC_BT709;
    }

    if (codec->pix_fmts[0] == AV_PIX_FMT_VIDEOTOOLBOX) {
#if defined(__APPLE__)
        // Set up a VideoToolbox hardware frames context so the encoder can
        // consume GPU-backed CVPixelBuffers directly (no CPU sws_scale).
        int hwErr = ::av_hwdevice_ctx_create(&_hwDeviceCtx, AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
                                             nullptr, nullptr, 0);
        if (hwErr >= 0) {
            _hwFramesCtx = ::av_hwframe_ctx_alloc(_hwDeviceCtx);
            if (_hwFramesCtx) {
                auto* frames = reinterpret_cast<AVHWFramesContext*>(_hwFramesCtx->data);
                frames->format    = AV_PIX_FMT_VIDEOTOOLBOX;
                frames->sw_format = AV_PIX_FMT_NV12;   // native CVPixelBuffer format
                frames->width     = _outParams.width;
                frames->height    = _outParams.height;
                // Need at least enough buffers to cover our ring + whatever the
                // encoder retains mid-flight. MAX_EXPORT_BUFFER_FRAMES is 20;
                // double that to give slack for in-flight encoder references.
                frames->initial_pool_size = MAX_EXPORT_BUFFER_FRAMES * 2;
                hwErr = ::av_hwframe_ctx_init(_hwFramesCtx);
                if (hwErr >= 0) {
                    _videoCodecContext->pix_fmt       = AV_PIX_FMT_VIDEOTOOLBOX;
                    _videoCodecContext->hw_frames_ctx = ::av_buffer_ref(_hwFramesCtx);
                } else {
                    spdlog::warn("VideoWriter - av_hwframe_ctx_init failed ({}), falling back to software pix_fmt", hwErr);
                    ::av_buffer_unref(&_hwFramesCtx);
                }
            }
            if (_hwFramesCtx == nullptr) {
                ::av_buffer_unref(&_hwDeviceCtx);
            }
        } else {
            spdlog::warn("VideoWriter - av_hwdevice_ctx_create(VideoToolbox) failed ({})", hwErr);
        }
#endif
        if (AV_CODEC_ID_H265 == codec->id) {
            // HEVC sw encoder seems to have issues if frames aren't sent in real time, require hw encoder
            // or drop to h264 or even to mpeg4
            ::av_opt_set_int(_videoCodecContext->priv_data, "allow_sw", 0, AV_OPT_SEARCH_CHILDREN);
        } else {
            ::av_opt_set_int(_videoCodecContext->priv_data, "allow_sw", 1, AV_OPT_SEARCH_CHILDREN);
        }
        // VideoToolbox performance hints: we're encoding offline and want max throughput.
        // - realtime=0: don't pace to wall-clock; encode as fast as possible.
        // - prio_speed=1: bias the encoder toward speed over quality.
        // - frames_before=0: don't buffer a large lookahead window.
        ::av_opt_set_int(_videoCodecContext->priv_data, "realtime", 0, AV_OPT_SEARCH_CHILDREN);
        ::av_opt_set_int(_videoCodecContext->priv_data, "prio_speed", 1, AV_OPT_SEARCH_CHILDREN);
        ::av_opt_set_int(_videoCodecContext->priv_data, "frames_before", 0, AV_OPT_SEARCH_CHILDREN);
    } else {
#ifdef _WIN32
        std::string codecName = codec->name ? codec->name : "";
        bool isNvenc = (codecName.find("nvenc") != std::string::npos);
        bool isAmf   = (codecName.find("_amf")  != std::string::npos);
        bool isQsv   = (codecName.find("_qsv")  != std::string::npos);
        if (isNvenc) {
            if (_outParams.videoBitrate == 0) {
                ::av_opt_set(_videoCodecContext->priv_data, "rc", "vbr", 0);
                ::av_opt_set(_videoCodecContext->priv_data, "cq", "18", 0);
            }
            if (::av_opt_set(_videoCodecContext->priv_data, "preset", "p4", 0) != 0)
                ::av_opt_set(_videoCodecContext->priv_data, "preset", "medium", 0);
        } else if (isAmf) {
            ::av_opt_set(_videoCodecContext->priv_data, "quality", "quality", 0);
            if (_outParams.videoBitrate == 0) {
                ::av_opt_set(_videoCodecContext->priv_data, "rc", "cqp", 0);
                ::av_opt_set_int(_videoCodecContext->priv_data, "qp_i", 18, 0);
                ::av_opt_set_int(_videoCodecContext->priv_data, "qp_p", 18, 0);
            }
        } else if (isQsv) {
            ::av_opt_set(_videoCodecContext->priv_data, "preset", "medium", 0);
            if (_outParams.videoBitrate == 0)
                ::av_opt_set_int(_videoCodecContext->priv_data, "global_quality", 18, 0);
        } else {
            ::av_opt_set(_videoCodecContext->priv_data, "preset", "fast", 0);
            ::av_opt_set(_videoCodecContext->priv_data, "crf", "18", AV_OPT_SEARCH_CHILDREN);
        }
#else
        ::av_opt_set(_videoCodecContext->priv_data, "preset", "fast", 0);
        ::av_opt_set(_videoCodecContext->priv_data, "crf", "18", AV_OPT_SEARCH_CHILDREN);
#endif
    }
    if (_formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        _videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    int status = ::avcodec_open2(_videoCodecContext, nullptr, nullptr);
    if (status != 0 && codec->pix_fmts[0] == AV_PIX_FMT_VIDEOTOOLBOX) {
        spdlog::warn("VideoWriter - VideoToolbox encoder failed to open, downgrading");
        // could not initialize hardware encoder, drop to ffmpeg mpeg4
        if (strcmp(codec->name, "hevc_videotoolbox") == 0) {
            //first try downgrade from h265 -> h264
            codec = ::avcodec_find_encoder_by_name("h264_videotoolbox");
            if (codec == nullptr) {
                codec = ::avcodec_find_encoder_by_name("mpeg4");
            }
        } else {
            //still couldn't get the hardware encoder, default to software mpeg4 encoder
            codec = ::avcodec_find_encoder_by_name("mpeg4");
        }
        if (codec) {
            ::avcodec_free_context(&_videoCodecContext);
            _videoCodecContext = nullptr;
            // Tear down the hardware frames context we tried to attach above;
            // the fallback encoder may be software (e.g. mpeg4) and expect a
            // software pix_fmt. Leaving _hwFramesCtx populated would cause
            // initializeFrames() to allocate AV_PIX_FMT_VIDEOTOOLBOX frames
            // for a codec that can't consume them.
            if (_hwFramesCtx != nullptr) {
                ::av_buffer_unref(&_hwFramesCtx);
            }
            if (_hwDeviceCtx != nullptr) {
                ::av_buffer_unref(&_hwDeviceCtx);
            }
            return initializeVideo(codec);
        }
    }
#ifdef _WIN32
    if (status != 0) {
        std::string codecName = codec->name ? codec->name : "";
        bool isHw = (codecName.find("nvenc") != std::string::npos ||
                     codecName.find("_amf")  != std::string::npos ||
                     codecName.find("_qsv")  != std::string::npos);
        if (isHw) {
            spdlog::warn("VideoWriter - Windows HW encoder '{}' failed ({}), trying lower-priority encoders",
                         codecName, status);
            ::avcodec_free_context(&_videoCodecContext);
            _videoCodecContext = nullptr;
            bool isH265 = (codec->id == AV_CODEC_ID_H265);
            const char* const candidates[] = {
                isH265 ? "hevc_nvenc" : "h264_nvenc",
                isH265 ? "hevc_amf"   : "h264_amf",
                isH265 ? "hevc_qsv"   : "h264_qsv",
                isH265 ? "libx265"    : "libx264",
            };
            bool foundFailed = false;
            for (const char* name : candidates) {
                if (codecName == name) { foundFailed = true; continue; }
                if (!foundFailed) continue;
                const AVCodec* next = ::avcodec_find_encoder_by_name(name);
                if (next != nullptr) {
                    spdlog::info("VideoWriter - Retrying with encoder '{}'", name);
                    return initializeVideo(next);
                }
            }
            spdlog::error("VideoWriter - No fallback encoder available after '{}' failed", codecName);
            return false;
        }
    }
#endif
    if (status != 0) {
        if (_videoCodecContext) {
            ::avcodec_free_context(&_videoCodecContext);
            _videoCodecContext = nullptr;
        }

        spdlog::info("VideoWriter - Error opening video codec context: {}", status);
        return false;
    }

    AVStream* video_st = ::avformat_new_stream(_formatContext, nullptr);
    video_st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    video_st->time_base.num = 1000;
    video_st->time_base.den = _outParams.fps * 1000;
    video_st->id = _formatContext->nb_streams - 1;
    status = ::avcodec_parameters_from_context(video_st->codecpar, _videoCodecContext);
    if (status != 0)
        throw std::runtime_error("VideoWriter - Error setting video stream parameters");
    _formatContext->video_codec_id = codec->id;
    if (AV_CODEC_ID_H265 == codec->id) {
        video_st->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');
    } else if (AV_CODEC_ID_H264 == codec->id) {
        video_st->codecpar->codec_tag = MKTAG('a', 'v', 'c', '1');
    }
    video_st->disposition |= AV_DISPOSITION_DEFAULT;
    return true;
}

void FFmpegVideoWriter::initializeAudio(const AVCodec* codec)
{
    AVStream* audio_st = ::avformat_new_stream(_formatContext, nullptr);
    audio_st->time_base.num = 1;
    audio_st->time_base.den = _outParams.audioSampleRate;
    audio_st->id = _formatContext->nb_streams - 1;

    _audioCodecContext = ::avcodec_alloc_context3(codec);
#if LIBAVUTIL_VERSION_MAJOR < 57
    _audioCodecContext->channels = 2;
    _audioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
#else
    _audioCodecContext->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
#endif
    _audioCodecContext->sample_rate = _outParams.audioSampleRate;
    _audioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    _audioCodecContext->bit_rate = 128000;

    if (_formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        _audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int status = ::avcodec_open2(_audioCodecContext, nullptr, nullptr);
    if (status != 0)
        throw std::runtime_error("VideoWriter - Error opening audio codec context");

    status = ::avcodec_parameters_from_context(audio_st->codecpar, _audioCodecContext);
    if (status != 0)
        throw std::runtime_error("VideoWriter - Error setting audio stream parameters");
    this->_formatContext->audio_codec_id = codec->id;
}

void FFmpegVideoWriter::initializeFrames()
{
    int status = 0;
    const bool useHwFrames = (_hwFramesCtx != nullptr);
    for (int x = 0; x < MAX_EXPORT_BUFFER_FRAMES; x++) {
        _videoFrames[x] = ::av_frame_alloc();
        _videoFrames[x]->width  = _outParams.width;
        _videoFrames[x]->height = _outParams.height;
        if (useHwFrames) {
            // av_hwframe_get_buffer sets format=AV_PIX_FMT_VIDEOTOOLBOX and
            // attaches a pool-allocated CVPixelBufferRef in data[3].
            status = ::av_hwframe_get_buffer(_hwFramesCtx, _videoFrames[x], 0);
        } else {
            _videoFrames[x]->format = _outPfmt;
            status = ::av_frame_get_buffer(_videoFrames[x], 0);
        }
        if (status != 0) {
            throw std::runtime_error("VideoWriter - Error initializing video frame");
        }
        _videoFrames[x]->pts = 0LL;
        _videoFrames[x]->nb_samples = 0;
    }
    // Software path only: build the RGB24 staging frame + sws colour converter.
    // The hardware path renders straight into each frame's CVPixelBuffer.
    // Note: _swsContext does not do any scaling in the case where we need to pad out
    //       the width/height; may just get an extra black column or row
    if (!useHwFrames) {
        _colorConversionFrame = ::av_frame_alloc();
        _colorConversionFrame->width = _outParams.width;
        _colorConversionFrame->height = _outParams.height;
        _colorConversionFrame->format = _inPfmt;
        status = ::av_frame_get_buffer(_colorConversionFrame, 1);
        if (status != 0) {
            throw std::runtime_error("VideoWriter - Error initializing color-conversion frame");
        }
        int flags = SWS_FAST_BILINEAR; // doesn't matter too much since we're just doing a colorspace conversion
        AVPixelFormat inPfmt = static_cast<AVPixelFormat>(_inPfmt);
        AVPixelFormat outPfmt = static_cast<AVPixelFormat>(_outPfmt);

        _swsContext = ::sws_getContext(_outParams.width, _outParams.height, inPfmt,
                                       _outParams.width, _outParams.height, outPfmt,
                                       flags, nullptr, nullptr, nullptr);
        if (_swsContext == nullptr) {
            throw std::runtime_error("VideoWriter - Error initializing color-converter");
        }
        // Output full-range BT.709 YUV (Y: 0-255) to match the source sRGB pixels.
        // Without this, sws_scale squeezes Y to 16-235 (studio swing) and the
        // exported video looks dark on players that treat it as full range.
        if (::sws_setColorspaceDetails(_swsContext,
                ::sws_getCoefficients(SWS_CS_ITU709), 1,
                ::sws_getCoefficients(SWS_CS_ITU709), 1,
                0, 1 << 16, 1 << 16) < 0) {
            spdlog::warn("VideoWriter - sws_setColorspaceDetails failed; export may have limited-range color");
        }
    }
    if (_audioCodecContext != nullptr) {
        _audioFrame = ::av_frame_alloc();
        _audioFrame->format = AV_SAMPLE_FMT_FLTP;
        _audioFrame->nb_samples = _audioCodecContext->frame_size;
#if LIBAVUTIL_VERSION_MAJOR < 57
        _audioFrame->channels = 2;
        _audioFrame->channel_layout = AV_CH_LAYOUT_STEREO;
#else
        _audioFrame->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
#endif
        _audioFrame->sample_rate = _outParams.audioSampleRate;
        status = ::av_frame_get_buffer(_audioFrame, 0);
        if (status != 0) {
            throw std::runtime_error("VideoWriter - Error initializing audio frame");
        }
        _audioFrame->pts = 0LL;
    }
}

void FFmpegVideoWriter::initializePackets()
{
    _videoPacket = ::av_packet_alloc();

    if (!_videoOnly) {
        _audioPacket = ::av_packet_alloc();
    }
}

void FFmpegVideoWriter::exportFrames(int videoFrameCount)
{
    int progressValueReported = 0;

    // Accumulate the initial packet of compressed video (actually 35 video frames)
    _videoFrames[_curVideoFrame]->nb_samples = 0;
    int endFrameIndex = pushVideoUntilPacketFilled(0);

    // Write the first packet of compressed video
    _videoPacket->stream_index = 0;
    int status = ::av_interleaved_write_frame(_formatContext, _videoPacket);
    if (status < 0)
        throw std::runtime_error("VideoWriter - error writing initial compressed video packet");

    // We'll need to push a bunch of audio packets through in order to "catch up" to the video
    if (!_videoOnly) {
        int64_t numAudioSamplesToPush = endFrameIndex * _outParams.audioSampleRate / _outParams.fps;
        for (int64_t numAudioSamplesPushed = 0; numAudioSamplesPushed < numAudioSamplesToPush;) {
            // For AAC, first packet will be 2048 samples; always 1024 after that
            _audioFrame->nb_samples = 0;
            int64_t ptsBefore = _audioFrame->pts;
            pushAudioUntilPacketFilled();

            int64_t elapsed = _audioFrame->pts - ptsBefore;
            numAudioSamplesPushed += elapsed;
            _audioPacket->stream_index = 1;
            status = ::av_interleaved_write_frame(_formatContext, _audioPacket);
            if (status < 0)
                throw std::runtime_error("VideoWriter - error writing compressed audio packet");
        }
    }

    // ... and beyond the inital write_frame() calls...
    while (endFrameIndex <= videoFrameCount) {
        // Some housekeeping for cancel and progress reporting
        if (_queryForCancel != nullptr && _queryForCancel()) {
            ::avio_closep(&_formatContext->pb);
            return;
        }
        double exportPercentage = double(endFrameIndex) / videoFrameCount;
        int progressAsInt = int(100 * exportPercentage);
        if (progressAsInt != progressValueReported) {
            if (_progressReporter != nullptr)
                _progressReporter(progressAsInt);
            progressValueReported = progressAsInt;
        }

        _videoFrames[_curVideoFrame]->nb_samples = 0;
        endFrameIndex = pushVideoUntilPacketFilled(endFrameIndex);

        // Write the packet of compressed video
        _videoPacket->stream_index = 0;
        int status = ::av_interleaved_write_frame(_formatContext, _videoPacket);
        if (status < 0)
            throw std::runtime_error("VideoWriter - error writing compressed video packet");

        // Process and write some (typically 1 to 3) packets of audio to keep roughly in sync with video
        if (!_videoOnly) {
            int64_t numAudioSamplesToPush = endFrameIndex * _outParams.audioSampleRate / _outParams.fps - _audioFrame->pts;
            int64_t numAudioFramesToPush = numAudioSamplesToPush / _audioCodecContext->frame_size;
            for (int64_t i = 0; i < numAudioFramesToPush; ++i) {
                _audioFrame->nb_samples = 0;
                pushAudioUntilPacketFilled();

                _audioPacket->stream_index = 1;
                status = ::av_interleaved_write_frame(_formatContext, _audioPacket);
                if (status < 0)
                    throw std::runtime_error("VideoWriter - error writing compressed audio packet");
            }
        }
    }

    // Finally, clear out any buffered data
    status = ::avcodec_send_frame(_videoCodecContext, nullptr);
    if (status < 0)
        throw std::runtime_error("VideoWriter - error clearing compressed-video cache");
    while (1) {
        status = ::avcodec_receive_packet(_videoCodecContext, _videoPacket);
        if (status == AVERROR_EOF)
            break;
        if (status == 0) {
            _videoPacket->stream_index = 0;
            status = ::av_interleaved_write_frame(_formatContext, _videoPacket);
            if (status < 0)
                throw std::runtime_error("VideoWriter - error writing cached video frame data");
        }
    }

    if (!_videoOnly) {
        status = ::avcodec_send_frame(_audioCodecContext, nullptr);
        if (status < 0)
            throw std::runtime_error("VideoWriter - error clearing compressed-audio cache");
        while (1) {
            status = ::avcodec_receive_packet(_audioCodecContext, _audioPacket);
            if (status == AVERROR_EOF)
                break;
            if (status == 0) {
                _audioPacket->stream_index = 1;
                status = ::av_interleaved_write_frame(_formatContext, _audioPacket);
                if (status < 0)
                    throw std::runtime_error("VideoWriter - error writing cached audio frame data");
            }
        }
    }

    if (_progressReporter != nullptr)
        _progressReporter(100);
}

void FFmpegVideoWriter::completeExport()
{
    int status = ::av_write_trailer(_formatContext);
    if (status != 0)
        throw std::runtime_error("VideoWriter - Error writing file trailer");

    status = ::avio_closep(&_formatContext->pb);
    if (status != 0)
        throw std::runtime_error("VideoWriter - Error closing output file");
}

void FFmpegVideoWriter::cleanup()
{
    if (_videoPacket != nullptr)
        ::av_packet_free(&_videoPacket);
    if (_audioPacket != nullptr)
        ::av_packet_free(&_audioPacket);

    if (_colorConversionFrame != nullptr)
        ::av_frame_free(&_colorConversionFrame);
    for (int x = 0; x < MAX_EXPORT_BUFFER_FRAMES; x++) {
        if (_videoFrames[x]) {
            ::av_frame_free(&_videoFrames[x]);
        }
        _videoFrames[x] = nullptr;
    }
    if (_audioFrame != nullptr) {
        ::av_frame_free(&_audioFrame);
    }

    if (_formatContext != nullptr) {
        if (_formatContext->pb != nullptr) {
            ::avio_closep(&_formatContext->pb);
        }
        ::avformat_free_context(_formatContext);
        _formatContext = nullptr;
    }

    if (_audioCodecContext != nullptr)
        ::avcodec_free_context(&_audioCodecContext);

    if (_videoCodecContext != nullptr)
        ::avcodec_free_context(&_videoCodecContext);

    if (_swsContext != nullptr) {
        ::sws_freeContext(_swsContext);
        _swsContext = nullptr;
    }

    if (_hwFramesCtx != nullptr) {
        ::av_buffer_unref(&_hwFramesCtx);
    }
    if (_hwDeviceCtx != nullptr) {
        ::av_buffer_unref(&_hwDeviceCtx);
    }
}

int FFmpegVideoWriter::pushVideoUntilPacketFilled(int index)
{
    int status = 0;

    uint8_t* data[] = { nullptr, nullptr, nullptr, nullptr };
    int stride[] = { 0, 0, 0, 0 };
    int frameHeight = 0;
    int frameSize = 0;

    if (_colorConversionFrame) {
        data[0] = _colorConversionFrame->data[0];
        stride[0] = _colorConversionFrame->linesize[0];
        frameHeight = _colorConversionFrame->height;
        frameSize = stride[0] * frameHeight;
    }
    do {
        VideoWriterFrame vf;
        vf.width = _inParams.width;
        vf.height = _inParams.height;

        if (_hwFramesCtx != nullptr) {
            // When using a hw frames pool, the encoder may still hold a reference
            // to the CVPixelBuffer from a previous pass through this ring slot.
            // av_frame_make_writable() detects that and pulls a fresh buffer from
            // the pool; if the encoder has already released it, this is a no-op.
            int mw = ::av_frame_make_writable(_videoFrames[_curVideoFrame]);
            if (mw < 0) {
                throw std::runtime_error("VideoWriter - av_frame_make_writable (hw) failed");
            }
            // The renderer fills the pool CVPixelBuffer (data[3]) directly.
            vf.nativeSurface = _videoFrames[_curVideoFrame]->data[3];
            _getVideo(vf, index++);
        } else {
            vf.rgbBuffer = data[0];
            vf.rgbBufferSize = frameSize;
            if (_getVideo(vf, index++)) {
                int height = ::sws_scale(_swsContext, data, stride, 0, frameHeight, _videoFrames[_curVideoFrame]->data, _videoFrames[_curVideoFrame]->linesize);
                if (height != _videoCodecContext->height) {
                    throw std::runtime_error("VideoWriter - color conversion error");
                }
            }
        }

        status = ::avcodec_send_frame(_videoCodecContext, _videoFrames[_curVideoFrame]);
        if (status < 0) {
            throw std::runtime_error("VideoWriter - error sending video frame to compresssor");
        }

        int nbSamples = _videoFrames[_curVideoFrame]->nb_samples;
        _curVideoFrame++;
        if (_curVideoFrame == MAX_EXPORT_BUFFER_FRAMES) {
            _curVideoFrame = 0;
        }
        _curPts += _ptsIncrement;
        _videoFrames[_curVideoFrame]->pts = _curPts;
        _videoFrames[_curVideoFrame]->nb_samples = nbSamples;

        status = ::avcodec_receive_packet(_videoCodecContext, _videoPacket);
        if (status == AVERROR(EAGAIN)) {
            continue;
        }
        if (status < 0) {
            throw std::runtime_error("VideoWriter - error receiving compressed video");
        }
    } while (status != 0);
    return index;
}

void FFmpegVideoWriter::pushAudioUntilPacketFilled()
{
    int status = 0;

    float* dstLeft = reinterpret_cast<float*>(_audioFrame->buf[0]->data);
    float* dstRight = reinterpret_cast<float*>(_audioFrame->buf[1]->data);

    do {
        // todo - handle when we can't get a full frame of audio
        _getAudio(dstLeft, dstRight, _audioCodecContext->frame_size);
        _audioFrame->nb_samples = _audioCodecContext->frame_size;

        status = ::avcodec_send_frame(_audioCodecContext, _audioFrame);
        if (status < 0)
            throw std::runtime_error("VideoWriter - error sending audio frame to compresssor");
        _audioFrame->pts += _audioCodecContext->frame_size;

        status = ::avcodec_receive_packet(_audioCodecContext, _audioPacket);
        if (status == AVERROR(EAGAIN))
            continue;
        if (status < 0)
            throw std::runtime_error("VideoWriter - error receiving compressed audio");
    } while (status != 0);
}
