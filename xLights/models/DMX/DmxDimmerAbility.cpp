/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxDimmerAbility.h"
#include "../BaseObject.h"
#include "../../Color.h"

DmxDimmerAbility::DmxDimmerAbility() :
    dimmer_channel(0)
{
}

DmxDimmerAbility::~DmxDimmerAbility()
{
    //dtor
}

void DmxDimmerAbility::AddDimmerTypeProperties(wxPropertyGridInterface *grid) {

    auto p = grid->Append(new wxPropertyCategory("Dimmer Properties", "DmxDimmerProperties"));
    
    p = grid->Append(new wxUIntProperty("Dimmer Channel", "MhDimmerChannel", dimmer_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int DmxDimmerAbility::OnDimmerPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

     if ("MhDimmerChannel" == event.GetPropertyName()) {
         dimmer_channel = (int)event.GetPropertyValue().GetLong();
         ModelXml->DeleteAttribute("MhDimmerChannel");
         ModelXml->AddAttribute("MhDimmerChannel", wxString::Format("%d", dimmer_channel));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxDimmerAbility::OnPropertyGridChange::DMXDimmerChannel");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxDimmerAbility::OnPropertyGridChange::DMXDimmerChannel");
         base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxDimmerAbility::OnPropertyGridChange::DMXDimmerChannel");
         return 0;
     }
     return -1;
}


