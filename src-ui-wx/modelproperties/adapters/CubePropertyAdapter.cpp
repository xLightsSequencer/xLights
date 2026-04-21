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

#include "CubePropertyAdapter.h"
#include "models/CubeModel.h"
#include "models/OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
    "Front Bottom Left",
    "Front Bottom Right",
    "Front Top Left",
    "Front Top Right",
    "Back Bottom Left",
    "Back Bottom Right",
    "Back Top Left",
    "Back Top Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(8, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* CUBE_STYLES_VALUES[] = {
    "Vertical Front/Back",
    "Vertical Left/Right",
    "Horizontal Front/Back",
    "Horizontal Left/Right",
    "Stacked Front/Back",
    "Stacked Left/Right"
};
static wxPGChoices CUBE_STYLES(wxArrayString(6, CUBE_STYLES_VALUES));

static const char* STRAND_STYLES_VALUES[] = {
    "Zig Zag",
    "No Zig Zag",
    "Aternate Pixel"
};
static wxPGChoices STRAND_STYLES(wxArrayString(3, STRAND_STYLES_VALUES));

CubePropertyAdapter::CubePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _cube(static_cast<CubeModel&>(model)) {}

void CubePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    grid->Append(new wxEnumProperty("Starting Location", "CubeStart", TOP_BOT_LEFT_RIGHT, _cube.GetCubeStartIndex()));
    grid->Append(new wxEnumProperty("Direction", "CubeStyle", CUBE_STYLES, _cube.GetCubeStyleIndex()));
    grid->Append(new wxEnumProperty("Strand Style", "StrandPerLine", STRAND_STYLES, _cube.GetStrandStyleIndex()));
    auto p = grid->Append(new wxBoolProperty("Layers All Start in Same Place", "StrandPerLayer", _cube.IsStrandPerLayer()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Width", "CubeWidth", _cube.GetCubeWidth()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Height", "CubeHeight", _cube.GetCubeHeight()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Depth", "CubeDepth", _cube.GetCubeDepth()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Strings", "CubeStrings", _cube.GetCubeStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");
}

int CubePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("CubeStart" == event.GetPropertyName()) {
        _cube.SetCubeStartIndex(static_cast<int>(event.GetValue().GetLong()));
        _cube.IncrementChangeCount();
        _cube.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CubePropertyAdapter::OnPropertyGridChange::CubeStart");
        return 0;
    } else if ("CubeStyle" == event.GetPropertyName()) {
        _cube.SetCubeStyleIndex(static_cast<int>(event.GetPropertyValue().GetLong()));
        _cube.IncrementChangeCount();
        _cube.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CubePropertyAdapter::OnPropertyGridChange::CubeStyle");
        return 0;
    } else if ("StrandPerLine" == event.GetPropertyName()) {
        _cube.SetStrandStyleIndex(static_cast<int>(event.GetPropertyValue().GetLong()));
        _cube.IncrementChangeCount();
        _cube.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CubePropertyAdapter::OnPropertyGridChange::StrandPerLine");
        return 0;
    } else if ("StrandPerLayer" == event.GetPropertyName()) {
        _cube.SetStrandPerLayer(event.GetPropertyValue().GetBool());
        _cube.IncrementChangeCount();
        _cube.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CubePropertyAdapter::OnPropertyGridChange::StrandPerLayer");
        return 0;
    } else if ("CubeWidth" == event.GetPropertyName()) {
        _cube.SetCubeWidth(static_cast<int>(event.GetPropertyValue().GetLong()));
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubePropertyAdapter::OnPropertyGridChange::CubeWidth");
        return 0;
    } else if ("CubeHeight" == event.GetPropertyName()) {
        _cube.SetCubeHeight(static_cast<int>(event.GetPropertyValue().GetLong()));
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubePropertyAdapter::OnPropertyGridChange::CubeHeight");
        return 0;
    } else if ("CubeDepth" == event.GetPropertyName()) {
        _cube.SetCubeDepth(static_cast<int>(event.GetPropertyValue().GetLong()));
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CubePropertyAdapter::OnPropertyGridChange::CubeDepth");
        return 0;
    } else if ("CubeStrings" == event.GetPropertyName()) {
        _cube.SetCubeStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _cube.IncrementChangeCount();
        _cube.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CubePropertyAdapter::OnPropertyGridChange::CubeStrings");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
