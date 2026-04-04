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

#include "SpherePropertyAdapter.h"
#include "../../../models/SphereModel.h"
#include "../../../models/OutputModelManager.h"

SpherePropertyAdapter::SpherePropertyAdapter(Model& model)
    : MatrixPropertyAdapter(model), _sphere(static_cast<SphereModel&>(model)) {}

void SpherePropertyAdapter::AddStyleProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->Append(new wxIntProperty("Degrees", "Degrees", _sphere.GetSphereDegrees()));
    p->SetAttribute("Min", "45");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Southern Latitude", "StartLatitude", _sphere.GetStartLatitude()));
    p->SetAttribute("Min", "-89");
    p->SetAttribute("Max", "-1");
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Northern Latitude", "EndLatitude", _sphere.GetEndLatitude()));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "89");
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _sphere.HasAlternateNodes()));
    p->SetEditor("CheckBox");
    p->Enable(_sphere.IsNoZigZag() == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _sphere.IsNoZigZag()));
    p->SetEditor("CheckBox");
    p->Enable(_sphere.HasAlternateNodes() == false);
}

int SpherePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "StartLatitude") {
        _sphere.SetStartLatitude(static_cast<int>(event.GetPropertyValue().GetLong()));
        _sphere.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpherePropertyAdapter::OnPropertyGridChange::StartLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpherePropertyAdapter::OnPropertyGridChange::StartLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpherePropertyAdapter::OnPropertyGridChange::StartLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpherePropertyAdapter::OnPropertyGridChange::StartLatitude");
        return 0;
    } else if (event.GetPropertyName() == "EndLatitude") {
        _sphere.SetEndLatitude(static_cast<int>(event.GetPropertyValue().GetLong()));
        _sphere.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpherePropertyAdapter::OnPropertyGridChange::EndLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpherePropertyAdapter::OnPropertyGridChange::EndLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpherePropertyAdapter::OnPropertyGridChange::EndLatitude");
        _sphere.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpherePropertyAdapter::OnPropertyGridChange::EndLatitude");
        return 0;
    } else if (event.GetPropertyName() == "Degrees") {
        _sphere.SetDegrees(static_cast<int>(event.GetPropertyValue().GetLong()));
        _sphere.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpherePropertyAdapter::OnPropertyGridChange::Degrees");
        _sphere.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpherePropertyAdapter::OnPropertyGridChange::Degrees");
        _sphere.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpherePropertyAdapter::OnPropertyGridChange::Degrees");
        _sphere.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpherePropertyAdapter::OnPropertyGridChange::Degrees");
        return 0;
    } else if (event.GetPropertyName() == "AlternateNodes") {
        _sphere.SetAlternateNodes(event.GetPropertyValue().GetBool());
        _sphere.IncrementChangeCount();
        _sphere.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpherePropertyAdapter::OnPropertyGridChange::AlternateNodes");
        _sphere.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpherePropertyAdapter::OnPropertyGridChange::AlternateNodes");
        _sphere.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpherePropertyAdapter::OnPropertyGridChange::AlternateNodes");
        _sphere.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpherePropertyAdapter::OnPropertyGridChange::AlternateNodes");
        grid->GetPropertyByName("NoZig")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    } else if (event.GetPropertyName() == "NoZig") {
        _sphere.SetNoZigZag(event.GetPropertyValue().GetBool());
        _sphere.IncrementChangeCount();
        _sphere.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpherePropertyAdapter::OnPropertyGridChange::NoZig");
        _sphere.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpherePropertyAdapter::OnPropertyGridChange::NoZig");
        _sphere.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpherePropertyAdapter::OnPropertyGridChange::NoZig");
        _sphere.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpherePropertyAdapter::OnPropertyGridChange::NoZig");
        grid->GetPropertyByName("AlternateNodes")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    }

    return MatrixPropertyAdapter::OnPropertyGridChange(grid, event);
}
