#ifndef VIEWOBJECT_H
#define VIEWOBJECT_H

#include "BaseObject.h"
#include "ModelScreenLocation.h"
#include "ObjectManager.h"

class ViewObject : public BaseObject
{
    public:
        ViewObject(const ObjectManager &manger);
        virtual ~ViewObject();

        bool GetIs3dOnly() { return only_3d; }

    protected:

    private:
        bool only_3d;
};

template <class ScreenLocation>
class ObjectWithScreenLocation : public ViewObject {
public:
    virtual const ModelScreenLocation &GetObjectScreenLocation() const {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetModelScreenLocation() {
        return screenLocation;
    }
protected:
    ObjectWithScreenLocation(const ObjectManager &manager) : ViewObject(manager) {}
    virtual ~ObjectWithScreenLocation() {}
    ScreenLocation screenLocation;
};

#endif // VIEWOBJECT_H
