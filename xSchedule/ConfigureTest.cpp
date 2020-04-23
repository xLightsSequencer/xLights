/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ConfigureTest.h"

//(*InternalHeaders(ConfigureTest)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ScheduleOptions.h"

//(*IdInit(ConfigureTest)
const long ConfigureTest::ID_STATICTEXT1 = wxNewId();
const long ConfigureTest::ID_CHOICE1 = wxNewId();
const long ConfigureTest::ID_STATICTEXT2 = wxNewId();
const long ConfigureTest::ID_SPINCTRL1 = wxNewId();
const long ConfigureTest::ID_STATICTEXT3 = wxNewId();
const long ConfigureTest::ID_SPINCTRL2 = wxNewId();
const long ConfigureTest::ID_STATICTEXT4 = wxNewId();
const long ConfigureTest::ID_SPINCTRL3 = wxNewId();
const long ConfigureTest::ID_BUTTON1 = wxNewId();
const long ConfigureTest::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ConfigureTest,wxDialog)
	//(*EventTable(ConfigureTest)
	//*)
END_EVENT_TABLE()

ConfigureTest::ConfigureTest(wxWindow* parent, TestOptions* testOptions, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _testOptions = testOptions;

	//(*Initialize(ConfigureTest)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Configure test mode"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->SetSelection( Choice1->Append(_("Alternate")) );
	Choice1->Append(_("Foreground"));
	Choice1->Append(_("A-B-C"));
	Choice1->Append(_("A-B-C-All"));
	Choice1->Append(_("A-B-C-All-None"));
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Foreground:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("255"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 255, _T("ID_SPINCTRL1"));
	SpinCtrl1->SetValue(_T("255"));
	FlexGridSizer1->Add(SpinCtrl1, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Background:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl2 = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("ID_SPINCTRL2"));
	SpinCtrl2->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl2, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Interval (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl3 = new wxSpinCtrl(this, ID_SPINCTRL3, _T("500"), wxDefaultPosition, wxDefaultSize, 0, 0, 30000, 500, _T("ID_SPINCTRL3"));
	SpinCtrl3->SetValue(_T("500"));
	FlexGridSizer1->Add(SpinCtrl3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConfigureTest::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConfigureTest::OnButton_CancelClick);
	//*)

    Choice1->Append(_("A"));
    Choice1->Append(_("B"));
    Choice1->Append(_("C"));

    Choice1->SetStringSelection(_testOptions->GetMode());
    SpinCtrl1->SetValue(_testOptions->GetLevel1());
    SpinCtrl2->SetValue(_testOptions->GetLevel2());
    SpinCtrl3->SetValue(_testOptions->GetInterval());
}

ConfigureTest::~ConfigureTest()
{
	//(*Destroy(ConfigureTest)
	//*)
}

void ConfigureTest::OnButton_OkClick(wxCommandEvent& event)
{
    _testOptions->SetLevel1(SpinCtrl1->GetValue());
    _testOptions->SetLevel2(SpinCtrl2->GetValue());
    _testOptions->SetInterval(SpinCtrl3->GetValue());
    _testOptions->SetMode(Choice1->GetStringSelection().ToStdString());
    EndDialog(wxID_OK);
}

void ConfigureTest::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
