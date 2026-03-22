/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "SpinnerPropertyAdapter.h"
#include "../../../models/SpinnerModel.h"
#include "../../../OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
    "Center Counter Clockwise",
    "Center Clockwise",
    "End Counter Clockwise",
    "End Clockwise",
    "Center Alternate Counter Clockwise",
    "Center Alternate Clockwise"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(6, TOP_BOT_LEFT_RIGHT_VALUES));

SpinnerPropertyAdapter::SpinnerPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _spinner(static_cast<SpinnerModel&>(model)) {}

void SpinnerPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "SpinnerStringCount", _spinner.GetNumSpinnerStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    p = grid->Append(new wxUIntProperty("Arms/String", "FoldCount", _spinner.GetArmsPerString()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Lights/Arm", "SpinnerArmNodeCount", _spinner.GetNodesPerArm()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Hollow %", "Hollow", _spinner.GetHollowPercent()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 80);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Start Angle", "StartAngle", _spinner.GetStartAngle()));
    p->SetAttribute("Min", -360);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc", "Arc", _spinner.GetArcAngle()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT, _spinner.EncodeStartLocation()));

    p = grid->Append(new wxBoolProperty("Zig-Zag Start", "ZigZag", _spinner.HasZigZag()));
    p->SetEditor("CheckBox");
    p->Enable(_spinner.HasAlternateNodes() == false);
}

void SpinnerPropertyAdapter::UpdateTypeProperties(wxPropertyGridInterface* grid) {
    grid->GetPropertyByName("ZigZag")->Enable(_spinner.HasAlternateNodes() == false);
}

int SpinnerPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("SpinnerStringCount" == event.GetPropertyName()) {
        _spinner.SetNumSpinnerStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerPropertyAdapter::OnPropertyGridChange::SpinnerStringCount");
        return 0;
    } else if ("SpinnerArmNodeCount" == event.GetPropertyName()) {
        _spinner.SetNodesPerArm(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerPropertyAdapter::OnPropertyGridChange::SpinnerArmNodeCount");
        return 0;
    } else if ("FoldCount" == event.GetPropertyName()) {
        _spinner.SetArmsPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerPropertyAdapter::OnPropertyGridChange::FoldCount");
        return 0;
    } else if ("Hollow" == event.GetPropertyName()) {
        _spinner.SetHollow(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SpinnerPropertyAdapter::OnPropertyGridChange::Hollow");
        return 0;
    } else if ("StartAngle" == event.GetPropertyName()) {
        _spinner.SetStartAngle(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SpinnerPropertyAdapter::OnPropertyGridChange::StartAngle");
        return 0;
    } else if ("Arc" == event.GetPropertyName()) {
        _spinner.SetArc(static_cast<int>(event.GetPropertyValue().GetLong()));
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SpinnerPropertyAdapter::OnPropertyGridChange::Arc");
        return 0;
    } else if ("ZigZag" == event.GetPropertyName()) {
        _spinner.SetZigZag(event.GetPropertyValue().GetBool());
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SpinnerPropertyAdapter::OnPropertyGridChange::ZigZag");
        return 0;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        _spinner.DecodeStartLocation(event.GetValue().GetLong());
        _spinner.SetDirection(_spinner.GetIsLtoR() ? "L" : "R");
        _spinner.SetStartSide(_spinner.GetIsBtoT() ? "B" : "T");
        _spinner.IncrementChangeCount();
        _spinner.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SpinnerPropertyAdapter::OnPropertyGridChange::MatrixStart");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
