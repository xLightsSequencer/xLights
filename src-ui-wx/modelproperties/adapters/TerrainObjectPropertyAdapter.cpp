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
#include <wx/settings.h>

#include "TerrainObjectPropertyAdapter.h"
#include "models/TerrainObject.h"
#include "models/RulerObject.h"
#include "models/ModelScreenLocation.h"
#include "utils/ExternalHooks.h"
#include "shared/utils/wxUtilities.h"

TerrainObjectPropertyAdapter::TerrainObjectPropertyAdapter(TerrainObject& obj)
    : ViewObjectPropertyAdapter(obj), _terrain(obj) {}

void TerrainObjectPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxImageFileProperty("Image",
        "Image",
        _terrain.GetImageFile()));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
                                        "|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", _terrain.GetTransparency()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", _terrain.GetBrightness()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Line Spacing", "TerrainLineSpacing", _terrain.GetSpacing()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1024);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Terrain Width", "TerrainWidth", _terrain.GetWidth()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Terrain depth", "TerrainDepth", _terrain.GetDepth()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxColourProperty("Grid Color", "gridColor", xlColorToWxColour(_terrain.GetGridColorObj())));

    p = grid->Append(new wxBoolProperty("Hide Grid", "HideGrid", _terrain.IsHideGrid()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Hide Image", "HideImage", _terrain.IsHideImage()));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Brush Size", "TerrainBrushSize", _terrain.GetBrushSize()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Edit Terrain", "TerrainEdit", _terrain.IsEditTerrain()));
    p->SetAttribute("UseCheckbox", true);

    if (RulerObject::GetRuler() != nullptr) {
        p = grid->Append(new wxStringProperty("Terrain Spacing", "RealSpacing",
            RulerObject::PrescaledMeasureDescription(RulerObject::Measure(_terrain.GetSpacing()))
        ));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

int TerrainObjectPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("Image" == event.GetPropertyName()) {
        _terrain.ClearImages();
        std::string imageFile = event.GetValue().GetString().ToStdString();
        _terrain.SetImageFileDirect(imageFile);
        ObtainAccessToURL(imageFile);
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Image");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Image");
        return 0;
    } else if ("Transparency" == event.GetPropertyName()) {
        _terrain.SetTransparency((int)event.GetPropertyValue().GetLong());
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Transparency");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        _terrain.SetBrightness((int)event.GetPropertyValue().GetLong());
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Brightness");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Brightness");
        return 0;
    } else if ("TerrainLineSpacing" == event.GetPropertyName()) {
        _terrain.SetSpacing((int)event.GetPropertyValue().GetLong());
        _terrain.UpdateSize();
        if (grid->GetPropertyByName("RealSpacing") != nullptr && RulerObject::GetRuler() != nullptr) {
            grid->GetPropertyByName("RealSpacing")->SetValueFromString(
                RulerObject::PrescaledMeasureDescription(RulerObject::Measure(_terrain.GetSpacing())));
        }
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrainLineSpacing");
        _terrain.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrainLineSpacing");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrainLineSpacing");
        return 0;
    } else if ("TerrainWidth" == event.GetPropertyName()) {
        _terrain.SetWidth((int)event.GetPropertyValue().GetLong());
        _terrain.UpdateSize();
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrainWidth");
        _terrain.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrainWidth");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrainWidth");
        return 0;
    } else if ("TerrainDepth" == event.GetPropertyName()) {
        _terrain.SetDepth((int)event.GetPropertyValue().GetLong());
        _terrain.UpdateSize();
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrainDepth");
        _terrain.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrainDepth");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrainDepth");
        return 0;
    } else if ("gridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("gridColor");
        wxColour c;
        c << p->GetValue();
        xlColor xc = wxColourToXlColor(c);
        _terrain.SetGridColor(xc);
        p->SetValue(WXVARIANT(xlColorToWxColour(xc)));
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::gridColor");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::gridColor");
        return 0;
    } else if ("HideGrid" == event.GetPropertyName()) {
        _terrain.SetHideGrid(event.GetValue().GetBool());
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::HideGrid");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::HideGrid");
        return 0;
    } else if ("HideImage" == event.GetPropertyName()) {
        _terrain.SetHideImage(event.GetValue().GetBool());
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::HideImage");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::HideImage");
        return 0;
    } else if ("TerrainBrushSize" == event.GetPropertyName()) {
        int brushSize = (int)event.GetPropertyValue().GetLong();
        _terrain.SetBrushSize(brushSize);
        _terrain.GetObjectScreenLocation().SetToolSize(brushSize);
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrainBrushSize");
        return 0;
    } else if (event.GetPropertyName() == "TerrainEdit") {
        bool edit = event.GetValue().GetBool();
        _terrain.SetEditTerrain(edit);
        if (edit) {
            _terrain.GetObjectScreenLocation().SetActiveHandle(std::nullopt);
            _terrain.GetObjectScreenLocation().SetEdit(true);
        } else {
            _terrain.GetObjectScreenLocation().SetActiveHandleToCentre();
            _terrain.GetObjectScreenLocation().SetAxisTool(ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE);
            _terrain.GetObjectScreenLocation().SetEdit(false);
        }
        _terrain.IncrementChangeCount();
        _terrain.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrainEdit");
        _terrain.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrainEdit");
        _terrain.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrainEdit");
        return 0;
    }

    return ViewObjectPropertyAdapter::OnPropertyGridChange(grid, event);
}
