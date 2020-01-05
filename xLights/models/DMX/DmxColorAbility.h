#ifndef DMXCOLORABILITY_H
#define DMXCOLORABILITY_H

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxColorAbility
{
    public:
        DmxColorAbility();
        virtual ~DmxColorAbility();

        void AddColorTypeProperties(wxPropertyGridInterface *grid);
        int OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base);

        int GetRedChannel() {return red_channel;}
        int GetGreenChannel() {return green_channel;}
        int GetBlueChannel() {return blue_channel;}
        int GetWhiteChannel() {return white_channel;}

    protected:
        int red_channel;
        int green_channel;
        int blue_channel;
        int white_channel;

    private:
};

#endif // DMXCOLORABILITY_H
