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

namespace XmlSerialize {

std::vector<std::vector<std::vector<int>>> ParseCustomModel(const std::string& customModel);
std::vector<std::vector<std::vector<int>>> ParseCompressed(const std::string& compressed);
std::vector<std::vector<std::vector<int>>> ParseCustomModelDataFromXml(const wxXmlNode* node);
void DeserializeFaceInfo(wxXmlNode* f, FaceStateData & faceInfo);
void DeserializeStateInfo(wxXmlNode* f, FaceStateData & stateInfo);

void DeserializeModelScreenLocationAttributes(BaseObject* object, wxXmlNode* node, bool importing);
void DeserializeTwoPointScreenLocationAttributes(BaseObject* object, wxXmlNode* node);
void DeserializeThreePointScreenLocationAttributes(BaseObject* object, wxXmlNode* node);
void DeserializePolyPointScreenLocationAttributes(BaseObject* object, wxXmlNode* node);

}
