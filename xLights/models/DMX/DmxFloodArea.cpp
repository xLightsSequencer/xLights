/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxFloodArea.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodArea::DmxFloodArea(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxFloodlight(node, manager, zeroBased)
{
}

DmxFloodArea::~DmxFloodArea()
{
    //dtor
}

void DmxFloodArea::InitModel() {
    DmxFloodlight::InitModel();
    DisplayAs = "DmxFloodArea";
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
