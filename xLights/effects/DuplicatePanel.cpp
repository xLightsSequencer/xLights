/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DuplicatePanel.h"
#include "DuplicateEffect.h"

//(*InternalHeaders(DuplicatePanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(DuplicatePanel)
const long DuplicatePanel::ID_STATICTEXT1 = wxNewId();
const long DuplicatePanel::ID_CHOICE_Duplicate_Model = wxNewId();
const long DuplicatePanel::ID_STATICTEXT2 = wxNewId();
const long DuplicatePanel::ID_SPINCTRL_Duplicate_Layer = wxNewId();
const long DuplicatePanel::ID_CHECKBOX_Duplicate_Override_Palette = wxNewId();
const long DuplicatePanel::ID_CHECKBOX_Duplicate_Override_Color = wxNewId();
const long DuplicatePanel::ID_CHECKBOX_Duplicate_Override_Timing = wxNewId();
const long DuplicatePanel::ID_CHECKBOX_Duplicate_Override_Buffer = wxNewId();
//*)

BEGIN_EVENT_TABLE(DuplicatePanel,wxPanel)
	//(*EventTable(DuplicatePanel)
	//*)
END_EVENT_TABLE()

DuplicatePanel::DuplicatePanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(DuplicatePanel)
	wxFlexGridSizer* FlexGridSizer77;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer77 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer77->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer77->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Model = new BulkEditChoice(this, ID_CHOICE_Duplicate_Model, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Duplicate_Model"));
	FlexGridSizer77->Add(Choice_Model, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Layer:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer77->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Layer = new BulkEditSpinCtrl(this, ID_SPINCTRL_Duplicate_Layer, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 1000, 1, _T("ID_SPINCTRL_Duplicate_Layer"));
	SpinCtrl_Layer->SetValue(_T("1"));
	FlexGridSizer77->Add(SpinCtrl_Layer, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer77->Add(0,0,1, wxALL|wxEXPAND, 5);
	CheckBox_Override_Palette = new BulkEditCheckBox(this, ID_CHECKBOX_Duplicate_Override_Palette, _("Override duplicated Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Duplicate_Override_Palette"));
	CheckBox_Override_Palette->SetValue(false);
	FlexGridSizer77->Add(CheckBox_Override_Palette, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer77->Add(0,0,1, wxALL|wxEXPAND, 5);
	CheckBox1 = new BulkEditCheckBox(this, ID_CHECKBOX_Duplicate_Override_Color, _("Override duplicated Color settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Duplicate_Override_Color"));
	CheckBox1->SetValue(false);
	FlexGridSizer77->Add(CheckBox1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer77->Add(0,0,1, wxALL|wxEXPAND, 5);
	CheckBox2 = new BulkEditCheckBox(this, ID_CHECKBOX_Duplicate_Override_Timing, _("Override duplicated Layer Blending settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Duplicate_Override_Timing"));
	CheckBox2->SetValue(false);
	FlexGridSizer77->Add(CheckBox2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer77->Add(0,0,1, wxALL|wxEXPAND, 5);
	CheckBox3 = new BulkEditCheckBox(this, ID_CHECKBOX_Duplicate_Override_Buffer, _("Override duplicated Layer settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Duplicate_Override_Buffer"));
	CheckBox3->SetValue(false);
	FlexGridSizer77->Add(CheckBox3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer77);
	FlexGridSizer77->Fit(this);
	FlexGridSizer77->SetSizeHints(this);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&DuplicatePanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&DuplicatePanel::OnValidateWindow, 0, this);

    SetName("ID_PANEL_DUPLICATE");

	ValidateWindow();
}

DuplicatePanel::~DuplicatePanel()
{
	//(*Destroy(DuplicatePanel)
	//*)
}

void DuplicatePanel::ValidateWindow()
{
}

void DuplicatePanel::OnCheckBox1Click(wxCommandEvent& event)
{
}

void DuplicatePanel::OnChoice_ModelSelect(wxCommandEvent& event)
{
    SpinCtrl_Layer->SetRange(1, DuplicateEffect::GetLayersForModel(Choice_Model->GetStringSelection()));
    ValidateWindow();
}
