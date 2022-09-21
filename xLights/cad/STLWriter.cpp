
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "STLWriter.h"
#include "CADPoint.h"

void STLWriter::WriteHeader()
{
	m_file << "solid xLights\n";
}

void STLWriter::WriteEndOfFile()
{
	WriteTriangles();
	m_file << "endsolid xLights\n";
}

void STLWriter::WriteText(CADPoint const& /*loc*/, std::string const& /*text*/, float /*height*/, uint32_t /*color*/, float /*rotation*/)
{
	//not implemented
}

void STLWriter::WriteWire(CADPoint const& /*loc1*/, CADPoint const& /*loc2*/, uint32_t /*color*/)
{
	//not implemented
}

void STLWriter::WriteNode(CADPoint const& loc, uint32_t /*color*/)
{
	WriteNode(loc, 0.2f);
}

void STLWriter::WriteFirstNode(CADPoint const& loc, uint32_t /*color*/)
{
	WriteNode(loc, 0.4f);
}

void STLWriter::WriteNode(CADPoint const& loc, float offset)
{
	//draw nodes as cubes, TODO: switch to Icosahedron http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
	//12 triangles, 2 per face, 8 vertices, 6 directions

	//swap Z and Y because xLights used different axises

	STLPoint bfl(loc.X - offset, -loc.Z - offset, loc.Y - offset);
	STLPoint bfr(loc.X - offset, -loc.Z + offset, loc.Y - offset);
	STLPoint tfl(loc.X + offset, -loc.Z - offset, loc.Y - offset);
	STLPoint tfr(loc.X + offset, -loc.Z + offset, loc.Y - offset);

	STLPoint brl(loc.X - offset, -loc.Z - offset, loc.Y + offset);
	STLPoint brr(loc.X - offset, -loc.Z + offset, loc.Y + offset);
	STLPoint trl(loc.X + offset, -loc.Z - offset, loc.Y + offset);
	STLPoint trr(loc.X + offset, -loc.Z + offset, loc.Y + offset);

	STLPoint top(1, 0, 0);
	STLPoint bot(-1, 0, 0);
	STLPoint left(0, -1, 0);
	STLPoint right(0, 1, 0);
	STLPoint frnt(0, 0, -1);
	STLPoint rear(0, 0, 1);

	AddTriangle(STLTriangle(trl, tfl, tfr, top));
	AddTriangle(STLTriangle(trr, trl, tfr, top));

	AddTriangle(STLTriangle(bfl, brl, bfr, bot));
	AddTriangle(STLTriangle(brl, brr, bfr, bot));

	AddTriangle(STLTriangle(bfl, bfr, tfr, frnt));
	AddTriangle(STLTriangle(tfl, bfl, tfr, frnt));

	AddTriangle(STLTriangle(brr, brl, trr, rear));
	AddTriangle(STLTriangle(brl, trl, trr, rear));

	AddTriangle(STLTriangle(bfl, tfl, trl, left));
	AddTriangle(STLTriangle(brl, bfl, trl, left));

	AddTriangle(STLTriangle(tfr, bfr, trr, right));
	AddTriangle(STLTriangle(bfr, brr, trr, right));
}

void STLWriter::AddTriangle(STLTriangle const& triangle)
{
	m_triangle.push_back(triangle);
}

void STLWriter::WriteTriangles()
{
	for (auto const& tri : m_triangle) {
		m_file << tri.ExportToString();
	}
}