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
#include <wx/settings.h>

#include "MatrixPropertyAdapter.h"
#include "../../../models/MatrixModel.h"
#include "../../../OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
    "Top Left",
    "Top Right",
    "Bottom Left",
    "Bottom Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* MATRIX_STYLES_VALUES[] = {
    "Horizontal",
    "Vertical"
};
static wxPGChoices MATRIX_STYLES(wxArrayString(2, MATRIX_STYLES_VALUES));

MatrixPropertyAdapter::MatrixPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _matrix(static_cast<MatrixModel&>(model)) {}

void MatrixPropertyAdapter::AddStyleProperties(wxPropertyGridInterface* grid) {
    grid->Append(new wxEnumProperty("Direction", "MatrixStyle", MATRIX_STYLES, _matrix.isVerticalMatrix() ? 1 : 0));
    wxPGProperty* p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _matrix.HasAlternateNodes()));
    p->SetEditor("CheckBox");
    p->Enable(_matrix.IsNoZigZag() == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _matrix.IsNoZigZag()));
    p->SetEditor("CheckBox");
    p->Enable(_matrix.HasAlternateNodes() == false);
}

void MatrixPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    AddStyleProperties(grid);

    wxPGProperty* p = grid->Append(new wxUIntProperty("# Strings", "MatrixStringCount", _matrix.GetNumPhysicalStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 10000);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller. *This would also be the 'Height' of a Horizontal Virtual Matrix.");

    if (_matrix.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights/String", "MatrixLightCount", _matrix.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "MatrixLightCount", _matrix.GetNodesPerString()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String. \n *This would also be the 'Width' of a Horizontal Virtual Matrix.");
    }

    p = grid->Append(new wxUIntProperty("Strands/String", "MatrixStrandCount", _matrix.GetStrandsPerString()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically how many times the #String ZigZags.");
    if (_matrix.GetNodesPerString() % _matrix.GetStrandsPerString() != 0) {
        p->SetBackgroundColour(*wxRED);
        p->SetHelpString("Strands/String must divide into Nodes/String evenly.");
    } else {
        p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }

    grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT,
        _matrix.GetIsLtoR() ? (_matrix.GetIsBtoT() ? 2 : 0) : (_matrix.GetIsBtoT() ? 3 : 1)));
}

int MatrixPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("MatrixStyle" == event.GetPropertyName()) {
        _matrix.SetVertical(event.GetPropertyValue().GetLong());
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "MatrixPropertyAdapter::OnPropertyGridChange::MatrixStyle");
        return 0;
    } else if ("MatrixStringCount" == event.GetPropertyName()) {
        _matrix.SetNumMatrixStrings(static_cast<int>(event.GetPropertyValue().GetLong()));
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixPropertyAdapter::OnPropertyGridChange::MatrixStringCount");
        return 0;
    } else if ("MatrixLightCount" == event.GetPropertyName()) {
        _matrix.SetNodesPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixPropertyAdapter::OnPropertyGridChange::MatrixLightCount");
        return 0;
    } else if ("MatrixStrandCount" == event.GetPropertyName()) {
        _matrix.SetStrandsPerString(static_cast<int>(event.GetPropertyValue().GetLong()));
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixPropertyAdapter::OnPropertyGridChange::MatrixStrandCount");
        return 0;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        long val = event.GetValue().GetLong();
        _matrix.SetDirection(val == 0 || val == 2 ? "L" : "R");
        _matrix.SetStartSide(val == 0 || val == 1 ? "T" : "B");
        _matrix.SetIsLtoR(val == 0 || val == 2);
        _matrix.SetIsBtoT(val >= 2);
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "MatrixPropertyAdapter::OnPropertyGridChange::MatrixStart");
        return 0;
    } else if (event.GetPropertyName() == "AlternateNodes") {
        _matrix.SetAlternateNodes(event.GetPropertyValue().GetBool());
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "MatrixPropertyAdapter::OnPropertyGridChange::AlternateNodes");
        grid->GetPropertyByName("NoZig")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    } else if (event.GetPropertyName() == "NoZig") {
        _matrix.SetNoZigZag(event.GetPropertyValue().GetBool());
        _matrix.IncrementChangeCount();
        _matrix.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "MatrixPropertyAdapter::OnPropertyGridChange::NoZig");
        grid->GetPropertyByName("AlternateNodes")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
