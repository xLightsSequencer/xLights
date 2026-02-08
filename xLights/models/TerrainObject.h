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
#include "TerrainScreenLocation.h"
#include <mutex>

class ModelPreview;
class xlTexture;
class xlVertexAccumulator;
class xlVertexTextureAccumulator;

class TerrainObject : public ObjectWithScreenLocation<TerrainScreenLocation>
{
public:
    TerrainObject(const ViewObjectManager &manager);
    virtual ~TerrainObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;


    void SetImageFile(const std::string & imageFile);
    void SetTransparency(int val) { transparency = val; }
    void SetBrightness(float val) { brightness = val; }
    void SetSpacing(int val) { spacing = val; }
    void SetWidth(int val) { width = val; }
    void SetDepth(int val) { depth = val; }
    void SetHideGrid(bool val) { hide_grid = val; }
    void SetHideImage(bool val) { hide_image = val; }
    void SetGridColor(const std::string& color) { gridColor.SetFromString(color); }

    const std::string& GetImageFile() const { return _imageFile; }
    int GetSpacing() const { return spacing; }
    const std::string GetGridColor() const { return std::string(gridColor); }
    int GetWidth() const { return width; }
    int GetDepth() const { return depth; }
    bool IsHideImage() const { return hide_image; }
    bool IsHideGrid() const { return hide_grid; }
    int GetImgWidth() const { return img_width; }
    int GetImgHeight() const { return img_height; }
    int GetTransparency() const { return transparency; }
    int GetBrightness() const { return brightness; }

    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:
    void UpdateSize();

private:
    std::string _imageFile {""};
    int spacing {50};
    xlColor gridColor {0,128,0};
    int width {1000};
    int height {10};
    int depth {1000};
    bool editTerrian {false};
    bool hide_image {false};
    bool hide_grid {false};
    int brush_size {1};
    int img_width {1};
    int img_height {1};
    int transparency {0};
    int brightness {100};
    
    std::map<std::string, xlTexture*> _images;
    xlVertexAccumulator *grid {nullptr};
    xlVertexTextureAccumulator *texture {nullptr};
    std::mutex mtx;
};
