#pragma once

#include <list>
#include <string>

#include <wx/xml/xml.h>

class RGBEffects {
    wxXmlDocument* _rgbEffects = nullptr;

public:
    RGBEffects();
    ~RGBEffects() {
        if (_rgbEffects != nullptr) {
            delete _rgbEffects;
        }
    }
    std::list<std::string> GetModels(const std::string& ofType);
    wxXmlNode* GetModel(const std::string& model);
};
