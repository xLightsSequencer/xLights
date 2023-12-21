/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CheckSequenceSettingsPanel.h"

//(*InternalHeaders(CheckSequenceSettingsPanel)
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"
#include "../graphics/xlGraphicsBase.h"

//(*IdInit(CheckSequenceSettingsPanel)
const long CheckSequenceSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX6 = wxNewId();
const long CheckSequenceSettingsPanel::ID_CHECKBOX7 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CheckSequenceSettingsPanel,wxPanel)
	//(*EventTable(CheckSequenceSettingsPanel)
	//*)
END_EVENT_TABLE()

CheckSequenceSettingsPanel::CheckSequenceSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(CheckSequenceSettingsPanel)
	wxGridBagSizer* GridBagSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Disabling check sequence checks can reduce clutter in your check sequence results\nbut can also mask causes of issues such as incorrect pixels lighting up or slow rendering.\n\nPlease ensure you understand what the check sequence options do before deciding\nto disable them on someone\'s advice."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_DupUniv = new wxCheckBox(this, ID_CHECKBOX1, _("Disable checks on duplicate use of universe/id across controllers."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_DupUniv->SetValue(false);
	CheckBox_DupUniv->SetHelpText(_("If you are using unicast and not using #universe:startchannel addressing then this check can be disabled."));
	GridBagSizer1->Add(CheckBox_DupUniv, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_NonContigChOnPort = new wxCheckBox(this, ID_CHECKBOX2, _("Disable checks for non-contiguous channels on controller ports."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_NonContigChOnPort->SetValue(false);
	CheckBox_NonContigChOnPort->SetHelpText(_("If you only use controllers that support virtual strings then this check can be safely disabled."));
	GridBagSizer1->Add(CheckBox_NonContigChOnPort, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_PreviewGroup = new wxCheckBox(this, ID_CHECKBOX3, _("Disable checks for groups containing models from different previews."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_PreviewGroup->SetValue(false);
	CheckBox_PreviewGroup->SetHelpText(_("Adding models to groups from different previews can make them appear in other views with no obvious reason why they do."));
	GridBagSizer1->Add(CheckBox_PreviewGroup, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_DupNodeMG = new wxCheckBox(this, ID_CHECKBOX4, _("Disable checks for duplicate nodes in model groups."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_DupNodeMG->SetValue(false);
	CheckBox_DupNodeMG->SetHelpText(_("Duplicate nodes in model groups can lead to pixels lighting during effects at unexpected times."));
	GridBagSizer1->Add(CheckBox_DupNodeMG, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_TransTime = new wxCheckBox(this, ID_CHECKBOX5, _("Disable transition time checking."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_TransTime->SetValue(false);
	CheckBox_TransTime->SetHelpText(_("Transition times that overlap or extend beyond the duration of the effect can lead to unexpected amounts of dimming."));
	GridBagSizer1->Add(CheckBox_TransTime, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_CustomSizeCheck = new wxCheckBox(this, ID_CHECKBOX6, _("Disable custom model size checking."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_CustomSizeCheck->SetValue(false);
	CheckBox_CustomSizeCheck->SetHelpText(_("Large custom models with largely empty cells generate significant rendering overhead. You may want to consider shrinking the custom model dimensions if this can done without too significantly adversely affecting appearance."));
	GridBagSizer1->Add(CheckBox_CustomSizeCheck, wxGBPosition(6, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	CheckBox_DisableSketch = new wxCheckBox(this, ID_CHECKBOX7, _("Disable sketch image file checking."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
	CheckBox_DisableSketch->SetValue(false);
	CheckBox_DisableSketch->SetHelpText(_("Sketch effect image files are not essential to rendering."));
	GridBagSizer1->Add(CheckBox_DisableSketch, wxGBPosition(7, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_DupUnivClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_NonContigChOnPortClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_PreviewGroupClick);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_DupNodeMGClick);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_TransTimeClick);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_CustomSizeCheckClick);
	Connect(ID_CHECKBOX7,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CheckSequenceSettingsPanel::OnCheckBox_DisableSketchClick);
	//*)

	CheckBox_DupUniv->SetToolTip(CheckBox_DupUniv->GetHelpText());
    CheckBox_NonContigChOnPort->SetToolTip(CheckBox_NonContigChOnPort->GetHelpText());
    CheckBox_PreviewGroup->SetToolTip(CheckBox_PreviewGroup->GetHelpText());
    CheckBox_DupNodeMG->SetToolTip(CheckBox_DupNodeMG->GetHelpText());
    CheckBox_TransTime->SetToolTip(CheckBox_TransTime->GetHelpText());
    CheckBox_CustomSizeCheck->SetToolTip(CheckBox_CustomSizeCheck->GetHelpText());
    CheckBox_DisableSketch->SetToolTip(CheckBox_DisableSketch->GetHelpText());
}

CheckSequenceSettingsPanel::~CheckSequenceSettingsPanel()
{
	//(*Destroy(CheckSequenceSettingsPanel)
	//*)
}

bool CheckSequenceSettingsPanel::TransferDataToWindow() {
    CheckBox_DupUniv->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("DupUniv"));
    CheckBox_NonContigChOnPort->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("NonContigChOnPort"));
    CheckBox_PreviewGroup->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("PreviewGroup"));
    CheckBox_DupNodeMG->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("DupNodeMG"));
    CheckBox_TransTime->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("TransTime"));
    CheckBox_CustomSizeCheck->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("CustomSizeCheck"));
    CheckBox_DisableSketch->SetValue(xLightsFrame::IsCheckSequenceOptionDisabled("SketchImage"));
    return true;
}
bool CheckSequenceSettingsPanel::TransferDataFromWindow() {
    xLightsFrame::SetCheckSequenceOptionDisable("DupUniv", CheckBox_DupUniv->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("NonContigChOnPort", CheckBox_NonContigChOnPort->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("PreviewGroup", CheckBox_PreviewGroup->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("DupNodeMG", CheckBox_DupNodeMG->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("TransTime", CheckBox_TransTime->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("CustomSizeCheck", CheckBox_CustomSizeCheck->IsChecked());
    xLightsFrame::SetCheckSequenceOptionDisable("SketchImage", CheckBox_DisableSketch->IsChecked());
    return true;
}

void CheckSequenceSettingsPanel::OnCheckBox_DupUnivClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_NonContigChOnPortClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_PreviewGroupClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_DupNodeMGClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_TransTimeClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_CustomSizeCheckClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void CheckSequenceSettingsPanel::OnCheckBox_DisableSketchClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
