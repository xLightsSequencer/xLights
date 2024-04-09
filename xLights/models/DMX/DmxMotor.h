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

#include "DmxMotorBase.h"

#include <wx/string.h>
#include <glm/glm.hpp>
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxMotor : public DmxMotorBase
{
protected:
    DmxMotor() :
        DmxMotorBase()
    {}
    public:
        DmxMotor(wxXmlNode* node, wxString _name);
        virtual ~DmxMotor();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

        std::string GetName() const { return base_name; }
        int GetChannelCoarse() const override { return channel_coarse; }
        int GetChannelFine() const override { return channel_fine; }
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

        int ConvertPostoCmd(float position) override;
        float GetPosition(int channel_value) override;
        bool Is16Bit() { return channel_fine > 0; }

        void SetChannelCoarse(int chan);

protected:

    wxXmlNode* node_xml = nullptr;
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

    BaseObject* base = nullptr;

};

