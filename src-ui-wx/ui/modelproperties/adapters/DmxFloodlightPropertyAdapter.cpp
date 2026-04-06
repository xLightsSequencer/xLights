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

#include "DmxFloodlightPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "models/DMX/DmxFloodlight.h"
#include "controllers/ControllerCaps.h"
#include "models/OutputModelManager.h"

DmxFloodlightPropertyAdapter::DmxFloodlightPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _flood(static_cast<DmxFloodlight&>(model)) {}

void DmxFloodlightPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _flood.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_flood.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_flood.GetPresetAbility(), _flood.GetDmxChannelCount());
    }

    ControllerCaps* caps = _flood.GetControllerCaps();
    if (_flood.HasColorAbility()) {
        DmxAbilityPropertyHelpers::AddColorProperties(grid, _flood.GetColorAbility(), _flood.IsPWMProtocol() && caps && caps->SupportsPWM());
    }
    if (_flood.HasShutterAbility()) {
        DmxAbilityPropertyHelpers::AddShutterProperties(grid, *_flood.GetShutterAbility());
    }
    if (_flood.HasBeamAbility()) {
        DmxAbilityPropertyHelpers::AddBeamProperties(grid, *_flood.GetBeamAbility());
    }
    grid->Collapse("DmxShutterProperties");
    grid->Collapse("DmxBeamProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

void DmxFloodlightPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) {
        p->Enable(false);
    }

    // DmxModel base disabling
    p = grid->GetPropertyByName("ModelStringType");
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
}

int DmxFloodlightPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (_flood.HasColorAbility() && DmxAbilityPropertyHelpers::OnColorPropertyGridChange(grid, event, _flood.GetColorAbility(), &_flood) == 0) {
        return 0;
    }

    if (_flood.HasShutterAbility() && DmxAbilityPropertyHelpers::OnShutterPropertyGridChange(grid, event, *_flood.GetShutterAbility(), &_flood) == 0) {
        return 0;
    }

    if (_flood.HasBeamAbility() && DmxAbilityPropertyHelpers::OnBeamPropertyGridChange(grid, event, *_flood.GetBeamAbility(), &_flood) == 0) {
        return 0;
    }

    // DmxModel base handling
    _flood.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _flood.IncrementChangeCount();
        _flood.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_flood.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_flood.GetPresetAbility(), _flood.GetDmxChannelCount(), &_flood) == 0) {
        _flood.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
