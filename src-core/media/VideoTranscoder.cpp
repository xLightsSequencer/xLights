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
#include "media/VideoWriter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
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
#ifdef FF_API_AYUV_CODECID
    case AV_CODEC_ID_AYUV:
#endif
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
    // Two source-property bits drive codec selection:
    //
    //   sourceIsUncompressed  — rawvideo / V210 / etc. We must NOT pass these
    //     through as rawvideo in MOV: AVFoundation's QuickTime stack accepts
    //     the container (asset.playable=YES) but AVAssetReader silently
    //     reads zero frames, which on sequence reopen surfaces as
    //     "couldn't read those mov files". ProRes 4444 is the right target
    //     for these — visually-lossless, AVFoundation-decodable.
    //
    //   sourceHasAlpha — qtrle mattes etc.; HEVC/H.264 would silently flatten
    //     transparency. ProRes 4444 (prores_ks) preserves it and is
    //     decodable by AVFoundation on macOS and iPadOS 15.4+.
    bool sourceIsUncompressed = IsUncompressedCodec(srcCodecId);
    bool sourceHasAlpha = false;
    bool sourceWideChroma = false; // genuine 4:4:4 (no chroma subsampling) in the pixel data
    bool sourceHighBitDepth = false;
    if (const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(c.decCtx->pix_fmt)) {
        sourceHasAlpha = (desc->flags & AV_PIX_FMT_FLAG_ALPHA) != 0;
        if (desc->nb_components >= 3) {
            sourceWideChroma = (desc->log2_chroma_w == 0 && desc->log2_chroma_h == 0);
        }
        sourceHighBitDepth = desc->comp[0].depth > 8;
    }

    // --- Choose the VideoWriter target -----------------------------------
    // Lossless-needing sources (uncompressed / alpha / 4:4:4 / >8-bit) target
    // rawvideo: on macOS VideoWriter encodes that as AVFoundation BGRA
    // passthrough — bit-exact at ANY width and alpha-preserving (no more
    // 8-aligned-width constraint or ProRes-for-odd-width fallback), and
    // AVAssetReader reads it back. Everything else transcodes to HEVC.
    // (If AVFoundation is unavailable the FFmpeg rawvideo path takes over.)
    const bool needLossless = sourceHasAlpha || sourceIsUncompressed || sourceWideChroma || sourceHighBitDepth;
    const int inputChannels = sourceHasAlpha ? 4 : 3;

    VideoWriterParams params;
    params.width = width;
    params.height = height;
    params.fps = std::max(1, static_cast<int>(std::llround(av_q2d(srcFrameRate))));
    params.audioSampleRate = 0;            // video only
    params.inputChannels = inputChannels;
    params.cpuFrames = true;
    params.lossless = false;               // lossless-ness is conveyed by the codec choice
    if (needLossless) {
        params.videoCodec = "rawvideo";
    } else {
        params.videoCodec = "H.265";
    }

    long long totalFrames = 0;
    if (inStream->nb_frames > 0) {
        totalFrames = inStream->nb_frames;
    } else if (c.inCtx->duration > 0 && srcFrameRate.num > 0) {
        totalFrames = static_cast<long long>(static_cast<double>(c.inCtx->duration) / AV_TIME_BASE * av_q2d(srcFrameRate));
    }
    if (totalFrames <= 0) {
        totalFrames = 1;  // unknown: the writer holds the last decoded frame rather than truncating
    }

    // Decode plumbing (the encode half is now VideoWriter's job).
    c.decFrame = av_frame_alloc();
    c.inPkt = av_packet_alloc();
    if (!c.decFrame || !c.inPkt) {
        return "frame/packet alloc failed";
    }
    const AVPixelFormat rgbFmt = (inputChannels == 4) ? AV_PIX_FMT_RGBA : AV_PIX_FMT_RGB24;
    AVPixelFormat swsSrcFmt = AV_PIX_FMT_NONE;

    bool decoderFlushed = false;
    bool cancelled = false;
    int framesEmitted = 0;
    std::string runtimeError;

    // Pull the next decoded frame into c.decFrame; false at EOF / on error.
    auto decodeNext = [&]() -> bool {
        for (;;) {
            int r = avcodec_receive_frame(c.decCtx, c.decFrame);
            if (r == 0) {
                return true;
            }
            if (r == AVERROR_EOF) {
                return false;
            }
            if (r != AVERROR(EAGAIN)) {
                runtimeError = "decoder receive_frame failed: " + AvErr(r);
                return false;
            }
            if (decoderFlushed) {
                return false;
            }
            int rr = av_read_frame(c.inCtx, c.inPkt);
            if (rr == AVERROR_EOF) {
                avcodec_send_packet(c.decCtx, nullptr);  // flush
                decoderFlushed = true;
                continue;
            }
            if (rr < 0) {
                runtimeError = "av_read_frame failed: " + AvErr(rr);
                return false;
            }
            if (c.inPkt->stream_index != videoStreamIdx) {
                av_packet_unref(c.inPkt);
                continue;
            }
            int sr = avcodec_send_packet(c.decCtx, c.inPkt);
            av_packet_unref(c.inPkt);
            if (sr < 0 && sr != AVERROR(EAGAIN)) {
                runtimeError = "decoder send_packet failed: " + AvErr(sr);
                return false;
            }
        }
    };

    try {
        VideoWriter writer(outputPath, params, /*videoOnly*/ true);
        writer.setQueryForCancelCallback([&]() { return cancelled; });
        writer.setGetVideoCallback([&](VideoWriterFrame& vf, unsigned) {
            if (!decodeNext()) {
                return false;  // EOF / error: hold the previous frame
            }
            if (swsSrcFmt == AV_PIX_FMT_NONE) {
                swsSrcFmt = static_cast<AVPixelFormat>(c.decFrame->format);
                c.sws = sws_getContext(width, height, swsSrcFmt, width, height, rgbFmt,
                                       SWS_BICUBIC, nullptr, nullptr, nullptr);
            }
            if (c.sws) {
                uint8_t* dstData[4] = { vf.rgbBuffer, nullptr, nullptr, nullptr };
                int dstLinesize[4] = { width * inputChannels, 0, 0, 0 };
                sws_scale(c.sws, c.decFrame->data, c.decFrame->linesize, 0, height, dstData, dstLinesize);
            }
            av_frame_unref(c.decFrame);
            ++framesEmitted;
            if (progress && !progress(framesEmitted, static_cast<int>(totalFrames))) {
                cancelled = true;
            }
            return true;
        });

        writer.initialize();
        writer.exportFrames(static_cast<int>(totalFrames));

        if (cancelled) {
            std::error_code ec;
            std::filesystem::remove(outputPath, ec);
            return "Cancelled";
        }
        writer.completeExport();
    } catch (const std::exception& e) {
        return std::string("Transcode failed: ") + e.what();
    }

    if (!runtimeError.empty()) {
        return runtimeError;
    }

    spdlog::debug("Transcoded {} -> {} ({} frames, {})", inputPath, outputPath, framesEmitted, params.videoCodec);
    return "";
}
