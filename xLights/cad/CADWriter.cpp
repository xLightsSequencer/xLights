
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CADWriter.h"


#include "UtilFunctions.h"

CADWriter::CADWriter(std::string fileName) :
	m_fileName(std::move(fileName))
{
}

CADWriter::~CADWriter()
{
	m_file.close();
}

bool CADWriter::Open()
{
	m_file.open(m_fileName.c_str(), std::ios::out);
	if (!m_file.good()) {
		DisplayError("Unable to create file : " + m_fileName);
		return false;
	}
	return true;
}

void CADWriter::WriteFirstNode(CADPoint const& loc, uint32_t color) {
	WriteNode(loc,color);
}

void CADWriter::WriteLastNode(CADPoint const& loc, uint32_t color)
{
	WriteNode(loc, color);
}