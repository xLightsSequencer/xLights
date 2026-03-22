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

#include <pugixml.hpp>

// These functions are not used yet...I moved them to this file just to save them for when we handle RGB effects file later
    void SerializeViewsObject(pugi::xml_node node, xLightsFrame* xlights);
    void SerializeColorsObject(pugi::xml_node node, xLightsFrame* xlights);
    void SerializeLayoutGroupsObject(pugi::xml_node node, xLightsFrame* xlights);
    void SerializePerspectivesObject(pugi::xml_node node, xLightsFrame* xlights);
    void SerializeSettingsObject(pugi::xml_node node, xLightsFrame* xlights);
