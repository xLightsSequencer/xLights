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
    // AVFoundation, unlike the FFmpeg path, encodes whatever dimensions
    // it's given; keep them even for parity with H.264/H.265 4:2:0.
    if (_outParams.width % 2)
        ++_outParams.width;
    if (_outParams.height % 2)
        ++_outParams.height;
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
    return Bridge::CanExport(outPath, params.videoCodec);
}

void AVFoundationVideoWriter::initialize()
{
    const bool hasAudio = !_videoOnly && _outParams.audioSampleRate > 0;
    _bridge = Bridge::CreateWriter(_path, _outParams.videoCodec,
                                   _outParams.width, _outParams.height, _outParams.fps,
                                   _outParams.videoBitrate, hasAudio, _outParams.audioSampleRate);
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

    long long audioSamplesPushed = 0;
    int progressReported = 0;
    std::vector<float> left;
    std::vector<float> right;

    for (int idx = 0; idx < videoFrameCount; ++idx) {
        if (_queryForCancel != nullptr && _queryForCancel()) {
            (void)Bridge::Finish(_bridge, true);  // cancel: result not meaningful
            return;
        }

        void* pb = Bridge::RequestPixelBuffer(_bridge);
        if (pb == nullptr) {
            throw std::runtime_error("VideoWriter - failed to obtain a pixel buffer");
        }
        VideoWriterFrame vf;
        vf.width = _outParams.width;
        vf.height = _outParams.height;
        vf.nativeSurface = pb;
        _getVideo(vf, static_cast<unsigned>(idx));
        if (!Bridge::AppendVideoFrame(_bridge, pb, idx)) {
            throw std::runtime_error("VideoWriter - failed to append video frame");
        }

        if (doAudio) {
            // Keep audio roughly in step with video: push enough samples so the
            // audio timeline reaches the end of frame idx.
            long long target = static_cast<long long>(idx + 1) * sampleRate / fps;
            int chunk = static_cast<int>(target - audioSamplesPushed);
            if (chunk > 0) {
                left.assign(static_cast<size_t>(chunk), 0.0f);
                right.assign(static_cast<size_t>(chunk), 0.0f);
                _getAudio(left.data(), right.data(), chunk);
                if (!Bridge::AppendAudio(_bridge, left.data(), right.data(), chunk, audioSamplesPushed)) {
                    throw std::runtime_error("VideoWriter - failed to append audio");
                }
                audioSamplesPushed += chunk;
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
