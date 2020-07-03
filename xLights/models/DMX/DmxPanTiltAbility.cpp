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

#include "DmxPanTiltAbility.h"
#include "../BaseObject.h"

DmxPanTiltAbility::DmxPanTiltAbility()
{
}

DmxPanTiltAbility::~DmxPanTiltAbility()
{
    //dtor
}

void DmxPanTiltAbility::AddPanTiltTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty* p = grid->Append(new wxUIntProperty("Pan Channel", "DmxPanChannel", pan_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Pan Orientation", "DmxPanOrient", pan_orient));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Pan Deg of Rot", "DmxPanDegOfRot", pan_deg_of_rot));
    p->SetAttribute("Min", -1000);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Pan Slew Limit (deg/sec)", "DmxPanSlewLimit", pan_slew_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Tilt Channel", "DmxTiltChannel", tilt_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Tilt Orientation", "DmxTiltOrient", tilt_orient));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Tilt Deg of Rot", "DmxTiltDegOfRot", tilt_deg_of_rot));
    p->SetAttribute("Min", -1000);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Tilt Slew Limit (deg/sec)", "DmxTiltSlewLimit", tilt_slew_limit));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 500);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

}

int DmxPanTiltAbility::OnPanTiltPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

     if ("DmxPanChannel" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanChannel");
         ModelXml->AddAttribute("DmxPanChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanChannel");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanChannel");
         base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanChannel");
         return 0;
     }
     else if ("DmxPanOrient" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanOrient");
         ModelXml->AddAttribute("DmxPanOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanOrient");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanOrient");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanOrient");
         return 0;
     }
     else if ("DmxPanDegOfRot" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanDegOfRot");
         ModelXml->AddAttribute("DmxPanDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanDegOfRot");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanDegOfRot");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanDegOfRot");
         return 0;
     }
     else if ("DmxPanSlewLimit" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanSlewLimit");
         ModelXml->AddAttribute("DmxPanSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanSlewLimit");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanSlewLimit");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXPanSlewLimit");
         return 0;
     }
    else if ("DmxTiltChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltChannel");
        ModelXml->AddAttribute("DmxTiltChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltChannel");
        return 0;
     }
    else if ("DmxTiltOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltOrient");
        ModelXml->AddAttribute("DmxTiltOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltOrient");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltOrient");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltOrient");
        return 0;
     }
    else if ("DmxTiltDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltDegOfRot");
        ModelXml->AddAttribute("DmxTiltDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltDegOfRot");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltDegOfRot");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltDegOfRot");
        return 0;
     }
    else if ("DmxTiltSlewLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltSlewLimit");
        ModelXml->AddAttribute("DmxTiltSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltSlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltSlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxPanTiltAbility::OnPanTiltPropertyGridChange::DMXTiltSlewLimit");
        return 0;
     }

     return -1;
}
