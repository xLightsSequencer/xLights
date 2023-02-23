/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "CADModel.h"
#include "CADWriter.h"
#include "CADPoint.h"

#include "../models/Model.h"
#include "../Pixels.h"
#include "../Color.h"

#include <algorithm>

void CADModel::LoadModel(Model* model, bool twoD, bool addNodeNumbers, bool addConnections)
{
	float minX = FLT_MAX;
	float minY = FLT_MAX;
	float minZ = FLT_MAX;
	for (size_t i = 0; i < model->GetNodeCount(); i++) {
		std::vector<std::tuple<float, float, float>> pts;
		model->GetNode3DScreenCoords(i, pts);

		for (auto [x,y,z] : pts) {
			minX = std::min(minX, x);
			minY = std::min(minY, y);
			minZ = std::min(minZ, z);

			if (twoD) {
				model->GetModelScreenLocation().TranslatePoint(x, y, z);
			}

			m_nodes.push_back(CADPoint(x, y, z));
			if (addNodeNumbers) {
				m_texts.push_back(CADText(x, y, z, std::to_string(i + 1), 1.0F));
			}
		}
	}
	if (twoD) {
		model->GetModelScreenLocation().TranslatePoint(minX, minY, minZ);
	}
	m_texts.push_back(CADText(minX, minY - 4.0F, minZ, model->GetName(), 2.0F));

	if (addConnections) {
		if (!model->GetControllerName().empty() && model->GetControllerName() != NO_CONTROLLER && model->GetControllerName() != USE_START_CHANNEL) {
			m_texts.push_back(CADText(minX, minY - 6.0F, minZ, "Controller: " + model->GetControllerName(), 1.5F));
			std::string const chain = "Model Chain: " + (model->GetModelChain() == "" ? "Beginning" : model->GetModelChain());
			m_texts.push_back(CADText(minX, minY - 10.0F, minZ, chain, 1.5F));
		} else {
			m_texts.push_back(CADText(minX, minY - 6.0F, minZ, "Start Channel: " + model->ModelStartChannel, 1.5F));
		}

		if (model->GetControllerPort() != 0 && !model->GetControllerProtocol().empty()) {
			std::string portName = (IsPixelProtocol(model->GetControllerProtocol()) ? "Pixel Port: " : "Serial Port: ") + model->GetControllerConnectionPortRangeString();
			if (model->GetSmartRemote() != 0) {
				portName += ":";
				portName += model->GetSmartRemoteLetter();
			}
			m_texts.push_back(CADText(minX, minY - 8.0F, minZ, portName, 1.5F));
		}
	}

	m_color = xlColor(model->GetTagColour()).GetRGB(false);
}

void CADModel::ShiftModel(float x, float y, float z)
{
	for (auto & pt : m_nodes) {
		pt.X += x;
		pt.Y += y;
		pt.Z += z;
	}

	for (auto& ln : m_wires) {
		ln.Loc1.X += x;
		ln.Loc1.Y += y;
		ln.Loc1.Z += z;

		ln.Loc2.X += x;
		ln.Loc2.Y += y;
		ln.Loc2.Z += z;
	}

	for (auto& txt : m_texts) {
		txt.Loc.X += x;
		txt.Loc.Y += y;
		txt.Loc.Z += z;
	}
}

void CADModel::WriteToCAD(CADWriter* writer)
{
	writer->WriteNodeHeader();

	//do something special for first node, diff shape or color?
	if (m_nodes.size() > 0) {
		writer->WriteFirstNode(m_nodes.front(), m_color);
	}

	if (m_nodes.size() > 2) {
		std::for_each(std::begin(m_nodes) + 1, std::end(m_nodes) - 1, [&](CADPoint pt) {
			writer->WriteNode(pt, m_color);
		});
	}

	//do something special for last node, diff shape or color?
	if (m_nodes.size() > 1) {
		writer->WriteLastNode(m_nodes.back(), m_color);
	}

	writer->WriteNodeFooter();

	writer->WriteWireHeader();
	for (auto& ln : m_wires) {
		writer->WriteWire(ln.Loc1, ln.Loc2, m_color);
	}
	writer->WriteWireFooter();

	writer->WriteTextHeader();
	for (auto& txt : m_texts) {
		writer->WriteText(txt.Loc, txt.Text, txt.Size, m_color, txt.Rotation);
	}
	writer->WriteTextFooter();
}