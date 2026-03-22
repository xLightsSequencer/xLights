/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxColorAbilityWheel.h"
#include "../Model.h"
#include "../Node.h"
#include "../../Color.h"

DmxColorAbilityWheel::DmxColorAbilityWheel() :
    DmxColorAbility()
{
    _colorType = DMX_COLOR_TYPE::DMX_COLOR_WHEEL;
    InitColor();
};

void DmxColorAbilityWheel::InitColor()
{
    if (colors.empty()) {
        colors.emplace_back(xlWHITE, 10);
        colors.emplace_back(xlRED, 30);
        colors.emplace_back(xlGREEN, 50);
        colors.emplace_back(xlBLUE, 70);
    }
}

bool DmxColorAbilityWheel::IsColorChannel(uint32_t channel) const
{
    return (wheel_channel == channel || dimmer_channel == channel);
}

int DmxColorAbilityWheel::GetNumChannels() const
{
    int num_channels = 0;
    num_channels += wheel_channel > 0 ? 1 : 0;
    num_channels += dimmer_channel > 0 ? 1 : 0;
    return num_channels;
}

void DmxColorAbilityWheel::SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const
{
    if (auto const& colordata = GetDMXWheelValue(color); colordata) {
        if (CheckChannel(wheel_channel, pixelVector.size())) {
            pixelVector[wheel_channel - 1] = *colordata;
        }
        if (CheckChannel(dimmer_channel, pixelVector.size())) {
            HSVValue hsv = color.asHSV();
            int intensity = (hsv.value * 255.0);
            xlColor c(intensity, intensity, intensity);
            pixelVector[dimmer_channel - 1] = c;
        }
    }
}

std::list<std::string> DmxColorAbilityWheel::CheckModelSettings(Model *m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    if (wheel_channel > nodeCount) {
        res.push_back("    ERR: Model " + m->GetName() + " color wheel channel refers to a channel (" + std::to_string(wheel_channel) + ") not present on the model which only has " + std::to_string(nodeCount) + " channels.");
    }
    if (dimmer_channel > nodeCount) {
        res.push_back("    ERR: Model " + m->GetName() + " dimmer channel refers to a channel (" + std::to_string(dimmer_channel) + ") not present on the model which only has " + std::to_string(nodeCount) + " channels.");
    }
    return res;
}

bool DmxColorAbilityWheel::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    return (wheel_channel < nodeCount + 1 &&
            dimmer_channel < nodeCount + 1);
}

xlColor DmxColorAbilityWheel::GetBeamColor( const std::vector<NodeBaseClassPtr>& Nodes) const
{
    auto NodeCount = Nodes.size();
    xlColor beam_color(xlWHITE);
    if (CheckChannel(wheel_channel, NodeCount) && !colors.empty()) {
        xlColor proxy = xlBLACK;
        Nodes[wheel_channel - 1]->GetColor(proxy);

        if (auto const& colordata = GetWheelColorFromDMXValue(proxy); colordata) {
            beam_color = *colordata;
        }else {
             beam_color = xlBLACK;
        }

        if (CheckChannel(dimmer_channel, NodeCount)) {
            xlColor proxy;
            Nodes[dimmer_channel - 1]->GetColor(proxy);
            HSVValue hsv = proxy.asHSV();
            beam_color.red = (beam_color.red * hsv.value);
            beam_color.blue = (beam_color.blue * hsv.value);
            beam_color.green = (beam_color.green * hsv.value);
        }
    }
    return beam_color;
}

bool DmxColorAbilityWheel::ApplyChannelTransparency(xlColor& color, int transparency, uint32_t channel) const
{
    if (channel == wheel_channel) {
        color = xlWHITE;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == dimmer_channel) {
        color = xlWHITE;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    }
    return false;
}

void DmxColorAbilityWheel::GetColor(xlColor &color, int transparency, int blackTransparency, bool allowSelected,
                               const xlColor *c, const std::vector<NodeBaseClassPtr> &Nodes) const {
    xlColor beam_color(xlBLACK);
    if (c != nullptr) {
        beam_color = *c;
    } else if (!allowSelected) {
        if (CheckChannel(wheel_channel, Nodes.size())) {
            xlColor proxy;
            Nodes[wheel_channel - 1]->GetColor(proxy);
            if (auto const& colordata = GetWheelColorFromDMXValue(proxy); colordata) {
                beam_color = *colordata;
                if (CheckChannel(dimmer_channel, Nodes.size())) {
                    xlColor proxy;
                    Nodes[dimmer_channel - 1]->GetColor(proxy);

                    HSVValue hsv = proxy.asHSV();
                    beam_color.red = (beam_color.red * hsv.value);
                    beam_color.blue = (beam_color.blue * hsv.value);
                    beam_color.green = (beam_color.green * hsv.value);
                }
            }
        }
    }
    int trans = beam_color == xlBLACK ? blackTransparency : transparency;
    Model::ApplyTransparency(beam_color, trans, trans);
    color = beam_color;
}

[[nodiscard]] xlColor DmxColorAbilityWheel::GetColorPixels(xlColorVector const& pixelVector) const
{
    xlColor beam_color(xlBLACK);
    if (CheckChannel(wheel_channel, pixelVector.size())) {
        xlColor const& proxy = pixelVector[wheel_channel - 1];
        if (auto const& colordata = GetWheelColorFromDMXValue(proxy); colordata) {
            beam_color = *colordata;
            if (CheckChannel(dimmer_channel, pixelVector.size())) {
                xlColor const& proxy = pixelVector[dimmer_channel - 1];
                HSVValue hsv = proxy.asHSV();
                beam_color.red = (beam_color.red * hsv.value);
                beam_color.blue = (beam_color.blue * hsv.value);
                beam_color.green = (beam_color.green * hsv.value);
            }
        }
    }
    return beam_color;
}

void DmxColorAbilityWheel::SetNodeNames(std::vector<std::string>& names, const std::string &pfx) const
{
    if (CheckChannel(wheel_channel, names.size())) {
        names[wheel_channel - 1] = pfx + "Color Wheel";
    }
    if (CheckChannel(dimmer_channel, names.size())) {
        names[dimmer_channel - 1] = pfx + "Dimmer";
    }
}

void DmxColorAbilityWheel::AddColor(const std::string& dmxcolor, uint8_t dmxVal)
{
    colors.emplace_back(xlColor(dmxcolor), dmxVal);
}

std::optional<xlColor> DmxColorAbilityWheel::GetDMXWheelValue(xlColor const& color) const
{
    if (auto const found{ std::find_if(colors.begin(), colors.end(),
                                       [&color](auto const& col)
        {
            //return color == col.color;
            return (std::abs(color.asHSL().hue - col.color.asHSL().hue) < 0.01);
        }) };
        found != colors.end()) {
        uint8_t dmxV{ (*found).dmxValue };
        xlColor c(dmxV, dmxV, dmxV);
        return c;
    }
    return std::nullopt;
}

std::optional<xlColor> DmxColorAbilityWheel::GetWheelColorFromDMXValue(xlColor const& dmx) const
{
    // if the colour has a zero colour in the list then take colour lower than the specified value
    // If the colour has a 255 colour in the list then take the colour higher than the specified value
    // else assume the colour is the mid point so take the closest colour
    std::optional<xlColor> colour = std::nullopt;

    auto zero = std::find_if(colors.begin(), colors.end(),
                             [](auto const& col) { return col.dmxValue == 0; });

    if (zero != colors.end()) {

        int dmxValue = -1;

        // because colours may not be in order
        for (const auto& it : colors)
        {
            if (it.dmxValue <= dmx.Red() && it.dmxValue > dmxValue)
            {
                colour = it.color;
                dmxValue = it.dmxValue;
            }
        }
    } else {
        auto twofivefive = std::find_if(colors.begin(), colors.end(),
                                 [](auto const& col) { return col.dmxValue == 255; });

        if (twofivefive != colors.end()) {

            int dmxValue = 256;

            // because colours may not be in order
            for (const auto& it : colors) {
                if (it.dmxValue >= dmx.Red() && it.dmxValue < dmxValue) {
                    colour = it.color;
                    dmxValue = it.dmxValue;
                }
            }   
        }
        else
        {
            // because colours may not be in order ... we need to sort them to be safe
            auto temp = colors;
            std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b) { return a.dmxValue < b.dmxValue; });

            for (size_t i = 0; i < temp.size(); ++i)
            {
                // anything less than the first colour value is that colour
                if (i == 0 && dmx.Red() <= temp[i].dmxValue)
                {
                    colour = temp[i].color;
                    break;
                }
                // anything greater than the last colour value is that colour
                else if (i == temp.size() - 1)
                {
                    colour = temp[i].color;
                    break;
                }
                else
                {
                    if (dmx.Red() >= temp[i].dmxValue && dmx.Red() <= temp[i + 1].dmxValue)
                    {
                        if (dmx.Red() <= temp[i].dmxValue + (temp[i+1].dmxValue - temp[i].dmxValue) / 2)
                        {
                            colour = temp[i].color;
                        }
                        else
                        {
                            colour = temp[i+1].color;
                        }
                        break;
                    }
                }
            }
        }
    }

    return colour;
}

xlColorVector DmxColorAbilityWheel::GetColors() const
{
    xlColorVector colorset;
    for (auto const& col : colors) {
        colorset.push_back( col.color );
    }
    return colorset;
}

int DmxColorAbilityWheel::GetDMXWheelIndex(xlColor const& color) const {
    if (auto const found{ std::find_if(colors.begin(), colors.end(),
                                       [&color](auto const& col) {
                                           // return color == col.color;
                                           return (std::abs(color.asHSL().hue - col.color.asHSL().hue) < 0.01);
                                       }) };
        found != colors.end()) {
        size_t index = std::distance(colors.begin(), found);
        if (index != colors.size()) {
            return index;
        }
    }
    return -1;
}


void DmxColorAbilityWheel::GetPWMOutputs(std::map<uint32_t, PWMOutput> &map) const {
    if (wheel_channel > 0) {
        map[wheel_channel] = PWMOutput(wheel_channel, PWMOutput::Type::LED, 1, "Wheel");
    }
    if (dimmer_channel > 0) {
        map[dimmer_channel] = PWMOutput(dimmer_channel, PWMOutput::Type::LED, 1, "Dimmer");
    }
}
