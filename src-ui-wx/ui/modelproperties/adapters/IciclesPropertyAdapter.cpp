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

#include "IciclesPropertyAdapter.h"
#include "../ScreenLocationPropertyHelper.h"
#include "models/IciclesModel.h"
#include "models/ModelScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
#include "models/OutputModelManager.h"
#include "../PropertyGridHelpers.h"

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

IciclesPropertyAdapter::IciclesPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _icicles(static_cast<IciclesModel&>(model)) {}

void IciclesPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "IciclesStrings", _icicles.GetNumIcicleStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_icicles.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "IciclesLights", _icicles.GetLightsPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "IciclesLights", _icicles.GetLightsPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _icicles.HasAlternateNodes()));
    p->SetEditor("CheckBox");

    grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", _icicles.GetDropPattern()));

    grid->Append(new wxEnumProperty("Starting Location", "IciclesStart", LEFT_RIGHT, _icicles.GetIsLtoR() ? 0 : 1));
}

int IciclesPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("IciclesStrings" == event.GetPropertyName()) {
        _icicles.SetNumIcicleStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _icicles.IncrementChangeCount();
        _icicles.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesPropertyAdapter::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesLights" == event.GetPropertyName()) {
        _icicles.SetLightsPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _icicles.IncrementChangeCount();
        _icicles.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesPropertyAdapter::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesDrops" == event.GetPropertyName()) {
        _icicles.SetDropPattern(event.GetPropertyValue().GetString());
        _icicles.IncrementChangeCount();
        _icicles.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "IciclesPropertyAdapter::OnPropertyGridChange::IciclesDrops");
        return 0;
    } else if ("IciclesStart" == event.GetPropertyName()) {
        _icicles.SetDirection(event.GetValue().GetLong() == 0 ? "L" : "R");
        _icicles.SetIsLtoR(event.GetValue().GetLong() == 0);
        _icicles.IncrementChangeCount();
        _icicles.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "IciclesPropertyAdapter::OnPropertyGridChange::IciclesStart");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        _icicles.SetAlternateNodes(event.GetPropertyValue().GetBool());
        _icicles.IncrementChangeCount();
        _icicles.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "IciclesPropertyAdapter::OnPropertyGridChange::AlternateNodes");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

void IciclesPropertyAdapter::AddDimensionProperties(wxPropertyGridInterface* grid) {
    ScreenLocationPropertyHelper::AddDimensionProperties(_icicles.GetModelScreenLocation(), grid, 1.0);
}
