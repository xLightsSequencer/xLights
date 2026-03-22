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

#include "ArchesPropertyAdapter.h"
#include "../ScreenLocationPropertyHelper.h"
#include "../../../models/ArchesModel.h"
#include "../../../models/ThreePointScreenLocation.h"
#include "../../../OutputModelManager.h"

static const char* LEFT_RIGHT_VALUES[] = {
    "Green Square",
    "Blue Square"
};
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

static const char* LEFT_RIGHT_INSIDE_OUTSIDE_VALUES[] = {
    "Green Square Inside",
    "Green Square Outside",
    "Blue Square Inside",
    "Blue Square Outside"
};
static wxPGChoices LEFT_RIGHT_INSIDE_OUTSIDE(wxArrayString(4, LEFT_RIGHT_INSIDE_OUTSIDE_VALUES));

ArchesPropertyAdapter::ArchesPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _arches(static_cast<ArchesModel&>(model)) {}

void ArchesPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxBoolProperty("Layered Arches", "LayeredArches", _arches.GetLayerSizeCount() != 0));
    p->SetEditor("CheckBox");

    if (_arches.GetLayerSizeCount() == 0) {
        p = grid->Append(new wxUIntProperty("# Arches", "ArchesCount", _arches.GetParm1()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Nodes Per Arch", "ArchesNodes", _arches.GetParm2()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes", "ArchesNodes", _arches.GetParm2()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        AddLayerSizeProperty(grid);

        p = grid->Append(new wxUIntProperty("Hollow %", "Hollow", _arches.GetHollow()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 95);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxBoolProperty("Zig-Zag Layers", "ZigZag", _arches.GetZigZag()));
        p->SetEditor("CheckBox");
    }

    p = grid->Append(new wxUIntProperty("Lights Per Node", "ArchesLights", _arches.GetParm3()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc Degrees", "ArchesArc", _arches.GetArc()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Arch Tilt", "ArchesSkew", dynamic_cast<ThreePointScreenLocation&>(_arches.GetModelScreenLocation()).GetAngle()));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    if (_arches.GetLayerSizeCount() == 0) {
        p = grid->Append(new wxIntProperty("Gap Between Arches", "ArchesGap", _arches.GetGap()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetEditor("SpinCtrl");
    }

    if (_arches.GetLayerSizeCount() != 0) {
        grid->Append(new wxEnumProperty("Starting Location", "ArchesStart", LEFT_RIGHT_INSIDE_OUTSIDE,
            (_arches.GetIsLtoR() ? 0 : 2) + (_arches.GetIsBtoT() ? 1 : 0)));
    } else {
        grid->Append(new wxEnumProperty("Starting Location", "ArchesStart", LEFT_RIGHT, _arches.GetIsLtoR() ? 0 : 1));
    }
}

void ArchesPropertyAdapter::AddDimensionProperties(wxPropertyGridInterface* grid) {
    if (_arches.GetLayerSizeCount() == 0 && _arches.GetParm1() != 0) {
        ScreenLocationPropertyHelper::AddDimensionProperties(_arches.GetModelScreenLocation(), grid, 1.0 / _arches.GetParm1());
    } else {
        ScreenLocationPropertyHelper::AddDimensionProperties(_arches.GetModelScreenLocation(), grid, 1.0);
    }
}

int ArchesPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("ArchesCount" == event.GetPropertyName()) {
        _arches.SetParm1(static_cast<int>(event.GetPropertyValue().GetLong()));
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_UPDATE_PROPERTYGRID |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesCount");
        return 0;
    } else if ("ArchesNodes" == event.GetPropertyName()) {
        _arches.SetParm2(static_cast<int>(event.GetPropertyValue().GetLong()));
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesNodes");
        return 0;
    } else if ("ArchesLights" == event.GetPropertyName()) {
        _arches.SetParm3(static_cast<int>(event.GetPropertyValue().GetLong()));
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesLights");
        return 0;
    } else if ("ArchesArc" == event.GetPropertyName()) {
        _arches.SetArc(static_cast<int>(event.GetPropertyValue().GetLong()));
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesArc");
        return 0;
    } else if ("ArchesSkew" == event.GetPropertyName()) {
        dynamic_cast<ThreePointScreenLocation&>(_arches.GetModelScreenLocation()).SetAngle(event.GetPropertyValue().GetLong());
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesSkew");
        return 0;
    } else if ("LayeredArches" == event.GetPropertyName()) {
        if (event.GetPropertyValue().GetBool()) {
            _arches.SetParm1(1);
            _arches.SetLayerSizeCount(1);
            _arches.SetLayerSize(0, _arches.GetParm2());
        } else {
            _arches.SetLayerSizeCount(0);
        }
        _arches.OnLayerSizesChange(true);
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesPropertyAdapter::HandleLayerSizePropertyChange::LayeredArches");
        return 0;
    } else if ("ZigZag" == event.GetPropertyName()) {
        _arches.SetZigZag(event.GetPropertyValue().GetBool());
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesZigZag");
        return 0;
    } else if ("Hollow" == event.GetPropertyName()) {
        _arches.SetHollow(event.GetPropertyValue().GetLong());
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesHollow");
        return 0;
    } else if ("ArchesGap" == event.GetPropertyName()) {
        _arches.SetGap(event.GetPropertyValue().GetLong());
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesGap");
        return 0;
    } else if ("ArchesStart" == event.GetPropertyName()) {
        int value = event.GetValue().GetLong();
        if (_arches.GetLayerSizeCount() != 0) {
            _arches.SetDirection((value == 0 || value == 1) ? "L" : "R");
            _arches.SetStartSide((value == 0 || value == 2) ? "T" : "B");
            _arches.SetIsBtoT(value != 0 && value != 2);
        } else {
            _arches.SetDirection(value == 0 ? "L" : "R");
        }
        _arches.IncrementChangeCount();
        _arches.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ArchesPropertyAdapter::OnPropertyGridChange::ArchesStart");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
