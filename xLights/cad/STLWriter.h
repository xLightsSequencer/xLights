#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// https://en.wikipedia.org/wiki/STL_(file_format)

#include "CADWriter.h"
#include "CADPoint.h"
#include <vector>
#include <sstream>

struct STLWriter : CADWriter
{
	using CADWriter::CADWriter;

	bool MapToScreen() override { return false; };

	void WriteHeader() override;
	void WriteEndOfFile() override;
	void WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation = 0.0) override;
	void WriteWire(CADPoint const& loc1, CADPoint const& loc2, uint32_t color ) override;

	void WriteFirstNode(CADPoint const& loc, uint32_t color) override;
	void WriteNode(CADPoint const& loc, uint32_t color ) override;
	void WriteNode(CADPoint const& loc, float offset);

private:
	struct STLPoint : CADPoint
	{
		using CADPoint::CADPoint;
		std::string asString() const
		{
			return std::to_string(X) + " " +
				std::to_string(Y) + " " +
				std::to_string(Z);
		}
	};
	struct STLTriangle
	{
		STLTriangle(STLPoint v1, STLPoint v2, STLPoint v3, STLPoint d) :
			vtx1(v1), vtx2(v2), vtx3(v3), dir(d) {};
		STLPoint vtx1;
		STLPoint vtx2;
		STLPoint vtx3;
		STLPoint dir;
		std::string ExportToString() const
		{
			std::stringstream ss;
			ss << "  facet normal "
				<< dir.asString() << "\n"
				<< "    outer loop\n"
				<< "      vertex " << vtx1.asString() << "\n"
				<< "      vertex " << vtx2.asString() << "\n"
				<< "      vertex " << vtx3.asString() << "\n"
				<< "    endloop\n"
				<< "  endfacet\n";
			return ss.str();
		}
	};
	std::vector<STLTriangle> m_triangle;

	void AddTriangle(STLTriangle const& triangle);
	void WriteTriangles();
};