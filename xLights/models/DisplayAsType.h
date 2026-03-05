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

// Type-safe enum for the DisplayAs field on BaseObject.
// Each value corresponds to a model, view object, or special type.
// The canonical XML string for each value is defined in DisplayAsType.cpp
// and must match the strings in XmlNodeKeys.h for serialization compatibility.
enum class DisplayAsType {
    // Model types
    Arches,
    CandyCanes,
    ChannelBlock,
    Circle,
    Cube,
    Custom,
    DmxMovingHead,
    DmxMovingHeadAdv,
    DmxFloodArea,
    DmxFloodlight,
    DmxGeneral,
    DmxServo,
    DmxServo3d,
    DmxSkull,
    Icicles,
    Image,
    Matrix,
    ModelGroup,
    MultiPoint,
    SingleLine,
    PolyLine,
    Sphere,
    Spinner,
    Star,
    Tree,
    WindowFrame,
    Wreath,

    // View object types
    Gridlines,
    Terrain,     // XML value is "Terrian" (known legacy misspelling, preserved for compat)
    Mesh,
    Ruler,

    // Special types
    SubModel,
    ObjectGroup,

    // Sentinel for unrecognized strings
    Unknown
};

// Returns the canonical XML-compatible string for a DisplayAsType.
std::string DisplayAsTypeToString(DisplayAsType type);

// Parses a string (from XML or legacy data) into a DisplayAsType.
// Handles legacy values:
//   "Vert Matrix", "Horiz Matrix" -> Matrix
//   "Tree 360", "Tree Flat", "Tree Ribbon", etc. -> Tree
//   "DmxServo3Axis" -> DmxServo3d
//   Unknown strings -> Unknown
DisplayAsType DisplayAsTypeFromString(const std::string& str);

// Returns true for all DMX model types.
// Replaces the pattern: DisplayAs.rfind("Dmx", 0) == 0
bool IsDmxDisplayType(DisplayAsType type);
