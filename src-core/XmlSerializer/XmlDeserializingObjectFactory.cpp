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
#include "../models/ViewObjectManager.h"

using namespace XmlSerialize;

ViewObject* XmlDeserializingObjectFactory::Deserialize(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    std::string type = node.attribute(XmlNodeKeys::DisplayAsAttribute).as_string("DisplayAs Missing");

    if (type.empty()) {
        throw std::runtime_error("Object has an empty DisplayAs attribute");
    }

    if (type == XmlNodeKeys::GridlinesType) {
        return DeserializeGridlines(node, objects, importing);
    } else if (type == XmlNodeKeys::ImageType) {
        return DeserializeImage(node, objects, importing);
    } else if (type == XmlNodeKeys::MeshType) {
        return DeserializeMesh(node, objects, importing);
    } else if (type == XmlNodeKeys::TerrainType || type == XmlNodeKeys::TerrianType) {
        return DeserializeTerrain(node, objects, importing);
    } else if (type == XmlNodeKeys::RulerType) {
        return DeserializeRuler(node, objects, importing);
    }
    throw std::runtime_error("Unknown object type: " + type);
}

void XmlDeserializingObjectFactory::CommonDeserializeSteps(ViewObject* model, pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    DeserializeBaseObjectAttributes(model, node, objects, importing);
    DeserializeModelScreenLocationAttributes(model, node, importing);
}

void XmlDeserializingObjectFactory::DeserializeBaseObjectAttributes(ViewObject* object, pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    std::string name = node.attribute("name").as_string();
    // Trim whitespace
    auto start = name.find_first_not_of(" \t\r\n");
    if (start != std::string::npos) {
        auto end = name.find_last_not_of(" \t\r\n");
        name = name.substr(start, end - start + 1);
    } else {
        name.clear();
    }
    if (importing) {
        name = objects.GenerateObjectName(name);
    }
    object->SetLayoutGroup("Default", true);
    object->SetName(name);
    object->SetActive(node.attribute(XmlNodeKeys::ActiveAttribute).as_int(1));
    object->SetFromBase(node.attribute(XmlNodeKeys::FromBaseAttribute).as_int(0));
}

void XmlDeserializingObjectFactory::DeserializeTerrainScreenLocationAttributes(ViewObject* object, pugi::xml_node node) {
    int spacing = node.attribute(XmlNodeKeys::TerrainLineAttribute).as_int(50);
    int width = node.attribute(XmlNodeKeys::TerrainWidthAttribute).as_int(1000);
    int depth = node.attribute(XmlNodeKeys::TerrainDepthAttribute).as_int(1000);
    int num_points_wide = width / spacing + 1;
    int num_points_deep = depth / spacing + 1;
    int num_points = num_points_wide * num_points_deep;
    TerrainScreenLocation& screenLoc = dynamic_cast<TerrainScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.UpdateSize(num_points_wide, num_points_deep, num_points);
    screenLoc.SetDataFromString(node.attribute(XmlNodeKeys::PointDataAttribute).as_string());
}

ViewObject* XmlDeserializingObjectFactory::DeserializeGridlines(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    GridlinesObject* object = new GridlinesObject(objects);
    CommonDeserializeSteps(object, node, objects, importing);
    object->SetGridLineSpacing(node.attribute("GridLineSpacing").as_int(50));
    object->SetGridWidth(node.attribute("GridWidth").as_int(1000));
    object->SetGridHeight(node.attribute("GridHeight").as_int(1000));
    object->SetGridColor(node.attribute("GridColor").as_string("#008000"));
    object->SetHasAxis(std::string_view(node.attribute("GridAxis").as_string("0")) == "1");
    object->SetPointToFront(std::string_view(node.attribute("PointToFront").as_string("0")) == "1");
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeImage(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    ImageObject* object = new ImageObject(objects);
    CommonDeserializeSteps(object, node, objects, importing);
    object->SetImageFile(node.attribute(XmlNodeKeys::ImageAttribute).as_string());
    object->SetTransparency(node.attribute(XmlNodeKeys::TransparencyAttribute).as_int(0));
    object->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_int(100));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeMesh(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    MeshObject* object = new MeshObject(objects);
    CommonDeserializeSteps(object, node, objects, importing);
    object->SetObjectFile(node.attribute(XmlNodeKeys::ObjFileAttribute).as_string());
    object->SetMeshOnly(std::string_view(node.attribute(XmlNodeKeys::MeshOnlyAttribute).as_string("0")) == "1");
    object->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_float(100));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeTerrain(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    TerrainObject* object = new TerrainObject(objects);
    CommonDeserializeSteps(object, node, objects, importing);
    DeserializeTerrainScreenLocationAttributes(object, node);
    object->SetImageFile(node.attribute(XmlNodeKeys::ImageAttribute).as_string());
    object->SetTransparency(node.attribute(XmlNodeKeys::TransparencyAttribute).as_int(0));
    object->SetBrightness(node.attribute(XmlNodeKeys::BrightnessAttribute).as_int(100));
    object->SetSpacing(node.attribute(XmlNodeKeys::TerrainLineAttribute).as_int(50));
    object->SetWidth(node.attribute(XmlNodeKeys::TerrainWidthAttribute).as_int(1000));
    object->SetDepth(node.attribute(XmlNodeKeys::TerrainDepthAttribute).as_int(1000));
    object->SetHideGrid(std::string_view(node.attribute(XmlNodeKeys::HideGridAttribute).as_string("0")) == "1");
    object->SetHideImage(std::string_view(node.attribute(XmlNodeKeys::HideImageAttribute).as_string("0")) == "1");
    object->SetGridColor(node.attribute(XmlNodeKeys::GridColorAttribute).as_string("#008000"));
    object->Setup();
    return object;
}

ViewObject* XmlDeserializingObjectFactory::DeserializeRuler(pugi::xml_node node, ViewObjectManager& objects, bool importing) {
    RulerObject* object = new RulerObject(objects);
    CommonDeserializeSteps(object, node, objects, importing);
    DeserializeTwoPointScreenLocationAttributes(object, node);
    object->SetUnits(node.attribute(XmlNodeKeys::UnitsAttribute).as_int(0));
    object->SetLength(node.attribute(XmlNodeKeys::LengthAttribute).as_float(1));
    object->Setup();
    return object;
}
