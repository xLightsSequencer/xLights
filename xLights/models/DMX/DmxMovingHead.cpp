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
#include <wx/xml/xml.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxMovingHead.h"
#include "DmxBeamAbility.h"
#include "DmxDimmerAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxColorAbilityWheel.h"
#include "DmxPresetAbility.h"
#include "DmxShutterAbility.h"
#include "../ModelScreenLocation.h"
#include "../../controllers/ControllerCaps.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxMovingHead::DmxMovingHead(const ModelManager &manager) :
    DmxMovingHeadComm(manager)
{
    color_ability = std::make_unique<DmxColorAbilityRGB>();
    dimmer_ability = std::make_unique<DmxDimmerAbility>();
    shutter_ability = std::make_unique<DmxShutterAbility>();
    beam_ability = std::make_unique<DmxBeamAbility>();
    beam_ability->SetDefaultBeamLength(4.0);
    beam_ability->SetDefaultBeamWidth(30.0);
    beam_ability->SetBeamLength(4.0);
    beam_ability->SetBeamWidth(30.0);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetRedChannel(1);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetGreenChannel(2);
    dynamic_cast<DmxColorAbilityRGB*>(color_ability.get())->SetBlueChannel(3);
}

DmxMovingHead::~DmxMovingHead()
{
}

DmxMotor* DmxMovingHead::CreatePanMotor(const std::string& name)
{
    pan_motor = std::make_unique<DmxMotor>(name);
    return pan_motor.get();
}

DmxMotor* DmxMovingHead::CreateTiltMotor(const std::string& name)
{
    tilt_motor = std::make_unique<DmxMotor>(name);
    return tilt_motor.get();
}

#define ToRadians(x) ((double)x * PI / (double)180.0)

class dmxPoint {

public:
    float x;
    float y;

    dmxPoint(float x_, float y_, float angle_)
        : x(x_), y(y_)
    {
        float s = RenderBuffer::sin(ToRadians(angle_));
        float c = RenderBuffer::cos(ToRadians(angle_));

        // rotate point
        float xnew = x * c - y * s;
        float ynew = x * s + y * c;

        // translate point
        x = xnew;
        y = ynew;
    }

private:
};

class dmxPoint3 {

public:
    float x;
    float y;
    float z;

    dmxPoint3(float x_, float y_, float z_, float pan_angle_, float tilt_angle_ = 0, float nod_angle_ = 0.0)
        : x(x_), y(y_), z(z_)
    {
        float pan_angle = wxDegToRad(pan_angle_);
        float tilt_angle = wxDegToRad(tilt_angle_);
        float nod_angle = wxDegToRad(nod_angle_);

        glm::vec4 position = glm::vec4(glm::vec3(x_, y_, z_), 1.0);

        glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrixNod = glm::rotate(glm::mat4(1.0f), nod_angle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec4 model_position = rotationMatrixPan * rotationMatrixTilt * rotationMatrixNod * position;
        x = model_position.x;
        y = model_position.y;
        z = model_position.z;
    }
};

static wxPGChoices DMX_COLOR_TYPES(wxArrayString(4, DMX_COLOR_TYPES_VALUES));

static wxPGChoices DMX_STYLES;

static wxPGChoices DMX_FIXTURES;

enum DMX_STYLE {
    DMX_STYLE_MOVING_HEAD_TOP,
    DMX_STYLE_MOVING_HEAD_SIDE,
    DMX_STYLE_MOVING_HEAD_BARS,
    DMX_STYLE_MOVING_HEAD_TOP_BARS,
    DMX_STYLE_MOVING_HEAD_SIDE_BARS,
    DMX_STYLE_MOVING_HEAD_3D
};

void DmxMovingHead::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    if (DMX_STYLES.GetCount() == 0) {
        DMX_STYLES.Add("Moving Head Top");
        DMX_STYLES.Add("Moving Head Side");
        DMX_STYLES.Add("Moving Head Bars");
        DMX_STYLES.Add("Moving Head Top Bars");
        DMX_STYLES.Add("Moving Head Side Bars");
        DMX_STYLES.Add("Moving Head 3D");
    }

    grid->Append(new wxEnumProperty("DMX Style", "DmxStyle", DMX_STYLES, dmx_style_val));

    if (DMX_FIXTURES.GetCount() == 0) {
        DMX_FIXTURES.Add("MH1");
        DMX_FIXTURES.Add("MH2");
        DMX_FIXTURES.Add("MH3");
        DMX_FIXTURES.Add("MH4");
        DMX_FIXTURES.Add("MH5");
        DMX_FIXTURES.Add("MH6");
        DMX_FIXTURES.Add("MH7");
        DMX_FIXTURES.Add("MH8");
    }

    grid->Append(new wxEnumProperty("Fixture", "DmxFixture", DMX_FIXTURES, fixture_val));

    DmxModel::AddTypeProperties(grid, outputManager);

    wxPGProperty* p = grid->Append(new wxBoolProperty("Hide Body", "HideBody", hide_body));
    p->SetAttribute("UseCheckbox", true);

    pan_motor->AddTypeProperties(grid);
    tilt_motor->AddTypeProperties(grid);

    grid->Append(new wxPropertyCategory("Color Properties", "DmxColorAbility"));
    int selected = 3; // show Unused if not selected
    if (nullptr != color_ability) {
        selected = DMX_COLOR_TYPES.Index(color_ability->GetTypeName());
    }
    grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES, selected));
    if (nullptr != color_ability) {
        ControllerCaps *caps = GetControllerCaps();
        color_ability->AddColorTypeProperties(grid, IsPWMProtocol() && caps && caps->SupportsPWM());
    }
    grid->Collapse("DmxColorAbility");

    dimmer_ability->AddDimmerTypeProperties(grid);
    shutter_ability->AddShutterTypeProperties(grid);
    beam_ability->AddBeamTypeProperties(grid);
    grid->Collapse("DmxDimmerProperties");
    grid->Collapse("DmxShutterProperties");
    grid->Collapse("DmxBeamProperties");

    grid->Append(new wxPropertyCategory("Common Properties", "CommonProperties"));
}

int DmxMovingHead::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, this) == 0) {
        return 0;
    }

    if (pan_motor->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (tilt_motor->OnPropertyGridChange(grid, event, this, GetModelScreenLocation().IsLocked()) == 0) {
        return 0;
    }

    if (shutter_ability->OnShutterPropertyGridChange(grid, event, this) == 0) {
        return 0;
    }

    if (dimmer_ability->OnDimmerPropertyGridChange(grid, event, this) == 0) {
        return 0;
    }

    if (beam_ability->OnBeamPropertyGridChange(grid, event, this) == 0) {
        return 0;
    }

    if ("DmxStyle" == event.GetPropertyName()) {
        dmx_style_val = event.GetPropertyValue().GetLong();
        if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP) {
            dmx_style = "Moving Head Top";
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE) {
            dmx_style = "Moving Head Side";
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS) {
            dmx_style = "Moving Head Bars";
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
            dmx_style = "Moving Head TopBars";
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
            dmx_style = "Moving Head SideBars";
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_3D) {
            dmx_style = "Moving Head 3D";
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        return 0;
    } else if ("HideBody" == event.GetPropertyName()) {
        hide_body = event.GetPropertyValue().GetBool();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::HideBody");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::HideBody");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::HideBody");
        return 0;
    }
    else if ("DmxColorType" == event.GetPropertyName()) {
        int color_type = event.GetPropertyValue().GetInteger();
        InitColorAbility(color_type);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        return 0;
    } else if ("DmxFixture" == event.GetPropertyName()) {
        fixture_val = event.GetPropertyValue().GetLong();
        dmx_fixture = FixtureIDtoString(fixture_val);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DmxFixture");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DmxFixture");
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxMovingHead::InitModel() {
    DmxModel::InitModel();
    screenLocation.SetRenderSize(1, 1, 1);
    if (screenLocation.GetScaleZ() < 1.0) {
        screenLocation.SetScaleZ(1.0);
    }
    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;

    // create pan motor
    if (pan_motor == nullptr) {
        std::string new_name = "PanMotor";
        pan_motor = std::make_unique<DmxMotor>(new_name);
        pan_motor->SetChannelCoarse(1);
        pan_motor->SetRangeOfMOtion(540.0);
        pan_motor->SetOrientHome(90);
        pan_motor->SetSlewLimit(180);
    }

    // create tilt motor
    if (tilt_motor == nullptr) {
        std::string new_name = "TiltMotor";
        tilt_motor = std::make_unique<DmxMotor>(new_name);
        tilt_motor->SetChannelCoarse(3);
        tilt_motor->SetOrientHome(90);
        tilt_motor->SetSlewLimit(180);
    }

    pan_motor->Init();
    tilt_motor->Init();

    dmx_style_val = DMX_STYLE_MOVING_HEAD_TOP;
    if( dmx_style == "Moving Head Side" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_SIDE;
    } else if( dmx_style == "Moving Head Bars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_BARS;
    } else if( dmx_style == "Moving Head TopBars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_TOP_BARS;
    } else if( dmx_style == "Moving Head SideBars" ) {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_SIDE_BARS;
    } else if (dmx_style == "Moving Head 3D") {
        dmx_style_val = DMX_STYLE_MOVING_HEAD_3D;
    }

    if (dmx_style.empty()) {
        dmx_style = "Moving Head Top";
    }

    fixture_val = FixtureStringtoID(dmx_fixture);

    if (dmx_fixture.empty()) {
        dmx_fixture = "MH1";
    }
}

void DmxMovingHead::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
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
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([is_3d, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, is_3d, !allowSelected, c);
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxMovingHead::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
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

std::list<std::string> DmxMovingHead::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();

    if (pan_motor->GetChannelCoarse() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan motor coarse is assigned to channel %d but the model only has %d channels.", GetName(), pan_motor->GetChannelCoarse(), nodeCount));
    }

    if (pan_motor->GetChannelFine() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan motor fine is assigned to channel %d but the model only has %d channels.", GetName(), pan_motor->GetChannelFine(), nodeCount));
    }

    if (tilt_motor->GetChannelCoarse() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt motor coarse is assigned to channel %d but the model only has %d channels.", GetName(), tilt_motor->GetChannelCoarse(), nodeCount));
    }

    if (tilt_motor->GetChannelFine() > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt motor fine is assigned to channel %d but the model only has %d channels.", GetName(), tilt_motor->GetChannelFine(), nodeCount));
    }

    res.splice(res.end(), DmxModel::CheckModelSettings());
    return res;
}

void DmxMovingHead::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is3d, bool active, const xlColor* c)
{
    if (pan_motor->GetChannelCoarse() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (pan_motor->GetChannelFine() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelCoarse() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }
    if (tilt_motor->GetChannelFine() > Nodes.size()) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    size_t NodeCount = Nodes.size();
    if ((( nullptr != color_ability ) && !color_ability->IsValidModelSettings(this)) ||
        !preset_ability->IsValidModelSettings(this) ||
        shutter_ability->GetShutterChannel() > NodeCount ||
        dimmer_ability->GetDimmerChannel() > NodeCount )
    {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor marker_color(xlBLACK);
    xlColor black(xlBLACK);
    xlColor base_color(200, 200, 200);
    xlColor base_color2(150, 150, 150);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    xlColor color_angle;

    int trans = color == xlBLACK ? blackTransparency : transparency;

    xlColor beam_color = color_ability == nullptr ? xlWHITE : color_ability->GetBeamColor(Nodes);

    // apply dimmer to beam
    if (dimmer_ability->GetDimmerChannel() > 0 && active) {
        xlColor proxy;
        Nodes[dimmer_ability->GetDimmerChannel() - 1]->GetColor(proxy);
        HSVValue hsv = proxy.asHSV();
        beam_color.red = (beam_color.red * hsv.value);
        beam_color.blue = (beam_color.blue * hsv.value);
        beam_color.green = (beam_color.green * hsv.value);
    }

    if (!active) {
        beam_color = xlWHITE;
    } else {
        marker_color = beam_color;
    }
    ApplyTransparency(beam_color, trans, trans);
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);
    ApplyTransparency(pnt_color, trans, trans);

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
        pan_angle = -1.0f * pan_motor->GetPosition(GetChannelValue(pan_motor->GetChannelCoarse() - 1, pan_motor->GetChannelFine() - 1));
    }
    pan_angle -= pan_motor->GetOrientZero();

    float tilt_angle = 0;
    if (tilt_motor->GetChannelCoarse() > 0 && active) {
        tilt_angle = -1.0f * tilt_motor->GetPosition(GetChannelValue(tilt_motor->GetChannelCoarse() - 1, tilt_motor->GetChannelFine() - 1));
    }
    tilt_angle -= tilt_motor->GetOrientZero();

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
    float tilt_pos = RenderBuffer::cos(ToRadians(tilt_angle));
    if (tilt_pos < 0.0f) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
        tilt_pos *= -1.0f;
    }

    float angle;
    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
        angle = pan_angle;
    } else {
        angle = tilt_angle;
    }

    // save the model state
    st.ms = ms;
    st.pan_angle = pan_angle_raw;
    st.tilt_angle = tilt_angle;

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_ability->GetShutterChannel() > 0 && shutter_ability->GetShutterChannel() <= NodeCount && active) {
        xlColor proxy;
        Nodes[shutter_ability->GetShutterChannel() - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_ability->GetShutterThreshold();
        } else {
            shutter_open = shutter_value <= std::abs(shutter_ability->GetShutterThreshold());
        }
    }

    auto tvac = tprogram->getAccumulator();
    int tStart = tvac->getCount();
    auto vac = sprogram->getAccumulator();
    int startVert = vac->getCount();

    float beam_length_displayed = beam_ability->GetBeamLength() / 2.0f;

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
        tprogram->addStep([=](xlGraphicsContext* ctx) {
            ctx->Translate(-0.25f, 0, 0);
            ctx->Scale(0.5f, 0.5f, 1.0f);
        });
        sprogram->addStep([=](xlGraphicsContext* ctx) {
            ctx->Translate(-0.25f, 0, 0);
            ctx->Scale(0.5f, 0.5f, 1.0f);
        });
        beam_length_displayed *= 2;
    }

    float angle1 = angle - beam_ability->GetBeamWidth() / 2.0f;
    float angle2 = angle + beam_ability->GetBeamWidth() / 2.0f;
    if (angle1 < 0.0f) {
        angle1 += 360.0f;
    }
    if (angle2 > 360.f) {
        angle2 -= 360.0f;
    }
    float x1 = (RenderBuffer::cos(ToRadians(angle1)) * beam_length_displayed);
    float y1 = (RenderBuffer::sin(ToRadians(angle1)) * beam_length_displayed);
    float x2 = (RenderBuffer::cos(ToRadians(angle2)) * beam_length_displayed);
    float y2 = (RenderBuffer::sin(ToRadians(angle2)) * beam_length_displayed);

    // Draw the light beam
    if (dmx_style_val != DMX_STYLE_MOVING_HEAD_BARS && dmx_style_val != DMX_STYLE_MOVING_HEAD_3D && shutter_open) {
        tvac->AddVertex(0, 0, 0, beam_color);
        beam_color.alpha = 0;
        tvac->AddVertex(x1, y1, 0, beam_color);
        tvac->AddVertex(x2, y2, 0, beam_color);
    }

    if (!hide_body) {
        if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
            vac->AddCircleAsTriangles(0, 0, 0, 0.5f, ccolor, ccolor, 0, 48);

            // draw angle line
            dmxPoint p1(0, -0.05f, angle);
            dmxPoint p2(0.5f, -0.05f, angle);
            dmxPoint p3(0.5f, 0.05f, angle);
            dmxPoint p4(0, 0.05f, angle);

            vac->AddVertex(p1.x, p1.y, 0.1f, pnt_color);
            vac->AddVertex(p2.x, p2.y, 0.1f, pnt_color);
            vac->AddVertex(p3.x, p3.y, 0.1f, pnt_color);

            vac->AddVertex(p1.x, p1.y, 0.1f, pnt_color);
            vac->AddVertex(p3.x, p3.y, 0.1f, pnt_color);
            vac->AddVertex(p4.x, p4.y, 0.1f, pnt_color);

            // draw tilt marker
            dmxPoint marker(tilt_pos / 2.0f, 0, angle);
            vac->AddCircleAsTriangles(marker.x, marker.y, 0.2f, 0.07f, black, black);
            vac->AddCircleAsTriangles(marker.x, marker.y, 0.3f, 0.06f, marker_color, marker_color);
        } else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE || dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
            // draw head
            dmxPoint p1(0.5f, -0.5f, angle);
            dmxPoint p2(0.5f, +0.5f, angle);
            dmxPoint p3(-0.4f, +0.4f, angle);
            dmxPoint p4(-0.5f, +0.2f, angle);
            dmxPoint p5(-0.5f, -0.2f, angle);
            dmxPoint p6(-0.4f, -0.4f, angle);
            vac->AddVertex(p1.x, p1.y, 0.1f, ccolor);
            vac->AddVertex(p2.x, p2.y, 0.1f, ccolor);
            vac->AddVertex(p6.x, p6.y, 0.1f, ccolor);

            vac->AddVertex(p2.x, p2.y, 0.1f, ccolor);
            vac->AddVertex(p3.x, p3.y, 0.1f, ccolor);
            vac->AddVertex(p6.x, p6.y, 0.1f, ccolor);

            vac->AddVertex(p3.x, p3.y, 0.1f, ccolor);
            vac->AddVertex(p5.x, p5.y, 0.1f, ccolor);
            vac->AddVertex(p6.x, p6.y, 0.1f, ccolor);
            vac->AddVertex(p3.x, p3.y, 0.1f, ccolor);
            vac->AddVertex(p4.x, p4.y, 0.1f, ccolor);
            vac->AddVertex(p5.x, p5.y, 0.1f, ccolor);

            // draw base
            vac->AddCircleAsTriangles(0, 0, 0.2f, 0.2f, base_color, base_color, 0, 32);
            vac->AddRectAsTriangles(-0.2f, 0, 0.2f, -0.7f, 0.2f, base_color);

            // draw pan marker
            dmxPoint p7(0.2f, 0.05f, pan_angle);
            dmxPoint p8(0.2f, -0.05f, pan_angle);
            vac->AddVertex(0, 0, 0.3f, marker_color);
            vac->AddVertex(p7.x, p7.y, 0.3f, marker_color);
            vac->AddVertex(p8.x, p8.y, 0.3f, marker_color);
        }
    }

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
        // draw the bars
        xlColor proxy;
        xlColor pink(255, 51, 255);
        xlColor turqoise(64, 224, 208);

        ApplyTransparency(pink, trans, trans);
        ApplyTransparency(turqoise, trans, trans);

        vac->AddRectAsTriangles(0.6f, 0.9f, 0.65f, -0.9f, ccolor);
        vac->AddRectAsTriangles(1.5f, 0.9f, 1.45f, -0.9f, ccolor);
        vac->AddRectAsTriangles(0.6f, 0.9f, 1.5f, 0.85f, ccolor);
        vac->AddRectAsTriangles(0.6f, -0.9f, 1.5f, -0.85f, ccolor);

        float lineSize = 1.7f / ((float)NodeCount);
        float barSize = lineSize * 0.8f;
        float lineStart = 0.825f;
        for (int i = 1; i <= NodeCount; ++i) {
            Nodes[i - 1]->GetColor(proxy);
            float val = (float)proxy.red;
            float offsetx = val / 255.0f * 0.8f;
            if (color_ability->ApplyChannelTransparency(proxy, trans, i)) {
            } else if (i == pan_motor->GetChannelCoarse()) {
                proxy = pink;
            } else if (i == tilt_motor->GetChannelCoarse()) {
                proxy = turqoise;
            } else {
                proxy = ccolor;
            }
            vac->AddRectAsTriangles(0.65f, lineStart, 0.65f + offsetx, lineStart - barSize, 0.3f, proxy);
            lineStart -= lineSize;
        }
    }

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_3D) {
        while (pan_angle_raw > 360.0f)
            pan_angle_raw -= 360.0f;
        pan_angle_raw = 360.0f - pan_angle_raw;
        bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

        Draw3DBeam(tvac, beam_color, beam_length_displayed, pan_angle_raw, tilt_angle, shutter_open);

        if (!hide_body) {
            if (!facing_right) {
                Draw3DDMXBaseRight(*vac, base_color, pan_angle_raw);
                Draw3DDMXHead(*vac, base_color2, pan_angle_raw, tilt_angle);
                Draw3DDMXBaseLeft(*vac, base_color, pan_angle_raw);
            } else {
                Draw3DDMXBaseLeft(*vac, base_color, pan_angle_raw);
                Draw3DDMXHead(*vac, base_color2, pan_angle_raw, tilt_angle);
                Draw3DDMXBaseRight(*vac, base_color, pan_angle_raw);
            }
        }
    }
    int tEnd = tvac->getCount();
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->drawTriangles(tvac, tStart, tEnd - tStart);
    });

    int end = vac->getCount();
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->drawTriangles(vac, startVert, end - startVert);
    });
}

static inline void AddVertex(xlVertexColorAccumulator& va, const dmxPoint3& point, const xlColor& c)
{
    va.AddVertex(point.x, point.y, point.z, c);
}

void DmxMovingHead::Draw3DBeam(xlVertexColorAccumulator* tvac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open)
{
    xlColor beam_color_end(beam_color);
    beam_color_end.alpha = 0;

    bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

    float combined_angle = tilt_angle;
    if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
        if (shutter_open) {
            float angle1 = float(beam_ability->GetBeamWidth()) / 2.0f;
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

void DmxMovingHead::Draw3DDMXBaseLeft(xlVertexColorAccumulator& va, const xlColor& c, float pan_angle)
{
    constexpr float scaleFactor = 15.0f;
    dmxPoint3 p10(-3 / scaleFactor, -1 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p11(3 / scaleFactor, -1 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p12(-3 / scaleFactor, -5 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p13(3 / scaleFactor, -5 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p14(0 / scaleFactor, -1 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p15(-1 / scaleFactor, 1 / scaleFactor, -5 / scaleFactor, pan_angle);
    dmxPoint3 p16(1 / scaleFactor, 1 / scaleFactor, -5 / scaleFactor, pan_angle);

    AddVertex(va, p10, c);
    AddVertex(va, p11, c);
    AddVertex(va, p12, c);
    AddVertex(va, p11, c);
    AddVertex(va, p12, c);
    AddVertex(va, p13, c);
    AddVertex(va, p10, c);
    AddVertex(va, p14, c);
    AddVertex(va, p15, c);
    AddVertex(va, p11, c);
    AddVertex(va, p14, c);
    AddVertex(va, p16, c);
    AddVertex(va, p15, c);
    AddVertex(va, p14, c);
    AddVertex(va, p16, c);

    dmxPoint3 p210(-3 / scaleFactor, -1 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p211(3 / scaleFactor, -1 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p212(-3 / scaleFactor, -5 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p213(3 / scaleFactor, -5 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p214(0 / scaleFactor, -1 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p215(-1 / scaleFactor, 1 / scaleFactor, -3 / scaleFactor, pan_angle);
    dmxPoint3 p216(1 / scaleFactor, 1 / scaleFactor, -3 / scaleFactor, pan_angle);

    AddVertex(va, p210, c);
    AddVertex(va, p211, c);
    AddVertex(va, p212, c);
    AddVertex(va, p211, c);
    AddVertex(va, p212, c);
    AddVertex(va, p213, c);
    AddVertex(va, p210, c);
    AddVertex(va, p214, c);
    AddVertex(va, p215, c);
    AddVertex(va, p211, c);
    AddVertex(va, p214, c);
    AddVertex(va, p216, c);
    AddVertex(va, p215, c);
    AddVertex(va, p214, c);
    AddVertex(va, p216, c);

    AddVertex(va, p10, c);
    AddVertex(va, p210, c);
    AddVertex(va, p212, c);
    AddVertex(va, p10, c);
    AddVertex(va, p12, c);
    AddVertex(va, p212, c);
    AddVertex(va, p10, c);
    AddVertex(va, p210, c);
    AddVertex(va, p215, c);
    AddVertex(va, p10, c);
    AddVertex(va, p15, c);
    AddVertex(va, p215, c);
    AddVertex(va, p15, c);
    AddVertex(va, p16, c);
    AddVertex(va, p215, c);
    AddVertex(va, p16, c);
    AddVertex(va, p216, c);
    AddVertex(va, p215, c);
    AddVertex(va, p16, c);
    AddVertex(va, p11, c);
    AddVertex(va, p211, c);
    AddVertex(va, p16, c);
    AddVertex(va, p211, c);
    AddVertex(va, p216, c);
    AddVertex(va, p13, c);
    AddVertex(va, p211, c);
    AddVertex(va, p213, c);
    AddVertex(va, p13, c);
    AddVertex(va, p211, c);
    AddVertex(va, p11, c);
}

void DmxMovingHead::Draw3DDMXBaseRight(xlVertexColorAccumulator& va, const xlColor& c, float pan_angle)
{
    constexpr float scaleFactor = 15.0f;
    dmxPoint3 p20(-3 / scaleFactor, -1 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p21(3 / scaleFactor, -1 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p22(-3 / scaleFactor, -5 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p23(3 / scaleFactor, -5 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p24(0 / scaleFactor, -1 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p25(-1 / scaleFactor, 1 / scaleFactor, 5 / scaleFactor, pan_angle);
    dmxPoint3 p26(1 / scaleFactor, 1 / scaleFactor, 5 / scaleFactor, pan_angle);

    AddVertex(va, p20, c);
    AddVertex(va, p21, c);
    AddVertex(va, p22, c);
    AddVertex(va, p21, c);
    AddVertex(va, p22, c);
    AddVertex(va, p23, c);
    AddVertex(va, p20, c);
    AddVertex(va, p24, c);
    AddVertex(va, p25, c);
    AddVertex(va, p21, c);
    AddVertex(va, p24, c);
    AddVertex(va, p26, c);
    AddVertex(va, p25, c);
    AddVertex(va, p24, c);
    AddVertex(va, p26, c);

    dmxPoint3 p220(-3 / scaleFactor, -1 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p221(3 / scaleFactor, -1 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p222(-3 / scaleFactor, -5 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p223(3 / scaleFactor, -5 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p224(0 / scaleFactor, -1 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p225(-1 / scaleFactor, 1 / scaleFactor, 3 / scaleFactor, pan_angle);
    dmxPoint3 p226(1 / scaleFactor, 1 / scaleFactor, 3 / scaleFactor, pan_angle);

    AddVertex(va, p220, c);
    AddVertex(va, p221, c);
    AddVertex(va, p222, c);
    AddVertex(va, p221, c);
    AddVertex(va, p222, c);
    AddVertex(va, p223, c);
    AddVertex(va, p220, c);
    AddVertex(va, p224, c);
    AddVertex(va, p225, c);
    AddVertex(va, p221, c);
    AddVertex(va, p224, c);
    AddVertex(va, p226, c);
    AddVertex(va, p225, c);
    AddVertex(va, p224, c);
    AddVertex(va, p226, c);

    AddVertex(va, p20, c);
    AddVertex(va, p220, c);
    AddVertex(va, p222, c);
    AddVertex(va, p20, c);
    AddVertex(va, p22, c);
    AddVertex(va, p222, c);
    AddVertex(va, p20, c);
    AddVertex(va, p220, c);
    AddVertex(va, p225, c);
    AddVertex(va, p20, c);
    AddVertex(va, p25, c);
    AddVertex(va, p225, c);
    AddVertex(va, p25, c);
    AddVertex(va, p26, c);
    AddVertex(va, p225, c);
    AddVertex(va, p26, c);
    AddVertex(va, p226, c);
    AddVertex(va, p225, c);
    AddVertex(va, p26, c);
    AddVertex(va, p21, c);
    AddVertex(va, p221, c);
    AddVertex(va, p26, c);
    AddVertex(va, p221, c);
    AddVertex(va, p226, c);
    AddVertex(va, p23, c);
    AddVertex(va, p221, c);
    AddVertex(va, p223, c);
    AddVertex(va, p23, c);
    AddVertex(va, p221, c);
    AddVertex(va, p21, c);
}

void DmxMovingHead::Draw3DDMXHead(xlVertexColorAccumulator& va, const xlColor& c, float pan_angle, float tilt_angle)
{
    // draw the head
    constexpr float scaleFactor = 15.0f;
    float pan_angle1 = pan_angle + 270.0f; // needs to be rotated from reference we drew it
    dmxPoint3 p31(-2 / scaleFactor, 3.45f / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p32(2 / scaleFactor, 3.45f / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p33(4 / scaleFactor, 0 / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p34(2 / scaleFactor, -3.45f / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p35(-2 / scaleFactor, -3.45f / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p36(-4 / scaleFactor, 0 / scaleFactor, -4 / scaleFactor, pan_angle1, 0, tilt_angle);

    dmxPoint3 p41(-1 / scaleFactor, 1.72f / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p42(1 / scaleFactor, 1.72f / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p43(2 / scaleFactor, 0 / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p44(1 / scaleFactor, -1.72f / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p45(-1 / scaleFactor, -1.72f / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);
    dmxPoint3 p46(-2 / scaleFactor, 0 / scaleFactor, 4 / scaleFactor, pan_angle1, 0, tilt_angle);

    AddVertex(va, p31, c);
    AddVertex(va, p32, c);
    AddVertex(va, p35, c);
    AddVertex(va, p34, c);
    AddVertex(va, p32, c);
    AddVertex(va, p35, c);
    AddVertex(va, p32, c);
    AddVertex(va, p33, c);
    AddVertex(va, p34, c);
    AddVertex(va, p31, c);
    AddVertex(va, p36, c);
    AddVertex(va, p35, c);

    AddVertex(va, p41, c);
    AddVertex(va, p42, c);
    AddVertex(va, p45, c);
    AddVertex(va, p44, c);
    AddVertex(va, p42, c);
    AddVertex(va, p45, c);
    AddVertex(va, p42, c);
    AddVertex(va, p43, c);
    AddVertex(va, p44, c);
    AddVertex(va, p41, c);
    AddVertex(va, p46, c);
    AddVertex(va, p45, c);

    AddVertex(va, p31, c);
    AddVertex(va, p41, c);
    AddVertex(va, p42, c);
    AddVertex(va, p31, c);
    AddVertex(va, p32, c);
    AddVertex(va, p42, c);

    AddVertex(va, p32, c);
    AddVertex(va, p42, c);
    AddVertex(va, p43, c);
    AddVertex(va, p32, c);
    AddVertex(va, p33, c);
    AddVertex(va, p43, c);

    AddVertex(va, p33, c);
    AddVertex(va, p43, c);
    AddVertex(va, p44, c);
    AddVertex(va, p33, c);
    AddVertex(va, p34, c);
    AddVertex(va, p44, c);

    AddVertex(va, p34, c);
    AddVertex(va, p44, c);
    AddVertex(va, p45, c);
    AddVertex(va, p34, c);
    AddVertex(va, p35, c);
    AddVertex(va, p45, c);

    AddVertex(va, p35, c);
    AddVertex(va, p45, c);
    AddVertex(va, p46, c);
    AddVertex(va, p35, c);
    AddVertex(va, p36, c);
    AddVertex(va, p46, c);

    AddVertex(va, p36, c);
    AddVertex(va, p46, c);
    AddVertex(va, p41, c);
    AddVertex(va, p36, c);
    AddVertex(va, p31, c);
    AddVertex(va, p41, c);
}

void DmxMovingHead::EnableFixedChannels(xlColorVector& pixelVector) const
{
    if (shutter_ability->GetShutterChannel() != 0 && shutter_ability->GetShutterOnValue()!= 0) {
        if (Nodes.size() > shutter_ability->GetShutterChannel() - 1) {
            xlColor c(shutter_ability->GetShutterOnValue(), shutter_ability->GetShutterOnValue(), shutter_ability->GetShutterOnValue());
            pixelVector[shutter_ability->GetShutterChannel() - 1] = c;
        }
    }
    DmxModel::EnableFixedChannels(pixelVector);
}

std::vector<std::string> DmxMovingHead::GenerateNodeNames() const
{
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_ability->GetShutterChannel() && shutter_ability->GetShutterChannel() < names.size()) {
        names[shutter_ability->GetShutterChannel() - 1] = "Shutter";
    }
    if (0 != pan_motor->GetChannelCoarse() && pan_motor->GetChannelCoarse() < names.size()) {
        names[pan_motor->GetChannelCoarse() - 1] = "Pan";
    }
    if (0 != tilt_motor->GetChannelCoarse() && tilt_motor->GetChannelCoarse() < names.size()) {
        names[tilt_motor->GetChannelCoarse() - 1] = "Tilt";
    }
    if (0 != pan_motor->GetChannelFine() && pan_motor->GetChannelFine() < names.size()) {
        names[pan_motor->GetChannelFine() - 1] = "Pan Fine";
    }
    if (0 != tilt_motor->GetChannelFine() && tilt_motor->GetChannelFine() < names.size()) {
        names[tilt_motor->GetChannelFine() - 1] = "Tilt Fine";
    }

    if (nullptr != color_ability) {
        color_ability->SetNodeNames(names);
    }

    return names;
}
