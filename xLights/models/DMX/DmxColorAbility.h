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
