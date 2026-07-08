/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ViewSettingsPanel.h"
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "xLightsMain.h"
#include <wx/preferences.h>

ViewSettingsPanel::ViewSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    frame(f)
{
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);

    auto addChoiceRow = [&](wxChoice*& choice, const wxString& label, const wxString& hint) {
        grid->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        choice = new wxChoice(this, wxID_ANY);
        grid->Add(choice, 0, wxALL, 5);
        grid->Add(0, 0);
        grid->Add(MakePreferenceHint(this, hint), 0, wxLEFT | wxBOTTOM, 5);
    };

    addChoiceRow(ToolIconSizeChoice, _("Effect Icon Size"), _("Size of the effect icons in the sequencer toolbar."));
    ToolIconSizeChoice->Append(_("Small"));
    ToolIconSizeChoice->SetSelection(ToolIconSizeChoice->Append(_("Medium")));
    ToolIconSizeChoice->Append(_("Large"));
    ToolIconSizeChoice->Append(_("Extra Large"));

    addChoiceRow(ModelHandleSizeChoice, _("Model Handle Size"), _("Size of the drag handles on models in the layout preview."));
    ModelHandleSizeChoice->SetSelection(ModelHandleSizeChoice->Append(_("Normal")));
    ModelHandleSizeChoice->Append(_("Large"));
    ModelHandleSizeChoice->Append(_("Extra Large"));
    ModelHandleSizeChoice->Append(_("Small"));

    addChoiceRow(EffectAssistChoice, _("Effect Assist Window"), _("When the Effect Assist panel is shown (always on/off, or auto for effects that use it)."));
    EffectAssistChoice->Append(_("Always On"));
    EffectAssistChoice->Append(_("Always Off"));
    EffectAssistChoice->SetSelection(EffectAssistChoice->Append(_("Auto Toggle")));

    addChoiceRow(Choice_TimelineZooming, _("Timeline Zooming"), _("Where the timeline zooms toward — the play marker or the mouse position."));
    Choice_TimelineZooming->SetSelection(Choice_TimelineZooming->Append(_("Play Marker Position")));
    Choice_TimelineZooming->Append(_("Mouse Marker Position"));

    addChoiceRow(CrosshairSizeChoice, _("Group Center Crosshair Size"), _("Size of the crosshair marking a group's centre in the layout."));
    CrosshairSizeChoice->Append(_("Large"));
    CrosshairSizeChoice->SetSelection(CrosshairSizeChoice->Append(_("Normal")));
    CrosshairSizeChoice->Append(_("Small"));
    CrosshairSizeChoice->Append(_("Tiny"));
    CrosshairSizeChoice->Append(_("None"));

    addChoiceRow(Choice_PaletteSize, _("Color Palette Size"), _("Size of the colour swatches in the Colors panel."));
    Choice_PaletteSize->SetSelection(Choice_PaletteSize->Append(_("Normal")));
    Choice_PaletteSize->Append(_("Large"));

    sizer->Add(grid, 0, wxEXPAND | wxALL, 5);

    struct Toggle {
        wxCheckBox** ctrl;
        wxString label;
        wxString hint;
        bool defaultValue;
    };
    const Toggle toggles[] = {
        { &PlayControlsCheckBox, _("Show Play Controls on Preview"), _("Overlay play/pause controls on the preview panels."), true },
        { &HousePreviewCheckBox, _("Auto Show House Preview"), _("Automatically open the House Preview when a sequence loads."), true },
        { &CheckBox_BaseShowFolder, _("Enable Base Show Folder Settings"), _("Allow settings to be inherited from a shared base show folder."), false },
        { &CheckBox_PresetPreview, _("Hide Preset Previews"), _("Don't render thumbnail previews in the preset panel."), false },
        { &CheckBox_ZoomMethod, _("Zoom To Cursor"), _("Zoom the sequencer toward the mouse cursor instead of the play position."), true },
        { &CheckBox_DisableKeyAcceleration, _("Disable key acceleration when held down"), _("Stop key repeat from accelerating when a key is held down."), false },
    };
    for (const auto& t : toggles) {
        *t.ctrl = new wxCheckBox(this, wxID_ANY, t.label);
        (*t.ctrl)->SetValue(t.defaultValue);
        sizer->Add(*t.ctrl, 0, wxLEFT | wxTOP, 8);
        sizer->Add(MakePreferenceHint(this, t.hint), 0, wxLEFT | wxBOTTOM, 26);
        (*t.ctrl)->Bind(wxEVT_CHECKBOX, &ViewSettingsPanel::OnChanged, this);
    }

    SetSizer(sizer);
    sizer->SetSizeHints(this);

    for (wxChoice* c : { ToolIconSizeChoice, ModelHandleSizeChoice, EffectAssistChoice,
                         Choice_TimelineZooming, CrosshairSizeChoice, Choice_PaletteSize }) {
        c->Bind(wxEVT_CHOICE, &ViewSettingsPanel::OnChanged, this);
    }

#ifdef _MSC_VER
    MSWDisableComposited();
#endif
}

ViewSettingsPanel::~ViewSettingsPanel()
{
}

bool ViewSettingsPanel::TransferDataToWindow()
{
    HousePreviewCheckBox->SetValue(frame->AutoShowHousePreview());
    PlayControlsCheckBox->SetValue(frame->PlayControlsOnPreview());
    CheckBox_BaseShowFolder->SetValue(frame->IsShowBaseShowFolder());
    int i = frame->EffectAssistMode();
    if (i >= 3) {
        i = 0;
    }
    EffectAssistChoice->SetSelection(i);
    ModelHandleSizeChoice->SetSelection(frame->GetModelHandleSize());
    CrosshairSizeChoice->SetSelection(frame->GetCrosshairSize());
    int ts = frame->ToolIconSize();
    switch (ts) {
    case 48:
        ToolIconSizeChoice->SetSelection(3);
        break;
    case 32:
        ToolIconSizeChoice->SetSelection(2);
        break;
    case 24:
        ToolIconSizeChoice->SetSelection(1);
        break;
    case 16:
    default:
        ToolIconSizeChoice->SetSelection(0);
        break;
    }

    Choice_TimelineZooming->SetSelection(frame->GetTimelineZooming() & 1);
    CheckBox_PresetPreview->SetValue(frame->HidePresetPreview());
    CheckBox_DisableKeyAcceleration->SetValue(frame->IsDisableKeyAcceleration());
    CheckBox_ZoomMethod->SetValue(frame->ZoomMethodToCursor());
    Choice_PaletteSize->SetStringSelection(frame->GetPaletteSizeString());

    return true;
}
bool ViewSettingsPanel::TransferDataFromWindow()
{
    frame->SetModelHandleSize(ModelHandleSizeChoice->GetSelection());
    frame->SetCrosshairSize(CrosshairSizeChoice->GetSelection());
    frame->SetEffectAssistMode(EffectAssistChoice->GetSelection());
    frame->SetPlayControlsOnPreview(PlayControlsCheckBox->IsChecked());
    frame->SetAutoShowHousePreview(HousePreviewCheckBox->IsChecked());
    frame->SetZoomMethodToCursor(CheckBox_ZoomMethod->IsChecked());
    frame->SetShowBaseShowFolder(CheckBox_BaseShowFolder->IsChecked());
    frame->SetDisableKeyAcceleration(CheckBox_DisableKeyAcceleration->IsChecked());
    switch (ToolIconSizeChoice->GetSelection()) {
    case 3:
        frame->SetToolIconSize(48);
        break;
    case 2:
        frame->SetToolIconSize(32);
        break;
    case 1:
        frame->SetToolIconSize(24);
        break;
    case 0:
    default:
        frame->SetToolIconSize(16);
        break;
    }

    frame->SetTimelineZooming(Choice_TimelineZooming->GetSelection());
    frame->SetHidePresetPreview(CheckBox_PresetPreview->IsChecked());
    frame->SetPaletteSizeString(Choice_PaletteSize->GetStringSelection());
    return true;
}

void ViewSettingsPanel::OnChanged(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
