#include "StartChannelDialog.h"

//(*InternalHeaders(StartChannelDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(StartChannelDialog)
const long StartChannelDialog::ID_SPINCTRL1 = wxNewId();
const long StartChannelDialog::ID_STATICTEXT2 = wxNewId();
const long StartChannelDialog::ID_SPINCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(StartChannelDialog,wxDialog)
	//(*EventTable(StartChannelDialog)
	//*)
END_EVENT_TABLE()

StartChannelDialog::StartChannelDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(StartChannelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
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
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("From Output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	OutputSpin = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 9999, 1, _T("ID_SPINCTRL2"));
	OutputSpin->SetValue(_T("1"));
	FlexGridSizer2->Add(OutputSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

StartChannelDialog::~StartChannelDialog()
{
	//(*Destroy(StartChannelDialog)
	//*)
}

void StartChannelDialog::Set(const wxString &s) {
    wxString start = s;
    if (start.Contains(":")) {
        wxString sNet = start.SubString(0, start.Find(":")-1);
        OutputSpin->SetValue(sNet);
        start = start.SubString(start.Find(":") + 1, start.size());
    }
    StartChannel->SetValue(start);
}
std::string StartChannelDialog::Get() {
    if (OutputSpin->GetValue() == 1) {
        return std::to_string(StartChannel->GetValue());
    }
    return std::to_string(OutputSpin->GetValue()) + ":" + std::to_string(StartChannel->GetValue());
}

