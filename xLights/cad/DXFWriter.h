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

//DXF Spec
//https://images.autodesk.com/adsk/files/autocad_2012_pdf_dxf-reference_enu.pdf

//Used as reference
//https://github.com/okosan/XDxfGen

// "LAYER 0" is default layer
// "Thickness is optional, defaults to 1"
// if no units are defined, spec defaults to English, I think - SEH
// rotation and angle vales are 0-360

//TODO: Add Header info, units, dxf version,
//TODO: thickness seems broken

#include "CADWriter.h"

struct CADPoint;

struct DXFWriter : CADWriter
{
	using CADWriter::CADWriter;

	void WriteHeader() override;
	void WriteEndOfFile() override;

	void WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation = 0.0) override;
	void WriteWire(CADPoint const& loc1, CADPoint const& loc2, uint32_t color) override;
	void WriteNode(CADPoint const& loc, uint32_t color) override;

private:
	float m_scale{ 1.0F };

	void DrawText(float x, float y, float z, std::string const& text, float height, float rotation, int thickness, int layer, uint32_t color);
	void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, int thickness, int layer , uint32_t color);
	void DrawCircle(float x, float y, float z, float radius, int thickness, int layer, uint32_t color );
	void DrawArc(float x, float y, float z, float radius, float startAngle, float endAngle, int thickness, int layer, uint32_t color);
	void DrawPoint(float x, float y, float z, int thickness, int layer, uint32_t color);

	void SetScale(float scale) {
		m_scale = scale;
	};
};