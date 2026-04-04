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

#include "ViewObjectPropertyAdapter.h"
#include "ScreenLocationPropertyHelper.h"
#include "../../models/ViewObject.h"
#include "../../models/DisplayAsType.h"
#include "../../models/OutputModelManager.h"

ViewObjectPropertyAdapter::ViewObjectPropertyAdapter(ViewObject& obj) : _viewObject(obj) {}

void ViewObjectPropertyAdapter::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty *p;
    grid->Append(new wxPropertyCategory(DisplayAsTypeToString(_viewObject.GetDisplayAs()), "ModelType"));
    p = grid->Append(new wxBoolProperty("Active", "Active", _viewObject.IsActive()));
    p->SetAttribute("UseCheckbox", true);

    AddTypeProperties(grid, outputManager);
}

void ViewObjectPropertyAdapter::UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    UpdateTypeProperties(grid);
}

void ViewObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
}

void ViewObjectPropertyAdapter::UpdateTypeProperties(wxPropertyGridInterface* grid) {
}

void ViewObjectPropertyAdapter::AddSizeLocationProperties(wxPropertyGridInterface* grid) {
    ScreenLocationPropertyHelper::AddSizeLocationProperties(_viewObject.GetObjectScreenLocation(), grid);
}

void ViewObjectPropertyAdapter::AddDimensionProperties(wxPropertyGridInterface* grid) {
}

int ViewObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "Active") {
        _viewObject.SetActive(event.GetValue().GetBool());
        return 0;
    }

    int i = ScreenLocationPropertyHelper::OnPropertyGridChange(_viewObject.GetObjectScreenLocation(), grid, event);

    _viewObject.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ViewObject::OnPropertyGridChange");

    return i;
}
