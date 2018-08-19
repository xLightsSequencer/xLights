#ifndef GRIDLINESOBJECT_H
#define GRIDLINESOBJECT_H

#include "ViewObject.h"

class GridlinesObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager);
    virtual ~GridlinesObject();

    virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;

    virtual void Draw(DrawGLUtils::xl3Accumulator &va3, bool allowSelected = false) override;

protected:

private:
    int line_spacing;
    xlColor color;
    float width;
    float height;
};

#endif // GRIDLINESOBJECT_H
