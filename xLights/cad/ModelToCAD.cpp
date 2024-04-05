/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelToCAD.h"
#include "CADModel.h"
#include "CADWriter.h"
#include "DXFWriter.h"
#include "STLWriter.h"
#include "VRMLWriter.h"

#include "../models/Model.h"
#include "../models/ModelManager.h"

namespace ModelToCAD
{
	std::unique_ptr<CADWriter> Create(std::string fileName, std::string const& type)
	{
		if (type == "dxf") {
			return std::make_unique<DXFWriter>(std::move(fileName));
		} else if (type == "stl") {
			return std::make_unique<STLWriter>(std::move(fileName));
		} else if (type == "wrl") {
			return std::make_unique<VRMLWriter>(std::move(fileName));
		}
		return nullptr;
	}

	bool ExportCAD(Model* m, std::string filePath, std::string const& type)
	{
		auto cadFile = Create(std::move(filePath), type);
		if (!cadFile || !cadFile->Open()) {
			return false;
		}
		cadFile->WriteHeader();
		CADModel model;
		model.LoadModel(m, cadFile->MapToScreen(), true, false);
		model.WriteToCAD(cadFile.get());

		cadFile->WriteEndOfFile();
		return true;
	}

	//TODO: Make full layout export work with STL and VRML files
	bool ExportCAD(ModelManager* allmodels, std::string filePath, std::string const& type, std::string const& layout)
	{
		auto cadFile = Create(std::move(filePath), type);
		if (!cadFile || !cadFile->Open()) {
			return false;
		}
		cadFile->WriteHeader();
		for (auto m = allmodels->begin(); m != allmodels->end(); ++m) {
			Model* mm = m->second;

			if (mm->GetLayoutGroup() != layout) {
				continue;
			}

			if (mm->GetDisplayAs() == "ModelGroup") {
				continue;
			}

			CADModel model;
			model.LoadModel(mm, cadFile->MapToScreen(), false, true);
			model.WriteToCAD(cadFile.get());
		}

		cadFile->WriteEndOfFile();
		return true;
	}
};

