#pragma once

#include <list>

#include <wx/xml/xml.h>

#include "LorController.h"

class LorControllers
{
    #pragma region Member Variables
    std::list<LorController*> _controllers;
    bool _dirty = false;
    #pragma endregion

public:

    #pragma region Construtors and Destructors
    LorControllers() { _dirty = true; }
    LorControllers(wxXmlNode* node);
    virtual ~LorControllers() {}
    void Save(wxXmlNode* node);
    #pragma endregion

    #pragma region Getters and Setters
    std::list<LorController*>& GetControllers() { return _controllers; }

    int GetTotalChannels() const;

    bool IsDirty() const;
    void ClearDirty();
    #pragma endregion
};
