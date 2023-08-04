/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include "MhChannel.h"
#include "../BaseObject.h"
#include <glm/gtc/type_ptr.hpp>

MhChannel::MhChannel(wxXmlNode* node, wxString _name)
    : node_xml(node), base_name(_name)
{
}

MhChannel::~MhChannel()
{
}

void MhChannel::Init(BaseObject* base) {
    this->base = base;
    channel_coarse = wxAtoi(node_xml->GetAttribute("ChannelCoarse", "0"));
    channel_fine = wxAtoi(node_xml->GetAttribute("ChannelFine", "0"));
}

void MhChannel::AddTypeProperties(wxPropertyGridInterface *grid) {
    
    auto p = grid->Append(new wxUIntProperty("Channel (Coarse)", base_name + "ChannelCoarse", channel_coarse));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Channel (Fine)", base_name + "ChannelFine", channel_fine));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int MhChannel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();

    if (base_name + "ChannelCoarse" == name) {
        node_xml->DeleteAttribute("ChannelCoarse");
        node_xml->AddAttribute("ChannelCoarse", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MhChannel::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MhChannel::OnPropertyGridChange::ChannelCoarse");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MhChannel::OnPropertyGridChange::ChannelCoarse");
        return 0;
    }
    else if (base_name + "ChannelFine" == name) {
        node_xml->DeleteAttribute("ChannelFine");
        node_xml->AddAttribute("ChannelFine", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MhChannel::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MhChannel::OnPropertyGridChange::ChannelFine");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MhChannel::OnPropertyGridChange::ChannelFine");
        return 0;
    }

    return -1;
}
