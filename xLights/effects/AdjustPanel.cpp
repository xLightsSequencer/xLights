/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(AdjustPanel)
 #include <wx/choice.h>
 #include <wx/intl.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 //*)

#include <wx/textentry.h>

#include "AdjustPanel.h"
#include "AdjustEffect.h"
#include "EffectPanelUtils.h"
#include "../UtilFunctions.h"

//(*IdInit(AdjustPanel)
const long AdjustPanel::ID_STATICTEXT1 = wxNewId();
const long AdjustPanel::ID_CHOICE_Action = wxNewId();
const long AdjustPanel::ID_STATICTEXT2 = wxNewId();
const long AdjustPanel::ID_SPINCTRL_Value1 = wxNewId();
const long AdjustPanel::ID_STATICTEXT3 = wxNewId();
const long AdjustPanel::ID_SPINCTRL_Value2 = wxNewId();
const long AdjustPanel::ID_STATICTEXT4 = wxNewId();
const long AdjustPanel::ID_SPINCTRL_NthChannel = wxNewId();
const long AdjustPanel::ID_STATICTEXT5 = wxNewId();
const long AdjustPanel::ID_SPINCTRL_StartingAt = wxNewId();
const long AdjustPanel::ID_STATICTEXT6 = wxNewId();
const long AdjustPanel::ID_SPINCTRL_Count = wxNewId();
//*)

BEGIN_EVENT_TABLE(AdjustPanel,wxPanel)
	//(*EventTable(AdjustPanel)
	//*)
END_EVENT_TABLE()

AdjustPanel::AdjustPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(AdjustPanel)
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer_Main = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer_Main->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Adjustment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer_Main->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Action = new BulkEditChoice(this, ID_CHOICE_Action, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Action"));
	FlexGridSizer_Main->Add(Choice_Action, 1, wxALL|wxEXPAND, 5);
	StaticText_Value1 = new wxStaticText(this, ID_STATICTEXT2, _("Value 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer_Main->Add(StaticText_Value1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value1 = new BulkEditSpinCtrl(this, ID_SPINCTRL_Value1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -255, 255, 0, _T("ID_SPINCTRL_Value1"));
	SpinCtrl_Value1->SetValue(_T("0"));
	FlexGridSizer_Main->Add(SpinCtrl_Value1, 1, wxALL|wxEXPAND, 5);
	StaticText_Value2 = new wxStaticText(this, ID_STATICTEXT3, _("Value 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer_Main->Add(StaticText_Value2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value2 = new BulkEditSpinCtrl(this, ID_SPINCTRL_Value2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -255, 255, 0, _T("ID_SPINCTRL_Value2"));
	SpinCtrl_Value2->SetValue(_T("0"));
	FlexGridSizer_Main->Add(SpinCtrl_Value2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT4, _("Nth Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer_Main->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NthChannel = new BulkEditSpinCtrl(this, ID_SPINCTRL_NthChannel, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 32, 1, _T("ID_SPINCTRL_NthChannel"));
	SpinCtrl_NthChannel->SetValue(_T("1"));
	FlexGridSizer_Main->Add(SpinCtrl_NthChannel, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT5, _("Starting At:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer_Main->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartingAt = new BulkEditSpinCtrl(this, ID_SPINCTRL_StartingAt, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL_StartingAt"));
	SpinCtrl_StartingAt->SetValue(_T("1"));
	FlexGridSizer_Main->Add(SpinCtrl_StartingAt, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT6, _("Count:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer_Main->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Count = new BulkEditSpinCtrl(this, ID_SPINCTRL_Count, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 0, _T("ID_SPINCTRL_Count"));
	SpinCtrl_Count->SetValue(_T("0"));
	FlexGridSizer_Main->Add(SpinCtrl_Count, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer_Main);
	FlexGridSizer_Main->Fit(this);
	FlexGridSizer_Main->SetSizeHints(this);

	Connect(ID_CHOICE_Action,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&AdjustPanel::OnChoice_ActionSelect);
	//*)
    SetName("ID_PANEL_ADJUST");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&AdjustPanel::OnVCChanged, 0, this);
	Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&AdjustPanel::OnValidateWindow, 0, this);

	Choice_Action->Append("None");
    Choice_Action->Append("Adjust By Value");
    Choice_Action->Append("Adjust By Percentage");
    Choice_Action->Append("Set Minimum");
    Choice_Action->Append("Set Maximum");
    Choice_Action->Append("Set Range");
    Choice_Action->Append("Shift With Wrap By Value");
    Choice_Action->Append("Prevent Range");
    Choice_Action->Append("Reverse");

	ValidateWindow();
}

AdjustPanel::~AdjustPanel()
{
	//(*Destroy(AdjustPanel)
	//*)
}

void AdjustPanel::ValidateWindow()
{
    auto action = Choice_Action->GetStringSelection();

    if (action == "None") {
        StaticText_Value1->SetLabel("Unused:");
        SpinCtrl_Value1->Disable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Adjust By Value") {
        StaticText_Value1->SetLabel("Adjust by:");
        SpinCtrl_Value1->SetRange(-255, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Adjust By Percentage") {
        StaticText_Value1->SetLabel("Adjust by:");
        SpinCtrl_Value1->SetRange(-100, 25500);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Set Minimum") {
        StaticText_Value1->SetLabel("Minimum:");
        SpinCtrl_Value1->SetRange(0, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Set Maximum") {
        StaticText_Value1->SetLabel("Maximum:");
        SpinCtrl_Value1->SetRange(0, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Set Range") {
        StaticText_Value1->SetLabel("Minimum:");
        SpinCtrl_Value1->SetRange(0, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Maximum:");
        SpinCtrl_Value2->SetRange(0, 255);
        SpinCtrl_Value2->Enable();
    } else if (action == "Shift With Wrap By Value") {
        StaticText_Value1->SetLabel("Shift by:");
        SpinCtrl_Value1->SetRange(-255, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    } else if (action == "Prevent Range") {
        StaticText_Value1->SetLabel("Minimum:");
        SpinCtrl_Value1->SetRange(0, 255);
        SpinCtrl_Value1->Enable();
        StaticText_Value2->SetLabel("Maximum:");
        SpinCtrl_Value2->SetRange(0, 255);
        SpinCtrl_Value2->Enable();
    } else if (action == "Reverse") {
        StaticText_Value1->SetLabel("Unused:");
        SpinCtrl_Value1->Disable();
        StaticText_Value2->SetLabel("Unused:");
        SpinCtrl_Value2->Disable();
    }
}

void AdjustPanel::OnChoice_ActionSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
