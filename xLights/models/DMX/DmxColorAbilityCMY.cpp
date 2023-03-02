/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/advprops.h>
#include <wx/propgrid/propgrid.h>
#include <wx/xml/xml.h>

#include "DmxColorAbilityCMY.h"
#include "../BaseObject.h"
#include "../Model.h"

#include "../../Color.h"
#include "../Node.h"

void DmxColorAbilityCMY::InitColor(wxXmlNode* ModelXml)
{
    cyan_channel = wxAtoi(ModelXml->GetAttribute("DmxCyanChannel", "1"));
    magenta_channel = wxAtoi(ModelXml->GetAttribute("DmxMagentaChannel", "2"));
    yellow_channel = wxAtoi(ModelXml->GetAttribute("DmxYellowChannel", "3"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
}

bool DmxColorAbilityCMY::IsColorChannel(uint32_t channel) const
{
    return (cyan_channel == channel || magenta_channel == channel || yellow_channel == channel || white_channel == channel);
}

float DmxColorAbilityCMY::GetK(xlColor c) const
{
    return 1.0 - std::max((float)c.Red() / 255.0, std::max((float)c.Green() / 255.0, (float)c.Blue() / 255.0));
}

uint8_t DmxColorAbilityCMY::GetC(xlColor c) const
{
    if (c.red == c.green && c.red == c.blue)
        return c.red;
    float K = GetK(c);
    return ((1.0 - (float)c.Red() / 255.0 - K) * 255) / (1 - K);
}

uint8_t DmxColorAbilityCMY::GetM(xlColor c) const
{
    if (c.red == c.green && c.red == c.blue)
        return c.red;
    float K = GetK(c);
    return ((1.0 - (float)c.Green() / 255.0 - K) * 255) / (1 - K);
}

uint8_t DmxColorAbilityCMY::GetY(xlColor c) const
{
    if (c.red == c.green && c.red == c.blue)
        return c.red;
    float K = GetK(c);
    return ((1.0 - (float)c.Blue() / 255.0 - K) * 255) / (1 - K);
}

void DmxColorAbilityCMY::SetColorPixels(const xlColor& color, xlColorVector& pixelVector) const
{
    xlColor c;
    if (CheckChannel(white_channel, pixelVector.size()) && color.red == color.green && color.red == color.blue) {
        c.red = color.red;
        c.green = color.red;
        c.blue = color.red;
        pixelVector[white_channel - 1] = c;
    } else {
        if (CheckChannel(cyan_channel, pixelVector.size())) {
            c.red = GetC(color);
            c.green = c.red;
            c.blue = c.red;
            pixelVector[cyan_channel - 1] = c;
        }
        if (CheckChannel(magenta_channel, pixelVector.size())) {
            c.red = GetM(color);
            c.green = c.red;
            c.blue = c.red;
            pixelVector[magenta_channel - 1] = c;
        }
        if (CheckChannel(yellow_channel, pixelVector.size())) {
            c.red = GetY(color);
            c.green = c.red;
            c.blue = c.red;
            pixelVector[yellow_channel - 1] = c;
        }
    }
}

std::list<std::string> DmxColorAbilityCMY::CheckModelSettings(Model* m) const
{
    std::list<std::string> res;
    auto nodeCount = m->GetNodeCount();

    if (cyan_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s cyan channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), cyan_channel, nodeCount));
    }
    if (magenta_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s magenta channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), magenta_channel, nodeCount));
    }
    if (yellow_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s yellow channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), yellow_channel, nodeCount));
    }
    if (white_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s white channel refers to a channel (%d) not present on the model which only has %d channels.", m->GetName(), white_channel, nodeCount));
    }
    return res;
}

bool DmxColorAbilityCMY::IsValidModelSettings(Model* m) const
{
    auto nodeCount = m->GetNodeCount();

    return (cyan_channel < nodeCount + 1 &&
            magenta_channel < nodeCount + 1 &&
            yellow_channel < nodeCount + 1 &&
            white_channel < nodeCount + 1);
}

void DmxColorAbilityCMY::AddColorTypeProperties(wxPropertyGridInterface* grid) const
{
    wxPGProperty* p = grid->Append(new wxUIntProperty("Cyan Channel", "DmxCyanChannel", cyan_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Magenta Channel", "DmxMagentaChannel", magenta_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Yellow Channel", "DmxYellowChannel", yellow_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("White Channel", "DmxWhiteChannel", white_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int DmxColorAbilityCMY::OnColorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base)
{
    if ("DmxCyanChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxCyanChannel");
        ModelXml->AddAttribute("DmxCyanChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXCyanChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXCyanChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXCyanChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXCyanChannel");
        return 0;
    } else if ("DmxMagentaChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxMagentaChannel");
        ModelXml->AddAttribute("DmxMagentaChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXMagentaChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXMagentaChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXMagentaChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXMagentaChannel");
        return 0;
    } else if ("DmxYellowChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxYellowChannel");
        ModelXml->AddAttribute("DmxYellowChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXYellowChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXYellowChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXYellowChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXYellowChannel");
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

xlColor DmxColorAbilityCMY::GetRGB(uint8_t cyan, uint8_t magenta, uint8_t yellow) const
{
    xlColor c = xlBLACK;

    if (cyan == magenta && cyan == yellow) {
        c.red = cyan;
        c.green = cyan;
        c.blue = cyan;
    } else {
        c.red = 255.0 * (1.0 - (float)cyan / 255.0);
        c.green = 255.0 * (1.0 - (float)magenta / 255.0);
        c.blue = 255.0 * (1.0 - (float)yellow / 255.0);
    }

    return c;
}

xlColor DmxColorAbilityCMY::GetBeamColor(const std::vector<NodeBaseClassPtr>& Nodes) const
{
    auto NodeCount = Nodes.size();
    xlColor beam_color(xlWHITE);
    if (cyan_channel > 0 && magenta_channel > 0 && yellow_channel > 0) {
        xlColor proxy = xlBLACK;
        if (CheckChannel(white_channel, NodeCount)) {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK) {
            xlColor cyan = xlBLACK;
            xlColor magenta = xlBLACK;
            xlColor yellow = xlBLACK;
            if (CheckChannel(cyan_channel, NodeCount)) {
                Nodes[cyan_channel - 1]->GetColor(cyan);
            }
            if (CheckChannel(magenta_channel, NodeCount)) {
                Nodes[magenta_channel - 1]->GetColor(magenta);
            }
            if (CheckChannel(yellow_channel, NodeCount)) {
                Nodes[yellow_channel - 1]->GetColor(yellow);
            }
            beam_color = GetRGB(cyan.red, magenta.red, yellow.red);
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

bool DmxColorAbilityCMY::ApplyChannelTransparency(xlColor& color, int transparency, uint32_t channel) const
{
    if (channel == cyan_channel) {
        color = xlCYAN;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == magenta_channel) {
        color = xlMAGENTA;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == yellow_channel) {
        color = xlYELLOW;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    } else if (channel == white_channel) {
        color = xlWHITE;
        Model::ApplyTransparency(color, transparency, transparency);
        return true;
    }
    return false;
}

void DmxColorAbilityCMY::GetColor(xlColor& color, int transparency, int blackTransparency, bool allowSelected,
                                  const xlColor* c, const std::vector<NodeBaseClassPtr>& Nodes) const
{
    xlColor beam_color(xlWHITE);
    auto NodeCount = Nodes.size();
    if (c != nullptr) {
        beam_color = *c;
    } else if (!allowSelected) {
        if (CheckChannel(cyan_channel, NodeCount) &&
            CheckChannel(magenta_channel, NodeCount) &&
            CheckChannel(yellow_channel, NodeCount)) {
            xlColor proxy = xlBLACK;
            if (CheckChannel(white_channel, NodeCount)) {
                Nodes[white_channel - 1]->GetColor(proxy);
                beam_color = proxy;
            }

            if (proxy == xlBLACK) {
                xlColor cyan = xlBLACK;
                xlColor magenta = xlBLACK;
                xlColor yellow = xlBLACK;
                Nodes[cyan_channel - 1]->GetColor(cyan);
                Nodes[magenta_channel - 1]->GetColor(magenta);
                Nodes[yellow_channel - 1]->GetColor(yellow);
                beam_color = GetRGB(cyan.red, magenta.red, yellow.red);
            }
        } else if (CheckChannel(white_channel, NodeCount)) {
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

[[nodiscard]] xlColor DmxColorAbilityCMY::GetColorPixels(xlColorVector const& pixelVector) const
{
    auto NodeCount = pixelVector.size();
    xlColor beam_color(xlBLACK);
    if (CheckChannel(cyan_channel, NodeCount) &&
        CheckChannel(magenta_channel, NodeCount) &&
        CheckChannel(yellow_channel, NodeCount)) {
        if (CheckChannel(white_channel, NodeCount)) {
            beam_color = pixelVector[white_channel - 1];
        }

        if (beam_color == xlBLACK) {
            beam_color = GetRGB(pixelVector[cyan_channel - 1].red, pixelVector[magenta_channel - 1].red, pixelVector[yellow_channel - 1].red);
        }
    } else if (CheckChannel(white_channel, NodeCount)) {
        beam_color.red = pixelVector[white_channel - 1].red;
        beam_color.green = pixelVector[white_channel - 1].red;
        beam_color.blue = pixelVector[white_channel - 1].red;
    }
    return beam_color;
}

void DmxColorAbilityCMY::ExportParameters(wxFile& f, wxXmlNode* ModelXml) const
{
    wxString rc = ModelXml->GetAttribute("DmxCyanChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxMagentaChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxYellowChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    f.Write(wxString::Format("DmxCyanChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxMagentaChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxYellowChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
}

void DmxColorAbilityCMY::ImportParameters(wxXmlNode* ImportXml, Model* m) const
{
    wxString rc = ImportXml->GetAttribute("DmxCyanChannel");
    wxString gc = ImportXml->GetAttribute("DmxMagentaChannel");
    wxString bc = ImportXml->GetAttribute("DmxYellowChannel");
    wxString wc = ImportXml->GetAttribute("DmxWhiteChannel");

    m->SetProperty("DmxCyanChannel", rc);
    m->SetProperty("DmxMagentaChannel", gc);
    m->SetProperty("DmxYellowChannel", bc);
    m->SetProperty("DmxWhiteChannel", wc);
}

void DmxColorAbilityCMY::SetNodeNames(std::vector<std::string>& names) const
{
    if (CheckChannel(cyan_channel, names.size())) {
        names[cyan_channel - 1] = "Cyan";
    }
    if (CheckChannel(magenta_channel, names.size())) {
        names[magenta_channel - 1] = "Magenta";
    }
    if (CheckChannel(yellow_channel, names.size())) {
        names[yellow_channel - 1] = "Yellow";
    }
    if (CheckChannel(white_channel, names.size())) {
        names[white_channel - 1] = "White";
    }
}
