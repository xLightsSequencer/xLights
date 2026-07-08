/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputSettingsPanel.h"
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include <wx/preferences.h>
#include "xLightsMain.h"
#include "utils/ip_utils.h"

OutputSettingsPanel::OutputSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    FrameSyncCheckBox = new wxCheckBox(this, wxID_ANY, _("Use Frame Sync"));
    sizer->Add(FrameSyncCheckBox, 0, wxLEFT | wxTOP, 8);
    sizer->Add(MakePreferenceHint(this, _("Send an E1.31 sync packet each frame so multiple controllers stay in step.")), 0, wxLEFT | wxBOTTOM, 26);

    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Force Local IP")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    ForceLocalIPChoice = new wxChoice(this, wxID_ANY);
    ForceLocalIPChoice->SetSelection(ForceLocalIPChoice->Append(wxEmptyString));
    grid->Add(ForceLocalIPChoice, 1, wxALL | wxEXPAND, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("Send all network output from a specific local adapter (leave blank to auto-select).")), 0, wxLEFT | wxBOTTOM, 5);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Duplicate Frames to Suppress")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    DuplicateSuppressChoice = new wxChoice(this, wxID_ANY);
    DuplicateSuppressChoice->SetSelection(DuplicateSuppressChoice->Append(_("None")));
    DuplicateSuppressChoice->Append(_("10"));
    DuplicateSuppressChoice->Append(_("20"));
    DuplicateSuppressChoice->Append(_("40"));
    grid->Add(DuplicateSuppressChoice, 1, wxALL | wxEXPAND, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("Stop resending unchanged frames after this many duplicates to reduce network traffic.")), 0, wxLEFT | wxBOTTOM, 5);

    grid->Add(new wxStaticText(this, wxID_ANY, _("xFade/xSchedule")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    xFadexScheduleChoice = new wxChoice(this, wxID_ANY);
    xFadexScheduleChoice->SetSelection(xFadexScheduleChoice->Append(_("Disabled")));
    xFadexScheduleChoice->Append(_("Port A"));
    xFadexScheduleChoice->Append(_("Port B"));
    grid->Add(xFadexScheduleChoice, 1, wxALL | wxEXPAND, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("Share this show with xFade/xSchedule on the chosen sync port.")), 0, wxLEFT | wxBOTTOM, 5);

    sizer->Add(grid, 0, wxEXPAND | wxALL, 5);

    SetSizer(sizer);
    sizer->SetSizeHints(this);

    std::string localIP = frame->_outputManager.GetGlobalForceLocalIP();
    auto ips = ip_utils::GetLocalIPs();

    if (ips.size() == 0) {
        if (localIP != "") {
           localIP = "";
        }
    }
    wxArrayString choices;
    choices.push_back("");
    int sel = -1;
    int i = 0;
    for (auto it: ips) {
        if (it == localIP) {
            sel = i+1;
        }
        i++;
        choices.push_back(it);
    }
    if (sel == -1) {
        sel = 0;
    }
    ForceLocalIPChoice->Set(choices);
    ForceLocalIPChoice->SetSelection(sel);

    FrameSyncCheckBox->Bind(wxEVT_CHECKBOX, &OutputSettingsPanel::OnChanged, this);
    ForceLocalIPChoice->Bind(wxEVT_CHOICE, &OutputSettingsPanel::OnChanged, this);
    DuplicateSuppressChoice->Bind(wxEVT_CHOICE, &OutputSettingsPanel::OnChanged, this);
    xFadexScheduleChoice->Bind(wxEVT_CHOICE, &OutputSettingsPanel::OnChanged, this);
}

OutputSettingsPanel::~OutputSettingsPanel()
{
}

bool OutputSettingsPanel::TransferDataFromWindow() {
    frame->SetXFadePort(xFadexScheduleChoice->GetSelection());
    frame->SetE131Sync(FrameSyncCheckBox->IsChecked());
    frame->_outputManager.SetGlobalForceLocalIP(ForceLocalIPChoice->GetStringSelection());
    switch (DuplicateSuppressChoice->GetSelection()) {
        case 3:
            frame->SetSuppressDuplicateFrames(40);
            break;
        case 2:
            frame->SetSuppressDuplicateFrames(20);
            break;
        case 1:
            frame->SetSuppressDuplicateFrames(10);
            break;
        default:
            frame->SetSuppressDuplicateFrames(0);
            break;
    }
    return true;
}
bool OutputSettingsPanel::TransferDataToWindow() {
    xFadexScheduleChoice->SetSelection(frame->XFadePort());
    FrameSyncCheckBox->SetValue(frame->E131Sync());
    switch (frame->SuppressDuplicateFrames()) {
        case 40:
            DuplicateSuppressChoice->SetSelection(3);
            break;
        case 20:
            DuplicateSuppressChoice->SetSelection(2);
            break;
        case 10:
            DuplicateSuppressChoice->SetSelection(1);
            break;
        default:
            DuplicateSuppressChoice->SetSelection(0);
            break;
    }
    return true;
}

void OutputSettingsPanel::OnChanged(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
