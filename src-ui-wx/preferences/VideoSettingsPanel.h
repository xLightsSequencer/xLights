#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>

class wxCheckBox;
class wxChoice;
class wxCommandEvent;
class wxSpinCtrlDouble;
class wxSpinDoubleEvent;
class xLightsFrame;

class VideoSettingsPanel : public wxPanel
{
public:
    VideoSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~VideoSettingsPanel() = default;

    virtual bool TransferDataFromWindow() override;
    virtual bool TransferDataToWindow() override;

private:
    xLightsFrame* frame = nullptr;
    wxCheckBox* HardwareVideoDecodingCheckBox = nullptr;
    wxChoice* HardwareVideoRenderChoice = nullptr;
    wxChoice* ChoiceCodec = nullptr;
    wxSpinCtrlDouble* SpinCtrlDoubleBitrate = nullptr;

    // Mirror the original behaviour: write changes back immediately on
    // platforms where the preferences editor applies as-you-go.
    void ApplyIfImmediate();
    void OnControlChanged(wxCommandEvent& event);
    void OnBitrateChanged(wxSpinDoubleEvent& event);
};
