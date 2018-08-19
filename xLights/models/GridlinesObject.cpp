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

	float half_size = width / 2.0f;
	for (float i = 0; i <= half_size; i += line_spacing)
	{
		va3.AddVertex(-half_size, 0, i, color);
		va3.AddVertex(half_size, 0, i, color);
		va3.AddVertex(-half_size, 0, -i, color);
		va3.AddVertex(half_size, 0, -i, color);
	}
	va3.AddVertex(-half_size, 0, 0, xaxis);
	va3.AddVertex(half_size, 0, 0, xaxis);

	half_size = height / 2.0f;
	for (float i = 0; i <= half_size; i += line_spacing)
	{
		va3.AddVertex(i, 0, -half_size, color);
		va3.AddVertex(i, 0, half_size, color);
		va3.AddVertex(-i, 0, -half_size, color);
		va3.AddVertex(-i, 0, half_size, color);
	}
	va3.AddVertex(0, 0, -half_size, zaxis);
	va3.AddVertex(0, 0, half_size, zaxis);

    va3.Finish(GL_LINES);
}

