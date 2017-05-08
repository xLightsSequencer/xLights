#ifndef COLORMANAGER_H
#define COLORMANAGER_H

#include <map>
#include "Color.h"

class xLightsFrame;
class wxXmlDocument;
class wxXmlNode;

class ColorManager
{
    public:
        ColorManager(xLightsFrame*);
        virtual ~ColorManager();

        const xlColor* GetTimingColor(int colorIndex);
        void SetNewColor(std::string name, xlColor* color);
        xlColor* GetColor(std::string name);

        void Save(wxXmlDocument* doc);
        void Load(wxXmlNode* colors_node);

    protected:

    private:
        wxXmlNode* Save() const;

        std::map<std::string, xlColor*> colors;
        xLightsFrame* xlights;

};

#endif // COLORMANAGER_H
