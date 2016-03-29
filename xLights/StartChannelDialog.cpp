#include "StartChannelDialog.h"

//(*InternalHeaders(StartChannelDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(StartChannelDialog)
const long StartChannelDialog::ID_SPINCTRL1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON2 = wxNewId();
const long StartChannelDialog::ID_SPINCTRL2 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON3 = wxNewId();
const long StartChannelDialog::ID_CHOICE1 = wxNewId();
const long StartChannelDialog::ID_RADIOBUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(StartChannelDialog,wxDialog)
	//(*EventTable(StartChannelDialog)
	//*)
END_EVENT_TABLE()

#include "models/ModelManager.h"
#include "models/Model.h"

StartChannelDialog::StartChannelDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(StartChannelDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	StartChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 999999, 1, _T("ID_SPINCTRL1"));
	StartChannel->SetValue(_T("1"));
	FlexGridSizer2->Add(StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Offset From"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	NoneButton = new wxRadioButton(this, ID_RADIOBUTTON1, _("None (Absolute)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	NoneButton->SetValue(true);
	FlexGridSizer3->Add(NoneButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	OutputButton = new wxRadioButton(this, ID_RADIOBUTTON2, _("Output Number"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer3->Add(OutputButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	OutputSpin = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 9999, 1, _T("ID_SPINCTRL2"));
	OutputSpin->SetValue(_T("1"));
	FlexGridSizer3->Add(OutputSpin, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelButton = new wxRadioButton(this, ID_RADIOBUTTON3, _("End of Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	FlexGridSizer3->Add(ModelButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ModelChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ModelChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer3->Add(ModelChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StartModelButton = new wxRadioButton(this, ID_RADIOBUTTON4, _("Start of Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	FlexGridSizer3->Add(StartModelButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 3);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StartChannelDialog::OnButtonSelect);
	//*)
}

StartChannelDialog::~StartChannelDialog()
{
	//(*Destroy(StartChannelDialog)
	//*)
}
void StartChannelDialog::Set(const wxString &s, const ModelManager &models) {
    wxString start = s;
    wxArrayString  list;
    for (auto it = models.begin(); it != models.end(); it++) {
        if (it->second->GetDisplayAs() != "ModelGroup") {
            list.push_back(it->first);
        }
    }
    ModelChoice->Append(list);
    if (start.Contains(":")) {
        wxString sNet = start.SubString(0, start.Find(":")-1);
        if (sNet[0] == '@') {
            ModelChoice->Enable();
            StartModelButton->SetValue(true);
            OutputSpin->Disable();
            ModelChoice->SetStringSelection(sNet.SubString(1, sNet.size()));
        } else if (sNet[0] == '>' || sNet[0] == '<' || models[sNet.ToStdString()] != nullptr) {
            ModelChoice->Enable();
            ModelButton->SetValue(true);
            OutputSpin->Disable();
            ModelChoice->SetStringSelection(sNet[0] == '<' || sNet[0] == '>'  ? sNet.SubString(1, sNet.size()) : sNet);
        } else {
            OutputSpin->SetValue(sNet);
            ModelChoice->Disable();
            OutputSpin->Enable();
            OutputButton->SetValue(true);
        }
        start = start.SubString(start.Find(":") + 1, start.size());
    } else {
        NoneButton->SetValue(true);
        OutputSpin->Disable();
        ModelChoice->Disable();
    }
    StartChannel->SetValue(start);
}
std::string StartChannelDialog::Get() {
    if (OutputButton->GetValue() && OutputSpin->GetValue() != 1) {
        return std::to_string(OutputSpin->GetValue()) + ":" + std::to_string(StartChannel->GetValue());
    } else if (ModelButton->GetValue()) {
        return ">" + ModelChoice->GetStringSelection().ToStdString() + ":" + std::to_string(StartChannel->GetValue());
    } else if (StartModelButton->GetValue()) {
        return "@" + ModelChoice->GetStringSelection().ToStdString() + ":" + std::to_string(StartChannel->GetValue());
    }
    return std::to_string(StartChannel->GetValue());
}


void StartChannelDialog::OnButtonSelect(wxCommandEvent& event)
{
    if (NoneButton->GetValue()) {
        ModelChoice->Disable();
        OutputSpin->Disable();
    } else if (OutputButton->GetValue()) {
        ModelChoice->Disable();
        OutputSpin->Enable();
    } else {
        ModelChoice->Enable();
        OutputSpin->Disable();
    }
}
