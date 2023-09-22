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
    LorControllers(const LorControllers& from);
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
