#ifndef CADWRITER_H
#define CADWRITER_H

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
#include <fstream>

#include "CADPoint.h"

struct CADWriter
{
	CADWriter(std::string fileName);

	virtual ~CADWriter();

	virtual bool MapToScreen() { return true; }

	virtual bool Open();

	virtual void WriteHeader() { };
	virtual void WriteEndOfFile() { };

	virtual void WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation = 0.0 ) = 0;
	virtual void WriteWire(CADPoint const& loc1 , CADPoint const& loc2, uint32_t color) = 0;
	virtual void WriteNode(CADPoint const& loc, uint32_t color) = 0;

	virtual void WriteFirstNode(CADPoint const& loc, uint32_t color);
	virtual void WriteLastNode(CADPoint const& loc, uint32_t color);

	virtual void WriteTextHeader() { };
	virtual void WriteTextFooter() { };
	virtual void WriteWireHeader() { };
	virtual void WriteWireFooter() { };
	virtual void WriteNodeHeader() { };
	virtual void WriteNodeFooter() { };

protected:
	const std::string m_fileName;
	std::ofstream m_file;
};

#endif
