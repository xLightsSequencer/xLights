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

class ViewObject;
class wxXmlNode;
class xLightsFrame;

struct XmlDeserializingObjectFactory {
    ViewObject* Deserialize(wxXmlNode* node, xLightsFrame* xlights, bool importing);

private:
    void CommonDeserializeSteps(ViewObject* model, wxXmlNode* node, xLightsFrame* xlights, bool importing);

    void DeserializeBaseObjectAttributes(ViewObject* model, wxXmlNode* node, xLightsFrame* xlights, bool importing);
    void DeserializeTerrainScreenLocationAttributes(ViewObject* object, wxXmlNode* node);

    ViewObject* DeserializeGridlines(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeImage(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeMesh(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeRuler(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    ViewObject* DeserializeTerrain(wxXmlNode* node, xLightsFrame* xlights, bool importing);
};
