/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CheckSequenceSettingsPanel.h"
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include <wx/preferences.h>
#include "xLightsMain.h"

CheckSequenceSettingsPanel::CheckSequenceSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id,const wxPoint& pos,const wxSize& size) : frame(f)
{
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* intro = new wxStaticText(this, wxID_ANY,
        _("Disabling check sequence checks can reduce clutter in your check sequence results, "
          "but can also mask causes of issues such as incorrect pixels lighting up or slow "
          "rendering. Please make sure you understand what each option does before disabling "
          "it on someone's advice."));
    intro->Wrap(560);
    sizer->Add(intro, 0, wxALL, 8);

    struct Item {
        wxCheckBox** ctrl;
        wxString label;
        wxString hint;
    };
    const Item items[] = {
        { &CheckBox_DupUniv, _("Disable checks on duplicate use of universe/id across controllers."),
          _("If you are using unicast and not using #universe:startchannel addressing then this check can be disabled.") },
        { &CheckBox_NonContigChOnPort, _("Disable checks for non-contiguous channels on controller ports."),
          _("If you only use controllers that support virtual strings then this check can be safely disabled.") },
        { &CheckBox_PreviewGroup, _("Disable checks for groups containing models from different previews."),
          _("Adding models to groups from different previews can make them appear in other views with no obvious reason why.") },
        { &CheckBox_DupNodeMG, _("Disable checks for duplicate nodes in model groups."),
          _("Duplicate nodes in model groups can light pixels during effects at unexpected times.") },
        { &CheckBox_TransTime, _("Disable transition time checking."),
          _("Transition times that overlap or extend beyond the effect duration can lead to unexpected dimming.") },
        { &CheckBox_CustomSizeCheck, _("Disable custom model size checking."),
          _("Large custom models with mostly empty cells add significant rendering overhead; consider shrinking them.") },
        { &CheckBox_DisableSketch, _("Disable sketch image file checking."),
          _("Sketch effect image files are not essential to rendering.") },
    };

    for (const auto& it : items) {
        *it.ctrl = new wxCheckBox(this, wxID_ANY, it.label);
        sizer->Add(*it.ctrl, 0, wxLEFT | wxTOP, 8);
        sizer->Add(MakePreferenceHint(this, it.hint), 0, wxLEFT | wxBOTTOM, 26);
        (*it.ctrl)->Bind(wxEVT_CHECKBOX, &CheckSequenceSettingsPanel::OnChanged, this);
    }

    SetSizer(sizer);
    sizer->SetSizeHints(this);
}

CheckSequenceSettingsPanel::~CheckSequenceSettingsPanel()
{
}

bool CheckSequenceSettingsPanel::TransferDataToWindow() {
    CheckBox_DupUniv->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("DupUniv"));
    CheckBox_NonContigChOnPort->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("NonContigChOnPort"));
    CheckBox_PreviewGroup->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("PreviewGroup"));
    CheckBox_DupNodeMG->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("DupNodeMG"));
    CheckBox_TransTime->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("TransTime"));
    CheckBox_CustomSizeCheck->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("CustomSizeCheck"));
    CheckBox_DisableSketch->SetValue(xLightsFrame::IsCheckSequenceOptionDisabledS("SketchImage"));
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

void CheckSequenceSettingsPanel::OnChanged(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
