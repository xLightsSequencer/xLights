#ifndef DMXPANTILTABILITY_H
#define DMXPANTILTABILITY_H

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxPanTiltAbility
{
    public:
        DmxPanTiltAbility();
        virtual ~DmxPanTiltAbility();

        void AddPanTiltTypeProperties(wxPropertyGridInterface *grid);
        int OnPanTiltPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base);

        int GetPanChannel() { return pan_channel; }
        int GetTiltChannel() { return tilt_channel; }

    protected:
        int pan_channel;
        int pan_orient;
        int pan_deg_of_rot;
        float pan_slew_limit;
        int tilt_channel;
        int tilt_orient;
        int tilt_deg_of_rot;
        float tilt_slew_limit;

    private:
};

#endif // DMXPANTILTABILITY_H
