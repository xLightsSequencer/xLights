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
#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxMovingHead::DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
  : DmxModel(node, manager, zeroBased), hide_body(false), style_changed(false), dmx_style("MOVING_HEAD_TOP"),
    dmx_style_val(0), beam_length(4)
{
    beam_width = GetDefaultBeamWidth();
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxMovingHead::~DmxMovingHead()
{
    //dtor
}

const double PI = 3.141592653589793238463;
#define ToRadians(x) ((double)x * PI / (double)180.0)

class dmxPoint {

public:
    float x;
    float y;

    dmxPoint(float x_, float y_, int cx_, int cy_, float scale_, float angle_)
        : x(x_), y(y_), cx(cx_), cy(cy_), scale(scale_)
    {
        float s = RenderBuffer::sin(ToRadians(angle_));
        float c = RenderBuffer::cos(ToRadians(angle_));

        // scale point
        x *= scale;
        y *= scale;

        // rotate point
        float xnew = x * c - y * s;
        float ynew = x * s + y * c;

        // translate point
        x = xnew + cx;
        y = ynew + cy;
    }

private:
    float cx;
    float cy;
    float scale;
};

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

static wxPGChoices DMX_STYLES;

enum DMX_STYLE {
    DMX_STYLE_MOVING_HEAD_TOP,
    DMX_STYLE_MOVING_HEAD_SIDE,
    DMX_STYLE_MOVING_HEAD_BARS,
    DMX_STYLE_MOVING_HEAD_TOP_BARS,
    DMX_STYLE_MOVING_HEAD_SIDE_BARS,
    DMX_STYLE_MOVING_HEAD_3D
};

void DmxMovingHead::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (DMX_STYLES.GetCount() == 0) {
        DMX_STYLES.Add("Moving Head Top");
        DMX_STYLES.Add("Moving Head Side");
        DMX_STYLES.Add("Moving Head Bars");
        DMX_STYLES.Add("Moving Head Top Bars");
        DMX_STYLES.Add("Moving Head Side Bars");
        DMX_STYLES.Add("Moving Head 3D");
    }

    grid->Append(new wxEnumProperty("DMX Style", "DmxStyle", DMX_STYLES, dmx_style_val));

    DmxModel::AddTypeProperties(grid);

    wxPGProperty* p = grid->Append(new wxBoolProperty("Hide Body", "HideBody", hide_body));
    p->SetAttribute("UseCheckbox", true);

    AddPanTiltTypeProperties(grid);
    AddColorTypeProperties(grid);
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

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
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
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE) {
            dmx_style = "Moving Head Side";
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS) {
            dmx_style = "Moving Head Bars";
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
            dmx_style = "Moving Head TopBars";
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
            dmx_style = "Moving Head SideBars";
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_3D) {
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
    }
    else if ("HideBody" == event.GetPropertyName()) {
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

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxMovingHead::InitModel() {
    DmxModel::InitModel();
    if (DisplayAs != "DmxMovingHead3D") {
        DisplayAs = "DmxMovingHead";
        screenLocation.SetRenderSize(1, 1);
    }
    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;
    hide_body = ModelXml->GetAttribute("HideBody", "False") == "True";
	dmx_style = ModelXml->GetAttribute("DmxStyle", "Moving Head Top");

    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "0"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "0"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "0"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
    
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
}

void DmxMovingHead::DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, float& sz, bool active, bool is_3d)
{
    static wxStopWatch sw;
    float angle, pan_angle, pan_angle_raw, tilt_angle, angle1, angle2, beam_length_displayed;
    int x1, x2, y1, y2;
    size_t NodeCount = Nodes.size();
    DrawGLUtils::xlAccumulator& va = is_3d ? va3 : va2;

    if (pan_channel > NodeCount ||
        tilt_channel > NodeCount ||
        red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount)
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor pnt_color(xlRED);
    xlColor beam_color(xlWHITE);
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

        xlColor proxy = xlBLACK;
        if (white_channel > 0)
        {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK)
        {
            Nodes[red_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            Nodes[green_channel - 1]->GetColor(proxy);
            beam_color.green = proxy.red;
            Nodes[blue_channel - 1]->GetColor(proxy);
            beam_color.blue = proxy.red;
        }
    }
    else if (white_channel > 0)
    {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        beam_color.green = proxy.red;
        beam_color.blue = proxy.red;
    }

    if (!active) {
        beam_color = xlWHITE;
    }
    else {
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
    long old_ms = 0;
    float rot_angle = (float)(((BoxedScreenLocation)screenLocation).GetRotation());

    std::vector<std::string> old_state = GetModelState();
    if (old_state.size() > 0 && active) {
        old_ms = std::atol(old_state[0].c_str());
        old_pan_angle = std::atof(old_state[1].c_str());
        old_tilt_angle = std::atof(old_state[2].c_str());
    }

    if (pan_channel > 0 && active) {
        Nodes[pan_channel - 1]->GetColor(color_angle);
        pan_angle = (color_angle.red / 255.0f) * pan_deg_of_rot + pan_orient;
    }
    else {
        pan_angle = pan_orient;
    }

    long ms = sw.Time();
    long time_delta = ms - old_ms;

    if (time_delta != 0 && old_state.size() > 0 && active) {
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

    pan_angle_raw = pan_angle;
    if (tilt_channel > 0 && active) {
        Nodes[tilt_channel - 1]->GetColor(color_angle);
        tilt_angle = (color_angle.red / 255.0f) * tilt_deg_of_rot + tilt_orient;
    }
    else {
        tilt_angle = tilt_orient;
    }

    if (time_delta != 0 && old_state.size() > 0 && active) {
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
    int tilt_pos = (int)(RenderBuffer::cos(ToRadians(tilt_angle)) * radius * 0.8);
    if (tilt_pos < 0) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        }
        else {
            pan_angle += 180.0f;
        }
        tilt_pos *= -1;
    }

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
        angle = pan_angle;
    }
    else {
        angle = tilt_angle;
    }

    // save the model state
    std::vector<std::string> state;
    state.push_back(std::to_string(ms));
    state.push_back(std::to_string(pan_angle_raw));
    state.push_back(std::to_string(tilt_angle));
    SaveModelState(state);

    float sf = 12.0f;
    float scale = radius / sf;

    int bars_deltax = (int)(scale * sf * 1.0f);
    int bars_deltay = (int)(scale * sf * 1.1f);

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
        scale /= 2.0f;
        tilt_pos /= 2;
    }

    beam_length_displayed = scale * sf * beam_length;
    angle1 = angle - beam_width / 2.0f;
    angle2 = angle + beam_width / 2.0f;
    if (angle1 < 0.0f) {
        angle1 += 360.0f;
    }
    if (angle2 > 360.f) {
        angle2 -= 360.0f;
    }
    x1 = (int)(RenderBuffer::cos(ToRadians(angle1)) * beam_length_displayed);
    y1 = (int)(RenderBuffer::sin(ToRadians(angle1)) * beam_length_displayed);
    x2 = (int)(RenderBuffer::cos(ToRadians(angle2)) * beam_length_displayed);
    y2 = (int)(RenderBuffer::sin(ToRadians(angle2)) * beam_length_displayed);

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        }
        else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    // Draw the light beam
    if (dmx_style_val != DMX_STYLE_MOVING_HEAD_BARS && dmx_style_val != DMX_STYLE_MOVING_HEAD_3D && shutter_open) {
        va.AddVertex(sx, sy, sz, beam_color);
        beam_color.alpha = 0;
        va.AddVertex(sx + x1, sy + y1, sz, beam_color);
        va.AddVertex(sx + x2, sy + y2, sz, beam_color);
    }

    if (!hide_body)
    {
        if (dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP || dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS) {
            va.AddTrianglesCircle(sx, sy, sz, scale * sf, ccolor, ccolor);

            // draw angle line
            dmxPoint p1(0, -1, sx, sy, scale, angle);
            dmxPoint p2(12, -1, sx, sy, scale, angle);
            dmxPoint p3(12, 1, sx, sy, scale, angle);
            dmxPoint p4(0, 1, sx, sy, scale, angle);

            va.AddVertex(p1.x, p1.y, sz, pnt_color);
            va.AddVertex(p2.x, p2.y, sz, pnt_color);
            va.AddVertex(p3.x, p3.y, sz, pnt_color);

            va.AddVertex(p1.x, p1.y, sz, pnt_color);
            va.AddVertex(p3.x, p3.y, sz, pnt_color);
            va.AddVertex(p4.x, p4.y, sz, pnt_color);

            // draw tilt marker
            dmxPoint marker(tilt_pos, 0, sx, sy, 1.0, angle);
            va.AddTrianglesCircle(marker.x, marker.y, sz, scale * sf * 0.22, black, black);
            va.AddTrianglesCircle(marker.x, marker.y, sz, scale * sf * 0.20, marker_color, marker_color);
        }
        else if (dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE || dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {
            // draw head
            dmxPoint p1(12, -13, sx, sy, scale, angle);
            dmxPoint p2(12, +13, sx, sy, scale, angle);
            dmxPoint p3(-12, +10, sx, sy, scale, angle);
            dmxPoint p4(-15, +5, sx, sy, scale, angle);
            dmxPoint p5(-15, -5, sx, sy, scale, angle);
            dmxPoint p6(-12, -10, sx, sy, scale, angle);

            va.AddVertex(p1.x, p1.y, sz, ccolor);
            va.AddVertex(p2.x, p2.y, sz, ccolor);
            va.AddVertex(p6.x, p6.y, sz, ccolor);

            va.AddVertex(p2.x, p2.y, sz, ccolor);
            va.AddVertex(p3.x, p3.y, sz, ccolor);
            va.AddVertex(p6.x, p6.y, sz, ccolor);

            va.AddVertex(p3.x, p3.y, sz, ccolor);
            va.AddVertex(p5.x, p5.y, sz, ccolor);
            va.AddVertex(p6.x, p6.y, sz, ccolor);

            va.AddVertex(p3.x, p3.y, sz, ccolor);
            va.AddVertex(p4.x, p4.y, sz, ccolor);
            va.AddVertex(p5.x, p5.y, sz, ccolor);

            // draw base
            va.AddTrianglesCircle(sx, sy, sz, scale * sf * 0.6, base_color, base_color);
            va.AddRect(sx - scale * sf * 0.6, sy, sx + scale * sf * 0.6, sy - scale * sf * 2, sz, base_color);

            // draw pan marker
            dmxPoint p7(7, 2, sx, sy, scale, pan_angle);
            dmxPoint p8(7, -2, sx, sy, scale, pan_angle);
            va.AddVertex(sx, sy, sz, marker_color);
            va.AddVertex(p7.x, p7.y, sz, marker_color);
            va.AddVertex(p8.x, p8.y, sz, marker_color);
        }
    }

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_TOP_BARS ||
        dmx_style_val == DMX_STYLE_MOVING_HEAD_SIDE_BARS) {

        if (dmx_style_val == DMX_STYLE_MOVING_HEAD_BARS) {
            bars_deltax = 0;
        }
        // draw the bars
        xlColor proxy;
        xlColor red(xlRED);
        xlColor green(xlGREEN);
        xlColor blue(xlBLUE);
        xlColor white(xlWHITE);
        xlColor pink(255, 51, 255);
        xlColor turqoise(64, 224, 208);
        ApplyTransparency(red, trans, trans);
        ApplyTransparency(green, trans, trans);
        ApplyTransparency(blue, trans, trans);
        ApplyTransparency(pink, trans, trans);
        ApplyTransparency(turqoise, trans, trans);
        int stepy = (int)(radius * 0.15f);
        int gapy = (int)(radius * 0.1f);
        if (gapy < 1) gapy = 1;
        va.AddRect(sx + bars_deltax - gapy - 2, sy + bars_deltay + gapy + 2, sx + bars_deltax + radius + gapy + 2, sy + bars_deltay - (stepy + gapy) * (NodeCount - 1) - stepy - gapy - 2, sz, ccolor);
        va.AddRect(sx + bars_deltax - gapy, sy + bars_deltay + gapy, sx + bars_deltax + radius + gapy, sy + bars_deltay - (stepy + gapy) * (NodeCount - 1) - stepy - gapy, sz, black);
        for (int i = 1; i <= NodeCount; ++i) {
            Nodes[i - 1]->GetColor(proxy);
            float val = (float)proxy.red;
            float offsetx = val / 255.0 * radius;
            if (i == pan_channel) {
                proxy = pink;
            }
            else if (i == tilt_channel) {
                proxy = turqoise;
            }
            else if (i == red_channel) {
                proxy = red;
            }
            else if (i == green_channel) {
                proxy = green;
            }
            else if (i == blue_channel) {
                proxy = blue;
            }
            else if (i == white_channel) {
                proxy = white;
            }
            else {
                proxy = ccolor;
            }
            va.AddRect(sx + bars_deltax, sy + bars_deltay - (stepy + gapy) * (i - 1), sx + bars_deltax + offsetx, sy + bars_deltay - (stepy + gapy) * (i - 1) - stepy, sz, proxy);
        }
    }

    if (dmx_style_val == DMX_STYLE_MOVING_HEAD_3D) {
        xlColor beam_color_end(beam_color);
        beam_color_end.alpha = 0;

        while (pan_angle_raw > 360.0f) pan_angle_raw -= 360.0f;
        pan_angle_raw = 360.0f - pan_angle_raw;
        bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

        float combined_angle = tilt_angle + rot_angle;
        if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
            if (shutter_open) {
                dmxPoint3 p1(beam_length_displayed, -5, -5, sx, sy, scale, pan_angle_raw, combined_angle);
                dmxPoint3 p2(beam_length_displayed, -5, 5, sx, sy, scale, pan_angle_raw, combined_angle);
                dmxPoint3 p3(beam_length_displayed, 5, -5, sx, sy, scale, pan_angle_raw, combined_angle);
                dmxPoint3 p4(beam_length_displayed, 5, 5, sx, sy, scale, pan_angle_raw, combined_angle);
                dmxPoint3 p0(0, 0, 0, sx, sy, scale, pan_angle_raw, combined_angle);


                if (facing_right) {
                    va.AddVertex(p2.x, p2.y, beam_color_end);
                    va.AddVertex(p4.x, p4.y, beam_color_end);
                    va.AddVertex(p0.x, p0.y, beam_color);
                }
                else {
                    va.AddVertex(p1.x, p1.y, beam_color_end);
                    va.AddVertex(p3.x, p3.y, beam_color_end);
                    va.AddVertex(p0.x, p0.y, beam_color);
                }

                va.AddVertex(p1.x, p1.y, beam_color_end);
                va.AddVertex(p2.x, p2.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);

                va.AddVertex(p3.x, p3.y, beam_color_end);
                va.AddVertex(p4.x, p4.y, beam_color_end);
                va.AddVertex(p0.x, p0.y, beam_color);

                if (!facing_right) {
                    va.AddVertex(p2.x, p2.y, beam_color_end);
                    va.AddVertex(p4.x, p4.y, beam_color_end);
                    va.AddVertex(p0.x, p0.y, beam_color);
                }
                else {
                    va.AddVertex(p1.x, p1.y, beam_color_end);
                    va.AddVertex(p3.x, p3.y, beam_color_end);
                    va.AddVertex(p0.x, p0.y, beam_color);
                }
            }
        }

        if (!hide_body)
        {
            if (facing_right) {
                Draw3DDMXBaseRight(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
                Draw3DDMXHead(va, base_color2, sx, sy, scale, pan_angle_raw, combined_angle);
                Draw3DDMXBaseLeft(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
            }
            else {
                Draw3DDMXBaseLeft(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
                Draw3DDMXHead(va, base_color2, sx, sy, scale, pan_angle_raw, combined_angle);
                Draw3DDMXBaseRight(va, base_color, sx, sy, scale, pan_angle_raw, rot_angle);
            }
        }
    }

    va.Finish(GL_TRIANGLES);
}

void DmxMovingHead::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    if (!IsActive()) return;

    float sz = 0;
    DrawGLUtils::xl3Accumulator dummy;
    DrawModel(preview, va, dummy, c, sx, sy, sz, active, false);
}

void DmxMovingHead::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    if (!IsActive()) return;

    DrawGLUtils::xlAccumulator dummy;
    DrawModel(preview, dummy, va, c, sx, sy, sz, active, true);
}

void DmxMovingHead::Draw3DDMXBaseLeft(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& rot_angle)
{
    dmxPoint3 p10(-3, -1, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p11(3, -1, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p12(-3, -5, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p13(3, -5, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p14(0, -1, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p15(-1, 1, -5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p16(1, 1, -5, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p14.x, p14.y, c);
    va.AddVertex(p16.x, p16.y, c);

    dmxPoint3 p210(-3, -1, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p211(3, -1, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p212(-3, -5, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p213(3, -5, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p214(0, -1, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p215(-1, 1, -3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p216(1, 1, -3, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p213.x, p213.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p214.x, p214.y, c);
    va.AddVertex(p216.x, p216.y, c);

    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p12.x, p12.y, c);
    va.AddVertex(p212.x, p212.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p210.x, p210.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p10.x, p10.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p15.x, p15.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p215.x, p215.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p11.x, p11.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p16.x, p16.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p216.x, p216.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p213.x, p213.y, c);
    va.AddVertex(p13.x, p13.y, c);
    va.AddVertex(p211.x, p211.y, c);
    va.AddVertex(p11.x, p11.y, c);
}

void DmxMovingHead::Draw3DDMXBaseRight(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& rot_angle)
{
    dmxPoint3 p20(-3, -1, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p21(3, -1, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p22(-3, -5, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p23(3, -5, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p24(0, -1, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p25(-1, 1, 5, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p26(1, 1, 5, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p24.x, p24.y, c);
    va.AddVertex(p26.x, p26.y, c);

    dmxPoint3 p220(-3, -1, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p221(3, -1, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p222(-3, -5, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p223(3, -5, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p224(0, -1, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p225(-1, 1, 3, sx, sy, scale, pan_angle, rot_angle);
    dmxPoint3 p226(1, 1, 3, sx, sy, scale, pan_angle, rot_angle);

    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p223.x, p223.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p224.x, p224.y, c);
    va.AddVertex(p226.x, p226.y, c);

    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p22.x, p22.y, c);
    va.AddVertex(p222.x, p222.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p220.x, p220.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p20.x, p20.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p25.x, p25.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p225.x, p225.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p21.x, p21.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p26.x, p26.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p226.x, p226.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p223.x, p223.y, c);
    va.AddVertex(p23.x, p23.y, c);
    va.AddVertex(p221.x, p221.y, c);
    va.AddVertex(p21.x, p21.y, c);
}

void DmxMovingHead::Draw3DDMXHead(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& tilt_angle)
{
    // draw the head
    float pan_angle1 = pan_angle + 270.0f;  // needs to be rotated from reference we drew it
    dmxPoint3 p31(-2, 3.45f, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p32(2, 3.45f, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p33(4, 0, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p34(2, -3.45f, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p35(-2, -3.45f, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p36(-4, 0, -4, sx, sy, scale, pan_angle1, 0, tilt_angle);

    dmxPoint3 p41(-1, 1.72f, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p42(1, 1.72f, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p43(2, 0, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p44(1, -1.72f, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p45(-1, -1.72f, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);
    dmxPoint3 p46(-2, 0, 4, sx, sy, scale, pan_angle1, 0, tilt_angle);

    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p35.x, p35.y, c);

    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p45.x, p45.y, c);

    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p42.x, p42.y, c);

    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p42.x, p42.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p32.x, p32.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p43.x, p43.y, c);

    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p43.x, p43.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p33.x, p33.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p44.x, p44.y, c);

    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p44.x, p44.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p34.x, p34.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p45.x, p45.y, c);

    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p45.x, p45.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p35.x, p35.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p46.x, p46.y, c);

    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p46.x, p46.y, c);
    va.AddVertex(p41.x, p41.y, c);
    va.AddVertex(p36.x, p36.y, c);
    va.AddVertex(p31.x, p31.y, c);
    va.AddVertex(p41.x, p41.y, c);
}

void DmxMovingHead::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    ExportBaseParameters(f);

    wxString s = ModelXml->GetAttribute("DmxStyle");
    wxString pdr = ModelXml->GetAttribute("DmxPanDegOfRot", "540");
    wxString tdr = ModelXml->GetAttribute("DmxTiltDegOfRot", "180");
    wxString pc = ModelXml->GetAttribute("DmxPanChannel", "0");
    wxString po = ModelXml->GetAttribute("DmxPanOrient", "0");
    wxString tc = ModelXml->GetAttribute("DmxTiltChannel", "0");
    wxString to = ModelXml->GetAttribute("DmxTiltOrient", "0");
    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    wxString sc = ModelXml->GetAttribute("DmxShutterChannel", "0");
    wxString so = ModelXml->GetAttribute("DmxShutterOpen", "1");
    wxString dbl = ModelXml->GetAttribute("DmxBeamLength", "1");
    wxString dbw = ModelXml->GetAttribute("DmxBeamWidth", "1");

    f.Write(wxString::Format("DmxStyle=\"%s\" ", s));
    f.Write(wxString::Format("DmxPanDegOfRot=\"%s\" ", pdr));
    f.Write(wxString::Format("DmxTiltDegOfRot=\"%s\" ", tdr));
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
    f.Write(wxString::Format("DmxBeamLength=\"%s\" ", dbl));
    f.Write(wxString::Format("DmxBeamWidth=\"%s\" ", dbw));

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

void DmxMovingHead::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
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

            wxString s = root->GetAttribute("DmxStyle");
            wxString pdr = root->GetAttribute("DmxPanDegOfRot");
            wxString tdr = root->GetAttribute("DmxTiltDegOfRot");
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
            wxString dbl = root->GetAttribute("DmxBeamLength", "1");
            wxString dbw = root->GetAttribute("DmxBeamWidth", "1");

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
            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);
            SetProperty("DmxShutterChannel", sc);
            SetProperty("DmxShutterOpen", so);
            SetProperty("DmxBeamLimit", bl);
            SetProperty("DmxBeamLength", dbl);
            SetProperty("DmxBeamWidth", dbw);

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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHead::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxMovingHead::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxMovingHead model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxMovingHead model file.");
    }
}
