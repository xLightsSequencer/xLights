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

#include "DmxColorAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxColorAbilityWheel.h"
#include "DmxPresetAbility.h"
#include "DmxMovingHeadAdv.h"
#include "Mesh.h"
#include "Servo.h"
#include "ServoConfigDialog.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

enum MOTION_LINK {
    MOTION_LINK_MESH1,
    MOTION_LINK_MESH2
};

#define ToRadians(x) ((double)x * PI / (double)180.0)

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3(float x_, float y_, float z_, float pan_angle_, float tilt_angle_ = 0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 model_position = rotationMatrixPan * rotationMatrixTilt * position;
        x = model_position.x;
        y = model_position.y;
        z = model_position.z;
    }
};

DmxMovingHeadAdv::DmxMovingHeadAdv(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), beam_length(4)
{
    beam_width = GetDefaultBeamWidth();
    SetFromXml(node, zeroBased);
}

DmxMovingHeadAdv::~DmxMovingHeadAdv()
{
    Clear();
}

void DmxMovingHeadAdv::Clear() {
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    servos.clear();
    for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    motion_meshs.clear();
    for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
        if (*it != nullptr) {
            delete* it;
        }
    }
    static_meshs.clear();
}

void DmxMovingHeadAdv::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    DmxModel::AddTypeProperties(grid, outputManager);

    auto p = grid->Append(new wxBoolProperty("16 Bit", "Bits16", _16bit));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Show Pivot Axes", "PivotAxes", show_pivot));
    p->SetAttribute("UseCheckbox", true);

    for (const auto& it : servos) {
        it->AddTypeProperties(grid);
    }

    AddPanTiltTypeProperties(grid);
    grid->Collapse("PanTiltProperties");

    if (nullptr != color_ability) {
        grid->Append(new wxPropertyCategory("Color Properties", "DmxColorAbility"));
        int selected = DMX_COLOR_TYPES.Index(color_ability->GetTypeName());
        grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES, selected));
        color_ability->AddColorTypeProperties(grid);
        grid->Collapse("DmxColorAbility");
    }
    AddShutterTypeProperties(grid);
    grid->Collapse("DmxShutterProperties");

    p = grid->Append(new wxPropertyCategory("Beam Properties", "BeamProperties"));

    p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Display Width", "DmxBeamWidth", beam_width));
    p->SetAttribute("Min", 0.01);
    p->SetAttribute("Max", 150);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Beam Orientation", "DmxBeamOrient", beam_orient));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Y Offset", "DmxBeamYOffset", beam_y_offset));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 50);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    grid->Collapse("BeamProperties");

    for (const auto& it : static_meshs) {
        it->AddTypeProperties(grid);
    }

    for (const auto& it : motion_meshs) {
        it->AddTypeProperties(grid);
    }

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

void DmxMovingHeadAdv::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    auto p = grid->GetPropertyByName("DmxPanChannel");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("DmxPanDegOfRot");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("DmxTiltChannel");
    if (p != nullptr) {
        p->Hide(true);
    }
    p = grid->GetPropertyByName("DmxTiltDegOfRot");
    if (p != nullptr) {
        p->Hide(true);
    }

    DmxModel::DisableUnusedProperties(grid);
}

int DmxMovingHeadAdv::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    std::string name = event.GetPropertyName().ToStdString();

    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnPanTiltPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnShutterPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if ("DmxChannelCount" == event.GetPropertyName()) {
        int channels = (int)event.GetPropertyValue().GetLong();
        int min_channels = num_servos * (_16bit ? 2 : 1);
        if (channels < min_channels) {
            wxPGProperty* p = grid->GetPropertyByName("DmxChannelCount");
            if (p != nullptr) {
                p->SetValue(min_channels);
            }
            std::string msg = wxString::Format("You have %d servos at %d bits so you need %d channels minimum.", num_servos, _16bit ? 16 : 8, min_channels);
            wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
            return 0;
        }
    }

    if ("PivotAxes" == name) {
        if (event.GetValue().GetBool()) {
            show_pivot = true;
        } else {
            show_pivot = false;
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::PivotAxes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::OnPropertyGridChange::PivotAxes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::PivotAxes");
        return 0;
    }

    if (event.GetPropertyName() == "Bits16") {
        ModelXml->DeleteAttribute("Bits16");
        if (event.GetValue().GetBool()) {
            _16bit = true;
            ModelXml->AddAttribute("Bits16", "1");
        } else {
            _16bit = false;
            ModelXml->AddAttribute("Bits16", "0");
        }

        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] != nullptr) {
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }

        int min_channels = num_servos * (_16bit ? 2 : 1);
        if (parm1 < min_channels) {
            UpdateChannelCount(min_channels, true);
        }
        update_node_names = true;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::Bits16");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::OnPropertyGridChange::Bits16");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::Bits16");
        return 0;
    }
    else if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamLength");
        return 0;
    }
    else if ("DmxBeamWidth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamWidth");
        ModelXml->AddAttribute("DmxBeamWidth", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DMXBeamWidth");
        return 0;
    }
    else if ("DmxBeamOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamOrient");
        ModelXml->AddAttribute("DmxBeamOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        return 0;
    }
    else if ("DmxBeamYOffset" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamYOffset");
        ModelXml->AddAttribute("DmxBeamYOffset", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        return 0;
    }
    else if ("DmxColorType" == event.GetPropertyName()) {
        int color_type = event.GetPropertyValue().GetInteger();

        ModelXml->DeleteAttribute("DmxColorType");
        ModelXml->AddAttribute("DmxColorType", wxString::Format("%d", color_type));

        if (color_type == 0) {
            color_ability = std::make_unique<DmxColorAbilityRGB>(ModelXml);
        } else  if (color_type == 1) {
            color_ability = std::make_unique<DmxColorAbilityWheel>(ModelXml);
        }
        else {
            color_ability = std::make_unique<DmxColorAbilityCMY>(ModelXml);
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHeadAdv::OnPropertyGridChange::DmxColorType");
        return 0;
    }

    for (const auto& it : servos) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    for (const auto& it : static_meshs) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    for (const auto& it : motion_meshs) {
        if (it->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
            return 0;
        }
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxMovingHeadAdv::InitModel()
{
    _16bit = wxAtoi(ModelXml->GetAttribute("Bits16", "1"));

    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (parm1 < min_channels) {
        UpdateChannelCount(min_channels, false);
        std::string msg = wxString::Format("Channel count increased to %d to accommodate %d servos at %d bits.", min_channels, num_servos, _16bit ? 16 : 8);
        wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
    }

    DmxModel::InitModel();
    DisplayAs = "DmxMovingHeadAdv";

    pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", "0"));
    pan_slew_limit = wxAtof(ModelXml->GetAttribute("DmxPanSlewLimit", "0"));
    tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", "0"));
    tilt_slew_limit = wxAtof(ModelXml->GetAttribute("DmxTiltSlewLimit", "0"));
    shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
    shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    shutter_on_value = wxAtoi(ModelXml->GetAttribute("DmxShutterOnValue", "0"));
    
    int color_type = wxAtoi(ModelXml->GetAttribute("DmxColorType", "0"));
    if (color_type == 0) {
        color_ability = std::make_unique<DmxColorAbilityRGB>(ModelXml);
    }else if (color_type == 1) {
        color_ability = std::make_unique<DmxColorAbilityWheel>(ModelXml);
    }
    else {
        color_ability = std::make_unique<DmxColorAbilityCMY>(ModelXml);
    }

    beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "4.0"));
    beam_width = GetDefaultBeamWidth();
    if (ModelXml->HasAttribute("DmxBeamWidth")) {
        beam_width = wxAtof(ModelXml->GetAttribute("DmxBeamWidth"));
    }
    beam_orient = wxAtoi(ModelXml->GetAttribute("DmxBeamOrient", "0"));
    beam_y_offset = wxAtof(ModelXml->GetAttribute("DmxBeamYOffset", "0"));

    // clear links
    for (int i = 0; i < SUPPORTED_SERVOS; ++i) {
        servo_links[i] = -1;
        mesh_links[i] = -1;
    }

    // clear any extras
    while (servos.size() > num_servos) {
        Servo* ptr = servos.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        servos.pop_back();
    }
    while (static_meshs.size() > num_static) {
        Mesh* ptr = static_meshs.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        static_meshs.pop_back();
    }
    while (motion_meshs.size() > num_motion) {
        Mesh* ptr = motion_meshs.back();
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
        motion_meshs.pop_back();
    }

    // resize vector arrays
    if (static_meshs.size() < num_static) {
        static_meshs.resize(num_static);
    }
    if (motion_meshs.size() < num_motion) {
        motion_meshs.resize(num_motion);
    }
    if (servos.size() < num_servos) {
        servos.resize(num_servos);
    }

    wxXmlNode* n = ModelXml->GetChildren();

    while (n != nullptr) {
        std::string name = n->GetName();

        if ("PanServo" == name) {
             if (servos[0] == nullptr) {
                 servos[0] = new Servo(n, name, false);
             }
        } else if ("TiltServo" == name) {
            if (servos[1] == nullptr) {
                servos[1] = new Servo(n, name, false);
            }
        } else if ("BaseMesh" == name) {
            if (static_meshs[0] == nullptr) {
                static_meshs[0] = new Mesh(n, name);
            }
        } else if ("YokeMesh" == name) {
            if (motion_meshs[0] == nullptr) {
                motion_meshs[0] = new Mesh(n, name);
            }
        } else if ("HeadMesh" == name) {
            if (motion_meshs[1] == nullptr) {
                motion_meshs[1] = new Mesh(n, name);
            }
        }
        n = n->GetNext();
    }

    // create pan servo
    if (servos[0] == nullptr) {
        std::string new_name = "PanServo";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        servos[0] = new Servo(new_node, new_name, true);
        servos[0]->SetChannel(1, this);
    }

    // create tilt servo
    if (servos[1] == nullptr) {
        std::string new_name = "TiltServo";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        servos[1] = new Servo(new_node, new_name, true);
        servos[1]->SetChannel(_16bit ? 3 : 2, this);
    }

    // create base mesh
    if (static_meshs[0] == nullptr) {
        std::string new_name = "BaseMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        static_meshs[0] = new Mesh(new_node, new_name);
    }

    // create yoke mesh
    if (motion_meshs[0] == nullptr) {
        std::string new_name = "YokeMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        motion_meshs[0] = new Mesh(new_node, new_name);
    }

    // create head mesh
    if (motion_meshs[1] == nullptr) {
        std::string new_name = "HeadMesh";
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
        ModelXml->AddChild(new_node);
        motion_meshs[1] = new Mesh(new_node, new_name);
    }

    // set linkages
    servo_links[0] = -1;
    servo_links[1] = -1;
    mesh_links[0] = -1;
    mesh_links[1] = 0;

    brightness = wxAtoi(ModelXml->GetAttribute("Brightness", "100"));

    for (const auto& it : servos) {
        it->Init(this);
        it->Set16Bit(_16bit);
    }

    bool last_exists = false;
    for (const auto& it : static_meshs) {
        it->Init(this, !last_exists);
        last_exists = it->HasObjFile();
    }

    last_exists = num_static > 0 ? static_meshs[0]->HasObjFile() : false;
    for (const auto& it : motion_meshs) {
        it->Init(this, !last_exists);
        last_exists = it->HasObjFile();
    }

    // renumber servo changed if number of bits changed
    if (update_bits) {
        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] != nullptr) {
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }
        update_bits = false;
    }

    // create node names
    std::string names = "";
    if (_16bit) {
        names = "Pan,-Pan,Tilt,-Tilt";
    } else {
        names = "Pan,Tilt";
    }
    SetNodeNames(names, update_node_names);
    update_node_names = false;
}

void DmxMovingHeadAdv::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                      xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is_3d,
                                      const xlColor* c, bool allowSelected, bool wiring,
                                      bool highlightFirst, int highlightpixel,
                                      float* boundingBox)
{
    if (!IsActive())
        return;

    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(Nodes);
    if (boundingBox) {
        boundingBox[0] = -0.5;
        boundingBox[1] = -0.5;
        boundingBox[2] = -0.5;
        boundingBox[3] = 0.5;
        boundingBox[4] = 0.5;
        boundingBox[5] = 0.5;
    }
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, !allowSelected, c);
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted);
        } else {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
        }
    }
}

void DmxMovingHeadAdv::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool mustEnd = false;
    xlGraphicsContext* ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }
    if (ctx) {
        int w, h;
        preview->GetSize(&w, &h);
        float scaleX = float(w) * 0.95f / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95f / GetModelScreenLocation().RenderHt;

        float aspect = screenLocation.GetScaleX();
        aspect /= screenLocation.GetScaleY();
        if (scaleY < scaleX) {
            scaleX = scaleY * aspect;
        } else {
            scaleY = scaleX / aspect;
        }
        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
            ctx->TranslateViewMatrix(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                                     h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
            ctx->TranslateViewMatrix(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                                     h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0f);
        });
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), true, nullptr);
        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

std::list<std::string> DmxMovingHeadAdv::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (min_channels > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s requires more channels %d than have been allocated to it %d.", GetName(), min_channels, nodeCount));
    }
    int i = 1;
    for (const auto& it : servos) {
        if (it->GetChannel() > nodeCount) {
            res.push_back(wxString::Format("    ERR: Model %s servo %d is assigned to channel %d but the model only has %d channels.", GetName(), i, it->GetChannel(), nodeCount));
        }
        i++;
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxMovingHeadAdv::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool active, const xlColor* c)
{
    // crash protection
    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (min_channels > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    for (const auto& it : servos) {
        if (it->GetChannel() > Nodes.size()) {
            DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
            return;
        }
    }
    size_t NodeCount = Nodes.size();
    if (!color_ability->IsValidModelSettings(this) ||
        !preset_ability->IsValidModelSettings(this) ||
        shutter_channel > NodeCount)
    {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    // retrieve the model state
    float old_pan_angle = 0.0f;
    float old_tilt_angle = 0.0f;
    uint32_t old_ms = 0;

    PanTiltState &st = panTiltStates[preview->GetName().ToStdString()];
    if (active) {
        old_ms = st.ms;
        old_pan_angle = st.pan_angle;
        old_tilt_angle = st.tilt_angle;
    }

    float pan_angle = 0;
    if (servos[0]->GetChannel() > 0 && active) {
        pan_angle = servos[0]->GetPosition(GetChannelValue(servos[0]->GetChannel() - 1, servos[0]->Is16Bit()));
    }
    pan_angle += pan_orient;

    float tilt_angle = 0;
    if (servos[1]->GetChannel() > 0 && active) {
        tilt_angle = servos[1]->GetPosition(GetChannelValue(servos[1]->GetChannel() - 1, servos[1]->Is16Bit()));
    }
    tilt_angle += tilt_orient;

    uint32_t ms = preview->getCurrentFrameTime();
    uint32_t time_delta = 0;
    if (ms > old_ms) {
        time_delta = ms - old_ms;
    } else if (ms == old_ms && active) {
        pan_angle = old_pan_angle;
        tilt_angle = old_tilt_angle;
    }
    if (time_delta > 500) {
        // more than 1/2 second off, assume a jump of some sort
        time_delta = 0;
    }

    if (time_delta != 0 && active) {
        // pan slew limiting
        if (pan_slew_limit > 0.0f) {
            float slew_limit = pan_slew_limit * (float)time_delta / 1000.0f;
            float pan_delta = pan_angle - old_pan_angle;
            if (std::abs(pan_delta) > slew_limit) {
                if (pan_delta < 0) {
                    slew_limit *= -1.0f;
                }
                pan_angle = old_pan_angle + slew_limit;
            }
        }
    }

    float pan_angle_raw = pan_angle;

    if (time_delta != 0 && active) {
        // tilt slew limiting
        if (tilt_slew_limit > 0.0f) {
            float slew_limit = tilt_slew_limit * (float)time_delta / 1000.0f;
            float tilt_delta = tilt_angle - old_tilt_angle;
            if (std::abs(tilt_delta) > slew_limit) {
                if (tilt_delta < 0) {
                    slew_limit *= -1.0f;
                }
                tilt_angle = old_tilt_angle + slew_limit;
            }
        }
    }

    // Determine if we need to flip the beam
    int tilt_pos = (int)(RenderBuffer::cos(ToRadians(tilt_angle)));
    if (tilt_pos < 0) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
    }

    // save the model state
    st.ms = ms;
    st.pan_angle = pan_angle_raw;
    st.tilt_angle = tilt_angle;

    float servo_pos[2] = { pan_angle, tilt_angle };
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 servo_matrix[SUPPORTED_SERVOS];
    glm::mat4 motion_matrix[SUPPORTED_SERVOS];

    // Draw Static Meshs
    for (int i = 0; i < num_static; ++i) {
        static_meshs[i]->Draw(this, preview, sprogram, tprogram, Identity, Identity, i < num_motion ? !motion_meshs[i]->GetExists(this, ctx) : false, 0, 0, 0, false, false);
    }

    // Get servo positions and fill motion matrices
    for (int i = 0; i < servos.size(); ++i) {
        if (servos[i]->GetChannel() > 0 && active) {
            //servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1, servos[i]->Is16Bit()));
            if (servos[i]->IsTranslate()) {
                glm::vec3 scale = GetBaseObjectScreenLocation().GetScaleMatrix();
                servo_pos[i] /= scale.x;
            }
        }
        servo_matrix[i] = Identity;
        motion_matrix[i] = Identity;
        servos[i]->FillMotionMatrix(servo_pos[i], servo_matrix[i]);
    }

    // Determine motion mesh linkages
    for (int i = 0; i < num_motion; ++i) {
        int link = mesh_links[i];
        int nesting = num_motion;
        std::vector<int> link_list;
        // if mesh does not link to himself
        if (link != i) {
            // iterate through pushing mesh links into a list
            while ((link != -1) && (nesting > 0)) {
                link_list.push_back(link);
                link = mesh_links[link];
                nesting--; // prevents circular loops from hanging things up
            }
            // multiply motion matrix based on list in reverse order
            while (!link_list.empty()) {
                link = link_list.back();
                link_list.pop_back();
                motion_matrix[i] = motion_matrix[i] * servo_matrix[link];
            }
        }
    }

    // add motion based on servo mapping
    for (int i = 0; i < servos.size(); ++i) {
        // see if servo links to his own mesh
        if (servo_links[i] == -1) {
            motion_matrix[i] = motion_matrix[i] * servo_matrix[i];
        }
        // check if any other servos map to this mesh
        for (int j = 0; j < servos.size(); ++j) {
            if (j != i) {
                if (servo_links[j] == i) {
                    motion_matrix[i] = motion_matrix[i] * servo_matrix[j];
                }
            }
        }
    }

    // Draw Motion Meshs
    for (int i = 0; i < num_motion; ++i) {
        motion_meshs[i]->Draw(this, preview, sprogram, tprogram, Identity, motion_matrix[i], i < num_static ? !static_meshs[i]->GetExists(this, ctx) : false,
                              servos[i]->GetPivotOffsetX(), servos[i]->GetPivotOffsetY(), servos[i]->GetPivotOffsetZ(), servos[i]->IsRotate() && show_pivot, !active);
    }

    // Everything below here is for drawing the light beam

    float beam_length_displayed = beam_length;
    
    float scw = screenLocation.GetRenderWi() * screenLocation.GetScaleX();
    float sch = screenLocation.GetRenderHt() * screenLocation.GetScaleY();
    float scd = screenLocation.GetRenderDp() * screenLocation.GetScaleZ();
    float sbl = std::max(scw, std::max(sch, scd));
    beam_length_displayed *= sbl;
    float beam_offset = beam_y_offset * sch;

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= NodeCount && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        } else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    xlColor color;
    if (c != nullptr) {
        color = *c;
    }
    int trans = color == xlBLACK ? blackTransparency : transparency;
    xlColor beam_color = color_ability->GetBeamColor(Nodes);
    if (!active) {
        beam_color = xlWHITE;
    }
    ApplyTransparency(beam_color, trans, trans);

    while (pan_angle_raw > 360.0f)
        pan_angle_raw -= 360.0f;
    pan_angle_raw = 360.0f - pan_angle_raw + beam_orient;

    //Draw3DBeam(tvac, beam_color, beam_length_displayed, , tilt_angle, shutter_open);
    auto vac = tprogram->getAccumulator();
    int start = vac->getCount();
    Draw3DBeam(vac, beam_color, beam_length_displayed, -pan_angle_raw, -tilt_angle, shutter_open, beam_offset);
    int end = vac->getCount();
    tprogram->addStep([=](xlGraphicsContext *ctx) {
        ctx->drawTriangles(vac, start, end - start);
    });
}

void DmxMovingHeadAdv::Draw3DBeam(xlVertexColorAccumulator* tvac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset)
{
    xlColor beam_color_end(beam_color);
    beam_color_end.alpha = 0;

    bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

    float combined_angle = tilt_angle;
    if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
        if (shutter_open) {
            float angle1 = float(beam_width) / 2.0f;
            if (angle1 < 0.0f) {
                angle1 += 360.0f;
            }
            float x1 = (RenderBuffer::cos(ToRadians(angle1)) * beam_length_displayed);
            float y1 = (RenderBuffer::sin(ToRadians(angle1)) * beam_length_displayed);

            dmxPoint3 p1(x1, -y1, -y1, pan_angle_raw, combined_angle);
            dmxPoint3 p2(x1, -y1, y1, pan_angle_raw, combined_angle);
            dmxPoint3 p3(x1, y1, -y1, pan_angle_raw, combined_angle);
            dmxPoint3 p4(x1, y1, y1, pan_angle_raw, combined_angle);
            dmxPoint3 p0(0, 0, 0, pan_angle_raw, combined_angle);
            p0.y += beam_offset;
            p1.y += beam_offset;
            p2.y += beam_offset;
            p3.y += beam_offset;
            p4.y += beam_offset;

            if (!facing_right) {
                tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            } else {
                tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            }

            tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
            tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
            tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);

            tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
            tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
            tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);

            if (facing_right) {
                tvac->AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                tvac->AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            } else {
                tvac->AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                tvac->AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                tvac->AddVertex(p0.x, p0.y, p0.z, beam_color);
            }
        }
    }
}

void DmxMovingHeadAdv::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<DmxMovingHeadAdv \n");

    ExportBaseParameters(f);

    f.Write(wxString::Format("NumServos=\"%i\" ", num_servos));
    f.Write(wxString::Format("NumStatic=\"%i\" ", num_static));
    f.Write(wxString::Format("NumMotion=\"%i\" ", num_motion));

    wxString bits = ModelXml->GetAttribute("Bits16");
    f.Write(wxString::Format("Bits16=\"%s\" ", bits));

    // servo linkages
    for (int i = 0; i < num_servos; ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Servo" + num + "Linkage";
        if( ModelXml->HasAttribute(this_link)) {
            std::string link = ModelXml->GetAttribute(this_link, "");
            f.Write(wxString::Format("%s=\"%s\" ", this_link,link));
        }
    }

    // mesh linkages
    for (int i = 0; i < num_servos; ++i) {
        std::string num = std::to_string(i + 1);
        std::string this_link = "Mesh" + num + "Linkage";
        if( ModelXml->HasAttribute(this_link)) {
            std::string link = ModelXml->GetAttribute(this_link, "");
            f.Write(wxString::Format("%s=\"%s\" ", this_link,link));
        }
    }

    f.Write(" >\n");

    wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();

    for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Serialise(ModelXml, f, show_dir);
    }

    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    f.Write("</DmxMovingHeadAdv>");
    f.Close();
}

void DmxMovingHeadAdv::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "DmxMovingHeadAdv" || root->GetName() == "dmxservo3axis") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");
        wxString bits = root->GetAttribute("Bits16", "1");
        wxString dct = root->GetAttribute("DmxColorType", "0");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model
        SetProperty("DmxColorType", dct);

        int color_type = wxAtoi(dct);
        if (color_type == 0) {
            color_ability = std::make_unique<DmxColorAbilityRGB>(ModelXml);
        } else if (color_type == 1) {
            color_ability = std::make_unique<DmxColorAbilityWheel>(ModelXml);
        }
        else {
            color_ability = std::make_unique<DmxColorAbilityCMY>(ModelXml);
        }
        color_ability->ImportParameters(root, this);

        wxXmlNode* n = root->GetChildren();
        while (n != nullptr) {
            std::string name = n->GetName();

            if ("StaticMesh" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "StaticMesh1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            } else if ("MotionMesh" == name) { // convert original name that had no number
                // copy attributes to new name
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "MotionMesh1");
                root->AddChild(new_node);
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    new_node->AddAttribute(a->GetName(), a->GetValue());
                }
            }
            n = n->GetNext();
        }

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);
        SetProperty("Bits16", bits);

        num_static = wxAtoi(root->GetAttribute("NumStatic", "1"));
        num_motion = wxAtoi(root->GetAttribute("NumMotion", "1"));
        SetProperty("NumServos", std::to_string(num_servos));
        SetProperty("NumStatic", std::to_string(num_static));
        SetProperty("NumMotion", std::to_string(num_motion));

        // resize vector arrays
        if (static_meshs.size() < num_static) {
            static_meshs.resize(num_static);
        }
        if (motion_meshs.size() < num_motion) {
            motion_meshs.resize(num_motion);
        }
        if (servos.size() < num_servos) {
            servos.resize(num_servos);
        }

        // create any missing servos
        for (int i = 0; i < num_servos; ++i) {
            if (servos[i] == nullptr) {
                std::string new_name = "Servo" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                servos[i] = new Servo(new_node, new_name, true);
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1, this);
            }
        }

        // create any missing static meshes
        for (int i = 0; i < num_static; ++i) {
            if (static_meshs[i] == nullptr) {
                std::string new_name = "StaticMesh" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                static_meshs[i] = new Mesh(new_node, new_name);
            }
        }

        // create any missing motion meshes
        for (int i = 0; i < num_motion; ++i) {
            if (motion_meshs[i] == nullptr) {
                std::string new_name = "MotionMesh" + std::to_string(i + 1);
                wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, new_name);
                ModelXml->AddChild(new_node);
                motion_meshs[i] = new Mesh(new_node, new_name);
            }
        }

        // servo linkages
        for (int i = 0; i < num_servos; ++i) {
            std::string num = std::to_string(i + 1);
            std::string this_link = "Servo" + num + "Linkage";
            if( root->HasAttribute(this_link)) {
                wxString link = root->GetAttribute(this_link);
                SetProperty(this_link, link);
            }
        }

        // mesh linkages
        for (int i = 0; i < num_servos; ++i) {
            std::string num = std::to_string(i + 1);
            std::string this_link = "Mesh" + num + "Linkage";
            if( root->HasAttribute(this_link)) {
                wxString link = root->GetAttribute(this_link);
                SetProperty(this_link, link);
            }
        }

        wxString show_dir = GetModelManager().GetXLightsFrame()->GetShowDirectory();
        for (auto it = static_meshs.begin(); it != static_meshs.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }
        for (auto it = motion_meshs.begin(); it != motion_meshs.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }
        for (auto it = servos.begin(); it != servos.end(); ++it) {
            (*it)->Serialise(root, ModelXml, show_dir);
        }

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHeadAdv::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxMovingHeadAdv model file.");
    }
}
