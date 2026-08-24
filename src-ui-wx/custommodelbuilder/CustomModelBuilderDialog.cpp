/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CustomModelBuilderDialog.h"

#ifdef XLIGHTS_HAVE_LIVE_CAMERA

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dcbuffer.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
#include <wx/time.h>
#include <wx/utils.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <queue>
#include <tuple>

#include "xLightsMain.h"
#include "xLightsVersion.h"
#include "settings/XLightsConfigAdapter.h"
#include "shared/utils/wxUtilities.h"
#include "models/Model.h"
#include "models/CustomModel.h"
#include "models/OutputModelManager.h"
#include "outputs/OutputManager.h"

#include <log.h>

namespace {

constexpr int TIMER_MS = 33;              // ~30fps, matches typical webcam frame rate
constexpr long BASELINE_WAIT_MS = 500;    // lights-off settle before baseline sampling
constexpr long BASELINE_SAMPLE_GAP_MS = 150;
constexpr int BASELINE_SAMPLES = 3;
constexpr long MIN_SETTLE_FLOOR_MS = 60;  // never sample sooner than this after lighting a node
constexpr long MAX_SETTLE_MS = 400;       // pre-calibration / worst-case settle
constexpr long NODE_TIMEOUT_EXTRA_MS = 400; // how long past the settle point to keep looking before "missing"
constexpr int CONFIRM_DISTANCE_PX = 15;   // two consecutive detections within this = confirmed
constexpr int MAX_GRID_DIMENSION = 250;
constexpr int MAX_BLOB_PIXELS = 50000;
constexpr double GRID_SIZE_FACTOR = 1.25; // grid dimensions vs. the raw nearest-neighbour spacing

// brightness ladder probed by auto-brightness, lowest first
constexpr int AUTO_BRIGHT_LEVELS[] = { 16, 32, 48, 64, 96, 128, 160, 200, 255 };
constexpr size_t AUTO_BRIGHT_LEVEL_COUNT = sizeof(AUTO_BRIGHT_LEVELS) / sizeof(AUTO_BRIGHT_LEVELS[0]);

const char* SETTING_LAST_MODEL = "CustomModelBuilder_LastModel";
const char* SETTING_LAST_CAMERA = "CustomModelBuilder_LastCamera";
const char* SETTING_BRIGHTNESS = "CustomModelBuilder_Brightness";

} // namespace

CustomModelBuilderDialog::CustomModelBuilderDialog(xLightsFrame* parent, OutputManager* outputManager,
                                                     const std::string& initialCameraSymbolicLink) :
    wxDialog(parent, wxID_ANY, _("Custom Model Builder"), wxDefaultPosition, wxSize(1100, 750),
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
    _frame(parent),
    _outputManager(outputManager),
    _initialCameraSymbolicLink(initialCameraSymbolicLink),
    _timer(this)
{
    _epochMs = wxGetUTCTimeMillis().GetValue();

    auto* top = new wxBoxSizer(wxVERTICAL);

    // top row: camera + model + brightness
    auto* config = new wxFlexGridSizer(0, 4, 5, 8);
    config->AddGrowableCol(1, 1);
    config->AddGrowableCol(3, 1);

    config->Add(new wxStaticText(this, wxID_ANY, _("Camera:")), 0, wxALIGN_CENTER_VERTICAL);
    _labelCamera = new wxStaticText(this, wxID_ANY, wxEmptyString);
    config->Add(_labelCamera, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);

    config->Add(new wxStaticText(this, wxID_ANY, _("Model:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    _choiceModel = new wxChoice(this, wxID_ANY);
    config->Add(_choiceModel, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);

    config->Add(new wxStaticText(this, wxID_ANY, _("Brightness:")), 0, wxALIGN_CENTER_VERTICAL);
    auto* brightRow = new wxBoxSizer(wxHORIZONTAL);
    _sliderBrightness = new wxSlider(this, wxID_ANY, 200, 1, 255);
    _sliderBrightness->SetToolTip(_("Output intensity used when lighting nodes. If detection misses nodes, try raising this; if the camera image blooms/washes out, lower it."));
    brightRow->Add(_sliderBrightness, 1, wxALIGN_CENTER_VERTICAL);
    _labelBrightness = new wxStaticText(this, wxID_ANY, "200");
    brightRow->Add(_labelBrightness, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    _buttonAutoBrightness = new wxButton(this, wxID_ANY, _("Auto"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _buttonAutoBrightness->SetToolTip(_("Lights node 1 at increasing levels and picks the lowest brightness the camera can reliably see, plus a safety margin."));
    brightRow->Add(_buttonAutoBrightness, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    _labelModelInfo = new wxStaticText(this, wxID_ANY, _("Select a model."));
    brightRow->Add(_labelModelInfo, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 12);
    config->Add(brightRow, 1, wxEXPAND);

    top->Add(config, 0, wxEXPAND | wxALL, 8);

    // middle: live preview + node list
    auto* middle = new wxBoxSizer(wxHORIZONTAL);

    _preview = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
    _preview->SetBackgroundStyle(wxBG_STYLE_PAINT);
    _preview->SetMinSize(wxSize(640, 400));
    _preview->SetToolTip(_("Scroll wheel: zoom. Middle/right drag: pan while zoomed. Left click: place the target node. Drag a dot to fix it."));
    middle->Add(_preview, 3, wxEXPAND | wxRIGHT, 6);

    auto* right = new wxBoxSizer(wxVERTICAL);
    _listNodes = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(260, -1),
                                wxLC_REPORT);
    _listNodes->AppendColumn(_("Node"), wxLIST_FORMAT_RIGHT, 55);
    _listNodes->AppendColumn(_("Status"), wxLIST_FORMAT_LEFT, 90);
    _listNodes->AppendColumn(_("X"), wxLIST_FORMAT_RIGHT, 50);
    _listNodes->AppendColumn(_("Y"), wxLIST_FORMAT_RIGHT, 50);
    right->Add(_listNodes, 1, wxEXPAND | wxBOTTOM, 5);

    _buttonRetrySelected = new wxButton(this, wxID_ANY, _("Retry Selected Nodes"));
    _buttonRetrySelected->SetToolTip(_("Re-lights the selected node(s) and re-detects them live. Ctrl/Shift-click the list to select several."));
    right->Add(_buttonRetrySelected, 0, wxEXPAND | wxBOTTOM, 4);
    _buttonRetryMissing = new wxButton(this, wxID_ANY, _("Retry All Missing"));
    right->Add(_buttonRetryMissing, 0, wxEXPAND | wxBOTTOM, 4);
    _buttonGuessMissing = new wxButton(this, wxID_ANY, _("Guess Missing"));
    _buttonGuessMissing->SetToolTip(_("Interpolates a position for each still-missing node from its nearest found neighbours on the string."));
    right->Add(_buttonGuessMissing, 0, wxEXPAND);

    middle->Add(right, 1, wxEXPAND);
    top->Add(middle, 1, wxEXPAND | wxLEFT | wxRIGHT, 8);

    // bottom: scan controls + progress
    auto* bottom = new wxBoxSizer(wxHORIZONTAL);
    _buttonLightAll = new wxToggleButton(this, wxID_ANY, _("Light All (aim camera)"));
    _buttonLightAll->SetToolTip(_("Turns every node on so you can aim and frame the camera before mapping."));
    bottom->Add(_buttonLightAll, 0, wxRIGHT, 8);
    _buttonClear = new wxButton(this, wxID_ANY, _("Clear Mapping"));
    _buttonClear->SetToolTip(_("Throws away every mapped position so you can start over - do this after moving the camera, as old positions no longer line up."));
    bottom->Add(_buttonClear, 0, wxRIGHT, 8);
    _buttonStart = new wxButton(this, wxID_ANY, _("Start Mapping"));
    bottom->Add(_buttonStart, 0, wxRIGHT, 4);
    _buttonPause = new wxToggleButton(this, wxID_ANY, _("Pause"));
    bottom->Add(_buttonPause, 0, wxRIGHT, 4);
    _buttonStopScan = new wxButton(this, wxID_ANY, _("Stop"));
    bottom->Add(_buttonStopScan, 0, wxRIGHT, 12);
    _gauge = new wxGauge(this, wxID_ANY, 100);
    bottom->Add(_gauge, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    _buttonFinish = new wxButton(this, wxID_ANY, _("Finish..."));
    bottom->Add(_buttonFinish, 0);
    top->Add(bottom, 0, wxEXPAND | wxALL, 8);

    _labelStatus = new wxStaticText(this, wxID_ANY, _("Pick a model, aim the camera at the prop, then Start Mapping. Click the video to place a node by hand, drag any dot to fix it, and scroll to zoom in for pixel-level checks."));
    top->Add(_labelStatus, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(top);

    Bind(wxEVT_TIMER, &CustomModelBuilderDialog::OnTimer, this);
    _preview->Bind(wxEVT_PAINT, &CustomModelBuilderDialog::OnPaintPreview, this);
    _preview->Bind(wxEVT_LEFT_DOWN, &CustomModelBuilderDialog::OnPreviewLeftDown, this);
    _preview->Bind(wxEVT_LEFT_UP, &CustomModelBuilderDialog::OnPreviewLeftUp, this);
    _preview->Bind(wxEVT_MOTION, &CustomModelBuilderDialog::OnPreviewMotion, this);
    _preview->Bind(wxEVT_MIDDLE_DOWN, &CustomModelBuilderDialog::OnPreviewPanDown, this);
    _preview->Bind(wxEVT_MIDDLE_UP, &CustomModelBuilderDialog::OnPreviewPanUp, this);
    _preview->Bind(wxEVT_RIGHT_DOWN, &CustomModelBuilderDialog::OnPreviewPanDown, this);
    _preview->Bind(wxEVT_RIGHT_UP, &CustomModelBuilderDialog::OnPreviewPanUp, this);
    _choiceModel->Bind(wxEVT_CHOICE, &CustomModelBuilderDialog::OnChoiceModel, this);
    _sliderBrightness->Bind(wxEVT_SLIDER, &CustomModelBuilderDialog::OnSliderBrightness, this);
    _buttonAutoBrightness->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonAutoBrightness, this);
    _buttonLightAll->Bind(wxEVT_TOGGLEBUTTON, &CustomModelBuilderDialog::OnButtonLightAll, this);
    _buttonStart->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonStart, this);
    _buttonPause->Bind(wxEVT_TOGGLEBUTTON, &CustomModelBuilderDialog::OnButtonPause, this);
    _buttonStopScan->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonStopScan, this);
    _buttonRetryMissing->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonRetryMissing, this);
    _buttonRetrySelected->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonRetrySelected, this);
    _buttonGuessMissing->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonGuessMissing, this);
    _buttonClear->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonClear, this);
    _buttonFinish->Bind(wxEVT_BUTTON, &CustomModelBuilderDialog::OnButtonFinish, this);
    Bind(wxEVT_CLOSE_WINDOW, &CustomModelBuilderDialog::OnClose, this);

    // Windows delivers WM_MOUSEWHEEL to the focused control, which is rarely
    // the preview panel - catch the wheel on every control (and the dialog)
    // and zoom whenever the pointer is over the preview, wherever focus is
    Bind(wxEVT_MOUSEWHEEL, &CustomModelBuilderDialog::OnPreviewWheel, this);
    for (wxWindow* child : GetChildren()) {
        child->Bind(wxEVT_MOUSEWHEEL, &CustomModelBuilderDialog::OnPreviewWheel, this);
    }

    PopulateCameraChoice();
    PopulateModelChoice();
    OpenSelectedCamera();

    long remembered = GetXLightsConfig()->ReadLong(SETTING_BRIGHTNESS, 0);
    if (remembered >= 1 && remembered <= 255) {
        _sliderBrightness->SetValue((int)remembered);
        _labelBrightness->SetLabel(wxString::Format("%ld", remembered));
    }

    ValidateWindow();

    _timer.Start(TIMER_MS);
}

CustomModelBuilderDialog::~CustomModelBuilderDialog()
{
    Cleanup();
    if (_camera != nullptr) {
        delete _camera;
        _camera = nullptr;
    }
}

// idempotent - runs on close-box, on Finish (EndModal skips wxEVT_CLOSE_WINDOW)
// via the destructor, and before that from OnClose
void CustomModelBuilderDialog::Cleanup()
{
    _timer.Stop();
    StopScan(false);
    GetXLightsConfig()->Write(SETTING_BRIGHTNESS, (long)_sliderBrightness->GetValue());
    GetXLightsConfig()->Flush();
    if (_outputsStarted) {
        _outputManager->AllOff();
        if (!_wasOutputting) {
            _frame->DisableOutputs();
        }
        _outputsStarted = false;
    }
    if (_camera != nullptr) {
        _camera->Stop();
    }
}

long CustomModelBuilderDialog::NowMs() const
{
    return (long)(wxGetUTCTimeMillis().GetValue() - _epochMs);
}

#pragma region Setup

void CustomModelBuilderDialog::PopulateCameraChoice()
{
    _cameras = EnumerateLiveCameras();
    if (_cameras.empty()) {
        _selectedCameraIndex = -1;
        _labelCamera->SetLabel(_("(no camera found)"));
        return;
    }

    int sel = -1;
    if (!_initialCameraSymbolicLink.empty()) {
        for (size_t i = 0; i < _cameras.size(); ++i) {
            if (_cameras[i].symbolicLink == _initialCameraSymbolicLink) {
                sel = (int)i;
                break;
            }
        }
    }
    if (sel < 0) {
        wxString last = wxString::FromUTF8(GetXLightsConfig()->Read(SETTING_LAST_CAMERA, std::string()));
        if (!last.IsEmpty()) {
            for (size_t i = 0; i < _cameras.size(); ++i) {
                if (wxString::FromUTF8(_cameras[i].name) == last) {
                    sel = (int)i;
                    break;
                }
            }
        }
    }
    if (sel < 0) sel = 0;

    _selectedCameraIndex = sel;
    _labelCamera->SetLabel(wxString::FromUTF8(_cameras[sel].name));
}

void CustomModelBuilderDialog::PopulateModelChoice()
{
    _choiceModel->Clear();
    std::vector<wxString> names;
    for (const auto& it : _frame->AllModels) {
        if (it.second != nullptr && it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            names.push_back(it.second->GetName());
        }
    }
    std::sort(names.begin(), names.end());
    for (const auto& n : names) {
        _choiceModel->Append(n);
    }
    // no default selection: silently lighting whichever model sorts first is a
    // trap - the scan runs, the camera sees nothing, and nothing looks wrong
    wxString last = wxString::FromUTF8(GetXLightsConfig()->Read(SETTING_LAST_MODEL, std::string()));
    int sel = last.IsEmpty() ? wxNOT_FOUND : _choiceModel->FindString(last);
    if (sel != wxNOT_FOUND) _choiceModel->SetSelection(sel);
    ApplySelectedModel();
}

void CustomModelBuilderDialog::OpenSelectedCamera()
{
    if (_camera != nullptr) {
        delete _camera;
        _camera = nullptr;
    }
    _lastFrame.clear();
    _frameWidth = 0;
    _frameHeight = 0;
    _zoom = 1.0; // a different camera means a different frame size - start fitted
    _panning = false;

    if (_selectedCameraIndex < 0 || _selectedCameraIndex >= (int)_cameras.size()) return;

    GetXLightsConfig()->Write(SETTING_LAST_CAMERA, _cameras[_selectedCameraIndex].name);
    _camera = new LiveCameraCapture(_cameras[_selectedCameraIndex].symbolicLink);
    if (!_camera->IsValid()) {
        delete _camera;
        _camera = nullptr;
        SetStatus("Could not open the camera. Is it in use by another application?");
    }
}

Model* CustomModelBuilderDialog::GetSelectedModel() const
{
    wxString sel = _choiceModel->GetStringSelection();
    if (sel.IsEmpty()) return nullptr;
    return _frame->AllModels[sel.ToStdString()];
}

void CustomModelBuilderDialog::ApplySelectedModel()
{
    Model* m = GetSelectedModel();
    if (m == nullptr) {
        _labelModelInfo->SetLabel(_("Select a model."));
        return;
    }
    GetXLightsConfig()->Write(SETTING_LAST_MODEL, m->GetName());
    _labelModelInfo->SetLabel(wxString::Format(_("Start channel %u, %u nodes, %u channels/node"),
                                               (unsigned)(m->GetFirstChannel() + 1),
                                               (unsigned)m->GetNodeCount(),
                                               (unsigned)m->GetChanCountPerNode()));
    Layout();
}

#pragma endregion Setup

#pragma region Output

void CustomModelBuilderDialog::SetLit(int litNode)
{
    _litNode = litNode;
    SendOutputFrame();
}

void CustomModelBuilderDialog::SendOutputFrame()
{
    if (!_outputsStarted || _outputManager == nullptr || !_outputManager->IsOutputting()) return;
    if (_nodeCount == 0 || _startChannel == 0) return;

    uint8_t intensity = (uint8_t)_sliderBrightness->GetValue();
    _outputManager->StartFrame(NowMs());
    for (uint32_t j = 0; j < _nodeCount; ++j) {
        uint8_t value = (_litNode == -2 || (int)j == _litNode) ? intensity : 0;
        for (uint32_t c = 0; c < _chanPerNode; ++c) {
            _outputManager->SetOneChannel(_startChannel + j * _chanPerNode + c - 1, value);
        }
    }
    _outputManager->EndFrame();
}

#pragma endregion Output

#pragma region Scanning

// captures the model's output parameters and brings the output system up under
// this dialog's control. Returns false (with the error already shown) when
// there is no usable model/camera or output could not start.
bool CustomModelBuilderDialog::PrepareOutput(Model* m)
{
    if (m == nullptr) {
        DisplayError("Select a model to map first.", this);
        return false;
    }
    if (_camera == nullptr || !_camera->IsValid() || _lastFrame.empty()) {
        DisplayError("No live camera image. Select a working camera first.", this);
        return false;
    }

    _model = m;
    _startChannel = (uint32_t)(m->GetFirstChannel() + 1);
    _nodeCount = (uint32_t)m->GetNodeCount();
    _chanPerNode = std::max(1, (int)m->GetChanCountPerNode());

    if (!_outputsStarted) {
        _wasOutputting = _outputManager->IsOutputting();
        // The frame's OutputTimer must not be running while this dialog owns the
        // output: a tick would overwrite the whole channel buffer with sequence
        // data when the Sequencer tab is current, stomping our node frames (prop
        // stays dark). The menu handler restores the timer once the dialog closes.
        if (!_wasOutputting) {
            _frame->ForceEnableOutputs(false);
        } else {
            _frame->StopOutputTimer();
        }
        _outputsStarted = true;
    }
    if (!_outputManager->IsOutputting()) {
        _outputsStarted = false;
        DisplayError("Could not start light output - nothing will light.\n\n"
                     "Check the controller is powered/reachable and that no other program "
                     "(or a second copy of xLights) is outputting to it.", this);
        return false;
    }

    // while we are driving the lights, put the camera in dark mode (short manual
    // exposure, minimum gain) so the ambient scene goes near-black and lit
    // pixels have maximum contrast; restored whenever the light work ends
    SetCameraDarkMode(true);
    return true;
}

void CustomModelBuilderDialog::SetCameraDarkMode(bool enabled)
{
    if (_camera != nullptr && _camera->IsValid()) {
        _camera->SetDarkMode(enabled);
    }
}

void CustomModelBuilderDialog::StartAutoBrightness()
{
    if (!PrepareOutput(GetSelectedModel())) return;

    _savedBrightness = _sliderBrightness->GetValue();
    _autoBrightRun = true;
    _buttonLightAll->SetValue(false);
    _paused = false;
    _buttonPause->SetValue(false);
    _gauge->SetRange((int)AUTO_BRIGHT_LEVEL_COUNT);
    _gauge->SetValue(0);
    _baselineSamples.clear();
    _state = ScanState::Baseline;
    _stateStartMs = NowMs();
    SetLit(-1);
    SetStatus("Auto brightness: measuring baseline (all lights off)...");
    ValidateWindow();
}

// chosenLevel < 0 means no ladder level was visible
void CustomModelBuilderDialog::FinishAutoBrightness(int chosenLevel)
{
    _autoBrightRun = false;
    _state = ScanState::Idle;
    SetLit(-1);
    SetCameraDarkMode(false);
    if (chosenLevel < 0) {
        _sliderBrightness->SetValue(_savedBrightness);
        _labelBrightness->SetLabel(wxString::Format("%d", _savedBrightness));
        SetStatus("Auto brightness: the test node was not visible even at 255. Check the camera can see the prop (use Light All) and that node 1 works.");
    } else {
        // headroom above the barely-visible level, same margin the earlier
        // live-capture implementation validated in the field
        int final_ = std::min(255, (int)std::lround(chosenLevel * 1.3) + 5);
        _sliderBrightness->SetValue(final_);
        _labelBrightness->SetLabel(wxString::Format("%d", final_));
        spdlog::info("CustomModelBuilder: auto brightness picked {} (lowest visible level {}).", final_, chosenLevel);
        SetStatus(wxString::Format(_("Auto brightness: node visible from level %d - brightness set to %d."), chosenLevel, final_).ToStdString());
    }
    _gauge->SetValue(0);
    ValidateWindow();
}

void CustomModelBuilderDialog::StartScan(std::vector<uint32_t> queue)
{
    if (queue.empty()) return;
    if (!PrepareOutput(GetSelectedModel())) return;

    if (_nodes.size() != _nodeCount) {
        _nodes.assign(_nodeCount, NodeInfo());
        RefreshNodeList();
    }

    spdlog::info("CustomModelBuilder: scan starting - {} nodes, start channel {}, {} ch/node, brightness {}.",
                 queue.size(), _startChannel, _chanPerNode, _sliderBrightness->GetValue());

    _autoBrightRun = false;
    _buttonLightAll->SetValue(false);
    _queue = std::move(queue);
    _queuePos = 0;
    _paused = false;
    _buttonPause->SetValue(false);
    _gauge->SetRange((int)_queue.size());
    _gauge->SetValue(0);
    _baselineSamples.clear();
    // re-measure the camera latency each scan - brightness/exposure may have changed
    _latencyCalibrated = false;
    _minSettleMs = MIN_SETTLE_FLOOR_MS;
    _pendingCount = 0;
    _state = ScanState::Baseline;
    _stateStartMs = NowMs();
    SetLit(-1);
    SetStatus(wxString::Format(_("Mapping '%s' - measuring baseline (all lights off)..."),
                               wxString(_model->GetName())).ToStdString());
    ValidateWindow();
}

void CustomModelBuilderDialog::StopScan(bool completed)
{
    if (_state == ScanState::Idle || _state == ScanState::Done) return;
    if (_autoBrightRun || _state == ScanState::AutoBright) {
        _autoBrightRun = false;
        _state = ScanState::Idle;
        SetLit(-1);
        SetCameraDarkMode(false);
        _sliderBrightness->SetValue(_savedBrightness);
        _labelBrightness->SetLabel(wxString::Format("%d", _savedBrightness));
        SetStatus("Auto brightness cancelled.");
        ValidateWindow();
        return;
    }
    _state = ScanState::Done;
    _paused = false;
    _buttonPause->SetValue(false);
    SetLit(-1);
    // back to normal exposure so the user can see the prop while hand-placing
    // or dragging nodes on the frozen result
    SetCameraDarkMode(false);

    int found = 0, missing = 0;
    for (const auto& n : _nodes) {
        if (n.status == NodeInfo::Status::Missing) ++missing;
        else if (n.status != NodeInfo::Status::NotScanned) ++found;
    }
    if (completed) {
        SetStatus(wxString::Format(_("Mapping complete: %d of %u nodes located, %d missing. Retry or click the video to place missing nodes, drag dots to fix, then Finish."),
                                   found, (unsigned)_nodes.size(), missing).ToStdString());
    } else {
        SetStatus(wxString::Format(_("Mapping stopped: %d nodes located so far."), found).ToStdString());
    }
    ValidateWindow();
}

void CustomModelBuilderDialog::ComputeBaseline()
{
    const size_t sz = _baselineSamples.empty() ? 0 : _baselineSamples[0].size();
    _baseline.assign(sz, 0);
    if (sz == 0) return;

    // per-pixel average of the samples
    std::vector<uint32_t> acc(sz, 0);
    for (const auto& s : _baselineSamples) {
        if (s.size() != sz) continue;
        for (size_t i = 0; i < sz; ++i) acc[i] += s[i];
    }
    for (size_t i = 0; i < sz; ++i) _baseline[i] = (uint8_t)(acc[i] / _baselineSamples.size());

    // noise floor: the 99.9th percentile per-pixel deviation of the samples from
    // the average, so the detection threshold self-tunes to sensor noise and
    // flickery ambient light instead of using a fixed magic number.
    uint32_t hist[256] = { 0 };
    uint64_t total = 0;
    for (const auto& s : _baselineSamples) {
        if (s.size() != sz) continue;
        for (size_t i = 0; i < sz; ++i) {
            int d = std::abs((int)s[i] - (int)_baseline[i]);
            ++hist[d];
            ++total;
        }
    }
    int noise = 0;
    uint64_t below = 0;
    for (int v = 0; v < 256; ++v) {
        below += hist[v];
        if (below >= total - total / 1000) {
            noise = v;
            break;
        }
    }
    _noiseFloor = noise;
    _detectThreshold = std::clamp(noise * 3 + 10, 40, 200);
    spdlog::debug("CustomModelBuilder: baseline noise {} -> detection threshold {}.", noise, _detectThreshold);
}

bool CustomModelBuilderDialog::DetectBlob(wxPoint& outPos, int& outPeak) const
{
    outPeak = 0;
    if (_lastFrame.size() != _baseline.size() || _baseline.empty()) return false;

    const int w = _frameWidth;
    const int h = _frameHeight;
    const size_t pixels = (size_t)w * h;
    if (pixels * 3 != _lastFrame.size()) return false;

    // brightness delta vs baseline, max across R/G/B so any lit colour counts
    _diffScratch.resize(pixels);
    std::vector<uint8_t>& diff = _diffScratch;
    int peak = 0;
    size_t peakIdx = 0;
    for (size_t i = 0; i < pixels; ++i) {
        int d0 = (int)_lastFrame[i * 3 + 0] - (int)_baseline[i * 3 + 0];
        int d1 = (int)_lastFrame[i * 3 + 1] - (int)_baseline[i * 3 + 1];
        int d2 = (int)_lastFrame[i * 3 + 2] - (int)_baseline[i * 3 + 2];
        int d = std::max({ d0, d1, d2, 0 });
        diff[i] = (uint8_t)std::min(d, 255);
        if (d > peak) {
            peak = d;
            peakIdx = i;
        }
    }
    outPeak = peak;
    if (peak < _detectThreshold) return false;

    // flood fill from the peak over pixels at least half as bright, and take
    // the brightness-weighted centroid of that blob - a stray dim reflection
    // elsewhere in frame can't drag the position because it isn't connected.
    const uint8_t blobThreshold = (uint8_t)std::max(_detectThreshold, peak / 2);
    _visitedScratch.assign(pixels, 0);
    std::vector<uint8_t>& visited = _visitedScratch;
    std::queue<size_t> open;
    open.push(peakIdx);
    visited[peakIdx] = 1;
    double sumX = 0, sumY = 0, sumW = 0;
    int count = 0;
    while (!open.empty() && count < MAX_BLOB_PIXELS) {
        size_t i = open.front();
        open.pop();
        ++count;
        int x = (int)(i % w);
        int y = (int)(i / w);
        double weight = diff[i];
        sumX += x * weight;
        sumY += y * weight;
        sumW += weight;
        const int dx[4] = { 1, -1, 0, 0 };
        const int dy[4] = { 0, 0, 1, -1 };
        for (int k = 0; k < 4; ++k) {
            int nx = x + dx[k];
            int ny = y + dy[k];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
            size_t ni = (size_t)ny * w + nx;
            if (visited[ni] || diff[ni] < blobThreshold) continue;
            visited[ni] = 1;
            open.push(ni);
        }
    }
    if (sumW <= 0) return false;
    outPos = wxPoint((int)std::lround(sumX / sumW), (int)std::lround(sumY / sumW));
    return true;
}

void CustomModelBuilderDialog::RecordResult(uint32_t node, bool found, const wxPoint& pos, int peak)
{
    if (node >= _nodes.size()) return;
    auto& n = _nodes[node];
    n.peak = peak;
    if (found) {
        n.status = NodeInfo::Status::Found;
        n.pos = pos;
    } else {
        n.status = NodeInfo::Status::Missing;
        n.pos = wxPoint(-1, -1);
    }
    UpdateNodeRow(node);
    _listNodes->EnsureVisible((long)node);
}

void CustomModelBuilderDialog::AdvanceScan(long nowMs)
{
    if (_paused) {
        _stateStartMs = nowMs; // don't count paused time as settle time
        return;
    }

    if (_state == ScanState::Baseline) {
        if (nowMs - _stateStartMs < BASELINE_WAIT_MS) return;
        long sinceWait = nowMs - _stateStartMs - BASELINE_WAIT_MS;
        if ((long)_baselineSamples.size() * BASELINE_SAMPLE_GAP_MS <= sinceWait && !_lastFrame.empty()) {
            _baselineSamples.push_back(_lastFrame);
        }
        if ((int)_baselineSamples.size() >= BASELINE_SAMPLES) {
            ComputeBaseline();
            if (_autoBrightRun) {
                _autoBrightIdx = 0;
                _state = ScanState::AutoBright;
                _stateStartMs = nowMs;
                _sliderBrightness->SetValue(AUTO_BRIGHT_LEVELS[0]);
                _labelBrightness->SetLabel(wxString::Format("%d", AUTO_BRIGHT_LEVELS[0]));
                SetLit(0);
                SetStatus(wxString::Format(_("Auto brightness: testing level %d..."), AUTO_BRIGHT_LEVELS[0]).ToStdString());
            } else {
                _queuePos = 0;
                _state = ScanState::Settle;
                _stateStartMs = nowMs;
                SetLit((int)_queue[_queuePos]);
                SetStatus(wxString::Format(_("Mapping node %u of %u..."), (unsigned)_queue[_queuePos] + 1, (unsigned)_nodes.size()).ToStdString());
            }
        }
        return;
    }

    if (_state == ScanState::AutoBright) {
        if (nowMs - _stateStartMs < MAX_SETTLE_MS) return;

        wxPoint pos;
        int peak = 0;
        bool found = DetectBlob(pos, peak);
        // same acceptance rule the stash version settled on: the on/off delta
        // must clear 4x the measured off/off noise floor to count as visible
        int required = std::max(_noiseFloor * 4, 40);
        int level = AUTO_BRIGHT_LEVELS[_autoBrightIdx];
        spdlog::debug("CustomModelBuilder: auto brightness level {} -> peak {} (needs {}).", level, peak, required);
        if (found && peak >= required) {
            FinishAutoBrightness(level);
            return;
        }
        ++_autoBrightIdx;
        if (_autoBrightIdx >= AUTO_BRIGHT_LEVEL_COUNT) {
            FinishAutoBrightness(-1);
            return;
        }
        level = AUTO_BRIGHT_LEVELS[_autoBrightIdx];
        _sliderBrightness->SetValue(level);
        _labelBrightness->SetLabel(wxString::Format("%d", level));
        SetLit(0); // resend at the new intensity
        _stateStartMs = nowMs;
        _gauge->SetValue((int)_autoBrightIdx);
        SetStatus(wxString::Format(_("Auto brightness: testing level %d..."), level).ToStdString());
    }

    if (_state == ScanState::Settle) {
        long elapsed = nowMs - _stateStartMs;
        if (elapsed < _minSettleMs) return;

        uint32_t node = _queue[_queuePos];
        wxPoint pos;
        int peak = 0;
        bool found = DetectBlob(pos, peak);

        if (found) {
            bool confirmed = _pendingCount > 0 &&
                             std::abs(pos.x - _pendingPos.x) <= CONFIRM_DISTANCE_PX &&
                             std::abs(pos.y - _pendingPos.y) <= CONFIRM_DISTANCE_PX;
            if (!confirmed) {
                // first sighting - remember it and confirm on the next frame
                _pendingPos = pos;
                _pendingCount = 1;
                return;
            }
            // measure the real light->camera latency off the first confirmed node
            // and pace the rest of the scan to it instead of the worst case
            if (!_latencyCalibrated) {
                long latency = std::max(0L, elapsed - TIMER_MS);
                _minSettleMs = std::clamp(latency + TIMER_MS, MIN_SETTLE_FLOOR_MS, MAX_SETTLE_MS);
                _latencyCalibrated = true;
                spdlog::debug("CustomModelBuilder: measured camera latency ~{}ms -> per-node settle {}ms.", latency, _minSettleMs);
            }
            NextQueueNode(nowMs, node, true, pos, peak);
            return;
        }

        _pendingCount = 0;
        if (elapsed < _minSettleMs + NODE_TIMEOUT_EXTRA_MS) return; // keep looking
        NextQueueNode(nowMs, node, false, pos, peak);
    }
}

void CustomModelBuilderDialog::NextQueueNode(long nowMs, uint32_t node, bool found, const wxPoint& pos, int peak)
{
    RecordResult(node, found, pos, peak);
    _gauge->SetValue((int)_queuePos + 1);
    _pendingCount = 0;

    ++_queuePos;
    if (_queuePos >= _queue.size()) {
        StopScan(true);
        return;
    }
    SetLit((int)_queue[_queuePos]);
    _stateStartMs = nowMs;
    SetStatus(wxString::Format(_("Mapping node %u of %u... (last node %s)"),
                               (unsigned)_queue[_queuePos] + 1, (unsigned)_nodes.size(),
                               found ? _("found") : _("MISSED")).ToStdString());
}

#pragma endregion Scanning

#pragma region Results

static wxString StatusLabel(int status)
{
    switch (status) {
    case 1: return _("Found");
    case 2: return _("Missing");
    case 3: return _("Manual");
    case 4: return _("Guessed");
    default: return _("-");
    }
}

void CustomModelBuilderDialog::RefreshNodeList()
{
    _listNodes->DeleteAllItems();
    for (size_t i = 0; i < _nodes.size(); ++i) {
        long row = _listNodes->InsertItem((long)i, wxString::Format("%u", (unsigned)i + 1));
        (void)row;
        UpdateNodeRow((uint32_t)i);
    }
}

void CustomModelBuilderDialog::UpdateNodeRow(uint32_t node)
{
    if ((long)node >= _listNodes->GetItemCount()) return;
    const auto& n = _nodes[node];
    _listNodes->SetItem((long)node, 1, StatusLabel((int)n.status));
    _listNodes->SetItem((long)node, 2, n.pos.x >= 0 ? wxString::Format("%d", n.pos.x) : wxString(""));
    _listNodes->SetItem((long)node, 3, n.pos.y >= 0 ? wxString::Format("%d", n.pos.y) : wxString(""));
}

void CustomModelBuilderDialog::GuessMissing()
{
    int guessed = 0;
    for (size_t i = 0; i < _nodes.size(); ++i) {
        if (_nodes[i].status != NodeInfo::Status::Missing && _nodes[i].status != NodeInfo::Status::NotScanned) continue;

        int prev = -1, next = -1;
        for (int j = (int)i - 1; j >= 0; --j) {
            if (_nodes[j].pos.x >= 0) { prev = j; break; }
        }
        for (int j = (int)i + 1; j < (int)_nodes.size(); ++j) {
            if (_nodes[j].pos.x >= 0) { next = j; break; }
        }
        if (prev < 0 || next < 0) continue;

        double t = (double)((int)i - prev) / (double)(next - prev);
        _nodes[i].pos = wxPoint(
            (int)std::lround(_nodes[prev].pos.x + t * (_nodes[next].pos.x - _nodes[prev].pos.x)),
            (int)std::lround(_nodes[prev].pos.y + t * (_nodes[next].pos.y - _nodes[prev].pos.y)));
        _nodes[i].status = NodeInfo::Status::Guessed;
        UpdateNodeRow((uint32_t)i);
        ++guessed;
    }
    SetStatus(wxString::Format(_("Guessed positions for %d nodes (shown orange). Drag any that look wrong."), guessed).ToStdString());
    _preview->Refresh();
}

bool CustomModelBuilderDialog::BuildGrid(std::vector<std::vector<int>>& grid) const
{
    std::vector<std::pair<uint32_t, wxPoint>> placed;
    for (size_t i = 0; i < _nodes.size(); ++i) {
        if (_nodes[i].pos.x >= 0) placed.emplace_back((uint32_t)i, _nodes[i].pos);
    }
    if (placed.size() < 2) return false;

    // cell size = a low percentile of the nearest-neighbour distances rather
    // than the single closest pair, so one near-duplicate detection can't
    // inflate the whole grid; the collision handling below absorbs the few
    // genuinely-tighter pairs by nudging them to an adjacent free cell
    std::vector<double> nearest(placed.size(), 1e12);
    for (size_t a = 0; a < placed.size(); ++a) {
        for (size_t b = a + 1; b < placed.size(); ++b) {
            double dx = placed[a].second.x - placed[b].second.x;
            double dy = placed[a].second.y - placed[b].second.y;
            double d = std::sqrt(dx * dx + dy * dy);
            if (d > 0.5) {
                nearest[a] = std::min(nearest[a], d);
                nearest[b] = std::min(nearest[b], d);
            }
        }
    }
    std::sort(nearest.begin(), nearest.end());
    double minSep = nearest[nearest.size() / 10]; // 10th percentile
    if (minSep > 1e11 || minSep < 1.0) minSep = 1.0;

    int minX = INT_MAX, minY = INT_MAX, maxX = 0, maxY = 0;
    for (const auto& p : placed) {
        minX = std::min(minX, p.second.x);
        minY = std::min(minY, p.second.y);
        maxX = std::max(maxX, p.second.x);
        maxY = std::max(maxY, p.second.y);
    }

    double scale = GRID_SIZE_FACTOR / minSep;
    int w = (int)std::lround((maxX - minX) * scale) + 1;
    int h = (int)std::lround((maxY - minY) * scale) + 1;
    int maxDim = std::max(w, h);
    if (maxDim > MAX_GRID_DIMENSION) {
        scale *= (double)MAX_GRID_DIMENSION / maxDim;
        w = (int)std::lround((maxX - minX) * scale) + 1;
        h = (int)std::lround((maxY - minY) * scale) + 1;
    }

    // round the grid up to the nearest 5 in each direction for tidier model
    // dimensions, centring the mapped nodes in the extra cells (safe against
    // MAX_GRID_DIMENSION because that is itself a multiple of 5)
    int offX = (((w + 4) / 5) * 5 - w) / 2;
    int offY = (((h + 4) / 5) * 5 - h) / 2;
    w = ((w + 4) / 5) * 5;
    h = ((h + 4) / 5) * 5;

    grid.assign(h, std::vector<int>(w, -1));
    for (const auto& p : placed) {
        int x = (int)std::lround((p.second.x - minX) * scale) + offX;
        int y = (int)std::lround((p.second.y - minY) * scale) + offY;
        // nearest free cell if two nodes round onto the same one
        bool set = false;
        for (int r = 0; r <= 10 && !set; ++r) {
            for (int oy = -r; oy <= r && !set; ++oy) {
                for (int ox = -r; ox <= r && !set; ++ox) {
                    if (std::max(std::abs(ox), std::abs(oy)) != r) continue;
                    int cx = x + ox;
                    int cy = y + oy;
                    if (cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
                    if (grid[cy][cx] == -1) {
                        grid[cy][cx] = (int)p.first + 1;
                        set = true;
                    }
                }
            }
        }
        if (!set) {
            spdlog::warn("CustomModelBuilder: could not place node {} in the grid (too crowded).", p.first + 1);
        }
    }
    return true;
}

void CustomModelBuilderDialog::UpdateModelInPlace(const std::vector<std::vector<int>>& grid)
{
    auto* cm = dynamic_cast<CustomModel*>(_model);
    if (cm == nullptr) return;

    cm->SetCustomWidth((long)grid[0].size());
    cm->SetCustomHeight((long)grid.size());
    cm->SetCustomDepth(1);
    std::vector<std::vector<std::vector<int>>> data;
    data.push_back(grid);
    cm->SetCustomData(data);

    _frame->GetOutputModelManager()->AddASAPWork(
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
        OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW,
        "CustomModelBuilderDialog::UpdateModelInPlace");
}

bool CustomModelBuilderDialog::SaveAsXModel(const std::vector<std::vector<int>>& grid)
{
    wxLogNull logNo; // avoid a spurious "error 0" message after the file is written
    wxString filename = wxFileSelector(_("Save custom model"), wxEmptyString,
                                       _model != nullptr ? wxString(_model->GetName()) : wxString("NewCustomModel"),
                                       wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel",
                                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
    if (filename.IsEmpty()) return false;

    std::vector<std::vector<std::vector<int>>> data;
    data.push_back(grid);
    std::string cm = CustomModel::ToCustomModel(data);
    std::string compressed = CustomModel::ToCompressed(data);

    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError("Unable to create file " + filename.ToStdString() + ".", this);
        return false;
    }
    wxString name = wxFileName(filename).GetName();
    std::string stringType = _model != nullptr ? _model->GetStringType() : "RGB Nodes";
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("CustomWidth=\"%d\" ", (int)grid[0].size()));
    f.Write(wxString::Format("CustomHeight=\"%d\" ", (int)grid.size()));
    f.Write("Depth=\"1\" ");
    f.Write(wxString::Format("StringType=\"%s\" ", stringType));
    f.Write("Transparency=\"0\" PixelSize=\"2\" ModelBrightness=\"0\" Antialias=\"1\" StrandNames=\"\" NodeNames=\"\" ");
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    if (!compressed.empty()) {
        f.Write("CustomModelCompressed=\"");
        f.Write(compressed);
        f.Write("\" ");
    }
    f.Write(wxString::Format("SourceVersion=\"%s\" ", xlights_version_string));
    f.Write(" >\n</custommodel>");
    f.Close();
    spdlog::info("CustomModelBuilder: saved {}.", filename.ToStdString());
    return true;
}

#pragma endregion Results

#pragma region Preview

wxPoint CustomModelBuilderDialog::PanelToCamera(const wxPoint& p) const
{
    if (_fitScale <= 0) return wxPoint(-1, -1);
    return wxPoint((int)std::lround((p.x - _fitOffset.x) / _fitScale),
                   (int)std::lround((p.y - _fitOffset.y) / _fitScale));
}

wxPoint CustomModelBuilderDialog::CameraToPanel(const wxPoint& p) const
{
    return wxPoint((int)std::lround(p.x * _fitScale) + _fitOffset.x,
                   (int)std::lround(p.y * _fitScale) + _fitOffset.y);
}

int CustomModelBuilderDialog::FindNodeNear(const wxPoint& panelPt, int tolerancePx) const
{
    int best = -1;
    long bestDist = tolerancePx * tolerancePx + 1;
    for (size_t i = 0; i < _nodes.size(); ++i) {
        if (_nodes[i].pos.x < 0) continue;
        wxPoint sp = CameraToPanel(_nodes[i].pos);
        long dx = sp.x - panelPt.x;
        long dy = sp.y - panelPt.y;
        long d = dx * dx + dy * dy;
        if (d < bestDist) {
            bestDist = d;
            best = (int)i;
        }
    }
    return best;
}

int CustomModelBuilderDialog::TargetNodeForClick() const
{
    if (_state == ScanState::Settle && _paused && _queuePos < _queue.size()) {
        return (int)_queue[_queuePos];
    }
    // only place by hand when exactly one node is selected - with a
    // multi-selection there is no way to know which one the click means
    if ((_state == ScanState::Idle || _state == ScanState::Done) && _listNodes->GetSelectedItemCount() == 1) {
        long sel = _listNodes->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (sel >= 0 && sel < (long)_nodes.size()) return (int)sel;
    }
    return -1;
}

void CustomModelBuilderDialog::OnPaintPreview(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC dc(_preview);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();

    wxSize ps = _preview->GetClientSize();
    if (_lastFrame.empty() || _frameWidth <= 0 || _frameHeight <= 0) {
        dc.SetTextForeground(*wxWHITE);
        dc.DrawText(_("No camera image"), 10, 10);
        return;
    }

    double sx = (double)ps.x / _frameWidth;
    double sy = (double)ps.y / _frameHeight;
    if (_zoom <= 1.0) {
        // fully zoomed out: fit and centre the frame, and forget any pan
        _zoom = 1.0;
        _viewCenterX = _frameWidth / 2.0;
        _viewCenterY = _frameHeight / 2.0;
    }
    _fitScale = std::min(sx, sy) * _zoom;
    // keep the view inside the frame while zoomed in
    double halfW = ps.x / (2.0 * _fitScale);
    double halfH = ps.y / (2.0 * _fitScale);
    _viewCenterX = (_frameWidth * _fitScale <= ps.x) ? _frameWidth / 2.0 : std::clamp(_viewCenterX, halfW, _frameWidth - halfW);
    _viewCenterY = (_frameHeight * _fitScale <= ps.y) ? _frameHeight / 2.0 : std::clamp(_viewCenterY, halfH, _frameHeight - halfH);
    _fitOffset = wxPoint((int)std::lround(ps.x / 2.0 - _viewCenterX * _fitScale),
                         (int)std::lround(ps.y / 2.0 - _viewCenterY * _fitScale));

    wxImage img(_frameWidth, _frameHeight, const_cast<unsigned char*>(_lastFrame.data()), true);
    // scale only the visible part of the frame - at high zoom scaling the whole
    // frame would allocate enormous intermediate bitmaps
    int cx0 = std::clamp((int)std::floor((0 - _fitOffset.x) / _fitScale), 0, _frameWidth);
    int cy0 = std::clamp((int)std::floor((0 - _fitOffset.y) / _fitScale), 0, _frameHeight);
    int cx1 = std::clamp((int)std::ceil((ps.x - _fitOffset.x) / _fitScale), 0, _frameWidth);
    int cy1 = std::clamp((int)std::ceil((ps.y - _fitOffset.y) / _fitScale), 0, _frameHeight);
    if (cx1 > cx0 && cy1 > cy0) {
        wxImage sub = img.GetSubImage(wxRect(cx0, cy0, cx1 - cx0, cy1 - cy0));
        wxBitmap bmp(sub.Scale((int)std::lround((cx1 - cx0) * _fitScale), (int)std::lround((cy1 - cy0) * _fitScale), wxIMAGE_QUALITY_NEAREST));
        dc.DrawBitmap(bmp, (int)std::lround(cx0 * _fitScale) + _fitOffset.x, (int)std::lround(cy0 * _fitScale) + _fitOffset.y);
    }

    std::vector<bool> selected(_nodes.size(), false);
    long selRow = -1;
    while ((selRow = _listNodes->GetNextItem(selRow, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0) {
        if (selRow < (long)_nodes.size()) selected[selRow] = true;
    }

    // solid black backing behind all overlay text - bare coloured digits
    // disappear into the bright blobs they sit on; labels also grow with the
    // zoom so they stay comfortably readable while inspecting pixels
    dc.SetBackgroundMode(wxSOLID);
    dc.SetTextBackground(*wxBLACK);
    int labelPt = std::min(10 + (int)std::lround(3.0 * (_zoom - 1.0)), 22);
    dc.SetFont(wxFont(labelPt, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    for (size_t i = 0; i < _nodes.size(); ++i) {
        const auto& n = _nodes[i];
        if (n.pos.x < 0) continue;
        wxPoint sp = CameraToPanel(n.pos);
        wxColour colour;
        switch (n.status) {
        case NodeInfo::Status::Manual: colour = wxColour(0, 200, 255); break;
        case NodeInfo::Status::Guessed: colour = wxColour(255, 160, 0); break;
        default: colour = wxColour(0, 220, 0); break;
        }
        dc.SetBrush(wxBrush(colour));
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawCircle(sp, 4);
        if (selected[i]) {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(wxPen(*wxWHITE, 2));
            dc.DrawCircle(sp, 8);
        }
        dc.SetTextForeground(colour);
        dc.DrawText(wxString::Format("%u", (unsigned)i + 1), sp.x + 6, sp.y - 6);
    }

    // currently-lit node marker while scanning
    if (_state == ScanState::Settle && _queuePos < _queue.size()) {
        dc.SetTextForeground(*wxYELLOW);
        dc.SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.DrawText(wxString::Format(_("Lighting node %u"), (unsigned)_queue[_queuePos] + 1), 10, 10);
    }

    if (_zoom > 1.0) {
        dc.SetTextForeground(*wxWHITE);
        dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.DrawText(wxString::Format("%.1fx", _zoom), ps.x - 45, 8);
    }
}

void CustomModelBuilderDialog::OnPreviewLeftDown(wxMouseEvent& event)
{
    _preview->SetFocus(); // so the scroll wheel zooms the preview, not the node list
    if (_state == ScanState::Idle || _state == ScanState::Done) {
        int nearNode = FindNodeNear(event.GetPosition());
        if (nearNode >= 0) {
            _draggingNode = nearNode;
            // plain click selects just this node; ctrl-click adds it to the
            // selection so a batch can be built up for Retry Selected Nodes
            if (!event.ControlDown()) {
                long s = -1;
                while ((s = _listNodes->GetNextItem(s, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0) {
                    if (s != (long)nearNode) _listNodes->SetItemState(s, 0, wxLIST_STATE_SELECTED);
                }
            }
            _listNodes->SetItemState((long)nearNode, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            _listNodes->EnsureVisible((long)nearNode);
            _preview->Refresh();
            return;
        }
    }
    // click on empty video: place the target node by hand
    int target = TargetNodeForClick();
    if (target >= 0) {
        wxPoint cp = PanelToCamera(event.GetPosition());
        if (cp.x >= 0 && cp.y >= 0 && cp.x < _frameWidth && cp.y < _frameHeight) {
            _nodes[target].pos = cp;
            _nodes[target].status = NodeInfo::Status::Manual;
            UpdateNodeRow((uint32_t)target);
            SetStatus(wxString::Format(_("Node %d placed manually."), target + 1).ToStdString());
            _preview->Refresh();
        }
    }
}

void CustomModelBuilderDialog::OnPreviewMotion(wxMouseEvent& event)
{
    if (_panning && (event.MiddleIsDown() || event.RightIsDown())) {
        wxPoint p = event.GetPosition();
        if (_fitScale > 0) {
            _viewCenterX -= (p.x - _panLast.x) / _fitScale;
            _viewCenterY -= (p.y - _panLast.y) / _fitScale;
        }
        _panLast = p;
        _preview->Refresh();
        return;
    }
    if (_draggingNode >= 0 && event.LeftIsDown()) {
        wxPoint cp = PanelToCamera(event.GetPosition());
        cp.x = std::clamp(cp.x, 0, std::max(0, _frameWidth - 1));
        cp.y = std::clamp(cp.y, 0, std::max(0, _frameHeight - 1));
        _nodes[_draggingNode].pos = cp;
        if (_nodes[_draggingNode].status != NodeInfo::Status::Found) {
            _nodes[_draggingNode].status = NodeInfo::Status::Manual;
        }
        UpdateNodeRow((uint32_t)_draggingNode);
        _preview->Refresh();
    }
}

void CustomModelBuilderDialog::OnPreviewLeftUp(wxMouseEvent& /*event*/)
{
    _draggingNode = -1;
}

void CustomModelBuilderDialog::OnPreviewWheel(wxMouseEvent& event)
{
    // bound dialog-wide, so locate the pointer relative to the preview
    // ourselves and pass the event on when it is over some other control
    wxPoint p = _preview->ScreenToClient(wxGetMousePosition());
    wxSize ps = _preview->GetClientSize();
    if (p.x < 0 || p.y < 0 || p.x >= ps.x || p.y >= ps.y) {
        event.Skip();
        return;
    }
    if (_lastFrame.empty() || _frameWidth <= 0 || _fitScale <= 0) return;
    int rot = event.GetWheelRotation();
    if (rot == 0) return;

    double newZoom = std::clamp(_zoom * ((rot > 0) ? 1.25 : 1.0 / 1.25), 1.0, 16.0);
    if (newZoom == _zoom) return;

    // keep the camera pixel under the cursor stationary while zooming; the
    // paint handler clamps the resulting view centre back inside the frame
    double camX = (p.x - _fitOffset.x) / _fitScale;
    double camY = (p.y - _fitOffset.y) / _fitScale;
    double newScale = std::min((double)ps.x / _frameWidth, (double)ps.y / _frameHeight) * newZoom;
    _zoom = newZoom;
    _viewCenterX = camX + (ps.x / 2.0 - p.x) / newScale;
    _viewCenterY = camY + (ps.y / 2.0 - p.y) / newScale;
    _preview->Refresh();
}

void CustomModelBuilderDialog::OnPreviewPanDown(wxMouseEvent& event)
{
    _preview->SetFocus();
    if (_zoom > 1.0) {
        _panning = true;
        _panLast = event.GetPosition();
    }
}

void CustomModelBuilderDialog::OnPreviewPanUp(wxMouseEvent& /*event*/)
{
    _panning = false;
}

#pragma endregion Preview

#pragma region Events

void CustomModelBuilderDialog::OnTimer(wxTimerEvent& /*event*/)
{
    if (_camera != nullptr) {
        std::ignore = _camera->TryGetLatestFrame(_lastFrame, _frameWidth, _frameHeight);
    }

    long now = NowMs();
    if (_state == ScanState::Baseline || _state == ScanState::Settle || _state == ScanState::AutoBright) {
        AdvanceScan(now);
    }

    // periodic re-send keeps e1.31/ddp receivers from timing out back to
    // their idle state between node changes
    SendOutputFrame();

    _preview->Refresh(false);
}

void CustomModelBuilderDialog::OnChoiceModel(wxCommandEvent& /*event*/)
{
    ApplySelectedModel();
    _nodes.clear();
    _listNodes->DeleteAllItems();
    _state = ScanState::Idle;
    ValidateWindow();
}

void CustomModelBuilderDialog::OnSliderBrightness(wxCommandEvent& /*event*/)
{
    _labelBrightness->SetLabel(wxString::Format("%d", _sliderBrightness->GetValue()));
}

void CustomModelBuilderDialog::OnButtonAutoBrightness(wxCommandEvent& /*event*/)
{
    StartAutoBrightness();
}

void CustomModelBuilderDialog::OnButtonLightAll(wxCommandEvent& /*event*/)
{
    Model* m = GetSelectedModel();
    if (m == nullptr) {
        _buttonLightAll->SetValue(false);
        return;
    }
    if (_buttonLightAll->GetValue()) {
        if (!PrepareOutput(m)) {
            _buttonLightAll->SetValue(false);
            return;
        }
        SetLit(-2);
        SetStatus(wxString::Format(_("All %u nodes of '%s' on. Aim and focus the camera so the whole prop is in frame, then Start Mapping."),
                                   (unsigned)_nodeCount, wxString(m->GetName())).ToStdString());
    } else {
        SetLit(-1);
        // only leave dark mode if no scan needs it - Light All can be toggled
        // off without stopping an in-flight mapping run
        if (_state == ScanState::Idle || _state == ScanState::Done) {
            SetCameraDarkMode(false);
        }
    }
}

void CustomModelBuilderDialog::OnButtonStart(wxCommandEvent& /*event*/)
{
    Model* m = GetSelectedModel();
    if (m == nullptr) return;
    uint32_t count = (uint32_t)m->GetNodeCount();
    _nodes.assign(count, NodeInfo());
    RefreshNodeList();
    std::vector<uint32_t> queue(count);
    for (uint32_t i = 0; i < count; ++i) queue[i] = i;
    StartScan(std::move(queue));
}

void CustomModelBuilderDialog::OnButtonPause(wxCommandEvent& /*event*/)
{
    _paused = _buttonPause->GetValue();
    if (_paused) {
        SetStatus("Paused. You can click the video to place the current node by hand, then resume.");
    }
}

void CustomModelBuilderDialog::OnButtonStopScan(wxCommandEvent& /*event*/)
{
    StopScan(false);
}

void CustomModelBuilderDialog::OnButtonRetryMissing(wxCommandEvent& /*event*/)
{
    std::vector<uint32_t> queue;
    for (size_t i = 0; i < _nodes.size(); ++i) {
        if (_nodes[i].status == NodeInfo::Status::Missing || _nodes[i].status == NodeInfo::Status::NotScanned) {
            queue.push_back((uint32_t)i);
        }
    }
    if (queue.empty()) {
        SetStatus("No missing nodes to retry.");
        return;
    }
    StartScan(std::move(queue));
}

void CustomModelBuilderDialog::OnButtonRetrySelected(wxCommandEvent& /*event*/)
{
    std::vector<uint32_t> queue;
    long sel = -1;
    while ((sel = _listNodes->GetNextItem(sel, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0) {
        if (sel < (long)_nodes.size()) queue.push_back((uint32_t)sel);
    }
    if (queue.empty()) {
        SetStatus("Select one or more nodes in the list first (Ctrl/Shift-click for several).");
        return;
    }
    StartScan(std::move(queue));
}

void CustomModelBuilderDialog::OnButtonClear(wxCommandEvent& /*event*/)
{
    if (wxMessageBox(_("Clear all mapped node positions? You will need to scan again."),
                     _("Clear Mapping"), wxYES_NO | wxICON_QUESTION, this) != wxYES) {
        return;
    }
    _nodes.assign(_nodes.size(), NodeInfo());
    RefreshNodeList();
    _preview->Refresh();
    SetStatus("Mapping cleared. Aim the camera, then Start Mapping to scan again.");
    ValidateWindow();
}

void CustomModelBuilderDialog::OnButtonGuessMissing(wxCommandEvent& /*event*/)
{
    GuessMissing();
}

void CustomModelBuilderDialog::OnButtonFinish(wxCommandEvent& /*event*/)
{
    StopScan(false);

    std::vector<std::vector<int>> grid;
    if (!BuildGrid(grid)) {
        DisplayError("Not enough located nodes to build a model - map at least 2 nodes first.", this);
        return;
    }

    int placed = 0;
    for (const auto& row : grid) {
        for (int c : row) {
            if (c >= 0) ++placed;
        }
    }

    auto* cm = _model != nullptr ? dynamic_cast<CustomModel*>(_model) : nullptr;
    if (cm != nullptr) {
        wxMessageDialog prompt(this,
            wxString::Format(_("%d of %u nodes mapped into a %dx%d grid.\n\nUpdate the model '%s' with this layout, or save it as a new .xmodel file?"),
                             placed, (unsigned)_nodes.size(), (int)grid[0].size(), (int)grid.size(), _model->GetName()),
            _("Custom Model Builder"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
        prompt.SetYesNoCancelLabels(wxString::Format(_("Update '%s'"), _model->GetName()), _("Save as .xmodel..."), _("Cancel"));
        int answer = prompt.ShowModal();
        if (answer == wxID_YES) {
            UpdateModelInPlace(grid);
            EndModal(wxID_OK);
            return;
        }
        if (answer == wxID_NO) {
            if (SaveAsXModel(grid)) {
                EndModal(wxID_OK);
            }
            return;
        }
        return; // cancelled - stay in the dialog
    }

    wxMessageDialog prompt(this,
        wxString::Format(_("%d of %u nodes mapped into a %dx%d grid.\n\nThe selected model is not a Custom model so it cannot be updated in place - save the result as a .xmodel file?"),
                         placed, (unsigned)_nodes.size(), (int)grid[0].size(), (int)grid.size()),
        _("Custom Model Builder"), wxYES_NO | wxICON_QUESTION);
    prompt.SetYesNoLabels(_("Save as .xmodel..."), _("Cancel"));
    if (prompt.ShowModal() == wxID_YES) {
        if (SaveAsXModel(grid)) {
            EndModal(wxID_OK);
        }
    }
}

void CustomModelBuilderDialog::OnClose(wxCloseEvent& event)
{
    Cleanup();
    event.Skip();
}

#pragma endregion Events

void CustomModelBuilderDialog::SetStatus(const std::string& text)
{
    _labelStatus->SetLabel(wxString::FromUTF8(text));
}

void CustomModelBuilderDialog::ValidateWindow()
{
    bool scanning = (_state == ScanState::Baseline || _state == ScanState::Settle || _state == ScanState::AutoBright);
    bool haveResults = false;
    for (const auto& n : _nodes) {
        if (n.pos.x >= 0) {
            haveResults = true;
            break;
        }
    }

    _choiceModel->Enable(!scanning);
    _buttonLightAll->Enable(!scanning);
    _buttonAutoBrightness->Enable(!scanning);
    _buttonStart->Enable(!scanning);
    _buttonPause->Enable(scanning);
    _buttonStopScan->Enable(scanning);
    _buttonRetryMissing->Enable(!scanning && !_nodes.empty());
    _buttonRetrySelected->Enable(!scanning && !_nodes.empty());
    _buttonGuessMissing->Enable(!scanning && haveResults);
    _buttonClear->Enable(!scanning && haveResults);
    _buttonFinish->Enable(!scanning && haveResults);
}

#endif // XLIGHTS_HAVE_LIVE_CAMERA
