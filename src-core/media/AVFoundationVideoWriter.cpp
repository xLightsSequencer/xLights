/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __APPLE__

#include "AVFoundationVideoWriter.h"
#include "media/AVFoundationVideoWriterBridge.h"

#include <log.h>

#include <stdexcept>
#include <vector>

namespace Bridge = AppleAVFoundationVideoWriterBridge;

AVFoundationVideoWriter::AVFoundationVideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly) :
    VideoWriterImpl(outPath, params, videoOnly)
{
    // H.264/H.265 use 4:2:0 chroma and need even dimensions. ProRes 4444 (4:4:4)
    // and the uncompressed BGRA passthrough (rawvideo) encode exact sizes —
    // required so the CPU-frame BGRA copy stays a 1:1 same-size fill.
    const bool exactDims = _outParams.videoCodec.find("ProRes") != std::string::npos ||
                           _outParams.videoCodec.find("prores") != std::string::npos ||
                           _outParams.videoCodec.find("rawvideo") != std::string::npos;
    if (!exactDims) {
        if (_outParams.width % 2)
            ++_outParams.width;
        if (_outParams.height % 2)
            ++_outParams.height;
    }
}

AVFoundationVideoWriter::~AVFoundationVideoWriter()
{
    if (_bridge != nullptr) {
        Bridge::DestroyWriter(_bridge);
        _bridge = nullptr;
    }
}

bool AVFoundationVideoWriter::CanExport(const std::string& outPath, const VideoWriterParams& params)
{
    if (!Bridge::CanExport(outPath, params.videoCodec)) {
        return false;  // .avi / unsupported container -> FFmpeg
    }
    const bool isRaw = params.videoCodec.find("rawvideo") != std::string::npos;
    if (isRaw) {
        return true;  // uncompressed BGRA passthrough: bit-exact, any dims, alpha-preserving
    }
    const bool isProRes = params.videoCodec.find("ProRes") != std::string::npos ||
                          params.videoCodec.find("prores") != std::string::npos;
    if (isProRes) {
        return true;  // ProRes 4444: exact dims, AVFoundation-decodable, alpha-preserving
    }
    // H.264 / H.265
    if (params.lossless) {
        return false;  // true lossless H.264/HEVC isn't achievable via AVAssetWriter
    }
    if (params.cpuFrames) {
        // CPU frames are copied 1:1 into a same-size BGRA buffer (no sws scale),
        // so the encoder's even-dimension rounding must not change the size.
        if (params.width < 16 || params.height < 16) {
            return false;
        }
        if ((params.width % 2) != 0 || (params.height % 2) != 0) {
            return false;
        }
    }
    return true;
}

void AVFoundationVideoWriter::initialize()
{
    const bool hasAudio = !_videoOnly && _outParams.audioSampleRate > 0;
    _bridge = Bridge::CreateWriter(_path, _outParams.videoCodec,
                                   _outParams.width, _outParams.height, _outParams.fps,
                                   _outParams.videoBitrate, _outParams.videoQuality,
                                   hasAudio, _outParams.audioSampleRate,
                                   _outParams.cpuFrames, _outParams.inputChannels);
    if (_bridge == nullptr || !Bridge::IsValid(_bridge)) {
        throw std::runtime_error("VideoWriter - AVFoundation writer initialization failed");
    }
    if (!Bridge::Start(_bridge)) {
        throw std::runtime_error("VideoWriter - AVFoundation writer failed to start");
    }
}

void AVFoundationVideoWriter::exportFrames(int videoFrameCount)
{
    const int fps = _outParams.fps > 0 ? _outParams.fps : 1;
    const int sampleRate = _outParams.audioSampleRate;
    const bool doAudio = !_videoOnly && sampleRate > 0;

    if (doAudio) {
        // Audio is pulled by the bridge on its own queue, never from this
        // thread: the interleaving writer stops accepting video whenever it
        // lacks the matching audio, and this thread can be blocked on video
        // backpressure — supplying audio from here deadlocks the export.
        // The callback only reads AudioManager's pre-decoded PCM, so pulling
        // it from a background queue is safe.
        auto getAudio = _getAudio;
        Bridge::BeginAudio(_bridge,
                           [getAudio](float* left, float* right, int numSamples) {
                               if (getAudio != nullptr) {
                                   getAudio(left, right, numSamples);
                               }
                           },
                           static_cast<long long>(videoFrameCount) * sampleRate / fps);
    }

    int progressReported = 0;

    for (int idx = 0; idx < videoFrameCount; ++idx) {
        if (_queryForCancel != nullptr && _queryForCancel()) {
            (void)Bridge::Finish(_bridge, true);  // cancel: result not meaningful
            return;
        }

        VideoWriterFrame vf;
        vf.width = _outParams.width;
        vf.height = _outParams.height;

        if (_outParams.cpuFrames) {
            // CPU caller fills rgbBuffer; copy it into a pool BGRA buffer.
            const int channels = _outParams.inputChannels;
            _cpuBuf.resize(static_cast<size_t>(_outParams.width) * _outParams.height * channels);
            vf.rgbBuffer = _cpuBuf.data();
            vf.rgbBufferSize = static_cast<int>(_cpuBuf.size());
            _getVideo(vf, static_cast<unsigned>(idx));

            void* pb = Bridge::RequestPixelBuffer(_bridge);
            if (pb == nullptr) {
                throw std::runtime_error("VideoWriter - failed to obtain a pixel buffer");
            }
            if (!Bridge::FillPixelBufferRGB(_bridge, pb, _cpuBuf.data(), channels, _outParams.width, _outParams.height)) {
                throw std::runtime_error("VideoWriter - failed to fill pixel buffer");
            }
            if (!Bridge::AppendVideoFrame(_bridge, pb, idx)) {
                throw std::runtime_error("VideoWriter - failed to append video frame");
            }
        } else {
            void* pb = Bridge::RequestPixelBuffer(_bridge);
            if (pb == nullptr) {
                throw std::runtime_error("VideoWriter - failed to obtain a pixel buffer");
            }
            vf.nativeSurface = pb;
            _getVideo(vf, static_cast<unsigned>(idx));
            if (!Bridge::AppendVideoFrame(_bridge, pb, idx)) {
                throw std::runtime_error("VideoWriter - failed to append video frame");
            }
        }

        int pct = videoFrameCount > 0 ? static_cast<int>(100.0 * idx / videoFrameCount) : 100;
        if (pct != progressReported) {
            if (_progressReporter != nullptr) {
                _progressReporter(pct);
            }
            progressReported = pct;
        }
    }

    if (_progressReporter != nullptr) {
        _progressReporter(100);
    }
}

void AVFoundationVideoWriter::completeExport()
{
    if (_bridge == nullptr) {
        return;
    }
    if (!Bridge::Finish(_bridge, false)) {
        throw std::runtime_error("VideoWriter - AVFoundation writer failed to finish");
    }
}

#endif // __APPLE__
