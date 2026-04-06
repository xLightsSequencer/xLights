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

#include "CirclePropertyAdapter.h"
#include "models/CircleModel.h"
#include "models/OutputModelManager.h"

static const char* CIRCLE_START_LOCATION_VALUES[] = {
    "Top Outer-CCW",
    "Top Outer-CW",
    "Top Inner-CCW",
    "Top Inner-CW",
    "Bottom Outer-CCW",
    "Bottom Outer-CW",
    "Bottom Inner-CCW",
    "Bottom Inner-CW"
};
static wxPGChoices CIRCLE_START_LOCATION(wxArrayString(8, CIRCLE_START_LOCATION_VALUES));

CirclePropertyAdapter::CirclePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _circle(static_cast<CircleModel&>(model)) {}

void CirclePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "CircleStringCount", _circle.GetNumCircleStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_circle.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "CircleLightCount", _circle.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "CircleLightCount", _circle.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxUIntProperty("Center %", "CircleCenterPercent", _circle.GetCenterPercent()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    AddLayerSizeProperty(grid);

    int start = _circle.GetIsLtoR() ? 1 : 0;
    if (_circle.IsInsideOut()) {
        start += 2;
    }
    if (_circle.GetIsBtoT()) {
        start += 4;
    }
    grid->Append(new wxEnumProperty("Starting Location", "CircleStart", CIRCLE_START_LOCATION, start));
}

int CirclePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("CircleStringCount" == event.GetPropertyName()) {
        _circle.SetNumCircleStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _circle.IncrementChangeCount();
        _circle.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CirclePropertyAdapter::OnPropertyGridChange::CircleStringCount");
        return 0;
    } else if ("CircleLightCount" == event.GetPropertyName()) {
        _circle.SetCircleNodesPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _circle.IncrementChangeCount();
        _circle.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CirclePropertyAdapter::OnPropertyGridChange::CircleLightCount");
        return 0;
    } else if ("CircleCenterPercent" == event.GetPropertyName()) {
        _circle.SetCenterPercent(static_cast<int>(event.GetPropertyValue().GetLong()));
        _circle.IncrementChangeCount();
        _circle.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CirclePropertyAdapter::OnPropertyGridChange::CircleCenterPercent");
        return 0;
    } else if ("CircleStart" == event.GetPropertyName()) {
        int v = event.GetValue().GetLong();
        _circle.SetDirection(v & 0x1 ? "L" : "R");
        _circle.SetStartSide(v < 4 ? "T" : "B");
        _circle.SetIsLtoR(v & 0x1);
        _circle.SetIsBtoT(v >= 4);
        _circle.SetInsideOut((v & 0x2) != 0);
        _circle.IncrementChangeCount();
        _circle.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CirclePropertyAdapter::OnPropertyGridChange::CircleStart");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
