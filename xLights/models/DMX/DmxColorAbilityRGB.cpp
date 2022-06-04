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

#include "DmxColorAbilityRGB.h"
#include "../BaseObject.h"
#include "../Model.h"

#include "../../Color.h"
#include "../Node.h"

void DmxColorAbilityRGB::InitColor( wxXmlNode* ModelXml)
{
    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "1"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "2"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "3"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
}

bool DmxColorAbilityRGB::IsColorChannel(uint32_t channel) const
{
    return (red_channel == channel || green_channel == channel || blue_channel == channel || white_channel == channel);
}

void DmxColorAbilityRGB::SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const
{
    xlColor c;
    if (white_channel > 0 && color.red == color.green && color.red == color.blue) {
        c.red = color.red;
        c.green = color.red;
        c.blue = color.red;
        if (pixelVector.size() > white_channel - 1)
            pixelVector[white_channel - 1] = c;
    } else {
        if (red_channel != 0) {
            c.red = color.red;
            c.green = color.red;
            c.blue = color.red;
            if (pixelVector.size() > red_channel - 1)
                pixelVector[red_channel - 1] = c;
        }
        if (green_channel != 0) {
            c.red = color.green;
            c.green = color.green;
            c.blue = color.green;
            if (pixelVector.size() > green_channel - 1)
                pixelVector[green_channel - 1] = c;
        }
        if (blue_channel != 0) {
            c.red = color.blue;
            c.green = color.blue;
            c.blue = color.blue;
            if (pixelVector.size() > blue_channel - 1)
                pixelVector[blue_channel - 1] = c;
        }
    }
}

std::list<std::string> DmxColorAbilityRGB::CheckModelSettings(Model *m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    if (red_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s red channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), red_channel, nodeCount));
    }
    if (green_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s green channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), green_channel, nodeCount));
    }
    if (blue_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s blue channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), blue_channel, nodeCount));
    }
    if (white_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s white channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), white_channel, nodeCount));
    }
    return res;
}

bool DmxColorAbilityRGB::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    return (red_channel < nodeCount &&
        green_channel < nodeCount &&
        blue_channel < nodeCount &&
        white_channel < nodeCount);
}

void DmxColorAbilityRGB::AddColorTypeProperties(wxPropertyGridInterface *grid) const {

    wxPGProperty* p = grid->Append(new wxUIntProperty("Red Channel", "DmxRedChannel", red_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Green Channel", "DmxGreenChannel", green_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Blue Channel", "DmxBlueChannel", blue_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("White Channel", "DmxWhiteChannel", white_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int DmxColorAbilityRGB::OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

    if ("DmxRedChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxRedChannel");
        ModelXml->AddAttribute("DmxRedChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        return 0;
    } else if ("DmxGreenChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxGreenChannel");
        ModelXml->AddAttribute("DmxGreenChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        return 0;
    } else if ("DmxBlueChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBlueChannel");
        ModelXml->AddAttribute("DmxBlueChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        return 0;
    } else if ("DmxWhiteChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxWhiteChannel");
        ModelXml->AddAttribute("DmxWhiteChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        return 0;
    }
    return -1;
}

xlColor DmxColorAbilityRGB::GetBeamColor( const std::vector<NodeBaseClassPtr>& Nodes) const
{
    auto NodeCount = Nodes.size();
    xlColor beam_color(xlWHITE);
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy = xlBLACK;
        if (white_channel > 0 && white_channel <= NodeCount) {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK) {
            if (red_channel <= NodeCount) {
                Nodes[red_channel - 1]->GetColor(proxy);
                beam_color.red = proxy.red;
            }
            if (green_channel <= NodeCount) {
                Nodes[green_channel - 1]->GetColor(proxy);
                beam_color.green = proxy.red;
            }
            if (blue_channel <= NodeCount) {
                Nodes[blue_channel - 1]->GetColor(proxy);
                beam_color.blue = proxy.red;
            }
        }
    } else if (white_channel > 0 && white_channel <= NodeCount) {
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
    if (c != nullptr) {
        beam_color = *c;
    } else if (!allowSelected) {
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
    }
    int trans = beam_color == xlBLACK ? blackTransparency : transparency;
    Model::ApplyTransparency(beam_color, trans, trans);
    color = beam_color;
}

[[nodiscard]] xlColor DmxColorAbilityRGB::GetColorPixels(xlColorVector const& pixelVector ) const
{
    xlColor beam_color( xlBLACK );
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        if (white_channel > 0) {
            beam_color = pixelVector[white_channel - 1];
        }

        if (beam_color == xlBLACK) {
            beam_color.red = pixelVector[red_channel - 1].red;
            beam_color.green = pixelVector[green_channel - 1].red;
            beam_color.blue = pixelVector[blue_channel - 1].red;
        }
    } else if (white_channel > 0) {
        beam_color.red = pixelVector[white_channel - 1].red;
        beam_color.green = pixelVector[white_channel - 1].red;
        beam_color.blue = pixelVector[white_channel - 1].red;
    }
    return beam_color;
}

void DmxColorAbilityRGB::ExportParameters(wxFile& f, wxXmlNode* ModelXml) const
{
    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
}

void DmxColorAbilityRGB::ImportParameters(wxXmlNode* ImportXml, Model* m) const
{
    wxString rc = ImportXml->GetAttribute("DmxRedChannel");
    wxString gc = ImportXml->GetAttribute("DmxGreenChannel");
    wxString bc = ImportXml->GetAttribute("DmxBlueChannel");
    wxString wc = ImportXml->GetAttribute("DmxWhiteChannel");

    m->SetProperty("DmxRedChannel", rc);
    m->SetProperty("DmxGreenChannel", gc);
    m->SetProperty("DmxBlueChannel", bc);
    m->SetProperty("DmxWhiteChannel", wc);
}

void DmxColorAbilityRGB::SetNodeNames(std::vector<std::string>& names) const
{
    if (0 != red_channel && red_channel < names.size()) {
        names[red_channel - 1] = "Red";
    }
    if (0 != blue_channel && blue_channel < names.size()) {
        names[blue_channel - 1] = "Blue";
    }
    if (0 != green_channel && green_channel < names.size()) {
        names[green_channel - 1] = "Green";
    }
    if (0 != white_channel && white_channel < names.size()) {
        names[white_channel - 1] = "White";
    }
}
