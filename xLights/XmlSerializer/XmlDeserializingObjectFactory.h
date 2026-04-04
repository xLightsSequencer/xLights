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

class ViewObject;
class xLightsFrame;

struct XmlDeserializingObjectFactory {
    ViewObject* Deserialize(pugi::xml_node node, xLightsFrame* xlights, bool importing);

private:
    void CommonDeserializeSteps(ViewObject* model, pugi::xml_node node, xLightsFrame* xlights, bool importing);

    void DeserializeBaseObjectAttributes(ViewObject* model, pugi::xml_node node, xLightsFrame* xlights, bool importing);
    void DeserializeTerrainScreenLocationAttributes(ViewObject* object, pugi::xml_node node);

    ViewObject* DeserializeGridlines(pugi::xml_node node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeImage(pugi::xml_node node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeMesh(pugi::xml_node node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeRuler(pugi::xml_node node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeTerrain(pugi::xml_node node, xLightsFrame* xlights, bool importing);
};
