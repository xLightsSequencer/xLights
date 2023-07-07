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
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxMotor
{
    public:
        DmxMotor(wxXmlNode* node, wxString _name);
        virtual ~DmxMotor();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

        int GetChannelCoarse() const { return channel_coarse; }
        int GetChannelFine() const { return channel_fine; }
        int GetMinLimit() const { return min_limit; }
        int GetMaxLimit() const { return max_limit; }
        int GetMinValue() const { return 0; }
        int GetMaxValue() const { return 65535; }
        float GetRangeOfMotion() const { return range_of_motion; }
        int GetOrientZero() const { return orient_zero; }
        int GetOrientUp() const { return orient_up; }
        float GetSlewLimit() const { return slew_limit; }
        int ConvertPostoCmd( float position );
        float GetPosition(int channel_value);
        bool Is16Bit() { return channel_fine > 0; }

        void SetChannelCoarse(int chan, BaseObject* base);

    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        int channel_coarse;
        int channel_fine;
        int min_value;
        int max_value;
        int min_limit;
        int max_limit;
        float range_of_motion;
        int orient_zero;
        int orient_up;
        float slew_limit;
        bool reverse;
        int rev;

    BaseObject* base;
};

