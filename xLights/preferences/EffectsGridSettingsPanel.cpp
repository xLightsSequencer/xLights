/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectsGridSettingsPanel.h"

//(*InternalHeaders(EffectsGridSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(EffectsGridSettingsPanel)
const long EffectsGridSettingsPanel::ID_CHOICE1 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long EffectsGridSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX6 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long EffectsGridSettingsPanel::ID_CHOICE2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectsGridSettingsPanel,wxPanel)
	//(*EventTable(EffectsGridSettingsPanel)
	//*)
END_EVENT_TABLE()

EffectsGridSettingsPanel::EffectsGridSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
	//(*Initialize(EffectsGridSettingsPanel)
	wxGridBagSizer* GridBagSizer1;
	wxStaticText* StaticText5;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText5, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	GridSpacingChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	GridSpacingChoice->Append(_("Extra Small"));
	GridSpacingChoice->Append(_("Small"));
	GridSpacingChoice->SetSelection( GridSpacingChoice->Append(_("Medium")) );
	GridSpacingChoice->Append(_("Large"));
	GridSpacingChoice->Append(_("Extra Large"));
	GridBagSizer1->Add(GridSpacingChoice, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	IconBackgroundsCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Icon Backgrounds"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	IconBackgroundsCheckBox->SetValue(false);
	GridBagSizer1->Add(IconBackgroundsCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodeValuesCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Node Values"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	NodeValuesCheckBox->SetValue(false);
	GridBagSizer1->Add(NodeValuesCheckBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SnapToTimingCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Snap to Timing Marks"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	SnapToTimingCheckBox->SetValue(false);
	GridBagSizer1->Add(SnapToTimingCheckBox, wxGBPosition(3, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Double Click Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SmallWaveformCheckBox = new wxCheckBox(this, ID_CHECKBOX4, _("Small Waveform"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	SmallWaveformCheckBox->SetValue(false);
	GridBagSizer1->Add(SmallWaveformCheckBox, wxGBPosition(5, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TransistionMarksCheckBox = new wxCheckBox(this, ID_CHECKBOX6, _("Display Transition Marks"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	TransistionMarksCheckBox->SetValue(true);
	GridBagSizer1->Add(TransistionMarksCheckBox, wxGBPosition(6, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ColorUpdateWarnCheckBox = new wxCheckBox(this, ID_CHECKBOX5, _("Hide Color Update Warning"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	ColorUpdateWarnCheckBox->SetValue(false);
	GridBagSizer1->Add(ColorUpdateWarnCheckBox, wxGBPosition(7, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DoubleClickChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	DoubleClickChoice->Append(_("Edit Text"));
	DoubleClickChoice->SetSelection( DoubleClickChoice->Append(_("Play Timing")) );
	GridBagSizer1->Add(DoubleClickChoice, wxGBPosition(4, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnGridSpacingChoiceSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnIconBackgroundsCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnNodeValuesCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnSnapToTimingCheckBoxClick);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnSmallWaveformCheckBoxClick);
	Connect(ID_CHECKBOX6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnTransistionMarksCheckBoxClick);
	Connect(ID_CHECKBOX5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnColorUpdateWarnCheckBoxClick);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EffectsGridSettingsPanel::OnDoubleClickChoiceSelect);
	//*)
}

EffectsGridSettingsPanel::~EffectsGridSettingsPanel()
{
	//(*Destroy(EffectsGridSettingsPanel)
	//*)
}


bool EffectsGridSettingsPanel::TransferDataToWindow() {
    NodeValuesCheckBox->SetValue(frame->GridNodeValues());
    IconBackgroundsCheckBox->SetValue(frame->GridIconBackgrounds());
    DoubleClickChoice->SetSelection(frame->TimingPlayOnDClick());
    SmallWaveformCheckBox->SetValue(frame->SmallWaveform());
    SnapToTimingCheckBox->SetValue(frame->SnapToTimingMarks());
    TransistionMarksCheckBox->SetValue(!frame->IsSuppressFadeHints());
    ColorUpdateWarnCheckBox->SetValue(frame->SuppressColorWarn());
    int gs = frame->GridSpacing();
    switch (gs) {
        case 48:
            GridSpacingChoice->SetSelection(4);
            break;
        case 32:
            GridSpacingChoice->SetSelection(3);
            break;
        case 24:
            GridSpacingChoice->SetSelection(2);
            break;
        case 12:
            GridSpacingChoice->SetSelection(0);
            break;
        case 16:
        default:
            GridSpacingChoice->SetSelection(1);
            break;
    }
    return true;
}
bool EffectsGridSettingsPanel::TransferDataFromWindow() {
    switch (GridSpacingChoice->GetSelection()) {
        case 4:
            frame->SetGridSpacing(48);
            break;
        case 3:
            frame->SetGridSpacing(32);
            break;
        case 2:
            frame->SetGridSpacing(24);
            break;
        case 1:
            frame->SetGridSpacing(16);
            break;
        case 0:
        default:
            frame->SetGridSpacing(12);
            break;
    }
    frame->SetGridNodeValues(NodeValuesCheckBox->IsChecked());
    frame->SetGridIconBackgrounds(IconBackgroundsCheckBox->IsChecked());
    frame->SetTimingPlayOnDClick(DoubleClickChoice->GetSelection());
    frame->SetSmallWaveform(SmallWaveformCheckBox->IsChecked());
    frame->SetSnapToTimingMarks(SnapToTimingCheckBox->IsChecked());
    frame->SetSuppressFadeHints(!TransistionMarksCheckBox->IsChecked());
    frame->SetSuppressColorWarn(ColorUpdateWarnCheckBox->IsChecked());
    return true;
}


void EffectsGridSettingsPanel::OnIconBackgroundsCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnNodeValuesCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnSnapToTimingCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnSmallWaveformCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnGridSpacingChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnTransistionMarksCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnDoubleClickChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void EffectsGridSettingsPanel::OnColorUpdateWarnCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
