#ifndef GRIDLINESOBJECT_H
#define GRIDLINESOBJECT_H

#include "ViewObject.h"

class GridlinesObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        virtual ~GridlinesObject();

    protected:
        GridlinesObject(const ObjectManager &manager);

    private:
};

#endif // GRIDLINESOBJECT_H
