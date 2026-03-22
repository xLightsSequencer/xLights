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

#include "StarPropertyAdapter.h"
#include "../../../models/StarModel.h"
#include "../../../OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
    "Top Ctr-CCW",
    "Top Ctr-CW",
    "Top Ctr-CCW Inside",
    "Top Ctr-CW Inside",
    "Bottom Ctr-CW",
    "Bottom Ctr-CCW",
    "Bottom Ctr-CW Inside",
    "Bottom Ctr-CCW Inside",
    "Left Bottom-CW",
    "Left Bottom-CCW",
    "Right Bottom-CW",
    "Right Bottom-CCW"
};

static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(12, TOP_BOT_LEFT_RIGHT_VALUES));

StarPropertyAdapter::StarPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _star(static_cast<StarModel&>(model)) {}

void StarPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "StarStringCount", _star.GetNumStarStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_star.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "StarLightCount", _star.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "StarLightCount", _star.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxUIntProperty("# Points", "StarStrandCount", _star.GetStarPoints()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    int ssl = 0;
    std::string startLoc = _star.GetStartLocation();
    for (size_t i = 0; i < TOP_BOT_LEFT_RIGHT.GetCount(); i++) {
        if (TOP_BOT_LEFT_RIGHT[i].GetText() == startLoc) {
            ssl = i;
            break;
        }
    }

    grid->Append(new wxEnumProperty("Starting Location", "StarStart", TOP_BOT_LEFT_RIGHT, ssl));
    AddLayerSizeProperty(grid);

    p = grid->Append(new wxFloatProperty("Outer to Inner Ratio", "StarRatio", _star.GetStarRatio()));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    if (_star.GetNumStrands() > 1) {
        p = grid->Append(new wxUIntProperty("Inner Layer %", "StarCenterPercent", _star.GetInnerPercent()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");
    }
}

int StarPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("StarStringCount" == event.GetPropertyName()) {
        _star.SetNumStarStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarPropertyAdapter::OnPropertyGridChange::StarStringCount");
        return 0;
    } else if ("StarLightCount" == event.GetPropertyName()) {
        _star.SetStarNodesPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarPropertyAdapter::OnPropertyGridChange::StarLightCount");
        return 0;
    } else if ("StarStrandCount" == event.GetPropertyName()) {
        _star.SetStarPoints(static_cast<int>(event.GetPropertyValue().GetLong()));
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarPropertyAdapter::OnPropertyGridChange::StarStrandCount");
        return 0;
    } else if ("StarStart" == event.GetPropertyName()) {
        _star.SetStarStartLocation(TOP_BOT_LEFT_RIGHT_VALUES[event.GetValue().GetLong()]);
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "StarPropertyAdapter::OnPropertyGridChange::StarStart");
        return 0;
    } else if ("StarCenterPercent" == event.GetPropertyName()) {
        _star.SetInnerPercent(static_cast<int>(event.GetPropertyValue().GetLong()));
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "StarPropertyAdapter::OnPropertyGridChange::StarCenterPercent");
        return 0;
    } else if ("StarRatio" == event.GetPropertyName()) {
        _star.SetStarRatio(event.GetValue().GetDouble());
        _star.IncrementChangeCount();
        _star.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "StarPropertyAdapter::OnPropertyGridChange::StarRatio");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
