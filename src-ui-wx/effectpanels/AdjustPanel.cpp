/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AdjustPanel.h"

#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

AdjustPanel::AdjustPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    auto* actionCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Action", this));
    if (actionCtrl) actionCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    ValidateWindow();
}

void AdjustPanel::ValidateWindow() {
    auto* actionCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Action", this));
    auto* spin1 = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Value1", this));
    auto* spin2 = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Value2", this));
    auto* label1 = dynamic_cast<wxStaticText*>(wxWindow::FindWindowByName("ID_STATICTEXT_Value1", this));
    auto* label2 = dynamic_cast<wxStaticText*>(wxWindow::FindWindowByName("ID_STATICTEXT_Value2", this));
    if (!actionCtrl || !spin1 || !spin2 || !label1 || !label2) return;

    auto action = actionCtrl->GetStringSelection().ToStdString();

    if (action == "None") {
        label1->SetLabel("Unused:");
        spin1->Disable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Adjust By Value") {
        label1->SetLabel("Adjust by:");
        spin1->SetRange(-255, 255);
        spin1->Enable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Adjust By Percentage") {
        label1->SetLabel("Adjust by:");
        spin1->SetRange(-100, 25500);
        spin1->Enable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Set Minimum") {
        label1->SetLabel("Minimum:");
        spin1->SetRange(0, 255);
        spin1->Enable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Set Maximum") {
        label1->SetLabel("Maximum:");
        spin1->SetRange(0, 255);
        spin1->Enable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Set Range") {
        label1->SetLabel("Minimum:");
        spin1->SetRange(0, 255);
        spin1->Enable();
        label2->SetLabel("Maximum:");
        spin2->SetRange(0, 255);
        spin2->Enable();
    } else if (action == "Shift With Wrap By Value") {
        label1->SetLabel("Shift by:");
        spin1->SetRange(-255, 255);
        spin1->Enable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    } else if (action == "Prevent Range") {
        label1->SetLabel("Minimum:");
        spin1->SetRange(0, 255);
        spin1->Enable();
        label2->SetLabel("Maximum:");
        spin2->SetRange(0, 255);
        spin2->Enable();
    } else if (action == "Reverse") {
        label1->SetLabel("Unused:");
        spin1->Disable();
        label2->SetLabel("Unused:");
        spin2->Disable();
    }
}
