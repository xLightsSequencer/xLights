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

#include "Color.h"

class wxXmlNode;

class DimmingCurve
{
    public:
        DimmingCurve();
        virtual ~DimmingCurve();
    
    
        virtual void apply(xlColor &c) = 0;
        virtual void reverse(xlColor &c) = 0;
    
    
        static DimmingCurve *createFromXML(wxXmlNode *node);
        static DimmingCurve *createBrightnessGamma(int brightness, float gamma);
        static DimmingCurve *createFromFile(const wxString &file);
    
    protected:
    private:
};
