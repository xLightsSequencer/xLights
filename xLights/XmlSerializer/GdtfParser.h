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

#include <wx/xml/xml.h>
#include <map>
#include <string>
#include <list>

class Model;
class xLightsFrame;

namespace XmlSerialize {

    // Structure to hold parsed DMX channel value information
    struct DMXChannelValue {
        std::string description;
        int low;
        int high;
        
        DMXChannelValue() : low(0), high(0) {}
    };

    // Structure to hold parsed DMX channel information
    struct DMXChannelInfo {
        std::string attribute;
        int channelStart;
        int channels;
        std::list<DMXChannelValue> values;
        
        DMXChannelInfo() : channelStart(0), channels(0) {}
        
        bool IsMovingHead() const {
            return attribute == "Pan" || attribute == "Tilt";
        }
    };

    // Structure to hold parsed GDTF data
    struct GdtfModelData {
        std::list<DMXChannelInfo> channels;
        int totalChannels;
        bool isMovingHead;
        std::string selectedMode;
        
        GdtfModelData() : totalChannels(0), isMovingHead(false) {}
    };

    // Parse GDTF description.xml and return structured data
    // Returns true if parsing was successful, false if cancelled or failed
    bool ParseGdtfDescriptionXml(
        wxXmlDocument& gdtf_doc,
        xLightsFrame* xlights,
        bool& cancelled,
        GdtfModelData& outData
    );

    // Convert parsed GDTF data to DMX model
    Model* CreateDmxModelFromGdtfData(
        Model* existingModel,
        const GdtfModelData& gdtfData,
        xLightsFrame* xlights
    );

} // namespace XmlSerialize
