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

#include "RulerObjectPropertyAdapter.h"
#include "../../../models/RulerObject.h"

static const char *UNITS_VALUES[] = {
    "Meters", "Centimeters", "Millimeters", "Yards", "Feet",
    "Inches"};
static wxArrayString RULER_UNITS(6, UNITS_VALUES);

RulerObjectPropertyAdapter::RulerObjectPropertyAdapter(RulerObject& obj)
    : ViewObjectPropertyAdapter(obj), _ruler(obj) {}

void RulerObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxEnumProperty("Units", "Units", RULER_UNITS, wxArrayInt(), _ruler.GetUnits()));

    p = grid->Append(new wxFloatProperty("Real Length", "Length", _ruler.GetLength()));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Min", 0.01);
    p->SetEditor("SpinCtrl");
}

int RulerObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("Units" == event.GetPropertyName()) {
        _ruler.SetUnits((int)event.GetPropertyValue().GetLong());
        _ruler.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "RulerObject::OnPropertyGridChange::Units");
        return 0;
    } else if ("Length" == event.GetPropertyName()) {
        float len = event.GetPropertyValue().GetDouble();
        if (len < 0.01) len = 0.01f;
        _ruler.SetLength(len);
        _ruler.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "RulerObject::OnPropertyGridChange::Length");
        return 0;
    }

    return ViewObjectPropertyAdapter::OnPropertyGridChange(grid, event);
}
