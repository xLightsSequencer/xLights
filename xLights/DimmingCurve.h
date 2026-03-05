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

#include "Color.h"

#include <map>
#include <string>


class wxXmlNode;

class DimmingCurve
{
    public:
        DimmingCurve();
        virtual ~DimmingCurve();
    
    
        virtual void apply(xlColor &c) const = 0;
        virtual void reverse(xlColor &c) const = 0;
    
        static DimmingCurve *createFromInfo(const std::map<std::string, std::map<std::string, std::string>> &info);
        
        static DimmingCurve *createBrightnessGamma(int brightness, float gamma);
        static DimmingCurve *createFromFile(const wxString &file);
    protected:
    private:
};
