/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/filedlg.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/propgrid.h>
#include <wx/xml/xml.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat3x3.hpp>

#include "ModelScreenLocation.h"
#include "TwinklyModel.h"
#include "xLightsVersion.h"
#include "../ModelPreview.h"
#include "../OutputModelManager.h"
#include "../UtilFunctions.h"
#include "../xLightsMain.h"

TwinklyModel::TwinklyModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased) :
    ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
    SingleNode = false;
    SingleChannel = false;
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
}

TwinklyModel::~TwinklyModel() = default;

void TwinklyModel::InitModel()
{
    // load from xml
    wxXmlNode* child = ModelXml->GetChildren();
    lights.clear();
    while (child != nullptr) {
        if (child->GetName() == "LightPosition") {
            wxString X = child->GetAttribute("X", "0");
            wxString Y = child->GetAttribute("Y", "0");
            wxString Z = child->GetAttribute("Z", "0");
            LightPosition light;
            X.ToDouble(&light.x);
            Y.ToDouble(&light.y);
            Z.ToDouble(&light.z);
            lights.push_back(light);
        }
        child = child->GetNext();
    }

    parm1 = 1;
    parm2 = lights.size();

    int chanIncr = GetNodeChannelCount(StringType);

    // evaluate bounding box
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::min();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::min();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::min();
    for (auto& light : lights) {
        minX = std::min(minX, light.x);
        maxX = std::max(maxX, light.x);
        minY = std::min(minY, light.y);
        maxY = std::max(maxY, light.y);
        minZ = std::min(minZ, light.z);
        maxZ = std::max(maxZ, light.z);
    }
    double centerX = minX;
    double centerY = minY;
    double centerZ = minZ + (maxZ - minZ) / 2;
    double scale = EXPECTED_HEIGHT / (maxY - minY);

    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
        SetLayerSize(0, lights.size());
        SetNodeCount(1, lights.size(), rgbOrder);
        SetBufferSize(GetLayerSizeCount(), lights.size());
    }

    double renderSizeX = (maxX - minX) * scale;
    double renderSizeY = (maxY - minY) * scale;
    double renderSizeZ = (maxZ - minZ) * scale;
    screenLocation.SetRenderSize(renderSizeX, renderSizeY, renderSizeZ);

    wxASSERT(lights.size() <= Nodes.size());
    int chan = stringStartChan[0];
    for (size_t i = 0; i < lights.size(); i++) {
        Nodes[i]->ActChan = chan;
        Nodes[i]->Coords[0].screenX = (lights[i].x - centerX) * scale;
        Nodes[i]->Coords[0].screenY = (lights[i].y - centerY) * scale * screenLocation.GetHeight();
        Nodes[i]->Coords[0].screenZ = (lights[i].z - centerZ) * scale;
        Nodes[i]->Coords[0].bufX = i;
        chan += chanIncr;
    }
}

void TwinklyModel::AddTypeProperties(wxPropertyGridInterface* grid)
{
}

int TwinklyModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    return Model::OnPropertyGridChange(grid, event);
}

bool TwinklyModel::SetLayout(const wxJSONValue& layout)
{
    const wxJSONInternalMap* map = layout.AsMap();
    if (map == nullptr) {
        return false;
    }
    auto coordinatesIt = map->find("coordinates");
    if (coordinatesIt == map->end()) {
        return false;
    }
    const wxJSONInternalArray* coordinates = coordinatesIt->second.AsArray();
    if (coordinates == nullptr) {
        return false;
    }
    std::vector<LightPosition> lightsTemp;
    for (int i = 0; i < coordinates->Count(); i++) {
        const wxJSONInternalMap* pixelMap = (*coordinates)[i].AsMap();
        if (pixelMap == nullptr) {
            return false;
        }
        LightPosition pos;
        {
            auto iterator = pixelMap->find("x");
            if (iterator != pixelMap->end()) {
                iterator->second.AsDouble(pos.x);
            }
        }
        {
            auto iterator = pixelMap->find("y");
            if (iterator != pixelMap->end()) {
                iterator->second.AsDouble(pos.y);
            }
        }
        {
            auto iterator = pixelMap->find("z");
            if (iterator != pixelMap->end()) {
                iterator->second.AsDouble(pos.z);
            }
        }
        lightsTemp.push_back(pos);
    }
    lights = lightsTemp;

    // export xml
    SaveToXml();
    return true;
}

void TwinklyModel::SaveToXml()
{
    wxXmlNode* child = ModelXml->GetChildren();

    // remove current locations
    while (child != nullptr) {
        if (child->GetName() == "LightPosition") {
            auto next = child->GetNext();
            ModelXml->RemoveChild(child);
            delete child;
            child = next;
            continue;
        }
        child = child->GetNext();
    }

    for (const auto& light : lights) {
        wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "LightPosition");
        node->AddAttribute("X", std::to_string(light.x));
        node->AddAttribute("Y", std::to_string(light.y));
        node->AddAttribute("Z", std::to_string(light.z));
        ModelXml->AddChild(node);
    }
}
