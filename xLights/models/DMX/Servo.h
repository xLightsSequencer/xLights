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

#include "../PWMOutput.h"
#include <map>

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class Servo
{
    public:
        Servo(const std::string& _name, bool _is2d);
        virtual ~Servo();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid, bool pwm);

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        int GetChannel() const { return channel; }
        int GetMinLimit() const { return min_limit; }
        int GetMaxLimit() const { return max_limit; }
        int GetRangeOfMotion() const { return range_of_motion; }
        float GetPivotOffsetX() const { return pivot_offset_x; }
        float GetPivotOffsetY() const { return pivot_offset_y; }
        float GetPivotOffsetZ() const { return pivot_offset_z; }
        bool IsTranslate() const;
        bool IsRotate() const;
        void FillMotionMatrix(float servo_pos, glm::mat4& motion_matrix);
        float GetPosition(int channel_value);
        bool Is16Bit() { return _16bit; }
        Servo* GetLink() { return link; }

        void SetChannel(int chan) { channel = chan; }
        void SetMinLimit(int val) { min_limit = val; }
        void SetMaxLimit(int val) { max_limit = val; }
        void SetRangeOfMotion(float val) { range_of_motion = val; }
        void SetStyle(const std::string& style) { servo_style = style; }
        void Set16Bit(bool value);
        void SetLink(Servo* val) { link = val; }

        void SetControllerMin(int val) { controller_min = val; }
        void SetControllerMax(int val) { controller_max = val; }
        void SetControllerReverse(bool val) { controller_reverse = val; }
        void SetControllerZero(const std::string& val) { controller_zero = val; }
        void SetControllerDataType(const std::string& val) { controller_dataType = val; }

        int GetControllerMin() { return controller_min; }
        int GetControllerMax() { return controller_max; }
        bool GetControllerReverse() { return controller_reverse; }
        const std::string& GetControllerZero() { return controller_zero; }
        const std::string& GetControllerDataType() { return controller_dataType; }

        void SetScaledPivotOffsetX(float val) { pivot_offset_x = val / offset_scale; }
        void SetScaledPivotOffsetY(float val) { pivot_offset_y = val / offset_scale; }
        void SetScaledPivotOffsetZ(float val) { pivot_offset_z = val / offset_scale; }
        float GetScaledPivotOffsetX() { return pivot_offset_x * offset_scale; }
        float GetScaledPivotOffsetY() { return pivot_offset_y * offset_scale; }
        float GetScaledPivotOffsetZ() { return pivot_offset_z * offset_scale; }

        void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const;

    protected:

    private:
        wxString base_name;
        int channel;
        int min_limit;
        int max_limit;
        float range_of_motion;
        float pivot_offset_x;
        float pivot_offset_y;
        float pivot_offset_z;
        int servo_style_val;
        std::string servo_style;
        bool _16bit;
        float offset_scale;
        bool is_2d;
        Servo* link;
        BaseObject* base;
    
        // used for display only in the previews
        int lastValue = 0;
    
        int controller_min = 1000;
        int controller_max = 2000;
        bool controller_reverse = false;
        std::string controller_zero = "Hold";
        std::string controller_dataType = "Scaled";
};

