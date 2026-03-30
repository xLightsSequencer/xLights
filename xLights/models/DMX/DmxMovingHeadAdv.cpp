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

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxMovingHeadAdv.h"
#include "DmxBeamAbility.h"
#include "DmxColorAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxColorAbilityWheel.h"
#include "DmxDimmerAbility.h"
#include "DmxMotor.h"
#include "DmxPresetAbility.h"
#include "DmxShutterAbility.h"
#include "Mesh.h"
#include "MovingHeads/MhFeature.h"
#include "../../ModelPreview.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include "../../XmlSerializer/XmlNodeKeys.h"

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
        float pan_angle = glm::radians(pan_angle_);
        float tilt_angle = glm::radians(tilt_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 model_position = position * rotationMatrixTilt * rotationMatrixPan;
        x = model_position.x;
        y = model_position.y;
        z = model_position.z;
    }
};

DmxMovingHeadAdv::DmxMovingHeadAdv(const ModelManager &manager) :
    DmxMovingHeadComm(manager)
{
    DisplayAs = DisplayAsType::DmxMovingHeadAdv;
    obj_path = GetResourcesDir() + "/meshobjects/SimpleMovingHead/";
    color_ability = std::make_unique<DmxColorAbilityRGB>();
    dimmer_ability = std::make_unique<DmxDimmerAbility>();
    shutter_ability = std::make_unique<DmxShutterAbility>();
    beam_ability = std::make_unique<DmxBeamAbility>();
    beam_ability->SetSupportsOrient(true);
    beam_ability->SetSupportsYOffset(true);
    beam_ability->SetDefaultBeamLength(4.0);
    beam_ability->SetDefaultBeamWidth(4.0);
    beam_ability->SetDefaultBeamYOffset(17.0);
    beam_ability->SetBeamLength(4.0);
    beam_ability->SetBeamWidth(4.0);
    beam_ability->SetBeamYOffset(17.0);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetRedChannel(5);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetGreenChannel(6);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetBlueChannel(7);
    
    // create pan motor
    pan_motor = std::make_unique<DmxMotor>("PanMotor");
    pan_motor->SetChannelCoarse(1);
    pan_motor->SetRangeOfMOtion(540.0);
    pan_motor->SetOrientHome(90);
    pan_motor->SetSlewLimit(180);
    
    // create tilt motor
    tilt_motor = std::make_unique<DmxMotor>("TiltMotor");
    tilt_motor->SetChannelCoarse(3);
    tilt_motor->SetOrientHome(90);
    tilt_motor->SetSlewLimit(180);
}

DmxMovingHeadAdv::~DmxMovingHeadAdv()
{
}

Mesh* DmxMovingHeadAdv::CreateBaseMesh(const std::string& name)
{
    base_mesh = std::make_unique<Mesh>(name);
    return base_mesh.get();
}

Mesh* DmxMovingHeadAdv::CreateYokeMesh(const std::string& name)
{
    yoke_mesh = std::make_unique<Mesh>(name);
    return yoke_mesh.get();
}

Mesh* DmxMovingHeadAdv::CreateHeadMesh(const std::string& name)
{
    head_mesh = std::make_unique<Mesh>(name);
    return head_mesh.get();
}

void DmxMovingHeadAdv::MapChannelName(std::vector<std::string>& array, int chan, std::string name)
{
    if (chan > 0) {
        array[chan-1] = name;
    }
}

void DmxMovingHeadAdv::InitModel()
{
    DmxModel::InitModel();

    // create base mesh
    if (base_mesh == nullptr) {
        std::string new_name = "BaseMesh";
        base_mesh = std::make_unique<Mesh>(new_name);
        base_mesh->SetObjFile(obj_path + "MovingHeadBase.obj");
        base_mesh->SetBrightness(40);
    }

    // create yoke mesh
    if (yoke_mesh == nullptr) {
        std::string new_name = "YokeMesh";
        yoke_mesh = std::make_unique<Mesh>(new_name);
        yoke_mesh->SetObjFile(obj_path + "MovingHeadYoke.obj");
        yoke_mesh->SetBrightness(50);
        yoke_mesh->SetRotateY(90);
    }

    // create head mesh
    if (head_mesh == nullptr) {
        std::string new_name = "HeadMesh";
        head_mesh = std::make_unique<Mesh>(new_name);
        head_mesh->SetObjFile(obj_path + "MovingHead.obj");
        head_mesh->SetBrightness(80);
        head_mesh->SetRotateX(90);
        head_mesh->SetRotateY(90);
        head_mesh->SetOffsetY(17);
    }

    bool base_defined = base_mesh->HasObjFile();
    bool yoke_defined = yoke_mesh->HasObjFile();
    bool head_defined = head_mesh->HasObjFile();

    pan_motor->Init();
    tilt_motor->Init();
    base_mesh->Init(this, head_defined ? false : (yoke_defined ? false : true));
    yoke_mesh->Init(this, head_defined ? false : (base_defined ? (yoke_defined ? true : false) : true));
    head_mesh->Init(this, head_defined ? true : (yoke_defined ? true : false));

    // I'd really like a better way to do this...colors are defaulting to
    // channels 1, 2, 3 and conflicting with Pan and Tilt defaults of 1 and 3
    CorrectDefaultColorChannels();

    // create node names
    std::string names = "";
    std::vector<std::string> nodestrings;
    nodestrings.resize(GetNumChannels());
    if (nodeNames.size() == 0) {
        int chan = pan_motor->GetChannelCoarse();
        MapChannelName(nodestrings, chan, "Pan");
        if (pan_motor->Is16Bit()) {
            chan = pan_motor->GetChannelFine();
            MapChannelName(nodestrings, chan, "Pan Fine");
        }
        chan = tilt_motor->GetChannelCoarse();
        MapChannelName(nodestrings, chan, "Tilt");
        if (tilt_motor->Is16Bit()) {
            chan = tilt_motor->GetChannelFine();
            nodestrings[chan] = "Tilt Fine";
        }
        if (nullptr != color_ability) {
            if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
                DmxColorAbilityRGB* crgb = dynamic_cast<DmxColorAbilityRGB*>(color_ability.get());
                chan = crgb->GetRedChannel();
                MapChannelName(nodestrings, chan, "Red");
                chan = crgb->GetGreenChannel();
                MapChannelName(nodestrings, chan, "Green");
                chan = crgb->GetBlueChannel();
                MapChannelName(nodestrings, chan, "Blue");
                chan = crgb->GetWhiteChannel();
                MapChannelName(nodestrings, chan, "White");
            }
            else if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW) {
                DmxColorAbilityCMY* ccmy = dynamic_cast<DmxColorAbilityCMY*>(color_ability.get());
                chan = ccmy->GetCyanChannel();
                MapChannelName(nodestrings, chan, "Cyan");
                chan = ccmy->GetMagentaChannel();
                MapChannelName(nodestrings, chan, "Magenta");
                chan = ccmy->GetYellowChannel();
                MapChannelName(nodestrings, chan, "Yellow");
                chan = ccmy->GetWhiteChannel();
                MapChannelName(nodestrings, chan, "White");
            }
        }
        for (size_t i = 0; i < nodestrings.size(); i++) {
            if (i > 0) names += ',';
            names += nodestrings[i];
        }
    }
    SetNodeNames(names, update_node_names);
    
    fixture_val = FixtureStringtoID(dmx_fixture);

    if (dmx_fixture.empty()) {
        dmx_fixture = "MH1";
    }
}

void DmxMovingHeadAdv::CorrectDefaultColorChannels()
{
    if (nullptr != color_ability) {
        if (color_ability->GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW) {
            DmxColorAbilityRGB* crgb = dynamic_cast<DmxColorAbilityRGB*>(color_ability.get());
            if (pan_motor->GetChannelCoarse() == (int)crgb->GetRedChannel() && crgb->GetRedChannel() == 1) {
                crgb->SetRedChannel(5);
                crgb->SetGreenChannel(6);
                crgb->SetBlueChannel(7);
            }
        }
    }
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
    sprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, !allowSelected, c);
    sprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=, this](xlGraphicsContext* ctx) {
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
        float scaleX = float(w) * 0.95f / float(GetModelScreenLocation().RenderWi);
        float scaleY = float(h) * 0.95f / float(GetModelScreenLocation().RenderHt);
        if (GetModelScreenLocation().RenderDp > 1) {
            float scaleZ = float(w) * 0.95f / float(GetModelScreenLocation().RenderDp);
            scaleX = std::min(scaleX, scaleZ);
        }

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
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
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

int DmxMovingHeadAdv::GetMinChannels()
{
    int min_channels = (pan_motor->Is16Bit() ? 2 : 1) + (tilt_motor->Is16Bit() ? 2 : 1);
    if( nullptr != color_ability ) {
        min_channels += color_ability->GetNumChannels();
    }
    min_channels += shutter_ability->GetShutterChannel() > 0 ? 1 : 0;
    return min_channels;
}

std::list<std::string> DmxMovingHeadAdv::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();
    int min_channels = GetMinChannels();
    if (min_channels > nodeCount) {
        res.push_back("    ERR: Model " + GetName() + " requires more channels " + std::to_string(min_channels) + " than have been allocated to it " + std::to_string(nodeCount) + ".");
    }
    
    if (pan_motor->GetChannelCoarse() > nodeCount) {
        res.push_back("    ERR: Model " + GetName() + " pan motor coarse is assigned to channel " + std::to_string(pan_motor->GetChannelCoarse()) + " but the model only has " + std::to_string(nodeCount) + " channels.");
    }

    if (pan_motor->GetChannelFine() > nodeCount) {
        res.push_back("    ERR: Model " + GetName() + " pan motor fine is assigned to channel " + std::to_string(pan_motor->GetChannelFine()) + " but the model only has " + std::to_string(nodeCount) + " channels.");
    }

    if (tilt_motor->GetChannelCoarse() > nodeCount) {
        res.push_back("    ERR: Model " + GetName() + " tilt motor coarse is assigned to channel " + std::to_string(tilt_motor->GetChannelCoarse()) + " but the model only has " + std::to_string(nodeCount) + " channels.");
    }

    if (tilt_motor->GetChannelFine() > nodeCount) {
        res.push_back("    ERR: Model " + GetName() + " tilt motor fine is assigned to channel " + std::to_string(tilt_motor->GetChannelFine()) + " but the model only has " + std::to_string(nodeCount) + " channels.");
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxMovingHeadAdv::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool active, const xlColor* c)
{
    // crash protection
    int min_channels = GetMinChannels();
    if (min_channels > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (pan_motor->GetChannelCoarse() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (pan_motor->GetChannelFine() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelCoarse() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelFine() > (int)Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    size_t NodeCount = Nodes.size();
    if ((( nullptr != color_ability ) && !color_ability->IsValidModelSettings(this)) ||
        !preset_ability->IsValidModelSettings(this) ||
        shutter_ability->GetShutterChannel() > (int)NodeCount ||
        dimmer_ability->GetDimmerChannel() > (int)NodeCount)
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
    if (pan_motor->GetChannelCoarse() > 0 && active) {
        pan_angle = pan_motor->GetPosition(GetChannelValue(pan_motor->GetChannelCoarse() - 1, pan_motor->GetChannelFine() - 1));
    }
    pan_angle += pan_motor->GetOrientZero();

    float tilt_angle = 0;
    if (tilt_motor->GetChannelCoarse() > 0 && active) {
        tilt_angle = tilt_motor->GetPosition(GetChannelValue(tilt_motor->GetChannelCoarse() - 1, tilt_motor->GetChannelFine() - 1));
    }
    tilt_angle += tilt_motor->GetOrientZero();

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
        if (pan_motor->GetSlewLimit() > 0.0f) {
            float slew_limit = pan_motor->GetSlewLimit() * (float)time_delta / 1000.0f;
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
        if (tilt_motor->GetSlewLimit() > 0.0f) {
            float slew_limit = tilt_motor->GetSlewLimit() * (float)time_delta / 1000.0f;
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

    glm::mat4 Identity = glm::mat4(1.0f);

    // Draw Meshs
    base_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, false, 0, 0, 0, false, false);
    yoke_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, 0, pan_angle_raw, 0, false, 0, 0, 0, false, !active);
    head_mesh->Draw(this, preview, sprogram, tprogram, Identity, Identity, tilt_angle, pan_angle_raw, 0, false, 0, 0, 0, false, !active);

    // Everything below here is for drawing the light beam
    float beam_length_displayed = beam_ability->GetBeamLength();
    
    float scw = screenLocation.GetRenderWi() * screenLocation.GetScaleX();
    float sch = screenLocation.GetRenderHt() * screenLocation.GetScaleY();
    float scd = screenLocation.GetRenderDp() * screenLocation.GetScaleZ();
    float sbl = std::max(scw, std::max(sch, scd));
    beam_length_displayed *= sbl;

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_ability->GetShutterChannel() > 0 && shutter_ability->GetShutterChannel() <= (int)NodeCount && active) {
        xlColor proxy;
        Nodes[shutter_ability->GetShutterChannel() - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_ability->GetShutterThreshold();
        } else {
            shutter_open = shutter_value <= std::abs(shutter_ability->GetShutterThreshold());
        }
    }

    xlColor color;
    if (c != nullptr) {
        color = *c;
    }
    int trans = color == xlBLACK ? blackTransparency : transparency;
    xlColor beam_color = color_ability == nullptr ? xlWHITE : color_ability->GetBeamColor(Nodes);
    if (!active) {
        beam_color = xlWHITE;
    }

    // apply dimmer to beam
    if (dimmer_ability->GetDimmerChannel() > 0 && active) {
        xlColor proxy;
        Nodes[dimmer_ability->GetDimmerChannel() - 1]->GetColor(proxy);
        HSVValue hsv = proxy.asHSV();
        beam_color.red = (beam_color.red * hsv.value);
        beam_color.blue = (beam_color.blue * hsv.value);
        beam_color.green = (beam_color.green * hsv.value);
    }

    ApplyTransparency(beam_color, trans, trans);

    pan_angle_raw += beam_ability->GetBeamOrient();
    while (pan_angle_raw > 360.0f)
        pan_angle_raw -= 360.0f;
    pan_angle_raw = 360.0f - pan_angle_raw;

    auto vac = tprogram->getAccumulator();
    int start = vac->getCount();
    Draw3DBeam(vac, beam_color, beam_length_displayed, pan_angle_raw, tilt_angle, shutter_open, beam_ability->GetBeamYOffset());
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

    if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
        if (shutter_open) {
            float angle1 = beam_ability->GetBeamWidth() / 2.0f;
            if (angle1 < 0.0f) {
                angle1 += 360.0f;
            }
            float x1 = (RenderBuffer::cos(ToRadians(angle1)) * beam_length_displayed);
            float y1 = (RenderBuffer::sin(ToRadians(angle1)) * beam_length_displayed);

            dmxPoint3 p1(x1, -y1, -y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p2(x1, -y1, y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p3(x1, y1, -y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p4(x1, y1, y1, pan_angle_raw, tilt_angle);
            dmxPoint3 p0(0, 0, 0, pan_angle_raw, tilt_angle);
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

std::vector<std::string> DmxMovingHeadAdv::GenerateNodeNames() const {
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_ability->GetShutterChannel() && shutter_ability->GetShutterChannel() < (int)names.size()) {
        names[shutter_ability->GetShutterChannel() - 1] = "Shutter";
    }
    if (0 != pan_motor->GetChannelCoarse() && pan_motor->GetChannelCoarse() < (int)names.size()) {
        names[pan_motor->GetChannelCoarse() - 1] = "Pan";
    }
    if (0 != tilt_motor->GetChannelCoarse() && tilt_motor->GetChannelCoarse() < (int)names.size()) {
        names[tilt_motor->GetChannelCoarse() - 1] = "Tilt";
    }
    if (0 != pan_motor->GetChannelFine() && pan_motor->GetChannelFine() < (int)names.size()) {
        names[pan_motor->GetChannelFine() - 1] = "Pan Fine";
    }
    if (0 != tilt_motor->GetChannelFine() && tilt_motor->GetChannelFine() < (int)names.size()) {
        names[tilt_motor->GetChannelFine() - 1] = "Tilt Fine";
    }

    return names;
}
