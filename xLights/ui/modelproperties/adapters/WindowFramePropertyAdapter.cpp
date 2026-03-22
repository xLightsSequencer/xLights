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

#include "WindowFramePropertyAdapter.h"
#include "../../../models/WindowFrameModel.h"
#include "../../../OutputModelManager.h"

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = {
    "Top Left",
    "Top Right",
    "Bottom Left",
    "Bottom Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* CLOCKWISE_ANTI_VALUES[] = {
    "Clockwise",
    "Counter Clockwise"
};
static wxPGChoices CLOCKWISE_ANTI(wxArrayString(2, CLOCKWISE_ANTI_VALUES));

WindowFramePropertyAdapter::WindowFramePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _windowFrame(static_cast<WindowFrameModel&>(model)) {}

void WindowFramePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Lights Top", "WFTopCount", _windowFrame.GetParm1()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Lights Left/Right", "WFLeftRightCount", _windowFrame.GetParm2()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Lights Bottom", "WFBottomCount", _windowFrame.GetParm3()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxEnumProperty("Starting Location", "WFStartLocation", TOP_BOT_LEFT_RIGHT,
        _windowFrame.GetIsLtoR() ? (_windowFrame.GetIsBtoT() ? 2 : 0) : (_windowFrame.GetIsBtoT() ? 3 : 1)));

    grid->Append(new wxEnumProperty("Direction", "WFDirection", CLOCKWISE_ANTI, _windowFrame.GetRotation()));
}

int WindowFramePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("WFTopCount" == event.GetPropertyName()) {
        _windowFrame.SetParm1(static_cast<int>(event.GetPropertyValue().GetLong()));
        _windowFrame.IncrementChangeCount();
        _windowFrame.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFramePropertyAdapter::OnPropertyGridChange::WFTopCount");
        return 0;
    } else if ("WFLeftRightCount" == event.GetPropertyName()) {
        _windowFrame.SetParm2(static_cast<int>(event.GetPropertyValue().GetLong()));
        _windowFrame.IncrementChangeCount();
        _windowFrame.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFramePropertyAdapter::OnPropertyGridChange::WFLeftRightCount");
        return 0;
    } else if ("WFBottomCount" == event.GetPropertyName()) {
        _windowFrame.SetParm3(static_cast<int>(event.GetPropertyValue().GetLong()));
        _windowFrame.IncrementChangeCount();
        _windowFrame.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFramePropertyAdapter::OnPropertyGridChange::WFBottomCount");
        return 0;
    } else if ("WFStartLocation" == event.GetPropertyName()) {
        long val = event.GetValue().GetLong();
        _windowFrame.SetDirection((val == 0 || val == 2) ? "L" : "R");
        _windowFrame.SetStartSide((val == 0 || val == 1) ? "T" : "B");
        _windowFrame.SetIsLtoR(val == 0 || val == 2);
        _windowFrame.SetIsBtoT(val >= 2);
        _windowFrame.IncrementChangeCount();
        _windowFrame.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "WindowFramePropertyAdapter::OnPropertyGridChange::WFStartLocation");
        return 0;
    } else if ("WFDirection" == event.GetPropertyName()) {
        _windowFrame.SetRotation(event.GetValue().GetLong());
        _windowFrame.IncrementChangeCount();
        _windowFrame.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "WindowFramePropertyAdapter::OnPropertyGridChange::WFDirection");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
