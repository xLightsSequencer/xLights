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

#include <string>

#include "../utils/LayoutLock.h"

class BaseObject;
class OutputModelManager;
class UICallbacks;

#ifdef GetObject
#undef GetObject  // Windows wingdi.h defines GetObject as GetObjectW
#endif

class ObjectManager
{
public:
    ObjectManager();
    virtual ~ObjectManager();

    std::string GenerateObjectName(const std::string& candidateName) const;

    virtual BaseObject *GetObject(const std::string &name) const = 0;
    virtual UICallbacks* GetUICallbacks() const { return nullptr; }
    virtual OutputModelManager* GetOutputModelManager() const { return nullptr; }

    // While the layout lock is set, the derived managers' Delete and Rename
    // refuse. Adds are not refused here: their callers keep using the object.
    void SetLayoutLock(const LayoutLock* lock) { _layoutLock = lock; }
    bool IsLayoutLocked() const { return _layoutLock != nullptr && _layoutLock->IsLocked(); }

protected:
    const LayoutLock* _layoutLock = nullptr;
};
