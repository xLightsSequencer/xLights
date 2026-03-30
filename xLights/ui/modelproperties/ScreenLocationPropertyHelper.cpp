/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ScreenLocationPropertyHelper.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../../models/BoxedScreenLocation.h"
#include "../../models/TwoPointScreenLocation.h"
#include "../../models/ThreePointScreenLocation.h"
#include "../../models/PolyPointScreenLocation.h"
#include "../../models/RulerObject.h"
#include "../../models/Model.h"
#include "../../UtilFunctions.h"
#include "../wxUtilities.h"
#include "../../utils/string_utils.h"

// ========== BoxedScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const BoxedScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc._locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("X", "ModelX", loc.worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxRED);
    prop = grid->Append(new wxFloatProperty("Y", "ModelY", loc.worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z", "ModelZ", loc.worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("ScaleX", "ScaleX", loc.scalex));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", "ScaleY", loc.scaley));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", "ScaleZ", loc.scalez));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", "RotateX", loc.rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", "RotateY", loc.rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", "RotateZ", loc.rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const BoxedScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Width (%s)", RulerObject::GetUnitDescription()), "RealWidth", loc.GetRealWidth()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Min", 0.01);
    prop = grid->Append(new wxFloatProperty(wxString::Format("Height (%s)", RulerObject::GetUnitDescription()), "RealHeight", loc.GetRealHeight()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Min", 0.01);
    if (loc.supportsZScaling) {
        prop = grid->Append(new wxFloatProperty(wxString::Format("Depth (%s)", RulerObject::GetUnitDescription()), "RealDepth", loc.GetRealDepth()));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Min", 0.01);
    }
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(BoxedScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!loc._locked && "ScaleX" == name) {
        loc.scalex = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ScaleY" == name) {
        loc.scaley = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ScaleZ" == name) {
        loc.scalez = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelX" == name) {
        loc.worldPos_x = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (loc._locked && "ModelX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelY" == name) {
        loc.worldPos_y = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (loc._locked && "ModelY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelZ" == name) {
        loc.worldPos_z = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (loc._locked && "ModelZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RotateX" == name) {
        loc.rotatex = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (loc._locked && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RotateY" == name) {
        loc.rotatey = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (loc._locked && "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RotateZ" == name) {
        loc.rotatez = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (loc._locked && "RotateZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RealWidth" == name) {
        loc.SetMWidth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "RealWidth" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RealHeight" == name) {
        loc.SetMHeight(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "RealHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RealDepth" == name) {
        loc.SetMDepth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "RealDepth" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name) {
        loc._locked = event.GetValue().GetBool();
        loc.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }
    return 0;
}

// ========== TwoPointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc._locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("WorldX", "WorldX", loc.worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("WorldY", "WorldY", loc.worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("WorldZ", "WorldZ", loc.worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");

    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("X1", "ModelX1", loc.worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Y1", "ModelY1", loc.worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z1", "ModelZ1", loc.worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    prop = grid->Append(new wxFloatProperty("X2", "ModelX2", loc.x2 + loc.worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("Y2", "ModelY2", loc.y2 + loc.worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("Z2", "ModelZ2", loc.z2 + loc.worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", RulerObject::Measure(loc.origin, loc.point2)));
    prop->SetAttribute("Precision", 2);
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!loc._locked && "WorldX" == name) {
        loc.worldPos_x = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        return 0;
    }
    else if (loc._locked && "WorldX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "WorldY" == name) {
        loc.worldPos_y = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        return 0;
    }
    else if (loc._locked && "WorldY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "WorldZ" == name) {
        loc.worldPos_z = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        return 0;
    }
    else if (loc._locked && "WorldZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelX1" == name) {
        float old_world_x = loc.worldPos_x;
        loc.worldPos_x = event.GetValue().GetDouble();
        loc.x2 += old_world_x - loc.worldPos_x;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelX1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelY1" == name) {
        float old_world_y = loc.worldPos_y;
        loc.worldPos_y = event.GetValue().GetDouble();
        loc.y2 += old_world_y - loc.worldPos_y;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelY1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelZ1" == name) {
        float old_world_z = loc.worldPos_z;
        loc.worldPos_z = event.GetValue().GetDouble();
        loc.z2 += old_world_z - loc.worldPos_z;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelZ1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelX2" == name) {
        loc.x2 = event.GetValue().GetDouble() - loc.worldPos_x;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelX2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelY2" == name) {
        loc.y2 = event.GetValue().GetDouble() - loc.worldPos_y;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelY2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelZ2" == name) {
        loc.z2 = event.GetValue().GetDouble() - loc.worldPos_z;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "ModelZ2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RealLength" == name) {
        auto origLen = RulerObject::UnMeasure(RulerObject::Measure(loc.origin, loc.point2));
        auto len = RulerObject::UnMeasure(event.GetValue().GetDouble());
        loc.x2 = (loc.x2 * len) / origLen;
        loc.y2 = (loc.y2 * len) / origLen;
        loc.z2 = (loc.z2 * len) / origLen;
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc._locked && "RealLength" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name) {
        loc._locked = event.GetValue().GetBool();
        loc.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }
    return 0;
}

// ========== ThreePointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid) {
    AddSizeLocationProperties(static_cast<const TwoPointScreenLocation&>(loc), grid);
    wxPGProperty* prop = grid->Append(new wxFloatProperty("Height", "ModelHeight", loc.height));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    if (loc.supportsShear) {
        prop = grid->Append(new wxFloatProperty("Shear", "ModelShear", loc.shear));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.1);
        prop->SetEditor("SpinCtrl");
    }
    prop = grid->Append(new wxFloatProperty("RotateX", "RotateX", loc.rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1);
    prop->SetEditor("SpinCtrl");
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    AddDimensionProperties(static_cast<const TwoPointScreenLocation&>(loc), grid, 1.0);
    float width = RulerObject::Measure(loc.origin, loc.point2);
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Height (%s)", RulerObject::GetUnitDescription()), "RealHeight",
                                                           (width * loc.height) / 2.0 * factor));
    prop->ChangeFlag(wxPGFlags::ReadOnly, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    if (!loc._locked && "ModelHeight" == name) {
        loc.height = event.GetValue().GetDouble();
        if (std::abs(loc.height) < 0.01f) {
            if (loc.height < 0.0f) {
                loc.height = -0.01f;
            } else {
                loc.height = 0.01f;
            }
        }
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        return 0;
    }
    else if (loc._locked && "ModelHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "ModelShear" == name) {
        loc.shear = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        return 0;
    }
    else if (loc._locked && "ModelShear" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc._locked && "RotateX" == name) {
        loc.rotatex = event.GetValue().GetDouble();
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (loc._locked && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    return OnPropertyGridChange(static_cast<TwoPointScreenLocation&>(loc), grid, event);
}

// ========== PolyPointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc._locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("X1", "ModelX1", loc.mPos[0].x + loc.worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Y1", "ModelY1", loc.mPos[0].y + loc.worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z1", "ModelZ1", loc.mPos[0].z + loc.worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    for (int i = 1; i < loc.num_points; ++i) {
        prop = grid->Append(new wxFloatProperty(wxString::Format("X%d", i + 1), wxString::Format("ModelX%d", i + 1), loc.mPos[i].x + loc.worldPos_x));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
        prop = grid->Append(new wxFloatProperty(wxString::Format("Y%d", i + 1), wxString::Format("ModelY%d", i + 1), loc.mPos[i].y + loc.worldPos_y));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
        prop = grid->Append(new wxFloatProperty(wxString::Format("Z%d", i + 1), wxString::Format("ModelZ%d", i + 1), loc.mPos[i].z + loc.worldPos_z));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
    }
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    float len = 0;
    auto last = loc.mPos[0].AsVector();
    for (int i = 1; i < (int)loc.mPos.size(); i++) {
        len += RulerObject::Measure(last, loc.mPos[i].AsVector());
        last = loc.mPos[i].AsVector();
    }
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", len));
    prop->ChangeFlag(wxPGFlags::ReadOnly, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    last = loc.mPos[0].AsVector();
    for (int i = 1; i < (int)loc.mPos.size(); i++) {
        len = RulerObject::Measure(last, loc.mPos[i].AsVector());
        last = loc.mPos[i].AsVector();

        auto seg = wxString::Format("Segment %d (%s)", i, RulerObject::GetUnitDescription());
        prop = grid->Append(new wxFloatProperty(seg, "REAL" + seg, len));
        prop->SetAttribute("Precision", 2);
    }
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (StartsWith(name, "REALSegment ")) {
        if (loc._locked) {
            event.Veto();
            return 0;
        } else {
            auto o = name.find(" ", 12);
            wxASSERT(o != std::string::npos);

            loc.selected_handle = wxAtoi(name.substr(12, o - 12)) - 1;

            wxASSERT(loc.selected_handle + 1 < loc.mPos.size());

            float oldLen = 0.0f;
            oldLen = RulerObject::UnMeasure(RulerObject::Measure(loc.mPos[loc.selected_handle].AsVector(), loc.mPos[loc.selected_handle + 1].AsVector()));
            float len = RulerObject::UnMeasure(event.GetValue().GetDouble());

            float dx = (loc.mPos[loc.selected_handle + 1].x - loc.mPos[loc.selected_handle].x) * len / oldLen - (loc.mPos[loc.selected_handle + 1].x - loc.mPos[loc.selected_handle].x);
            float dy = (loc.mPos[loc.selected_handle + 1].y - loc.mPos[loc.selected_handle].y) * len / oldLen - (loc.mPos[loc.selected_handle + 1].y - loc.mPos[loc.selected_handle].y);
            float dz = (loc.mPos[loc.selected_handle + 1].z - loc.mPos[loc.selected_handle].z) * len / oldLen - (loc.mPos[loc.selected_handle + 1].z - loc.mPos[loc.selected_handle].z);

            if (isnan(dx))
                dx = 1.0f;
            if (isnan(dy))
                dy = 1.0f;
            if (isnan(dz))
                dz = 1.0f;

            for (auto i = loc.selected_handle + 1; i < (int)loc.mPos.size(); i++) {
                loc.mPos[i].x += dx;
                loc.mPos[i].y += dy;
                loc.mPos[i].z += dz;
            }

            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            return 0;
        }
    }
    else if (name.length() > 6) {
        loc.selected_handle = wxAtoi(name.substr(6, name.length() - 6)) - 1;
        loc.selected_segment = -1;
        if (!loc._locked && name.find("ModelX") != std::string::npos) {
            loc.mPos[loc.selected_handle].x = event.GetValue().GetDouble() - loc.worldPos_x;
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            return 0;
        }
        else if (loc._locked && name.find("ModelX") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!loc._locked && name.find("ModelY") != std::string::npos) {
            loc.mPos[loc.selected_handle].y = event.GetValue().GetDouble() - loc.worldPos_y;
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            return 0;
        }
        else if (loc._locked && name.find("ModelY") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!loc._locked && name.find("ModelZ") != std::string::npos) {
            loc.mPos[loc.selected_handle].z = event.GetValue().GetDouble() - loc.worldPos_z;
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            return 0;
        }
        else if (loc._locked && name.find("ModelZ") != std::string::npos) {
            event.Veto();
            return 0;
        }
    }
    else if ("Locked" == name) {
        loc._locked = event.GetValue().GetBool();
        loc.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }
    return 0;
}

// ========== Dispatch methods (ModelScreenLocation&) ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const ModelScreenLocation& loc, wxPropertyGridInterface* grid) {
    if (auto* boxed = dynamic_cast<const BoxedScreenLocation*>(&loc)) {
        AddSizeLocationProperties(*boxed, grid);
    } else if (auto* two = dynamic_cast<const TwoPointScreenLocation*>(&loc)) {
        AddSizeLocationProperties(*two, grid);
    } else if (auto* poly = dynamic_cast<const PolyPointScreenLocation*>(&loc)) {
        AddSizeLocationProperties(*poly, grid);
    } else if (auto* three = dynamic_cast<const ThreePointScreenLocation*>(&loc)) {
        AddSizeLocationProperties(*three, grid);
    }
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const ModelScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    if (auto* boxed = dynamic_cast<const BoxedScreenLocation*>(&loc)) {
        AddDimensionProperties(*boxed, grid, factor);
    } else if (auto* two = dynamic_cast<const TwoPointScreenLocation*>(&loc)) {
        AddDimensionProperties(*two, grid, factor);
    } else if (auto* poly = dynamic_cast<const PolyPointScreenLocation*>(&loc)) {
        AddDimensionProperties(*poly, grid, factor);
    } else if (auto* three = dynamic_cast<const ThreePointScreenLocation*>(&loc)) {
        AddDimensionProperties(*three, grid, factor);
    }
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(ModelScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (auto* boxed = dynamic_cast<BoxedScreenLocation*>(&loc)) {
        return OnPropertyGridChange(*boxed, grid, event);
    } else if (auto* two = dynamic_cast<TwoPointScreenLocation*>(&loc)) {
        return OnPropertyGridChange(*two, grid, event);
    } else if (auto* poly = dynamic_cast<PolyPointScreenLocation*>(&loc)) {
        return OnPropertyGridChange(*poly, grid, event);
    } else if (auto* three = dynamic_cast<ThreePointScreenLocation*>(&loc)) {
        return OnPropertyGridChange(*three, grid, event);
    }
    return 0;
}
