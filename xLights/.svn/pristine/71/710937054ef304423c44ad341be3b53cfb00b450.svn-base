#include "ShowDatesDialog.h"

//(*InternalHeaders(ShowDatesDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(ShowDatesDialog)
const long ShowDatesDialog::ID_STATICTEXT1 = wxNewId();
const long ShowDatesDialog::ID_CALENDARCTRL_START = wxNewId();
const long ShowDatesDialog::ID_STATICLINE1 = wxNewId();
const long ShowDatesDialog::ID_STATICTEXT2 = wxNewId();
const long ShowDatesDialog::ID_CALENDARCTRL_END = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShowDatesDialog,wxDialog)
	//(*EventTable(ShowDatesDialog)
	//*)
END_EVENT_TABLE()

ShowDatesDialog::ShowDatesDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ShowDatesDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Set Show Dates"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Show Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	CalendarCtrlStart = new wxCalendarCtrl(this, ID_CALENDARCTRL_START, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CALENDARCTRL_START"));
	FlexGridSizer2->Add(CalendarCtrlStart, 1, wxALL|wxALIGN_RIGHT|wxALIGN_TOP, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	FlexGridSizer1->Add(StaticLine1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Show End"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	CalendarCtrlEnd = new wxCalendarCtrl(this, ID_CALENDARCTRL_END, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CALENDARCTRL_END"));
	FlexGridSizer3->Add(CalendarCtrlEnd, 1, wxALL|wxALIGN_RIGHT|wxALIGN_TOP, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();
	//*)
}

ShowDatesDialog::~ShowDatesDialog()
{
	//(*Destroy(ShowDatesDialog)
	//*)
}

