#include "OutputSettingsPanel.h"

//(*InternalHeaders(OutputSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"
#include "../UtilFunctions.h"

//(*IdInit(OutputSettingsPanel)
const long OutputSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long OutputSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE1 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE2 = wxNewId();
const long OutputSettingsPanel::ID_CHOICE3 = wxNewId();
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

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	FrameSyncCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Use Frame Sync"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	FrameSyncCheckBox->SetValue(false);
	GridBagSizer1->Add(FrameSyncCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Force Local IP"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Duplicate Frames to Suppress"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("xFade/xSchedule"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
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
	xFadexScheduleChoice->Append(_("Port A"));
	xFadexScheduleChoice->Append(_("Port B"));
	GridBagSizer1->Add(xFadexScheduleChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&OutputSettingsPanel::OnFrameSyncCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnForceLocalIPChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnDuplicateSuppressChoiceSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&OutputSettingsPanel::OnxFadexScheduleChoiceSelect);
	//*)
    
    std::string localIP = frame->LocalIP();
    auto ips = GetLocalIPs();

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
    frame->SetLocalIP(ForceLocalIPChoice->GetStringSelection());
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
