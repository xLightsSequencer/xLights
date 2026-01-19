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
#include "BoxedScreenLocation.h"

class ModelPreview;
class xlVertexColorAccumulator;

class GridlinesObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    GridlinesObject(const ViewObjectManager &manager);
    virtual ~GridlinesObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

    void SetGridLineSpacing(int val) { line_spacing = val; }
    void SetGridWidth(int val) { width = val; }
    void SetGridHeight(int val) { height = val; }
    void SetGridColor(const std::string& color) { gridColor.SetFromString(color); }
    void SetHasAxis(bool val) { hasAxis = val; }
    void SetPointToFront(bool val) { pointToFront = val; }

    int GetGridLineSpacing() { return line_spacing; }
    int GetGridWidth() { return width; }
    int GetGridHeight() { return height; }
    const std::string GetGridColor() { return std::string(gridColor); }

protected:

private:
    int line_spacing = 50;
    xlColor gridColor {0,128,0};
    int width = 1000;
    int height = 1000;
    bool hasAxis = false;
    bool pointToFront = false;
    glm::vec3 createdRotation;
    
    xlVertexColorAccumulator *grid = nullptr;
};
