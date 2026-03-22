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
#include <wx/propgrid/editors.h>
#include <wx/msgdlg.h>

#include "DmxServo3dPropertyAdapter.h"
#include "../helpers/DmxAbilityPropertyHelpers.h"
#include "../helpers/DmxComponentPropertyHelpers.h"
#include "../../../models/DMX/DmxServo3D.h"
#include "../../../models/DMX/Mesh.h"
#include "../../../models/DMX/Servo.h"
#include "ServoConfigDialog.h"
#include "../../../controllers/ControllerCaps.h"
#include "../../../OutputModelManager.h"

static const std::string CLICK_TO_EDIT("--Click To Edit--");

class ServoConfigDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    ServoConfigDialogAdapter(DmxServo3d* model) :
        wxPGEditorDialogAdapter(), m_model(model)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property)) override
    {
        ServoConfigDialog dlg(propGrid);

        dlg.CheckBox_16bits->SetValue(m_model->Is16Bit());
        dlg.SpinCtrl_NumServos->SetValue(m_model->GetNumServos());
        dlg.SpinCtrl_NumStatic->SetValue(m_model->GetNumStatic());
        dlg.SpinCtrl_NumMotion->SetValue(m_model->GetNumMotion());
        dlg.SpinCtrl_NumMotion->SetRange(1, m_model->GetNumServos());

        if (dlg.ShowModal() == wxID_OK) {
            bool changed = false;

            int _num_servos = dlg.SpinCtrl_NumServos->GetValue();
            if (_num_servos != m_model->GetNumServos()) {
                m_model->SetNumServos(_num_servos);
                changed = true;
                m_model->UpdateNodeNames();
            }
            int _num_static = dlg.SpinCtrl_NumStatic->GetValue();
            if (_num_static != m_model->GetNumStatic()) {
                m_model->SetNumStatic(_num_static);
                changed = true;
            }
            int _num_motion = dlg.SpinCtrl_NumMotion->GetValue();
            if (_num_motion != m_model->GetNumMotion()) {
                m_model->SetNumMotion(_num_motion);
                changed = true;
            }
            bool _16bit = dlg.CheckBox_16bits->GetValue();
            if (_16bit != m_model->Is16Bit()) {
                changed = true;
                m_model->UpdateBits();
                m_model->UpdateNodeNames();
            }
            if (changed) {
                m_model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::ServoConfigDialogAdapter");
                m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::ServoConfigDialogAdapter");
            }

            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }

protected:
    DmxServo3d* m_model;
};

class ServoPopupDialogProperty : public wxStringProperty
{
public:
    ServoPopupDialogProperty(DmxServo3d* m,
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
            return new ServoConfigDialogAdapter(m_model);
        default:
            break;
        }
        return nullptr;
    }

protected:
    DmxServo3d* m_model = nullptr;
    int m_tp;
};

static wxPGChoices MOTION_LINKS;

DmxServo3dPropertyAdapter::DmxServo3dPropertyAdapter(Model& model)
    : DmxPropertyAdapter(model), _servo3d(static_cast<DmxServo3d&>(model)) {}

void DmxServo3dPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    // DmxModel base properties (# Channels + preset)
    auto p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", _servo3d.GetParm1()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (_servo3d.HasPresetAbility()) {
        DmxAbilityPropertyHelpers::AddPresetProperties(grid, *_servo3d.GetPresetAbility(), _servo3d.GetParm1());
    }

    p = grid->Append(new ServoPopupDialogProperty(&_servo3d, "Servo Config", "ServoConfig", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxBoolProperty("Show Pivot Axes", "PivotAxes", _servo3d.GetShowPivot()));
    p->SetAttribute("UseCheckbox", true);

    int num_servos = _servo3d.GetNumServos();

    for (int i = 0; i < num_servos; ++i) {
        ControllerCaps* caps = _servo3d.GetControllerCaps();
        DmxComponentPropertyHelpers::AddServoProperties(grid, *_servo3d.GetServo(i), _servo3d.IsPWMProtocol() && caps != nullptr && caps->SupportsPWM());
    }

    for (int i = 0; i < _servo3d.GetNumStatic(); ++i) {
        DmxComponentPropertyHelpers::AddMeshProperties(grid, *_servo3d.GetStaticMesh(i));
    }

    for (int i = 0; i < _servo3d.GetNumMotion(); ++i) {
        DmxComponentPropertyHelpers::AddMeshProperties(grid, *_servo3d.GetMotionMesh(i));
    }

    if (MOTION_LINKS.GetCount() != (unsigned int)num_servos) {
        MOTION_LINKS.Clear();
    }
    if (MOTION_LINKS.GetCount() == 0) {
        for (int i = 0; i < num_servos; ++i) {
            MOTION_LINKS.Add("Mesh " + std::to_string(i + 1));
        }
    }

    grid->Append(new wxPropertyCategory("Servo Linkage", "ServoMotionProperties"));
    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Servo " + std::to_string(i + 1) + " Linkage";
        std::string linkage2 = "Servo" + std::to_string(i + 1) + "Linkage";
        grid->Append(new wxEnumProperty(linkage, linkage2, MOTION_LINKS, _servo3d.GetServoLink(i) == -1 ? i : _servo3d.GetServoLink(i)));
    }
    grid->Collapse("ServoMotionProperties");

    grid->Append(new wxPropertyCategory("Mesh Linkage", "MeshMotionProperties"));
    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Mesh " + std::to_string(i + 1) + " Linkage";
        std::string linkage2 = "Mesh" + std::to_string(i + 1) + "Linkage";
        grid->Append(new wxEnumProperty(linkage, linkage2, MOTION_LINKS, _servo3d.GetMeshLink(i) == -1 ? i : _servo3d.GetMeshLink(i)));
    }
    grid->Collapse("MeshMotionProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxServo3dPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    int num_servos = _servo3d.GetNumServos();

    if ("DmxChannelCount" == event.GetPropertyName()) {
        int channels = (int)event.GetPropertyValue().GetLong();
        int min_channels = num_servos * (_servo3d.Is16Bit() ? 2 : 1);
        if (channels < min_channels) {
            wxPGProperty* p = grid->GetPropertyByName("DmxChannelCount");
            if (p != nullptr) {
                p->SetValue(min_channels);
            }
            std::string msg = wxString::Format("You have %d servos at %d bits so you need %d channels minimum.", num_servos, _servo3d.Is16Bit() ? 16 : 8, min_channels);
            wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
            return 0;
        }
    }

    if ("PivotAxes" == name) {
        _servo3d.SetShowPivot(event.GetValue().GetBool());
        _servo3d.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        _servo3d.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::PivotAxes");
        return 0;
    }

    for (int i = 0; i < num_servos; ++i) {
        if (DmxComponentPropertyHelpers::OnServoPropertyGridChange(grid, event, *_servo3d.GetServo(i), &_servo3d, _servo3d.GetModelScreenLocation().IsLocked() || _servo3d.IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < _servo3d.GetNumStatic(); ++i) {
        if (DmxComponentPropertyHelpers::OnMeshPropertyGridChange(grid, event, *_servo3d.GetStaticMesh(i), &_servo3d, _servo3d.GetModelScreenLocation().IsLocked() || _servo3d.IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < _servo3d.GetNumMotion(); ++i) {
        if (DmxComponentPropertyHelpers::OnMeshPropertyGridChange(grid, event, *_servo3d.GetMotionMesh(i), &_servo3d, _servo3d.GetModelScreenLocation().IsLocked() || _servo3d.IsFromBase()) == 0) {
            return 0;
        }
    }

    for (int i = 0; i < num_servos; ++i) {
        std::string linkage = "Servo" + std::to_string(i + 1) + "Linkage";
        if (linkage == name) {
            int link_num = event.GetPropertyValue().GetLong();
            if (link_num >= num_servos) {
                link_num = i;
            }
            _servo3d.SetServoLink(i, link_num == i ? -1 : link_num);
            _servo3d.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::ServoLinkage");
            return 0;
        }
        linkage = "Mesh" + std::to_string(i + 1) + "Linkage";
        if (linkage == name) {
            int link_num = event.GetPropertyValue().GetLong();
            if (link_num >= num_servos) {
                link_num = i;
            }
            _servo3d.SetMeshLink(i, link_num == i ? -1 : link_num);
            _servo3d.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            _servo3d.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxServo3d::OnPropertyGridChange::MeshLinkage");
            return 0;
        }
    }

    // DmxModel base handling
    _servo3d.IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        _servo3d.IncrementChangeCount();
        _servo3d.UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (_servo3d.HasPresetAbility() && DmxAbilityPropertyHelpers::OnPresetPropertyGridChange(grid, event, *_servo3d.GetPresetAbility(), _servo3d.GetParm1(), &_servo3d) == 0) {
        _servo3d.IncrementChangeCount();
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
