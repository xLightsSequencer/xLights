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
#include "TerrianScreenLocation.h"
#include "Image.h"

class ModelPreview;

class TerrianObject : public ObjectWithScreenLocation<TerrianScreenLocation>
{
public:
    TerrianObject(wxXmlNode *node, const ViewObjectManager &manager);
    virtual ~TerrianObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual void Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, DrawGLUtils::xl3Accumulator &tva3, bool allowSelected = false) override;

protected:

private:
    std::string _imageFile;
    int spacing;
    xlColor gridColor;
    int width;
    int height;
    int depth;
    int num_points_wide;
    int num_points_deep;
    int num_points;
    bool editTerrian;
    bool hide_image;
    bool hide_grid;
    int brush_size;
    std::map<std::string, Image*> _images;
    int img_width;
    int img_height;
    int transparency;
    float brightness;
};
