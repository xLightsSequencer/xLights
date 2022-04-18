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

#include "ViewObject.h"
#include "BoxedScreenLocation.h"

class ModelPreview;
class xlVertexColorAccumulator;

class GridlinesObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    GridlinesObject(wxXmlNode *node, const ViewObjectManager &manager);
    virtual ~GridlinesObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;


protected:

private:
    int line_spacing;
    xlColor gridColor;
    int width;
    int height;
    bool hasAxis;
    
    xlVertexColorAccumulator *grid;
};
