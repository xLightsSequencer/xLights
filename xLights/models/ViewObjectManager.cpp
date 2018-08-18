#include "ViewObjectManager.h"

ViewObjectManager::ViewObjectManager()
{
    //ctor
}

ViewObjectManager::~ViewObjectManager()
{
    //dtor
}

BaseObject* ViewObjectManager::GetObject(const std::string &name) const
{
    return (BaseObject*)GetViewObject(name);
}

ViewObject* ViewObjectManager::GetViewObject(const std::string &name) const
{
    auto it = view_objects.find(name);
    if (it == view_objects.end()) {
        return nullptr;
    }
    return it->second;
}
