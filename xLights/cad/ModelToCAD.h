#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>
#include <memory>

class Model;
class ModelManager;
struct CADWriter;

namespace ModelToCAD
{
	std::unique_ptr<CADWriter> Create(std::string fileName, std::string const& type);

	bool ExportCAD(Model* m, std::string filePath, std::string const& type);

	bool ExportCAD(ModelManager* allmodels, std::string filePath, std::string const& type, std::string const& layout);
};
