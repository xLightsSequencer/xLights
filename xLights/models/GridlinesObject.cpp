#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "GridlinesObject.h"
#include "DrawGLUtils.h"

GridlinesObject::GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), line_spacing(50), gridColor(xlColor(0,128, 0)),
   width(1000.0f), height(1000.0f)
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

    p = grid->Append(new wxColourProperty("Grid Color", "GridColor", gridColor.asWxColor()));
}

int GridlinesObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("GridLineSpacing" == event.GetPropertyName()) {
        line_spacing = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridLineSpacing");
        ModelXml->AddAttribute("GridLineSpacing", wxString::Format("%d", line_spacing));
        //SetFromXml(ModelXml);
        return 3 | 0x0008;
    }
    else if ("GridWidth" == event.GetPropertyName()) {
        width = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridWidth");
        ModelXml->AddAttribute("GridWidth", wxString::Format("%d", width));
        //SetFromXml(ModelXml);
        return 3 | 0x0008;
    }
    else if ("GridHeight" == event.GetPropertyName()) {
        height = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("GridHeight");
        ModelXml->AddAttribute("GridHeight", wxString::Format("%d", height));
        //SetFromXml(ModelXml);
        return 3 | 0x0008;
    }
    else if ("GridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("GridColor");
        wxColour c;
        c << p->GetValue();
        gridColor = c;
        ModelXml->DeleteAttribute("GridColor");
        ModelXml->AddAttribute("GridColor", gridColor);
        //SetFromXml(ModelXml);
        return 3 | 0x0008;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

void GridlinesObject::Draw(DrawGLUtils::xl3Accumulator &va3, bool allowSelected)
{
    float sx,sy,sz;

	va3.PreAlloc(width / line_spacing * 12);
	va3.PreAlloc(height / line_spacing * 12);

	xlColor xaxis = xlColor(128,0,0);
	xlColor zaxis = xlColor(0,0,128);

    float half_width = width / 2.0f;
    float half_height = height / 2.0f;
    for (float i = 0; i <= half_height; i += line_spacing)
	{
	    sx = -half_width; sy = 0; sz = i;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

        sx = half_width; sy = 0; sz = i;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

        sx = -half_width; sy = 0; sz = -i;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

        sx = half_width; sy = 0; sz = -i;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);
	}

    sx = -half_width; sy = 0; sz = 0;
    GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
    va3.AddVertex(sx, sy, sz, xaxis);

    sx = half_width; sy = 0; sz = 0;
    GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
    va3.AddVertex(sx, sy, sz, xaxis);

	for (float i = 0; i <= half_width; i += line_spacing)
	{
	    sx = i; sy = 0; sz = -half_height;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

	    sx = i; sy = 0; sz = half_height;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

	    sx = -i; sy = 0; sz = -half_height;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);

	    sx = -i; sy = 0; sz = half_height;
        GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
		va3.AddVertex(sx, sy, sz, gridColor);
	}
    sx = 0; sy = 0; sz = -half_height;
    GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
    va3.AddVertex(sx, sy, sz, zaxis);

    sx = 0; sy = 0; sz = half_height;
    GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
    va3.AddVertex(sx, sy, sz, zaxis);

    va3.Finish(GL_LINES);
}

