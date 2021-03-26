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

#include <string>

struct CADPoint
{
	CADPoint() = default;

	CADPoint(float x, float y, float z) :
		X(x), Y(y), Z(z)
	{ }
	float X;
	float Y;
	float Z;
};

struct CADLine
{
	CADPoint Loc1;
	CADPoint Loc2;
};

struct CADText
{
	CADText() = default;
	CADText(float x, float y, float z, std::string text, float size = 1.0F, float rotation = 0.0F) :
		Loc(x, y, z), Text(text), Size(size), Rotation(rotation)
	{ }
	CADPoint Loc;
	std::string Text;
	float Size{ 1.0F };
	float Rotation{ 0.0F };
};