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

#include "DmxMovingHeadPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../helpers/DmxComponentPropertyHelpers.h"
#include "../../../models/DMX/DmxMovingHead.h"
#include "../../../models/DMX/DmxColorAbility.h"
#include "../../../models/DMX/DmxMotor.h"
#include "../../../controllers/ControllerCaps.h"
#include "../../../models/OutputModelManager.h"

static const char* DMX_STYLE_VALUES[] = {
    "Moving Head Top",
    "Moving Head Side",
    "Moving Head Bars",
    "Moving Head Top Bars",
    "Moving Head Side Bars",
    "Moving Head 3D"
};

static const char* DMX_STYLE_NAMES[] = {
    "Moving Head Top",
    "Moving Head Side",
    "Moving Head Bars",
    "Moving Head TopBars",
    "Moving Head SideBars",
    "Moving Head 3D"
};

static const char* DMX_FIXTURE_VALUES[] = {
    "MH1", "MH2", "MH3", "MH4", "MH5", "MH6", "MH7", "MH8"
};

DmxMovingHeadPropertyAdapter::DmxMovingHeadPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _mh(static_cast<DmxMovingHead&>(model)) {}

void DmxMovingHeadPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    static wxPGChoices DMX_STYLES;
    if (DMX_STYLES.GetCount() == 0) {
        for (const auto& s : DMX_STYLE_VALUES) {
            DMX_STYLES.Add(s);
        }
    }

    grid->Append(new wxEnumProperty("DMX Style", "DmxStyle", DMX_STYLES, _mh.GetDmxStyleVal()));

    static wxPGChoices DMX_FIXTURES;
    if (DMX_FIXTURES.GetCount() == 0) {
        for (const auto& f : DMX_FIXTURE_VALUES) {
            DMX_FIXTURES.Add(f);
        }
    }

    grid->Append(new wxEnumProperty("Fixture", "DmxFixture", DMX_FIXTURES, _mh.GetFixtureVal() - 1));

    // DmxModel base properties (# Channels + preset)
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _mh.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_mh.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_mh.GetPresetAbility(), _mh.GetDmxChannelCount());
    }

    wxPGProperty* pb = grid->Append(new wxBoolProperty("Hide Body", "HideBody", _mh.GetHideBody()));
    pb->SetAttribute("UseCheckbox", true);

    DmxComponentPropertyHelpers::AddMotorProperties(grid, *_mh.GetPanMotor());
    DmxComponentPropertyHelpers::AddMotorProperties(grid, *_mh.GetTiltMotor());

    grid->Append(new wxPropertyCategory("Color Properties", "DmxColorAbility"));
    static wxPGChoices DMX_COLOR_TYPES(wxArrayString(4, DMX_COLOR_TYPES_VALUES));
    int selected = 3; // show Unused if not selected
    if (_mh.HasColorAbility()) {
        selected = DMX_COLOR_TYPES.Index(_mh.GetColorAbility()->GetTypeName());
    }
    grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES, selected));
    if (_mh.HasColorAbility()) {
        ControllerCaps* caps = _mh.GetControllerCaps();
        DmxAbilityPropertyHelpers::AddColorProperties(grid, _mh.GetColorAbility(), _mh.IsPWMProtocol() && caps && caps->SupportsPWM());
    }
    grid->Collapse("DmxColorAbility");

    DmxAbilityPropertyHelpers::AddDimmerProperties(grid, *_mh.GetDimmerAbility());
    DmxAbilityPropertyHelpers::AddShutterProperties(grid, *_mh.GetShutterAbility());
    DmxAbilityPropertyHelpers::AddBeamProperties(grid, *_mh.GetBeamAbility());
    grid->Collapse("DmxDimmerProperties");
    grid->Collapse("DmxShutterProperties");
    grid->Collapse("DmxBeamProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxMovingHeadPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (_mh.HasColorAbility() && DmxAbilityPropertyHelpers::OnColorPropertyGridChange(grid, event, _mh.GetColorAbility(), &_mh) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMotorPropertyGridChange(grid, event, *_mh.GetPanMotor(), &_mh, _mh.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMotorPropertyGridChange(grid, event, *_mh.GetTiltMotor(), &_mh, _mh.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (_mh.HasShutterAbility() && DmxAbilityPropertyHelpers::OnShutterPropertyGridChange(grid, event, *_mh.GetShutterAbility(), &_mh) == 0) {
        return 0;
    }

    if (_mh.HasDimmerAbility() && DmxAbilityPropertyHelpers::OnDimmerPropertyGridChange(grid, event, *_mh.GetDimmerAbility(), &_mh) == 0) {
        return 0;
    }

    if (_mh.HasBeamAbility() && DmxAbilityPropertyHelpers::OnBeamPropertyGridChange(grid, event, *_mh.GetBeamAbility(), &_mh) == 0) {
        return 0;
    }

    if ("DmxStyle" == event.GetPropertyName()) {
        int style_val = event.GetPropertyValue().GetLong();
        _mh.SetDmxStyleVal(style_val);
        if (style_val >= 0 && style_val < (int)std::size(DMX_STYLE_NAMES)) {
            _mh.SetDmxStyle(DMX_STYLE_NAMES[style_val]);
        }
        _mh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        return 0;
    } else if ("HideBody" == event.GetPropertyName()) {
        _mh.SetHideBody(event.GetPropertyValue().GetBool());
        _mh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::HideBody");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::HideBody");
        _mh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::HideBody");
        return 0;
    } else if ("DmxColorType" == event.GetPropertyName()) {
        int color_type = event.GetPropertyValue().GetInteger();
        _mh.InitColorAbility(color_type);
        _mh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        _mh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        _mh.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        return 0;
    } else if ("DmxFixture" == event.GetPropertyName()) {
        int fv = event.GetPropertyValue().GetLong();
        _mh.SetFixtureVal(fv);
        _mh.SetDmxFixture(DmxMovingHeadComm::FixtureIDtoString(fv));
        _mh.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DmxFixture");
        _mh.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DmxFixture");
        return 0;
    }

    // DmxModel base handling
    _mh.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _mh.IncrementChangeCount();
        _mh.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_mh.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_mh.GetPresetAbility(), _mh.GetDmxChannelCount(), &_mh) == 0) {
        _mh.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
