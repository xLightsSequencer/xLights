#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Internal interface for VideoWriter backends.
// Not part of the public API — only included by VideoWriter.cpp and the
// concrete backend implementations (FFmpegVideoWriter,
// AVFoundationVideoWriter).

#include "VideoWriter.h"

class VideoWriterImpl {
public:
    VideoWriterImpl(const std::string& outPath, const VideoWriterParams& params, bool videoOnly)
        : _path(outPath), _videoOnly(videoOnly), _inParams(params), _outParams(params) {}
    virtual ~VideoWriterImpl() = default;

    void setGetVideoCallback(GetVideoFrameCb fn) { _getVideo = fn; }
    void setGetAudioCallback(GetAudioFrameCb fn) { _getAudio = fn; }
    void setQueryForCancelCallback(QueryForCancelCb fn) { _queryForCancel = fn; }
    void setProgressReportCallback(ProgressReportCb fn) { _progressReporter = fn; }

    virtual void initialize() = 0;
    virtual void exportFrames(int videoFrameCount) = 0;
    virtual void completeExport() = 0;

    virtual bool usingAVFoundation() const { return false; }

    const VideoWriterParams& inputParams() const { return _inParams; }
    const VideoWriterParams& outputParams() const { return _outParams; }

protected:
    const std::string _path;
    const bool _videoOnly;
    VideoWriterParams _inParams;
    VideoWriterParams _outParams;

    GetVideoFrameCb _getVideo = nullptr;
    GetAudioFrameCb _getAudio = nullptr;
    QueryForCancelCb _queryForCancel = nullptr;
    ProgressReportCb _progressReporter = nullptr;
};
