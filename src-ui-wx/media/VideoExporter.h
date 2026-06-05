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

// Thin wxWidgets wrapper around the wx-free, backend-abstracted
// VideoWriter (src-core/media). It owns the wxProgressDialog +
// app-progress-indicator plumbing that can't live in core, and forwards
// the render/audio callbacks straight through to the core writer (which
// picks AVFoundation or FFmpeg under the hood). The bulk of the encoder
// logic now lives in src-core/media/{VideoWriter,FFmpegVideoWriter,
// AVFoundationVideoWriter}.

#include "media/VideoWriter.h"

#include <string>

class wxWindow;
class wxAppProgressIndicator;

class VideoExporter {
public:
    VideoExporter(wxWindow* parent,
                  int width, int height, float scaleFactor,
                  unsigned int frameDuration, unsigned int frameCount,
                  int audioChannelCount, int audioSampleRate,
                  const std::string& outPath, const std::string& codec,
                  int videoBitrate);
    ~VideoExporter();

    void setGetVideoCallback(GetVideoFrameCb fn);
    void setGetAudioCallback(GetAudioFrameCb fn);

    bool Export(wxAppProgressIndicator* appIndicator);

private:
    wxWindow* const _parent;
    unsigned int _frameCount = 0u;
    VideoWriter* _writer = nullptr;
};
