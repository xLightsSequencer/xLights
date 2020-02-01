#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxMovingHead3D.h"
#include "Mesh.h"

#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxMovingHead3D::DmxMovingHead3D(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
  : DmxMovingHead(node, manager, zeroBased)
{
    wxStandardPaths stdp = wxStandardPaths::Get();
#ifndef __WXMSW__
    obj_path = wxStandardPaths::Get().GetResourcesDir() + "/meshobjects/MovingHead3D/";
#else
    obj_path = wxFileName(stdp.GetExecutablePath()).GetPath() + "/meshobjects/MovingHead3D/";
#endif
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxMovingHead3D::~DmxMovingHead3D()
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

static wxPGChoices DMX_STYLES;

void DmxMovingHead3D::InitModel() {
    DisplayAs = "DmxMovingHead3D";
    DmxMovingHead::InitModel();

    if (base_node == nullptr) {
        wxXmlNode* n = ModelXml->GetChildren();
        while (n != nullptr) {
            std::string name = n->GetName();
            if ("BaseMesh" == name) {
                base_node = n;
            }
            else if ("HeadMesh" == name) {
                head_node = n;
            }
            n = n->GetNext();
        }
    }

    // create any missing node
    if (base_node == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "BaseMesh");
        ModelXml->AddChild(new_node);
        base_node = new_node;
    }
    if (head_node == nullptr) {
        wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "HeadMesh");
        ModelXml->AddChild(new_node);
        head_node = new_node;
    }

    if (base_mesh == nullptr) {
        if (base_node->HasAttribute("ObjFile")) {
            base_node->DeleteAttribute("ObjFile");
        }
        wxString f = obj_path + "MovingHead3D_Base.obj";
        base_node->AddAttribute("ObjFile", f);
        base_mesh = new Mesh(base_node, "BaseMesh");
        base_mesh->Init(this, true);
    }
    if (head_mesh == nullptr) {
        wxString f = obj_path + "MovingHead3D_Head.obj";
        if (head_node->HasAttribute("ObjFile")) {
            head_node->DeleteAttribute("ObjFile");
        }
        head_node->AddAttribute("ObjFile", f);
        head_mesh = new Mesh(head_node, "HeadMesh");
        head_mesh->Init(this, false);
    }
}

void DmxMovingHead3D::DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, float& sz, bool active, bool is_3d)
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
        white_channel > NodeCount ||
        shutter_channel > NodeCount)
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

    angle = tilt_angle;

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

    float beam_width = 30.0f;
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

    xlColor beam_color_end(beam_color);
    beam_color_end.alpha = 0;

    while (pan_angle_raw > 360.0f) pan_angle_raw -= 360.0f;
    pan_angle_raw = 360.0f - pan_angle_raw;
    bool facing_right = pan_angle_raw <= 90.0f || pan_angle_raw >= 270.0f;

    float combined_angle = tilt_angle + rot_angle;
    if (beam_color.red != 0 || beam_color.green != 0 || beam_color.blue != 0) {
        if (shutter_open) {
            dmxPoint3d p1(beam_length_displayed, -5, -5, sx, sy, sz, scale, pan_angle_raw, combined_angle);
            dmxPoint3d p2(beam_length_displayed, -5, 5, sx, sy, sz, scale, pan_angle_raw, combined_angle);
            dmxPoint3d p3(beam_length_displayed, 5, -5, sx, sy, sz, scale, pan_angle_raw, combined_angle);
            dmxPoint3d p4(beam_length_displayed, 5, 5, sx, sy, sz, scale, pan_angle_raw, combined_angle);
            dmxPoint3d p0(0, 0, 0, sx, sy, sz, scale, pan_angle_raw, combined_angle);

            if (facing_right) {
                va.AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                va.AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                va.AddVertex(p0.x, p0.y, p0.z, beam_color);
            }
            else {
                va.AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                va.AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                va.AddVertex(p0.x, p0.y, p0.z, beam_color);
            }

            va.AddVertex(p1.x, p1.y, p1.z, beam_color_end);
            va.AddVertex(p2.x, p2.y, p2.z, beam_color_end);
            va.AddVertex(p0.x, p0.y, p0.z, beam_color);

            va.AddVertex(p3.x, p3.y, p3.z, beam_color_end);
            va.AddVertex(p4.x, p4.y, p4.z, beam_color_end);
            va.AddVertex(p0.x, p0.y, p0.z, beam_color);

            if (!facing_right) {
                va.AddVertex(p2.x, p2.y, p2.z, beam_color_end);
                va.AddVertex(p4.x, p4.y, p4.z, beam_color_end);
                va.AddVertex(p0.x, p0.y, p0.z, beam_color);
            }
            else {
                va.AddVertex(p1.x, p1.y, p1.z, beam_color_end);
                va.AddVertex(p3.x, p3.y, p3.z, beam_color_end);
                va.AddVertex(p0.x, p0.y, p0.z, beam_color);
            }
        }
    }

    if (!hide_body)
    {
        if (is_3d) {
            glm::mat4 Identity = glm::mat4(1.0f);
            glm::vec3 scaling = GetModelScreenLocation().GetScaleMatrix();
            scaling.z = scaling.x;
            glm::mat4 scalingMatrix = glm::scale(Identity, scaling);
            glm::vec3 world = GetModelScreenLocation().GetWorldPosition();
            glm::mat4 translateMatrix = glm::translate(Identity, world);
            glm::quat rotateQuat = GetModelScreenLocation().GetRotationQuat();
            glm::mat4 base_matrix = translateMatrix * glm::toMat4(rotateQuat) * scalingMatrix;
            glm::mat4 base_motion_matrix = glm::rotate(Identity, glm::radians(pan_angle_raw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 head_motion_matrix = base_motion_matrix * glm::rotate(Identity, glm::radians(tilt_angle), glm::vec3(0.0f, 0.0f, 1.0f));

            // Draw Meshs
            base_mesh->Draw(this, preview, va3, base_matrix, base_motion_matrix, false, 0, 0, 0, false, false);
            head_mesh->Draw(this, preview, va3, base_matrix, head_motion_matrix, false, 0, 0, 0, true, false);
        }
        else
        {
            float x1 = -0.5f;
            float x2 = -0.5f;
            float x3 = 0.5f;
            float x4 = 0.5f;
            float y1 = -0.5f;
            float y2 = 0.5f;
            float y3 = 0.5f;
            float y4 = -0.5f;
            float z1 = 0.0f;
            float z2 = 0.0f;
            float z3 = 0.0f;
            float z4 = 0.0f;

            GetBaseObjectScreenLocation().TranslatePoint(x1, y1, z1);
            GetBaseObjectScreenLocation().TranslatePoint(x2, y2, z2);
            GetBaseObjectScreenLocation().TranslatePoint(x3, y3, z3);
            GetBaseObjectScreenLocation().TranslatePoint(x4, y4, z4);

            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
        }
    }

    va.Finish(GL_TRIANGLES);
}

void DmxMovingHead3D::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    float sz = 0;
    DrawGLUtils::xl3Accumulator dummy;
    DrawModel(preview, va, dummy, c, sx, sy, sz, active, false);
}

void DmxMovingHead3D::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    DrawGLUtils::xlAccumulator dummy;
    DrawModel(preview, dummy, va, c, sx, sy, sz, active, true);
}

void DmxMovingHead3D::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    DmxMovingHead::ImportXlightsModel(filename, xlights, min_x, max_x, min_y, max_y);

    SetProperty("DisplayAs", "DmxMovingHead3D");
}
