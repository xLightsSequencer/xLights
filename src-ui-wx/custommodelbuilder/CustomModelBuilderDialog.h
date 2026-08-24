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

// Tools -> Custom Model Builder. Windows + macOS: drives an existing model's
// lights one node at a time through the output system while a webcam watches,
// detecting each node's position live and overlaying it on the camera preview
// as the scan runs. The mapped positions become a custom model (update the
// scanned model in place, or save as a .xmodel).

#include "media/LiveCameraCapture.h"   // XLIGHTS_HAVE_LIVE_CAMERA

#ifdef XLIGHTS_HAVE_LIVE_CAMERA

#include <wx/dialog.h>
#include <wx/timer.h>

#include <cstdint>
#include <string>
#include <vector>

#include "media/LiveCameraCapture.h"

class wxButton;
class wxChoice;
class wxGauge;
class wxListEvent;
class wxListCtrl;
class wxPanel;
class wxSlider;
class wxStaticText;
class wxToggleButton;

class Model;
class OutputManager;
class xLightsFrame;

class CustomModelBuilderDialog : public wxDialog
{
public:
    // initialCameraSymbolicLink: platform camera id chosen
    // upstream in CustomModelMethodPickerDialog's "USB Webcam node
    // identification" flow - this dialog just opens it, it no longer has its
    // own camera dropdown. Empty falls back to the last-remembered camera,
    // then the first one found.
    CustomModelBuilderDialog(xLightsFrame* parent, OutputManager* outputManager,
                              const std::string& initialCameraSymbolicLink = std::string());
    virtual ~CustomModelBuilderDialog();

private:
    struct NodeInfo {
        enum class Status { NotScanned, Found, Missing, Manual, Guessed };
        Status status = Status::NotScanned;
        wxPoint pos = wxPoint(-1, -1); // camera pixel coordinates
        int peak = 0;                  // brightness delta at detection time
    };

    enum class ScanState { Idle, Baseline, Settle, AutoBright, Done };

    xLightsFrame* _frame = nullptr;
    OutputManager* _outputManager = nullptr;
    std::string _initialCameraSymbolicLink;

    // controls
    wxStaticText* _labelCamera = nullptr;
    wxChoice* _choiceModel = nullptr;
    wxStaticText* _labelModelInfo = nullptr;
    wxSlider* _sliderBrightness = nullptr;
    wxStaticText* _labelBrightness = nullptr;
    wxButton* _buttonAutoBrightness = nullptr;
    wxToggleButton* _buttonLightAll = nullptr;
    wxPanel* _preview = nullptr;
    wxListCtrl* _listNodes = nullptr;
    wxButton* _buttonStart = nullptr;
    wxToggleButton* _buttonPause = nullptr;
    wxButton* _buttonStopScan = nullptr;
    wxButton* _buttonRetryMissing = nullptr;
    wxButton* _buttonRetrySelected = nullptr;
    wxButton* _buttonGuessMissing = nullptr;
    wxButton* _buttonClear = nullptr;
    wxButton* _buttonFinish = nullptr;
    wxGauge* _gauge = nullptr;
    wxStaticText* _labelStatus = nullptr;
    wxTimer _timer;

    // camera
    std::vector<LiveCameraDevice> _cameras;
    int _selectedCameraIndex = -1;
    LiveCameraCapture* _camera = nullptr;
    std::vector<uint8_t> _lastFrame; // packed RGB24 top-down
    int _frameWidth = 0;
    int _frameHeight = 0;

    // selected model parameters (captured at scan start)
    Model* _model = nullptr;
    uint32_t _startChannel = 0; // 1-based
    uint32_t _nodeCount = 0;
    uint32_t _chanPerNode = 3;

    // scan state machine, advanced from the timer so the UI stays live
    ScanState _state = ScanState::Idle;
    bool _paused = false;
    std::vector<uint32_t> _queue; // 0-based node numbers still to scan
    size_t _queuePos = 0;
    long _stateStartMs = 0;
    int _litNode = -1; // -1 all off, -2 all on, else 0-based lit node
    bool _wasOutputting = false;
    bool _outputsStarted = false;
    long long _epochMs = 0;
    std::vector<uint8_t> _baseline;
    std::vector<std::vector<uint8_t>> _baselineSamples;
    int _detectThreshold = 40;

    // DetectBlob scratch buffers, reused across calls (it runs on essentially
    // every timer tick while scanning) to avoid a per-frame heap allocation.
    mutable std::vector<uint8_t> _diffScratch;
    mutable std::vector<uint8_t> _visitedScratch;

    // adaptive per-node pacing: the light->camera pipeline latency is measured
    // on the first node of a scan, then each node is sampled every frame from
    // that point instead of waiting a fixed worst-case settle time
    long _minSettleMs = 60;
    bool _latencyCalibrated = false;
    wxPoint _pendingPos = wxPoint(-1, -1);
    int _pendingCount = 0;

    // auto-brightness: after the baseline, one node is lit at increasing levels
    // and the lowest level that comfortably clears the noise floor wins
    bool _autoBrightRun = false;
    size_t _autoBrightIdx = 0;
    int _noiseFloor = 0;
    int _savedBrightness = 0;

    std::vector<NodeInfo> _nodes;

    // preview mapping camera<->panel, recomputed each paint
    double _fitScale = 1.0;
    wxPoint _fitOffset = wxPoint(0, 0);
    int _draggingNode = -1;

    // wheel zoom: 1 = whole frame fitted to the panel. While zoomed in the view
    // centre (camera pixel coordinates) can be panned by middle/right dragging.
    double _zoom = 1.0;
    double _viewCenterX = 0.0;
    double _viewCenterY = 0.0;
    bool _panning = false;
    wxPoint _panLast = wxPoint(0, 0);

    // setup
    void PopulateCameraChoice();
    void PopulateModelChoice();
    void OpenSelectedCamera();
    void ApplySelectedModel();
    Model* GetSelectedModel() const;

    // scanning
    bool PrepareOutput(Model* m);
    void SetCameraDarkMode(bool enabled);
    void StartAutoBrightness();
    void FinishAutoBrightness(int chosenLevel);
    void StartScan(std::vector<uint32_t> queue);
    void StopScan(bool completed);
    void AdvanceScan(long nowMs);
    void NextQueueNode(long nowMs, uint32_t node, bool found, const wxPoint& pos, int peak);
    void SendOutputFrame();
    void SetLit(int litNode);
    bool DetectBlob(wxPoint& outPos, int& outPeak) const;
    void ComputeBaseline();
    void RecordResult(uint32_t node, bool found, const wxPoint& pos, int peak);
    long NowMs() const;

    // results
    void RefreshNodeList();
    void UpdateNodeRow(uint32_t node);
    void GuessMissing();
    bool BuildGrid(std::vector<std::vector<int>>& grid) const;
    void UpdateModelInPlace(const std::vector<std::vector<int>>& grid);
    bool SaveAsXModel(const std::vector<std::vector<int>>& grid);

    // preview interaction
    wxPoint PanelToCamera(const wxPoint& p) const;
    wxPoint CameraToPanel(const wxPoint& p) const;
    int FindNodeNear(const wxPoint& panelPt, int tolerancePx = 10) const;
    int TargetNodeForClick() const;

    void ValidateWindow();
    void SetStatus(const std::string& text);
    void Cleanup();

    // events
    void OnTimer(wxTimerEvent& event);
    void OnPaintPreview(wxPaintEvent& event);
    void OnPreviewLeftDown(wxMouseEvent& event);
    void OnPreviewLeftUp(wxMouseEvent& event);
    void OnPreviewMotion(wxMouseEvent& event);
    void OnPreviewWheel(wxMouseEvent& event);
    void OnPreviewPanDown(wxMouseEvent& event);
    void OnPreviewPanUp(wxMouseEvent& event);
    void OnChoiceModel(wxCommandEvent& event);
    void OnSliderBrightness(wxCommandEvent& event);
    void OnButtonAutoBrightness(wxCommandEvent& event);
    void OnButtonLightAll(wxCommandEvent& event);
    void OnButtonStart(wxCommandEvent& event);
    void OnButtonPause(wxCommandEvent& event);
    void OnButtonStopScan(wxCommandEvent& event);
    void OnButtonRetryMissing(wxCommandEvent& event);
    void OnButtonRetrySelected(wxCommandEvent& event);
    void OnButtonGuessMissing(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& event);
    void OnButtonFinish(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
};

#endif // XLIGHTS_HAVE_LIVE_CAMERA
