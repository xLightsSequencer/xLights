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

#include "CandyCanePropertyAdapter.h"
#include "../ScreenLocationPropertyHelper.h"
#include "../../../models/CandyCaneModel.h"
#include "../../../models/ThreePointScreenLocation.h"
#include "../../../OutputModelManager.h"

static const char* LEFT_RIGHT_VALUES[] = {
    "Green Square",
    "Blue Square"
};
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

CandyCanePropertyAdapter::CandyCanePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _candyCane(static_cast<CandyCaneModel&>(model)) {}

void CandyCanePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Canes", "CandyCaneCount", _candyCane.GetNumCanes()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 20);
    p->SetEditor("SpinCtrl");

    if (_candyCane.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("Lights Per Cane", "CandyCaneNodes", _candyCane.GetLightsPerNode()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes Per Cane", "CandyCaneNodes", _candyCane.GetNodesPerCane()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 250);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("Lights Per Node", "CandyCaneLights", _candyCane.GetLightsPerNode()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");
    if (_candyCane.IsSingleNode()) {
        p->Hide(true);
    }

    p = grid->Append(new wxFloatProperty("Height", "CandyCaneHeight", _candyCane.GetCandyCaneHeight()));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Cane Rotation", "CandyCaneSkew", dynamic_cast<ThreePointScreenLocation&>(_candyCane.GetModelScreenLocation()).GetAngle()));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Reverse", "CandyCaneReverse", _candyCane.IsReverse()));
    p->SetEditor("CheckBox");
    p->Enable(!_candyCane.IsSticks());

    p = grid->Append(new wxBoolProperty("Sticks", "CandyCaneSticks", _candyCane.IsSticks()));
    p->SetEditor("CheckBox");

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _candyCane.HasAlternateNodes()));
    p->SetEditor("CheckBox");
    if (_candyCane.IsSingleNode()) {
        p->Enable(false);
    }

    grid->Append(new wxEnumProperty("Starting Location", "CandyCaneStart", LEFT_RIGHT, _candyCane.GetIsLtoR() ? 0 : 1));
}

void CandyCanePropertyAdapter::UpdateTypeProperties(wxPropertyGridInterface* grid) {
    if (_candyCane.IsSingleNode()) {
        grid->GetPropertyByName("CandyCaneLights")->Hide(true);
        grid->GetPropertyByName("AlternateNodes")->Enable(false);
    } else {
        grid->GetPropertyByName("CandyCaneLights")->Hide(false);
        grid->GetPropertyByName("AlternateNodes")->Enable();
    }

    grid->GetPropertyByName("CandyCaneReverse")->Enable(!_candyCane.IsSticks());
}

void CandyCanePropertyAdapter::AddDimensionProperties(wxPropertyGridInterface* grid) {
    if (_candyCane.GetNumCanes() != 0) {
        ScreenLocationPropertyHelper::AddDimensionProperties(_candyCane.GetModelScreenLocation(), grid, 6.0 / _candyCane.GetNumCanes());
    } else {
        ScreenLocationPropertyHelper::AddDimensionProperties(_candyCane.GetModelScreenLocation(), grid, 6.0);
    }
}

int CandyCanePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("CandyCaneCount" == event.GetPropertyName()) {
        _candyCane.SetNumCanes(static_cast<int>(event.GetPropertyValue().GetLong()));
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        _candyCane.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        _candyCane.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        _candyCane.AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneCount");
        return 0;
    } else if ("CandyCaneNodes" == event.GetPropertyName()) {
        _candyCane.SetNodesPerCane(static_cast<int>(event.GetPropertyValue().GetLong()));
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneNodes");
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneNodes");
        _candyCane.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneNodes");
        _candyCane.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneNodes");
        return 0;
    } else if ("CandyCaneLights" == event.GetPropertyName()) {
        _candyCane.SetLightsPerNode(static_cast<int>(event.GetPropertyValue().GetLong()));
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneLights");
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneLights");
        return 0;
    } else if ("CandyCaneReverse" == event.GetPropertyName()) {
        _candyCane.SetReverse(event.GetPropertyValue().GetBool());
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneReverse");
        return 0;
    } else if ("CandyCaneSkew" == event.GetPropertyName()) {
        dynamic_cast<ThreePointScreenLocation&>(_candyCane.GetModelScreenLocation()).SetAngle(event.GetPropertyValue().GetLong());
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneSkew");
        return 0;
    } else if ("CandyCaneHeight" == event.GetPropertyName()) {
        _candyCane.SetCaneHeight(event.GetPropertyValue().GetDouble());
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneHeight");
        return 0;
    } else if ("CandyCaneSticks" == event.GetPropertyName()) {
        _candyCane.SetSticks(event.GetPropertyValue().GetBool());
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneSticks");
        _candyCane.AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneSticks");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        _candyCane.SetAlternateNodes(event.GetPropertyValue().GetBool());
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::AlternateNodes");
        return 0;
    } else if ("CandyCaneStart" == event.GetPropertyName()) {
        _candyCane.SetDirection(event.GetValue().GetLong() == 0 ? "L" : "R");
        _candyCane.SetIsLtoR(event.GetValue().GetLong() == 0);
        _candyCane.IncrementChangeCount();
        _candyCane.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "CandyCanePropertyAdapter::OnPropertyGridChange::CandyCaneStart");
    } else if (event.GetPropertyName() == "ModelStringType") {
        wxPGProperty* p = grid->GetPropertyByName("CandyCaneLights");
        p->Hide(_candyCane.IsSingleNode());
        p = grid->GetPropertyByName("CandyCaneNodes");
        if (_candyCane.IsSingleNode()) {
            p->SetLabel("Lights Per Cane");
        } else {
            p->SetLabel("Nodes Per Cane");
        }
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
