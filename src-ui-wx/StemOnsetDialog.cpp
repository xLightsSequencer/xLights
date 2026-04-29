/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StemOnsetDialog.h"
#include "sequencer/StemWaveform.h"
#include "sequencer/TimeLine.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "media/AudioManager.h"

extern "C" {
#include "aubio/src/types.h"
#include "aubio/src/fvec.h"
#include "aubio/src/cvec.h"
#include "aubio/src/spectral/phasevoc.h"
#include "aubio/src/spectral/specdesc.h"
}


#include <cmath>
#include <algorithm>
#include <numeric>

const FrequencyBandPreset StemOnsetDialog::PRESETS[] = {
    {"Kick",          20.0f,    150.0f,   0.30f, 200, "energy"},
    {"Snare",         150.0f,   5000.0f,  0.25f, 80,  "specflux"},
    {"Hi-Hat",        5000.0f,  16000.0f, 0.50f, 60,  "specflux"},
    {"Toms",          80.0f,    800.0f,   0.35f, 150, "energy"},
    {"Full Range",    20.0f,    20000.0f, 0.30f, 50,  "default"},
    {"Custom",        20.0f,    20000.0f, 0.30f, 50,  "default"},
};
const int StemOnsetDialog::NUM_PRESETS = sizeof(PRESETS) / sizeof(PRESETS[0]);
const int StemOnsetDialog::CUSTOM_PRESET_INDEX = 5;

const long StemOnsetDialog::ID_CHOICE_PRESET = wxNewId();
const long StemOnsetDialog::ID_SLIDER_LOW_FREQ = wxNewId();
const long StemOnsetDialog::ID_SLIDER_HIGH_FREQ = wxNewId();
const long StemOnsetDialog::ID_SLIDER_THRESHOLD = wxNewId();
const long StemOnsetDialog::ID_SLIDER_MIN_INTERVAL = wxNewId();
const long StemOnsetDialog::ID_TEXT_TRACK_NAME = wxNewId();
const long StemOnsetDialog::ID_BTN_CREATE = wxNewId();
const long StemOnsetDialog::ID_BTN_CANCEL = wxNewId();
const long StemOnsetDialog::ID_DEBOUNCE_TIMER = wxNewId();

BEGIN_EVENT_TABLE(StemOnsetDialog, wxDialog)
    EVT_BUTTON(StemOnsetDialog::ID_BTN_CREATE, StemOnsetDialog::OnCreateTimingTrack)
    EVT_BUTTON(StemOnsetDialog::ID_BTN_CANCEL, StemOnsetDialog::OnCancel)
    EVT_TIMER(StemOnsetDialog::ID_DEBOUNCE_TIMER, StemOnsetDialog::OnDebounceTimer)
    EVT_CLOSE(StemOnsetDialog::OnClose)
END_EVENT_TABLE()

// Log-scale frequency conversion: slider 0-1000 maps to 20-20000 Hz
float StemOnsetDialog::SliderToFreq(int val)
{
    float t = (float)val / 1000.0f;
    return expf(logf(20.0f) + t * (logf(20000.0f) - logf(20.0f)));
}

int StemOnsetDialog::FreqToSlider(float hz)
{
    if (hz < 20.0f) hz = 20.0f;
    if (hz > 20000.0f) hz = 20000.0f;
    float t = (logf(hz) - logf(20.0f)) / (logf(20000.0f) - logf(20.0f));
    return (int)(t * 1000.0f);
}

wxString StemOnsetDialog::FormatFrequency(float hz)
{
    if (hz >= 1000.0f) {
        return wxString::Format("%.1f kHz", hz / 1000.0f);
    }
    return wxString::Format("%.0f Hz", hz);
}

StemOnsetDialog::StemOnsetDialog(wxWindow* parent, StemWaveform* waveform,
                                 xLightsFrame* frame, const std::string& stemName)
    : wxDialog(parent, wxID_ANY, "Onset Detection - " + stemName,
               wxDefaultPosition, wxSize(420, 440),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
      _waveform(waveform),
      _xlFrame(frame),
      _stemName(stemName),
      _debounceTimer(this, ID_DEBOUNCE_TIMER)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Frequency band preset
    wxBoxSizer* presetSizer = new wxBoxSizer(wxHORIZONTAL);
    presetSizer->Add(new wxStaticText(this, wxID_ANY, "Frequency Band:"), 0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    _presetChoice = new wxChoice(this, ID_CHOICE_PRESET);
    for (int i = 0; i < NUM_PRESETS; i++) {
        if (i == CUSTOM_PRESET_INDEX) {
            _presetChoice->Append("Custom");
        } else {
            wxString label = wxString::Format("%s (%s - %s)",
                PRESETS[i].name.c_str(),
                FormatFrequency(PRESETS[i].lowHz),
                FormatFrequency(PRESETS[i].highHz));
            _presetChoice->Append(label);
        }
    }
    _presetChoice->SetSelection(0);
    _presetChoice->Bind(wxEVT_CHOICE, &StemOnsetDialog::OnPresetChanged, this);
    presetSizer->Add(_presetChoice, 1, wxEXPAND, 0);
    mainSizer->Add(presetSizer, 0, wxEXPAND | wxALL, 8);

    // Low frequency slider (log scale)
    wxBoxSizer* lowFreqSizer = new wxBoxSizer(wxHORIZONTAL);
    lowFreqSizer->Add(new wxStaticText(this, wxID_ANY, "Low Freq:"), 0,
                      wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    int initLowSlider = FreqToSlider(PRESETS[0].lowHz);
    _lowFreqSlider = new wxSlider(this, ID_SLIDER_LOW_FREQ, initLowSlider, 0, 1000,
                                   wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    _lowFreqSlider->Bind(wxEVT_SLIDER, &StemOnsetDialog::OnLowFreqChanged, this);
    lowFreqSizer->Add(_lowFreqSlider, 1, wxEXPAND | wxRIGHT, 5);
    _lowFreqLabel = new wxStaticText(this, wxID_ANY, FormatFrequency(PRESETS[0].lowHz));
    _lowFreqLabel->SetMinSize(wxSize(65, -1));
    lowFreqSizer->Add(_lowFreqLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
    mainSizer->Add(lowFreqSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

    // High frequency slider (log scale)
    wxBoxSizer* highFreqSizer = new wxBoxSizer(wxHORIZONTAL);
    highFreqSizer->Add(new wxStaticText(this, wxID_ANY, "High Freq:"), 0,
                       wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    int initHighSlider = FreqToSlider(PRESETS[0].highHz);
    _highFreqSlider = new wxSlider(this, ID_SLIDER_HIGH_FREQ, initHighSlider, 0, 1000,
                                    wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    _highFreqSlider->Bind(wxEVT_SLIDER, &StemOnsetDialog::OnHighFreqChanged, this);
    highFreqSizer->Add(_highFreqSlider, 1, wxEXPAND | wxRIGHT, 5);
    _highFreqLabel = new wxStaticText(this, wxID_ANY, FormatFrequency(PRESETS[0].highHz));
    _highFreqLabel->SetMinSize(wxSize(65, -1));
    highFreqSizer->Add(_highFreqLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
    mainSizer->Add(highFreqSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Threshold slider
    wxBoxSizer* threshSizer = new wxBoxSizer(wxHORIZONTAL);
    threshSizer->Add(new wxStaticText(this, wxID_ANY, "Sensitivity:"), 0,
                     wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    int initThresh = (int)(PRESETS[0].defaultThreshold * 100.0f);
    _thresholdSlider = new wxSlider(this, ID_SLIDER_THRESHOLD, initThresh, 0, 100,
                                     wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    _thresholdSlider->Bind(wxEVT_SLIDER, &StemOnsetDialog::OnThresholdChanged, this);
    threshSizer->Add(_thresholdSlider, 1, wxEXPAND | wxRIGHT, 5);
    _thresholdLabel = new wxStaticText(this, wxID_ANY, wxString::Format("%.2f", PRESETS[0].defaultThreshold));
    _thresholdLabel->SetMinSize(wxSize(40, -1));
    threshSizer->Add(_thresholdLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
    mainSizer->Add(threshSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Minimum interval slider
    wxBoxSizer* intervalSizer = new wxBoxSizer(wxHORIZONTAL);
    intervalSizer->Add(new wxStaticText(this, wxID_ANY, "Min Interval:"), 0,
                       wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    int initInterval = PRESETS[0].defaultMinIntervalMS;
    _minIntervalSlider = new wxSlider(this, ID_SLIDER_MIN_INTERVAL, initInterval, 10, 500,
                                       wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
    _minIntervalSlider->Bind(wxEVT_SLIDER, &StemOnsetDialog::OnMinIntervalChanged, this);
    intervalSizer->Add(_minIntervalSlider, 1, wxEXPAND | wxRIGHT, 5);
    _minIntervalLabel = new wxStaticText(this, wxID_ANY, wxString::Format("%d ms", initInterval));
    _minIntervalLabel->SetMinSize(wxSize(55, -1));
    intervalSizer->Add(_minIntervalLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
    mainSizer->Add(intervalSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Timing track name
    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    nameSizer->Add(new wxStaticText(this, wxID_ANY, "Track Name:"), 0,
                   wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    _trackNameCtrl = new wxTextCtrl(this, ID_TEXT_TRACK_NAME,
                                     stemName + " - " + PRESETS[0].name);
    nameSizer->Add(_trackNameCtrl, 1, wxEXPAND, 0);
    mainSizer->Add(nameSizer, 0, wxEXPAND | wxALL, 8);

    // Status text
    _statusText = new wxStaticText(this, wxID_ANY, "Detecting onsets...");
    _statusText->SetForegroundColour(wxColour(100, 100, 100));
    mainSizer->Add(_statusText, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

    mainSizer->AddStretchSpacer(1);

    // Buttons (no Detect button - auto-detects)
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    _createButton = new wxButton(this, ID_BTN_CREATE, "Create Timing Track");
    _createButton->Enable(false);
    _cancelButton = new wxButton(this, ID_BTN_CANCEL, "Cancel");
    buttonSizer->Add(_createButton, 0, wxRIGHT, 5);
    buttonSizer->AddStretchSpacer(1);
    buttonSizer->Add(_cancelButton, 0, 0, 0);
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 8);

    SetSizer(mainSizer);
    SetMinSize(wxSize(380, 380));

    // Auto-detect on open
    CallAfter([this]() { RunOnsetDetection(); });
}

StemOnsetDialog::~StemOnsetDialog()
{
    _debounceTimer.Stop();
    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
        _waveform->Refresh(false);
    }
}

void StemOnsetDialog::ApplyBiquadBandpass(const float* input, float* output, long numSamples,
                                           int sampleRate, float lowHz, float highHz)
{
    float centerFreq = std::sqrt(lowHz * highHz);
    float bandwidth = highHz - lowHz;
    if (bandwidth <= 0) bandwidth = 100.0f;

    float w0 = 2.0f * M_PI * centerFreq / (float)sampleRate;
    float sinW0 = std::sin(w0);
    float cosW0 = std::cos(w0);
    float alpha = sinW0 * std::sinh(std::log(2.0f) / 2.0f * (bandwidth / centerFreq) * (w0 / sinW0));

    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cosW0;
    float a2 = 1.0f - alpha;

    // Normalize
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;

    for (long i = 0; i < numSamples; i++) {
        float x0 = input[i];
        float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        output[i] = y0;
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
    }
}

void StemOnsetDialog::RunOnsetDetection()
{
    AudioManager* media = _waveform->GetMedia();
    if (media == nullptr) {
        _statusText->SetLabel("No audio data available.");
        return;
    }

    long trackSize = media->GetTrackSize();
    long sampleRate = media->GetRate();
    if (trackSize <= 0 || sampleRate <= 0) {
        _statusText->SetLabel("Invalid audio data.");
        return;
    }

    SetCursor(wxCURSOR_WAIT);
    _statusText->SetLabel("Analyzing audio...");
    wxYield();

    // Determine frequency band from sliders (not presets, since Custom may be active)
    float lowHz = SliderToFreq(_lowFreqSlider->GetValue());
    float highHz = SliderToFreq(_highFreqSlider->GetValue());
    if (lowHz > highHz) std::swap(lowHz, highHz);

    // Determine onset method from current preset
    int presetIdx = _presetChoice->GetSelection();
    if (presetIdx < 0 || presetIdx >= NUM_PRESETS) presetIdx = 0;
    std::string onsetMethod = PRESETS[presetIdx].onsetMethod;

    // Skip bandpass filter for full-range (20-20000 Hz is effectively no-op)
    bool needFilter = (lowHz > 25.0f || highHz < 19000.0f);

    // Get pointer to raw audio samples
    float* rawPtr = media->GetRawLeftDataPtr(0);
    if (rawPtr == nullptr) {
        _statusText->SetLabel("Failed to access audio data.");
        SetCursor(wxCURSOR_ARROW);
        return;
    }

    std::vector<float> audioData(rawPtr, rawPtr + trackSize);

    // Apply bandpass filter if a frequency band is selected
    std::vector<float> filteredData;
    const float* analysisData = audioData.data();
    if (needFilter) {
        filteredData.resize(trackSize);
        ApplyBiquadBandpass(audioData.data(), filteredData.data(), trackSize,
                            (int)sampleRate, lowHz, highHz);
        analysisData = filteredData.data();
    }

    // Choose window/hop sizes based on sample rate
    uint_t winSize = (sampleRate > 48000) ? 2048 : 1024;
    uint_t hopSize = winSize / 2;

    int numFrames = (int)((trackSize - winSize) / hopSize);
    if (numFrames <= 0) {
        _statusText->SetLabel("Audio too short for onset detection.");
        SetCursor(wxCURSOR_ARROW);
        return;
    }

    // Create aubio objects
    aubio_pvoc_t* pv = new_aubio_pvoc(winSize, hopSize);
    aubio_specdesc_t* sd = new_aubio_specdesc(onsetMethod.c_str(), winSize);
    fvec_t* inputVec = new_fvec(hopSize);
    cvec_t* grain = new_cvec(winSize);
    fvec_t* onsetOut = new_fvec(1);

    if (pv == nullptr || sd == nullptr) {
        spdlog::error("Failed to create aubio phase vocoder or spectral descriptor");
        _statusText->SetLabel("Onset detection failed (aubio init error).");
        if (pv) del_aubio_pvoc(pv);
        if (sd) del_aubio_specdesc(sd);
        if (inputVec) del_fvec(inputVec);
        if (grain) del_cvec(grain);
        if (onsetOut) del_fvec(onsetOut);
        SetCursor(wxCURSOR_ARROW);
        return;
    }

    // Process audio in hops through phase vocoder + spectral descriptor
    _detectionFunction.clear();
    _detectionFunction.reserve(numFrames);

    for (int f = 0; f < numFrames; f++) {
        long offset = (long)f * hopSize;
        // Copy hop of audio into aubio input vector
        for (uint_t s = 0; s < hopSize && (offset + s) < trackSize; s++) {
            inputVec->data[s] = analysisData[offset + s];
        }
        // Phase vocoder: time domain -> spectral domain
        aubio_pvoc_do(pv, inputVec, grain);
        // Spectral descriptor: compute onset detection function value
        aubio_specdesc_do(sd, grain, onsetOut);
        _detectionFunction.push_back(onsetOut->data[0]);
    }

    // Clean up aubio objects
    del_fvec(onsetOut);
    del_cvec(grain);
    del_fvec(inputVec);
    del_aubio_specdesc(sd);
    del_aubio_pvoc(pv);

    _detectionFunctionHopSec = (float)hopSize / (float)sampleRate;
    _detected = true;

    spdlog::debug("Onset detection complete: {} frames, hop={:.3f}s, method={}",
                  numFrames, _detectionFunctionHopSec, onsetMethod);

    SetCursor(wxCURSOR_ARROW);
    ApplyThreshold();
}

void StemOnsetDialog::ApplyThreshold()
{
    if (!_detected || _detectionFunction.empty()) return;

    float threshold = (float)_thresholdSlider->GetValue() / 100.0f;
    int minIntervalMS = _minIntervalSlider->GetValue();
    float minIntervalSec = (float)minIntervalMS / 1000.0f;

    // Normalize the detection function
    float maxVal = *std::max_element(_detectionFunction.begin(), _detectionFunction.end());
    if (maxVal <= 0.0f) {
        _onsetTimesMS.clear();
        UpdatePreviewMarkers();
        UpdateStatusText();
        return;
    }

    _onsetTimesMS.clear();

    int minFrameGap = (int)(minIntervalSec / _detectionFunctionHopSec);
    if (minFrameGap < 1) minFrameGap = 1;

    float absThreshold = threshold * maxVal;
    int lastOnsetFrame = -minFrameGap - 1;

    for (int f = 1; f < (int)_detectionFunction.size() - 1; f++) {
        // Peak picking: local maximum above absolute threshold with min gap
        if (_detectionFunction[f] > absThreshold &&
            _detectionFunction[f] >= _detectionFunction[f - 1] &&
            _detectionFunction[f] >= _detectionFunction[f + 1] &&
            (f - lastOnsetFrame) >= minFrameGap) {

            float timeSec = (float)f * _detectionFunctionHopSec;
            int timeMS = (int)(timeSec * 1000.0f);
            _onsetTimesMS.push_back(timeMS);
            lastOnsetFrame = f;
        }
    }

    UpdatePreviewMarkers();
    UpdateStatusText();
}

void StemOnsetDialog::UpdatePreviewMarkers()
{
    if (_waveform != nullptr) {
        _waveform->SetOnsetMarkers(_onsetTimesMS);
        _waveform->render();
    }
}

void StemOnsetDialog::UpdateStatusText()
{
    _statusText->SetLabel(wxString::Format("Detected %d onsets.", (int)_onsetTimesMS.size()));
    _createButton->Enable(!_onsetTimesMS.empty());
}

void StemOnsetDialog::OnCreateTimingTrack(wxCommandEvent& event)
{
    if (_onsetTimesMS.empty() || _xlFrame == nullptr) return;

    std::string trackName = _trackNameCtrl->GetValue().ToStdString();
    if (trackName.empty()) {
        wxMessageBox("Please enter a timing track name.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    int frameMS = 50;
    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        frameMS = xLightsFrame::CurrentSeqXmlFile->GetFrameMS();
    }
    double frequency = 1000.0 / frameMS;

    TimingElement* element = _xlFrame->AddTimingElement(trackName);
    if (element == nullptr) return;

    EffectLayer* effectLayer = element->GetEffectLayer(0);
    if (effectLayer == nullptr) {
        effectLayer = element->AddEffectLayer();
    }

    for (size_t i = 0; i < _onsetTimesMS.size(); i++) {
        int startMS = TimeLine::RoundToMultipleOfPeriod(_onsetTimesMS[i], frequency);
        int endMS;
        if (i + 1 < _onsetTimesMS.size()) {
            endMS = TimeLine::RoundToMultipleOfPeriod(_onsetTimesMS[i + 1], frequency);
        } else {
            int seqEnd = 0;
            if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
                seqEnd = xLightsFrame::CurrentSeqXmlFile->GetSequenceDurationMS();
            }
            endMS = (seqEnd > startMS) ? seqEnd : startMS + frameMS;
        }
        if (endMS <= startMS) {
            endMS = startMS + frameMS;
        }
        effectLayer->AddEffect(0, "", "", "", startMS, endMS, EFFECT_NOT_SELECTED, false);
    }

    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(_xlFrame, eventRowHeaderChanged);

    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
        _waveform->Refresh(false);
    }

    _statusText->SetLabel(wxString::Format("Created timing track '%s' with %d marks.",
                                            trackName, (int)_onsetTimesMS.size()));

    Destroy();
}

void StemOnsetDialog::OnCancel(wxCommandEvent& event)
{
    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
        _waveform->Refresh(false);
    }
    Destroy();
}

void StemOnsetDialog::OnClose(wxCloseEvent& event)
{
    _debounceTimer.Stop();
    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
        _waveform->Refresh(false);
    }
    Destroy();
}

void StemOnsetDialog::OnThresholdChanged(wxCommandEvent& event)
{
    float val = (float)_thresholdSlider->GetValue() / 100.0f;
    _thresholdLabel->SetLabel(wxString::Format("%.2f", val));
    if (_detected) {
        ApplyThreshold();
    }
}

void StemOnsetDialog::OnMinIntervalChanged(wxCommandEvent& event)
{
    int val = _minIntervalSlider->GetValue();
    _minIntervalLabel->SetLabel(wxString::Format("%d ms", val));
    if (_detected) {
        ApplyThreshold();
    }
}

void StemOnsetDialog::OnPresetChanged(wxCommandEvent& event)
{
    int idx = _presetChoice->GetSelection();
    if (idx < 0 || idx >= NUM_PRESETS) return;

    // Update frequency sliders to preset values
    _lowFreqSlider->SetValue(FreqToSlider(PRESETS[idx].lowHz));
    _lowFreqLabel->SetLabel(FormatFrequency(PRESETS[idx].lowHz));
    _highFreqSlider->SetValue(FreqToSlider(PRESETS[idx].highHz));
    _highFreqLabel->SetLabel(FormatFrequency(PRESETS[idx].highHz));

    // Apply preset defaults for threshold and interval
    int threshVal = (int)(PRESETS[idx].defaultThreshold * 100.0f);
    _thresholdSlider->SetValue(threshVal);
    _thresholdLabel->SetLabel(wxString::Format("%.2f", PRESETS[idx].defaultThreshold));

    _minIntervalSlider->SetValue(PRESETS[idx].defaultMinIntervalMS);
    _minIntervalLabel->SetLabel(wxString::Format("%d ms", PRESETS[idx].defaultMinIntervalMS));

    // Update track name
    _trackNameCtrl->SetValue(_stemName + " - " + PRESETS[idx].name);

    // Clear previous detection and re-detect with new preset
    _detected = false;
    _detectionFunction.clear();
    _onsetTimesMS.clear();
    _createButton->Enable(false);
    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
    }

    RunOnsetDetection();
}

void StemOnsetDialog::OnLowFreqChanged(wxCommandEvent& event)
{
    float hz = SliderToFreq(_lowFreqSlider->GetValue());
    _lowFreqLabel->SetLabel(FormatFrequency(hz));

    // Ensure low doesn't exceed high
    float highHz = SliderToFreq(_highFreqSlider->GetValue());
    if (hz > highHz) {
        _highFreqSlider->SetValue(_lowFreqSlider->GetValue());
        _highFreqLabel->SetLabel(FormatFrequency(hz));
    }

    // Switch to Custom preset if not already
    if (_presetChoice->GetSelection() != CUSTOM_PRESET_INDEX) {
        _presetChoice->SetSelection(CUSTOM_PRESET_INDEX);
        _trackNameCtrl->SetValue(_stemName + " - Custom");
    }

    // Debounce: restart timer for stage 1 re-detection
    _debounceTimer.Start(150, wxTIMER_ONE_SHOT);
}

void StemOnsetDialog::OnHighFreqChanged(wxCommandEvent& event)
{
    float hz = SliderToFreq(_highFreqSlider->GetValue());
    _highFreqLabel->SetLabel(FormatFrequency(hz));

    // Ensure high doesn't go below low
    float lowHz = SliderToFreq(_lowFreqSlider->GetValue());
    if (hz < lowHz) {
        _lowFreqSlider->SetValue(_highFreqSlider->GetValue());
        _lowFreqLabel->SetLabel(FormatFrequency(hz));
    }

    // Switch to Custom preset if not already
    if (_presetChoice->GetSelection() != CUSTOM_PRESET_INDEX) {
        _presetChoice->SetSelection(CUSTOM_PRESET_INDEX);
        _trackNameCtrl->SetValue(_stemName + " - Custom");
    }

    // Debounce: restart timer for stage 1 re-detection
    _debounceTimer.Start(150, wxTIMER_ONE_SHOT);
}

void StemOnsetDialog::OnDebounceTimer(wxTimerEvent& event)
{
    _detected = false;
    _detectionFunction.clear();
    _onsetTimesMS.clear();
    _createButton->Enable(false);
    if (_waveform != nullptr) {
        _waveform->ClearOnsetMarkers();
    }

    RunOnsetDetection();
}
