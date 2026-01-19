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

#include "ViewObject.h"
#include "TerrianScreenLocation.h"

class ModelPreview;
class xlTexture;
class xlVertexAccumulator;
class xlVertexTextureAccumulator;

class TerrianObject : public ObjectWithScreenLocation<TerrianScreenLocation>
{
public:
    TerrianObject(const ViewObjectManager &manager);
    virtual ~TerrianObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

     // Getter methods for private members
    const std::string& getImageFile() const { return _imageFile; }
    int getSpacing() const { return spacing; }
    const xlColor& getGridColor() const { return gridColor; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getDepth() const { return depth; }
    int getNumPointsWide() const { return num_points_wide; }
    int getNumPointsDeep() const { return num_points_deep; }
    int getNumPoints() const { return num_points; }
    bool isEditTerrian() const { return editTerrian; }
    bool isHideImage() const { return hide_image; }
    bool isHideGrid() const { return hide_grid; }
    int getBrushSize() const { return brush_size; }
    int getImgWidth() const { return img_width; }
    int getImgHeight() const { return img_height; }
    int getTransparency() const { return transparency; }
    float getBrightness() const { return brightness; }
    std::string getPointData() const {
        return std::to_string(num_points) + "," + std::to_string(num_points_deep) + ",0.00," + std::to_string(num_points_wide);
    };
    std::string GetAttribute(const std::string);

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
    int img_width;
    int img_height;
    int transparency;
    float brightness;
    
    std::map<std::string, xlTexture*> _images;
    xlVertexAccumulator *grid;
    xlVertexTextureAccumulator *texture;

};
