/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// UI event handlers and UI-coupled render orchestration — separated from the
// platform-neutral render engine in Render.cpp.

#include "../../xLightsMain.h"
#include "render/IRenderProgressSink.h"
#include "render/RenderProgressInfo.h"
#include "../sequencer/RenderCommandEvent.h"
#include "../sequencer/MainSequencer.h"
#include "../diagnostics/RenderProgressDialog.h"
#include "../import-export/SeqExportDialog.h"
#include "utils/ExternalHooks.h"
#include "utils/UtilFunctions.h"
#include "models/ModelGroup.h"

#include <log.h>

// ---------------------------------------------------------------------------
// WxRenderProgressSink — desktop implementation of IRenderProgressSink.
// Creates a RenderProgressDialog with per-job wxGauge widgets.
// ---------------------------------------------------------------------------

class WxRenderProgressSink : public IRenderProgressSink {
public:
    explicit WxRenderProgressSink(wxWindow* parent)
        : _dialog(new RenderProgressDialog(parent))
    {}

    ~WxRenderProgressSink() override {
        delete _dialog;
    }

    void SetupJobProgress(IRenderJobStatus* job) override {
        wxStaticText* label = new wxStaticText(_dialog->scrolledWindow, wxID_ANY, job->GetName());
        _dialog->scrolledWindowSizer->Add(label, 1, wxALL | wxEXPAND, 3);
        wxGauge* g = new wxGauge(_dialog->scrolledWindow, wxID_ANY, 100);
        g->SetValue(0);
        g->SetMinSize(wxSize(200, -1));
        _dialog->scrolledWindowSizer->Add(g, 1, wxALL | wxEXPAND, 3);
        job->SetProgressCallback([g](int value, const std::string& tooltip) {
            if (g->GetValue() != value) {
                g->SetValue(value);
                if (!tooltip.empty()) {
                    g->SetToolTip(tooltip);
                }
            }
        });
    }

    void OnRenderSetupComplete() override {
        _dialog->scrolledWindow->SetSizer(_dialog->scrolledWindowSizer);
        _dialog->scrolledWindow->FitInside();
        _dialog->scrolledWindow->SetScrollRate(5, 5);
    }

    void Show() override {
        if (_dialog) _dialog->Show();
    }

    bool IsShown() const override {
        return _dialog && _dialog->IsShown();
    }

private:
    RenderProgressDialog* _dialog;
};

// ---------------------------------------------------------------------------
// AbortRender — uses wxYield, must be in UI layer
// ---------------------------------------------------------------------------

bool xLightsFrame::AbortRender(int maxTimeMS) {
    return AbortRender(maxTimeMS, nullptr);
}

bool xLightsFrame::AbortRender(int maxTimeMS, int* numThreadsAborted) {
    static bool inAbort = false;
    if (_renderEngine->IsRenderDone()) {
        return true;
    }
    if (inAbort) {
        return false;
    }
    inAbort = true;
    spdlog::info("Aborting rendering ...");
    _renderEngine->SignalAbort();

    int maxLoops = maxTimeMS / 10;
    int loops = 0;
    while (!_renderEngine->IsRenderDone() && loops < maxLoops) {
        loops++;
        _renderEngine->RenderMainThreadEffects();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        UpdateRenderStatus();
        if (!_renderEngine->IsRenderDone() && loops > 25) {
            wxYield();
        }
        if (loops % 200 == 0) {
            spdlog::info("    Waiting for renderers to abort. {} left.", (int)_renderEngine->GetRenderProgressInfo().size());
        }
    }
    spdlog::info("    Aborting renderers ... Done");
    inAbort = false;
    if (numThreadsAborted != nullptr) {
        *numThreadsAborted = _renderEngine->GetAbortedRenderJobs();
    }
    return _renderEngine->IsRenderDone();
}

// ---------------------------------------------------------------------------
// RenderContext overrides — delegate to RenderEngine
// ---------------------------------------------------------------------------

void xLightsFrame::RenderMainThreadEffects() {
    _renderEngine->RenderMainThreadEffects();
}

void xLightsFrame::RenderEffectForModel(const std::string &model, int startms, int endms, bool clear) {
    _renderEngine->RenderEffectForModel(model, startms, endms,
                                        _sequenceElements, _seqData,
                                        _suspendRender, modelsChangeCount, clear);
}

// ---------------------------------------------------------------------------
// RenderRange — dispatches RenderCommandEvent to the appropriate engine call
// ---------------------------------------------------------------------------

void xLightsFrame::RenderRange(RenderCommandEvent& evt)
{
    if (evt.deleted) {
        selectedEffect = 0;
    }
    if (evt.model == "") {
        if ((evt.start != -1) && (evt.end != -1)) {
            RenderTimeSlice(evt.start, evt.end, evt.clear);
        } else {
            if (!_suspendRender)
                _renderEngine->RenderDirtyModels(_sequenceElements, _seqData, _suspendRender, modelsChangeCount);
        }
    } else {
        if (!_suspendRender)
            RenderEffectForModel(evt.model, evt.start, evt.end, evt.clear);
    }
}

// ---------------------------------------------------------------------------
// RenderDone — UI refresh after a render group completes
// ---------------------------------------------------------------------------

void xLightsFrame::RenderDone()
{
    mainSequencer->PanelEffectGrid->Refresh();
}

// ---------------------------------------------------------------------------
// Progress dialog / status bar management
// ---------------------------------------------------------------------------

void xLightsFrame::OnProgressBarDoubleClick(wxMouseEvent& /*evt*/)
{
    if (_renderEngine->GetRenderProgressInfo().empty()) {
        return;
    }
    for (auto it : _renderEngine->GetRenderProgressInfo()) {
        if (it->progressSink) {
            it->progressSink->Show();
            return;
        }
    }
}

void xLightsFrame::OnRenderStatusTimerTrigger(wxTimerEvent& /*event*/)
{
    UpdateRenderStatus();
}

void xLightsFrame::UpdateRenderStatus()
{
    if (_renderEngine->GetRenderProgressInfo().empty()) {
        RenderStatusTimer.Stop();
        return;
    }

    RenderMainThreadEffects();

    for (auto it = _renderEngine->GetRenderProgressInfo().begin(); it != _renderEngine->GetRenderProgressInfo().end();) {
        int countModels = 0;
        int countFrames = 0;

        bool done = true;
        RenderProgressInfo* rpi = *it;
        bool shown = rpi->progressSink ? rpi->progressSink->IsShown() : false;

        int frames = rpi->endFrame - rpi->startFrame + 1;
        if (frames <= 0) frames = 1;

        for (size_t row = 0; row < (size_t)rpi->numRows; ++row) {
            if (rpi->jobs[row]) {
                int i = rpi->jobs[row]->GetCurrentFrame();
                if (i > rpi->jobs[row]->GetEndFrame()) {
                    i = END_OF_RENDER_FRAME;
                }
                if (i != END_OF_RENDER_FRAME) {
                    done = false;
                }
                if (rpi->jobs[row]->GetEndFrame() > rpi->endFrame) {
                    frames += rpi->jobs[row]->GetEndFrame() - rpi->endFrame;
                }
                if (rpi->jobs[row]->GetStartFrame() < rpi->startFrame) {
                    frames += rpi->startFrame - rpi->jobs[row]->GetStartFrame();
                }
                ++countModels;
                if (i == END_OF_RENDER_FRAME) {
                    countFrames += rpi->jobs[row]->GetEndFrame() - rpi->jobs[row]->GetStartFrame() + 1;
                    if (shown) {
                        rpi->jobs[row]->UpdateProgress(100);
                    }
                } else {
                    i -= rpi->jobs[row]->GetStartFrame();
                    if (shown) {
                        int val = (i > rpi->endFrame) ? 100 : (100 * i) / frames;
                        rpi->jobs[row]->UpdateProgress(val, rpi->jobs[row]->GetStatusForUser());
                    }
                    countFrames += i;
                }
            }
        }

        if (countFrames > 0 && countModels > 0) {
            int pct = (countFrames * 80) / (countModels * frames);
            static int lastVal = 0;
            if (lastVal != pct) {
                if (ProgressBar->GetValue() != (10 + pct)) {
                    ProgressBar->SetValue(10 + pct);
                    _appProgress->SetValue(10 + pct);
                }
                lastVal = pct;
            }
        }

        if (done) {
            if (IsRenderBell() && !_renderMode && mRendering) {
                wxBell();
            }
            rpi->CleanupJobs();
            _appProgress->SetValue(0);
            _appProgress->Reset();
            RenderDone();
            rpi->callback(_renderEngine->GetAbortedRenderJobs() > 0);
            delete rpi;
            rpi = nullptr;
            it = _renderEngine->GetRenderProgressInfo().erase(it);
        } else {
            ++it;
        }
    }
}

void xLightsFrame::LogRenderStatus()
{
    spdlog::debug("Logging render status ***************");
    spdlog::debug("Render tree size. {} entries.", _renderEngine->GetRenderTree().data.size());
    spdlog::debug("Render Thread status:\n{}", (const char*)GetThreadStatusReport().c_str());
    for (const auto& it : _renderEngine->GetRenderProgressInfo()) {
        int frames = it->endFrame - it->startFrame + 1;
        spdlog::debug("Render progress rows {}, start frame {}, end frame {}, frames {}.", it->numRows, it->startFrame, it->endFrame, frames);
        for (int i = 0; i < it->numRows; i++) {
            if (it->jobs[i] != nullptr) {
                auto job = it->jobs[i];
                int curFrame = job->GetCurrentFrame();
                if (curFrame > it->endFrame || curFrame == END_OF_RENDER_FRAME) {
                    curFrame = it->endFrame;
                }

                spdlog::debug("    Progress {} - {}.", (const char*)job->GetName().c_str(), (long)(curFrame - it->startFrame + 1) * 100 / frames);
                std::string su = job->GetStatusForUser();
                if (!su.empty()) {
                    spdlog::debug("             {}.", (const char*)su.c_str());
                }
                su = job->GetStatus();
                if (!su.empty()) {
                    spdlog::debug("             {}.", (const char*)su.c_str());
                }
            }
        }
    }
    spdlog::debug("*************************************");
}

// ---------------------------------------------------------------------------
// RenderGridToSeqData — full re-render (creates WxRenderProgressSink)
// ---------------------------------------------------------------------------

void xLightsFrame::RenderGridToSeqData(std::function<void(bool)>&& callback)
{
    _renderEngine->BuildRenderTree(_sequenceElements, modelsChangeCount);
    if (_renderEngine->GetRenderTree().data.empty()) {
        callback(false);
        return;
    }

    spdlog::debug("Render tree built. {} entries.", _renderEngine->GetRenderTree().data.size());

    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        callback(false);
        return;
    }
    std::list<Model*> models = _renderEngine->GetRenderTree().GetModels();
    for (auto it : _renderEngine->GetRenderProgressInfo()) {
        for (size_t row = 0; row < (size_t)it->numRows; ++row) {
            if (it->jobs[row]) {
                it->jobs[row]->AbortRender();
            }
        }
    }
    std::list<Model*> restricts;

    spdlog::debug("Rendering {} models {} frames.", models.size(), _seqData.NumFrames());

#ifdef DOTIMING
    auto sw = std::chrono::steady_clock::now();
    _renderEngine->Render(_sequenceElements, _seqData, models, restricts, 0, _seqData.NumFrames() - 1,
           std::make_unique<WxRenderProgressSink>(this), false,
           [this, models, restricts, sw, callback](bool) {
               printf("%s  Render 1:  %lld ms\n", xlightsFilename.c_str(),
                      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw).count());
               auto sw2 = std::chrono::steady_clock::now();
               _renderEngine->Render(_sequenceElements, _seqData, models, restricts, 0, _seqData.NumFrames() - 1,
                      std::make_unique<WxRenderProgressSink>(this), false,
                      [this, models, restricts, sw2, callback](bool) {
                          printf("%s  Render 2:  %lld ms\n", xlightsFilename.c_str(),
                                 std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw2).count());
                          auto sw3 = std::chrono::steady_clock::now();
                          _renderEngine->Render(_sequenceElements, _seqData, models, restricts, 0, _seqData.NumFrames() - 1,
                                 std::make_unique<WxRenderProgressSink>(this), false,
                                 [this, sw3, callback](bool aborted) {
                                     printf("%s  Render 3:  %lld ms\n", xlightsFilename.c_str(),
                                            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw3).count());
                                     callback(aborted);
                                 });
                      });
           });
#else
    _renderEngine->Render(_sequenceElements, _seqData, models, restricts, 0, _seqData.NumFrames() - 1,
           std::make_unique<WxRenderProgressSink>(this), false, std::move(callback));
#endif
}

// ---------------------------------------------------------------------------
// RenderTimeSlice — renders a time range with full UI state management
// ---------------------------------------------------------------------------

void xLightsFrame::RenderTimeSlice(int startms, int endms, bool clear)
{
    _renderEngine->BuildRenderTree(_sequenceElements, modelsChangeCount);
    spdlog::debug("Render tree built for time slice {}ms-{}ms. {} entries.",
                  startms, endms, _renderEngine->GetRenderTree().data.size());

    if (_renderEngine->GetRenderTree().data.empty()) {
        return;
    }
    const int numRows = _sequenceElements.GetElementCount();
    if (numRows == 0) {
        return;
    }
    std::list<Model*> models = _renderEngine->GetRenderTree().GetModels();
    std::list<Model*> restricts;
    if (startms < 0) startms = 0;
    if (endms < 0)   endms   = 0;

    int startframe = startms / _seqData.FrameTime() - 1;
    if (startframe < 0) startframe = 0;
    int endframe = endms / _seqData.FrameTime() + 1;
    if (endframe >= (int)_seqData.NumFrames()) endframe = _seqData.NumFrames() - 1;
    if (endframe < startframe) return;

    EnableSequenceControls(false);
    mRendering = true;
    ProgressBar->Show();
    GaugeSizer->Layout();
    SetStatusText(_("Rendering all layers for time slice"));
    ProgressBar->SetValue(0);
    _appProgress->SetValue(0);
    _appProgress->Reset();

    auto sw = std::chrono::steady_clock::now();
    _renderEngine->Render(_sequenceElements, _seqData, models, restricts, startframe, endframe,
           std::make_unique<WxRenderProgressSink>(this), clear,
           [this, sw](bool /*aborted*/) {
               spdlog::info("   Effects done.");
               ProgressBar->SetValue(100);
               float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::steady_clock::now() - sw)
                                       .count() /
                                   1000.0f;
               std::string displayBuff = fmt::format("Rendered in {:7.3f} seconds", elapsedTime);
               CallAfter(&xLightsFrame::SetStatusText, wxString(displayBuff), 0);
               mRendering = false;
               EnableSequenceControls(true);
               ProgressBar->Hide();
               _appProgress->SetValue(0);
               _appProgress->Reset();
               GaugeSizer->Layout();
           });
}

// ---------------------------------------------------------------------------
// DoExportModel — extract model data via RenderEngine and write to file
// ---------------------------------------------------------------------------

bool xLightsFrame::DoExportModel(unsigned int startFrame, unsigned int endFrame,
                                  const std::string& model, const std::string& fn,
                                  const std::string& fmt, bool doRender)
{
    if (endFrame == 0)
        endFrame = _seqData.NumFrames();

    Model* m = GetModel(model);
    if (m == nullptr)
        return false;

    if (m->GetDisplayAs() == DisplayAsType::ModelGroup)
        return false;

    std::string filename(fn);
    std::string format(fmt);

    auto sw = std::chrono::steady_clock::now();
    std::string Out3 = format.substr(0, 3);

    if (Out3 == "LSP") {
        filename = filename + "_USER";
    }
    std::filesystem::path oName(filename);

    if (oName.parent_path().empty()) {
        oName = std::filesystem::path(CurrentDir.ToStdString()) / oName.filename();
    }
    std::string fullpath;

    SetStatusText(fmt::format("Starting Export for {} - {}", format, Out3));
    wxYield();

    if (doRender) {
        RenderAll();
        while (mRendering) {
            wxYield();
        }
    }

    auto exported = _renderEngine->ExportModelData(model, _seqData);
    if (!exported.data)
        return false;

    SequenceData* data = exported.data.get();
    int cpn = exported.chansPerNode;

    if (Out3 == "Lcb") {
        oName.replace_extension(".lcb");
        fullpath = oName.string();
        int lcbVer = 1;
        if (format.find("S5") != std::string::npos) {
            lcbVer = 2;
        }
        WriteLcbFile(fullpath, data->NumChannels(), startFrame, endFrame, data, lcbVer, cpn);
    } else if (Out3 == "Vir") {
        oName.replace_extension(".vir");
        fullpath = oName.string();
        WriteVirFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "LSP") {
        oName.replace_extension(".xml");
        fullpath = oName.string();
        WriteLSPFile(fullpath, data->NumChannels(), startFrame, endFrame, data, cpn);
    } else if (Out3 == "HLS") {
        oName.replace_extension(".hlsnc");
        fullpath = oName.string();
        WriteHLSFile(fullpath, data->NumChannels(), startFrame, endFrame, data);
    } else if (Out3 == "FPP") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".eseq");
        fullpath = oName.string();
        bool v2 = format.find("Compressed") != std::string::npos;
        WriteFalconPiModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), v2);
    } else if (Out3 == "Com") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".mp4");
        fullpath = oName.string();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), true);
    } else if (Out3 == "Unc") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        fullpath = oName.string();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), false);
    } else if (Out3 == "Los") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".mov");
        fullpath = oName.string();
        WriteVideoModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), false);
    } else if (Out3 == "Min") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".bin");
        fullpath = oName.string();
        WriteMinleonNECModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model));
    } else if (Out3 == "GIF") {
        int stChan = m->GetNumberFromChannelString(m->ModelStartChannel);
        oName.replace_extension(".gif");
        fullpath = oName.string();
        WriteGIFModelFile(fullpath, data->NumChannels(), startFrame, endFrame, data, stChan, data->NumChannels(), GetModel(model), _seqData.FrameTime());
    }
    float s = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw).count();
    s /= 1000;
    SetStatusText(fmt::format("Finished writing model: {} in {:.3f}s ", fullpath, s));

    EnableSequenceControls(true);

    return true;
}

// ---------------------------------------------------------------------------
// ExportModel — UI dialog + export dispatch
// ---------------------------------------------------------------------------

void xLightsFrame::ExportModel(wxCommandEvent& command)
{
    unsigned int startFrame = 0;
    unsigned int endFrame = _seqData.NumFrames();
    std::string cmdStr = command.GetString().ToStdString();
    std::vector<std::string> as;
    Split(cmdStr, '|', as);
    if (as.size() == 3) {
        startFrame = (int)std::strtol(as[1].c_str(), nullptr, 10);
        endFrame   = (int)std::strtol(as[2].c_str(), nullptr, 10);
    }

    std::string model = as[0];
    Model* m = GetModel(model);
    if (m == nullptr)
        return;

    bool isgroup = (m->GetDisplayAs() == DisplayAsType::ModelGroup);

    bool isboxed = false;
    if (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(m) != nullptr) {
        isboxed = true;
    }

    SeqExportDialog dialog(this, m->GetName());
    dialog.ModelExportTypes(isgroup || !isboxed);
    dialog.SetExportType(command.GetString().Contains('|'), command.GetInt() == 1);

    if (dialog.ShowModal() == wxID_OK) {
        std::string filename = dialog.TextCtrlFilename->GetValue().ToStdString();
        ObtainAccessToURL(filename);
        EnableSequenceControls(false);
        std::string format = dialog.ChoiceFormat->GetStringSelection().ToStdString();

        DoExportModel(startFrame, endFrame, model, filename, format, command.GetInt() == 1);
    }
}
