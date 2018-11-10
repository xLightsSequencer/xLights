#include "RemoteModeConfigDialog.h"

//(*InternalHeaders(RemoteModeConfigDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(RemoteModeConfigDialog)
const long RemoteModeConfigDialog::ID_STATICTEXT1 = wxNewId();
const long RemoteModeConfigDialog::ID_SPINCTRL1 = wxNewId();
const long RemoteModeConfigDialog::ID_STATICTEXT2 = wxNewId();
const long RemoteModeConfigDialog::ID_SPINCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(RemoteModeConfigDialog,wxDialog)
	//(*EventTable(RemoteModeConfigDialog)
	//*)
END_EVENT_TABLE()

RemoteModeConfigDialog::RemoteModeConfigDialog(wxWindow* parent, int latency, int jitter, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(RemoteModeConfigDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Remote Offset (milliseconds):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Adjust = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -5000, 5000, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_Adjust->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Adjust, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Ignore Jitter (milliseconds):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_AcceptableJitter = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 1000, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_AcceptableJitter->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_AcceptableJitter, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    SpinCtrl_Adjust->SetValue(latency);
    SpinCtrl_AcceptableJitter->SetValue(jitter);
}

RemoteModeConfigDialog::~RemoteModeConfigDialog()
{
	//(*Destroy(RemoteModeConfigDialog)
	//*)
}

int RemoteModeConfigDialog::GetLatency() const
{
    return SpinCtrl_Adjust->GetValue();
}

int RemoteModeConfigDialog::GetJitter() const
{
    return SpinCtrl_AcceptableJitter->GetValue();
}