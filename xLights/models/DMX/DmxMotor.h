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

        int GetChannel() const { return channel; }
        int GetMinValue() const { return min_value; }
        int GetMaxValue() const { return max_value; }
        float GetRangeOfMotion() const { return range_of_motion; }
        int GetOrientZero() const { return orient_zero; }
        int GetOrientHome() const { return orient_home; }
        float GetSlewLimit() const { return slew_limit; }
        float GetPosition(int channel_value);
        bool Is16Bit() { return _16bit; }

        void SetChannel(int chan, BaseObject* base);
        void SetMinLimit(int val);
        void SetMaxLimit(int val);
        void SetRangeOfMotion(float val);
        void Set16Bit(bool value);

    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        int channel;
        int min_value;
        int max_value;
        float range_of_motion;
        int orient_zero;
        int orient_home;
        float slew_limit;
        bool _16bit;
        BaseObject* base;
};

