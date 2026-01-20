#pragma once
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

#include "XmlDeserializingObjectFactory.h"
#include "XmlSerializeFunctions.h"
#include "XmlNodeKeys.h"
#include "../models/GridlinesObject.h"
#include "../models/ImageObject.h"
#include "../models/MeshObject.h"
#include "../models/RulerObject.h"
#include "../models/TerrainObject.h"
#include "../models/TerrainScreenLocation.h"
#include "../models/ViewObject.h"
#include "../xLightsMain.h"

#include <wx/xml/xml.h>

using namespace XmlSerialize;

ViewObject* XmlDeserializingObjectFactory::Deserialize(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    auto type = node->GetAttribute(XmlNodeKeys::DisplayAsAttribute);
    
    if (type == XmlNodeKeys::GridlinesType) {
        return DeserializeGridlines(node, xlights, importing);
    } else if (type == XmlNodeKeys::ImageType) {
        return DeserializeImage(node, xlights, importing);
    } else if (type == XmlNodeKeys::MeshType) {
        return DeserializeMesh(node, xlights, importing);
    } else if (type == XmlNodeKeys::TerrainType) {
        return DeserializeTerrain(node, xlights, importing);
    } else if (type == XmlNodeKeys::RulerType) {
        return DeserializeRuler(node, xlights, importing);
    }
    throw std::runtime_error("Unknown object type: " + type);
}

void XmlDeserializingObjectFactory::CommonDeserializeSteps(ViewObject* model, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    DeserializeBaseObjectAttributes(model, node, xlights, importing);
    DeserializeModelScreenLocationAttributes(model, node, importing);
}

void XmlDeserializingObjectFactory::DeserializeBaseObjectAttributes(ViewObject* object, wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    std::string name = node->GetAttribute("name").Trim(true).Trim(false).ToStdString();
    if (importing)
    {
        name = xlights->AllObjects.GenerateObjectName(name);
    }
    object->SetLayoutGroup("Default");
    object->SetName(name);
    object->SetDisplayAs(node->GetAttribute(XmlNodeKeys::DisplayAsAttribute).ToStdString());
    object->SetActive(std::stoi(node->GetAttribute(XmlNodeKeys::ActiveAttribute, "1").ToStdString()));
}

void XmlDeserializingObjectFactory::DeserializeTerrainScreenLocationAttributes(ViewObject* object, wxXmlNode* node) {
    int spacing = std::stoi(node->GetAttribute(XmlNodeKeys::TerrainLineAttribute,"50").ToStdString());
    int width = std::stoi(node->GetAttribute(XmlNodeKeys::TerrainWidthAttribute,"1000").ToStdString());
    int depth = std::stoi(node->GetAttribute(XmlNodeKeys::TerrainDepthAttribute,"1000").ToStdString());
    int num_points_wide = width / spacing + 1;;
    int num_points_deep = depth / spacing + 1;
    int num_points = num_points_wide * num_points_deep;
    TerrainScreenLocation& screenLoc = dynamic_cast<TerrainScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetNumPointsWide(num_points_wide);
    screenLoc.SetNumPointsDeep(num_points_deep);
    screenLoc.SetNumPoints(num_points);
    screenLoc.SetDataFromString(node->GetAttribute(XmlNodeKeys::PointDataAttribute, "").ToStdString());
}

ViewObject* XmlDeserializingObjectFactory::DeserializeGridlines(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    GridlinesObject* object = new GridlinesObject(xlights->AllObjects);
    CommonDeserializeSteps(object, node, xlights, importing);
    object->SetGridLineSpacing(std::stoi(node->GetAttribute("GridLineSpacing","50").ToStdString()));
    object->SetGridWidth(std::stoi(node->GetAttribute("GridWidth","1000").ToStdString()));
    object->SetGridHeight(std::stoi(node->GetAttribute("GridHeight","1000").ToStdString()));
    object->SetGridColor(node->GetAttribute("GridColor","#008000"));
    object->SetHasAxis(node->GetAttribute("GridAxis","0") == "1");
    object->SetPointToFront(node->GetAttribute("PointToFront","0") == "1");
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeImage(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    ImageObject* object = new ImageObject(xlights->AllObjects);
    CommonDeserializeSteps(object, node, xlights, importing);
    object->SetImageFile(node->GetAttribute(XmlNodeKeys::ImageAttribute, xlEMPTY_STRING));
    object->SetTransparency(std::stoi(node->GetAttribute(XmlNodeKeys::TransparencyAttribute,"0").ToStdString()));
    object->SetBrightness(std::stoi(node->GetAttribute(XmlNodeKeys::BrightnessAttribute,"100").ToStdString()));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeMesh(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    MeshObject* object = new MeshObject(xlights->AllObjects);
    CommonDeserializeSteps(object, node, xlights, importing);
    object->SetObjectFile(node->GetAttribute(XmlNodeKeys::ObjFileAttribute, xlEMPTY_STRING));
    object->SetMeshOnly(node->GetAttribute(XmlNodeKeys::MeshOnlyAttribute, "0") == "1");
    object->SetBrightness(std::stof(node->GetAttribute(XmlNodeKeys::BrightnessAttribute,"100").ToStdString()));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeTerrain(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    TerrainObject* object = new TerrainObject(xlights->AllObjects);
    CommonDeserializeSteps(object, node, xlights, importing);
    DeserializeTerrainScreenLocationAttributes(object, node);
    object->SetImageFile(node->GetAttribute(XmlNodeKeys::ImageAttribute, xlEMPTY_STRING));
    object->SetTransparency(std::stoi(node->GetAttribute(XmlNodeKeys::TransparencyAttribute,"0").ToStdString()));
    object->SetBrightness(std::stoi(node->GetAttribute(XmlNodeKeys::BrightnessAttribute,"100").ToStdString()));
    object->SetSpacing(std::stoi(node->GetAttribute(XmlNodeKeys::TerrainLineAttribute,"50").ToStdString()));
    object->SetWidth(std::stoi(node->GetAttribute(XmlNodeKeys::TerrainWidthAttribute,"1000").ToStdString()));
    object->SetDepth(std::stoi(node->GetAttribute(XmlNodeKeys::TerrainDepthAttribute,"1000").ToStdString()));
    object->SetHideGrid(node->GetAttribute(XmlNodeKeys::HideGridAttribute,"0") == "1");
    object->SetHideImage(node->GetAttribute(XmlNodeKeys::HideImageAttribute,"0") == "1");
    object->SetGridColor(node->GetAttribute("GridColor","#008000"));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeRuler(wxXmlNode* node, xLightsFrame* xlights, bool importing) {
    RulerObject* object = new RulerObject(xlights->AllObjects);
    CommonDeserializeSteps(object, node, xlights, importing);
    DeserializeTwoPointScreenLocationAttributes(object, node);
    object->SetUnits(std::stoi(node->GetAttribute(XmlNodeKeys::UnitsAttribute, "0").ToStdString()));
    object->SetLength(std::stof(node->GetAttribute(XmlNodeKeys::LengthAttribute, "1").ToStdString()));
    object->Setup();
    return object;
}
