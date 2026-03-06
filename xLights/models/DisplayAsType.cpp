/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DisplayAsType.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include <unordered_map>

std::string DisplayAsTypeToString(DisplayAsType type) {
    switch (type) {
        case DisplayAsType::Arches:           return XmlNodeKeys::ArchesType;
        case DisplayAsType::CandyCanes:       return XmlNodeKeys::CandyCaneType;
        case DisplayAsType::ChannelBlock:     return XmlNodeKeys::ChannelBlockType;
        case DisplayAsType::Circle:           return XmlNodeKeys::CircleType;
        case DisplayAsType::Cube:             return XmlNodeKeys::CubeType;
        case DisplayAsType::Custom:           return XmlNodeKeys::CustomType;
        case DisplayAsType::DmxMovingHead:    return XmlNodeKeys::DmxMovingHeadType;
        case DisplayAsType::DmxMovingHeadAdv: return XmlNodeKeys::DmxMovingHeadAdvType;
        case DisplayAsType::DmxFloodArea:     return XmlNodeKeys::DmxFloodAreaType;
        case DisplayAsType::DmxFloodlight:    return XmlNodeKeys::DmxFloodlightType;
        case DisplayAsType::DmxGeneral:       return XmlNodeKeys::DmxGeneralType;
        case DisplayAsType::DmxServo:         return XmlNodeKeys::DmxServoType;
        case DisplayAsType::DmxServo3d:       return XmlNodeKeys::DmxServo3dType;
        case DisplayAsType::DmxSkull:         return XmlNodeKeys::DmxSkullType;
        case DisplayAsType::Icicles:          return XmlNodeKeys::IciclesType;
        case DisplayAsType::Image:            return XmlNodeKeys::ImageType;
        case DisplayAsType::Matrix:           return XmlNodeKeys::MatrixType;
        case DisplayAsType::ModelGroup:       return XmlNodeKeys::ModelGroupType;
        case DisplayAsType::MultiPoint:       return XmlNodeKeys::MultiPointType;
        case DisplayAsType::SingleLine:       return XmlNodeKeys::SingleLineType;
        case DisplayAsType::PolyLine:         return XmlNodeKeys::PolyLineType;
        case DisplayAsType::Sphere:           return XmlNodeKeys::SphereType;
        case DisplayAsType::Spinner:          return XmlNodeKeys::SpinnerType;
        case DisplayAsType::Star:             return XmlNodeKeys::StarType;
        case DisplayAsType::Tree:             return XmlNodeKeys::TreeType;
        case DisplayAsType::WindowFrame:      return XmlNodeKeys::WindowType;
        case DisplayAsType::Wreath:           return XmlNodeKeys::WreathType;
        case DisplayAsType::Gridlines:        return XmlNodeKeys::GridlinesType;
        case DisplayAsType::Terrain:          return XmlNodeKeys::TerrainType;
        case DisplayAsType::Mesh:             return XmlNodeKeys::MeshType;
        case DisplayAsType::Ruler:            return XmlNodeKeys::RulerType;
        case DisplayAsType::SubModel:         return "SubModel";
        case DisplayAsType::ObjectGroup:      return "ObjectGroup";
        case DisplayAsType::Unknown:          return "";
    }
    return "";
}

DisplayAsType DisplayAsTypeFromString(const std::string& str) {
    // Use a static map for O(1) lookups of known strings
    static const std::map<std::string, DisplayAsType> lookup = {
        // Current model types
        {XmlNodeKeys::ArchesType,           DisplayAsType::Arches},
        {XmlNodeKeys::CandyCaneType,        DisplayAsType::CandyCanes},
        {XmlNodeKeys::ChannelBlockType,     DisplayAsType::ChannelBlock},
        {XmlNodeKeys::CircleType,           DisplayAsType::Circle},
        {XmlNodeKeys::CubeType,             DisplayAsType::Cube},
        {XmlNodeKeys::CustomType,           DisplayAsType::Custom},
        {XmlNodeKeys::DmxMovingHeadType,    DisplayAsType::DmxMovingHead},
        {XmlNodeKeys::DmxMovingHeadAdvType, DisplayAsType::DmxMovingHeadAdv},
        {XmlNodeKeys::DmxFloodAreaType,     DisplayAsType::DmxFloodArea},
        {XmlNodeKeys::DmxFloodlightType,    DisplayAsType::DmxFloodlight},
        {XmlNodeKeys::DmxGeneralType,       DisplayAsType::DmxGeneral},
        {XmlNodeKeys::DmxServoType,         DisplayAsType::DmxServo},
        {XmlNodeKeys::DmxServo3dType,       DisplayAsType::DmxServo3d},
        {XmlNodeKeys::DmxSkullType,         DisplayAsType::DmxSkull},
        {XmlNodeKeys::IciclesType,          DisplayAsType::Icicles},
        {XmlNodeKeys::ImageType,            DisplayAsType::Image},
        {XmlNodeKeys::MatrixType,           DisplayAsType::Matrix},
        {XmlNodeKeys::ModelGroupType,       DisplayAsType::ModelGroup},
        {XmlNodeKeys::MultiPointType,       DisplayAsType::MultiPoint},
        {XmlNodeKeys::SingleLineType,       DisplayAsType::SingleLine},
        {XmlNodeKeys::PolyLineType,         DisplayAsType::PolyLine},
        {XmlNodeKeys::SphereType,           DisplayAsType::Sphere},
        {XmlNodeKeys::SpinnerType,          DisplayAsType::Spinner},
        {XmlNodeKeys::StarType,             DisplayAsType::Star},
        {XmlNodeKeys::TreeType,             DisplayAsType::Tree},
        {XmlNodeKeys::WindowType,           DisplayAsType::WindowFrame},
        {XmlNodeKeys::WreathType,           DisplayAsType::Wreath},
        // View object types
        {XmlNodeKeys::GridlinesType,        DisplayAsType::Gridlines},
        {XmlNodeKeys::TerrainType,          DisplayAsType::Terrain},
        {XmlNodeKeys::MeshType,             DisplayAsType::Mesh},
        {XmlNodeKeys::RulerType,            DisplayAsType::Ruler},
        // Special types
        {"SubModel",                        DisplayAsType::SubModel},
        {"ObjectGroup",                     DisplayAsType::ObjectGroup},
        // Legacy aliases
        {"DmxServo3Axis",                   DisplayAsType::DmxServo3d},
        {"Vert Matrix",                     DisplayAsType::Matrix},
        {"Horiz Matrix",                    DisplayAsType::Matrix},
    };

    auto it = lookup.find(str);
    if (it != lookup.end()) {
        return it->second;
    }

    // Handle legacy compound tree values: "Tree 360", "Tree Flat", "Tree Ribbon", etc.
    if (str.size() > 4 && str.rfind("Tree", 0) == 0 && str[4] == ' ') {
        return DisplayAsType::Tree;
    }

    return DisplayAsType::Unknown;
}

bool IsDmxDisplayType(DisplayAsType type) {
    switch (type) {
        case DisplayAsType::DmxMovingHead:
        case DisplayAsType::DmxMovingHeadAdv:
        case DisplayAsType::DmxFloodArea:
        case DisplayAsType::DmxFloodlight:
        case DisplayAsType::DmxGeneral:
        case DisplayAsType::DmxServo:
        case DisplayAsType::DmxServo3d:
        case DisplayAsType::DmxSkull:
            return true;
        default:
            return false;
    }
}
