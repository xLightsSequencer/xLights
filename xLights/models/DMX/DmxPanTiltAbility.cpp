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
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXPanChannel");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXPanChannel");
         base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DMXModel::OnPropertyGridChange::DMXPanChannel");
         return 0;
     }
     else if ("DmxPanOrient" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanOrient");
         ModelXml->AddAttribute("DmxPanOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXPanOrient");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXPanOrient");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXPanOrient");
         return 0;
     }
     else if ("DmxPanDegOfRot" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanDegOfRot");
         ModelXml->AddAttribute("DmxPanDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXPanDegOfRot");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXPanDegOfRot");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXPanDegOfRot");
         return 0;
     }
     else if ("DmxPanSlewLimit" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("DmxPanSlewLimit");
         ModelXml->AddAttribute("DmxPanSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
         base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXPanSlewLimit");
         base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXPanSlewLimit");
         base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXPanSlewLimit");
         return 0;
     }
    else if ("DmxTiltChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltChannel");
        ModelXml->AddAttribute("DmxTiltChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXTiltChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXTiltChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DMXModel::OnPropertyGridChange::DMXTiltChannel");
        return 0;
     }
    else if ("DmxTiltOrient" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltOrient");
        ModelXml->AddAttribute("DmxTiltOrient", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXTiltOrient");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXTiltOrient");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXTiltOrient");
        return 0;
     }
    else if ("DmxTiltDegOfRot" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltDegOfRot");
        ModelXml->AddAttribute("DmxTiltDegOfRot", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXTiltDegOfRot");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXTiltDegOfRot");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXTiltDegOfRot");
        return 0;
     }
    else if ("DmxTiltSlewLimit" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxTiltSlewLimit");
        ModelXml->AddAttribute("DmxTiltSlewLimit", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DMXModel::OnPropertyGridChange::DMXTiltSlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DMXModel::OnPropertyGridChange::DMXTiltSlewLimit");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DMXModel::OnPropertyGridChange::DMXTiltSlewLimit");
        return 0;
     }

     return -1;
}
