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

#include <wx/wx.h>
#include <wx/timer.h>
#include <vector>
#include <string>

class StemWaveform;
class xLightsFrame;

struct FrequencyBandPreset {
    std::string name;
    float lowHz;
    float highHz;
    float defaultThreshold;
    int defaultMinIntervalMS;
    std::string onsetMethod;
};

class StemOnsetDialog : public wxDialog
{
public:
    StemOnsetDialog(wxWindow* parent, StemWaveform* waveform,
                    xLightsFrame* frame, const std::string& stemName);
    virtual ~StemOnsetDialog();

    const std::vector<int>& GetOnsetTimesMS() const { return _onsetTimesMS; }

private:
    void OnCreateTimingTrack(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnThresholdChanged(wxCommandEvent& event);
    void OnMinIntervalChanged(wxCommandEvent& event);
    void OnPresetChanged(wxCommandEvent& event);
    void OnLowFreqChanged(wxCommandEvent& event);
    void OnHighFreqChanged(wxCommandEvent& event);
    void OnDebounceTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);

    void RunOnsetDetection();
    void ApplyThreshold();
    void UpdatePreviewMarkers();
    void UpdateStatusText();

    void ApplyBiquadBandpass(const float* input, float* output, long numSamples,
                             int sampleRate, float lowHz, float highHz);

    static float SliderToFreq(int val);
    static int FreqToSlider(float hz);
    static wxString FormatFrequency(float hz);

    StemWaveform* _waveform = nullptr;
    xLightsFrame* _xlFrame = nullptr;
    std::string _stemName;

    wxChoice* _presetChoice = nullptr;
    wxSlider* _lowFreqSlider = nullptr;
    wxStaticText* _lowFreqLabel = nullptr;
    wxSlider* _highFreqSlider = nullptr;
    wxStaticText* _highFreqLabel = nullptr;
    wxSlider* _thresholdSlider = nullptr;
    wxStaticText* _thresholdLabel = nullptr;
    wxSlider* _minIntervalSlider = nullptr;
    wxStaticText* _minIntervalLabel = nullptr;
    wxTextCtrl* _trackNameCtrl = nullptr;
    wxButton* _createButton = nullptr;
    wxButton* _cancelButton = nullptr;
    wxStaticText* _statusText = nullptr;

    wxTimer _debounceTimer;

    // Cached detection function from stage 1 (aubio spectral descriptor output)
    std::vector<float> _detectionFunction;
    float _detectionFunctionHopSec = 0.0f;

    // Final onset times after thresholding (stage 2)
    std::vector<int> _onsetTimesMS;

    bool _detected = false;

    static const FrequencyBandPreset PRESETS[];
    static const int NUM_PRESETS;
    static const int CUSTOM_PRESET_INDEX;

    static const long ID_CHOICE_PRESET;
    static const long ID_SLIDER_LOW_FREQ;
    static const long ID_SLIDER_HIGH_FREQ;
    static const long ID_SLIDER_THRESHOLD;
    static const long ID_SLIDER_MIN_INTERVAL;
    static const long ID_TEXT_TRACK_NAME;
    static const long ID_BTN_CREATE;
    static const long ID_BTN_CANCEL;
    static const long ID_DEBOUNCE_TIMER;

    DECLARE_EVENT_TABLE()
};
