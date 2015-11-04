#ifndef DIMMINGCURVE_H
#define DIMMINGCURVE_H

#include "Color.h"
#include <wx/xml/xml.h>


class DimmingCurve
{
    public:
        DimmingCurve();
        virtual ~DimmingCurve();
    
    
        virtual void apply(xlColor &c) = 0;
    
    
        static DimmingCurve *createFromXML(wxXmlNode *node);
        static DimmingCurve *createBrightnessGamma(int brightness, float gamma);
        static DimmingCurve *createFromFile(const wxString &file);
    
    protected:
    private:
};

#endif // DIMMINGCURVE_H
