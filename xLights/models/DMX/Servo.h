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

class Servo
{
    public:
        Servo(wxXmlNode* node, wxString _name, bool _is2d);
        virtual ~Servo();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

        int GetChannel() const { return channel; }
        int GetMinLimit() const { return min_limit; }
        int GetMaxLimit() const { return max_limit; }
        int GetRangeOfMotion() const { return range_of_motion; }
        float GetPivotOffsetX() const { return pivot_offset_x; }
        float GetPivotOffsetY() const { return pivot_offset_y; }
        float GetPivotOffsetZ() const { return pivot_offset_z; }
        bool IsTranslate() const;
        bool IsRotate() const;
        void FillMotionMatrix(float& servo_pos, glm::mat4& motion_matrix);
        float GetPosition(int channel_value);
        bool Is16Bit() { return _16bit; }
        Servo* GetLink() { return link; }

        void SetChannel(int chan, BaseObject* base);
        void SetMinLimit(int val);
        void SetMaxLimit(int val);
        void SetRangeOfMotion(float val);
        void SetStyle(const std::string& style);
        void Set16Bit(bool value);
        void SetLink(Servo* val) { link = val; }
        void SetPivotOffsetX(float val);
        void SetPivotOffsetY(float val);
        void SetPivotOffsetZ(float val);

    protected:

    private:
        wxXmlNode* node_xml;
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
};

