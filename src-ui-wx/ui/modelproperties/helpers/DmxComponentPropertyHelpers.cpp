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
#include <wx/msgdlg.h>

#include "DmxComponentPropertyHelpers.h"
#include "models/DMX/Servo.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/Mesh.h"
#include "models/DMX/DmxImage.h"
#include "models/BaseObject.h"
#include "models/Model.h"
#include "models/OutputModelManager.h"
#include "utils/ExternalHooks.h"
#include "graphics/xlMesh.h"

#include <filesystem>

namespace DmxComponentPropertyHelpers {

// ---- Servo ----

static wxPGChoices SERVO_STYLES;
static wxPGChoices ZERO_BEHAVIORS;
static wxPGChoices DATA_TYPES;

void AddServoProperties(wxPropertyGridInterface* grid, const Servo& servo, bool pwm) {
    std::string base_name = servo.GetName();
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    wxPGProperty* p = grid->Append(new wxUIntProperty("Channel", base_name + "Channel", servo.GetChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Min Limit", base_name + "MinLimit", servo.GetMinLimit()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", servo.Is16Bit() ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Max Limit", base_name + "MaxLimit", servo.GetMaxLimit()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", servo.Is16Bit() ? 65535 : 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Range of Motion", base_name + "RangeOfMotion", servo.GetRangeOfMotion()));
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1.0);
    p->SetAttribute("Min", -65535);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    if (SERVO_STYLES.GetCount() == 0) {
        SERVO_STYLES.Add("Translate X");
        SERVO_STYLES.Add("Translate Y");
        SERVO_STYLES.Add("Translate Z");
        SERVO_STYLES.Add("Rotate X");
        SERVO_STYLES.Add("Rotate Y");
        SERVO_STYLES.Add("Rotate Z");
    }
    if (DATA_TYPES.GetCount() == 0) {
        DATA_TYPES.Add("Scaled");
        DATA_TYPES.Add("Absolute");
        DATA_TYPES.Add("1/2 Absolute");
        DATA_TYPES.Add("2x Absolute");
    }
    if (ZERO_BEHAVIORS.GetCount() == 0) {
        ZERO_BEHAVIORS.Add("Hold");
        ZERO_BEHAVIORS.Add("Min");
        ZERO_BEHAVIORS.Add("Max");
        ZERO_BEHAVIORS.Add("Center");
        ZERO_BEHAVIORS.Add("Stop PWM");
    }

    int servo_style_val = SERVO_STYLES.Index(servo.GetStyle());
    if (servo_style_val == wxNOT_FOUND) servo_style_val = 0;
    grid->Append(new wxEnumProperty("Servo Style", base_name + "ServoStyle", SERVO_STYLES, servo_style_val));

    if (servo.IsRotate()) {
        p = grid->Append(new wxFloatProperty("Pivot Offset X", base_name + "PivotOffsetX", servo.GetScaledPivotOffsetX()));
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1.0);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxFloatProperty("Pivot Offset Y", base_name + "PivotOffsetY", servo.GetScaledPivotOffsetY()));
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1.0);
        p->SetEditor("SpinCtrl");

        if (!servo.Is2D()) {
            p = grid->Append(new wxFloatProperty("Pivot Offset Z", base_name + "PivotOffsetZ", servo.GetScaledPivotOffsetZ()));
            p->SetAttribute("Precision", 1);
            p->SetAttribute("Step", 1.0);
            p->SetEditor("SpinCtrl");
        }
    }
    if (pwm) {
        p = grid->Append(new wxUIntProperty("Contoller Min Limit", "Controller" + base_name + "MinLimit", servo.GetControllerMin()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 50000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Controller Max Limit", "Controller" + base_name + "MaxLimit", servo.GetControllerMax()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 50000);
        p->SetEditor("SpinCtrl");

        grid->Append(new wxBoolProperty("Controller Reverse", "Controller" + base_name + "Reverse", servo.GetControllerReverse()))->SetAttribute("UseCheckbox", 1);

        int zbv = ZERO_BEHAVIORS.Index(servo.GetControllerZero());
        int dtv = DATA_TYPES.Index(servo.GetControllerDataType());
        grid->Append(new wxEnumProperty("Controller Zero Behavior", "Controller" + base_name + "ZeroBehavior", ZERO_BEHAVIORS, zbv));
        grid->Append(new wxEnumProperty("Controller DataType", "Controller" + base_name + "DataType", DATA_TYPES, dtv));
    }
    grid->Collapse(wxString(base_name + "Properties"));
}

int OnServoPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, Servo& servo, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    std::string base_name = servo.GetName();
    std::string cname = "Controller" + base_name;

    if (base_name + "Channel" == name) {
        servo.SetChannel((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Servo::OnPropertyGridChange::Channel");
        return 0;
    } else if (base_name + "MinLimit" == name) {
        servo.SetMinLimit((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Servo::OnPropertyGridChange::MinLimit");
        return 0;
    } else if (base_name + "MaxLimit" == name) {
        servo.SetMaxLimit((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Servo::OnPropertyGridChange::MaxLimit");
        return 0;
    } else if (base_name + "RangeOfMotion" == name) {
        servo.SetRangeOfMotion((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Servo::OnPropertyGridChange::RangeOfMotion");
        return 0;
    } else if (base_name + "PivotOffsetX" == name) {
        servo.SetScaledPivotOffsetX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetX");
        return 0;
    } else if (base_name + "PivotOffsetY" == name) {
        servo.SetScaledPivotOffsetY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetY");
        return 0;
    } else if (base_name + "PivotOffsetZ" == name) {
        servo.SetScaledPivotOffsetZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Servo::OnPropertyGridChange::PivotOffsetZ");
        return 0;
    } else if (base_name + "ServoStyle" == name) {
        int style_val = event.GetPropertyValue().GetLong();
        servo.SetStyle(SERVO_STYLES.GetLabel(style_val).ToStdString());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                          OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                          OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
                          OutputModelManager::WORK_RELOAD_MODELLIST |
                          OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
                          OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Servo::OnPropertyGridChange::ServoStyle");
        return 0;
    } else if (name.starts_with(cname)) {
        if (name.ends_with("MinLimit")) {
            servo.SetControllerMin(event.GetValue().GetLong());
        } else if (name.ends_with("MaxLimit")) {
            servo.SetControllerMax(event.GetValue().GetLong());
        } else if (name.ends_with("Reverse")) {
            servo.SetControllerReverse(event.GetValue().GetBool());
        } else if (name.ends_with("ZeroBehavior")) {
            servo.SetControllerZero(ZERO_BEHAVIORS.GetLabel(event.GetValue().GetLong()).ToStdString());
        } else if (name.ends_with("DataType")) {
            servo.SetControllerDataType(DATA_TYPES.GetLabel(event.GetValue().GetLong()).ToStdString());
        }
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Servo::OnPropertyGridChange::" + name);
    }

    return -1;
}

// ---- DmxMotor ----

void AddMotorProperties(wxPropertyGridInterface* grid, const DmxMotor& motor) {
    std::string base_name = motor.GetName();
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    auto p = grid->Append(new wxUIntProperty("Channel (Coarse)", base_name + "ChannelCoarse", motor.GetChannelCoarse()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Channel (Fine)", base_name + "ChannelFine", motor.GetChannelFine()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Min Limit (deg)", base_name + "MinLimit", motor.GetMinLimit()));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 0);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Max Limit (deg)", base_name + "MaxLimit", motor.GetMaxLimit()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Range of Motion (deg)", base_name + "RangeOfMotion", motor.GetRangeOfMotion()));
    p->SetAttribute("Precision", 1);
    p->SetAttribute("Step", 1.0);
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 65535);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Orient to Zero (deg)", base_name + "OrientZero", motor.GetOrientZero()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    std::string label = "Orient Up (deg)";
    if (base_name == "PanMotor") {
        label = "Orient Forward (deg)";
    }
    p = grid->Append(new wxIntProperty(label, base_name + "OrientHome", motor.GetOrientHome()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Slew Limit (deg/sec)", base_name + "SlewLimit", motor.GetSlewLimit()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Reverse Rotation", base_name + "Reverse", motor.GetReverse()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Upside Down", base_name + "UpsideDown", motor.GetUpsideDown()));
    p->SetAttribute("UseCheckbox", true);

    grid->Collapse(wxString(base_name + "Properties"));
}

int OnMotorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxMotor& motor, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    std::string base_name = motor.GetName();

    if (base_name + "ChannelCoarse" == name) {
        motor.SetChannelCoarse((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMotor::OnPropertyGridChange::ChannelCoarse");
        return 0;
    } else if (base_name + "ChannelFine" == name) {
        motor.SetChannelFine((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMotor::OnPropertyGridChange::ChannelFine");
        return 0;
    } else if (base_name + "MinLimit" == name) {
        motor.SetMinLimit((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MinLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MinLimit");
        return 0;
    } else if (base_name + "MaxLimit" == name) {
        motor.SetMaxLimit((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::MaxLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::MaxLimit");
        return 0;
    } else if (base_name + "RangeOfMotion" == name) {
        motor.SetRangeOfMOtion((float)event.GetPropertyValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::RangeOfMotion");
        return 0;
    } else if (base_name + "OrientZero" == name) {
        motor.SetOrientZero((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientZero");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientZero");
        return 0;
    } else if (base_name + "OrientHome" == name) {
        motor.SetOrientHome((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::OrientHome");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::OrientHome");
        return 0;
    } else if (base_name + "SlewLimit" == name) {
        motor.SetSlewLimit((float)event.GetPropertyValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMotor::OnPropertyGridChange::SlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::SlewLimit");
        return 0;
    } else if (base_name + "Reverse" == name) {
        motor.SetReverse(event.GetValue().GetBool());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::Reverse");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMotor::OnPropertyGridChange::Reverse");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::Reverse");
        return 0;
    } else if (base_name + "UpsideDown" == name) {
        motor.SetUpsideDown(event.GetValue().GetBool());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMotor::OnPropertyGridChange::UpsideDown");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMotor::OnPropertyGridChange::UpsideDown");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMotor::OnPropertyGridChange::UpsideDown");
        return 0;
    }

    return -1;
}

// ---- Mesh ----

void AddMeshProperties(wxPropertyGridInterface* grid, const Mesh& mesh) {
    std::string base_name = mesh.GetName();
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    wxPGProperty* prop = grid->Append(new wxFileProperty("ObjFile", base_name + "ObjFile", mesh.GetObjFile()));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    prop = grid->Append(new wxBoolProperty("Mesh Only", base_name + "MeshOnly", mesh.GetMeshOnly()));
    prop->SetAttribute("UseCheckbox", true);
    prop = grid->Append(new wxUIntProperty("Brightness", base_name + "Brightness", mesh.GetBrightness()));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", mesh.GetOffsetX()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", mesh.GetOffsetY()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", mesh.GetOffsetZ()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleX", base_name + "ScaleX", mesh.GetScaleX()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", base_name + "ScaleY", mesh.GetScaleY()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", base_name + "ScaleZ", mesh.GetScaleZ()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", base_name + "RotateX", mesh.GetRotateX()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", base_name + "RotateY", mesh.GetRotateY()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", base_name + "RotateZ", mesh.GetRotateZ()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");

    grid->Collapse(wxString(base_name + "Properties"));
}

int OnMeshPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, Mesh& mesh, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    std::string base_name = mesh.GetName();

    if (!locked && base_name + "ObjFile" == name) {
        std::string objFile = event.GetValue().GetString().ToStdString();
        ObtainAccessToURL(objFile);

        auto mtfs = xlMesh::GetMaterialFilenamesFromOBJ(objFile, false);
        bool hasSpaces = false;
        std::filesystem::path path(objFile);
        for (auto& mtf : mtfs) {
            if (mtf.find(' ') != std::string::npos) {
                std::filesystem::path mtlpath(path);
                mtlpath.replace_filename(mtf);
                if (std::filesystem::exists(mtlpath)) {
                    hasSpaces = true;
                }
            }
        }
        if (hasSpaces) {
            if (wxMessageBox("The OBJ file contains materials with spaces in the filename.  This will prevent the materials from working.  Should we attempt to fix the file?",
                             "Files with spaces",
                             wxYES_NO | wxCENTRE | wxICON_WARNING) == wxYES) {
                xlMesh::FixMaterialFilenamesInOBJ(objFile);
            }
        }

        mesh.SetObjFile(objFile);
        mesh.NotifyObjFileChanged();
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ObjFile");
        return 0;
    } else if (locked && base_name + "ObjFile" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "Brightness" == name) {
        mesh.SetBrightness((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Mesh::OnPropertyGridChange::Brightness");
        return 0;
    } else if (locked && base_name + "Brightness" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "MeshOnly" == name) {
        mesh.SetMeshOnly(event.GetValue().GetBool());
        base->AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Mesh::OnPropertyGridChange::MeshOnly");
        return 0;
    } else if (locked && base_name + "MeshOnly" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "ScaleX" == name) {
        mesh.SetScaleX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "ScaleY" == name) {
        mesh.SetScaleY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "ScaleZ" == name) {
        mesh.SetScaleZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleZ" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetX" == name) {
        mesh.SetOffsetX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ModelX");
        return 0;
    } else if (locked && base_name + "OffsetX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetY" == name) {
        mesh.SetOffsetY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ModelY");
        return 0;
    } else if (locked && base_name + "OffsetY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetZ" == name) {
        mesh.SetOffsetZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::ModelZ");
        return 0;
    } else if (locked && base_name + "OffsetZ" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateX" == name) {
        mesh.SetRotateX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::RotateX");
        return 0;
    } else if (locked && base_name + "RotateX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateY" == name) {
        mesh.SetRotateY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::RotateY");
        return 0;
    } else if (locked && base_name + "RotateY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateZ" == name) {
        mesh.SetRotateZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "Mesh::OnPropertyGridChange::RotateZ");
        return 0;
    } else if (locked && base_name + "RotateZ" == name) {
        event.Veto();
        return 0;
    }

    return -1;
}

// ---- DmxImage ----

void AddImageProperties(wxPropertyGridInterface* grid, const DmxImage& image) {
    std::string base_name = image.GetName();
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    wxPGProperty* prop = grid->Append(new wxImageFileProperty("Image", base_name + "Image", image.GetImageFile()));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                           ";*.webp"
                                           "|All files (*.*)|*.*");

    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", image.GetOffsetX() * image.OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", image.GetOffsetY() * image.OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", image.GetOffsetZ() * image.OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleX", base_name + "ScaleX", image.GetScaleX()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", base_name + "ScaleY", image.GetScaleY()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", base_name + "ScaleZ", image.GetScaleZ()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", base_name + "RotateX", image.GetRotateX()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", base_name + "RotateY", image.GetRotateY()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", base_name + "RotateZ", image.GetRotateZ()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");

    grid->Collapse(wxString(base_name + "Properties"));
}

int OnImagePropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxImage& image, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    std::string base_name = image.GetName();

    if (base_name + "Image" == name) {
        std::string imageFile = event.GetValue().GetString().ToStdString();
        ObtainAccessToURL(imageFile);
        image.SetImageFile(imageFile);
        image.NotifyImageFileChanged();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Image");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Image");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::Image");
        return 0;
    } else if (!locked && base_name + "ScaleX" == name) {
        image.SetScaleX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "ScaleY" == name) {
        image.SetScaleY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "ScaleZ" == name) {
        image.SetScaleZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (locked && base_name + "ScaleZ" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetX" == name) {
        image.SetOffsetX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelX");
        return 0;
    } else if (locked && base_name + "OffsetX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetY" == name) {
        image.SetOffsetY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelY");
        return 0;
    } else if (locked && base_name + "OffsetY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "OffsetZ" == name) {
        image.SetOffsetZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelZ");
        return 0;
    } else if (locked && base_name + "OffsetZ" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateX" == name) {
        image.SetRotateX(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateX");
        return 0;
    } else if (locked && base_name + "RotateX" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateY" == name) {
        image.SetRotateY(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateY");
        return 0;
    } else if (locked && base_name + "RotateY" == name) {
        event.Veto();
        return 0;
    } else if (!locked && base_name + "RotateZ" == name) {
        image.SetRotateZ(event.GetValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateZ");
        return 0;
    } else if (locked && base_name + "RotateZ" == name) {
        event.Veto();
        return 0;
    }

    return -1;
}

} // namespace DmxComponentPropertyHelpers
