/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ScheduleDialog.h"
#include "Schedule.h"

//(*InternalHeaders(ScheduleDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ScheduleDialog)
const long ScheduleDialog::ID_CHECKBOX11 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT15 = wxNewId();
const long ScheduleDialog::ID_TEXTCTRL3 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT1 = wxNewId();
const long ScheduleDialog::ID_DATEPICKERCTRL1 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT2 = wxNewId();
const long ScheduleDialog::ID_DATEPICKERCTRL2 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX1 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT3 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT4 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT5 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT6 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT7 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT8 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT9 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT10 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX8 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX7 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX6 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX5 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX4 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX3 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX2 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT16 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL3 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT17 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL4 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT18 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT12 = wxNewId();
const long ScheduleDialog::ID_TEXTCTRL2 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT20 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL5 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT11 = wxNewId();
const long ScheduleDialog::ID_TEXTCTRL1 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT21 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL6 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX12 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX13 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT19 = wxNewId();
const long ScheduleDialog::ID_CHOICE1 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX9 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT13 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL1 = wxNewId();
const long ScheduleDialog::ID_CHECKBOX10 = wxNewId();
const long ScheduleDialog::ID_STATICTEXT14 = wxNewId();
const long ScheduleDialog::ID_SPINCTRL2 = wxNewId();
const long ScheduleDialog::ID_BUTTON1 = wxNewId();
const long ScheduleDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ScheduleDialog,wxDialog)
	//(*EventTable(ScheduleDialog)
	//*)
END_EVENT_TABLE()

ScheduleDialog::ScheduleDialog(wxWindow* parent, Schedule* schedule, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _schedule = schedule;

	//(*Initialize(ScheduleDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;

	Create(parent, id, _("Schedule Playlist"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Enabled = new wxCheckBox(this, ID_CHECKBOX11, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX11"));
	CheckBox_Enabled->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Enabled, 1, wxALL|wxEXPAND, 5);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT15, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer1->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Date:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DatePickerCtrl_Start = new wxDatePickerCtrl(this, ID_DATEPICKERCTRL1, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT|wxDP_SHOWCENTURY, wxDefaultValidator, _T("ID_DATEPICKERCTRL1"));
	FlexGridSizer1->Add(DatePickerCtrl_Start, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("End Date:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DatePickerCtrl_End = new wxDatePickerCtrl(this, ID_DATEPICKERCTRL2, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT|wxDP_SHOWCENTURY, wxDefaultValidator, _T("ID_DATEPICKERCTRL2"));
	FlexGridSizer1->Add(DatePickerCtrl_End, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_EveryYear = new wxCheckBox(this, ID_CHECKBOX1, _("Every year"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_EveryYear->SetValue(false);
	FlexGridSizer1->Add(CheckBox_EveryYear, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Days of Week:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(2, 7, 0, 0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("M"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Tu"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("W"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Th"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("F"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Sa"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Su"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer2->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Mon = new wxCheckBox(this, ID_CHECKBOX8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX8"));
	CheckBox_Mon->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Mon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Tue = new wxCheckBox(this, ID_CHECKBOX7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
	CheckBox_Tue->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Tue, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Wed = new wxCheckBox(this, ID_CHECKBOX6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_Wed->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Wed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Thu = new wxCheckBox(this, ID_CHECKBOX5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_Thu->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Thu, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fri = new wxCheckBox(this, ID_CHECKBOX4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Fri->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Fri, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Sat = new wxCheckBox(this, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_Sat->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Sat, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Sun = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_Sun->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Sun, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText16 = new wxStaticText(this, ID_STATICTEXT16, _("Every nth day:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
	FlexGridSizer1->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
	SpinCtrl_NthDay = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 14, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_NthDay->SetValue(_T("1"));
	FlexGridSizer4->Add(SpinCtrl_NthDay, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText17 = new wxStaticText(this, ID_STATICTEXT17, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
	FlexGridSizer4->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NthDayOffset = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 13, 0, _T("ID_SPINCTRL4"));
	SpinCtrl_NthDayOffset->SetValue(_T("0"));
	FlexGridSizer4->Add(SpinCtrl_NthDayOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_NextDay = new wxStaticText(this, ID_STATICTEXT18, _("2000-01-01"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
	FlexGridSizer4->Add(StaticText_NextDay, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("On Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer1->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	TextCtrl_OnTime = new wxTextCtrl(this, ID_TEXTCTRL2, _("19:00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer5->Add(TextCtrl_OnTime, 1, wxALL|wxEXPAND, 5);
	StaticText_OnOffsetMins = new wxStaticText(this, ID_STATICTEXT20, _("Offset Mins:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
	FlexGridSizer5->Add(StaticText_OnOffsetMins, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_OnOffsetMins = new wxSpinCtrl(this, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -120, 120, 0, _T("ID_SPINCTRL5"));
	SpinCtrl_OnOffsetMins->SetValue(_T("0"));
	FlexGridSizer5->Add(SpinCtrl_OnOffsetMins, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Off Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer1->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	TextCtrl_OffTime = new wxTextCtrl(this, ID_TEXTCTRL1, _("22:00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer6->Add(TextCtrl_OffTime, 1, wxALL|wxEXPAND, 5);
	StaticText_OffOffsetMins = new wxStaticText(this, ID_STATICTEXT21, _("Offset Mins:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
	FlexGridSizer6->Add(StaticText_OffOffsetMins, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_OffOffsetMins = new wxSpinCtrl(this, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -120, 120, 0, _T("ID_SPINCTRL6"));
	SpinCtrl_OffOffsetMins->SetValue(_T("0"));
	FlexGridSizer6->Add(SpinCtrl_OffOffsetMins, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_GracefullyInterrupt = new wxCheckBox(this, ID_CHECKBOX12, _("Gracefully interupt any lower priority playing schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX12"));
	CheckBox_GracefullyInterrupt->SetValue(false);
	FlexGridSizer1->Add(CheckBox_GracefullyInterrupt, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_HardStop = new wxCheckBox(this, ID_CHECKBOX13, _("Hard stop at end time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX13"));
	CheckBox_HardStop->SetValue(false);
	FlexGridSizer1->Add(CheckBox_HardStop, 1, wxALL|wxEXPAND, 5);
	StaticText18 = new wxStaticText(this, ID_STATICTEXT19, _("Refire frequency:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
	FlexGridSizer1->Add(StaticText18, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_FireFrequency = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_FireFrequency->SetSelection( Choice_FireFrequency->Append(_("Fire once")) );
	Choice_FireFrequency->Append(_("Fire every hour"));
	Choice_FireFrequency->Append(_("Fire every 30 minutes"));
	Choice_FireFrequency->Append(_("Fire every 20 minutes"));
	Choice_FireFrequency->Append(_("Fire every 15 minutes"));
	Choice_FireFrequency->Append(_("Fire every 10 minutes"));
	FlexGridSizer1->Add(Choice_FireFrequency, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Loop = new wxCheckBox(this, ID_CHECKBOX9, _("Loop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX9"));
	CheckBox_Loop->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Loop, 1, wxALL|wxEXPAND, 5);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Max Loops:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer1->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_MaxLoops = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_MaxLoops->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_MaxLoops, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shuffle = new wxCheckBox(this, ID_CHECKBOX10, _("Shuffle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX10"));
	CheckBox_Shuffle->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Shuffle, 1, wxALL|wxEXPAND, 5);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer1->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL2, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL2"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ScheduleDialog::OnTextCtrl_NameText);
	Connect(ID_DATEPICKERCTRL1,wxEVT_DATE_CHANGED,(wxObjectEventFunction)&ScheduleDialog::OnDatePickerCtrl_StartChanged);
	Connect(ID_DATEPICKERCTRL2,wxEVT_DATE_CHANGED,(wxObjectEventFunction)&ScheduleDialog::OnDatePickerCtrl_EndChanged);
	Connect(ID_CHECKBOX8,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX7,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnCheckBox_DOWClick);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ScheduleDialog::OnSpinCtrl_NthDayChange);
	Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ScheduleDialog::OnSpinCtrl_NthDayOffsetChange);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ScheduleDialog::OnTextCtrl_OnTimeText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ScheduleDialog::OnTextCtrl_OffTimeText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ScheduleDialog::OnChoice_FireFrequencySelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ScheduleDialog::OnButton_CancelClick);
	//*)

    Connect(ID_TEXTCTRL1, wxEVT_KILL_FOCUS, (wxObjectEventFunction)&ScheduleDialog::OnTextCtrl_OffKillFocus);
    Connect(ID_TEXTCTRL2, wxEVT_KILL_FOCUS, (wxObjectEventFunction)&ScheduleDialog::OnTextCtrl_OnKillFocus);

    TextCtrl_Name->SetValue(schedule->GetName());
    TextCtrl_OnTime->SetValue(schedule->GetStartTimeAsString());
    TextCtrl_OffTime->SetValue(schedule->GetEndTimeAsString());
    SpinCtrl_Priority->SetValue(schedule->GetPriority());
    SpinCtrl_MaxLoops->SetValue(schedule->GetLoops());
    DatePickerCtrl_Start->SetValue(schedule->GetStartDate());
    DatePickerCtrl_End->SetValue(schedule->GetEndDate());
    CheckBox_EveryYear->SetValue(schedule->GetEveryYear());
    CheckBox_Loop->SetValue(schedule->GetLoop());
    CheckBox_GracefullyInterrupt->SetValue(schedule->GetGracefullyInterrupt());
    CheckBox_Shuffle->SetValue(schedule->GetRandom());
    CheckBox_Mon->SetValue(schedule->IsOnDOW("Mon"));
    CheckBox_Tue->SetValue(schedule->IsOnDOW("Tue"));
    CheckBox_Wed->SetValue(schedule->IsOnDOW("Wed"));
    CheckBox_Thu->SetValue(schedule->IsOnDOW("Thu"));
    CheckBox_Fri->SetValue(schedule->IsOnDOW("Fri"));
    CheckBox_Sat->SetValue(schedule->IsOnDOW("Sat"));
    CheckBox_Sun->SetValue(schedule->IsOnDOW("Sun"));
    CheckBox_Enabled->SetValue(schedule->GetEnabled());
    SpinCtrl_NthDay->SetValue(schedule->GetNthDay());
    SpinCtrl_NthDayOffset->SetValue(schedule->GetNthDayOffset());
    Choice_FireFrequency->SetStringSelection(schedule->GetFireFrequency());
	SpinCtrl_OnOffsetMins->SetValue(schedule->GetOnOffsetMins());
	SpinCtrl_OffOffsetMins->SetValue(schedule->GetOffOffsetMins());
	CheckBox_HardStop->SetValue(schedule->IsHardStop());

    Choice_FireFrequency->SetToolTip("Times are relative to schedule start time.");

    SetEscapeId(Button_Cancel->GetId());

#ifdef _DEBUG
    Choice_FireFrequency->Append(_("Fire every 2 minutes"));
#endif

    ValidateWindow();
}

ScheduleDialog::~ScheduleDialog()
{
	//(*Destroy(ScheduleDialog)
	//*)
    _schedule->SetName(TextCtrl_Name->GetValue().ToStdString());
    _schedule->SetNthDay(SpinCtrl_NthDay->GetValue());
    _schedule->SetNthDayOffset(SpinCtrl_NthDayOffset->GetValue());
    _schedule->SetStartTime(TextCtrl_OnTime->GetValue().ToStdString());
    _schedule->SetEndTime(TextCtrl_OffTime->GetValue().ToStdString());
    _schedule->SetPriority(SpinCtrl_Priority->GetValue());
    _schedule->SetLoops(SpinCtrl_MaxLoops->GetValue());
    _schedule->SetStartDate(DatePickerCtrl_Start->GetValue());
    _schedule->SetEndDate(DatePickerCtrl_End->GetValue());
    _schedule->SetGracefullyInterrupt(CheckBox_GracefullyInterrupt->GetValue());
    _schedule->SetEveryYear(CheckBox_EveryYear->GetValue());
    _schedule->SetLoop(CheckBox_Loop->GetValue());
    _schedule->SetRandom(CheckBox_Shuffle->GetValue());
    _schedule->SetDOW(CheckBox_Mon->GetValue(),
        CheckBox_Tue->GetValue(),
        CheckBox_Wed->GetValue(),
        CheckBox_Thu->GetValue(),
        CheckBox_Fri->GetValue(),
        CheckBox_Sat->GetValue(),
        CheckBox_Sun->GetValue());
    _schedule->SetEnabled(CheckBox_Enabled->GetValue());
    _schedule->SetFireFrequency(Choice_FireFrequency->GetStringSelection());
	_schedule->SetOnOffsetMins(SpinCtrl_OnOffsetMins->GetValue());
	_schedule->SetOffOffsetMins(SpinCtrl_OffOffsetMins->GetValue());
	_schedule->SetHardStop(CheckBox_HardStop->GetValue());
}

void ScheduleDialog::ValidateWindow()
{
    if (Choice_FireFrequency->GetStringSelection() == "Fire once")
    {
        SpinCtrl_MaxLoops->Enable();
        CheckBox_Loop->Enable();
    }
    else
    {
        SpinCtrl_MaxLoops->SetValue(0);
        CheckBox_Loop->SetValue(false);
        SpinCtrl_MaxLoops->Disable();
        CheckBox_Loop->Disable();
    }

    if (TextCtrl_Name->GetValue() != "" &&
        DatePickerCtrl_Start->GetValue() <= DatePickerCtrl_End->GetValue() &&
        (CheckBox_Mon->GetValue() ||
            CheckBox_Tue->GetValue() ||
            CheckBox_Wed->GetValue() ||
            CheckBox_Thu->GetValue() ||
            CheckBox_Fri->GetValue() ||
            CheckBox_Sat->GetValue() ||
            CheckBox_Sun->GetValue()
            ))
    {
        Button_Ok->Enable(true);
    }
    else
    {
        Button_Ok->Enable(false);
    }

	auto st = TextCtrl_OnTime->GetValue().Lower();
	if (st == "sunrise" || st == "sunup" || st == "sunset" || st == "sundown") 		{
		if (!StaticText_OnOffsetMins->IsShown()) {
			StaticText_OnOffsetMins->Show();
			SpinCtrl_OnOffsetMins->Show();
			Layout();
		}
	}
	else {
		if (StaticText_OnOffsetMins->IsShown()) {
			StaticText_OnOffsetMins->Hide();
			SpinCtrl_OnOffsetMins->Hide();
			SpinCtrl_OnOffsetMins->SetValue(0);
			Layout();
		}
	}

	auto et = TextCtrl_OffTime->GetValue().Lower();
	if (et == "sunrise" || et == "sunup" || et == "sunset" || et == "sundown") {
		if (!StaticText_OffOffsetMins->IsShown()) {
			StaticText_OffOffsetMins->Show();
			SpinCtrl_OffOffsetMins->Show();
			Layout();
		}
	}
	else {
		if (StaticText_OffOffsetMins->IsShown()) {
			StaticText_OffOffsetMins->Hide();
			SpinCtrl_OffOffsetMins->Hide();
			SpinCtrl_OffOffsetMins->SetValue(0);
			Layout();
		}
	}

    StaticText_NextDay->SetLabel(Schedule::GetNextNthDay(SpinCtrl_NthDay->GetValue(), SpinCtrl_NthDayOffset->GetValue()));
}


void ScheduleDialog::OnDatePickerCtrl_StartChanged(wxDateEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnDatePickerCtrl_EndChanged(wxDateEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnTextCtrl_OnTimeText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnTextCtrl_OffTimeText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ScheduleDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ScheduleDialog::OnCheckBox_DOWClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnTextCtrl_NameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnSpinCtrl_NthDayChange(wxSpinEvent& event)
{
    SpinCtrl_NthDayOffset->SetRange(0, SpinCtrl_NthDay->GetValue() - 1);
    if (SpinCtrl_NthDayOffset->GetValue() > SpinCtrl_NthDayOffset->GetMax())
    {
        SpinCtrl_NthDayOffset->SetValue(SpinCtrl_NthDayOffset->GetMax());
    }
    ValidateWindow();
}

void ScheduleDialog::OnSpinCtrl_NthDayOffsetChange(wxSpinEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnChoice_FireFrequencySelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ScheduleDialog::OnTextCtrl_OnKillFocus(wxFocusEvent& event)
{
    wxDateTime dt;
    dt.ParseFormat(TextCtrl_OnTime->GetValue(), "%H:%M:%S");
    if (dt.GetSecond() != 0)
    {
        dt.SetSecond(0);
        TextCtrl_OnTime->SetValue(dt.Format("%H:%M:%S"));
    }
}

void ScheduleDialog::OnTextCtrl_OffKillFocus(wxFocusEvent& event)
{
    wxDateTime dt;
    dt.ParseFormat(TextCtrl_OffTime->GetValue(), "%H:%M:%S");
    if (dt.GetSecond() != 0)
    {
        dt.SetSecond(0);
        TextCtrl_OffTime->SetValue(dt.Format("%H:%M:%S"));
    }
}
