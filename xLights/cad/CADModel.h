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
#include <vector>

class Model;
struct CADWriter;
struct CADPoint;
struct CADLine;
struct CADText;

class CADModel
{
public:
	void LoadModel(Model * model, bool twoD, bool addNodeNumbers, bool addConnections);
	void ShiftModel(float x , float y, float z);
	void WriteToCAD(CADWriter* writer );

private:
	std::vector<CADPoint> m_nodes;
	std::vector<CADLine> m_wires;
	std::vector<CADText> m_texts;
	uint32_t m_color;
};