#ifndef VIEWOBJECTMANAGER_H
#define VIEWOBJECTMANAGER_H

#include <map>

#include "ObjectManager.h"

class ViewObject;

class ViewObjectManager : public ObjectManager
{
    public:
        ViewObjectManager();
        virtual ~ViewObjectManager();

        virtual BaseObject *GetObject(const std::string &name) const;
        ViewObject *GetViewObject(const std::string &name) const;

    protected:

    private:
        std::map<std::string, ViewObject *> view_objects;
};

#endif // VIEWOBJECTMANAGER_H
