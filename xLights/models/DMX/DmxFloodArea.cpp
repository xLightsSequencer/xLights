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

#include "DmxFloodArea.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodArea::DmxFloodArea(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxFloodlight(node, manager, zeroBased)
{
    color_ability = this;
}

DmxFloodArea::~DmxFloodArea()
{
    //dtor
}

void DmxFloodArea::InitModel() {
    DmxFloodlight::InitModel();
    DisplayAs = "DmxFloodArea";
}

void DmxFloodArea::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    if (!IsActive()) return;

    // determine if shutter is open for floods that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= Nodes.size() && active) {
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

    size_t NodeCount = Nodes.size();

    if (red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount)
    {
        return;
    }

    xlColor color;
    xlColor ecolor;
    xlColor beam_color(xlWHITE);
    if (c != nullptr) {
        color = *c;
        ecolor = *c;
    }

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy = xlBLACK;
        if (white_channel > 0) {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK) {
            Nodes[red_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            Nodes[green_channel - 1]->GetColor(proxy);
            beam_color.green = proxy.red;
            Nodes[blue_channel - 1]->GetColor(proxy);
            beam_color.blue = proxy.red;
        }
    }
    else if (white_channel > 0) {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        beam_color.green = proxy.red;
        beam_color.blue = proxy.red;
    }

    if (!active) {
        beam_color = color;
    }

    ApplyTransparency(beam_color, trans, trans);
    if (pixelStyle == 2) {
        ecolor = beam_color;
    }
    else {
        ecolor.alpha = 0;
    }

    float rw = ((BoxedScreenLocation)screenLocation).GetMWidth();
    float rh = ((BoxedScreenLocation)screenLocation).GetMHeight();

    if (shutter_open) {
        float halfwidth = rw / 2.0f;
        float halfheight = rh / 2.0f;
        glm::vec4 top_lt_corner = glm::vec4(sx-halfwidth, sy+halfheight, 0.0f, 1.0f);
        glm::vec4 top_rt_corner = glm::vec4(sx+halfwidth, sy+halfheight, 0.0f, 1.0f);
        glm::vec4 bot_lt_corner = glm::vec4(sx-halfwidth, sy-halfheight, 0.0f, 1.0f);
        glm::vec4 bot_rt_corner = glm::vec4(sx+halfwidth, sy-halfheight, 0.0f, 1.0f);
        va.AddVertex(top_lt_corner.x, top_lt_corner.y, ecolor);
        va.AddVertex(bot_lt_corner.x, bot_lt_corner.y, ecolor);
        va.AddVertex(sx, sy, beam_color);
        va.AddVertex(top_rt_corner.x, top_rt_corner.y, ecolor);
        va.AddVertex(bot_rt_corner.x, bot_rt_corner.y, ecolor);
        va.AddVertex(sx, sy, beam_color);
        va.AddVertex(top_lt_corner.x, top_lt_corner.y, ecolor);
        va.AddVertex(top_rt_corner.x, top_rt_corner.y, ecolor);
        va.AddVertex(sx, sy, beam_color);
        va.AddVertex(bot_lt_corner.x, bot_lt_corner.y, ecolor);
        va.AddVertex(bot_rt_corner.x, bot_rt_corner.y, ecolor);
        va.AddVertex(sx, sy, beam_color);
        va.Finish(GL_TRIANGLES);
    }
}

void DmxFloodArea::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active)
{
    if (!IsActive()) return;

    // determine if shutter is open for floods that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= Nodes.size() && active) {
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

    size_t NodeCount = Nodes.size();

    if (red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount) {
        return;
    }

    xlColor color;
    xlColor ecolor;
    xlColor beam_color(xlWHITE);
    if (c != nullptr) {
        color = *c;
        ecolor = *c;
    }

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {

        xlColor proxy = xlBLACK;
        if (white_channel > 0) {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK) {
            Nodes[red_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            Nodes[green_channel - 1]->GetColor(proxy);
            beam_color.green = proxy.red;
            Nodes[blue_channel - 1]->GetColor(proxy);
            beam_color.blue = proxy.red;
        }
    }
    else if (white_channel > 0) {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        beam_color.green = proxy.red;
        beam_color.blue = proxy.red;
    }

    if (!active) {
        beam_color = color;
    }

    ApplyTransparency(beam_color, trans, trans);
    if (pixelStyle == 2) {
        ecolor = beam_color;
    }
    else {
        ecolor.alpha = 0;
    }

    float rw = ((BoxedScreenLocation)screenLocation).GetMWidth();
    float rh = ((BoxedScreenLocation)screenLocation).GetMHeight();
    glm::quat rotation = GetModelScreenLocation().GetRotationQuat();

    if (shutter_open) {
        float halfwidth = rw / 2.0f;
        float halfheight = rh / 2.0f;
        auto dist = std::min(halfwidth, halfheight) * beam_length;
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-sx, -sy, -sz));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(sx, sy, sz));
        glm::mat4 RotationMatrix = glm::toMat4(rotation);
        glm::vec4 top_lt_corner = glm::vec4(sx - halfwidth, sy + halfheight, sz + dist, 1.0f);
        glm::vec4 top_rt_corner = glm::vec4(sx + halfwidth, sy + halfheight, sz + dist, 1.0f);
        glm::vec4 bot_lt_corner = glm::vec4(sx - halfwidth, sy - halfheight, sz + dist, 1.0f);
        glm::vec4 bot_rt_corner = glm::vec4(sx + halfwidth, sy - halfheight, sz + dist, 1.0f);
        top_lt_corner = translateBack * RotationMatrix * translateToOrigin * top_lt_corner;
        top_rt_corner = translateBack * RotationMatrix * translateToOrigin * top_rt_corner;
        bot_lt_corner = translateBack * RotationMatrix * translateToOrigin * bot_lt_corner;
        bot_rt_corner = translateBack * RotationMatrix * translateToOrigin * bot_rt_corner;
        va.AddVertex(top_lt_corner.x, top_lt_corner.y, top_lt_corner.z, ecolor);
        va.AddVertex(bot_lt_corner.x, bot_lt_corner.y, bot_lt_corner.z, ecolor);
        va.AddVertex(sx, sy, sz, beam_color);
        va.AddVertex(top_rt_corner.x, top_rt_corner.y, top_rt_corner.z, ecolor);
        va.AddVertex(bot_rt_corner.x, bot_rt_corner.y, bot_rt_corner.z, ecolor);
        va.AddVertex(sx, sy, sz, beam_color);
        va.AddVertex(top_lt_corner.x, top_lt_corner.y, top_lt_corner.z, ecolor);
        va.AddVertex(top_rt_corner.x, top_rt_corner.y, top_rt_corner.z, ecolor);
        va.AddVertex(sx, sy, sz, beam_color);
        va.AddVertex(bot_lt_corner.x, bot_lt_corner.y, bot_lt_corner.z, ecolor);
        va.AddVertex(bot_rt_corner.x, bot_rt_corner.y, bot_rt_corner.z, ecolor);
        va.AddVertex(sx, sy, sz, beam_color);
        va.Finish(GL_TRIANGLES);
    }
}
