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

//(*InternalHeaders(OutputSettingsPanel)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/valtext.h>

#include <algorithm>

#include <wx/preferences.h>
#include "xLightsMain.h"
#include "utils/ip_utils.h"

//(*IdInit(OutputSettingsPanel)
const long OutputSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long OutputSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE1 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE2 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE3 = wxNewId();
const long OutputSettingsPanel::ID_TEXTCTRL1 = wxNewId();
const long OutputSettingsPanel::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OutputSettingsPanel,wxPanel)
	//(*EventTable(OutputSettingsPanel)
	//*)
END_EVENT_TABLE()

OutputSettingsPanel::OutputSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(OutputSettingsPanel)
	wxGridBagSizer* GridBagSizer1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	FrameSyncCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Use Frame Sync"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	FrameSyncCheckBox->SetValue(false);
	GridBagSizer1->Add(FrameSyncCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Force Local IP"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Duplicate Frames to Suppress"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("API Port"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText3, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ForceLocalIPChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ForceLocalIPChoice->SetSelection( ForceLocalIPChoice->Append(wxEmptyString) );
	GridBagSizer1->Add(ForceLocalIPChoice, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DuplicateSuppressChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	DuplicateSuppressChoice->SetSelection( DuplicateSuppressChoice->Append(_("None")) );
	DuplicateSuppressChoice->Append(_("10"));
	DuplicateSuppressChoice->Append(_("20"));
	DuplicateSuppressChoice->Append(_("40"));
	GridBagSizer1->Add(DuplicateSuppressChoice, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	xFadexScheduleChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	xFadexScheduleChoice->SetSelection( xFadexScheduleChoice->Append(_("Disabled")) );
	xFadexScheduleChoice->Append(_("Port A (49913)"));
	xFadexScheduleChoice->Append(_("Port B (49914)"));
	GridBagSizer1->Add(xFadexScheduleChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Default Max Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText4, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DefaultMaxBrightnessText = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS), _T("ID_TEXTCTRL1"));
	DefaultMaxBrightnessText->SetToolTip(_("Brightness (1-100) stamped on newly added controllers. Leave blank to keep new controllers at their own default (100)."));
	GridBagSizer1->Add(DefaultMaxBrightnessText, wxGBPosition(4, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ApplyBrightnessButton = new wxButton(this, ID_BUTTON1, _("Apply to all existing controllers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	GridBagSizer1->Add(ApplyBrightnessButton, wxGBPosition(5, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OutputSettingsPanel::OnFrameSyncCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnForceLocalIPChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnDuplicateSuppressChoiceSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnxFadexScheduleChoiceSelect);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OutputSettingsPanel::OnDefaultMaxBrightnessText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OutputSettingsPanel::OnApplyBrightnessButtonClick);
	//*)
    
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
    GridBagSizer1->Layout();
    GridBagSizer1->Fit(this);
    GridBagSizer1->SetSizeHints(this);
}

OutputSettingsPanel::~OutputSettingsPanel()
{
	//(*Destroy(OutputSettingsPanel)
	//*)
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
    // Blank == unset (0); any entered value is constrained to 1..100.
    int b = 0;
    wxString bstr = DefaultMaxBrightnessText->GetValue();
    bstr.Trim(true).Trim(false);
    long v = 0;
    if (!bstr.IsEmpty() && bstr.ToLong(&v)) {
        b = std::clamp<int>((int)v, 1, 100);
    }
    frame->SetDefaultMaxBrightness(b);
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
    const int b = frame->GetDefaultMaxBrightness();
    DefaultMaxBrightnessText->ChangeValue(b > 0 ? wxString::Format("%d", b) : wxString());
    ApplyBrightnessButton->Enable(b > 0);
    return true;
}

void OutputSettingsPanel::OnFrameSyncCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OutputSettingsPanel::OnForceLocalIPChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OutputSettingsPanel::OnDuplicateSuppressChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OutputSettingsPanel::OnxFadexScheduleChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OutputSettingsPanel::OnDefaultMaxBrightnessText(wxCommandEvent& event)
{
    // Enforce the 1..100 ceiling live: rewrite the field if the user exceeds 100.
    wxString s = DefaultMaxBrightnessText->GetValue();
    s.Trim(true).Trim(false);
    long v = 0;
    const bool parsed = !s.IsEmpty() && s.ToLong(&v);
    if (parsed && v > 100) {
        DefaultMaxBrightnessText->ChangeValue("100"); // ChangeValue does not re-fire EVT_TEXT
        DefaultMaxBrightnessText->SetInsertionPointEnd();
        v = 100;
    }

    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
    // Enable the apply button when the field holds a usable (>=1) value.
    ApplyBrightnessButton->Enable(parsed && v >= 1);
}

void OutputSettingsPanel::OnApplyBrightnessButtonClick(wxCommandEvent& event)
{
    // Commit + normalise the field (clamp to 1..100) before we apply it.
    TransferDataFromWindow();
    TransferDataToWindow();

    const int b = frame->GetDefaultMaxBrightness();
    if (b <= 0) {
        return;
    }
    if (wxMessageBox(wxString::Format(_("Set the Default Max Brightness of every existing controller that supports it to %d%%?\n\nThis cannot be undone."), b),
                     _("Apply to all existing controllers"), wxYES_NO | wxICON_QUESTION, this) != wxYES) {
        return;
    }
    frame->ApplyDefaultMaxBrightnessToAllControllers();
}
