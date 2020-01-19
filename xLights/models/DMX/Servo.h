#ifndef SERVO_H
#define SERVO_H

#include <wx/string.h>
#include <glm/glm.hpp>
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class Servo
{
    public:
        Servo(wxXmlNode* node, wxString _name);
        virtual ~Servo();

        void Init(BaseObject* base);

        void AddTypeProperties(wxPropertyGridInterface* grid);

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

        int GetChannel() const { return channel; }
        void SetChannel(int chan, BaseObject* base);
        int GetMinLimit() const { return min_limit; }
        int GetMaxLimit() const { return max_limit; }
        int GetRangeOfMotion() const { return range_of_motion; }
        float GetPivotOffsetX() const { return pivot_offset_x; }
        float GetPivotOffsetY() const { return pivot_offset_y; }
        bool IsTranslate() const;
        bool IsRotate() const;
        void FillMotionMatrix(float& servo_pos, glm::mat4& motion_matrix);
        float GetPosition(int channel_value);
        void Set16Bit(bool value);

    protected:

    private:
        wxXmlNode* node_xml;
        wxString base_name;
        int channel;
        int min_limit;
        int max_limit;
        int range_of_motion;
        float pivot_offset_x;
        float pivot_offset_y;
        int servo_style_val;
        std::string servo_style;
        bool _16bit;
};

#endif // SERVO_H

