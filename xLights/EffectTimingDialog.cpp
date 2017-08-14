#include "EffectTimingDialog.h"
#include "sequencer/Effect.h"

//(*InternalHeaders(EffectTimingDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectTimingDialog)
const long EffectTimingDialog::ID_STATICTEXT2 = wxNewId();
const long EffectTimingDialog::ID_SPINCTRL1 = wxNewId();
const long EffectTimingDialog::ID_STATICTEXT3 = wxNewId();
const long EffectTimingDialog::ID_SPINCTRL2 = wxNewId();
const long EffectTimingDialog::ID_STATICTEXT4 = wxNewId();
const long EffectTimingDialog::ID_SPINCTRL3 = wxNewId();
const long EffectTimingDialog::ID_BUTTON1 = wxNewId();
const long EffectTimingDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectTimingDialog,wxDialog)
	//(*EventTable(EffectTimingDialog)
	//*)
END_EVENT_TABLE()

EffectTimingDialog::EffectTimingDialog(wxWindow* parent, Effect* eff, int timeInterval,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    wxASSERT(timeInterval > 0);
    _timeInterval = 1000 / timeInterval;

	//(*Initialize(EffectTimingDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Effect Timing"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Start Time (MS):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartTime = new StepSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_StartTime->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_StartTime, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Duration (MS):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Duration = new StepSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_Duration->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("End Time (MS):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_EndTime = new StepSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL3"));
	SpinCtrl_EndTime->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_EndTime, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_StartTimeChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_DurationChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_EndTimeChange);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTimingDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectTimingDialog::OnButton_CancelClick);
	//*)

    Connect(ID_SPINCTRL1, wxEVT_SPINCTRL, (wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_StartTimeChange);
    Connect(ID_SPINCTRL2, wxEVT_SPINCTRL, (wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_DurationChange);
    Connect(ID_SPINCTRL3, wxEVT_SPINCTRL, (wxObjectEventFunction)&EffectTimingDialog::OnSpinCtrl_EndTimeChange);

    SpinCtrl_StartTime->SetStep(_timeInterval);
    SpinCtrl_EndTime->SetStep(_timeInterval);
    SpinCtrl_Duration->SetStep(_timeInterval);

    SpinCtrl_StartTime->SetRange(0, _timeInterval * 49999);
    SpinCtrl_EndTime->SetRange(_timeInterval, _timeInterval * 50000);
    SpinCtrl_Duration->SetRange(_timeInterval, _timeInterval * 50000);

    SpinCtrl_StartTime->SetValue(eff->GetStartTimeMS());
    SpinCtrl_EndTime->SetValue(eff->GetEndTimeMS());
    SpinCtrl_Duration->SetValue(eff->GetEndTimeMS() - eff->GetStartTimeMS());
}

EffectTimingDialog::~EffectTimingDialog()
{
	//(*Destroy(EffectTimingDialog)
	//*)
}

void EffectTimingDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void EffectTimingDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void EffectTimingDialog::OnSpinCtrl_StartTimeChange(wxSpinEvent& event)
{
    if (SpinCtrl_StartTime->GetValue() >= SpinCtrl_EndTime->GetValue())
    {
        SpinCtrl_EndTime->SetValue(SpinCtrl_StartTime->GetValue() + _timeInterval);
    }
    SpinCtrl_Duration->SetValue(SpinCtrl_EndTime->GetValue() - SpinCtrl_StartTime->GetValue());
}

void EffectTimingDialog::OnSpinCtrl_DurationChange(wxSpinEvent& event)
{
    SpinCtrl_EndTime->SetValue(SpinCtrl_StartTime->GetValue() + SpinCtrl_Duration->GetValue());
}

void EffectTimingDialog::OnSpinCtrl_EndTimeChange(wxSpinEvent& event)
{
    if (SpinCtrl_EndTime->GetValue() <= SpinCtrl_StartTime->GetValue())
    {
        SpinCtrl_StartTime->SetValue(SpinCtrl_EndTime->GetValue() - _timeInterval);
    }
    SpinCtrl_Duration->SetValue(SpinCtrl_EndTime->GetValue() - SpinCtrl_StartTime->GetValue());
}
