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
#include <wx/propgrid/editors.h>

#include "DmxSkullPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../helpers/DmxComponentPropertyHelpers.h"
#include "../../../models/DMX/DmxSkull.h"
#include "../../../models/DMX/Servo.h"
#include "../../../models/DMX/Mesh.h"
#include "SkullConfigDialog.h"
#include "../../../controllers/ControllerCaps.h"
#include "../../../OutputModelManager.h"

static const std::string CLICK_TO_EDIT("--Click To Edit--");

class SkullConfigDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    SkullConfigDialogAdapter(DmxSkull* model) :
        wxPGEditorDialogAdapter(), m_model(model)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property)) override
    {
        SkullConfigDialog dlg(propGrid);

        dlg.CheckBox_16bits->SetValue(m_model->Is16Bit());
        dlg.CheckBox_Jaw->SetValue(m_model->HasJaw());
        dlg.CheckBox_Pan->SetValue(m_model->HasPan());
        dlg.CheckBox_Tilt->SetValue(m_model->HasTilt());
        dlg.CheckBox_Nod->SetValue(m_model->HasNod());
        dlg.CheckBox_EyeLR->SetValue(m_model->HasEyeLR());
        dlg.CheckBox_EyeUD->SetValue(m_model->HasEyeUD());
        dlg.CheckBox_Color->SetValue(m_model->HasColor());
        dlg.CheckBox_Skulltronix->SetValue(false);

        if (dlg.ShowModal() == wxID_OK) {
            bool changed = false;

            if (dlg.CheckBox_Jaw->GetValue() != m_model->HasJaw()) {
                m_model->SetHasJaw(dlg.CheckBox_Jaw->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_Pan->GetValue() != m_model->HasPan()) {
                m_model->SetHasPan(dlg.CheckBox_Pan->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_Tilt->GetValue() != m_model->HasTilt()) {
                m_model->SetHasTilt(dlg.CheckBox_Tilt->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_Nod->GetValue() != m_model->HasNod()) {
                m_model->SetHasNod(dlg.CheckBox_Nod->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_EyeLR->GetValue() != m_model->HasEyeLR()) {
                m_model->SetHasEyeLR(dlg.CheckBox_EyeLR->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_EyeUD->GetValue() != m_model->HasEyeUD()) {
                m_model->SetHasEyeUD(dlg.CheckBox_EyeUD->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_Color->GetValue() != m_model->HasColor()) {
                m_model->SetHasColor(dlg.CheckBox_Color->GetValue());
                changed = true;
            }
            if (dlg.CheckBox_16bits->GetValue() != m_model->Is16Bit()) {
                m_model->SetIs16Bit(dlg.CheckBox_16bits->GetValue());
                changed = true;
            }

            if (dlg.CheckBox_Skulltronix->GetValue()) {
                m_model->SetSkulltronix();
                changed = true;
            }

            if (changed) {
                m_model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::SkullConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkull::SkullConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::SkullConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxSkull::SkullConfigDialogAdapter");
            }

            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }

protected:
    DmxSkull* m_model;
};

class SkullPopupDialogProperty : public wxStringProperty
{
public:
    SkullPopupDialogProperty(DmxSkull* m,
                             const wxString& label,
                             const wxString& name,
                             const wxString& value,
                             int type) :
        wxStringProperty(label, name, value), m_model(m), m_tp(type)
    {
    }
    virtual const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrlAndButton;
    }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        switch (m_tp) {
        case 1:
            return new SkullConfigDialogAdapter(m_model);
        default:
            break;
        }
        return nullptr;
    }

protected:
    DmxSkull* m_model = nullptr;
    int m_tp;
};

DmxSkullPropertyAdapter::DmxSkullPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _skull(static_cast<DmxSkull&>(model)) {}

void DmxSkullPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    // DmxModel base properties (# Channels + preset)
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _skull.GetParm1()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_skull.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_skull.GetPresetAbility(), _skull.GetParm1());
    }

    p = grid->Append(new SkullPopupDialogProperty(&_skull, "Skull Config", "SkullConfig", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxBoolProperty("Mesh Only", "MeshOnly", _skull.IsMeshOnly()));
    p->SetAttribute("UseCheckbox", true);

    ControllerCaps* caps = _skull.GetControllerCaps();
    bool doPWM = _skull.IsPWMProtocol() && caps != nullptr && caps->SupportsPWM();

    if (_skull.HasJaw() && _skull.GetJawServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetJawServo(), doPWM);
    }
    if (_skull.HasPan() && _skull.GetPanServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetPanServo(), doPWM);
    }
    if (_skull.HasTilt() && _skull.GetTiltServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetTiltServo(), doPWM);
    }
    if (_skull.HasNod() && _skull.GetNodServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetNodServo(), doPWM);
    }
    if (_skull.HasEyeUD() && _skull.GetEyeUDServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetEyeUDServo(), doPWM);
    }
    if (_skull.HasEyeLR() && _skull.GetEyeLRServo() != nullptr) {
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_skull.GetEyeLRServo(), doPWM);
    }

    grid->Append(new wxPropertyCategory("Orientation Properties", "OrientProperties"));

    if (_skull.HasJaw()) {
        p = grid->Append(new wxIntProperty("Jaw Orientation", "DmxJawOrient", _skull.GetJawOrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasPan()) {
        p = grid->Append(new wxIntProperty("Pan Orientation", "DmxPanOrient", _skull.GetPanOrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasTilt()) {
        p = grid->Append(new wxIntProperty("Tilt Orientation", "DmxTiltOrient", _skull.GetTiltOrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasNod()) {
        p = grid->Append(new wxIntProperty("Nod Orientation", "DmxNodOrient", _skull.GetNodOrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasEyeUD()) {
        p = grid->Append(new wxIntProperty("Eye Up/Down Orientation", "DmxEyeUDOrient", _skull.GetEyeUDOrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasEyeLR()) {
        p = grid->Append(new wxIntProperty("Eye Left/Right Orientation", "DmxEyeLROrient", _skull.GetEyeLROrient()));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (_skull.HasColor()) {
        grid->Append(new wxPropertyCategory("Color Properties", "ColorProperties"));

        p = grid->Append(new wxUIntProperty("Eye Brightness Channel", "DmxEyeBrtChannel", _skull.GetEyeBrightnessChannel()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 512);
        p->SetEditor("SpinCtrl");
        if (_skull.HasColorAbility()) {
            ControllerCaps* caps2 = _skull.GetControllerCaps();
            DmxAbilityPropertyHelpers::AddColorProperties(grid, _skull.GetColorAbility(), _skull.IsPWMProtocol() && caps2 && caps2->SupportsPWM());
        }
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

void DmxSkullPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }
}

int DmxSkullPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("MeshOnly" == event.GetPropertyName()) {
        _skull.SetMeshOnly(event.GetValue().GetBool());
        _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::MeshOnly");
        _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::MeshOnly");
        return 0;
    }

    if (_skull.HasColor()) {
        if (_skull.HasColorAbility() && DmxAbilityPropertyHelpers::OnColorPropertyGridChange(grid, event, _skull.GetColorAbility(), &_skull) == 0) {
            return 0;
        }
    }

    if (_skull.HasJaw() && _skull.GetJawServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetJawServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxJawOrient" == event.GetPropertyName()) {
            _skull.SetJawOrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DmxJawOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DmxJawOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DmxJawOrient");
            return 0;
        }
    }

    if (_skull.HasPan() && _skull.GetPanServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetPanServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxPanOrient" == event.GetPropertyName()) {
            _skull.SetPanOrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            return 0;
        }
    }

    if (_skull.HasTilt() && _skull.GetTiltServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetTiltServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxTiltOrient" == event.GetPropertyName()) {
            _skull.SetTiltOrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            return 0;
        }
    }

    if (_skull.HasNod() && _skull.GetNodServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetNodServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxNodOrient" == event.GetPropertyName()) {
            _skull.SetNodOrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            return 0;
        }
    }

    if (_skull.HasEyeUD() && _skull.GetEyeUDServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetEyeUDServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxEyeUDOrient" == event.GetPropertyName()) {
            _skull.SetEyeUDOrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            return 0;
        }
    }

    if (_skull.HasEyeLR() && _skull.GetEyeLRServo() != nullptr) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_skull.GetEyeLRServo(), &_skull, _skull.GetModelScreenLocation().IsLocked() || _skull.IsFromBase()) == 0) {
            return 0;
        }
        if ("DmxEyeLROrient" == event.GetPropertyName()) {
            _skull.SetEyeLROrient((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            _skull.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            return 0;
        }
    }

    if (_skull.HasColor()) {
        if ("DmxEyeBrtChannel" == event.GetPropertyName()) {
            _skull.SetEyeBrightnessChannel((int)event.GetPropertyValue().GetLong());
            _skull.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            _skull.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            _skull.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            return 0;
        }
    }

    // DmxModel base handling
    _skull.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _skull.IncrementChangeCount();
        _skull.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_skull.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_skull.GetPresetAbility(), _skull.GetParm1(), &_skull) == 0) {
        _skull.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
