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

// https://en.wikipedia.org/wiki/VRML
//Text strings are copied from a FreeCAD Export

#include "CADWriter.h"

struct VRMLWriter : CADWriter
{
	using CADWriter::CADWriter;

	bool MapToScreen() override { return false; };

	void WriteHeader() override;
	void WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation = 0.0 ) override;
	void WriteWire(CADPoint const& loc1, CADPoint const& loc2, uint32_t color ) override;
	void WriteNode(CADPoint const& loc, uint32_t color) override;
	void WriteFirstNode(CADPoint const& loc, uint32_t color) override;
	void WriteLastNode(CADPoint const& loc, uint32_t color) override;

	void WriteNode(CADPoint const& loc, std::string const& color);
};