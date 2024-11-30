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

#include <wx/string.h>
#include <glm/glm.hpp>

class wxXmlNode;

class MhChannel
{
    public:

        class MhRange
        {
        public:
            MhRange(wxXmlNode* node, wxString pretty_name);
            virtual ~MhRange() = default;

            void Init();
            void SetRangeMin(std::string& val);
            void SetRangeMax(std::string& val);

            std::string GetName() { return name; }
            int GetMin() { return min; }
            int GetMax() { return max; }
            std::string GetMinStr() { return wxString::Format(wxT("%i"), min ); }
            std::string GetMaxStr() { return wxString::Format(wxT("%i"), max ); }

            void SetName(std::string& val) { name = val; }

        private:
            wxXmlNode* range_node;
            wxString name;
            unsigned int min = 0;
            unsigned int max = 255;
        };

        MhChannel(wxXmlNode* node, wxString pretty_name);
        virtual ~MhChannel() = default;

        void Init();

        std::string GetName() { return name; }
        wxXmlNode* GetXmlNode() { return node_xml; }

        std::vector<std::unique_ptr<MhRange>>& GetRanges() { return ranges; }

        int GetChannelCoarse() const { return channel_coarse; }
        int GetChannelFine() const { return channel_fine; }

        std::string GetChanCoarseStr() { return channel_coarse > 0 ? wxString::Format(wxT("%i"), channel_coarse ) : ""; }
        std::string GetChanFineStr() { return channel_fine > 0 ? wxString::Format(wxT("%i"), channel_fine ) : ""; }

        void SetChannelCoarse(std::string& val);
        void SetChannelFine(std::string& val);

        void AddRange(std::string& name);
    
        bool Is16Bit() { return channel_fine > 0; }

    protected:

    private:
        wxXmlNode* node_xml;
        wxString name;
        int channel_coarse = 0;
        int channel_fine = 0;
    
        std::vector<std::unique_ptr<MhChannel::MhRange>> ranges;
};

