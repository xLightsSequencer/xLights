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
#include <wx/stdpaths.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxMovingHead3D.h"
#include "DmxPresetAbility.h"
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
    SetFromXml(node, zeroBased);
}

DmxMovingHead3D::~DmxMovingHead3D()
{
    //dtor
}

#define ToRadians(x) ((double)x * PI / (double)180.0)

void DmxMovingHead3D::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    wxPGProperty* p = grid->GetPropertyByName("DmxStyle");
    if (p != nullptr) {
        p->Hide(true);
    }

    DmxModel::DisableUnusedProperties(grid);
}
void DmxMovingHead3D::SetFromXml(wxXmlNode* ModelNode, bool zb) {
    base_node = nullptr;
    head_node = nullptr;
    if (base_mesh) {
        delete base_mesh;
        base_mesh = nullptr;
    }
    if (head_mesh) {
        delete head_mesh;
        head_mesh = nullptr;
    }
    DmxMovingHead::SetFromXml(ModelNode, zb);
}
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
    float w = 1;
    float h = 1;
    float d = 1;

    if (base_mesh == nullptr) {
        if (base_node->HasAttribute("ObjFile")) {
            base_node->DeleteAttribute("ObjFile");
        }
        wxString f = obj_path + "MovingHead3D_Base.obj";
        base_node->AddAttribute("ObjFile", f);
        base_mesh = new Mesh(base_node, "BaseMesh");
        base_mesh->Init(this, true);
        
        w = std::max(w, screenLocation.GetRenderWi());
        h = std::max(h, screenLocation.GetRenderHt());
        d = std::max(d, screenLocation.GetRenderDp());
    }
    if (head_mesh == nullptr) {
        wxString f = obj_path + "MovingHead3D_Head.obj";
        if (head_node->HasAttribute("ObjFile")) {
            head_node->DeleteAttribute("ObjFile");
        }
        head_node->AddAttribute("ObjFile", f);
        head_mesh = new Mesh(head_node, "HeadMesh");
        head_mesh->Init(this, true);
        w = std::max(w, screenLocation.GetRenderWi());
        h = std::max(h, screenLocation.GetRenderHt());
        d = std::max(d, screenLocation.GetRenderDp());
    }
    screenLocation.SetRenderSize(w, h, d);
}

std::list<std::string> DmxMovingHead3D::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();

    if (pan_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s pan channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), pan_channel, nodeCount));
    }
    if (tilt_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s tilt channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), tilt_channel, nodeCount));
    }
    if (shutter_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s shutter channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), shutter_channel, nodeCount));
    }

    res.splice(res.end(), DmxModel::CheckModelSettings());
    return res;
}

void DmxMovingHead3D::DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor* c) {
    static wxStopWatch sw;
    float pan_angle, pan_angle_raw, tilt_angle, beam_length_displayed; //, angle1, angle2
    //int x1, x2, y1, y2;
    size_t NodeCount = Nodes.size();

    if (!color_ability->IsValidModelSettings(this) || pan_channel > NodeCount ||
        !preset_ability->IsValidModelSettings(this) ||
        tilt_channel > NodeCount ||
        shutter_channel > NodeCount)
    {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
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

    color_ability->GetColor(beam_color, transparency, blackTransparency, !active, c, Nodes);
    
    int trans = color == xlBLACK ? blackTransparency : transparency;
    if (!active) {
        beam_color = xlWHITE;
    } else {
        marker_color = beam_color;
    }
    ApplyTransparency(ccolor, trans, trans);
    ApplyTransparency(base_color, trans, trans);
    ApplyTransparency(base_color2, trans, trans);
    ApplyTransparency(pnt_color, trans, trans);

    xlColor color_angle;

    // retrieve the model state
    float old_pan_angle = 0.0f;
    float old_tilt_angle = 0.0f;
    long old_ms = 0;

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
    } else {
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
    int tilt_pos = (int)(RenderBuffer::cos(ToRadians(tilt_angle)));
    if (tilt_pos < 0) {
        if (pan_angle >= 180.0f) {
            pan_angle -= 180.0f;
        } else {
            pan_angle += 180.0f;
        }
    }

    // save the model state
    std::vector<std::string> state;
    state.push_back(std::to_string(ms));
    state.push_back(std::to_string(pan_angle_raw));
    state.push_back(std::to_string(tilt_angle));
    SaveModelState(state);

    beam_length_displayed = beam_length;
    
    float scw = screenLocation.GetRenderWi() * screenLocation.GetScaleX();
    float sch = screenLocation.GetRenderHt() * screenLocation.GetScaleY();
    float scd = screenLocation.GetRenderDp() * screenLocation.GetScaleZ();
    float sbl = std::max(scw, std::max(sch, scd));
    beam_length_displayed *= sbl;

    // determine if shutter is open for heads that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        } else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }
    
    while (pan_angle_raw > 360.0f) pan_angle_raw -= 360.0f;
    pan_angle_raw = 360.0f - pan_angle_raw;


    auto vac = tprogram->getAccumulator();
    int start = vac->getCount();
    Draw3DBeam(vac, beam_color, beam_length_displayed, pan_angle_raw, tilt_angle, shutter_open);
    int end = vac->getCount();
    tprogram->addStep([=](xlGraphicsContext *ctx) {
        ctx->drawTriangles(vac, start, end - start);
    });
    if (!hide_body) {
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 base_motion_matrix = glm::rotate(Identity, glm::radians(pan_angle_raw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 head_motion_matrix = base_motion_matrix * glm::rotate(Identity, glm::radians(tilt_angle), glm::vec3(0.0f, 0.0f, 1.0f));

        // Draw Meshs
        base_mesh->Draw(this, preview, sprogram, tprogram, Identity, base_motion_matrix, false);
        head_mesh->Draw(this, preview, sprogram, tprogram, Identity, head_motion_matrix, false);
    }
}

void DmxMovingHead3D::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) {

    DmxMovingHead::ImportXlightsModel(root, xlights, min_x, max_x, min_y, max_y);

    SetProperty("DisplayAs", "DmxMovingHead3D");
}
