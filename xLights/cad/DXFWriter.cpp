
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DXFWriter.h"
#include "CADPoint.h"

void DXFWriter::WriteHeader()
{
	m_file << "0\n";
	m_file << "SECTION\n";
	m_file << "2\n";
	m_file << "ENTITIES\n";
}

void DXFWriter::WriteEndOfFile()
{
	m_file << "0\n";
	m_file << "ENDSEC\n";
	m_file << "0\n";
	m_file << "EOF\n";
}

void DXFWriter::WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation)
{
	DrawText(loc.X, loc.Y, loc.Z, text, height, rotation, 1, 1, color);
}

void DXFWriter::WriteWire(CADPoint const& loc1, CADPoint const& loc2, uint32_t color)
{
	DrawLine(loc1.X, loc1.Y, loc1.Z, loc2.X, loc2.Y, loc2.Z, 1, 2, color);
}

void DXFWriter::WriteNode(CADPoint const& loc, uint32_t color)
{
	DrawCircle(loc.X, loc.Y, loc.Z, 0.2f, 1, 0, color);
}

void DXFWriter::DrawText(float x, float y, float z, std::string const& text, float height, float rotation, int thickness, int layer, uint32_t color)
{
	m_file << "0\n";
	m_file << "TEXT\n";
	m_file << "8\n";
	m_file << std::to_string(layer) << '\n';

	if (thickness != 1) {
		m_file << "39\n";
		m_file << std::to_string(thickness) << '\n';
	}

	m_file << "10\n";
	m_file << std::to_string(x * m_scale) << '\n';
	m_file << "20\n";
	m_file << std::to_string(y * m_scale) << '\n';
	m_file << "30\n";
	m_file << std::to_string(z * m_scale) << '\n';
	m_file << "1\n";
	m_file << text << '\n';
	m_file << "40\n";
	m_file << std::to_string(height) << '\n';

	if (rotation != 0) {
		m_file << "50\n";
		m_file << std::to_string(rotation) << '\n';
	}

	if (color != 0) {
		m_file << "420\n";
		m_file << std::to_string(color) << '\n';
	}
}

void DXFWriter::DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, int thickness, int layer, uint32_t color)
{
	m_file << "0\n";
	m_file << "LINE\n";
	m_file << "8\n";
	m_file << std::to_string(layer) << '\n';

	if (thickness != 1) {
		m_file << "39\n";
		m_file << std::to_string(thickness) << '\n';
	}
	m_file << "10\n";
	m_file << std::to_string(x1 * m_scale) << '\n';
	m_file << "20\n";
	m_file << std::to_string(y1 * m_scale) << '\n';
	m_file << "30\n";
	m_file << std::to_string(z1 * m_scale) << '\n';

	m_file << "11\n";
	m_file << std::to_string(x2 * m_scale) << '\n';
	m_file << "21\n";
	m_file << std::to_string(y2 * m_scale) << '\n';
	m_file << "31\n";
	m_file << std::to_string(z2 * m_scale) << '\n';

	if (color != 0) {
		m_file << "420\n";
		m_file << std::to_string(color) << '\n';
	}
}

void DXFWriter::DrawCircle(float x, float y, float z, float radius, int thickness, int layer, uint32_t color)
{
	m_file << "0\n";
	m_file << "CIRCLE\n";
	m_file << "8\n";
	m_file << std::to_string(layer) << '\n';

	if (thickness != 1) {
		m_file << "39\n";
		m_file << std::to_string(thickness) << '\n';
	}
	m_file << "10\n";
	m_file << std::to_string(x * m_scale) << '\n';
	m_file << "20\n";
	m_file << std::to_string(y * m_scale) << '\n';
	m_file << "30\n";
	m_file << std::to_string(z * m_scale) << '\n';
	m_file << "40\n";
	m_file << std::to_string(radius * m_scale) << '\n';

	if (color != 0) {
		m_file << "420\n";
		m_file << std::to_string(color) << '\n';
	}
}

void DXFWriter::DrawArc(float x, float y, float z, float radius, float startAngle, float endAngle, int thickness, int layer, uint32_t color)
{
	m_file << "0\n";
	m_file << "CIRCLE\n";
	m_file << "8\n";
	m_file << std::to_string(layer) << '\n';

	if (thickness != 1) {
		m_file << "39\n";
		m_file << std::to_string(thickness) << '\n';
	}
	m_file << "10\n";
	m_file << std::to_string(x * m_scale) << '\n';
	m_file << "20\n";
	m_file << std::to_string(y * m_scale) << '\n';
	m_file << "30\n";
	m_file << std::to_string(z * m_scale) << '\n';
	m_file << "40\n";
	m_file << std::to_string(radius * m_scale) << '\n';
	m_file << "50\n";
	m_file << std::to_string(startAngle) << '\n';
	m_file << "51\n";
	m_file << std::to_string(endAngle) << '\n';

	if (color != 0) {
		m_file << "420\n";
		m_file << std::to_string(color) << '\n';
	}
}

void DXFWriter::DrawPoint(float x, float y, float z, int thickness, int layer, uint32_t color )
{
	m_file << "0\n";
	m_file << "POINT\n";
	m_file << "8\n";
	m_file << std::to_string(layer) << '\n';

	if (thickness != 1) {
		m_file << "39\n";
		m_file << std::to_string(thickness) << '\n';
	}
	m_file << "10\n";
	m_file << std::to_string(x * m_scale) << '\n';
	m_file << "20\n";
	m_file << std::to_string(y * m_scale) << '\n';
	m_file << "30\n";
	m_file << std::to_string(z * m_scale) << '\n';

	if (color != 0) {
		m_file << "420\n";
		m_file << std::to_string(color) << '\n';
	}
}