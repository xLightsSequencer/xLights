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

#include "DmxGeneralPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../../../models/DMX/DmxGeneral.h"
#include "../../../controllers/ControllerCaps.h"
#include "../../../OutputModelManager.h"

DmxGeneralPropertyAdapter::DmxGeneralPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _general(static_cast<DmxGeneral&>(model)) {}

void DmxGeneralPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _general.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_general.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_general.GetPresetAbility(), _general.GetDmxChannelCount());
    }

    if (_general.HasColorAbility()) {
        ControllerCaps* caps = _general.GetControllerCaps();
        DmxAbilityPropertyHelpers::AddColorProperties(grid, _general.GetColorAbility(), _general.IsPWMProtocol() && caps && caps->SupportsPWM());
    }
}

int DmxGeneralPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (_general.HasColorAbility() && DmxAbilityPropertyHelpers::OnColorPropertyGridChange(grid, event, _general.GetColorAbility(), &_general) == 0) {
        return 0;
    }

    // DmxModel base handling
    _general.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _general.IncrementChangeCount();
        _general.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_general.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_general.GetPresetAbility(), _general.GetDmxChannelCount(), &_general) == 0) {
        _general.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
