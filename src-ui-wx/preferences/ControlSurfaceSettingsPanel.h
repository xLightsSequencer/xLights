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

#include <wx/panel.h>

#include <string>

class wxButton;
class wxCheckBox;
class wxChoice;
class wxStaticText;
class xLightsFrame;

class ControlSurfaceSettingsPanel : public wxPanel {
public:
    ControlSurfaceSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY);

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

private:
    void PopulatePorts(const std::string& inPort, const std::string& outPort);
    void GetSelectedPorts(std::string& inPort, std::string& outPort) const;
    void UpdateStatus();
    void UpdateEnabledState();
    void OnChanged(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);

    xLightsFrame* frame;
    wxCheckBox* CheckBox_Enable = nullptr;
    wxChoice* Choice_Input = nullptr;
    wxChoice* Choice_Output = nullptr;
    wxButton* Button_Refresh = nullptr;
    wxStaticText* StaticText_Status = nullptr;
};
