#ifndef LORCONTROLLERS_H
#define LORCONTROLLERS_H

#include <list>
#include <wx/xml/xml.h>

#include "LorController.h"

class LorControllers
{
    std::list<LorController*> _controllers;
    int _changeCount;
    int _lastSavedChangeCount;

public:
    LorControllers();
    LorControllers(wxXmlNode* node);
    virtual ~LorControllers();

    void Save(wxXmlNode* node);

    std::list<LorController*>* GetControllers() { return &_controllers; }
    void SetDirty() { _changeCount++; }
    bool IsDirty() const;
    int GetTotalChannels() const;
    void ClearDirty();
};

#endif // LORCONTROLLERS_H
