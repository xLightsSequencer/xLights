/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <spdlog/fmt/fmt.h>

#include "DmxColorAbilityRGB.h"
#include "../Model.h"


#include "../Node.h"

DmxColorAbilityRGB::DmxColorAbilityRGB() :
    DmxColorAbility()
{
    _colorType = DMX_COLOR_TYPE::DMX_COLOR_RGBW;
};

void DmxColorAbilityRGB::InitColor()
{
}

bool DmxColorAbilityRGB::IsColorChannel(uint32_t channel) const
{
    return (red_channel == channel || green_channel == channel || blue_channel == channel || white_channel == channel);
}

int DmxColorAbilityRGB::GetNumChannels() const
{
    int num_channels = 0;
    num_channels += red_channel > 0 ? 1 : 0;
    num_channels += green_channel > 0 ? 1 : 0;
    num_channels += blue_channel > 0 ? 1 : 0;
    num_channels += white_channel > 0 ? 1 : 0;
    return num_channels;
}

void DmxColorAbilityRGB::SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const
{
    xlColor c;
    if (CheckChannel(white_channel, pixelVector.size())
        && color.red == color.green && color.red == color.blue) {
        c.red = color.red;
        c.green = color.red;
        c.blue = color.red;
        pixelVector[white_channel - 1] = c;
    } else {
        if (CheckChannel( red_channel , pixelVector.size())) {
            c.red = color.red;
            c.green = color.red;
            c.blue = color.red;
            pixelVector[red_channel - 1] = c;
        }
        if (CheckChannel( green_channel , pixelVector.size())) {
            c.red = color.green;
            c.green = color.green;
            c.blue = color.green;
            pixelVector[green_channel - 1] = c;
        }
        if (CheckChannel( blue_channel, pixelVector.size() )) {
            c.red = color.blue;
            c.green = color.blue;
            c.blue = color.blue;
            pixelVector[blue_channel - 1] = c;
        }
    }
}

std::list<std::string> DmxColorAbilityRGB::CheckModelSettings(Model *m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    if (red_channel > nodeCount) {
        res.push_back(fmt::format("    ERR: Model {} red channel refers to a channel ({}) not present on the model which only has {} channels.", m->GetName(), red_channel, nodeCount));
    }
    if (green_channel > nodeCount) {
        res.push_back(fmt::format("    ERR: Model {} green channel refers to a channel ({}) not present on the model which only has {} channels.", m->GetName(), green_channel, nodeCount));
    }
    if (blue_channel > nodeCount) {
        res.push_back(fmt::format("    ERR: Model {} blue channel refers to a channel ({}) not present on the model which only has {} channels.", m->GetName(), blue_channel, nodeCount));
    }
    if (white_channel > nodeCount) {
        res.push_back(fmt::format("    ERR: Model {} white channel refers to a channel ({}) not present on the model which only has {} channels.", m->GetName(), white_channel, nodeCount));
    }
    return res;
}

bool DmxColorAbilityRGB::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    return (red_channel < nodeCount + 1 &&
        green_channel < nodeCount + 1 &&
        blue_channel < nodeCount + 1 &&
        white_channel < nodeCount + 1);
}

xlColor DmxColorAbilityRGB::GetBeamColor( const std::vector<NodeBaseClassPtr>& Nodes) const
{
    auto NodeCount = Nodes.size();
    xlColor beam_color(xlWHITE);
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy = xlBLACK;
        if (CheckChannel(white_channel, NodeCount)) {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK) {
            if (CheckChannel(red_channel, NodeCount)) {
                Nodes[red_channel - 1]->GetColor(proxy);
                beam_color.red = proxy.red;
            }
            if (CheckChannel(green_channel, NodeCount)) {
                Nodes[green_channel - 1]->GetColor(proxy);
                beam_color.green = proxy.red;
            }
            if (CheckChannel(blue_channel, NodeCount)) {
                Nodes[blue_channel - 1]->GetColor(proxy);
                beam_color.blue = proxy.red;
            }
        }
    } else if (CheckChannel(white_channel, NodeCount)) {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        beam_color.green = proxy.red;
        beam_color.blue = proxy.red;
    }
    return beam_color;
}

bool DmxColorAbilityRGB::ApplyChannelTransparency(xlColor& color,int transparency, uint32_t channel) const
{
    if (channel == red_channel) {
        color = xlRED;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == green_channel) {
        color = xlGREEN;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == blue_channel) {
        color = xlBLUE;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == white_channel) {
        color = xlWHITE;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    }
    return false;
}

void DmxColorAbilityRGB::GetColor(xlColor &color, int transparency, int blackTransparency, bool allowSelected,
                               const xlColor *c, const std::vector<NodeBaseClassPtr> &Nodes) const {
    xlColor beam_color(xlWHITE);
    auto NodeCount = Nodes.size();
    if (c != nullptr) {
        beam_color = *c;
    } else if (!allowSelected) {
        if (CheckChannel(red_channel, NodeCount) &&
            CheckChannel(blue_channel, NodeCount) &&
            CheckChannel(green_channel, NodeCount)) {
            xlColor proxy = xlBLACK;
            if (CheckChannel(white_channel , NodeCount)) {
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
        else if (CheckChannel(white_channel,NodeCount)) {
            xlColor proxy;
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            beam_color.green = proxy.red;
            beam_color.blue = proxy.red;
        }
    }
    int trans = beam_color == xlBLACK ? blackTransparency : transparency;
    Model::ApplyTransparency(beam_color, trans, trans);
    color = beam_color;
}

[[nodiscard]] xlColor DmxColorAbilityRGB::GetColorPixels(xlColorVector const& pixelVector ) const
{
    auto NodeCount = pixelVector.size();
    xlColor beam_color( xlBLACK );
    if (CheckChannel(red_channel, NodeCount) &&
        CheckChannel(blue_channel, NodeCount) &&
        CheckChannel(green_channel, NodeCount)) {
        if (CheckChannel(white_channel , NodeCount)) {
            beam_color = pixelVector[white_channel - 1];
        }

        if (beam_color == xlBLACK) {
            beam_color.red = pixelVector[red_channel - 1].red;
            beam_color.green = pixelVector[green_channel - 1].red;
            beam_color.blue = pixelVector[blue_channel - 1].red;
        }
    } else if (CheckChannel(white_channel , NodeCount)) {
        beam_color.red = pixelVector[white_channel - 1].red;
        beam_color.green = pixelVector[white_channel - 1].red;
        beam_color.blue = pixelVector[white_channel - 1].red;
    }
    return beam_color;
}

void DmxColorAbilityRGB::SetNodeNames(std::vector<std::string>& names, const std::string &pfx) const
{
    if (CheckChannel(red_channel , names.size())) {
        names[red_channel - 1] = pfx + "Red";
    }
    if (CheckChannel( blue_channel , names.size())) {
        names[blue_channel - 1] = pfx + "Blue";
    }
    if (CheckChannel( green_channel , names.size())) {
        names[green_channel - 1] = pfx + "Green";
    }
    if (CheckChannel( white_channel , names.size())) {
        names[white_channel - 1] = pfx + "White";
    }
}

void DmxColorAbilityRGB::GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const {
    if (red_channel > 0) {
        map[red_channel] = PWMOutput(red_channel, PWMOutput::Type::LED, 1, "Red", red_brightness, red_gamma);
    }
    if (green_channel > 0) {
        map[green_channel] = PWMOutput(green_channel, PWMOutput::Type::LED, 1, "Green", green_brightness, green_gamma);
    }
    if (blue_channel > 0) {
        map[blue_channel] = PWMOutput(blue_channel, PWMOutput::Type::LED, 1, "Blue", blue_brightness, blue_gamma);
    }
    if (white_channel > 0) {
        map[white_channel] = PWMOutput(white_channel, PWMOutput::Type::LED, 1, "White", white_brightness, white_gamma);
    }
}
