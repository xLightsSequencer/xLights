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

#include "WreathPropertyAdapter.h"
#include "../../../models/WreathModel.h"
#include "../../../OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { "Top Ctr-CCW", "Top Ctr-CW", "Bottom Ctr-CW", "Bottom Ctr-CCW" };
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

WreathPropertyAdapter::WreathPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _wreath(static_cast<WreathModel&>(model)) {}

void WreathPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "WreathStringCount", _wreath.GetNumWreathStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_wreath.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "WreathLightCount", _wreath.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 640);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "WreathLightCount", _wreath.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 640);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    grid->Append(new wxEnumProperty("Starting Location", "WreathStart", TOP_BOT_LEFT_RIGHT,
        _wreath.GetIsLtoR() ? (_wreath.GetIsBtoT() ? 2 : 0) : (_wreath.GetIsBtoT() ? 3 : 1)));
}

int WreathPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("WreathStringCount" == event.GetPropertyName()) {
        _wreath.SetNumWreathStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _wreath.IncrementChangeCount();
        _wreath.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "WreathPropertyAdapter::OnPropertyGridChange::WreathStringCount");
        return 0;
    } else if ("WreathLightCount" == event.GetPropertyName()) {
        _wreath.SetWreathNodesPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _wreath.IncrementChangeCount();
        _wreath.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "WreathPropertyAdapter::OnPropertyGridChange::WreathLightCount");
        return 0;
    } else if ("WreathStart" == event.GetPropertyName()) {
        long val = event.GetValue().GetLong();
        _wreath.SetDirection(val == 0 || val == 2 ? "L" : "R");
        _wreath.SetStartSide(val == 0 || val == 1 ? "T" : "B");
        _wreath.SetIsLtoR(val == 0 || val == 2);
        _wreath.SetIsBtoT(val >= 2);
        _wreath.IncrementChangeCount();
        _wreath.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "WreathPropertyAdapter::OnPropertyGridChange::WreathStart");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
