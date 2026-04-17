/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoTranscoder.h"

#include <filesystem>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <log.h>

namespace {

bool IsUncompressedCodec(AVCodecID id)
{
    switch (id) {
    case AV_CODEC_ID_RAWVIDEO:
    case AV_CODEC_ID_V210:
    case AV_CODEC_ID_V308:
    case AV_CODEC_ID_V408:
    case AV_CODEC_ID_R210:
    case AV_CODEC_ID_AYUV:
    case AV_CODEC_ID_BMP:
        return true;
    default:
        return false;
    }
}

std::string AvErr(int err)
{
    char buf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_make_error_string(buf, sizeof(buf), err);
    return std::string(buf);
}

struct Cleanup {
    AVFormatContext* inCtx = nullptr;
    AVCodecContext* decCtx = nullptr;
    AVFormatContext* outCtx = nullptr;
    AVCodecContext* encCtx = nullptr;
    SwsContext* sws = nullptr;
    AVFrame* decFrame = nullptr;
    AVFrame* encFrame = nullptr;
    AVPacket* inPkt = nullptr;
    AVPacket* outPkt = nullptr;

    ~Cleanup()
    {
        if (sws) sws_freeContext(sws);
        if (decFrame) av_frame_free(&decFrame);
        if (encFrame) av_frame_free(&encFrame);
        if (inPkt) av_packet_free(&inPkt);
        if (outPkt) av_packet_free(&outPkt);
        if (decCtx) avcodec_free_context(&decCtx);
        if (encCtx) avcodec_free_context(&encCtx);
        if (inCtx) avformat_close_input(&inCtx);
        if (outCtx) {
            if (outCtx->pb && !(outCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&outCtx->pb);
            }
            avformat_free_context(outCtx);
        }
    }
};

// Drain any packets the encoder produced, muxing them into the output.
// Returns empty string on success, error on failure. AVERROR_EOF / EAGAIN
// are not errors.
std::string DrainEncoder(AVCodecContext* encCtx, AVFormatContext* outCtx,
                         AVStream* outStream, AVPacket* outPkt)
{
    for (;;) {
        int ret = avcodec_receive_packet(encCtx, outPkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return "";
        }
        if (ret < 0) {
            return "encoder receive_packet failed: " + AvErr(ret);
        }
        av_packet_rescale_ts(outPkt, encCtx->time_base, outStream->time_base);
        outPkt->stream_index = outStream->index;
        ret = av_interleaved_write_frame(outCtx, outPkt);
        av_packet_unref(outPkt);
        if (ret < 0) {
            return "write_frame failed: " + AvErr(ret);
        }
    }
}

} // namespace

std::string VideoTranscoder::SuggestedOutputPath(const std::string& inputPath)
{
    std::filesystem::path p(inputPath);
    p.replace_extension(".mov");
    return p.string();
}

std::string VideoTranscoder::Transcode(const std::string& inputPath,
                                       const std::string& outputPath,
                                       ProgressCallback progress)
{
    if (inputPath.empty() || outputPath.empty()) {
        return "Input or output path is empty";
    }
    if (inputPath == outputPath) {
        return "Input and output paths must differ";
    }

    Cleanup c;
    int ret = 0;

    // --- Open input -------------------------------------------------------
    ret = avformat_open_input(&c.inCtx, inputPath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        return "Could not open input: " + AvErr(ret);
    }
    ret = avformat_find_stream_info(c.inCtx, nullptr);
    if (ret < 0) {
        return "Could not read stream info: " + AvErr(ret);
    }

    int videoStreamIdx = -1;
    for (unsigned i = 0; i < c.inCtx->nb_streams; ++i) {
        if (c.inCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIdx = (int)i;
            break;
        }
    }
    if (videoStreamIdx < 0) {
        return "No video stream found";
    }
    AVStream* inStream = c.inCtx->streams[videoStreamIdx];
    AVCodecID srcCodecId = inStream->codecpar->codec_id;

    const AVCodec* decoder = avcodec_find_decoder(srcCodecId);
    if (!decoder) {
        return "No decoder available for source codec";
    }
    c.decCtx = avcodec_alloc_context3(decoder);
    if (!c.decCtx) return "alloc decoder context failed";
    ret = avcodec_parameters_to_context(c.decCtx, inStream->codecpar);
    if (ret < 0) return "decoder parameters_to_context failed: " + AvErr(ret);
    c.decCtx->pkt_timebase = inStream->time_base;

    ret = avcodec_open2(c.decCtx, decoder, nullptr);
    if (ret < 0) {
        return "Could not open decoder: " + AvErr(ret);
    }

    int width = c.decCtx->width;
    int height = c.decCtx->height;
    AVRational srcFrameRate = av_guess_frame_rate(c.inCtx, inStream, nullptr);
    if (srcFrameRate.num == 0 || srcFrameRate.den == 0) {
        srcFrameRate = AVRational{ 30, 1 }; // fallback for GIFs / unknown
    }

    // --- Decide output codec ---------------------------------------------
    bool useRawvideo = IsUncompressedCodec(srcCodecId);

    AVCodecID outCodecId = useRawvideo ? AV_CODEC_ID_RAWVIDEO : AV_CODEC_ID_HEVC;
    const AVCodec* encoder = nullptr;
    if (!useRawvideo) {
        // Priority order:
        //   1. hevc_videotoolbox — macOS hardware HEVC
        //   2. libx265           — software HEVC (cross-platform)
        //   3. h264_videotoolbox — macOS hardware H.264
        //   4. libx264           — software H.264 (almost always available on Windows)
        //   5. rawvideo          — lossless fallback
        struct Candidate { const char* name; AVCodecID id; };
        static constexpr Candidate kCandidates[] = {
            { "hevc_videotoolbox", AV_CODEC_ID_HEVC  },
            { "libx265",           AV_CODEC_ID_HEVC  },
            { "h264_videotoolbox", AV_CODEC_ID_H264  },
            { "libx264",           AV_CODEC_ID_H264  },
        };
        for (const auto& cand : kCandidates) {
            encoder = avcodec_find_encoder_by_name(cand.name);
            if (encoder) {
                outCodecId = cand.id;
                spdlog::debug("Transcoder: using encoder '{}'", cand.name);
                break;
            }
        }
        if (!encoder) {
            spdlog::warn("Transcoder: no HEVC/H.264 encoder available, falling back to rawvideo");
            useRawvideo = true;
            outCodecId = AV_CODEC_ID_RAWVIDEO;
        }
    }
    if (useRawvideo) {
        encoder = avcodec_find_encoder(AV_CODEC_ID_RAWVIDEO);
        if (!encoder) {
            return "No rawvideo encoder available";
        }
    }

    // --- Open output ------------------------------------------------------
    ret = avformat_alloc_output_context2(&c.outCtx, nullptr, "mov", outputPath.c_str());
    if (ret < 0 || !c.outCtx) {
        return "Could not create output context: " + AvErr(ret);
    }
    AVStream* outStream = avformat_new_stream(c.outCtx, nullptr);
    if (!outStream) return "avformat_new_stream failed";

    c.encCtx = avcodec_alloc_context3(encoder);
    if (!c.encCtx) return "alloc encoder context failed";
    c.encCtx->width = width;
    c.encCtx->height = height;
    c.encCtx->time_base = av_inv_q(srcFrameRate);
    c.encCtx->framerate = srcFrameRate;
    c.encCtx->sample_aspect_ratio = AVRational{ 1, 1 };

    AVPixelFormat encPixFmt;
    if (useRawvideo) {
        encPixFmt = AV_PIX_FMT_RGB24;
        c.encCtx->pix_fmt = encPixFmt;
        c.encCtx->field_order = AV_FIELD_PROGRESSIVE;
        c.encCtx->gop_size = 1;
        c.encCtx->has_b_frames = 0;
        c.encCtx->max_b_frames = 0;
    } else {
        std::string encName(encoder->name);
        bool isVideotoolbox = (encName == "hevc_videotoolbox" || encName == "h264_videotoolbox");
        // videotoolbox wants NV12; software encoders (libx264/libx265) want YUV420P.
        encPixFmt = isVideotoolbox ? AV_PIX_FMT_NV12 : AV_PIX_FMT_YUV420P;
        c.encCtx->pix_fmt = encPixFmt;
        c.encCtx->gop_size = 30;
        c.encCtx->max_b_frames = 2;
        if (isVideotoolbox) {
            av_opt_set_int(c.encCtx, "allow_sw", 1, AV_OPT_SEARCH_CHILDREN);
#if defined(__aarch64__)
            c.encCtx->flags |= AV_CODEC_FLAG_QSCALE;
            c.encCtx->global_quality = FF_QP2LAMBDA * 65; // ~visually lossless
#else
            c.encCtx->bit_rate = (int64_t)width * height * 4 * 8; // generous
#endif
        } else {
            av_opt_set(c.encCtx, "crf", "18", AV_OPT_SEARCH_CHILDREN);
            av_opt_set(c.encCtx, "preset", "medium", AV_OPT_SEARCH_CHILDREN);
        }
    }

    if (c.outCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        c.encCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(c.encCtx, encoder, nullptr);
    if (ret < 0) {
        return "Could not open encoder: " + AvErr(ret);
    }

    ret = avcodec_parameters_from_context(outStream->codecpar, c.encCtx);
    if (ret < 0) {
        return "encoder parameters_from_context failed: " + AvErr(ret);
    }
    outStream->time_base = c.encCtx->time_base;
    outStream->avg_frame_rate = srcFrameRate;
    outStream->r_frame_rate = srcFrameRate;
    if (outCodecId == AV_CODEC_ID_RAWVIDEO) {
        outStream->codecpar->codec_tag = MKTAG('r', 'a', 'w', ' ');
    } else if (outCodecId == AV_CODEC_ID_HEVC) {
        outStream->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');
    } else if (outCodecId == AV_CODEC_ID_H264) {
        outStream->codecpar->codec_tag = MKTAG('a', 'v', 'c', '1');
    }

    ret = avio_open(&c.outCtx->pb, outputPath.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
        return "Could not open output file: " + AvErr(ret);
    }
    ret = avformat_write_header(c.outCtx, nullptr);
    if (ret < 0) {
        return "Could not write header: " + AvErr(ret);
    }

    // --- Setup frames & swscale ------------------------------------------
    c.decFrame = av_frame_alloc();
    c.encFrame = av_frame_alloc();
    c.inPkt = av_packet_alloc();
    c.outPkt = av_packet_alloc();
    if (!c.decFrame || !c.encFrame || !c.inPkt || !c.outPkt) {
        return "frame/packet alloc failed";
    }
    c.encFrame->format = encPixFmt;
    c.encFrame->width = width;
    c.encFrame->height = height;
    ret = av_frame_get_buffer(c.encFrame, 0);
    if (ret < 0) return "av_frame_get_buffer failed: " + AvErr(ret);

    // swscale will be created lazily after the first decoded frame so we
    // know the actual decoder pixel format (some decoders pick a different
    // format than codecpar advertises, e.g. paletted GIF -> rgb8).
    AVPixelFormat swsSrcFmt = AV_PIX_FMT_NONE;

    long long totalFrames = 0;
    if (inStream->nb_frames > 0) {
        totalFrames = inStream->nb_frames;
    } else if (c.inCtx->duration > 0 && srcFrameRate.num > 0) {
        totalFrames = (long long)((double)c.inCtx->duration / AV_TIME_BASE *
                                  av_q2d(srcFrameRate));
    }

    int64_t outPts = 0;
    int frameCount = 0;
    bool cancelled = false;
    bool readDone = false;

    auto processDecoded = [&](AVFrame* src) -> std::string {
        if (swsSrcFmt == AV_PIX_FMT_NONE) {
            swsSrcFmt = (AVPixelFormat)src->format;
            // Fast path: source already matches encoder input layout.
            // Avoid building an sws_ctx so the data path is a plain copy.
            if (swsSrcFmt != encPixFmt) {
                c.sws = sws_getContext(width, height, swsSrcFmt,
                                       width, height, encPixFmt,
                                       SWS_POINT, nullptr, nullptr, nullptr);
                if (!c.sws) return "sws_getContext failed";
            }
        }

        if (c.sws) {
            int r = sws_scale(c.sws, src->data, src->linesize, 0, height,
                              c.encFrame->data, c.encFrame->linesize);
            if (r < 0) return "sws_scale failed: " + AvErr(r);
        } else {
            int r = av_frame_copy(c.encFrame, src);
            if (r < 0) return "av_frame_copy failed: " + AvErr(r);
        }

        c.encFrame->pts = outPts++;
        int r = avcodec_send_frame(c.encCtx, c.encFrame);
        if (r < 0) return "encoder send_frame failed: " + AvErr(r);
        std::string err = DrainEncoder(c.encCtx, c.outCtx, outStream, c.outPkt);
        if (!err.empty()) return err;

        ++frameCount;
        if (progress && !progress(frameCount, (int)totalFrames)) {
            cancelled = true;
        }
        return "";
    };

    // --- Read / decode / encode loop -------------------------------------
    while (!cancelled) {
        ret = av_read_frame(c.inCtx, c.inPkt);
        if (ret == AVERROR_EOF) {
            readDone = true;
        } else if (ret < 0) {
            return "av_read_frame failed: " + AvErr(ret);
        }

        if (!readDone && c.inPkt->stream_index != videoStreamIdx) {
            av_packet_unref(c.inPkt);
            continue;
        }

        AVPacket* sendPkt = readDone ? nullptr : c.inPkt;
        ret = avcodec_send_packet(c.decCtx, sendPkt);
        if (ret < 0 && ret != AVERROR_EOF) {
            av_packet_unref(c.inPkt);
            return "decoder send_packet failed: " + AvErr(ret);
        }
        if (!readDone) av_packet_unref(c.inPkt);

        for (;;) {
            ret = avcodec_receive_frame(c.decCtx, c.decFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) {
                return "decoder receive_frame failed: " + AvErr(ret);
            }
            std::string err = processDecoded(c.decFrame);
            av_frame_unref(c.decFrame);
            if (!err.empty()) return err;
            if (cancelled) break;
        }

        if (readDone) break;
    }

    if (cancelled) {
        avio_closep(&c.outCtx->pb);
        std::error_code ec;
        std::filesystem::remove(outputPath, ec);
        return "Cancelled";
    }

    // Flush encoder
    ret = avcodec_send_frame(c.encCtx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        return "encoder flush send failed: " + AvErr(ret);
    }
    std::string err = DrainEncoder(c.encCtx, c.outCtx, outStream, c.outPkt);
    if (!err.empty()) return err;

    ret = av_write_trailer(c.outCtx);
    if (ret < 0) {
        return "av_write_trailer failed: " + AvErr(ret);
    }

    spdlog::debug("Transcoded {} -> {} ({} frames, {})", inputPath, outputPath,
                  frameCount, encoder->name);
    return "";
}
