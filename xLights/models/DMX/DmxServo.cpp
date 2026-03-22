/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <format>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxServo.h"
#include "DmxImage.h"
#include "Servo.h"

#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include "../../XmlSerializer/XmlNodeKeys.h"
#include <log4cpp/Category.hh>
#include "../../ModelPreview.h"

static const int SUPPORTED_SERVOS = 24;

DmxServo::DmxServo(const ModelManager &manager)
    : DmxModel(manager)
{
    DisplayAs = DisplayAsType::DmxServo;
}

DmxServo::~DmxServo()
{
}

void DmxServo::SetNumServos(int val)
{
    num_servos = val;
    if (servos.size() < num_servos) {
        servos.resize(num_servos);
    }
    if (static_images.size() < num_servos) {
        static_images.resize(num_servos);
    }
    if (motion_images.size() < num_servos) {
        motion_images.resize(num_servos);
    }
}

DmxImage* DmxServo::CreateStaticImage(const std::string& name, int idx)
{
    static_images[idx] = std::make_unique<DmxImage>(name);
    return static_images[idx].get();
}

DmxImage* DmxServo::CreateMotionImage(const std::string& name, int idx)
{
    motion_images[idx] = std::make_unique<DmxImage>(name);
    return motion_images[idx].get();
}

Servo* DmxServo::CreateServo(const std::string& name, int idx)
{
    servos[idx] = std::make_unique<Servo>(name, true);
    return servos[idx].get();
}

void DmxServo::InitModel()
{
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (_dmxChannelCount < min_channels) {
        UpdateChannelCount(min_channels, false);
        std::string msg = std::format("Channel count increased to {} to accommodate {} servos at {} bits.", min_channels, num_servos, _16bit ? 16 : 8);
        wxMessageBox(msg, "Minimum Channel Violation", wxOK | wxCENTER);
    }

    DmxModel::InitModel();

    screenLocation.SetRenderSize(1, 1, 1);

    // resize vector arrays
    if (static_images.size() != num_servos) {
        static_images.resize(num_servos);
    }
    if (motion_images.size() != num_servos) {
        motion_images.resize(num_servos);
    }
    if (servos.size() != num_servos) {
        servos.resize(num_servos);
    }

    // create any missing servos
    for (int i = 0; i < num_servos; ++i) {
        if (static_images[i] == nullptr) {
            std::string new_name = "StaticImage" + std::to_string(i + 1);
            static_images[i] = std::make_unique<DmxImage>(new_name);
        }
        if (motion_images[i] == nullptr) {
            std::string new_name = "MotionImage" + std::to_string(i + 1);
            motion_images[i] = std::make_unique<DmxImage>(new_name);
            motion_images[i]->SetOffsetZ(i * 0.01f); // offset on creation so its not hidden
        }
        if (servos[i] == nullptr) {
            std::string new_name = "Servo" + std::to_string(i + 1);
            servos[i] = std::make_unique<Servo>(new_name, true);
            servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1);
        }
    }

    for (auto it = servos.begin(); it != servos.end(); ++it) {
        (*it)->Init(this);
        (*it)->Set16Bit(_16bit);
    }

    for (auto it = static_images.begin(); it != static_images.end(); ++it) {
        (*it)->Init(this);
    }

    for (auto it = motion_images.begin(); it != motion_images.end(); ++it) {
        (*it)->Init(this);
    }

    // create node names
    std::string names = "";
    int index = 1;
    for (auto it = servos.begin(); it != servos.end(); ++it) {
        if (!names.empty()) {
            names += ",";
        }
        if (_16bit) {
            names += "Servo" + std::to_string(index) + ",-Servo" + std::to_string(index);
        } else {
            names += "Servo" + std::to_string(index);
        }
        index++;
    }
    SetNodeNames(names, update_node_names);
    update_node_names = false;
}

void DmxServo::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                    xlGraphicsProgram* solidProgram, xlGraphicsProgram* transparentProgram, bool is_3d,
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

    xlGraphicsProgram* program = transparentProgram;
    program->addStep([is_3d, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->Scale(1.0, 1.0, 0.0);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, program, c, !allowSelected);
    program->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected()|| (Highlighted() && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted(), IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxServo::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
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
        float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;

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
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->Scale(scaleX, scaleY, 1.0);
        });
        DrawModel(preview, ctx, preview->getCurrentTransparentProgram(), nullptr, true);
        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

std::list<std::string> DmxServo::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (min_channels > nodeCount) {
        res.push_back(std::format("    ERR: Model {} requires more channels {} than have been allocated to it {}.", GetName(), min_channels, nodeCount));
    }
    if (motion_images.size() < num_servos) {
        res.push_back(std::format("    ERR: Model {} Insufficient images defined {} when {} required.", GetName(), motion_images.size(), num_servos));
    }
    int i = 1;
    for (const auto& it : servos) {
        if (it->GetChannel() > nodeCount) {
            res.push_back(std::format("    ERR: Model {} servo {} is assigned to channel {} but the model only has {} channels.", GetName(), i, it->GetChannel(), nodeCount));
        }
        i++;
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxServo::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* program, const xlColor* c, bool active)
{
    // crash protection
    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (min_channels > Nodes.size()) {
        DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (motion_images.size() < num_servos) {
        DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
        return;
    }
    for (const auto& it : servos) {
        if (it->GetChannel() > Nodes.size()) {
            DmxModel::DrawInvalid(program, &(GetModelScreenLocation()), false, false);
            return;
        }
    }

    float servo_pos[SUPPORTED_SERVOS] = { 0.0f };

    for (int i = 0; i < servos.size(); ++i) {
        //need to layer the images via slight Z offsets or they blend together and don't appear.
        glm::mat4 motionMatrix = glm::mat4(1.0f);
        motionMatrix = glm::translate(motionMatrix, glm::vec3(0, 0, 0.2 * float(i)));
        static_images[i]->Draw(this, preview, program, motionMatrix, transparency, brightness, !motion_images[i]->GetExists(), 0, 0, false, false);
        if (servos[i]->GetChannel() > 0 && active) {
            servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1, servos[i]->Is16Bit()));
        }
        servos[i]->FillMotionMatrix(servo_pos[i], motionMatrix);
        motionMatrix = glm::translate(motionMatrix, glm::vec3(0, 0, 0.2 * float(i) + 0.1));
        motion_images[i]->Draw(this, preview, program, motionMatrix,
                               transparency, brightness, !static_images[i]->GetExists(),
                               servos[i]->GetPivotOffsetX(), servos[i]->GetPivotOffsetY(),
                               servos[i]->IsRotate(), !active);
    }
}

void DmxServo::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    DmxModel::GetPWMOutputs(channels);
    for (auto &s : servos) {
        s->GetPWMOutputs(channels);
    }
}
