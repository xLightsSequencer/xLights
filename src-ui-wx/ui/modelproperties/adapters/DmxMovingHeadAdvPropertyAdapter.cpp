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

#include "DmxMovingHeadAdvPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../helpers/DmxComponentPropertyHelpers.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxColorAbility.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/Mesh.h"
#include "controllers/ControllerCaps.h"
#include "models/OutputModelManager.h"
#include "PositionZoneDialog.h"
#include "../PropertyGridHelpers.h"

static const char* DMX_FIXTURE_VALUES[] = {
    "MH1", "MH2", "MH3", "MH4", "MH5", "MH6", "MH7", "MH8"
};

DmxMovingHeadAdvPropertyAdapter::DmxMovingHeadAdvPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _mha(static_cast<DmxMovingHeadAdv&>(model)) {}

class PositionZoneDialogAdapter : public wxPGEditorDialogAdapter {
public:
    PositionZoneDialogAdapter(DmxMovingHeadAdv& model) :
        wxPGEditorDialogAdapter(), m_model(model) {
    }

    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        auto zones = m_model.GetPositionZones();
        PositionZoneDialog dlg(zones, propGrid);
        if (dlg.ShowModal() == wxID_OK) {
            m_model.SetPositionZones(zones);
            m_model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PositionZoneDialogAdapter");
            m_model.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PositionZoneDialogAdapter");
            m_model.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PositionZoneDialogAdapter");
            m_model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PositionZoneDialogAdapter");
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }

protected:
    DmxMovingHeadAdv& m_model;
};

class PositionZonePopupProperty : public wxStringProperty {
public:
    PositionZonePopupProperty(DmxMovingHeadAdv& model,
                              const wxString& label,
                              const wxString& name,
                              const wxString& value) :
        wxStringProperty(label, name, value), _model(model) {
    }

    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }

    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new PositionZoneDialogAdapter(_model);
    }

protected:
    DmxMovingHeadAdv& _model;
};

void DmxMovingHeadAdvPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    static wxPGChoices DMX_FIXTURES;
    if (DMX_FIXTURES.GetCount() == 0) {
        for (const auto& f : DMX_FIXTURE_VALUES) {
            DMX_FIXTURES.Add(f);
        }
    }

    grid->Append(new wxEnumProperty("Fixture", "DmxFixture", DMX_FIXTURES, _mha.GetFixtureVal() - 1));

    // DmxModel base properties (# Channels + preset)
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _mha.GetDmxChannelCount()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_mha.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_mha.GetPresetAbility(), _mha.GetDmxChannelCount());
    }

    DmxComponentPropertyHelpers::AddMotorProperties(grid, *_mha.GetPanMotor());
    DmxComponentPropertyHelpers::AddMotorProperties(grid, *_mha.GetTiltMotor());

    grid->Append(new wxPropertyCategory("Color Properties", "DmxColorAbility"));
    static wxPGChoices DMX_COLOR_TYPES(wxArrayString(4, DMX_COLOR_TYPES_VALUES));
    int selected = 3; // show Unused if not selected
    if (_mha.HasColorAbility()) {
        selected = DMX_COLOR_TYPES.Index(_mha.GetColorAbility()->GetTypeName());
    }
    grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES, selected));
    if (_mha.HasColorAbility()) {
        ControllerCaps* caps = _mha.GetControllerCaps();
        DmxAbilityPropertyHelpers::AddColorProperties(grid, _mha.GetColorAbility(), _mha.IsPWMProtocol() && caps && caps->SupportsPWM());
    }
    grid->Collapse("DmxColorAbility");

    DmxAbilityPropertyHelpers::AddDimmerProperties(grid, *_mha.GetDimmerAbility());
    DmxAbilityPropertyHelpers::AddShutterProperties(grid, *_mha.GetShutterAbility());
    DmxAbilityPropertyHelpers::AddBeamProperties(grid, *_mha.GetBeamAbility());
    grid->Collapse("DmxDimmerProperties");
    grid->Collapse("DmxShutterProperties");
    grid->Collapse("DmxBeamProperties");

    DmxComponentPropertyHelpers::AddMeshProperties(grid, *_mha.GetBaseMesh());
    DmxComponentPropertyHelpers::AddMeshProperties(grid, *_mha.GetYokeMesh());
    DmxComponentPropertyHelpers::AddMeshProperties(grid, *_mha.GetHeadMesh());

    grid->Append(new wxPropertyCategory("Position Zones", "PositionZones"));
    auto pz = grid->Append(new PositionZonePopupProperty(_mha, "Zones", "PositionZoneConfig", CLICK_TO_EDIT));
    grid->LimitPropertyEditing(pz);

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

void DmxMovingHeadAdvPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    // rotation around the Z axis causes issues when the pan and tilt rotations are applied
    auto p = grid->GetPropertyByName("c");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("YokeMeshRotateZ");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("HeadMeshRotateZ");
    if (p != nullptr) {
        p->Hide(true);
    }

    DmxPropertyAdapter::DisableUnusedProperties(grid);
}

int DmxMovingHeadAdvPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (_mha.HasColorAbility() && DmxAbilityPropertyHelpers::OnColorPropertyGridChange(grid, event, _mha.GetColorAbility(), &_mha) == 0) {
        return 0;
    }

    if (_mha.HasDimmerAbility() && DmxAbilityPropertyHelpers::OnDimmerPropertyGridChange(grid, event, *_mha.GetDimmerAbility(), &_mha) == 0) {
        return 0;
    }

    if (_mha.HasShutterAbility() && DmxAbilityPropertyHelpers::OnShutterPropertyGridChange(grid, event, *_mha.GetShutterAbility(), &_mha) == 0) {
        return 0;
    }

    if (_mha.HasBeamAbility() && DmxAbilityPropertyHelpers::OnBeamPropertyGridChange(grid, event, *_mha.GetBeamAbility(), &_mha) == 0) {
        return 0;
    }

    if ("DmxFixture" == event.GetPropertyName()) {
        _mha.SetFixtureVal(event.GetPropertyValue().GetLong());
        _mha.SetDmxFixture(DmxMovingHeadComm::FixtureIDtoString(event.GetPropertyValue().GetLong()));
        _mha.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxFixture");
        _mha.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxFixture");
        return 0;
    } else if ("DmxColorType" == event.GetPropertyName()) {
        int color_type = event.GetPropertyValue().GetInteger();
        _mha.InitColorAbility(color_type);
        _mha.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        _mha.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        _mha.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        _mha.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMotorPropertyGridChange(grid, event, *_mha.GetPanMotor(), &_mha, _mha.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMotorPropertyGridChange(grid, event, *_mha.GetTiltMotor(), &_mha, _mha.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMeshPropertyGridChange(grid, event, *_mha.GetBaseMesh(), &_mha, _mha.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMeshPropertyGridChange(grid, event, *_mha.GetYokeMesh(), &_mha, _mha.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (DmxComponentPropertyHelpers::OnMeshPropertyGridChange(grid, event, *_mha.GetHeadMesh(), &_mha, _mha.GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    // DmxModel base handling
    _mha.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _mha.IncrementChangeCount();
        _mha.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_mha.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_mha.GetPresetAbility(), _mha.GetDmxChannelCount(), &_mha) == 0) {
        _mha.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
