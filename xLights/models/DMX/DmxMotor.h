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
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxMotor
{
protected:
    DmxMotor() {}
    public:
        DmxMotor(const std::string& _name);
        virtual ~DmxMotor();

        void Init();

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        std::string GetName() const { return base_name; }
        int GetChannelCoarse() const { return channel_coarse; }
        int GetChannelFine() const { return channel_fine; }
        int GetMinLimit() const { return min_limit; }
        int GetMaxLimit() const { return max_limit; }
        int GetMinValue() const { return 0; }
        int GetMaxValue() const { return 65535; }
        float GetRangeOfMotion() const { return range_of_motion; }
        int GetOrientZero() const { return orient_zero; }
        int GetOrientHome() const { return orient_home; }
        float GetSlewLimit() const { return slew_limit; }
        bool GetReverse() const { return reverse; }
        bool GetUpsideDown() const { return upside_down; }

        int ConvertPostoCmd(float position);
        float GetPosition(int channel_value);
        bool Is16Bit() { return channel_fine > 0; }

        void SetChannelCoarse(int chan) { channel_coarse = chan; }
        void SetChannelFine(int chan) { channel_fine = chan; }
        void SetMinLimit(int limit) { min_limit = limit; }
        void SetMaxLimit(int limit) { max_limit = limit; }
        void SetRangeOfMOtion(float range) { range_of_motion = range; }
        void SetOrientZero(int val) { orient_zero = val; }
        void SetOrientHome(int val) { orient_home = val; }
        void SetSlewLimit(float limit) { slew_limit = limit; }
        void SetReverse(bool val) { reverse = val; }
        void SetUpsideDown(bool val) { upside_down = val; }

protected:

    wxString base_name;
    int channel_coarse{ 0 };
    int channel_fine{ 0 };
    int min_value{ 0 };
    int max_value{ 65535 };
    int min_limit{ -180 };
    int max_limit{ 180 };
    float range_of_motion{180.0f};
    int orient_zero { 0 };
    int orient_home{ 0 };
    float slew_limit{ 0.0F };
    bool reverse{false};
    bool upside_down{false};
    int rev{ 1 };
};

