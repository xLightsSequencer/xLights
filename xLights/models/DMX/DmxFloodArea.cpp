/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxFloodArea.h"
#include "../../graphics/IModelPreview.h"
#include "../ModelManager.h"
#include "../../graphics/xlGraphicsContext.h"
#include "../../graphics/xlGraphicsAccumulators.h"
#include "UtilFunctions.h"
#include "utils/xLightsVersion.h"
#include "../../XmlSerializer/XmlNodeKeys.h"

DmxFloodArea::DmxFloodArea(const ModelManager &manager)
    : DmxFloodlight(manager)
{
    DisplayAs = DisplayAsType::DmxFloodArea;
}

DmxFloodArea::~DmxFloodArea()
{
}

void DmxFloodArea::InitModel() {
    DmxFloodlight::InitModel();
}
void DmxFloodArea::DrawModel(xlVertexColorAccumulator *vac, xlColor &center, xlColor &edge, float beamratio) {
    float beam_length = beamratio/2.0f;
    vac->AddVertex(0, 0, 0, center);
    vac->AddVertex(-0.5, -0.5, beam_length, edge);
    vac->AddVertex(-0.5, 0.5, beam_length, edge);

    vac->AddVertex(0, 0, 0, center);
    vac->AddVertex(-0.5, 0.5, beam_length, edge);
    vac->AddVertex(0.5, 0.5, beam_length, edge);

    vac->AddVertex(0, 0, 0, center);
    vac->AddVertex(0.5, 0.5, beam_length, edge);
    vac->AddVertex(0.5, -0.5, beam_length, edge);

    vac->AddVertex(0, 0, 0, center);
    vac->AddVertex(0.5, -0.5, beam_length, edge);
    vac->AddVertex(-0.5, -0.5, beam_length, edge);
}
