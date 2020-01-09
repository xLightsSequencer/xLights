#ifndef DMXSHUTTERABILITY_H
#define DMXSHUTTERABILITY_H

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxShutterAbility
{
    public:
        DmxShutterAbility();
        virtual ~DmxShutterAbility();

        void AddShutterTypeProperties(wxPropertyGridInterface *grid);
        int OnShutterPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base);

        int GetShutterChannel() { return shutter_channel; }
        int GetShutterThreshold() { return shutter_threshold; }

    protected:
        int shutter_channel;
        int shutter_threshold;

    private:
};

#endif // DMXSHUTTERABILITY_H
