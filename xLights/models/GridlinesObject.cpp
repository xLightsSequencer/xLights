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

#include "GridlinesObject.h"
#include "ModelPreview.h"
#include "Model.h"
#include "RulerObject.h"

GridlinesObject::GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), gridColor(xlColor(0,128, 0))
{
    SetFromXml(node);
}

GridlinesObject::~GridlinesObject()
{
}

void GridlinesObject::InitModel() {
    if (ModelXml->HasAttribute("GridLineSpacing")) {
        line_spacing = wxAtoi(ModelXml->GetAttribute("GridLineSpacing"));
    }
    if (ModelXml->HasAttribute("GridWidth")) {
        width = wxAtoi(ModelXml->GetAttribute("GridWidth"));
    }
    if (ModelXml->HasAttribute("GridHeight")) {
        height = wxAtoi(ModelXml->GetAttribute("GridHeight"));
    }
    if (ModelXml->HasAttribute("GridColor")) {
        gridColor = xlColor(ModelXml->GetAttribute("GridColor", "#000000").ToStdString());
    }
    hasAxis = ModelXml->GetAttribute("GridAxis", "0") == "1";
    pointToFront = ModelXml->GetAttribute("PointToFront", "0") == "1";
}

void GridlinesObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    wxPGProperty *p = grid->Append(new wxUIntProperty("Line Spacing", "GridLineSpacing", line_spacing));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1024);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Grid Width", "GridWidth", width));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Grid Height", "GridHeight", height));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxColourProperty("Grid Color", "GridColor", gridColor.asWxColor()));

    p = grid->Append(new wxBoolProperty("Axis Lines", "GridAxis", hasAxis));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Point To Front", "PointToFront", hasAxis));
    p->SetAttribute("UseCheckbox", true);

    if (RulerObject::GetRuler() != nullptr) {
        p = grid->Append(new wxStringProperty("Grid Spacing", "RealSpacing",
            RulerObject::PrescaledMeasureDescription(RulerObject::Measure(line_spacing))
        ));
        p->ChangeFlag(wxPG_PROP_READONLY, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

int GridlinesObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("GridLineSpacing" == event.GetPropertyName()) {
        line_spacing = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridLineSpacing");
        ModelXml->AddAttribute("GridLineSpacing", wxString::Format("%d", line_spacing));
        if (grid->GetPropertyByName("RealSpacing") != nullptr && RulerObject::GetRuler() != nullptr) {
            grid->GetPropertyByName("RealSpacing")->SetValueFromString(RulerObject::PrescaledMeasureDescription(RulerObject::Measure(line_spacing)));
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        return 0;
    }
    else if ("GridWidth" == event.GetPropertyName()) {
        width = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridWidth");
        ModelXml->AddAttribute("GridWidth", wxString::Format("%d", width));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridWidth");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridWidth");
        return 0;
    }
    else if ("GridHeight" == event.GetPropertyName()) {
        height = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridHeight");
        ModelXml->AddAttribute("GridHeight", wxString::Format("%d", height));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridHeight");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridHeight");
        return 0;
    }
    else if ("GridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("GridColor");
        wxColour c;
        c << p->GetValue();
        gridColor = c;
        ModelXml->DeleteAttribute("GridColor");
        ModelXml->AddAttribute("GridColor", gridColor);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridColor");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridColor");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridColor");
        return 0;
    } else if (event.GetPropertyName() == "GridAxis") {
        ModelXml->DeleteAttribute("GridAxis");
        hasAxis = event.GetValue().GetBool();
        if (hasAxis) {
            ModelXml->AddAttribute("GridAxis", "1");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridAxis");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridAxis");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridAxis");
        return 0;
    } else if (event.GetPropertyName() == "PointToFront") {
        ModelXml->DeleteAttribute("PointToFront");
        pointToFront = event.GetValue().GetBool();
        if (pointToFront) {
            ModelXml->AddAttribute("PointToFront", "1");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::PointToFront");
        // AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridAxis");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::PointToFront");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool GridlinesObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }
    
    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);
    if (uiObjectsInvalid || grid == nullptr || createdRotation != GetObjectScreenLocation().GetRotation()) {
        if (grid) {
            delete grid;
        }
        grid = ctx->createVertexColorAccumulator();
        grid->SetName(GetName() + "_GridLines");

        float sx,sy,sz;

        grid->PreAlloc(width / line_spacing * 12 + height / line_spacing * 12);
        screenLocation.SetRenderSize(width, height, 10.0f);

        xlColor xaxis = xlColor(128,0,0);
        xlColor yaxis = xlColor(0,0,128);

        float half_width = width / 2.0f;
        float half_height = height / 2.0f;
        for (float i = 0; i <= half_height; i += line_spacing) {
            if (hasAxis && i == 0) {
                sx = -half_width; sy = 0; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, xaxis);

                sx = half_width; sy = 0; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, xaxis);
            } else {
                sx = -half_width; sy = i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = half_width; sy = i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -half_width; sy = -i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = half_width; sy = -i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);
            }
        }
        for (float i = 0; i <= half_width; i += line_spacing) {
            if (hasAxis && i == 0) {
                sx = 0; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, yaxis);

                sx = 0; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, yaxis);
            } else {
                sx = i; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = i; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -i; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -i; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);
            }
        }

        if (pointToFront) {
            createdRotation = GetObjectScreenLocation().GetRotation();

            GetObjectScreenLocation().SetRotation(glm::vec3(-90.0f, -90.0f, -90.0f));

            // draw an arrow in the middle pointing to the front

            xlColor arrowColour = gridColor.ContrastColourNotBlack();

            sx = 0;
            sy = 0;
            sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = 0.5 * line_spacing;
            sy = 0;
            sx = 1.5 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = -0.5 * line_spacing;
            sy = 0;
            sx = 1.5 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            GetObjectScreenLocation().SetRotation(createdRotation);
        }

        grid->Finalize(false, false);
        GetObjectScreenLocation().UpdateBoundingBox(width, height, 5.0f);
        uiObjectsInvalid = false;
    }
    
    solid->addStep([solid, this](xlGraphicsContext *ctx) {
        ctx->drawLines(grid);
    });
    
    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}
