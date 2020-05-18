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
#include "DrawGLUtils.h"
#include "ModelPreview.h"
#include "Model.h"

GridlinesObject::GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), line_spacing(50), gridColor(xlColor(0,128, 0)),
   width(1000.0f), height(1000.0f), hasAxis(false)
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
}

void GridlinesObject::AddTypeProperties(wxPropertyGridInterface *grid) {

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
}

int GridlinesObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("GridLineSpacing" == event.GetPropertyName()) {
        line_spacing = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridLineSpacing");
        ModelXml->AddAttribute("GridLineSpacing", wxString::Format("%d", line_spacing));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridLineSpacing");
        return 0;
    }
    else if ("GridWidth" == event.GetPropertyName()) {
        width = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridWidth");
        ModelXml->AddAttribute("GridWidth", wxString::Format("%d", width));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridWidth");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridWidth");
        return 0;
    }
    else if ("GridHeight" == event.GetPropertyName()) {
        height = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridHeight");
        ModelXml->AddAttribute("GridHeight", wxString::Format("%d", height));
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridAxis");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::GridAxis");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::GridAxis");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

void GridlinesObject::Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, DrawGLUtils::xl3Accumulator &tva3, bool allowSelected)
{
    if (!IsActive()) { return; }

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    float sx,sy,sz;

	va3.PreAlloc(width / line_spacing * 12);
	va3.PreAlloc(height / line_spacing * 12);

    screenLocation.SetRenderSize(width, height, 10.0f);

	xlColor xaxis = xlColor(128,0,0);
	xlColor yaxis = xlColor(0,0,128);

    float half_width = width / 2.0f;
    float half_height = height / 2.0f;
    for (float i = 0; i <= half_height; i += line_spacing)
    {
        if (hasAxis && i == 0)
        {
            // dont draw
        }
        else
        {
            sx = -half_width; sy = i; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = half_width; sy = i; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = -half_width; sy = -i; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = half_width; sy = -i; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);
        }
    }

	if( hasAxis ) {
        sx = -half_width; sy = 0; sz = 0;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
        va3.AddVertex(sx, sy, sz, xaxis);

        sx = half_width; sy = 0; sz = 0;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
        va3.AddVertex(sx, sy, sz, xaxis);
	}

    for (float i = 0; i <= half_width; i += line_spacing)
    {
        if (hasAxis && i == 0)
        {
            // dont draw
        }
        else
        {
            sx = i; sy = -half_height; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = i; sy = half_height; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = -i; sy = -half_height; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);

            sx = -i; sy = half_height; sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            va3.AddVertex(sx, sy, sz, gridColor);
        }
    }
	if( hasAxis ) {
        sx = 0; sy = -half_height; sz = 0;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
        va3.AddVertex(sx, sy, sz, yaxis);

        sx = 0; sy = half_height; sz = 0;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
        va3.AddVertex(sx, sy, sz, yaxis);
	}

    va3.Finish(GL_LINES, GL_LINE_SMOOTH);
    GetObjectScreenLocation().UpdateBoundingBox(width, height, 5.0f);  // FIXME: Modify to only call this when position changes

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(va3, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}
