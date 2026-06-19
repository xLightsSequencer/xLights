/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "media/VideoExporter.h"

#include <log.h>

#include <wx/progdlg.h>
#include <wx/appprogress.h>

#include <stdexcept>

namespace
{
    VideoWriterParams makeParams(int width, int height, int fps, int audioSampleRate,
                                 const std::string& codec, int videoBitrate)
    {
        VideoWriterParams p;
        p.width = width;
        p.height = height;
        p.fps = fps;
        p.audioSampleRate = audioSampleRate;
        p.videoCodec = codec;
        p.videoBitrate = videoBitrate;
        return p;
    }
}

VideoExporter::VideoExporter(wxWindow* parent,
                             int width, int height, float scaleFactor,
                             unsigned int frameDuration, unsigned int frameCount,
                             int audioChannelCount, int audioSampleRate,
                             const std::string& outPath, const std::string& codec,
                             int videoBitrate) :
    _parent(parent), _frameCount(frameCount)
{
    if (audioChannelCount != 2 && audioChannelCount != 0 && audioChannelCount != 1)
        throw std::runtime_error("VideoExporter - assumes mono or stereo for input and creating stereo for output currently");

    _writer = new VideoWriter(outPath,
                              makeParams(width * scaleFactor, height * scaleFactor,
                                         1000u / frameDuration, audioSampleRate, codec, videoBitrate),
                              audioSampleRate == 0);
}

VideoExporter::~VideoExporter()
{
    delete _writer;
    _writer = nullptr;
}

void VideoExporter::setGetVideoCallback(GetVideoFrameCb fn)
{
    _writer->setGetVideoCallback(fn);
}

void VideoExporter::setGetAudioCallback(GetAudioFrameCb fn)
{
    _writer->setGetAudioCallback(fn);
}

bool VideoExporter::Export(wxAppProgressIndicator* appIndicator)
{
    bool status = true;

    int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT;
    wxProgressDialog dlg(_("Export progress"), _("Exporting video..."), 100, _parent, style);
    appIndicator->SetRange(100);
    appIndicator->SetValue(0);

    auto cancelLambda = [&dlg]() {
        return dlg.WasCancelled();
    };
    _writer->setQueryForCancelCallback(cancelLambda);

    auto progressLambda = [&dlg, &appIndicator](int value) {
        dlg.Update(value);
        appIndicator->SetValue(value);
    };
    _writer->setProgressReportCallback(progressLambda);

    try {
        _writer->initialize();
        auto ip = _writer->inputParams();
        auto op = _writer->outputParams();
        spdlog::info("VideoExporter - exporting {} x {} video from {} x {} via {}",
                     op.width, op.height, ip.width, ip.height,
                     _writer->usingAVFoundation() ? "AVFoundation" : "FFmpeg");

        _writer->exportFrames(_frameCount);
        bool canceled = dlg.WasCancelled();
        if (canceled)
            spdlog::info("VideoExporter - exporting was canceled");

        if (!canceled)
            _writer->completeExport();
    } catch (const std::exception& re) {
        spdlog::error("Exception caught in VideoExporter - '{}'", re.what());
        status = false;
    }
    appIndicator->SetValue(0);
    appIndicator->Reset();
    dlg.Hide();

    return status;
}
