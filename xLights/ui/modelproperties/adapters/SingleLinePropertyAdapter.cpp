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

#include "SingleLinePropertyAdapter.h"
#include "../../../models/SingleLineModel.h"
#include "../../../models/OutputModelManager.h"

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

SingleLinePropertyAdapter::SingleLinePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _singleLine(static_cast<SingleLineModel&>(model)) {}

void SingleLinePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "SingleLineCount", _singleLine.GetNumLines()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_singleLine.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "SingleLineNodes", _singleLine.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "SingleLineNodes", _singleLine.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");

        p = grid->Append(new wxUIntProperty("Lights/Node", "SingleLineLights", _singleLine.GetLightsPerNode()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 300);
        p->SetEditor("SpinCtrl");
    }

    grid->Append(new wxEnumProperty("Starting Location", "SingleLineStart", LEFT_RIGHT, _singleLine.GetIsLtoR() ? 0 : 1));
}

int SingleLinePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    _singleLine.IncrementChangeCount();
    if ("SingleLineCount" == event.GetPropertyName()) {
        _singleLine.SetNumLines(static_cast<int>(event.GetPropertyValue().GetLong()));
        _singleLine.IncrementChangeCount();
        _singleLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SingleLinePropertyAdapter::OnPropertyGridChange::SingleLineCount");
        return 0;
    } else if ("SingleLineNodes" == event.GetPropertyName()) {
        _singleLine.SetNodesPerLine(static_cast<int>(event.GetPropertyValue().GetLong()));
        _singleLine.IncrementChangeCount();
        _singleLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SingleLinePropertyAdapter::OnPropertyGridChange::SingleLineNodes");
        return 0;
    } else if ("SingleLineLights" == event.GetPropertyName()) {
        _singleLine.SetLightsPerNode(static_cast<int>(event.GetPropertyValue().GetLong()));
        _singleLine.IncrementChangeCount();
        _singleLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SingleLinePropertyAdapter::OnPropertyGridChange::SingleLineLights");
        return 0;
    } else if ("SingleLineStart" == event.GetPropertyName()) {
        _singleLine.SetDirection(event.GetValue().GetLong() == 0 ? "L" : "R");
        _singleLine.SetIsLtoR(event.GetValue().GetLong() == 0);
        _singleLine.IncrementChangeCount();
        _singleLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "SingleLinePropertyAdapter::OnPropertyGridChange::SingleLineStart");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
