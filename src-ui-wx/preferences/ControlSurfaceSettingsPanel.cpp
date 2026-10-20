/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControlSurfaceSettingsPanel.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/preferences.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "sequencer/ControlSurfaceManager.h"
#include "xLightsMain.h"

namespace {
const wxString SAME_AS_INPUT_LABEL = "(Same as input)";
const wxString NO_OUTPUT_LABEL = "(None)";
const wxString MISSING_SUFFIX = " (not found)";

const char* HELP_TEXT =
    "Supports Mackie Control (MCU) surfaces such as the Behringer X-Touch, iCON and Mackie MCU.\n"
    "Set the surface to MCU mode.\n"
    "\n"
    "Play: play / pause    Stop: stop, press again to go to start\n"
    "Rewind / Fast Fwd: move 10 seconds    Cycle: loop play\n"
    "Marker: add timing mark    Jog wheel: move the play position a frame at a time\n"
    "Scrub: hear audio while jogging    Zoom: arrows and jog wheel zoom the timeline\n"
    "Arrows: move the effect selection\n"
    "The timecode display shows the play position.";

wxString PortLabel(const std::string& port, const wxArrayString& available) {
    wxString label = wxString::FromUTF8(port);
    if (available.Index(label) == wxNOT_FOUND) {
        label += MISSING_SUFFIX;
    }
    return label;
}

std::string PortFromLabel(const wxString& label) {
    wxString port = label;
    if (port.EndsWith(MISSING_SUFFIX)) {
        port.RemoveLast(MISSING_SUFFIX.length());
    }
    return port.ToStdString(wxConvUTF8);
}
} // namespace

ControlSurfaceSettingsPanel::ControlSurfaceSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id) :
    wxPanel(parent, id), frame(f) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    CheckBox_Enable = new wxCheckBox(this, wxID_ANY, _("Enable Mackie Control (MCU) surface"));
    sizer->Add(CheckBox_Enable, 0, wxALL, 5);

    auto* grid = new wxFlexGridSizer(0, 3, 0, 0);
    grid->AddGrowableCol(1);
    grid->Add(new wxStaticText(this, wxID_ANY, _("MIDI Input")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Input = new wxChoice(this, wxID_ANY);
    grid->Add(Choice_Input, 1, wxALL | wxEXPAND, 5);
    Button_Refresh = new wxButton(this, wxID_ANY, _("Refresh"));
    grid->Add(Button_Refresh, 0, wxALL, 5);
    grid->Add(new wxStaticText(this, wxID_ANY, _("MIDI Output")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_Output = new wxChoice(this, wxID_ANY);
    grid->Add(Choice_Output, 1, wxALL | wxEXPAND, 5);
    grid->AddSpacer(0);
    sizer->Add(grid, 0, wxEXPAND);

    StaticText_Status = new wxStaticText(this, wxID_ANY, wxEmptyString);
    sizer->Add(StaticText_Status, 0, wxALL, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, HELP_TEXT), 0, wxALL, 5);

    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);

    CheckBox_Enable->Bind(wxEVT_CHECKBOX, &ControlSurfaceSettingsPanel::OnChanged, this);
    Choice_Input->Bind(wxEVT_CHOICE, &ControlSurfaceSettingsPanel::OnChanged, this);
    Choice_Output->Bind(wxEVT_CHOICE, &ControlSurfaceSettingsPanel::OnChanged, this);
    Button_Refresh->Bind(wxEVT_BUTTON, &ControlSurfaceSettingsPanel::OnRefresh, this);

    TransferDataToWindow();
}

void ControlSurfaceSettingsPanel::PopulatePorts(const std::string& inPort, const std::string& outPort) {
    wxArrayString inputs;
    for (const auto& p : ControlSurfaceManager::ListInputPorts()) {
        inputs.Add(wxString::FromUTF8(p));
    }
    wxArrayString outputs;
    for (const auto& p : ControlSurfaceManager::ListOutputPorts()) {
        outputs.Add(wxString::FromUTF8(p));
    }

    Choice_Input->Clear();
    Choice_Input->Append(inputs);
    if (!inPort.empty()) {
        wxString label = PortLabel(inPort, inputs);
        if (Choice_Input->FindString(label) == wxNOT_FOUND) {
            Choice_Input->Append(label);
        }
        Choice_Input->SetStringSelection(label);
    } else if (!inputs.IsEmpty()) {
        Choice_Input->SetSelection(0);
    }

    Choice_Output->Clear();
    Choice_Output->Append(SAME_AS_INPUT_LABEL);
    Choice_Output->Append(NO_OUTPUT_LABEL);
    Choice_Output->Append(outputs);
    if (outPort == ControlSurfaceManager::SAME_AS_INPUT) {
        Choice_Output->SetSelection(0);
    } else if (outPort == ControlSurfaceManager::NO_OUTPUT) {
        Choice_Output->SetSelection(1);
    } else {
        wxString label = PortLabel(outPort, outputs);
        if (Choice_Output->FindString(label) == wxNOT_FOUND) {
            Choice_Output->Append(label);
        }
        Choice_Output->SetStringSelection(label);
    }
}

void ControlSurfaceSettingsPanel::GetSelectedPorts(std::string& inPort, std::string& outPort) const {
    inPort.clear();
    if (Choice_Input->GetSelection() != wxNOT_FOUND) {
        inPort = PortFromLabel(Choice_Input->GetStringSelection());
    }
    outPort = ControlSurfaceManager::SAME_AS_INPUT;
    int outSel = Choice_Output->GetSelection();
    if (outSel == 1) {
        outPort = ControlSurfaceManager::NO_OUTPUT;
    } else if (outSel > 1) {
        outPort = PortFromLabel(Choice_Output->GetStringSelection());
    }
}

void ControlSurfaceSettingsPanel::UpdateStatus() {
    auto* cs = frame->GetControlSurfaceManager();
    wxString status;
    if (cs == nullptr || !cs->IsEnabled()) {
        status = _("Status: disabled");
    } else if (cs->IsConnected()) {
        status = _("Status: connected");
    } else {
        status = _("Status: waiting for the surface to be connected");
    }
    StaticText_Status->SetLabel(status);
}

void ControlSurfaceSettingsPanel::UpdateEnabledState() {
    bool enabled = CheckBox_Enable->IsChecked();
    Choice_Input->Enable(enabled);
    Choice_Output->Enable(enabled);
    Button_Refresh->Enable(enabled);
}

bool ControlSurfaceSettingsPanel::TransferDataToWindow() {
    auto* cs = frame->GetControlSurfaceManager();
    if (cs == nullptr) {
        return true;
    }
    CheckBox_Enable->SetValue(cs->IsEnabled());
    PopulatePorts(cs->GetInputPort(), cs->GetOutputPort());
    UpdateEnabledState();
    UpdateStatus();
    return true;
}

bool ControlSurfaceSettingsPanel::TransferDataFromWindow() {
    auto* cs = frame->GetControlSurfaceManager();
    if (cs == nullptr) {
        return true;
    }
    std::string inPort;
    std::string outPort;
    GetSelectedPorts(inPort, outPort);
    cs->Configure(CheckBox_Enable->IsChecked(), inPort, outPort);
    UpdateStatus();
    return true;
}

void ControlSurfaceSettingsPanel::OnChanged(wxCommandEvent& event) {
    UpdateEnabledState();
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void ControlSurfaceSettingsPanel::OnRefresh(wxCommandEvent& event) {
    std::string inPort;
    std::string outPort;
    GetSelectedPorts(inPort, outPort);
    PopulatePorts(inPort, outPort);
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
