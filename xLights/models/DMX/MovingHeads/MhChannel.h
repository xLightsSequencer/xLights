#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/string.h>
#include <glm/glm.hpp>

class BaseObject;
class wxXmlNode;

class MhChannel
{
    public:

        class MhRange
        {
        public:
            explicit MhRange(wxString _name);
            virtual ~MhRange() = default;
            
            std::string GetName() { return name; }
        private:
            wxString name;
        };

        MhChannel(wxXmlNode* node, wxString _name);
        virtual ~MhChannel() = default;

        void Init(BaseObject* base);

        std::string GetBaseName() { return base_name; }
        wxXmlNode* GetXmlNode() { return node_xml; }

        std::vector<std::unique_ptr<MhRange>>& GetRanges() { return ranges; }

        int GetChannelCoarse() const { return channel_coarse; }
        int GetChannelFine() const { return channel_fine; }

        bool Is16Bit() { return channel_fine > 0; }

    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        int channel_coarse = 0;
        int channel_fine = 0;
        BaseObject* base;
    
        std::vector<std::unique_ptr<MhChannel::MhRange>> ranges;
};

