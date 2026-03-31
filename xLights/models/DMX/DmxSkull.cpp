/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <filesystem>
#include <format>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxSkull.h"
#include "Mesh.h"
#include "Servo.h"
#include "DmxColorAbilityRGB.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "UtilFunctions.h"
#include "../../XmlSerializer/XmlNodeKeys.h"

DmxSkull::DmxSkull(const ModelManager& manager) :
    DmxModel(manager)
{
    DisplayAs = DisplayAsType::DmxSkull;
    color_ability = std::make_unique<DmxColorAbilityRGB>();

    obj_path = GetResourcesDir() + "/meshobjects/Skull/";

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

    screenLocation.CreateWithDepth(true);
}

DmxSkull::~DmxSkull()
{
}

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3(float x_, float y_, float z_, int cx_, int cy_, float scale_, float pan_angle_, float tilt_angle_, float nod_angle_ = 0.0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = glm::radians(pan_angle_);
        float tilt_angle = glm::radians(tilt_angle_);
        float nod_angle = glm::radians(nod_angle_);

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

class dmxPoint3d
{
public:
    float x;
    float y;
    float z;

    dmxPoint3d(float x_, float y_, float z_, float cx_, float cy_, float cz_, float scale_, float pan_angle_, float tilt_angle_, float nod_angle_ = 0.0) :
        x(x_), y(y_), z(z_)
    {
        float pan_angle = glm::radians(pan_angle_);
        float tilt_angle = glm::radians(tilt_angle_);
        float nod_angle = glm::radians(nod_angle_);

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



std::unique_ptr<Servo> DmxSkull::CreateServo(const std::string& name, int type, const std::string& style)
{
    std::unique_ptr<Servo> servo = std::make_unique<Servo>(name, false);
    servo->SetMinLimit(default_min_limit[type]);
    servo->SetMaxLimit(default_max_limit[type]);
    servo->SetRangeOfMotion(default_range_of_motion[type]);
    servo->SetStyle(style);
    servo->SetChannel(default_channels[type]);
    servo->Init(this);
    servo->Set16Bit(_16bit);
    return servo;
}

Servo* DmxSkull::CreateServo(const std::string& name)
{
    if ("JawServo" == name) {
        jaw_servo = CreateServo("JawServo", JAW, "Rotate X");
        return jaw_servo.get();
    }
    else if ("PanServo" == name) {
        pan_servo = CreateServo("PanServo", PAN, "Rotate Y");
        return pan_servo.get();
    }
    else if ("TiltServo" == name) {
        tilt_servo = CreateServo("TiltServo", TILT, "Rotate Z");
        return tilt_servo.get();
    }
    else if ("NodServo" == name) {
        nod_servo = CreateServo("NodServo", NOD, "Rotate X");
        return nod_servo.get();
    }
    else if ("EyeUpDownServo" == name) {
        eye_ud_servo = CreateServo("EyeUpDownServo", EYE_UD, "Rotate X");
        return eye_ud_servo.get();
    }
    else if ("EyeLeftRightServo" == name) {
        eye_lr_servo = CreateServo("EyeLeftRightServo", EYE_LR, "Rotate Y");
        return eye_lr_servo.get();
    }
    return nullptr;
}

std::unique_ptr<Mesh> DmxSkull::CreateMesh(const std::string& name, const std::string& objfile)
{
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(name);
    mesh->SetObjFile(FixFile("", objfile));
    mesh->Init(this, false);
    return mesh;
}

Mesh* DmxSkull::CreateMesh(const std::string& name, bool add_path)
{
    if ("HeadMesh" == name) {
        head_mesh = CreateMesh("HeadMesh", (add_path ? obj_path : "") + "SkullHead.obj");
        return head_mesh.get();
    }
    else if ("JawMesh" == name) {
        jaw_mesh = CreateMesh("JawMesh", (add_path ? obj_path : "") +  "SkullJaw.obj");
        return jaw_mesh.get();
    }
    else if ("EyeMeshL" == name) {
        eye_l_mesh = CreateMesh("EyeMeshL", (add_path ? obj_path : "") +  "Eyeball.obj");
        return eye_l_mesh.get();
    }
    else if ("EyeMeshR" == name) {
        eye_r_mesh = CreateMesh("EyeMeshR", (add_path ? obj_path : "") +  "Eyeball.obj");
        return eye_r_mesh.get();
    }
    return nullptr;
}

void DmxSkull::InitModel()
{
    DmxModel::InitModel();
    screenLocation.SetRenderSize(1, 1, 1);

    SetNodeNames(default_node_names);

    // create any missing servos
    if (has_jaw && jaw_servo == nullptr)
        CreateServo("JawServo");
    if (has_pan && pan_servo == nullptr)
        CreateServo("PanServo");
    if (has_tilt && tilt_servo == nullptr)
        CreateServo("TiltServo");
    if (has_nod && nod_servo == nullptr)
        CreateServo("NodServo");
    if (has_eye_ud && eye_ud_servo == nullptr)
        CreateServo("EyeUpDownServo");
    if (has_eye_lr && eye_lr_servo == nullptr)
        CreateServo("EyeLeftRightServo");

    // create any missing meshes
    if (head_mesh == nullptr)
        CreateMesh("HeadMesh", true);
    if (jaw_mesh == nullptr)
        CreateMesh("JawMesh", true);
    if (eye_l_mesh == nullptr)
        CreateMesh("EyeMeshL", true);
    if (eye_r_mesh == nullptr)
        CreateMesh("EyeMeshR", true);
    head_mesh->SetHalfHeight(); // obj file is shifted up so its twice as tall as it need to be

    head_mesh->SetMeshOnly(mesh_only);
    jaw_mesh->SetMeshOnly(mesh_only);
    eye_l_mesh->SetMeshOnly(mesh_only);
    eye_r_mesh->SetMeshOnly(mesh_only);

    if (setup_skulltronix) {
        SetupSkulltronix();
    }
}

float DmxSkull::GetServoPos(Servo* _servo, bool active)
{
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

void DmxSkull::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                    xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is_3d,
                                    const xlColor* c, bool allowSelected, bool wiring,
                                    bool highlightFirst, int highlightpixel,
                                    float* boundingBox)
{
    if (!IsActive())
        return;

    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(1, 1, 1);
    if (boundingBox) {
        boundingBox[0] = -0.5;
        boundingBox[1] = -0.5;
        boundingBox[2] = -0.5;
        boundingBox[3] = 0.5;
        boundingBox[4] = 0.5;
        boundingBox[5] = 0.5;
    }
    sprogram->addStep([this, is_3d](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0, 1.0, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
        ctx->Scale(0.7f, 0.7f, 0.7f);
        ctx->Translate(0, -0.7f, is_3d ? 0 : 0.5f);
    });
    tprogram->addStep([this, is_3d](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
        ctx->Scale(0.7f, 0.7f, 0.7f);
        ctx->Translate(0, -0.7f, is_3d ? 0 : 0.5f);
    });
    DrawModel(preview, ctx, sprogram, tprogram, is_3d, !allowSelected, c);
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected() || (Highlighted() && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted(), IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxSkull::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
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
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0f);
            ctx->ScaleViewMatrix(0.5f, 0.5f, 0.5f);
            ctx->TranslateViewMatrix(0, -0.7f, 0.5f);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.0001f);
            ctx->TranslateViewMatrix(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                                     h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->ScaleViewMatrix(scaleX, scaleY, 1.0f);
            ctx->ScaleViewMatrix(0.5f, 0.5f, 1.0f);
            ctx->TranslateViewMatrix(0, -0.7f, 0.5f);
        });
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), false, true, nullptr);
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

std::list<std::string> DmxSkull::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();

    if (has_color && eye_brightness_channel > nodeCount) {
        res.push_back(std::format("    ERR: Model {} eye brightness channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), eye_brightness_channel, nodeCount));
    }

    if (has_jaw && jaw_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} jaw servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), jaw_servo->GetChannel(), nodeCount));
    }
    if (has_pan && pan_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} pan servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), pan_servo->GetChannel(), nodeCount));
    }
    if (has_tilt && tilt_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} tilt servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), tilt_servo->GetChannel(), nodeCount));
    }
    if (has_nod && nod_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} nod servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), nod_servo->GetChannel(), nodeCount));
    }
    if (has_eye_ud && eye_ud_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} eye up/down servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), eye_ud_servo->GetChannel(), nodeCount));
    }
    if (has_eye_lr && eye_lr_servo->GetChannel() > nodeCount) {
        res.push_back(std::format("    ERR: Model {} eye left/right servo channel refers to a channel ({}) not present on the model which only has {} channels.", GetName(), eye_lr_servo->GetChannel(), nodeCount));
    }

    res.splice(res.end(), DmxModel::CheckModelSettings());
    return res;
}

void DmxSkull::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is3d, bool active, const xlColor* c)
{
    size_t NodeCount = Nodes.size();

    // crash protection
    if (has_color && (eye_brightness_channel > (int)NodeCount ||
        !color_ability->IsValidModelSettings(this)))
    {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    if (has_jaw && jaw_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (has_pan && pan_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (has_tilt && tilt_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (has_nod && nod_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (has_eye_ud && eye_ud_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (has_eye_lr && eye_lr_servo->GetChannel() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor eye_color(xlWHITE);
    xlColor black(xlBLACK);
    xlColor base_color(200, 200, 200);
    xlColor base_color2(150, 150, 150);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    xlColor color_angle;
    if( has_color && nullptr != color_ability ) {
        color_ability->GetColor(eye_color, transparency, blackTransparency, !active, c, Nodes);
    } else {
        eye_color = xlBLACK;
        Model::ApplyTransparency(eye_color, blackTransparency, blackTransparency);
    }

    int trans = color == xlBLACK ? blackTransparency : transparency;
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);

    // Get servo positions
    float pan_pos, tilt_pos, nod_pos, jaw_pos, eye_x_pos, eye_y_pos;
    pan_pos = GetServoPos(pan_servo.get(), active && has_pan) + (active ? pan_orient : 0.0f);
    tilt_pos = GetServoPos(tilt_servo.get(), active && has_tilt) + (active ? tilt_orient : 0.0f);
    nod_pos = GetServoPos(nod_servo.get(), active && has_nod) + (active ? nod_orient : 0.0f);
    jaw_pos = GetServoPos(jaw_servo.get(), active && has_jaw) + (active ? jaw_orient : 0.0f);
    eye_x_pos = GetServoPos(eye_lr_servo.get(), active && has_eye_lr) + (active ? eye_lr_orient : 0.0f);
    eye_y_pos = GetServoPos(eye_ud_servo.get(), active && has_eye_ud) + (active ? eye_ud_orient : 0.0f);

    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 jaw_matrix = Identity;
    glm::mat4 pan_matrix = Identity;
    glm::mat4 tilt_matrix = Identity;
    glm::mat4 nod_matrix = Identity;
    glm::mat4 eye_x_matrix = Identity;
    glm::mat4 eye_y_matrix = Identity;

    // Fill motion matrices
    if (has_jaw)
        jaw_servo->FillMotionMatrix(jaw_pos, jaw_matrix);
    if (has_pan)
        pan_servo->FillMotionMatrix(pan_pos, pan_matrix);
    if (has_tilt)
        tilt_servo->FillMotionMatrix(tilt_pos, tilt_matrix);
    if (has_nod)
        nod_servo->FillMotionMatrix(nod_pos, nod_matrix);
    if (has_eye_lr)
        eye_lr_servo->FillMotionMatrix(eye_x_pos, eye_x_matrix);
    if (has_eye_ud)
        eye_ud_servo->FillMotionMatrix(eye_y_pos, eye_y_matrix);

    // Adjust scaling to render size of 1
    float jaw_pivot_y = 3.3f;
    float jaw_pivot_z = 0.4f;
    if (head_mesh->GetExists(this, ctx)) {
        float w = head_mesh->GetWidth();
        float scale = 1.0f / w;
        head_mesh->SetRenderScaling(scale);
        jaw_mesh->SetRenderScaling(scale);
        eye_l_mesh->SetRenderScaling(scale);
        eye_r_mesh->SetRenderScaling(scale);
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
    eye_x_matrix = eye_x_matrix * eye_y_matrix;
    eye_l_mesh->SetColor(eye_color, "EyeColor");
    eye_r_mesh->SetColor(eye_color, "EyeColor");
    head_mesh->Draw(this, preview, sprogram, tprogram, Identity, head_matrix, false, 0, 0, 0, false, false);
    jaw_mesh->Draw(this, preview, sprogram, tprogram, head_matrix, jaw_matrix, false, 0, jaw_pivot_y, jaw_pivot_z, true, false);
    eye_l_mesh->Draw(this, preview, sprogram, tprogram, head_matrix, eye_x_matrix, false, 0, 0, 0, false, false);
    eye_r_mesh->Draw(this, preview, sprogram, tprogram, head_matrix, eye_x_matrix, false, 0, 0, 0, false, false);
}

void DmxSkull::SetupServo(Servo* _servo, int channel, float min_limit, float max_limit, float range_of_motion, bool _16bit)
{
    if (_servo != nullptr) {
        _servo->SetMinLimit(min_limit);
        _servo->SetMaxLimit(max_limit);
        _servo->SetRangeOfMotion(range_of_motion);
        _servo->Set16Bit(_16bit);
        _servo->SetChannel(channel);
    }
}

void DmxSkull::SetupSkulltronix()
{
    SetupServo(jaw_servo.get(), 9, 500, 750, -20, true);
    SetupServo(pan_servo.get(), 13, 400, 1100, 180, true);
    SetupServo(tilt_servo.get(), 19, 442, 836, -40, true);
    SetupServo(nod_servo.get(), 11, 452, 745, 58, true);
    SetupServo(eye_ud_servo.get(), 15, 575, 1000, 70, true);
    SetupServo(eye_lr_servo.get(), 17, 499, 878, -70, true);

    eye_brightness_channel = 23;
    jaw_orient = 0;
    pan_orient = 90;
    tilt_orient = -45;
    nod_orient = 29;
    eye_ud_orient = 35;
    eye_lr_orient = -35;

    SetNodeNames(",,,,,,,Power,Jaw,-Jaw Fine,Nod,-Nod Fine,Pan,-Pan Fine,Eye UD,-Eye UD Fine,Eye LR,-Eye LR Fine,Tilt,-Tilt Fine,-Torso,-Torso Fine,Eye Brightness,Eye Red,Eye Green,Eye Blue", true);
    setup_skulltronix = false;
}

void DmxSkull::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    if (has_jaw) {
        jaw_servo->GetPWMOutputs(channels);
    }
    if (has_pan) {
        pan_servo->GetPWMOutputs(channels);
    }
    if (has_tilt) {
        tilt_servo->GetPWMOutputs(channels);
    }
    if (has_nod) {
        nod_servo->GetPWMOutputs(channels);
    }
    if (has_eye_ud) {
        eye_ud_servo->GetPWMOutputs(channels);
    }
    if (has_eye_lr) {
        eye_lr_servo->GetPWMOutputs(channels);
    }
    if (has_color) {
        color_ability->GetPWMOutputs(channels);
        if (eye_brightness_channel > 0) {
            channels[eye_brightness_channel] = PWMOutput(eye_brightness_channel, PWMOutput::Type::LED, 1, "Eye Brightness");
        }
    }
}

std::vector<std::string> DmxSkull::GenerateNodeNames() const {
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != pan_servo->GetChannel() && pan_servo->GetChannel() < (int)names.size()) {
        names[pan_servo->GetChannel() - 1] = "Pan";
        if (pan_servo->Is16Bit()) {
            names[pan_servo->GetChannel()] = "Pan Fine";
        }
    }
    if (0 != tilt_servo->GetChannel() && tilt_servo->GetChannel() < (int)names.size()) {
        names[tilt_servo->GetChannel() - 1] = "Tilt";
        if (tilt_servo->Is16Bit()) {
            names[tilt_servo->GetChannel()] = "Tilt Fine";
        }
    }
    if (0 != nod_servo->GetChannel() && nod_servo->GetChannel() < (int)names.size()) {
        names[nod_servo->GetChannel() - 1] = "Nod";
        if (nod_servo->Is16Bit()) {
            names[nod_servo->GetChannel()] = "Nod Fine";
        }
    }
    if (0 != jaw_servo->GetChannel() && jaw_servo->GetChannel() < (int)names.size()) {
        names[jaw_servo->GetChannel() - 1] = "Jaw";
        if (jaw_servo->Is16Bit()) {
            names[jaw_servo->GetChannel()] = "Jaw Fine";
        }
    }
    if (0 != eye_ud_servo->GetChannel() && eye_ud_servo->GetChannel() < (int)names.size()) {
        names[eye_ud_servo->GetChannel() - 1] = "Eye U-D";
        if (eye_ud_servo->Is16Bit()) {
            names[eye_ud_servo->GetChannel()] = "Eye U-D Fine";
        }
    }
    if (0 != eye_lr_servo->GetChannel() && eye_lr_servo->GetChannel() < (int)names.size()) {
        names[eye_lr_servo->GetChannel() - 1] = "Eye L-R";
        if (eye_lr_servo->Is16Bit()) {
            names[eye_lr_servo->GetChannel()] = "Eye L-R Fine";
        }
    }
    if (nullptr != color_ability) {
        color_ability->SetNodeNames(names, "Eye ");
    }
    if (eye_brightness_channel > 0) {
        names[eye_brightness_channel - 1] = "Eye Bright";
    }
    return names;
}

