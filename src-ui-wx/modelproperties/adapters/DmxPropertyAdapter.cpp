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

#include "DmxPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "models/DMX/DmxModel.h"
#include "models/OutputModelManager.h"

DmxPropertyAdapter::DmxPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _dmx(static_cast<DmxModel&>(model)) {}

void DmxPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _dmx.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_dmx.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_dmx.GetPresetAbility(), _dmx.GetDmxChannelCount());
    }
}

int DmxPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    _dmx.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _dmx.IncrementChangeCount();
        _dmx.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_dmx.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_dmx.GetPresetAbility(), _dmx.GetDmxChannelCount(), &_dmx) == 0) {
        _dmx.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

void DmxPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    // disable string type properties.  Only Single Color White allowed.
    wxPGProperty* p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }

    // Don't remove ModelStates ... these can be used for DMX devices that use a value range to set a colour or behaviour
}
