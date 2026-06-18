/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FFmpegAudioDecoder.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <spdlog/fmt/fmt.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <log.h>

#include "../utils/string_utils.h"

#define PCMFUDGE 32768
#define CONVERSION_BUFFER_SIZE 192000

// Due to Ubuntu still using FFMpeg 4.x, we have to use some deprecated API's
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// ---------- helpers ----------

// Count total samples by decoding (needed for accurate trackSize with resampling)
static long CountSamples(AVFormatContext* fmtCtx, AVCodecContext* codecCtx, AVStream* audioStream) {
    long total = 0;
    AVFrame* frame = av_frame_alloc();
    if (!frame) return 0;

    AVPacket* pkt = av_packet_alloc();
    av_seek_frame(fmtCtx, 0, 0, AVSEEK_FLAG_ANY);

    while (av_read_frame(fmtCtx, pkt) == 0) {
        if (pkt->stream_index == audioStream->index) {
            int status = avcodec_send_packet(codecCtx, pkt);
            if (status == 0) {
                do {
                    status = avcodec_receive_frame(codecCtx, frame);
                    if (status == AVERROR_EOF) break;
                    if (status == 0) total += frame->nb_samples;
                } while (status != AVERROR(EAGAIN));
            }
        }
        av_packet_unref(pkt);
    }

    // Flush delayed frames — codecs like WMA buffer output internally.
    // Without this, CountSamples underestimates for any codec with AV_CODEC_CAP_DELAY,
    // causing the main-decode buffers to be too small and overflowing the heap.
    avcodec_send_packet(codecCtx, nullptr);
    int flushStatus;
    do {
        flushStatus = avcodec_receive_frame(codecCtx, frame);
        if (flushStatus == 0) total += frame->nb_samples;
    } while (flushStatus == 0);

    av_packet_free(&pkt);
    av_frame_free(&frame);
    return total;
}

static void ExtractTags(AVFormatContext* fmtCtx, DecodedAudioInfo& info) {
    AVDictionaryEntry* tag = av_dict_get(fmtCtx->metadata, "title", nullptr, 0);
    if (tag) info.title = tag->value;
    tag = av_dict_get(fmtCtx->metadata, "album", nullptr, 0);
    if (tag) info.album = tag->value;
    tag = av_dict_get(fmtCtx->metadata, "artist", nullptr, 0);
    if (tag) info.artist = tag->value;

    AVDictionaryEntry* t = nullptr;
    while ((t = av_dict_get(fmtCtx->metadata, "", t, AV_DICT_IGNORE_SUFFIX))) {
        info.metadata[t->key] = t->value;
    }
}

// ---------- DecodeFile ----------

bool FFmpegAudioDecoder::DecodeFile(const std::string& path,
                                     long targetRate,
                                     int extra,
                                     DecodedAudioInfo& info,
                                     uint8_t*& pcmData, long& pcmDataSize,
                                     float*& leftData, float*& rightData,
                                     long& trackSize,
                                     std::function<void(int pct)> progress) {
    pcmData = nullptr;
    pcmDataSize = 0;
    leftData = nullptr;
    rightData = nullptr;
    trackSize = 0;

    // Open file
    AVFormatContext* fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) != 0) {
        spdlog::error("FFmpegAudioDecoder: Error opening {}", path);
        return false;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: Error finding stream info {}", path);
        return false;
    }

    // Find audio stream
    const
        AVCodec* cdc = nullptr;
    int streamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (streamIndex < 0) {
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: No audio stream in {}", path);
        return false;
    }

    AVStream* audioStream = fmtCtx->streams[streamIndex];
    if (!audioStream || !audioStream->codecpar) {
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: Invalid audio stream in {}", path);
        return false;
    }

    AVCodecContext* codecCtx = avcodec_alloc_context3(cdc);
    avcodec_parameters_to_context(codecCtx, audioStream->codecpar);
    if (avcodec_open2(codecCtx, cdc, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: Can't open codec for {}", path);
        return false;
    }

    // Extract info
int channels = codecCtx->ch_layout.nb_channels;
    if (channels <= 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return false;
    }

    info.sampleRate = codecCtx->sample_rate;
    info.channels = channels;
    info.bitRate = codecCtx->bit_rate;
    info.bitsPerSample = av_get_bytes_per_sample(codecCtx->sample_fmt);

    // Count total samples for accurate sizing. CountSamples fully flushes the codec
    // (including delayed frames for WMA etc.), so reset its internal state before the
    // main decode pass below.
    long rawSamples = CountSamples(fmtCtx, codecCtx, audioStream);
    avcodec_flush_buffers(codecCtx);
    double lengthInSeconds = (double)rawSamples / codecCtx->sample_rate;
    info.lengthMS = (long)floor(lengthInSeconds * 1000.0);

    // Calculate resampled track size
    if (targetRate > 0 && targetRate != codecCtx->sample_rate) {
        float f = (float)rawSamples * targetRate / codecCtx->sample_rate;
        trackSize = (long)f;
        extra += 2048; // extra space for resampling estimate inaccuracy
    } else {
        targetRate = codecCtx->sample_rate;
        trackSize = rawSamples;
    }
    info.trackSize = trackSize;

    // Extract tags
    ExtractTags(fmtCtx, info);

    // Allocate output buffers
    long floatBufSize = sizeof(float) * (trackSize + extra);
    leftData = (float*)calloc(floatBufSize, 1);
    if (!leftData) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: Can't allocate left channel buffer");
        return false;
    }

    if (channels >= 2) {
        rightData = (float*)calloc(floatBufSize, 1);
        if (!rightData) {
            free(leftData);
            leftData = nullptr;
            avcodec_free_context(&codecCtx);
            avformat_close_input(&fmtCtx);
            spdlog::error("FFmpegAudioDecoder: Can't allocate right channel buffer");
            return false;
        }
    } else {
        rightData = leftData;
    }

    int outChannels = 2;
    pcmDataSize = trackSize * outChannels * 2;
    pcmData = (uint8_t*)calloc(pcmDataSize + PCMFUDGE, 1);
    if (!pcmData) {
        if (rightData != leftData) free(rightData);
        free(leftData);
        leftData = nullptr;
        rightData = nullptr;
        pcmDataSize = 0;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        spdlog::error("FFmpegAudioDecoder: Can't allocate PCM buffer");
        return false;
    }

    // Track allocation capacities so loadResampled can clamp writes safely.
    // Float buffers hold (trackSize + extra) samples; PCM buffer holds
    // (pcmDataSize + PCMFUDGE) bytes == trackSize + PCMFUDGE/(outChannels*2) samples.
    long allocatedFloatSamples = trackSize + extra;
    long allocatedPcmSamples   = (pcmDataSize + PCMFUDGE) / (outChannels * (long)sizeof(uint16_t));
    long allocatedSamples      = std::min(allocatedFloatSamples, allocatedPcmSamples);

    // Set up resampler: input format -> 16-bit stereo at targetRate
    AVSampleFormat outFmt = AV_SAMPLE_FMT_S16;

AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, outChannels);
    AVChannelLayout inLayout;
    av_channel_layout_default(&inLayout, channels);
    SwrContext* swr = nullptr;
    swr_alloc_set_opts2(&swr, &outLayout, outFmt, targetRate,
                         &inLayout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);
    if (!swr) {
        spdlog::error("FFmpegAudioDecoder: swr_alloc failed");
        free(pcmData); pcmData = nullptr; pcmDataSize = 0;
        if (rightData != leftData) free(rightData);
        free(leftData); leftData = nullptr; rightData = nullptr;
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return false;
    }
    swr_init(swr);

    // Decode
    uint8_t* outBuffer = (uint8_t*)av_malloc(CONVERSION_BUFFER_SIZE * outChannels * 2);
    AVFrame* frame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();
    long read = 0;
    int lastpct = 0;

    auto loadResampled = [&](int sampleCount) {
        if (sampleCount <= 0) return;
        // Hard clamp: never write past the end of either allocated buffer.
        if (read + sampleCount > allocatedSamples) {
            sampleCount = (int)(allocatedSamples - read);
            if (sampleCount <= 0) return;
        }
        if (read + sampleCount > trackSize) {
            extra -= (read + sampleCount - trackSize);
            trackSize = read + sampleCount;
        }
        // Copy PCM
        memcpy(pcmData + (read * outChannels * sizeof(uint16_t)),
               outBuffer, sampleCount * outChannels * sizeof(uint16_t));
        // Convert to float
        for (int i = 0; i < sampleCount; i++) {
            int16_t s = *(int16_t*)(outBuffer + i * sizeof(int16_t) * outChannels);
            leftData[read + i] = (float)s / (float)0x8000;
            if (channels > 1) {
                s = *(int16_t*)(outBuffer + i * sizeof(int16_t) * outChannels + sizeof(int16_t));
                rightData[read + i] = (float)s / (float)0x8000;
            }
        }
        read += sampleCount;
        if (progress && trackSize > 0) {
            int pct = (int)(read * 100 / trackSize);
            if (pct >= lastpct + 10) {
                lastpct = pct / 10 * 10;
                progress(lastpct);
            }
        }
    };

    auto decodeFrame = [&](bool eof) {
        int status = avcodec_send_packet(codecCtx, eof ? nullptr : pkt);
        if (status == 0) {
            do {
                status = avcodec_receive_frame(codecCtx, frame);
                if (status == AVERROR_EOF) break;
                if (status == 0) {
                    int outSamples = swr_convert(swr, &outBuffer, CONVERSION_BUFFER_SIZE,
                                                  (const uint8_t**)frame->data, frame->nb_samples);
                    loadResampled(outSamples);
                }
            } while (status != AVERROR(EAGAIN));
        }
    };

    av_seek_frame(fmtCtx, 0, 0, AVSEEK_FLAG_ANY);

    int status;
    while ((status = av_read_frame(fmtCtx, pkt)) == 0) {
        if (pkt->stream_index == audioStream->index)
            decodeFrame(false);
        av_packet_unref(pkt);
    }

    if (status == AVERROR_EOF)
        decodeFrame(true);

    // Flush resampler
    int drained = swr_convert(swr, &outBuffer, CONVERSION_BUFFER_SIZE, nullptr, 0);
    loadResampled(drained);

    // Finalize trackSize
    trackSize = read;
    info.trackSize = trackSize;

    // Recalculate PCM data size based on actual decoded data
    pcmDataSize = read * outChannels * 2;

    // Cleanup
    swr_free(&swr);
    av_free(outBuffer);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);

    spdlog::debug("FFmpegAudioDecoder: Decoded {} samples from {}", read, path);
    return true;
}

// ---------- EncodeToFile ----------
// This is the existing AudioManager::EncodeAudio logic, moved here.

bool FFmpegAudioDecoder::EncodeToFile(const std::vector<float>& left,
                                       const std::vector<float>& right,
                                       size_t sampleRate,
                                       const std::string& filename) {
    spdlog::debug("FFmpegAudioDecoder: Writing {} samples to {} at rate {}", left.size(), filename, sampleRate);

    if (left.size() != right.size()) {
        spdlog::error("Left and right channel sizes do not match");
        return false;
    }

    // Find encoder
    AVCodecID codecId = EndsWith(filename, "m4a") ? AV_CODEC_ID_AAC : AV_CODEC_ID_MP3;
    const AVCodec* codec = codecId == AV_CODEC_ID_AAC ? avcodec_find_encoder_by_name("aac_at") : nullptr;
    if (!codec) codec = avcodec_find_encoder(codecId);
    if (!codec) codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        spdlog::error("MP3/AAC encoder not found");
        return false;
    }

    // Allocate output context
    AVFormatContext* fmtCtx = nullptr;
    avformat_alloc_output_context2(&fmtCtx, nullptr, nullptr, filename.c_str());
    if (!fmtCtx) {
        spdlog::error("Could not allocate output context");
        return false;
    }

    AVStream* audioStream = avformat_new_stream(fmtCtx, nullptr);
    if (!audioStream) {
        avformat_free_context(fmtCtx);
        spdlog::error("Could not create new stream");
        return false;
    }

    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        avformat_free_context(fmtCtx);
        spdlog::error("Could not allocate codec context");
        return false;
    }

    codecCtx->bit_rate = 320000;
    codecCtx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    codecCtx->sample_rate = (int)sampleRate;
    codecCtx->time_base = {1, (int)sampleRate};

av_channel_layout_default(&codecCtx->ch_layout, 2);
    if (fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_free_context(fmtCtx);
        spdlog::error("Could not open codec for encoding");
        return false;
    }

    avcodec_parameters_from_context(audioStream->codecpar, codecCtx);
    audioStream->time_base = codecCtx->time_base;

    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&fmtCtx->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            avcodec_free_context(&codecCtx);
            avformat_free_context(fmtCtx);
            spdlog::error("Could not open output file {}", filename);
            return false;
        }
    }

    if (avformat_write_header(fmtCtx, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avio_closep(&fmtCtx->pb);
        avformat_free_context(fmtCtx);
        spdlog::error("Error writing file header");
        return false;
    }

    // Set up resampler from S16 interleaved to codec's format
    SwrContext* swr = nullptr;
    AVSampleFormat inFmt = AV_SAMPLE_FMT_S16;

AVChannelLayout chLayout;
    av_channel_layout_default(&chLayout, 2);
    swr_alloc_set_opts2(&swr,
                         &chLayout, codecCtx->sample_fmt, codecCtx->sample_rate,
                         &chLayout, inFmt, (int)sampleRate,
                         0, nullptr);
    if (!swr || swr_init(swr) < 0) {
        spdlog::error("Could not init resampler for encoding");
        if (swr) swr_free(&swr);
        avcodec_free_context(&codecCtx);
        avio_closep(&fmtCtx->pb);
        avformat_free_context(fmtCtx);
        return false;
    }

    // Encode in frame-sized chunks
    int frameSize = codecCtx->frame_size > 0 ? codecCtx->frame_size : 1024;
    AVFrame* frame = av_frame_alloc();
    frame->format = codecCtx->sample_fmt;
    frame->nb_samples = frameSize;
av_channel_layout_copy(&frame->ch_layout, &codecCtx->ch_layout);
    av_frame_get_buffer(frame, 0);

    AVPacket* packet = av_packet_alloc();
    size_t totalSamples = left.size();
    size_t pos = 0;

    // Prepare interleaved 16-bit input
    int bufSize = frameSize * 2; // stereo
    auto* sampleBuf = (int16_t*)av_malloc(bufSize * sizeof(int16_t));

    while (pos < totalSamples) {
        int toProcess = (int)std::min((size_t)frameSize, totalSamples - pos);

        // Interleave float -> S16
        for (int i = 0; i < toProcess; i++) {
            float l = left[pos + i];
            float r = right[pos + i];
            l = std::max(-1.0f, std::min(1.0f, l));
            r = std::max(-1.0f, std::min(1.0f, r));
            sampleBuf[i * 2] = (int16_t)(l * 32767.0f);
            sampleBuf[i * 2 + 1] = (int16_t)(r * 32767.0f);
        }
        // Zero-fill remainder
        for (int i = toProcess; i < frameSize; i++) {
            sampleBuf[i * 2] = 0;
            sampleBuf[i * 2 + 1] = 0;
        }

        const uint8_t* inBuf = (const uint8_t*)sampleBuf;
        av_frame_make_writable(frame);
        frame->nb_samples = swr_convert(swr, frame->data, frameSize, &inBuf, toProcess);
        if (frame->nb_samples <= 0) {
            pos += toProcess;
            continue;
        }
        frame->pts = pos;

        int ret = avcodec_send_frame(codecCtx, frame);
        while (ret >= 0) {
            ret = avcodec_receive_packet(codecCtx, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) {
                spdlog::error("Error receiving encoded packet");
                av_free(sampleBuf);
                swr_free(&swr);
                av_frame_free(&frame);
                av_packet_free(&packet);
                avcodec_free_context(&codecCtx);
                avio_closep(&fmtCtx->pb);
                avformat_free_context(fmtCtx);
                return false;
            }
            av_packet_rescale_ts(packet, codecCtx->time_base, audioStream->time_base);
            packet->stream_index = audioStream->index;
            av_interleaved_write_frame(fmtCtx, packet);
            av_packet_unref(packet);
        }
        pos += toProcess;
    }

    // Flush resampler
    while (true) {
        av_frame_make_writable(frame);
        frame->nb_samples = swr_convert(swr, frame->data, frameSize, nullptr, 0);
        if (frame->nb_samples <= 0) break;
        frame->pts = pos;
        int ret = avcodec_send_frame(codecCtx, frame);
        while (ret >= 0) {
            ret = avcodec_receive_packet(codecCtx, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;
            av_packet_rescale_ts(packet, codecCtx->time_base, audioStream->time_base);
            packet->stream_index = audioStream->index;
            av_interleaved_write_frame(fmtCtx, packet);
            av_packet_unref(packet);
        }
    }

    // Flush encoder
    avcodec_send_frame(codecCtx, nullptr);
    while (true) {
        int ret = avcodec_receive_packet(codecCtx, packet);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;
        if (ret < 0) break;
        av_packet_rescale_ts(packet, codecCtx->time_base, audioStream->time_base);
        packet->stream_index = audioStream->index;
        av_interleaved_write_frame(fmtCtx, packet);
        av_packet_unref(packet);
    }

    av_write_trailer(fmtCtx);

    av_free(sampleBuf);
    swr_free(&swr);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codecCtx);
    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&fmtCtx->pb);
    avformat_free_context(fmtCtx);

    spdlog::debug("FFmpegAudioDecoder: Encoded {} samples to {}", totalSamples, filename);
    return true;
}

// ---------- GetAudioFileLength ----------

size_t FFmpegAudioDecoder::GetAudioFileLength(const std::string& filename) {
    AVFormatContext* fmtCtx = nullptr;

    if (avformat_open_input(&fmtCtx, filename.c_str(), nullptr, nullptr) != 0) {
        return 0;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        return 0;
    }

    size_t len = 0;
    if (fmtCtx->pb) {
        len = avio_size(fmtCtx->pb);
    }

    avformat_close_input(&fmtCtx);
    return len;
}

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
