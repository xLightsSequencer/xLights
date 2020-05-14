/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/editors.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxSkull.h"
#include "Mesh.h"
#include "Servo.h"
#include "SkullConfigDialog.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxSkull::DmxSkull(wxXmlNode* node, const ModelManager& manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased)
{
    wxStandardPaths stdp = wxStandardPaths::Get();
#ifndef __WXMSW__
    obj_path = wxStandardPaths::Get().GetResourcesDir() + "/meshobjects/Skull/";
#else
    obj_path = wxFileName(stdp.GetExecutablePath()).GetPath() + "/meshobjects/Skull/";
#endif

    color_ability = this;
    default_channels[JAW] = 1;
    default_channels[PAN] = 3;
    default_channels[TILT] = 5;
    default_channels[NOD] = 7;
    default_channels[EYE_UD] = 9;
    default_channels[EYE_LR] = 11;

    default_min_limit[JAW] = 500;
    default_min_limit[PAN] = 400;
    default_min_limit[TILT] = 442;
    default_min_limit[NOD] = 452;
    default_min_limit[EYE_UD] = 575;
    default_min_limit[EYE_LR] = 499;

    default_max_limit[JAW] = 750;
    default_max_limit[PAN] = 1100;
    default_max_limit[TILT] = 836;
    default_max_limit[NOD] = 745;
    default_max_limit[EYE_UD] = 1000;
    default_max_limit[EYE_LR] = 878;

    default_orient[JAW] = 0;
    default_orient[PAN] = 90;
    default_orient[TILT] = -20;
    default_orient[NOD] = 29;
    default_orient[EYE_UD] = 35;
    default_orient[EYE_LR] = -35;

    default_range_of_motion[JAW] = -20;
    default_range_of_motion[PAN] = 180;
    default_range_of_motion[TILT] = -40;
    default_range_of_motion[NOD] = 58;
    default_range_of_motion[EYE_UD] = 70;
    default_range_of_motion[EYE_LR] = -70;

    default_node_names = "Jaw,-Jaw Fine,Pan,-Pan Fine,Tilt,-Tilt Fine,Nod,-Nod Fine,Eye UD,-Eye UD Fine,Eye LR,-Eye LR Fine,-Torso,-Torso Fine,Eye Brightness,Eye Red,Eye Green,Eye Blue";

    SetFromXml(node, zeroBased);
    screenLocation.CreateWithDepth(true);
}

DmxSkull::~DmxSkull()
{
    //dtor
}

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3(float x_, float y_, float z_, int cx_, int cy_, float scale_, float pan_angle_, float tilt_angle_, float nod_angle_ = 0.0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);
        float nod_angle = wxDegToRad(nod_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrixNod = glm::rotate(glm::mat4(1.0f), nod_angle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3((float)cx_, (float)cy_, 0.0f));
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_));
        glm::vec4 model_position = translateMatrix * rotationMatrixPan * rotationMatrixTilt * rotationMatrixNod * scaleMatrix * position;
        x = model_position.x;
        y = model_position.y;
    }
};

class dmxPoint3d {

public:
    float x;
    float y;
    float z;

    dmxPoint3d(float x_, float y_, float z_, float cx_, float cy_, float cz_, float scale_, float pan_angle_, float tilt_angle_, float nod_angle_ = 0.0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);
        float nod_angle = wxDegToRad(nod_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrixNod = glm::rotate(glm::mat4(1.0f), nod_angle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cx_, cy_, cz_));
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_));
        glm::vec4 model_position = translateMatrix * rotationMatrixPan * rotationMatrixTilt * rotationMatrixNod * scaleMatrix * position;
        x = model_position.x;
        y = model_position.y;
        z = model_position.z;
    }
};

static void CheckResult(bool value, bool old_value, DmxSkull* model, const std::string attribute, bool& change) {
    if (value != old_value) {
        model->GetModelXml()->DeleteAttribute(attribute);
        model->GetModelXml()->AddAttribute(attribute, std::to_string(value));
        change = true;
    }
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class SkullConfigDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    SkullConfigDialogAdapter(DmxSkull* model)
        : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override {
        SkullConfigDialog dlg(propGrid);

        dlg.CheckBox_16bits->SetValue(m_model->Is16Bit());
        dlg.CheckBox_Jaw->SetValue(m_model->HasJaw());
        dlg.CheckBox_Pan->SetValue(m_model->HasPan());
        dlg.CheckBox_Tilt->SetValue(m_model->HasTilt());
        dlg.CheckBox_Nod->SetValue(m_model->HasNod());
        dlg.CheckBox_EyeLR->SetValue(m_model->HasEyeLR());
        dlg.CheckBox_EyeUD->SetValue(m_model->HasEyeUD());
        dlg.CheckBox_Color->SetValue(m_model->HasColor());
        dlg.CheckBox_Skulltronix->SetValue(m_model->IsSkulltronix());

        if (dlg.ShowModal() == wxID_OK) {
            bool changed = false;

            CheckResult(dlg.CheckBox_Jaw->GetValue(), m_model->HasJaw(), m_model, "HasJaw", changed);
            CheckResult(dlg.CheckBox_Pan->GetValue(), m_model->HasPan(), m_model, "HasPan", changed);
            CheckResult(dlg.CheckBox_Tilt->GetValue(), m_model->HasTilt(), m_model, "HasTilt", changed);
            CheckResult(dlg.CheckBox_Nod->GetValue(), m_model->HasNod(), m_model, "HasNod", changed);
            CheckResult(dlg.CheckBox_EyeLR->GetValue(), m_model->HasEyeLR(), m_model, "HasEyeLR", changed);
            CheckResult(dlg.CheckBox_EyeUD->GetValue(), m_model->HasEyeUD(), m_model, "HasEyeUD", changed);
            CheckResult(dlg.CheckBox_Color->GetValue(), m_model->HasColor(), m_model, "HasColor", changed);
            CheckResult(dlg.CheckBox_16bits->GetValue(), m_model->Is16Bit(), m_model, "Bits16", changed);
            CheckResult(dlg.CheckBox_Skulltronix->GetValue(), m_model->IsSkulltronix(), m_model, "Skulltronix", changed);

            if (dlg.CheckBox_Skulltronix->GetValue()) {
                if (!m_model->IsSkulltronix()) {
                    m_model->SetSkulltronix();
                }
            }

            if (changed) {
                // TODO - Make the default node names dynamic when selected servos is changed
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
        int type)
        : wxStringProperty(label, name, value), m_model(m), m_tp(type) {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
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

void DmxSkull::AddTypeProperties(wxPropertyGridInterface* grid) {

    DmxModel::AddTypeProperties(grid);

    wxPGProperty* p = grid->Append(new SkullPopupDialogProperty(this, "Skull Config", "SkullConfig", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxBoolProperty("Mesh Only", "MeshOnly", mesh_only));
    p->SetAttribute("UseCheckbox", true);

    if (has_jaw && jaw_servo != nullptr) {
        jaw_servo->AddTypeProperties(grid);
    }
    if (has_pan && pan_servo != nullptr) {
        pan_servo->AddTypeProperties(grid);
    }
    if (has_tilt && tilt_servo != nullptr) {
        tilt_servo->AddTypeProperties(grid);
    }
    if (has_nod && nod_servo != nullptr) {
        nod_servo->AddTypeProperties(grid);
    }
    if (has_eye_ud && eye_ud_servo != nullptr) {
        eye_ud_servo->AddTypeProperties(grid);
    }
    if (has_eye_lr && eye_lr_servo != nullptr) {
        eye_lr_servo->AddTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Orientation Properties", "OrientProperties"));

    if (has_pan) {
        p = grid->Append(new wxIntProperty("Pan Orientation", "DmxPanOrient", pan_orient));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (has_tilt) {
        p = grid->Append(new wxIntProperty("Tilt Orientation", "DmxTiltOrient", tilt_orient));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (has_nod) {
        p = grid->Append(new wxIntProperty("Nod Orientation", "DmxNodOrient", nod_orient));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (has_eye_ud) {
        p = grid->Append(new wxIntProperty("Eye Up/Down Orientation", "DmxEyeUDOrient", eye_ud_orient));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (has_eye_lr) {
        p = grid->Append(new wxIntProperty("Eye Left/Right Orientation", "DmxEyeLROrient", eye_lr_orient));
        p->SetAttribute("Min", -360);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (has_color) {
        grid->Append(new wxPropertyCategory("Color Properties", "ColorProperties"));

        p = grid->Append(new wxUIntProperty("Eye Brightness Channel", "DmxEyeBrtChannel", eye_brightness_channel));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 512);
        p->SetEditor("SpinCtrl");

        AddColorTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxSkull::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("MeshOnly" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("MeshOnly");
        mesh_only = event.GetValue().GetBool();
        if (mesh_only) {
            ModelXml->AddAttribute("MeshOnly", "1");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::MeshOnly");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::MeshOnly");
        return 0;
    }

    if (has_color) {
        if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
            return 0;
        }
    }

    if (has_jaw) {
        if (jaw_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    if (has_pan) {
        if (pan_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
        if ("DmxPanOrient" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxPanOrient");
            ModelXml->AddAttribute("DmxPanOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXPanOrient");
            return 0;
        }
    }

    if (has_tilt) {
        if (tilt_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
        if ("DmxTiltOrient" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxTiltOrient");
            ModelXml->AddAttribute("DmxTiltOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXTiltOrient");
            return 0;
        }
    }

    if (has_nod) {
        if (nod_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
        if ("DmxNodOrient" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxNodOrient");
            ModelXml->AddAttribute("DmxNodOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DMXNodOrient");
            return 0;
        }
    }

    if (has_eye_ud) {
        if (eye_ud_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
        if ("DmxEyeUDOrient" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxEyeUDOrient");
            ModelXml->AddAttribute("DmxEyeUDOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DmxEyeUDOrient");
            return 0;
        }
    }

    if (has_eye_lr) {
        if (eye_lr_servo->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
        if ("DmxEyeLROrient" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxEyeLROrient");
            ModelXml->AddAttribute("DmxEyeLROrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkull::OnPropertyGridChange::DmxEyeLROrient");
            return 0;
        }
    }

    if (has_color) {
        if ("DmxEyeBrtChannel" == event.GetPropertyName()) {
            ModelXml->DeleteAttribute("DmxEyeBrtChannel");
            ModelXml->AddAttribute("DmxEyeBrtChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkull::OnPropertyGridChange::DMXEyeBrtChannel");
            return 0;
        }
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxSkull::FixObjFile(wxXmlNode* node, const std::string& objfile) {
    if (node->HasAttribute("ObjFile")) {
        node->DeleteAttribute("ObjFile");
    }
    wxString f = obj_path + objfile;
    node->AddAttribute("ObjFile", f);
}

void DmxSkull::AddServo(Servo** _servo, const std::string& name, int type, const std::string& style) {
    if (*_servo == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, name);
        ModelXml->AddChild(new_node);
        Servo* s = new Servo(new_node, name, false);
        s->SetMinLimit(default_min_limit[type]);
        s->SetMaxLimit(default_max_limit[type]);
        s->SetRangeOfMotion(default_range_of_motion[type]);
        s->SetStyle(style);
        s->SetChannel(default_channels[type], this);
        *_servo = s;
    }
    (*_servo)->Init(this);
    (*_servo)->Set16Bit(_16bit);
}

void DmxSkull::AddMesh(Mesh** _mesh, const std::string& name, const std::string& objfile, bool set_size) {
    if (*_mesh == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, name);
        ModelXml->AddChild(new_node);
        Mesh* m = new Mesh(new_node, name);
        *_mesh = m;
        FixObjFile(new_node, objfile);
    }
    (*_mesh)->Init(this, set_size);
}

void DmxSkull::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxSkull";
    screenLocation.SetRenderSize(1, 1, 1);

    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "16"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "17"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "18"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
    eye_brightness_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeBrtChannel", "15"));
    pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", std::to_string(default_orient[PAN])));
    tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", std::to_string(default_orient[TILT])));
    nod_orient = wxAtoi(ModelXml->GetAttribute("DmxNodOrient", std::to_string(default_orient[NOD])));
    eye_ud_orient = wxAtoi(ModelXml->GetAttribute("DmxEyeUDOrient", std::to_string(default_orient[EYE_UD])));
    eye_lr_orient = wxAtoi(ModelXml->GetAttribute("DmxEyeLROrient", std::to_string(default_orient[EYE_LR])));
    has_jaw = wxAtoi(ModelXml->GetAttribute("HasJaw", "1"));
    has_pan = wxAtoi(ModelXml->GetAttribute("HasPan", "1"));
    has_tilt = wxAtoi(ModelXml->GetAttribute("HasTilt", "1"));
    has_nod = wxAtoi(ModelXml->GetAttribute("HasNod", "1"));
    has_eye_ud = wxAtoi(ModelXml->GetAttribute("HasEyeUD", "1"));
    has_eye_lr = wxAtoi(ModelXml->GetAttribute("HasEyeLR", "1"));
    has_color = wxAtoi(ModelXml->GetAttribute("HasColor", "1"));
    is_skulltronix = wxAtoi(ModelXml->GetAttribute("Skulltronix", "0"));
    _16bit = wxAtoi(ModelXml->GetAttribute("Bits16", "1"));
    mesh_only = ModelXml->GetAttribute("MeshOnly", "0") == "1";

    SetNodeNames(default_node_names);

    wxXmlNode* n = ModelXml->GetChildren();

    while (n != nullptr) {
        std::string name = n->GetName();

        if ("HeadMesh" == name) {
            if (head_mesh == nullptr) {
                FixObjFile(n, "SkullHead.obj");
                head_mesh = new Mesh(n, "HeadMesh");
            }
        }
        else if ("JawMesh" == name) {
            if (jaw_mesh == nullptr) {
                FixObjFile(n, "SkullJaw.obj");
                jaw_mesh = new Mesh(n, "JawMesh");
            }
        }
        else if ("EyeMeshL" == name) {
            if (eye_l_mesh == nullptr) {
                FixObjFile(n, "Eyeball.obj");
                eye_l_mesh = new Mesh(n, "EyeMeshL");
            }
        }
        else if ("EyeMeshR" == name) {
            if (eye_r_mesh == nullptr) {
                FixObjFile(n, "Eyeball.obj");
                eye_r_mesh = new Mesh(n, "EyeMeshR");
            }
        }
        else if ("JawServo" == name) {
            if (has_jaw && jaw_servo == nullptr) {
                jaw_servo = new Servo(n, "JawServo", false);
            }
        }
        else if ("PanServo" == name) {
            if (has_pan && pan_servo == nullptr) {
                pan_servo = new Servo(n, "PanServo", false);
            }
        }
        else if ("TiltServo" == name) {
            if (has_tilt && tilt_servo == nullptr) {
                tilt_servo = new Servo(n, "TiltServo", false);
            }
        }
        else if ("NodServo" == name) {
            if (has_nod && nod_servo == nullptr) {
                nod_servo = new Servo(n, "NodServo", false);
            }
        }
        else if ("EyeUpDownServo" == name) {
            if (has_eye_ud && eye_ud_servo == nullptr) {
                eye_ud_servo = new Servo(n, "EyeUpDownServo", false);
            }
        }
        else if ("EyeLeftRightServo" == name) {
            if (has_eye_lr && eye_lr_servo == nullptr) {
                eye_lr_servo = new Servo(n, "EyeLeftRightServo", false);
            }
        }
        n = n->GetNext();
    }

    // create any missing servos
    if( has_jaw ) AddServo(&jaw_servo, "JawServo", JAW, "Rotate X");
    if (has_pan) AddServo(&pan_servo, "PanServo", PAN, "Rotate Y");
    if (has_tilt) AddServo(&tilt_servo, "TiltServo", TILT, "Rotate Z");
    if (has_nod) AddServo(&nod_servo, "NodServo", NOD, "Rotate X");
    if (has_eye_ud) AddServo(&eye_ud_servo, "EyeUpDownServo", EYE_UD, "Rotate X");
    if (has_eye_lr) AddServo(&eye_lr_servo, "EyeLeftRightServo", EYE_LR, "Rotate Y");

    // create any missing meshes
    AddMesh(&head_mesh, "HeadMesh", "SkullHead.obj", false);
    head_mesh->SetHalfHeight();  // obj file is shifted up so its twice as tall as it need to be
    AddMesh(&jaw_mesh, "JawMesh", "SkullJaw.obj", false);
    AddMesh(&eye_l_mesh, "EyeMeshL", "Eyeball.obj", false);
    AddMesh(&eye_r_mesh, "EyeMeshR", "Eyeball.obj", false);

    head_mesh->SetMeshOnly(mesh_only);
    jaw_mesh->SetMeshOnly(mesh_only);
    eye_l_mesh->SetMeshOnly(mesh_only);
    eye_r_mesh->SetMeshOnly(mesh_only);

    if (setup_skulltronix) {
        SetupSkulltronix();
    }
}

float DmxSkull::GetServoPos(Servo* _servo, bool active) {
    float servo_pos = 0.0f;
    if (active && _servo->GetChannel() > 0) {
        servo_pos = _servo->GetPosition(GetChannelValue(_servo->GetChannel() - 1, _servo->Is16Bit()));
        if (_servo->IsTranslate()) {
            glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
            servo_pos /= scale.x;
        }
    }
    return servo_pos;
}

void DmxSkull::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) {
    if (!IsActive()) return;

    DrawGLUtils::xl3Accumulator dummy;
    DrawModel(preview, va, dummy, c, sx, sy, active, false);
}

void DmxSkull::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) {
    if (!IsActive()) return;

    DrawGLUtils::xlAccumulator dummy;
    DrawModel(preview, dummy, va, c, sx, sy, active, true);
}

void DmxSkull::DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor *c, float &sx, float &sy, bool active, bool is_3d)
{
    size_t NodeCount=Nodes.size();
    DrawGLUtils::xlAccumulator& va = is_3d ? va3 : va2;

    // crash protection
    if( eye_brightness_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount )
    {
        return;
    }

    if (has_jaw && jaw_servo->GetChannel() > Nodes.size()) { return; }
    if (has_pan && pan_servo->GetChannel() > Nodes.size()) { return; }
    if (has_tilt && tilt_servo->GetChannel() > Nodes.size()) { return; }
    if (has_nod && nod_servo->GetChannel() > Nodes.size()) { return; }
    if (has_eye_ud && eye_ud_servo->GetChannel() > Nodes.size()) { return; }
    if (has_eye_lr && eye_lr_servo->GetChannel() > Nodes.size()) { return; }


    xlColor ccolor(xlWHITE);
    xlColor eye_color(xlWHITE);
    xlColor black(xlBLACK);
    xlColor base_color(200, 200, 200);
    xlColor base_color2(150, 150, 150);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int dmx_size = ((BoxedScreenLocation)screenLocation).GetScaleX();
    float radius = (float)(dmx_size) / 2.0f;
    xlColor color_angle;

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if (has_color) {
        if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
            xlColor proxy;
            Nodes[red_channel - 1]->GetColor(proxy);
            eye_color.red = proxy.red;
            Nodes[green_channel - 1]->GetColor(proxy);
            eye_color.green = proxy.red;
            Nodes[blue_channel - 1]->GetColor(proxy);
            eye_color.blue = proxy.red;
        }
    }
    else {
        eye_color = xlWHITE;
    }
    if( (eye_color.red == 0 && eye_color.green == 0 && eye_color.blue == 0) || !active ) {
        eye_color = xlWHITE;
    } else {
        ApplyTransparency(eye_color, trans, trans);
    }
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);

    float sf = 12.0f;
    float scale = radius / sf;

    // Get servo positions
    float pan_pos, tilt_pos, nod_pos, jaw_pos, eye_x_pos, eye_y_pos;
    pan_pos = GetServoPos(pan_servo, active && has_pan) + (active? pan_orient : 0.0f);
    tilt_pos = GetServoPos(tilt_servo, active && has_tilt) + (active ? tilt_orient : 0.0f);
    nod_pos = GetServoPos(nod_servo, active && has_nod) + (active ? nod_orient : 0.0f);
    jaw_pos = GetServoPos(jaw_servo, active && has_jaw);
    eye_x_pos = GetServoPos(eye_lr_servo, active && has_eye_lr) + (active ? eye_lr_orient : 0.0f);
    eye_y_pos = GetServoPos(eye_ud_servo, active && has_eye_ud) + (active ? eye_ud_orient : 0.0f);

    if (is_3d) {
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::vec3 scaling = GetModelScreenLocation().GetScaleMatrix();
        glm::mat4 scalingMatrix = glm::scale(Identity, scaling);
        glm::vec3 world = GetModelScreenLocation().GetWorldPosition();
        glm::mat4 translateMatrix = glm::translate(Identity, world);
        glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
        glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;
        glm::mat4 jaw_matrix = Identity;
        glm::mat4 pan_matrix = Identity;
        glm::mat4 tilt_matrix = Identity;
        glm::mat4 nod_matrix = Identity;
        glm::mat4 eye_x_matrix = Identity;
        glm::mat4 eye_y_matrix = Identity;

        // Fill motion matrices
        if (has_jaw) jaw_servo->FillMotionMatrix(jaw_pos, jaw_matrix);
        if (has_pan) pan_servo->FillMotionMatrix(pan_pos, pan_matrix);
        if (has_tilt) tilt_servo->FillMotionMatrix(tilt_pos, tilt_matrix);
        if (has_nod) nod_servo->FillMotionMatrix(nod_pos, nod_matrix);
        if (has_eye_lr) eye_lr_servo->FillMotionMatrix(eye_x_pos, eye_x_matrix);
        if (has_eye_ud) eye_ud_servo->FillMotionMatrix(eye_y_pos, eye_y_matrix);

        // Adjust scaling to render size of 1
        float jaw_pivot_y = 3.3f;;
        float jaw_pivot_z = 0.4f;
        if (head_mesh->GetExists()) {
            float w = head_mesh->GetWidth();
            float scale = 1.0f / w;
            head_mesh->SetRenderScaling(scale);
            jaw_mesh->SetRenderScaling(scale);
            eye_l_mesh->SetRenderScaling(scale);
            eye_r_mesh->SetRenderScaling(scale);
            GetBaseObjectScreenLocation().UpdateBoundingBox(head_mesh->GetWidth(), head_mesh->GetHeight(), head_mesh->GetDepth());  // FIXME: Modify to only call this when position changes
            eye_l_mesh->SetOffsetX(-1.0f * scale);
            eye_l_mesh->SetOffsetY(4.5f * scale);
            eye_l_mesh->SetOffsetZ(3.2f * scale);
            eye_r_mesh->SetOffsetX(1.0f * scale);
            eye_r_mesh->SetOffsetY(4.5f * scale);
            eye_r_mesh->SetOffsetZ(3.2f * scale);
            jaw_pivot_y *= scale;
            jaw_pivot_z *= scale;
        }

        // Draw Meshs
        glm::mat4 head_matrix = pan_matrix * tilt_matrix * nod_matrix;
        glm::mat4 head_base = base_matrix * head_matrix;
        eye_x_matrix = eye_x_matrix * eye_y_matrix;
        eye_l_mesh->SetColor(eye_color, "EyeColor");
        eye_r_mesh->SetColor(eye_color, "EyeColor");
        head_mesh->Draw(this, preview, va3, base_matrix, head_matrix, false, 0, 0, 0, false, false);
        jaw_mesh->Draw(this, preview, va3, head_base, jaw_matrix, false, 0, jaw_pivot_y, jaw_pivot_z, true, false);
        eye_l_mesh->Draw(this, preview, va3, head_base, eye_x_matrix, false, 0, 0, 0, false, false);
        eye_r_mesh->Draw(this, preview, va3, head_base, eye_x_matrix, false, 0, 0, 0, false, false);
    }
    else {
        float eye_range_of_motion = 3.8f;
        float eye_lr_rom = has_eye_lr ? -eye_lr_servo->GetRangeOfMotion() : -70.0f;
        float eye_ud_rom = has_eye_ud ? -eye_ud_servo->GetRangeOfMotion() : -70.0f;

        if (!active) {
            // static positions in layout
            jaw_pos = -0.5f;
            pan_pos = 0.5f;
            tilt_pos = 0.5f;
            nod_pos = 0.5f;
            eye_x_pos = 0.5f * eye_range_of_motion - eye_range_of_motion / 2.0;
            eye_y_pos = 0.5f * eye_range_of_motion - eye_range_of_motion / 2.0;
        }
        else {
            jaw_pos = -jaw_pos / 5.0f - 0.5f;
            eye_x_pos = (eye_x_pos - eye_lr_orient) / eye_lr_rom * eye_range_of_motion - eye_range_of_motion / 2.0;
            eye_y_pos = (eye_y_pos - eye_ud_orient) / eye_ud_rom * eye_range_of_motion - eye_range_of_motion / 2.0;
        }

        // Create Head
        dmxPoint3 p1(-7.5f, 13.7f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p2(7.5f, 13.7f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p3(13.2f, 6.0f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p8(-13.2f, 6.0f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p4(9, -11.4f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p7(-9, -11.4f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p5(6.3f, -16, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p6(-6.3f, -16, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        dmxPoint3 p9(0, 3.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p10(-2.5f, -1.7f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p11(2.5f, -1.7f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        dmxPoint3 p14(0, -6.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p12(-6, -6.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p16(6, -6.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p13(-3, -11.4f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p15(3, -11.4f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        // Create Back of Head
        dmxPoint3 p1b(-7.5f, 13.7f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p2b(7.5f, 13.7f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p3b(13.2f, 6.0f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p8b(-13.2f, 6.0f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p4b(9, -11.4f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p7b(-9, -11.4f, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p5b(6.3f, -16, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p6b(-6.3f, -16, -3, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        // Create Lower Mouth
        dmxPoint3 p4m(9, -11.4f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p7m(-9, -11.4f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p5m(6.3f, -16 + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p6m(-6.3f, -16 + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p14m(0, -6.5f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p12m(-6, -6.5f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p16m(6, -6.5f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p13m(-3, -11.4f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 p15m(3, -11.4f + jaw_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        // Create Eyes
        dmxPoint3 left_eye_socket(-5, 7.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 right_eye_socket(5, 7.5f, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 left_eye(-5 + eye_x_pos, 7.5f + eye_y_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);
        dmxPoint3 right_eye(5 + eye_x_pos, 7.5f + eye_y_pos, 0.0f, sx, sy, scale, pan_pos, tilt_pos, nod_pos);

        // Draw Back of Head
        va.AddVertex(p1.x, p1.y, base_color2);
        va.AddVertex(p1b.x, p1b.y, base_color2);
        va.AddVertex(p2.x, p2.y, base_color2);
        va.AddVertex(p2b.x, p2b.y, base_color2);
        va.AddVertex(p1b.x, p1b.y, base_color2);
        va.AddVertex(p2.x, p2.y, base_color2);

        va.AddVertex(p2.x, p2.y, base_color);
        va.AddVertex(p2b.x, p2b.y, base_color);
        va.AddVertex(p3.x, p3.y, base_color);
        va.AddVertex(p3b.x, p3b.y, base_color);
        va.AddVertex(p2b.x, p2b.y, base_color);
        va.AddVertex(p3.x, p3.y, base_color);

        va.AddVertex(p3.x, p3.y, base_color2);
        va.AddVertex(p3b.x, p3b.y, base_color2);
        va.AddVertex(p4.x, p4.y, base_color2);
        va.AddVertex(p4b.x, p4b.y, base_color2);
        va.AddVertex(p3b.x, p3b.y, base_color2);
        va.AddVertex(p4.x, p4.y, base_color2);

        va.AddVertex(p4.x, p4.y, base_color);
        va.AddVertex(p4b.x, p4b.y, base_color);
        va.AddVertex(p5.x, p5.y, base_color);
        va.AddVertex(p5b.x, p5b.y, base_color);
        va.AddVertex(p4b.x, p4b.y, base_color);
        va.AddVertex(p5.x, p5.y, base_color);

        va.AddVertex(p5.x, p5.y, base_color2);
        va.AddVertex(p5b.x, p5b.y, base_color2);
        va.AddVertex(p6.x, p6.y, base_color2);
        va.AddVertex(p6b.x, p6b.y, base_color2);
        va.AddVertex(p5b.x, p5b.y, base_color2);
        va.AddVertex(p6.x, p6.y, base_color2);

        va.AddVertex(p6.x, p6.y, base_color);
        va.AddVertex(p6b.x, p6b.y, base_color);
        va.AddVertex(p7.x, p7.y, base_color);
        va.AddVertex(p7b.x, p7b.y, base_color);
        va.AddVertex(p6b.x, p6b.y, base_color);
        va.AddVertex(p7.x, p7.y, base_color);

        va.AddVertex(p7.x, p7.y, base_color2);
        va.AddVertex(p7b.x, p7b.y, base_color2);
        va.AddVertex(p8.x, p8.y, base_color2);
        va.AddVertex(p8b.x, p8b.y, base_color2);
        va.AddVertex(p7b.x, p7b.y, base_color2);
        va.AddVertex(p8.x, p8.y, base_color2);

        va.AddVertex(p8.x, p8.y, base_color);
        va.AddVertex(p8b.x, p8b.y, base_color);
        va.AddVertex(p1.x, p1.y, base_color);
        va.AddVertex(p1b.x, p1b.y, base_color);
        va.AddVertex(p8b.x, p8b.y, base_color);
        va.AddVertex(p1.x, p1.y, base_color);

        // Draw Front of Head
        va.AddVertex(p1.x, p1.y, ccolor);
        va.AddVertex(p2.x, p2.y, ccolor);
        va.AddVertex(p9.x, p9.y, ccolor);

        va.AddVertex(p2.x, p2.y, ccolor);
        va.AddVertex(p9.x, p9.y, ccolor);
        va.AddVertex(p11.x, p11.y, ccolor);

        va.AddVertex(p1.x, p1.y, ccolor);
        va.AddVertex(p9.x, p9.y, ccolor);
        va.AddVertex(p10.x, p10.y, ccolor);

        va.AddVertex(p1.x, p1.y, ccolor);
        va.AddVertex(p8.x, p8.y, ccolor);
        va.AddVertex(p10.x, p10.y, ccolor);

        va.AddVertex(p2.x, p2.y, ccolor);
        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p11.x, p11.y, ccolor);

        va.AddVertex(p8.x, p8.y, ccolor);
        va.AddVertex(p10.x, p10.y, ccolor);
        va.AddVertex(p12.x, p12.y, ccolor);

        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p11.x, p11.y, ccolor);
        va.AddVertex(p16.x, p16.y, ccolor);

        va.AddVertex(p7.x, p7.y, ccolor);
        va.AddVertex(p8.x, p8.y, ccolor);
        va.AddVertex(p12.x, p12.y, ccolor);

        va.AddVertex(p3.x, p3.y, ccolor);
        va.AddVertex(p4.x, p4.y, ccolor);
        va.AddVertex(p16.x, p16.y, ccolor);

        va.AddVertex(p10.x, p10.y, ccolor);
        va.AddVertex(p12.x, p12.y, ccolor);
        va.AddVertex(p14.x, p14.y, ccolor);

        va.AddVertex(p10.x, p10.y, ccolor);
        va.AddVertex(p11.x, p11.y, ccolor);
        va.AddVertex(p14.x, p14.y, ccolor);

        va.AddVertex(p11.x, p11.y, ccolor);
        va.AddVertex(p14.x, p14.y, ccolor);
        va.AddVertex(p16.x, p16.y, ccolor);

        va.AddVertex(p12.x, p12.y, ccolor);
        va.AddVertex(p13.x, p13.y, ccolor);
        va.AddVertex(p14.x, p14.y, ccolor);

        va.AddVertex(p14.x, p14.y, ccolor);
        va.AddVertex(p15.x, p15.y, ccolor);
        va.AddVertex(p16.x, p16.y, ccolor);

        // Draw Lower Mouth
        va.AddVertex(p4m.x, p4m.y, ccolor);
        va.AddVertex(p6m.x, p6m.y, ccolor);
        va.AddVertex(p7m.x, p7m.y, ccolor);

        va.AddVertex(p4m.x, p4m.y, ccolor);
        va.AddVertex(p5m.x, p5m.y, ccolor);
        va.AddVertex(p6m.x, p6m.y, ccolor);

        va.AddVertex(p7m.x, p7m.y, ccolor);
        va.AddVertex(p12m.x, p12m.y, ccolor);
        va.AddVertex(p13m.x, p13m.y, ccolor);

        va.AddVertex(p13m.x, p13m.y, ccolor);
        va.AddVertex(p14m.x, p14m.y, ccolor);
        va.AddVertex(p15m.x, p15m.y, ccolor);

        va.AddVertex(p4m.x, p4m.y, ccolor);
        va.AddVertex(p15m.x, p15m.y, ccolor);
        va.AddVertex(p16m.x, p16m.y, ccolor);

        // Draw Eyes
        va.AddTrianglesCircle(left_eye_socket.x, left_eye_socket.y, scale * sf * 0.25, black, black);
        va.AddTrianglesCircle(right_eye_socket.x, right_eye_socket.y, scale * sf * 0.25, black, black);
        va.AddTrianglesCircle(left_eye.x, left_eye.y, scale * sf * 0.10, eye_color, eye_color);
        va.AddTrianglesCircle(right_eye.x, right_eye.y, scale * sf * 0.10, eye_color, eye_color);

    }
    va.Finish(GL_TRIANGLES);
}

void DmxSkull::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    f.Write(wxString::Format("name=\"%s\" ", name));

    ExportBaseParameters(f);

    wxString po = ModelXml->GetAttribute("DmxPanOrient", std::to_string(default_orient[PAN]));
    wxString to = ModelXml->GetAttribute("DmxTiltOrient", std::to_string(default_orient[TILT]));
    wxString no = ModelXml->GetAttribute("DmxNodOrient", std::to_string(default_orient[NOD]));
    wxString eud = ModelXml->GetAttribute("DmxEyeUDOrient", std::to_string(default_orient[EYE_UD]));
    wxString elr = ModelXml->GetAttribute("DmxEyeLROrient", std::to_string(default_orient[EYE_LR]));
    wxString hj = ModelXml->GetAttribute("HasJaw", "1");
    wxString hp = ModelXml->GetAttribute("HasPan", "1");
    wxString ht = ModelXml->GetAttribute("HasTilt", "1");
    wxString hn = ModelXml->GetAttribute("HasNod", "1");
    wxString heu = ModelXml->GetAttribute("HasEyeUD", "1");
    wxString hel = ModelXml->GetAttribute("HasEyeLR", "1");
    wxString hc = ModelXml->GetAttribute("HasColor", "1");
    wxString is = ModelXml->GetAttribute("Skulltronix", "0");
    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "24");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "25");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "26");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    wxString eb = ModelXml->GetAttribute("DmxEyeBrtChannel", "23");
    wxString bits = ModelXml->GetAttribute("Bits16");

    wxString v = xlights_version_string;

    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("DmxPanOrient=\"%s\" ", po));
    f.Write(wxString::Format("DmxTiltOrient=\"%s\" ", to));
    f.Write(wxString::Format("DmxNodOrient=\"%s\" ", no));
    f.Write(wxString::Format("DmxEyeUDOrient=\"%s\" ", eud));
    f.Write(wxString::Format("DmxEyeLROrient=\"%s\" ", elr));
    f.Write(wxString::Format("HasJaw=\"%s\" ", hj));
    f.Write(wxString::Format("HasPan=\"%s\" ", hp));
    f.Write(wxString::Format("HasTilt=\"%s\" ", ht));
    f.Write(wxString::Format("HasNod=\"%s\" ", hn));
    f.Write(wxString::Format("HasEyeUD=\"%s\" ", heu));
    f.Write(wxString::Format("HasEyeLR=\"%s\" ", hel));
    f.Write(wxString::Format("HasColor=\"%s\" ", hc));
    f.Write(wxString::Format("Skulltronix=\"%s\" ", is));
    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
    f.Write(wxString::Format("DmxEyeBrtChannel=\"%s\" ", eb));
    f.Write(wxString::Format("Bits16=\"%s\" ", bits));
    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

    if (has_jaw) jaw_servo->Serialise(ModelXml, f, show_dir);
    if (has_pan) pan_servo->Serialise(ModelXml, f, show_dir);
    if (has_tilt) tilt_servo->Serialise(ModelXml, f, show_dir);
    if (has_nod) nod_servo->Serialise(ModelXml, f, show_dir);
    if (has_eye_lr) eye_lr_servo->Serialise(ModelXml, f, show_dir);
    if (has_eye_ud) eye_ud_servo->Serialise(ModelXml, f, show_dir);

    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    f.Write("</dmxmodel>");
    f.Close();
}

void DmxSkull::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxmodel")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");

            wxString po = root->GetAttribute("DmxPanOrient");
            wxString to = root->GetAttribute("DmxTiltOrient");
            wxString no = root->GetAttribute("DmxNodOrient");
            wxString eud = root->GetAttribute("DmxEyeUDOrient");
            wxString elr = root->GetAttribute("DmxEyeLROrient");
            wxString hj = root->GetAttribute("HasJaw");
            wxString hp = root->GetAttribute("HasPan");
            wxString ht = root->GetAttribute("HasTilt");
            wxString hn = root->GetAttribute("HasNod");
            wxString heu = root->GetAttribute("HasEyeUD");
            wxString hel = root->GetAttribute("HasEyeLR");
            wxString hc = root->GetAttribute("HasColor");
            wxString is = root->GetAttribute("Skulltronix");
            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");
            wxString eb = root->GetAttribute("DmxEyeBrtChannel");
            wxString bits = root->GetAttribute("Bits16");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("DmxPanOrient", po);
            SetProperty("DmxTiltOrient", to);
            SetProperty("DmxNodOrient", no);
            SetProperty("DmxEyeUDOrient", eud);
            SetProperty("DmxEyeLROrient", elr);
            SetProperty("HasJaw", hj);
            SetProperty("HasPan", hp);
            SetProperty("HasTilt", ht);
            SetProperty("HasNod", hn);
            SetProperty("HasEyeUD", heu);
            SetProperty("HasEyeLR", hel);
            SetProperty("HasColor", hc);
            SetProperty("Skulltronix", is);
            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);
            SetProperty("DmxEyeBrtChannel", eb);
            SetProperty("Bits16", bits);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

            if (hj == "1") jaw_servo->Serialise(root, ModelXml, show_dir);
            if (hp == "1") pan_servo->Serialise(root, ModelXml, show_dir);
            if (ht == "1") tilt_servo->Serialise(root, ModelXml, show_dir);
            if (hn == "1") nod_servo->Serialise(root, ModelXml, show_dir);
            if (hel == "1") eye_lr_servo->Serialise(root, ModelXml, show_dir);
            if (heu == "1") eye_ud_servo->Serialise(root, ModelXml, show_dir);

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
                else if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkull::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkull::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxSkull model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxSkull model file.");
    }
}

void DmxSkull::SetupServo(Servo* _servo, int channel, float min_limit, float max_limit, float range_of_motion, bool _16bit) {
    if (_servo != nullptr) {
        _servo->SetMinLimit(min_limit);
        _servo->SetMaxLimit(max_limit);
        _servo->SetRangeOfMotion(range_of_motion);
        _servo->Set16Bit(_16bit);
        _servo->SetChannel(channel, this);
    }
}

void DmxSkull::SetupSkulltronix() {

    SetupServo(jaw_servo, 9, 500, 750, -20, true);
    SetupServo(pan_servo, 13, 400, 1100, 180, true);
    SetupServo(tilt_servo, 19, 442, 836, -40, true);
    SetupServo(nod_servo, 11, 452, 745, 58, true);
    SetupServo(eye_ud_servo, 15, 575, 1000, 70, true);
    SetupServo(eye_lr_servo, 17, 499, 878, -70, true);

    ModelXml->DeleteAttribute("DmxPanOrient");
    ModelXml->AddAttribute("DmxPanOrient", "90");
    ModelXml->DeleteAttribute("DmxTiltOrient");
    ModelXml->AddAttribute("DmxTiltOrient", "-20");
    ModelXml->DeleteAttribute("DmxNodOrient");
    ModelXml->AddAttribute("DmxNodOrient", "29");
    ModelXml->DeleteAttribute("DmxEyeUDOrient");
    ModelXml->AddAttribute("DmxEyeUDOrient", "35");
    ModelXml->DeleteAttribute("DmxEyeLROrient");
    ModelXml->AddAttribute("DmxEyeLROrient", "-35");
    ModelXml->DeleteAttribute("DmxEyeBrtChannel");
    ModelXml->AddAttribute("DmxEyeBrtChannel", "23");
    ModelXml->DeleteAttribute("DmxRedChannel");
    ModelXml->AddAttribute("DmxRedChannel", "24");
    ModelXml->DeleteAttribute("DmxGreenChannel");
    ModelXml->AddAttribute("DmxGreenChannel", "25");
    ModelXml->DeleteAttribute("DmxBlueChannel");
    ModelXml->AddAttribute("DmxBlueChannel", "26");
    ModelXml->DeleteAttribute("DmxWhiteChannel");
    ModelXml->AddAttribute("DmxWhiteChannel", "0");

    red_channel = 24;
    green_channel = 25;
    blue_channel = 17;
    white_channel = 0;
    eye_brightness_channel = 23;
    pan_orient = 90;
    tilt_orient = -45;
    nod_orient = 29;
    eye_ud_orient = 35;
    eye_lr_orient = -35;

    SetNodeNames(",,,,,,,Power,Jaw,-Jaw Fine,Nod,-Nod Fine,Pan,-Pan Fine,Eye UD,-Eye UD Fine,Eye LR,-Eye LR Fine,Tilt,-Tilt Fine,-Torso,-Torso Fine,Eye Brightness,Eye Red,Eye Green,Eye Blue", true);
    setup_skulltronix = false;
}
