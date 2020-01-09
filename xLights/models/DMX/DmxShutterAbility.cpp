#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxShutterAbility.h"
#include "../BaseObject.h"

DmxShutterAbility::DmxShutterAbility()
: shutter_channel(0), shutter_threshold(1)
{
}

DmxShutterAbility::~DmxShutterAbility()
{
    //dtor
}

void DmxShutterAbility::AddShutterTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty* p = grid->Append(new wxUIntProperty("Shutter Channel", "DmxShutterChannel", shutter_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Shutter Open Threshold", "DmxShutterOpen", shutter_threshold));
    p->SetAttribute("Min", -255);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");
}

int DmxShutterAbility::OnShutterPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

     if ("DmxShutterChannel" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxShutterChannel");
         ModelXml->AddAttribute("DmxShutterChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxShutterAbility::OnPropertyGridChange::DMXShutterChannel");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxShutterAbility::OnPropertyGridChange::DMXShutterChannel");
         base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxShutterAbility::OnPropertyGridChange::DMXShutterChannel");
         return 0;
     }
     else if ("DmxShutterOpen" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxShutterOpen");
         ModelXml->AddAttribute("DmxShutterOpen", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxShutterAbility::OnPropertyGridChange::DMXShutterOpen");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxShutterAbility::OnPropertyGridChange::DMXShutterOpen");
         base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxShutterAbility::OnPropertyGridChange::DMXShutterOpen");
         return 0;
     }
     return -1;
}
