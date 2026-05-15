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
#include <wx/settings.h>

#include "models/BoxedScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
#include "models/ThreePointScreenLocation.h"
#include "models/PolyPointScreenLocation.h"
#include "models/RulerObject.h"
#include "models/Model.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "utils/FloatChecks.h"
#include "utils/string_utils.h"

// ========== BoxedScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const BoxedScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc.IsLocked()));
    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("X", "ModelX", loc.GetWorldPos_X()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxRED);
    prop = grid->Append(new wxFloatProperty("Y", "ModelY", loc.GetWorldPos_Y()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z", "ModelZ", loc.GetWorldPos_Z()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("ScaleX", "ScaleX", loc.GetScaleX()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", "ScaleY", loc.GetScaleY()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", "ScaleZ", loc.GetScaleZ()));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", "RotateX", loc.GetRotateX()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", "RotateY", loc.GetRotateY()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", "RotateZ", loc.GetRotateZ()));
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
    if (loc.GetSupportsZScaling()) {
        prop = grid->Append(new wxFloatProperty(wxString::Format("Depth (%s)", RulerObject::GetUnitDescription()), "RealDepth", loc.GetRealDepth()));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Min", 0.01);
    }
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(BoxedScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!loc.IsLocked() && "ScaleX" == name) {
        loc.SetScaleX(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ScaleY" == name) {
        loc.SetScaleY(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ScaleZ" == name) {
        loc.SetScaleZ(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelX" == name) {
        loc.SetWorldPos_X(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (loc.IsLocked() && "ModelX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelY" == name) {
        loc.SetWorldPos_Y(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (loc.IsLocked() && "ModelY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelZ" == name) {
        loc.SetWorldPos_Z(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (loc.IsLocked() && "ModelZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RotateX" == name) {
        loc.SetRotateX(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (loc.IsLocked() && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RotateY" == name) {
        loc.SetRotateY(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (loc.IsLocked() && "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RotateZ" == name) {
        loc.SetRotateZ(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (loc.IsLocked() && "RotateZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RealWidth" == name) {
        loc.SetMWidth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "RealWidth" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RealHeight" == name) {
        loc.SetMHeight(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "RealHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RealDepth" == name) {
        loc.SetMDepth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "RealDepth" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name) {
        loc.SetLocked(event.GetValue().GetBool());
        loc.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }
    return 0;
}

// ========== TwoPointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc.IsLocked()));
    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("WorldX", "WorldX", loc.GetWorldPos_X()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("WorldY", "WorldY", loc.GetWorldPos_Y()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("WorldZ", "WorldZ", loc.GetWorldPos_Z()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");

    prop->SetAttribute("UseCheckbox", 1);
    prop = grid->Append(new wxFloatProperty("X1", "ModelX1", loc.GetWorldPos_X()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Y1", "ModelY1", loc.GetWorldPos_Y()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z1", "ModelZ1", loc.GetWorldPos_Z()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    prop = grid->Append(new wxFloatProperty("X2", "ModelX2", loc.GetX2() + loc.GetWorldPos_X()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("Y2", "ModelY2", loc.GetY2() + loc.GetWorldPos_Y()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
    prop = grid->Append(new wxFloatProperty("Z2", "ModelZ2", loc.GetZ2() + loc.GetWorldPos_Z()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(BlueOrLightBlue());
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", RulerObject::Measure(loc.GetPoint1(), loc.GetPoint2())));
    prop->SetAttribute("Precision", 2);
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(TwoPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!loc.IsLocked() && "WorldX" == name) {
        loc.SetWorldPos_X(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        return 0;
    }
    else if (loc.IsLocked() && "WorldX" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "WorldY" == name) {
        loc.SetWorldPos_Y(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        return 0;
    }
    else if (loc.IsLocked() && "WorldY" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "WorldZ" == name) {
        loc.SetWorldPos_Z(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        return 0;
    }
    else if (loc.IsLocked() && "WorldZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelX1" == name) {
        float new_x = event.GetValue().GetDouble();
        float old_world_x = loc.GetWorldPos_X();
        loc.SetWorldPos_X(new_x);
        loc.SetX2(loc.GetX2() + old_world_x - new_x);
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelX1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelY1" == name) {
        float new_y = event.GetValue().GetDouble();
        float old_world_y = loc.GetWorldPos_Y();
        loc.SetWorldPos_Y(new_y);
        loc.SetY2(loc.GetY2() + old_world_y - new_y);
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelY1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelZ1" == name) {
        float new_z = event.GetValue().GetDouble();
        float old_world_z = loc.GetWorldPos_Z();
        loc.SetWorldPos_Z(new_z);
        loc.SetZ2(loc.GetZ2() + old_world_z - new_z);
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelZ1" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelX2" == name) {
        loc.SetX2(event.GetValue().GetDouble() - loc.GetWorldPos_X());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelX2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelY2" == name) {
        loc.SetY2(event.GetValue().GetDouble() - loc.GetWorldPos_Y());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelY2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelZ2" == name) {
        loc.SetZ2(event.GetValue().GetDouble() - loc.GetWorldPos_Z());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "ModelZ2" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RealLength" == name) {
        auto origLen = RulerObject::UnMeasure(RulerObject::Measure(loc.GetPoint1(), loc.GetPoint2()));
        auto len = RulerObject::UnMeasure(event.GetValue().GetDouble());
        loc.SetX2((loc.GetX2() * len) / origLen);
        loc.SetY2((loc.GetY2() * len) / origLen);
        loc.SetZ2((loc.GetZ2() * len) / origLen);
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (loc.IsLocked() && "RealLength" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name) {
        loc.SetLocked(event.GetValue().GetBool());
        loc.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                        OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }
    return 0;
}

// ========== ThreePointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid) {
    AddSizeLocationProperties(static_cast<const TwoPointScreenLocation&>(loc), grid);
    wxPGProperty* prop = grid->Append(new wxFloatProperty("Height", "ModelHeight", loc.GetHeight()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    if (loc.GetSupportsShear()) {
        prop = grid->Append(new wxFloatProperty("Shear", "ModelShear", loc.GetShear()));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.1);
        prop->SetEditor("SpinCtrl");
    }
    prop = grid->Append(new wxFloatProperty("RotateX", "RotateX", loc.GetRotateX()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1);
    prop->SetEditor("SpinCtrl");
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    AddDimensionProperties(static_cast<const TwoPointScreenLocation&>(loc), grid, 1.0);
    float width = RulerObject::Measure(loc.GetPoint1(), loc.GetPoint2());
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Height (%s)", RulerObject::GetUnitDescription()), "RealHeight",
                                                           (width * loc.GetHeight()) / 2.0 * factor));
    prop->ChangeFlag(wxPGFlags::ReadOnly, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(ThreePointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    if (!loc.IsLocked() && "ModelHeight" == name) {
        loc.SetHeight(event.GetValue().GetDouble());
        if (std::abs(loc.GetHeight()) < 0.01f) {
            if (loc.GetHeight() < 0.0f) {
                loc.SetHeight(-0.01f);
            } else {
                loc.SetHeight(0.01f);
            }
        }
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        return 0;
    }
    else if (loc.IsLocked() && "ModelHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "ModelShear" == name) {
        loc.SetShear(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        return 0;
    }
    else if (loc.IsLocked() && "ModelShear" == name) {
        event.Veto();
        return 0;
    }
    else if (!loc.IsLocked() && "RotateX" == name) {
        loc.SetRotateX(event.GetValue().GetDouble());
        loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (loc.IsLocked() && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    return OnPropertyGridChange(static_cast<TwoPointScreenLocation&>(loc), grid, event);
}

// ========== PolyPointScreenLocation ==========

void ScreenLocationPropertyHelper::AddSizeLocationProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid) {
    wxPGProperty* prop = grid->Append(new wxBoolProperty("Locked", "Locked", loc.IsLocked()));
    prop->SetAttribute("UseCheckbox", 1);
    auto worldPos = loc.GetWorldPosition();
    auto p0 = loc.GetPoint(0);
    prop = grid->Append(new wxFloatProperty("X1", "ModelX1", p0.x + worldPos.x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Y1", "ModelY1", p0.y + worldPos.y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = grid->Append(new wxFloatProperty("Z1", "ModelZ1", p0.z + worldPos.z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    for (int i = 1; i < loc.GetNumPoints(); ++i) {
        auto pt = loc.GetPoint(i);
        prop = grid->Append(new wxFloatProperty(wxString::Format("X%d", i + 1), wxString::Format("ModelX%d", i + 1), pt.x + worldPos.x));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
        prop = grid->Append(new wxFloatProperty(wxString::Format("Y%d", i + 1), wxString::Format("ModelY%d", i + 1), pt.y + worldPos.y));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
        prop = grid->Append(new wxFloatProperty(wxString::Format("Z%d", i + 1), wxString::Format("ModelZ%d", i + 1), pt.z + worldPos.z));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(BlueOrLightBlue());
    }
}

void ScreenLocationPropertyHelper::AddDimensionProperties(const PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, float factor) {
    float len = 0;
    auto last = loc.GetPoint(0);
    for (int i = 1; i < loc.GetNumPoints(); i++) {
        auto p = loc.GetPoint(i);
        len += RulerObject::Measure(last, p);
        last = p;
    }
    wxPGProperty* prop = grid->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", len));
    prop->ChangeFlag(wxPGFlags::ReadOnly, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    last = loc.GetPoint(0);
    for (int i = 1; i < loc.GetNumPoints(); i++) {
        auto p = loc.GetPoint(i);
        len = RulerObject::Measure(last, p);
        last = p;

        auto seg = wxString::Format("Segment %d (%s)", i, RulerObject::GetUnitDescription());
        prop = grid->Append(new wxFloatProperty(seg, "REAL" + seg, len));
        prop->SetAttribute("Precision", 2);
    }
}

int ScreenLocationPropertyHelper::OnPropertyGridChange(PolyPointScreenLocation& loc, wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (StartsWith(name, "REALSegment ")) {
        if (loc.IsLocked()) {
            event.Veto();
            return 0;
        } else {
            auto o = name.find(" ", 12);
            wxASSERT(o != std::string::npos);

            // "REAL Segment N" — N is 1-based segment number; segment N
            // joins point N-1 to point N. Skip the SetSelectedHandle→
            // GetSelectedHandle round-trip (which previously lost
            // information through the legacy int convention) and use
            // the parsed index directly. SetSelectedHandle is still
            // called for visual feedback (highlights the corresponding
            // vertex in the layout view).
            int h = wxAtoi(name.substr(12, o - 12)) - 1;
            loc.SetSelectedHandle(h);
            wxASSERT(h + 1 < (int)loc.GetNumPoints());
            auto p1 = loc.GetPoint(h);
            auto p2 = loc.GetPoint(h + 1);
            float oldLen = RulerObject::UnMeasure(RulerObject::Measure(p1, p2));
            float len = RulerObject::UnMeasure(event.GetValue().GetDouble());

            auto diff = p2 - p1;
            float dx = diff.x * len / oldLen - diff.x;
            float dy = diff.y * len / oldLen - diff.y;
            float dz = diff.z * len / oldLen - diff.z;

            // xl::isnan: oldLen==0 produces NaN through the divide above,
            // and std::isnan folds to `false` under -ffinite-math-only
            // (Release -ffast-math) — without this guard, NaN propagates
            // into every subsequent point's position and corrupts the model.
            if (xl::isnan(dx))
                dx = 1.0f;
            if (xl::isnan(dy))
                dy = 1.0f;
            if (xl::isnan(dz))
                dz = 1.0f;

            for (int i = h + 1; i < loc.GetNumPoints(); i++) {
                auto pt = loc.GetPoint(i);
                loc.SetPoint(i, pt + glm::vec3(dx, dy, dz));
            }

            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            return 0;
        }
    }
    else if (name.length() > 6) {
        int h = wxAtoi(name.substr(6, name.length() - 6)) - 1;
        loc.SetSelectedHandle(h);
        loc.SetSelectedSegment(-1);
        auto worldPos = loc.GetWorldPosition();
        auto pt = loc.GetPoint(h);
        if (!loc.IsLocked() && name.find("ModelX") != std::string::npos) {
            loc.SetPoint(h, glm::vec3(event.GetValue().GetDouble() - worldPos.x, pt.y, pt.z));
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            return 0;
        }
        else if (loc.IsLocked() && name.find("ModelX") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!loc.IsLocked() && name.find("ModelY") != std::string::npos) {
            loc.SetPoint(h, glm::vec3(pt.x, event.GetValue().GetDouble() - worldPos.y, pt.z));
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            return 0;
        }
        else if (loc.IsLocked() && name.find("ModelY") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!loc.IsLocked() && name.find("ModelZ") != std::string::npos) {
            loc.SetPoint(h, glm::vec3(pt.x, pt.y, event.GetValue().GetDouble() - worldPos.z));
            loc.AddASAPWork(OutputModelManager::WORK_SCREEN_LOCATION_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            return 0;
        }
        else if (loc.IsLocked() && name.find("ModelZ") != std::string::npos) {
            event.Veto();
            return 0;
        }
    }
    else if ("Locked" == name) {
        loc.SetLocked(event.GetValue().GetBool());
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
