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

#include "DmxMovingHead.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxColorAbilityWheel.h"
#include "DmxPresetAbility.h"
#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxMovingHead::DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
  : DmxModel(node, manager, zeroBased), hide_body(false), style_changed(false), dmx_style("Moving Head Top"),
    dmx_style_val(0), beam_length(4)
{
    beam_width = GetDefaultBeamWidth();
    SetFromXml(node, zeroBased);
}

DmxMovingHead::~DmxMovingHead()
{
    //dtor
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

static wxPGChoices DMX_STYLES;

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

    DmxModel::AddTypeProperties(grid, outputManager);

    wxPGProperty* p = grid->Append(new wxBoolProperty("Hide Body", "HideBody", hide_body));
    p->SetAttribute("UseCheckbox", true);

    AddPanTiltTypeProperties(grid);

    if (nullptr != color_ability) {
        int selected = DMX_COLOR_TYPES.Index(color_ability->GetTypeName());
        grid->Append(new wxEnumProperty("Color Type", "DmxColorType", DMX_COLOR_TYPES, selected));
        color_ability->AddColorTypeProperties(grid);
    }
    AddShutterTypeProperties(grid);

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
}

int DmxMovingHead::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnPanTiltPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnShutterPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if ("DmxStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxStyle");
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
        ModelXml->AddAttribute("DmxStyle", dmx_style);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DMXStyle");
        return 0;
    } else if ("HideBody" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("HideBody");
        if (event.GetPropertyValue().GetBool()) {
            ModelXml->AddAttribute("HideBody", "True");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::HideBody");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::HideBody");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::HideBody");
        return 0;
    }
    else if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DMXBeamLength");
        return 0;
    }
    else if ("DmxBeamWidth" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamWidth");
        ModelXml->AddAttribute("DmxBeamWidth", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DMXBeamWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DMXBeamWidth");
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxMovingHead::OnPropertyGridChange::DmxColorType");
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxMovingHead::InitModel() {
    DmxModel::InitModel();
    if (DisplayAs != "DmxMovingHead3D") {
        DisplayAs = "DmxMovingHead";
        screenLocation.SetRenderSize(1, 1, 1);
        if (screenLocation.GetScaleZ() < 1.0) {
            screenLocation.SetScaleZ(1.0);
        }
    }
    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;
    hide_body = ModelXml->GetAttribute("HideBody", "False") == "True";
	dmx_style = ModelXml->GetAttribute("DmxStyle", "Moving Head Top");

    int color_type = wxAtoi(ModelXml->GetAttribute("DmxColorType", "0"));
    if (color_type == 0) {
        color_ability = std::make_unique<DmxColorAbilityRGB>(ModelXml);
    }else if (color_type == 1) {
        color_ability = std::make_unique<DmxColorAbilityWheel>(ModelXml);
    }
    else {
        color_ability = std::make_unique<DmxColorAbilityCMY>(ModelXml);
    }

    pan_channel = wxAtoi(ModelXml->GetAttribute("DmxPanChannel", "0"));
	pan_orient = wxAtoi(ModelXml->GetAttribute("DmxPanOrient", "0"));
	pan_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxPanDegOfRot", "540"));
	pan_slew_limit = wxAtof(ModelXml->GetAttribute("DmxPanSlewLimit", "0"));
	tilt_channel = wxAtoi(ModelXml->GetAttribute("DmxTiltChannel", "0"));
	tilt_orient = wxAtoi(ModelXml->GetAttribute("DmxTiltOrient", "0"));
	tilt_deg_of_rot = wxAtoi(ModelXml->GetAttribute("DmxTiltDegOfRot", "180"));
	tilt_slew_limit = wxAtof(ModelXml->GetAttribute("DmxTiltSlewLimit", "0"));
	shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
	shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    shutter_on_value = wxAtoi(ModelXml->GetAttribute("DmxShutterOnValue", "0"));
	beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "4.0"));
    beam_width = GetDefaultBeamWidth();
    if (ModelXml->HasAttribute("DmxBeamWidth")) {
        beam_width = wxAtof(ModelXml->GetAttribute("DmxBeamWidth"));
    }

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
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
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

    if (pan_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), pan_channel, nodeCount));
    }
    if (tilt_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), tilt_channel, nodeCount));
    }

    res.splice(res.end(), DmxModel::CheckModelSettings());
    return res;
}

void DmxMovingHead::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is3d, bool active, const xlColor* c)
{
    size_t NodeCount = Nodes.size();

    if (pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        !color_ability->IsValidModelSettings(this) ||
        !preset_ability->IsValidModelSettings(this)) {
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

    xlColor beam_color = color_ability->GetBeamColor(Nodes);

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
    if (pan_channel > 0 && pan_channel <= NodeCount && active) {
        Nodes[pan_channel - 1]->GetColor(color_angle);
        pan_angle = (color_angle.red / 255.0f) * pan_deg_of_rot + pan_orient;
    } else {
        pan_angle = pan_orient;
    }

    float tilt_angle = 0;
    if (tilt_channel > 0 && tilt_channel <= NodeCount && active) {
        Nodes[tilt_channel - 1]->GetColor(color_angle);
        tilt_angle = (color_angle.red / 255.0f) * tilt_deg_of_rot + tilt_orient;
    } else {
        tilt_angle = tilt_orient;
    }

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
    float tilt_pos = RenderBuffer::cos(ToRadians(tilt_angle)) * 0.9f;
    if (tilt_pos < 0) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
        tilt_pos *= -1;
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

    auto tvac = tprogram->getAccumulator();
    int tStart = tvac->getCount();
    auto vac = sprogram->getAccumulator();
    int startVert = vac->getCount();

    float beam_length_displayed = beam_length / 2.0f;

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

    float angle1 = angle - beam_width / 2.0f;
    float angle2 = angle + beam_width / 2.0f;
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
            } else if (i == pan_channel) {
                proxy = pink;
            } else if (i == tilt_channel) {
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

void DmxMovingHead::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    ExportBaseParameters(f);

    wxString s = ModelXml->GetAttribute("DmxStyle", dmx_style);
    wxString pdr = ModelXml->GetAttribute("DmxPanDegOfRot", "540");
    wxString tdr = ModelXml->GetAttribute("DmxTiltDegOfRot", "180");
    wxString pc = ModelXml->GetAttribute("DmxPanChannel", "0");
    wxString po = ModelXml->GetAttribute("DmxPanOrient", "0");
    wxString psl = ModelXml->GetAttribute("DmxPanSlewLimit", "0");
    wxString tc = ModelXml->GetAttribute("DmxTiltChannel", "0");
    wxString to = ModelXml->GetAttribute("DmxTiltOrient", "0");
    wxString tsl = ModelXml->GetAttribute("DmxTiltSlewLimit", "0");

    wxString sc = ModelXml->GetAttribute("DmxShutterChannel", "0");
    wxString so = ModelXml->GetAttribute("DmxShutterOpen", "1");
    wxString sv = ModelXml->GetAttribute("DmxShutterOnValue", "0");
    wxString dbl = ModelXml->GetAttribute("DmxBeamLength", "4");
    wxString dbw = ModelXml->GetAttribute("DmxBeamWidth", "30");

    wxString dct = ModelXml->GetAttribute("DmxColorType", "0");

    if (s.empty()) {
        s = "Moving Head Top";
    }

    f.Write(wxString::Format("DmxStyle=\"%s\" ", s));
    f.Write(wxString::Format("DmxPanDegOfRot=\"%s\" ", pdr));
    f.Write(wxString::Format("DmxTiltDegOfRot=\"%s\" ", tdr));
    f.Write(wxString::Format("DmxPanChannel=\"%s\" ", pc));
    f.Write(wxString::Format("DmxPanOrient=\"%s\" ", po));
    f.Write(wxString::Format("DmxPanSlewLimit=\"%s\" ", psl));
    f.Write(wxString::Format("DmxTiltChannel=\"%s\" ", tc));
    f.Write(wxString::Format("DmxTiltOrient=\"%s\" ", to));
    f.Write(wxString::Format("DmxTiltSlewLimit=\"%s\" ", tsl));

    f.Write(wxString::Format("DmxShutterChannel=\"%s\" ", sc));
    f.Write(wxString::Format("DmxShutterOpen=\"%s\" ", so));
    f.Write(wxString::Format("DmxShutterOnValue=\"%s\" ", sv));
    f.Write(wxString::Format("DmxBeamLength=\"%s\" ", dbl));
    f.Write(wxString::Format("DmxBeamWidth=\"%s\" ", dbw));

    f.Write(wxString::Format("DmxColorType=\"%s\" ", dct));
    color_ability->ExportParameters(f,ModelXml);

    f.Write(" >\n");

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
    //ExportDimensions(f);
    f.Write("</dmxmodel>");
    f.Close();
}

void DmxMovingHead::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "dmxmodel") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");

        wxString s = root->GetAttribute("DmxStyle");
        wxString pdr = root->GetAttribute("DmxPanDegOfRot");
        wxString tdr = root->GetAttribute("DmxTiltDegOfRot");
        wxString pc = root->GetAttribute("DmxPanChannel");
        wxString po = root->GetAttribute("DmxPanOrient");
        wxString psl = root->GetAttribute("DmxPanSlewLimit");
        wxString tc = root->GetAttribute("DmxTiltChannel");
        wxString to = root->GetAttribute("DmxTiltOrient");
        wxString tsl = root->GetAttribute("DmxTiltSlewLimit");

        wxString sc = root->GetAttribute("DmxShutterChannel");
        wxString so = root->GetAttribute("DmxShutterOpen");
        wxString sv = root->GetAttribute("DmxShutterOnValue");
        wxString bl = root->GetAttribute("DmxBeamLimit");
        wxString dbl = root->GetAttribute("DmxBeamLength", "4");
        wxString dbw = root->GetAttribute("DmxBeamWidth", "30");
        wxString dct = root->GetAttribute("DmxColorType", "0");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("DmxStyle", s);
        SetProperty("DmxPanDegOfRot", pdr);
        SetProperty("DmxTiltDegOfRot", tdr);
        SetProperty("DmxPanChannel", pc);
        SetProperty("DmxPanOrient", po);
        SetProperty("DmxPanSlewLimit", psl);
        SetProperty("DmxTiltChannel", tc);
        SetProperty("DmxTiltOrient", to);
        SetProperty("DmxTiltSlewLimit", tsl);

        SetProperty("DmxShutterChannel", sc);
        SetProperty("DmxShutterOpen", so);
        SetProperty("DmxShutterOnValue", sv);
        SetProperty("DmxBeamLimit", bl);
        SetProperty("DmxBeamLength", dbl);
        SetProperty("DmxBeamWidth", dbw);
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

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHead::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxMovingHead model file.");
    }
}

void DmxMovingHead::EnableFixedChannels(xlColorVector& pixelVector)
{
    if (shutter_channel != 0 && shutter_on_value != 0) {
        if (Nodes.size() > shutter_channel - 1) {
            xlColor c(shutter_on_value, shutter_on_value, shutter_on_value);
            pixelVector[shutter_channel - 1] = c;
        }
    }
    DmxModel::EnableFixedChannels(pixelVector);
}

std::vector<std::string> DmxMovingHead::GenerateNodeNames() const
{
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_channel && shutter_channel < names.size()) {
        names[shutter_channel - 1] = "Shutter";
    }
    if (0 != pan_channel && pan_channel < names.size()) {
        names[pan_channel - 1] = "Pan";
    }
    if (0 != tilt_channel && tilt_channel < names.size()) {
        names[tilt_channel - 1] = "Tilt";
    }

    if (nullptr != color_ability) {
        color_ability->SetNodeNames(names);
    }

    return names;
}
