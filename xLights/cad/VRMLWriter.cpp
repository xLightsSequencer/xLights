
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VRMLWriter.h"
#include "CADPoint.h"

#include <wx/string.h>
#include "xLightsVersion.h"

void VRMLWriter::WriteHeader()
{
    m_file << "#VRML V2.0 utf8\n"
        << "WorldInfo {\n"
        << "  title \"Exported triangle mesh to VRML97\"\n"
        << "  info [\"Created by xLights "<< xlights_version_string.ToStdString() <<"\"]\n"
        << "}\n";
}

void VRMLWriter::WriteText(CADPoint const& loc, std::string const& text, float height, uint32_t color, float rotation)
{
	m_file
		<< "Transform {\n"
		<< "  scale 0.25 0.25 0.25\n"
		<< "  rotation 1 0 0 1.57\n"//rotate X pi/2 or 90
		<< "  scaleOrientation 0 0 1 0\n"
		<< "  center 0.000 0.000 0.000\n";

	m_file << "  translation " << loc.X  << " " << (-loc.Z - 0.21) << " " << (loc.Y - 0.20) << "\n";

	m_file
		<< "  children[\n"
		<< "    Shape { \n"
		<< "      appearance\n"
		<< "      Appearance {\n"
		<< "        material\n"
		<< "        Material {\n"
		<< "          diffuseColor 1.000 0.000 0.000\n"//red
		<< "        }\n"
		<< "      }\n"
		<< "      geometry Text{\n"
		<< "        string[\""<< text <<"\"]\n"
		<< "        fontStyle FontStyle { \n"
		<< "        family[\"Serenity\", \"SERIF\"]\n"
		<< "      }\n"
		<< "      length["<< text.length() <<"]\n"
		<< "      }\n"
		<< "    }\n"
		<< "  ]\n"
		<< "}\n";
}

void VRMLWriter::WriteWire(CADPoint const& /*loc1*/, CADPoint const& /*loc2*/, uint32_t /*color*/)
{
	//not implemented
}

void VRMLWriter::WriteNode(CADPoint const& loc, uint32_t /*color*/)
{
				  //R     G     B
	WriteNode(loc, "0.800 0.800 0.800");
}

void VRMLWriter::WriteFirstNode(CADPoint const& loc, uint32_t /*color*/)
{
				  //R     G     B
	WriteNode(loc, "0.000 0.000 1.000");
}

void VRMLWriter::WriteLastNode(CADPoint const& loc, uint32_t /*color*/)
{
				  //R     G     B
	WriteNode(loc, "0.000 1.000 0.000");
}

void VRMLWriter::WriteNode(CADPoint const& loc, std::string const& color)
{
	//Copied from FreeCAD Export
    m_file
		<< "Transform {\n"
		<< "  scale 1 1 1\n"
		<< "  rotation 0 0 1 0\n"
		<< "  scaleOrientation 0 0 1 0\n"
		<< "  center 0.000 0.000 0.000\n";

    m_file << "  translation " << loc.X << " " << -loc.Z << " " << loc.Y << "\n";

	m_file
		<< "  children[\n"
		<< "    Shape { \n"
		<< "      appearance\n"
		<< "      Appearance {\n"
		<< "        material\n"
		<< "        Material {\n"
		<< "          diffuseColor "<< color << "\n"
		<< "        }\n"
		<< "      }\n"
		<< "      geometry\n"
		<< "      IndexedFaceSet {\n"
		<< "        coord\n"
		<< "        Coordinate {\n"
		<< "          point [\n"
		<< "            0.000 -0.200 -0.200,\n"
		<< "            0.000 -0.200 0.200,\n"
		<< "            0.000 0.200 -0.200,\n"
		<< "            0.000 0.200 0.200,\n"
		<< "            0.400 -0.200 -0.200,\n"
		<< "            0.400 -0.200 0.200,\n"
		<< "            0.400 0.200 -0.200,\n"
		<< "            0.400 0.200 0.200\n"
		<< "          ]\n"
		<< "        }\n"
		<< "        coordIndex [\n"
		<< "          5, 4, 6, -1,\n"
		<< "          7, 5, 6, -1,\n"
		<< "          0, 1, 2, -1,\n"
		<< "          1, 3, 2, -1,\n"
		<< "          0, 2, 6, -1,\n"
		<< "          4, 0, 6, -1,\n"
		<< "          3, 1, 7, -1,\n"
		<< "          1, 5, 7, -1,\n"
		<< "          0, 4, 5, -1,\n"
		<< "          1, 0, 5, -1,\n"
		<< "          6, 2, 7, -1,\n"
		<< "          2, 3, 7, -1\n"
		<< "        ]\n"
		<< "      }\n"
		<< "    }\n"
		<< "  ]\n"
		<< "}\n";
}

