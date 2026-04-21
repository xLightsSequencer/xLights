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

#include "LabelPropertyAdapter.h"
#include "models/LabelModel.h"
#include "models/OutputModelManager.h"
#include "shared/utils/wxUtilities.h"

LabelPropertyAdapter::LabelPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _label(static_cast<LabelModel&>(model)) {}

void LabelPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    grid->Append(new wxStringProperty("Label Text", "LabelText", _label.GetLabelText()));

    wxPGProperty* p = grid->Append(new wxUIntProperty("Font Size", "LabelFontSize", _label.GetLabelFontSize()));
    p->SetAttribute("Min", 8);
    p->SetAttribute("Max", 40);
    p->SetEditor("SpinCtrl");

    xlColor tc = _label.GetLabelTextColor();
    wxColour col(tc.red, tc.green, tc.blue);
    grid->Append(new wxColourProperty("Text Color", "LabelTextColor", col));
}

int LabelPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("LabelText" == event.GetPropertyName()) {
        _label.SetLabelText(ToStdString(event.GetValue().GetString()));
        _label.IncrementChangeCount();
        _label.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "LabelPropertyAdapter::OnPropertyGridChange::LabelText");
        return 0;
    } else if ("LabelFontSize" == event.GetPropertyName()) {
        _label.SetLabelFontSize(event.GetValue().GetInteger());
        _label.IncrementChangeCount();
        _label.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "LabelPropertyAdapter::OnPropertyGridChange::LabelFontSize");
        return 0;
    } else if ("LabelTextColor" == event.GetPropertyName()) {
        wxColour col;
        col << event.GetValue();
        if (col.IsOk()) {
            xlColor xc = wxColourToXlColor(col);
            _label.SetLabelTextColor(xc);
            event.GetProperty()->SetValue(WXVARIANT(xlColorToWxColour(xc)));
        }
        _label.IncrementChangeCount();
        _label.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "LabelPropertyAdapter::OnPropertyGridChange::LabelTextColor");
        return 0;
    }
    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

void LabelPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelStates");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelStrandNodeNames");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("SubModels");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) p->Enable(false);

    p = grid->GetPropertyByName("ModelPixelBlackTransparency");
    if (p != nullptr) p->Enable(false);
}
