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
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include <wx/preferences.h>
#include "xLightsMain.h"

EffectsGridSettingsPanel::EffectsGridSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Labelled choices at the top.
    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Spacing")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    GridSpacingChoice = new wxChoice(this, wxID_ANY);
    GridSpacingChoice->Append(_("Extra Small"));
    GridSpacingChoice->Append(_("Small"));
    GridSpacingChoice->SetSelection(GridSpacingChoice->Append(_("Medium")));
    GridSpacingChoice->Append(_("Large"));
    GridSpacingChoice->Append(_("Extra Large"));
    grid->Add(GridSpacingChoice, 0, wxALL, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("Row height / effect icon size in the sequencer grid.")), 0, wxLEFT | wxBOTTOM, 5);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Double Click Mode")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    DoubleClickChoice = new wxChoice(this, wxID_ANY);
    DoubleClickChoice->Append(_("Edit Text"));
    DoubleClickChoice->SetSelection(DoubleClickChoice->Append(_("Play Timing")));
    grid->Add(DoubleClickChoice, 0, wxALL, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("What double-clicking a timing effect does — edit its text or play it.")), 0, wxLEFT | wxBOTTOM, 5);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Paste As")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    PasteAsChoice = new wxChoice(this, wxID_ANY);
    PasteAsChoice->SetSelection(PasteAsChoice->Append(_("Relative")));
    PasteAsChoice->Append(_("Layers"));
    grid->Add(PasteAsChoice, 0, wxALL, 5);
    grid->Add(0, 0);
    grid->Add(MakePreferenceHint(this, _("Relative pastes at the selected position; Layers preserves the copied layer structure.")), 0, wxLEFT | wxBOTTOM, 5);

    sizer->Add(grid, 0, wxEXPAND | wxALL, 5);

    struct Toggle {
        wxCheckBox** ctrl;
        wxString label;
        wxString hint;
        bool defaultValue;
    };
    const Toggle toggles[] = {
        { &IconBackgroundsCheckBox, _("Effect Backgrounds"), _("Show animated gif backgrounds for most effects."), false },
        { &NodeValuesCheckBox, _("Node Values"), _("Show individual node values on the grid."), false },
        { &GroupEffectIndicator, _("Group Effect Indicator"), _("Show a bar on a model row indicating an effect is present."), true },
        { &SnapToTimingCheckBox, _("Snap to Timing Marks"), _("Snap effect edges to the nearest timing mark while dragging."), false },
        { &SmallWaveformCheckBox, _("Small Waveform"), _("Reduce the vertical size of the audio waveform."), false },
        { &TransistionMarksCheckBox, _("Display Transition Marks"), _("Show in/out transition (fade) markers on effects."), true },
        { &ColorUpdateWarnCheckBox, _("Hide Color Update Warning"), _("Don't warn when applying colours to multiple selected effects."), false },
        { &ShowAlternateTimingFormatCheckBox, _("Show Alternate Timing Format"), _("Show sequencer timing in seconds and milliseconds."), false },
        { &BellOnRenderCompletion, _("Bell on render completion or error"), _("Play a sound when rendering finishes or errors."), false },
    };
    for (const auto& t : toggles) {
        *t.ctrl = new wxCheckBox(this, wxID_ANY, t.label);
        (*t.ctrl)->SetValue(t.defaultValue);
        sizer->Add(*t.ctrl, 0, wxLEFT | wxTOP, 8);
        sizer->Add(MakePreferenceHint(this, t.hint), 0, wxLEFT | wxBOTTOM, 26);
        (*t.ctrl)->Bind(wxEVT_CHECKBOX, &EffectsGridSettingsPanel::OnChanged, this);
    }

    SetSizer(sizer);
    sizer->SetSizeHints(this);

    GridSpacingChoice->Bind(wxEVT_CHOICE, &EffectsGridSettingsPanel::OnChanged, this);
    DoubleClickChoice->Bind(wxEVT_CHOICE, &EffectsGridSettingsPanel::OnChanged, this);
    PasteAsChoice->Bind(wxEVT_CHOICE, &EffectsGridSettingsPanel::OnChanged, this);
}

EffectsGridSettingsPanel::~EffectsGridSettingsPanel()
{
}

bool EffectsGridSettingsPanel::TransferDataToWindow() {
    NodeValuesCheckBox->SetValue(frame->GridNodeValues());
    IconBackgroundsCheckBox->SetValue(frame->GridIconBackgrounds());
    DoubleClickChoice->SetSelection(frame->TimingPlayOnDClick());
    SmallWaveformCheckBox->SetValue(frame->SmallWaveform());
    SnapToTimingCheckBox->SetValue(frame->SnapToTimingMarks());
    TransistionMarksCheckBox->SetValue(!frame->IsSuppressFadeHints());
    ColorUpdateWarnCheckBox->SetValue(frame->SuppressColorWarn());
    GroupEffectIndicator->SetValue(frame->ShowGroupEffectIndicator());
    ShowAlternateTimingFormatCheckBox->SetValue(frame->ShowAlternateTimingFormat());
    BellOnRenderCompletion->SetValue(frame->IsRenderBell());
    PasteAsChoice->SetSelection(frame->IsPasteAsLayers() ? 1 : 0);
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
    frame->SetShowGroupEffectIndicator(GroupEffectIndicator->IsChecked());
    frame->SetShowAlternateTimingFormat(ShowAlternateTimingFormatCheckBox->IsChecked());
    frame->SetRenderBell(BellOnRenderCompletion->IsChecked());
    frame->SetPasteAsLayers(PasteAsChoice->GetSelection() == 1);
    return true;
}

void EffectsGridSettingsPanel::OnChanged(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
