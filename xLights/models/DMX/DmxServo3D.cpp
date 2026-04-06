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

#include "DmxServo3D.h"
#include "Mesh.h"
#include "Servo.h"
#include "../../graphics/IModelPreview.h"
#include "../ModelManager.h"
#include "../../graphics/xlGraphicsContext.h"
#include "../../graphics/xlGraphicsAccumulators.h"
#include "xLightsVersion.h"
#include "../../render/UICallbacks.h"
#include "UtilFunctions.h"
#include "../../XmlSerializer/XmlNodeKeys.h"

enum MOTION_LINK {
    MOTION_LINK_MESH1,
    MOTION_LINK_MESH2,
    MOTION_LINK_MESH3,
    MOTION_LINK_MESH4,
    MOTION_LINK_MESH5,
    MOTION_LINK_MESH6,
    MOTION_LINK_MESH7,
    MOTION_LINK_MESH8,
    MOTION_LINK_MESH9,
    MOTION_LINK_MESH10,
    MOTION_LINK_MESH11,
    MOTION_LINK_MESH12,
    MOTION_LINK_MESH13,
    MOTION_LINK_MESH14,
    MOTION_LINK_MESH15,
    MOTION_LINK_MESH16,
    MOTION_LINK_MESH17,
    MOTION_LINK_MESH18,
    MOTION_LINK_MESH19,
    MOTION_LINK_MESH20,
    MOTION_LINK_MESH21,
    MOTION_LINK_MESH22,
    MOTION_LINK_MESH23,
    MOTION_LINK_MESH24
};

DmxServo3d::DmxServo3d(const ModelManager &manager)
    : DmxModel(manager)
{
    DisplayAs = DisplayAsType::DmxServo3d;
    for (int i = 0; i < SUPPORTED_SERVOS; ++i) {
        servo_links[i] = -1;
        mesh_links[i] = -1;
    }
}

DmxServo3d::~DmxServo3d()
{
}

void DmxServo3d::SetNumServos(int val)
{
    num_servos = val;
    if ((int)servos.size() < num_servos) {
        servos.resize(num_servos);
    }
}

void DmxServo3d::SetNumStatic(int val)
{
    num_static = val;
    if ((int)static_meshs.size() < num_static) {
        static_meshs.resize(num_static);
    }
}

void DmxServo3d::SetNumMotion(int val)
{
    num_motion = val;
    if ((int)motion_meshs.size() < num_motion) {
        motion_meshs.resize(num_motion);
    }
}

Mesh* DmxServo3d::CreateStaticMesh(const std::string& name, int idx)
{
    static_meshs[idx] = std::make_unique<Mesh>(name);
    return static_meshs[idx].get();
}

Mesh* DmxServo3d::CreateMotionMesh(const std::string& name, int idx)
{
    motion_meshs[idx] = std::make_unique<Mesh>(name);
    return motion_meshs[idx].get();
}

Servo* DmxServo3d::CreateServo(const std::string& name, int idx)
{
    servos[idx] = std::make_unique<Servo>(name, false);
    return servos[idx].get();
}

void DmxServo3d::InitModel()
{
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (_dmxChannelCount < min_channels) {
        UpdateChannelCount(min_channels, false);
        std::string msg = std::format("Channel count increased to {} to accommodate {} servos at {} bits.", min_channels, num_servos, _16bit ? 16 : 8);
        if (auto* ui = GetModelManager().GetUICallbacks()) {
            ui->ShowMessage(msg, "Minimum Channel Violation");
        }
    }

    DmxModel::InitModel();

    // resize vector arrays
    if ((int)servos.size() != num_servos) {
        servos.resize(num_servos);
    }
    if ((int)static_meshs.size() != num_static) {
        static_meshs.resize(num_static);
    }
    if ((int)motion_meshs.size() != num_motion) {
        motion_meshs.resize(num_motion);
    }

    // create any missing servos
    for (int i = 0; i < num_servos; ++i) {
        if (servos[i] == nullptr) {
            std::string new_name = "Servo" + std::to_string(i + 1);
            servos[i] = std::make_unique<Servo>(new_name, true);
            servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1);
        }
    }

    // create any missing static meshes
    for (int i = 0; i < num_static; ++i) {
        if (static_meshs[i] == nullptr) {
            std::string new_name = "StaticMesh" + std::to_string(i + 1);
            static_meshs[i] = std::make_unique<Mesh>(new_name);
        }
    }

    // create any missing motion meshes
    for (int i = 0; i < num_motion; ++i) {
        if (motion_meshs[i] == nullptr) {
            std::string new_name = "MotionMesh" + std::to_string(i + 1);
            motion_meshs[i] = std::make_unique<Mesh>(new_name);
        }
    }

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
                servos[i]->SetChannel(_16bit ? i * 2 + 1 : i + 1);
            }
        }
        update_bits = false;
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

void DmxServo3d::DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext* ctx,
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
    sprogram->addStep([is_3d, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([is_3d, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, !allowSelected);
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

void DmxServo3d::DisplayEffectOnWindow(IModelPreview* preview, double pointSize)
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
        w = preview->getWidth(); h = preview->getHeight();
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
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), true);
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

std::list<std::string> DmxServo3d::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = num_servos * (_16bit ? 2 : 1);

    if (min_channels > nodeCount) {
        res.push_back(std::format("    ERR: Model {} requires more channels {} than have been allocated to it {}.", GetName(), min_channels, nodeCount));
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

void DmxServo3d::DrawModel(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool active)
{
    // crash protection
    int min_channels = num_servos * (_16bit ? 2 : 1);
    if (min_channels > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    for (const auto& it : servos) {
        if (it->GetChannel() > (int)Nodes.size()) {
            DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
            return;
        }
    }

    float servo_pos[SUPPORTED_SERVOS] = { 0.0f };
    glm::mat4 Identity = glm::mat4(1.0f);
    glm::mat4 servo_matrix[SUPPORTED_SERVOS];
    glm::mat4 motion_matrix[SUPPORTED_SERVOS];

    // Draw Static Meshs
    for (int i = 0; i < num_static; ++i) {
        static_meshs[i]->Draw(this, preview, sprogram, tprogram, Identity, Identity, i < num_motion ? !motion_meshs[i]->GetExists(this, ctx) : false, 0, 0, 0, false, false);
    }

    // Get servo positions and fill motion matrices
    for (int i = 0; i < (int)servos.size(); ++i) {
        if (servos[i]->GetChannel() > 0 && active) {
            servo_pos[i] = servos[i]->GetPosition(GetChannelValue(servos[i]->GetChannel() - 1, servos[i]->Is16Bit()));
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
    for (int i = 0; i < (int)servos.size(); ++i) {
        // see if servo links to his own mesh
        if (servo_links[i] == -1) {
            motion_matrix[i] = motion_matrix[i] * servo_matrix[i];
        }
        // check if any other servos map to this mesh
        for (int j = 0; j < (int)servos.size(); ++j) {
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
}

void DmxServo3d::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    DmxModel::GetPWMOutputs(channels);
    for (auto &s : servos) {
        s->GetPWMOutputs(channels);
    }
}
