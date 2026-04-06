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

#include "GridlinesObjectPropertyAdapter.h"
#include "../../../models/GridlinesObject.h"
#include "../../../models/RulerObject.h"
#include "ui/shared/utils/wxUtilities.h"

GridlinesObjectPropertyAdapter::GridlinesObjectPropertyAdapter(GridlinesObject& obj)
    : ViewObjectPropertyAdapter(obj), _gridlines(obj) {}

void GridlinesObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty *p = grid->Append(new wxUIntProperty("Line Spacing", "GridLineSpacing", _gridlines.GetGridLineSpacing()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1024);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Grid Width", "GridWidth", _gridlines.GetGridWidth()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Grid Height", "GridHeight", _gridlines.GetGridHeight()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxColourProperty("Grid Color", "GridColor", xlColorToWxColour(_gridlines.GetGridColorObj())));

    p = grid->Append(new wxBoolProperty("Axis Lines", "GridAxis", _gridlines.GetHasAxis()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Point To Front", "PointToFront", _gridlines.GetPointToFront()));
    p->SetAttribute("UseCheckbox", true);

    if (RulerObject::GetRuler() != nullptr) {
        p = grid->Append(new wxStringProperty("Grid Spacing", "RealSpacing",
            RulerObject::PrescaledMeasureDescription(RulerObject::Measure(_gridlines.GetGridLineSpacing()))
        ));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

int GridlinesObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("GridLineSpacing" == event.GetPropertyName()) {
        _gridlines.SetGridLineSpacing((int)event.GetPropertyValue().GetLong());
        if (grid->GetPropertyByName("RealSpacing") != nullptr && RulerObject::GetRuler() != nullptr) {
            grid->GetPropertyByName("RealSpacing")->SetValueFromString(
                RulerObject::PrescaledMeasureDescription(RulerObject::Measure(_gridlines.GetGridLineSpacing())));
        }
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        return 0;
    } else if ("GridWidth" == event.GetPropertyName()) {
        _gridlines.SetGridWidth((int)event.GetPropertyValue().GetLong());
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridWidth");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridWidth");
        return 0;
    } else if ("GridHeight" == event.GetPropertyName()) {
        _gridlines.SetGridHeight((int)event.GetPropertyValue().GetLong());
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridHeight");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridHeight");
        return 0;
    } else if ("GridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("GridColor");
        wxColour c;
        c << p->GetValue();
        _gridlines.SetGridColor(wxColourToXlColor(c));
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridColor");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridColor");
        return 0;
    } else if (event.GetPropertyName() == "GridAxis") {
        _gridlines.SetHasAxis(event.GetValue().GetBool());
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridAxis");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridAxis");
        return 0;
    } else if (event.GetPropertyName() == "PointToFront") {
        _gridlines.SetPointToFront(event.GetValue().GetBool());
        _gridlines.IncrementChangeCount();
        _gridlines.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::PointToFront");
        _gridlines.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::PointToFront");
        return 0;
    }

    return ViewObjectPropertyAdapter::OnPropertyGridChange(grid, event);
}
