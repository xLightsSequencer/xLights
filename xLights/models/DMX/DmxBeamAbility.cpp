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

#include "DmxBeamAbility.h"
#include "../BaseObject.h"

DmxBeamAbility::DmxBeamAbility()
{
}

DmxBeamAbility::~DmxBeamAbility()
{
}

void DmxBeamAbility::AddBeamTypeProperties(wxPropertyGridInterface *grid) {

    auto p = grid->Append(new wxPropertyCategory("Beam Properties", "DmxBeamProperties"));
    
    p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Display Width", "DmxBeamWidth", beam_width));
    p->SetAttribute("Min", 0.01);
    p->SetAttribute("Max", 150);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    if (supports_orient) {
        p = grid->Append(new wxUIntProperty("Beam Orientation", "DmxBeamOrient", beam_orient));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }
    
    if (supports_y_offset) {
        p = grid->Append(new wxFloatProperty("Beam Y Offset", "DmxBeamYOffset", beam_y_offset));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1);
        p->SetEditor("SpinCtrl");
    }
}

int DmxBeamAbility::OnBeamPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base)
{
    if ("DmxBeamLength" == event.GetPropertyName()) {
        beam_length = (float)event.GetPropertyValue().GetDouble();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamLength");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamLength");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamLength");
        return 0;
    }
    else if ("DmxBeamWidth" == event.GetPropertyName()) {
        beam_width = (float)event.GetPropertyValue().GetDouble();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamWidth");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamWidth");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxDimmerAbility::OnDimmerPropertyGridChange::DmxBeamWidth");
        return 0;
    }
    else if ("DmxBeamOrient" == event.GetPropertyName()) {
        beam_orient = (int)event.GetPropertyValue().GetLong();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamOrient");
        return 0;
    }
    else if ("DmxBeamYOffset" == event.GetPropertyName()) {
        beam_y_offset = (float)event.GetPropertyValue().GetDouble();
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxMovingHeadAdv::OnPropertyGridChange::DmxBeamYOffset");
        return 0;
    }
    return -1;
}


