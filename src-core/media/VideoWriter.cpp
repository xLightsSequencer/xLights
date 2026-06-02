/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoWriter.h"
#include "VideoWriterImpl.h"

#if TARGET_OS_IPHONE
// iPad: AVFoundation only, no FFmpeg
#include "AVFoundationVideoWriter.h"
#else
// macOS / Linux / Windows: FFmpeg always available
#include "FFmpegVideoWriter.h"
#if defined(__APPLE__)
// macOS: also compile the AVFoundation path for runtime selection
#include "AVFoundationVideoWriter.h"
#include "VideoReader.h"   // reuse the global HW-accel toggle as the gate
#endif
#endif

#include <log.h>

namespace {

// Choose the primary backend for this platform / format. Never returns
// null on platforms with FFmpeg; on iPad it always returns the
// AVFoundation backend (the only one available).
VideoWriterImpl* createPrimaryImpl(const std::string& outPath, const VideoWriterParams& params, bool videoOnly)
{
#if TARGET_OS_IPHONE
    if (!AVFoundationVideoWriter::CanExport(outPath, params)) {
        spdlog::warn("VideoWriter - codec '{}' / container for {} is not natively supported; attempting AVFoundation anyway",
                     params.videoCodec, outPath);
    }
    return new AVFoundationVideoWriter(outPath, params, videoOnly);
#elif defined(__APPLE__)
    if (VideoReader::IsHardwareAcceleratedVideo() && AVFoundationVideoWriter::CanExport(outPath, params)) {
        return new AVFoundationVideoWriter(outPath, params, videoOnly);
    }
    if (!AVFoundationVideoWriter::CanExport(outPath, params)) {
        spdlog::warn("VideoWriter - codec '{}' / container for {} is not AVFoundation-encodable; using FFmpeg "
                     "(the FFmpeg encoder path is slated for removal on macOS)",
                     params.videoCodec, outPath);
    }
    return new FFmpegVideoWriter(outPath, params, videoOnly);
#else
    return new FFmpegVideoWriter(outPath, params, videoOnly);
#endif
}

} // namespace

VideoWriter::VideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly) :
    _outPath(outPath), _params(params), _videoOnly(videoOnly)
{
    _impl = createPrimaryImpl(_outPath, _params, _videoOnly);
}

VideoWriter::~VideoWriter()
{
    delete _impl;
    _impl = nullptr;
}

void VideoWriter::applyCallbacks()
{
    if (_impl == nullptr) {
        return;
    }
    _impl->setGetVideoCallback(_getVideo);
    _impl->setGetAudioCallback(_getAudio);
    _impl->setQueryForCancelCallback(_queryForCancel);
    _impl->setProgressReportCallback(_progressReporter);
}

void VideoWriter::setGetVideoCallback(GetVideoFrameCb fn)
{
    _getVideo = fn;
    if (_impl != nullptr) _impl->setGetVideoCallback(fn);
}
void VideoWriter::setGetAudioCallback(GetAudioFrameCb fn)
{
    _getAudio = fn;
    if (_impl != nullptr) _impl->setGetAudioCallback(fn);
}
void VideoWriter::setQueryForCancelCallback(QueryForCancelCb fn)
{
    _queryForCancel = fn;
    if (_impl != nullptr) _impl->setQueryForCancelCallback(fn);
}
void VideoWriter::setProgressReportCallback(ProgressReportCb fn)
{
    _progressReporter = fn;
    if (_impl != nullptr) _impl->setProgressReportCallback(fn);
}

void VideoWriter::initialize()
{
#if defined(__APPLE__) && !TARGET_OS_IPHONE
    // On macOS, if the AVFoundation backend fails to initialize, transparently
    // fall back to FFmpeg (which can encode anything). iPad has no FFmpeg, so
    // there is no fallback there.
    if (_impl != nullptr && _impl->usingAVFoundation()) {
        try {
            _impl->initialize();
            return;
        } catch (const std::exception& e) {
            spdlog::warn("VideoWriter - AVFoundation backend failed to initialize ({}), falling back to FFmpeg", e.what());
            delete _impl;
            _impl = new FFmpegVideoWriter(_outPath, _params, _videoOnly);
            applyCallbacks();
        }
    }
#endif
    _impl->initialize();
}

void VideoWriter::exportFrames(int videoFrameCount)
{
    _impl->exportFrames(videoFrameCount);
}

void VideoWriter::completeExport()
{
    _impl->completeExport();
}

const VideoWriterParams& VideoWriter::inputParams() const
{
    return _impl->inputParams();
}

const VideoWriterParams& VideoWriter::outputParams() const
{
    return _impl->outputParams();
}

bool VideoWriter::usingAVFoundation() const
{
    return _impl != nullptr && _impl->usingAVFoundation();
}
