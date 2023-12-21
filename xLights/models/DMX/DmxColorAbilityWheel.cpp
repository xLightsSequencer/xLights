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

#include "DmxColorAbilityWheel.h"
#include "../BaseObject.h"
#include "../Model.h"
#include "../Node.h"
#include "../../Color.h"

constexpr int MAX_COLORS{25};

void DmxColorAbilityWheel::InitColor( wxXmlNode* ModelXml)
{
    wheel_channel = wxAtoi(ModelXml->GetAttribute("DmxColorWheelChannel", "1"));
    dimmer_channel = wxAtoi(ModelXml->GetAttribute("DmxDimmerChannel", "2"));

    ReadColorSettings(ModelXml);

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
        res.push_back(wxString::Format("    ERR: Model %s color wheel channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), wheel_channel, nodeCount));
    }
    if (dimmer_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s dimmer channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), dimmer_channel, nodeCount));
    }
    return res;
}

bool DmxColorAbilityWheel::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    return (wheel_channel < nodeCount + 1 &&
            dimmer_channel < nodeCount + 1);
}

void DmxColorAbilityWheel::AddColorTypeProperties(wxPropertyGridInterface *grid) const {

    wxPGProperty* p = grid->Append(new wxUIntProperty("Color Wheel Channel", "DmxColorWheelChannel", wheel_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Dimmer Channel", "DmxDimmerChannel", dimmer_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Color Wheel Size", "DmxColorWheelSize", colors.size()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", MAX_COLORS);
    p->SetEditor("SpinCtrl");

    int index = 0;
    for (auto const& col : colors) {
        grid->AppendIn(p,
                       new wxColourProperty(wxString::Format("Color %d", 1 + index),
                                            wxString::Format("DmxColorWheelColor%d", index), col.color.asWxColor()));
        auto sp = grid->AppendIn(p,
            new wxUIntProperty(wxString::Format("Color %d DMX", 1 + index),
                wxString::Format("DmxColorWheelDMX%d", index), col.dmxValue));
        sp->SetAttribute("Min", 0);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");

        ++index;
    }
}

int DmxColorAbilityWheel::OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

    if ("DmxColorWheelChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxColorWheelChannel");
        ModelXml->AddAttribute("DmxColorWheelChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelChannel");
        return 0;
    } else if ("DmxDimmerChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxDimmerChannel");
        ModelXml->AddAttribute("DmxDimmerChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxDimmerChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxDimmerChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxDimmerChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxDimmerChannel");
        return 0;
    } else if ("DmxColorWheelSize" == event.GetPropertyName()) {

        int colorSize = (int)event.GetPropertyValue().GetInteger();
        // if new color size is less than num of colors, remove unneeded colors
        while (colorSize < colors.size()) {
            colors.pop_back();
        }
        // if color size is greater than  num of colors, add needed colors
        int diff = colorSize - colors.size();
        for (int i = 0; i < diff; i++) {
            auto const lastValue = (colors.back().dmxValue + 50) % 255;
            colors.emplace_back(xlRED, lastValue);
        }
        WriteColorSettings(ModelXml);

        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelSize");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelSize");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelSize");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxColorAbility::OnPropertyGridChange::DmxColorWheelSize");
        return 0;
    } else if (event.GetPropertyName().StartsWith("DmxColorWheelSize.DmxColorWheelDMX")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxColorWheelSize.DmxColorWheelDMX", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < colors.size()) {
            colors[index].dmxValue = dxmVal;
            WriteColorSettings(ModelXml);

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDMX");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDMX");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDMX");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDMX");
        }
    }
    else if (event.GetPropertyName().StartsWith("DmxColorWheelSize.DmxColorWheelColor") ){
        wxColour wheeleColour = *wxBLACK;
        wheeleColour << event.GetProperty()->GetValue();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxColorWheelSize.DmxColorWheelColor", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index<colors.size()) {
            colors[index].color = xlColor(wheeleColour);
            WriteColorSettings(ModelXml);

            base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelColor");
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelColor");
            base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelColor");
            base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelColor");
        }
    }
    return -1;
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

void DmxColorAbilityWheel::ExportParameters(wxFile& f, wxXmlNode* ModelXml) const
{
    wxString cwc = ModelXml->GetAttribute("DmxColorWheelChannel", "0");
    wxString dc = ModelXml->GetAttribute("DmxDimmerChannel", "0");
    f.Write(wxString::Format("DmxColorWheelChannel=\"%s\" ", cwc));
    f.Write(wxString::Format("DmxDimmerChannel=\"%s\" ", dc));

    for (int i = 0; i < MAX_COLORS; ++i) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", i);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", i);
        if (!ModelXml->HasAttribute(dmxkey) || !ModelXml->HasAttribute(colorkey)) {
            break;
        }

        wxString dmx = ModelXml->GetAttribute(dmxkey, "0");
        wxString color = ModelXml->GetAttribute(colorkey, "0");
        f.Write(wxString::Format("%s=\"%s\" ", dmxkey, dmx));
        f.Write(wxString::Format("%s=\"%s\" ", colorkey, color));
    }
}

void DmxColorAbilityWheel::ImportParameters(wxXmlNode* ImportXml, Model* m) const
{
    wxString cwc = ImportXml->GetAttribute("DmxColorWheelChannel");
    wxString dc = ImportXml->GetAttribute("DmxDimmerChannel");
    wxString bc = ImportXml->GetAttribute("DmxBlueChannel");
    wxString wc = ImportXml->GetAttribute("DmxWhiteChannel");

    m->SetProperty("DmxColorWheelChannel", cwc);
    m->SetProperty("DmxDimmerChannel", dc);


    for (int i = 0; i < MAX_COLORS; ++i) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", i);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", i);
        if (!ImportXml->HasAttribute(dmxkey) || !ImportXml->HasAttribute(colorkey)) {
            break;
        }

        wxString dmx = ImportXml->GetAttribute(dmxkey, "0");
        wxString color = ImportXml->GetAttribute(colorkey, "0");
        m->SetProperty( dmxkey, dmx);
        m->SetProperty( colorkey, color);
    }
}

void DmxColorAbilityWheel::SetNodeNames(std::vector<std::string>& names) const
{
    if (CheckChannel(wheel_channel, names.size())) {
        names[wheel_channel - 1] = "Color Wheel";
    }
    if (CheckChannel(dimmer_channel, names.size())) {
        names[dimmer_channel - 1] = "Dimmer";
    }
}

void DmxColorAbilityWheel::ReadColorSettings(wxXmlNode* ModelXml)
{
    for (int i =0; i< MAX_COLORS; ++i) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", i);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", i);
        if ( !ModelXml->HasAttribute(dmxkey) || !ModelXml->HasAttribute(colorkey) ) {
            break;
        }
        uint8_t dmxVal = wxAtoi(ModelXml->GetAttribute(dmxkey, "1"));
        wxString dmxcolor = ModelXml->GetAttribute(colorkey);
        colors.emplace_back(xlColor(dmxcolor), dmxVal);
    }
}

void DmxColorAbilityWheel::WriteColorSettings(wxXmlNode* ModelXml) const
{
    for (int i = 0; i < MAX_COLORS; ++i) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", i);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", i);
        if (ModelXml->HasAttribute(dmxkey)) {
            ModelXml->DeleteAttribute(dmxkey);
        }
        if (ModelXml->HasAttribute(colorkey)) {
            ModelXml->DeleteAttribute(colorkey);
        }
    }
    int index = 0;
    for (auto const& col : colors) {
        auto dmxkey = wxString::Format("DmxColorWheelDMX%d", index);
        auto colorkey = wxString::Format("DmxColorWheelColor%d", index);
        ModelXml->DeleteAttribute(dmxkey);
        ModelXml->AddAttribute(dmxkey, wxString::Format("%d", col.dmxValue));
        ModelXml->DeleteAttribute(colorkey);
        ModelXml->AddAttribute(colorkey, col.color);
        ++index;
    }
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
