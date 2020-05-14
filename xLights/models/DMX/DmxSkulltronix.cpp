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
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxSkulltronix.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxSkulltronix::DmxSkulltronix(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased)
{
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxSkulltronix::~DmxSkulltronix()
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

void DmxSkulltronix::AddTypeProperties(wxPropertyGridInterface *grid) {

    DmxModel::AddTypeProperties(grid);

    AddPanTiltTypeProperties(grid);

    wxPGProperty *p = grid->Append(new wxUIntProperty("Pan Min Limit", "DmxPanMinLimit", pan_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Pan Max Limit", "DmxPanMaxLimit", pan_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Tilt Min Limit", "DmxTiltMinLimit", tilt_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Tilt Max Limit", "DmxTiltMaxLimit", tilt_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nod Channel", "DmxNodChannel", nod_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nod Orientation", "DmxNodOrient", nod_orient));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nod Deg of Rot", "DmxNodDegOfRot", nod_deg_of_rot));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nod Min Limit", "DmxNodMinLimit", nod_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nod Max Limit", "DmxNodMaxLimit", nod_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Jaw Channel", "DmxJawChannel", jaw_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Jaw Min Limit", "DmxJawMinLimit", jaw_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Jaw Max Limit", "DmxJawMaxLimit", jaw_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye UD Channel", "DmxEyeUDChannel", eye_ud_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye UD Min Limit", "DmxEyeUDMinLimit", eye_ud_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye UD Max Limit", "DmxEyeUDMaxLimit", eye_ud_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye LR Channel", "DmxEyeLRChannel", eye_lr_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye LR Min Limit", "DmxEyeLRMinLimit", eye_lr_min_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye LR Max Limit", "DmxEyeLRMaxLimit", eye_lr_max_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Eye Brightness Channel", "DmxEyeBrtChannel", eye_brightness_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    AddColorTypeProperties(grid);
}

int DmxSkulltronix::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnPanTiltPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if ("DmxPanMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanMinLimit");
        ModelXml->AddAttribute("DmxPanMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXPanMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXPanMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXPanMinLimit");
        return 0;
    } else if ("DmxPanMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxPanMaxLimit");
        ModelXml->AddAttribute("DmxPanMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXPanMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXPanMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXPanMaxLimit");
        return 0;
    } else if ("DmxTiltMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltMinLimit");
        ModelXml->AddAttribute("DmxTiltMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMinLimit");
        return 0;
    } else if ("DmxTiltMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltMaxLimit");
        ModelXml->AddAttribute("DmxTiltMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXTiltMaxLimit");
        return 0;
    } else if ("DmxNodChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodChannel");
        ModelXml->AddAttribute("DmxNodChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXNodChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXNodChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::OnPropertyGridChange::DMXNodChannel");
        return 0;
    } else if ("DmxNodOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodOrient");
        ModelXml->AddAttribute("DmxNodOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXNodOrient");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXNodOrient");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXNodOrient");
        return 0;
    } else if ("DmxNodDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodDegOfRot");
        ModelXml->AddAttribute("DmxNodDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXNodDegOfRot");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXNodDegOfRot");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXNodDegOfRot");
        return 0;
    } else if ("DmxNodMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodMinLimit");
        ModelXml->AddAttribute("DmxNodMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXNodMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXNodMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXNodMinLimit");
        return 0;
    } else if ("DmxNodMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxNodMaxLimit");
        ModelXml->AddAttribute("DmxNodMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXNodMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXNodMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXNodMaxLimit");
        return 0;
    } else if ("DmxJawChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawChannel");
        ModelXml->AddAttribute("DmxJawChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXJawChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXJawChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::OnPropertyGridChange::DMXJawChannel");
        return 0;
    } else if ("DmxJawMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawMinLimit");
        ModelXml->AddAttribute("DmxJawMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXJawMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXJawMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXJawMinLimit");
        return 0;
    } else if ("DmxJawMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxJawMaxLimit");
        ModelXml->AddAttribute("DmxJawMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXJawMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXJawMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXJawMaxLimit");
        return 0;
    } else if ("DmxEyeBrtChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeBrtChannel");
        ModelXml->AddAttribute("DmxEyeBrtChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeBrtChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeBrtChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::OnPropertyGridChange::DMXEyeBrtChannel");
        return 0;
    } else if ("DmxEyeUDChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDChannel");
        ModelXml->AddAttribute("DmxEyeUDChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDChannel");
        return 0;
    } else if ("DmxEyeUDMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDMinLimit");
        ModelXml->AddAttribute("DmxEyeUDMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMinLimit");
        return 0;
    } else if ("DmxEyeUDMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeUDMaxLimit");
        ModelXml->AddAttribute("DmxEyeUDMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXEyeUDMaxLimit");
        return 0;
    } else if ("DmxEyeLRChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRChannel");
        ModelXml->AddAttribute("DmxEyeLRChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRChannel");
        return 0;
    } else if ("DmxEyeLRMinLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRMinLimit");
        ModelXml->AddAttribute("DmxEyeLRMinLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMinLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMinLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMinLimit");
        return 0;
    } else if ("DmxEyeLRMaxLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxEyeLRMaxLimit");
        ModelXml->AddAttribute("DmxEyeLRMaxLimit", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMaxLimit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMaxLimit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxSkulltronix::OnPropertyGridChange::DMXEyeLRMaxLimit");
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxSkulltronix::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxSkulltronix";
    screenLocation.SetRenderSize(1, 1);

    pan_channel = wxAtoi(ModelXml->GetAttribute("DmxPanChannel", "13"));
    pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", "90"));
    pan_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxPanDegOfRot", "180"));
    pan_slew_limit = wxAtof(ModelXml->GetAttribute("DmxPanSlewLimit", "0"));
    tilt_channel = wxAtoi(ModelXml->GetAttribute("DmxTiltChannel", "19"));
    tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", "315"));
    tilt_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxTiltDegOfRot", "90"));
    tilt_slew_limit = wxAtof(ModelXml->GetAttribute("DmxTiltSlewLimit", "0"));
    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "24"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "25"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "26"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
    tilt_min_limit = wxAtoi(ModelXml->GetAttribute("DmxTiltMinLimit", "442"));
    tilt_max_limit = wxAtoi(ModelXml->GetAttribute("DmxTiltMaxLimit", "836"));
    pan_min_limit = wxAtoi(ModelXml->GetAttribute("DmxPanMinLimit", "250"));
    pan_max_limit = wxAtoi(ModelXml->GetAttribute("DmxPanMaxLimit", "1250"));
    nod_channel = wxAtoi(ModelXml->GetAttribute("DmxNodChannel", "11"));
    nod_orient = wxAtoi(ModelXml->GetAttribute("DmxNodOrient", "331"));
    nod_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxNodDegOfRot", "58"));
    nod_min_limit = wxAtoi(ModelXml->GetAttribute("DmxNodMinLimit", "452"));
    nod_max_limit = wxAtoi(ModelXml->GetAttribute("DmxNodMaxLimit", "745"));
    jaw_channel = wxAtoi(ModelXml->GetAttribute("DmxJawChannel", "9"));
    jaw_min_limit = wxAtoi(ModelXml->GetAttribute("DmxJawMinLimit", "500"));
    jaw_max_limit = wxAtoi(ModelXml->GetAttribute("DmxJawMaxLimit", "750"));
    eye_brightness_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeBrtChannel", "23"));
    eye_ud_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeUDChannel", "15"));
    eye_ud_min_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeUDMinLimit", "575"));
    eye_ud_max_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeUDMaxLimit", "1000"));
    eye_lr_channel = wxAtoi(ModelXml->GetAttribute("DmxEyeLRChannel", "17"));
    eye_lr_min_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeLRMinLimit", "499"));
    eye_lr_max_limit = wxAtoi(ModelXml->GetAttribute("DmxEyeLRMaxLimit", "878"));

    SetNodeNames(",,,,,,,Power,Jaw,-Jaw Fine,Nod,-Nod Fine,Pan,-Pan Fine,Eye UD,-Eye UD Fine,Eye LR,-Eye LR Fine,Tilt,-Tilt Fine,-Torso,-Torso Fine,Eye Brightness,Eye Red,Eye Green,Eye Blue");
}

void DmxSkulltronix::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    if (!IsActive()) return;

    float pan_angle, pan_angle_raw, tilt_angle, nod_angle, jaw_pos, eye_x, eye_y;
    float jaw_range_of_motion = -4.0f;
    float eye_range_of_motion = 3.8f;
    int channel_value;
    size_t NodeCount=Nodes.size();
    bool beam_off = false;

    if( pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount )
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor eye_color(xlWHITE);
    xlColor marker_color(xlBLACK);
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
    if( red_channel > 0 && green_channel > 0 && blue_channel > 0 ) {
        xlColor proxy;
        Nodes[red_channel-1]->GetColor(proxy);
        eye_color.red = proxy.red;
        Nodes[green_channel-1]->GetColor(proxy);
        eye_color.green = proxy.red;
        Nodes[blue_channel-1]->GetColor(proxy);
        eye_color.blue = proxy.red;
    }
    if( (eye_color.red == 0 && eye_color.green == 0 && eye_color.blue == 0) || !active ) {
        eye_color = xlWHITE;
        beam_off = true;
    } else {
        ApplyTransparency(eye_color, trans, trans);
        marker_color = eye_color;
    }
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);
    ApplyTransparency(pnt_color, trans, trans);

    if( pan_channel > 0 ) {
        channel_value = GetChannelValue(pan_channel-1, true);
        pan_angle = ((channel_value - pan_min_limit) / (double)(pan_max_limit - pan_min_limit)) * pan_deg_of_rot + pan_orient;
    } else {
        pan_angle = 0.0f;
    }
    pan_angle_raw = pan_angle;
    if( tilt_channel > 0 ) {
        channel_value = GetChannelValue(tilt_channel-1, true);
        tilt_angle = (1.0 - ((channel_value - tilt_min_limit) / (double)(tilt_max_limit - tilt_min_limit))) * tilt_deg_of_rot + tilt_orient;
    } else {
        tilt_angle = 0.0f;
    }
    if( nod_channel > 0 ) {
        channel_value = GetChannelValue(nod_channel-1, true);
        nod_angle = (1.0 - ((channel_value - nod_min_limit) / (double)(nod_max_limit - nod_min_limit))) * nod_deg_of_rot + nod_orient;
    } else {
        nod_angle = 0.0f;
    }
    if( jaw_channel > 0 ) {
        channel_value = GetChannelValue(jaw_channel-1, true);
        jaw_pos = ((channel_value - jaw_min_limit) / (double)(jaw_max_limit - jaw_min_limit)) * jaw_range_of_motion - 0.5f;
    } else {
        jaw_pos = -0.5f;
    }
    if( eye_lr_channel > 0 ) {
        channel_value = GetChannelValue(eye_lr_channel-1, true);
        eye_x = (1.0 - ((channel_value - eye_lr_min_limit) / (double)(eye_lr_max_limit - eye_lr_min_limit))) * eye_range_of_motion - eye_range_of_motion/2.0;
    } else {
        eye_x = 0.0f;
    }
    if( eye_ud_channel > 0 ) {
        channel_value = GetChannelValue(eye_ud_channel-1, true);
        eye_y = ((channel_value - eye_ud_min_limit) / (double)(eye_ud_max_limit - eye_ud_min_limit)) * eye_range_of_motion - eye_range_of_motion/2.0;
    } else {
        eye_y = 0.0f;
    }

    if( !active ) {
        pan_angle = 0.5f * 180 + 90;
        tilt_angle = 0.5f * 90 + 315;
        nod_angle = 0.5f * 58 + 331;
        jaw_pos = -0.5f;
        eye_x = 0.5f * eye_range_of_motion - eye_range_of_motion/2.0;
        eye_y = 0.5f * eye_range_of_motion - eye_range_of_motion/2.0;
    }

    float sf = 12.0f;
    float scale = radius / sf;

    // Create Head
    dmxPoint3 p1(-7.5f, 13.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p2(7.5f, 13.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p3(13.2f, 6.0f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p8(-13.2f, 6.0f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p4(9, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7(-9, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5(6.3f, -16, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6(-6.3f, -16, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3 p9(0, 3.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p10(-2.5f, -1.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p11(2.5f, -1.7f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3 p14(0, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p12(-6, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p16(6, -6.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p13(-3, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p15(3, -11.4f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Back of Head
    dmxPoint3 p1b(-7.5f, 13.7f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p2b(7.5f, 13.7f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p3b(13.2f, 6.0f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p8b(-13.2f, 6.0f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p4b(9, -11.4f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7b(-9, -11.4f, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5b(6.3f, -16, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6b(-6.3f, -16, -3, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Lower Mouth
    dmxPoint3 p4m(9, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p7m(-9, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p5m(6.3f, -16+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p6m(-6.3f, -16+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p14m(0, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p12m(-6, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p16m(6, -6.5f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p13m(-3, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 p15m(3, -11.4f+jaw_pos, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

    // Create Eyes
    dmxPoint3 left_eye_socket(-5, 7.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 right_eye_socket(5, 7.5f, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 left_eye(-5+eye_x, 7.5f+eye_y, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3 right_eye(5+eye_x, 7.5f+eye_y, 0.0f, sx, sy, scale, pan_angle, tilt_angle, nod_angle);

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
    va.AddTrianglesCircle(left_eye_socket.x, left_eye_socket.y, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(right_eye_socket.x, right_eye_socket.y, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(left_eye.x, left_eye.y, scale*sf*0.10, eye_color, eye_color);
    va.AddTrianglesCircle(right_eye.x, right_eye.y, scale*sf*0.10, eye_color, eye_color);

    va.Finish(GL_TRIANGLES);
}

void DmxSkulltronix::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    if (!IsActive()) return;

    float pan_angle, pan_angle_raw, tilt_angle, nod_angle, jaw_pos, eye_x, eye_y;
    float jaw_range_of_motion = -4.0f;
    float eye_range_of_motion = 3.8f;
    int channel_value;
    size_t NodeCount = Nodes.size();
    bool beam_off = false;

    if (pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount)
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor eye_color(xlWHITE);
    xlColor marker_color(xlBLACK);
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
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy;
        Nodes[red_channel - 1]->GetColor(proxy);
        eye_color.red = proxy.red;
        Nodes[green_channel - 1]->GetColor(proxy);
        eye_color.green = proxy.red;
        Nodes[blue_channel - 1]->GetColor(proxy);
        eye_color.blue = proxy.red;
    }
    if ((eye_color.red == 0 && eye_color.green == 0 && eye_color.blue == 0) || !active) {
        eye_color = xlWHITE;
        beam_off = true;
    }
    else {
        ApplyTransparency(eye_color, trans, trans);
        marker_color = eye_color;
    }
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);
    ApplyTransparency(pnt_color, trans, trans);

    if (pan_channel > 0) {
        channel_value = GetChannelValue(pan_channel - 1, true);
        pan_angle = ((channel_value - pan_min_limit) / (double)(pan_max_limit - pan_min_limit)) * pan_deg_of_rot + pan_orient;
    }
    else {
        pan_angle = 0.0f;
    }
    pan_angle_raw = pan_angle;
    if (tilt_channel > 0) {
        channel_value = GetChannelValue(tilt_channel - 1, true);
        tilt_angle = (1.0 - ((channel_value - tilt_min_limit) / (double)(tilt_max_limit - tilt_min_limit))) * tilt_deg_of_rot + tilt_orient;
    }
    else {
        tilt_angle = 0.0f;
    }
    if (nod_channel > 0) {
        channel_value = GetChannelValue(nod_channel - 1, true);
        nod_angle = (1.0 - ((channel_value - nod_min_limit) / (double)(nod_max_limit - nod_min_limit))) * nod_deg_of_rot + nod_orient;
    }
    else {
        nod_angle = 0.0f;
    }
    if (jaw_channel > 0) {
        channel_value = GetChannelValue(jaw_channel - 1, true);
        jaw_pos = ((channel_value - jaw_min_limit) / (double)(jaw_max_limit - jaw_min_limit)) * jaw_range_of_motion - 0.5f;
    }
    else {
        jaw_pos = -0.5f;
    }
    if (eye_lr_channel > 0) {
        channel_value = GetChannelValue(eye_lr_channel - 1, true);
        eye_x = (1.0 - ((channel_value - eye_lr_min_limit) / (double)(eye_lr_max_limit - eye_lr_min_limit))) * eye_range_of_motion - eye_range_of_motion / 2.0;
    }
    else {
        eye_x = 0.0f;
    }
    if (eye_ud_channel > 0) {
        channel_value = GetChannelValue(eye_ud_channel - 1, true);
        eye_y = ((channel_value - eye_ud_min_limit) / (double)(eye_ud_max_limit - eye_ud_min_limit)) * eye_range_of_motion - eye_range_of_motion / 2.0;
    }
    else {
        eye_y = 0.0f;
    }

    if (!active) {
        pan_angle = 0.5f * 180 + 90;
        tilt_angle = 0.5f * 90 + 315;
        nod_angle = 0.5f * 58 + 331;
        jaw_pos = -0.5f;
        eye_x = 0.5f * eye_range_of_motion - eye_range_of_motion / 2.0;
        eye_y = 0.5f * eye_range_of_motion - eye_range_of_motion / 2.0;
    }

    float sf = 12.0f;
    float scale = radius / sf;

    // Create Head
    dmxPoint3d p1(-7.5f, 13.7f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p2(7.5f, 13.7f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p3(13.2f, 6.0f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p8(-13.2f, 6.0f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p4(9, -11.4f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p7(-9, -11.4f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p5(6.3f, -16, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p6(-6.3f, -16, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3d p9(0, 3.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p10(-2.5f, -1.7f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p11(2.5f, -1.7f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    dmxPoint3d p14(0, -6.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p12(-6, -6.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p16(6, -6.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p13(-3, -11.4f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p15(3, -11.4f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    // Create Back of Head
    dmxPoint3d p1b(-7.5f, 13.7f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p2b(7.5f, 13.7f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p3b(13.2f, 6.0f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p8b(-13.2f, 6.0f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p4b(9, -11.4f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p7b(-9, -11.4f, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p5b(6.3f, -16, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p6b(-6.3f, -16, -3, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    // Create Lower Mouth
    dmxPoint3d p4m(9, -11.4f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p7m(-9, -11.4f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p5m(6.3f, -16 + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p6m(-6.3f, -16 + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p14m(0, -6.5f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p12m(-6, -6.5f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p16m(6, -6.5f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p13m(-3, -11.4f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d p15m(3, -11.4f + jaw_pos, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    // Create Eyes
    dmxPoint3d left_eye_socket(-5, 7.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d right_eye_socket(5, 7.5f, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d left_eye(-5 + eye_x, 7.5f + eye_y, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);
    dmxPoint3d right_eye(5 + eye_x, 7.5f + eye_y, 0.0f, sx, sy, sz, scale, pan_angle, tilt_angle, nod_angle);

    // Draw Back of Head
    va.AddVertex(p1.x, p1.y, p1.z, base_color2);
    va.AddVertex(p1b.x, p1b.y, p1b.z, base_color2);
    va.AddVertex(p2.x, p2.y, p2.z, base_color2);
    va.AddVertex(p2b.x, p2b.y, p2b.z, base_color2);
    va.AddVertex(p1b.x, p1b.y, p1b.z, base_color2);
    va.AddVertex(p2.x, p2.y, p2.z, base_color2);

    va.AddVertex(p2.x, p2.y, p2.z, base_color);
    va.AddVertex(p2b.x, p2b.y, p2b.z, base_color);
    va.AddVertex(p3.x, p3.y, p3.z, base_color);
    va.AddVertex(p3b.x, p3b.y, p3b.z, base_color);
    va.AddVertex(p2b.x, p2b.y, p2b.z, base_color);
    va.AddVertex(p3.x, p3.y, p3.z, base_color);

    va.AddVertex(p3.x, p3.y, p3.z, base_color2);
    va.AddVertex(p3b.x, p3b.y, p3b.z, base_color2);
    va.AddVertex(p4.x, p4.y, p4.z, base_color2);
    va.AddVertex(p4b.x, p4b.y, p4b.z, base_color2);
    va.AddVertex(p3b.x, p3b.y, p3b.z, base_color2);
    va.AddVertex(p4.x, p4.y, p4.z, base_color2);

    va.AddVertex(p4.x, p4.y, p4.z, base_color);
    va.AddVertex(p4b.x, p4b.y, p4b.z, base_color);
    va.AddVertex(p5.x, p5.y, p5.z, base_color);
    va.AddVertex(p5b.x, p5b.y, p5b.z, base_color);
    va.AddVertex(p4b.x, p4b.y, p4b.z, base_color);
    va.AddVertex(p5.x, p5.y, p5.z, base_color);

    va.AddVertex(p5.x, p5.y, p5.z, base_color2);
    va.AddVertex(p5b.x, p5b.y, p5b.z, base_color2);
    va.AddVertex(p6.x, p6.y, p6.z, base_color2);
    va.AddVertex(p6b.x, p6b.y, p6b.z, base_color2);
    va.AddVertex(p5b.x, p5b.y, p5b.z, base_color2);
    va.AddVertex(p6.x, p6.y, p6.z, base_color2);

    va.AddVertex(p6.x, p6.y, p6.z, base_color);
    va.AddVertex(p6b.x, p6b.y, p6b.z, base_color);
    va.AddVertex(p7.x, p7.y, p7.z, base_color);
    va.AddVertex(p7b.x, p7b.y, p7b.z, base_color);
    va.AddVertex(p6b.x, p6b.y, p6b.z, base_color);
    va.AddVertex(p7.x, p7.y, p7.z, base_color);

    va.AddVertex(p7.x, p7.y, p7.z, base_color2);
    va.AddVertex(p7b.x, p7b.y, p7b.z, base_color2);
    va.AddVertex(p8.x, p8.y, p8.z, base_color2);
    va.AddVertex(p8b.x, p8b.y, p8b.z, base_color2);
    va.AddVertex(p7b.x, p7b.y, p7b.z, base_color2);
    va.AddVertex(p8.x, p8.y, p8.z, base_color2);

    va.AddVertex(p8.x, p8.y, p8.z, base_color);
    va.AddVertex(p8b.x, p8b.y, p8b.z, base_color);
    va.AddVertex(p1.x, p1.y, p1.z, base_color);
    va.AddVertex(p1b.x, p1b.y, p1b.z, base_color);
    va.AddVertex(p8b.x, p8b.y, p8b.z, base_color);
    va.AddVertex(p1.x, p1.y, p1.z, base_color);

    // Draw Front of Head
    va.AddVertex(p1.x, p1.y, p1.z, ccolor);
    va.AddVertex(p2.x, p2.y, p2.z, ccolor);
    va.AddVertex(p9.x, p9.y, p9.z, ccolor);

    va.AddVertex(p2.x, p2.y, p2.z, ccolor);
    va.AddVertex(p9.x, p9.y, p9.z, ccolor);
    va.AddVertex(p11.x, p11.y, p11.z, ccolor);

    va.AddVertex(p1.x, p1.y, p1.z, ccolor);
    va.AddVertex(p9.x, p9.y, p9.z, ccolor);
    va.AddVertex(p10.x, p10.y, p10.z, ccolor);

    va.AddVertex(p1.x, p1.y, p1.z, ccolor);
    va.AddVertex(p8.x, p8.y, p8.z, ccolor);
    va.AddVertex(p10.x, p10.y, p10.z, ccolor);

    va.AddVertex(p2.x, p2.y, p2.z, ccolor);
    va.AddVertex(p3.x, p3.y, p3.z, ccolor);
    va.AddVertex(p11.x, p11.y, p11.z, ccolor);

    va.AddVertex(p8.x, p8.y, p8.z, ccolor);
    va.AddVertex(p10.x, p10.y, p10.z, ccolor);
    va.AddVertex(p12.x, p12.y, p12.z, ccolor);

    va.AddVertex(p3.x, p3.y, p3.z, ccolor);
    va.AddVertex(p11.x, p11.y, p11.z, ccolor);
    va.AddVertex(p16.x, p16.y, p16.z, ccolor);

    va.AddVertex(p7.x, p7.y, p7.z, ccolor);
    va.AddVertex(p8.x, p8.y, p8.z, ccolor);
    va.AddVertex(p12.x, p12.y, p12.z, ccolor);

    va.AddVertex(p3.x, p3.y, p3.z, ccolor);
    va.AddVertex(p4.x, p4.y, p4.z, ccolor);
    va.AddVertex(p16.x, p16.y, p16.z, ccolor);

    va.AddVertex(p10.x, p10.y, p10.z, ccolor);
    va.AddVertex(p12.x, p12.y, p12.z, ccolor);
    va.AddVertex(p14.x, p14.y, p14.z, ccolor);

    va.AddVertex(p10.x, p10.y, p10.z, ccolor);
    va.AddVertex(p11.x, p11.y, p11.z, ccolor);
    va.AddVertex(p14.x, p14.y, p14.z, ccolor);

    va.AddVertex(p11.x, p11.y, p11.z, ccolor);
    va.AddVertex(p14.x, p14.y, p14.z, ccolor);
    va.AddVertex(p16.x, p16.y, p16.z, ccolor);

    va.AddVertex(p12.x, p12.y, p12.z, ccolor);
    va.AddVertex(p13.x, p13.y, p13.z, ccolor);
    va.AddVertex(p14.x, p14.y, p14.z, ccolor);

    va.AddVertex(p14.x, p14.y, p14.z, ccolor);
    va.AddVertex(p15.x, p15.y, p15.z, ccolor);
    va.AddVertex(p16.x, p16.y, p16.z, ccolor);

    // Draw Lower Mouth
    va.AddVertex(p4m.x, p4m.y, p4m.z, ccolor);
    va.AddVertex(p6m.x, p6m.y, p6m.z, ccolor);
    va.AddVertex(p7m.x, p7m.y, p7m.z, ccolor);

    va.AddVertex(p4m.x, p4m.y, p4m.z, ccolor);
    va.AddVertex(p5m.x, p5m.y, p5m.z, ccolor);
    va.AddVertex(p6m.x, p6m.y, p6m.z, ccolor);

    va.AddVertex(p7m.x, p7m.y, p7m.z, ccolor);
    va.AddVertex(p12m.x, p12m.y, p12m.z, ccolor);
    va.AddVertex(p13m.x, p13m.y, p13m.z, ccolor);

    va.AddVertex(p13m.x, p13m.y, p13m.z, ccolor);
    va.AddVertex(p14m.x, p14m.y, p14m.z, ccolor);
    va.AddVertex(p15m.x, p15m.y, p15m.z, ccolor);

    va.AddVertex(p4m.x, p4m.y, p4m.z, ccolor);
    va.AddVertex(p15m.x, p15m.y, p15m.z, ccolor);
    va.AddVertex(p16m.x, p16m.y, p16m.z, ccolor);

    // Draw Eyes
    va.AddTrianglesCircle(left_eye_socket.x, left_eye_socket.y, left_eye_socket.z, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(right_eye_socket.x, right_eye_socket.y, right_eye_socket.z, scale*sf*0.25, black, black);
    va.AddTrianglesCircle(left_eye.x, left_eye.y, left_eye.z, scale*sf*0.10, eye_color, eye_color);
    va.AddTrianglesCircle(right_eye.x, right_eye.y, right_eye.z, scale*sf*0.10, eye_color, eye_color);

    va.Finish(GL_TRIANGLES);
}

void DmxSkulltronix::ExportXlightsModel()
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

    wxString pdr = ModelXml->GetAttribute("DmxPanDegOfRot", "180");
    wxString tdr = ModelXml->GetAttribute("DmxTiltDegOfRot", "90");
    wxString s = ModelXml->GetAttribute("DmxStyle");
    wxString pc = ModelXml->GetAttribute("DmxPanChannel", "13");
    wxString po = ModelXml->GetAttribute("DmxPanOrient", "90");
    wxString tc = ModelXml->GetAttribute("DmxTiltChannel", "19");
    wxString to = ModelXml->GetAttribute("DmxTiltOrient", "315");
    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "24");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "25");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "26");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    wxString sc = ModelXml->GetAttribute("DmxShutterChannel", "0");
    wxString so = ModelXml->GetAttribute("DmxShutterOpen", "1");

    wxString tml = ModelXml->GetAttribute("DmxTiltMinLimit", "442");
    wxString tmxl = ModelXml->GetAttribute("DmxTiltMaxLimit", "836");
    wxString pml = ModelXml->GetAttribute("DmxPanMinLimit", "250");
    wxString pmxl = ModelXml->GetAttribute("DmxPanMaxLimit", "1250");
    wxString nc = ModelXml->GetAttribute("DmxNodChannel", "11");
    wxString no = ModelXml->GetAttribute("DmxNodOrient", "331");
    wxString ndr = ModelXml->GetAttribute("DmxNodDegOfRot", "58");
    wxString nml = ModelXml->GetAttribute("DmxNodMinLimit", "452");
    wxString nmxl = ModelXml->GetAttribute("DmxNodMaxLimit", "745");
    wxString jc = ModelXml->GetAttribute("DmxJawChannel", "9");
    wxString jml = ModelXml->GetAttribute("DmxJawMinLimit", "500");
    wxString jmxl = ModelXml->GetAttribute("DmxJawMaxLimit", "750");
    wxString eb = ModelXml->GetAttribute("DmxEyeBrtChannel", "23");
    wxString eudc = ModelXml->GetAttribute("DmxEyeUDChannel", "15");
    wxString eudml = ModelXml->GetAttribute("DmxEyeUDMinLimit", "575");
    wxString eudmxl = ModelXml->GetAttribute("DmxEyeUDMaxLimit", "1000");
    wxString elrc = ModelXml->GetAttribute("DmxEyeLRChannel", "17");
    wxString elml = ModelXml->GetAttribute("DmxEyeLRMinLimit", "499");
    wxString elrmxl = ModelXml->GetAttribute("DmxEyeLRMaxLimit", "878");

    wxString v = xlights_version_string;

    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("DmxPanDegOfRot=\"%s\" ", pdr));
    f.Write(wxString::Format("DmxTiltDegOfRot=\"%s\" ", tdr));
    f.Write(wxString::Format("DmxStyle=\"%s\" ", s));
    f.Write(wxString::Format("DmxPanChannel=\"%s\" ", pc));
    f.Write(wxString::Format("DmxPanOrient=\"%s\" ", po));
    f.Write(wxString::Format("DmxTiltChannel=\"%s\" ", tc));
    f.Write(wxString::Format("DmxTiltOrient=\"%s\" ", to));
    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
    f.Write(wxString::Format("DmxShutterChannel=\"%s\" ", sc));
    f.Write(wxString::Format("DmxShutterOpen=\"%s\" ", so));

    f.Write(wxString::Format("DmxTiltMinLimit=\"%s\" ", tml));
    f.Write(wxString::Format("DmxTiltMaxLimit=\"%s\" ", tmxl));
    f.Write(wxString::Format("DmxPanMinLimit=\"%s\" ", pml));
    f.Write(wxString::Format("DmxPanMaxLimit=\"%s\" ", pmxl));
    f.Write(wxString::Format("DmxNodChannel=\"%s\" ", nc));
    f.Write(wxString::Format("DmxNodOrient=\"%s\" ", no));
    f.Write(wxString::Format("DmxNodDegOfRot=\"%s\" ", ndr));
    f.Write(wxString::Format("DmxNodMinLimit=\"%s\" ", nml));
    f.Write(wxString::Format("DmxNodMaxLimit=\"%s\" ", nmxl));
    f.Write(wxString::Format("DmxJawChannel=\"%s\" ", jc));
    f.Write(wxString::Format("DmxJawMinLimit=\"%s\" ", jml));
    f.Write(wxString::Format("DmxJawMaxLimit=\"%s\" ", jmxl));
    f.Write(wxString::Format("DmxEyeBrtChannel=\"%s\" ", eb));
    f.Write(wxString::Format("DmxEyeUDChannel=\"%s\" ", eudc));
    f.Write(wxString::Format("DmxEyeUDMinLimit=\"%s\" ", eudml));
    f.Write(wxString::Format("DmxEyeUDMaxLimit=\"%s\" ", eudmxl));
    f.Write(wxString::Format("DmxEyeLRChannel=\"%s\" ", elrc));
    f.Write(wxString::Format("DmxEyeLRMinLimit=\"%s\" ", elml));
    f.Write(wxString::Format("DmxEyeLRMaxLimit=\"%s\" ", elrmxl));
    f.Write(" >\n");
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

void DmxSkulltronix::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
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

            wxString pdr = root->GetAttribute("DmxPanDegOfRot");
            wxString tdr = root->GetAttribute("DmxTiltDegOfRot");
            wxString s = root->GetAttribute("DmxStyle");
            wxString pc = root->GetAttribute("DmxPanChannel");
            wxString po = root->GetAttribute("DmxPanOrient");
            wxString psl = root->GetAttribute("DmxPanSlewLimit");
            wxString tc = root->GetAttribute("DmxTiltChannel");
            wxString to = root->GetAttribute("DmxTiltOrient");
            wxString tsl = root->GetAttribute("DmxTiltSlewLimit");
            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");
            wxString sc = root->GetAttribute("DmxShutterChannel");
            wxString so = root->GetAttribute("DmxShutterOpen");
            wxString bl = root->GetAttribute("DmxBeamLimit");

            wxString tml = root->GetAttribute("DmxTiltMinLimit");
            wxString tmxl = root->GetAttribute("DmxTiltMaxLimit");
            wxString pml = root->GetAttribute("DmxPanMinLimit");
            wxString pmxl = root->GetAttribute("DmxPanMaxLimit");
            wxString nc = root->GetAttribute("DmxNodChannel");
            wxString no = root->GetAttribute("DmxNodOrient");
            wxString ndr = root->GetAttribute("DmxNodDegOfRot");
            wxString nml = root->GetAttribute("DmxNodMinLimit");
            wxString nmxl = root->GetAttribute("DmxNodMaxLimit");
            wxString jc = root->GetAttribute("DmxJawChannel");
            wxString jml = root->GetAttribute("DmxJawMinLimit");
            wxString jmxl = root->GetAttribute("DmxJawMaxLimit");
            wxString eb = root->GetAttribute("DmxEyeBrtChannel");
            wxString eudc = root->GetAttribute("DmxEyeUDChannel");
            wxString eudml = root->GetAttribute("DmxEyeUDMinLimit");
            wxString eudmxl = root->GetAttribute("DmxEyeUDMaxLimit");
            wxString elrc = root->GetAttribute("DmxEyeLRChannel");
            wxString elml = root->GetAttribute("DmxEyeLRMinLimit");
            wxString elrmxl = root->GetAttribute("DmxEyeLRMaxLimit");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("DmxPanDegOfRot", pdr);
            SetProperty("DmxTiltDegOfRot", tdr);
            SetProperty("DmxStyle", s);
            SetProperty("DmxPanChannel", pc);
            SetProperty("DmxPanOrient", po);
            SetProperty("DmxPanSlewLimit", psl);
            SetProperty("DmxTiltChannel", tc);
            SetProperty("DmxTiltOrient", to);
            SetProperty("DmxTiltSlewLimit", tsl);
            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);
            SetProperty("DmxShutterChannel", sc);
            SetProperty("DmxShutterOpen", so);
            SetProperty("DmxBeamLimit", bl);

            SetProperty("DmxTiltMinLimit", tml);
            SetProperty("DmxTiltMaxLimit", tmxl);
            SetProperty("DmxPanMinLimit", pml);
            SetProperty("DmxPanMaxLimit", pmxl);
            SetProperty("DmxNodChannel", nc);
            SetProperty("DmxNodOrient", no);
            SetProperty("DmxNodDegOfRot", ndr);
            SetProperty("DmxNodMinLimit", nml);
            SetProperty("DmxNodMaxLimit", nmxl);
            SetProperty("DmxJawChannel", jc);
            SetProperty("DmxJawMinLimit", jml);
            SetProperty("DmxJawMaxLimit", jmxl);
            SetProperty("DmxEyeBrtChannel", eb);
            SetProperty("DmxEyeUDChannel", eudc);
            SetProperty("DmxEyeUDMinLimit", eudml);
            SetProperty("DmxEyeUDMaxLimit", eudmxl);
            SetProperty("DmxEyeLRChannel", elrc);
            SetProperty("DmxEyeLRMinLimit", elml);
            SetProperty("DmxEyeLRMaxLimit", elrmxl);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxSkulltronix::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxSkulltronix::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxSkulltronix model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxSkulltronix model file.");
    }
}
