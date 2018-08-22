#ifndef GRIDLINESOBJECT_H
#define GRIDLINESOBJECT_H

#include "ViewObject.h"

class GridlinesObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager);
    virtual ~GridlinesObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);

    virtual void Draw(DrawGLUtils::xl3Accumulator &va3, bool allowSelected = false) override;

protected:

private:
    int line_spacing;
    xlColor gridColor;
    int width;
    int height;
    bool hasAxis;
};

#endif // GRIDLINESOBJECT_H
