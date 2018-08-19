#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "GridlinesObject.h"
#include "DrawGLUtils.h"

GridlinesObject::GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), line_spacing(50), color(xlColor(0,128, 0)),
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
        width = wxAtof(ModelXml->GetAttribute("GridWidth"));
    }
    if (ModelXml->HasAttribute("GridHeight")) {
        height = wxAtof(ModelXml->GetAttribute("GridHeight"));
    }
}

void GridlinesObject::AddTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty *p = grid->Append(new wxUIntProperty("Line Spacing", "GridLineSpacing", line_spacing));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1024);
    p->SetEditor("SpinCtrl");
}

void GridlinesObject::Draw(DrawGLUtils::xl3Accumulator &va3, bool allowSelected)
{
	va3.PreAlloc(width / line_spacing * 12);
	va3.PreAlloc(height / line_spacing * 12);

	xlColor xaxis = xlColor(128,0,0);
	xlColor zaxis = xlColor(0,0,128);

    float half_width = width / 2.0f;
    float half_height = height / 2.0f;
    for (float i = 0; i <= half_height; i += line_spacing)
	{
		va3.AddVertex(-half_width, 0, i, color);
		va3.AddVertex(half_width, 0, i, color);
		va3.AddVertex(-half_width, 0, -i, color);
		va3.AddVertex(half_width, 0, -i, color);
	}
	va3.AddVertex(-half_width, 0, 0, xaxis);
	va3.AddVertex(half_width, 0, 0, xaxis);

	for (float i = 0; i <= half_width; i += line_spacing)
	{
		va3.AddVertex(i, 0, -half_height, color);
		va3.AddVertex(i, 0, half_height, color);
		va3.AddVertex(-i, 0, -half_height, color);
		va3.AddVertex(-i, 0, half_height, color);
	}
	va3.AddVertex(0, 0, -half_height, zaxis);
	va3.AddVertex(0, 0, half_height, zaxis);

    va3.Finish(GL_LINES);
}

