/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GridlinesObject.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "Model.h"

GridlinesObject::GridlinesObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), gridColor(xlColor(0,128, 0))
{
    DisplayAs = DisplayAsType::Gridlines;
}

GridlinesObject::~GridlinesObject()
{
}

void GridlinesObject::InitModel() {
}


bool GridlinesObject::Draw(IModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }
    
    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);
    if (uiObjectsInvalid || grid == nullptr || createdRotation != GetObjectScreenLocation().GetRotation()) {
        if (grid) {
            delete grid;
        }
        grid = ctx->createVertexColorAccumulator();
        grid->SetName(GetName() + "_GridLines");

        float sx,sy,sz;

        grid->PreAlloc(width / line_spacing * 12 + height / line_spacing * 12);
        screenLocation.SetRenderSize(width, height, 10.0f);

        xlColor xaxis = xlColor(128,0,0);
        xlColor yaxis = xlColor(0,0,128);

        float half_width = width / 2.0f;
        float half_height = height / 2.0f;
        for (float i = 0; i <= half_height; i += line_spacing) {
            if (hasAxis && i == 0) {
                sx = -half_width; sy = 0; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, xaxis);

                sx = half_width; sy = 0; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, xaxis);
            } else {
                sx = -half_width; sy = i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = half_width; sy = i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -half_width; sy = -i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = half_width; sy = -i; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);
            }
        }
        for (float i = 0; i <= half_width; i += line_spacing) {
            if (hasAxis && i == 0) {
                sx = 0; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, yaxis);

                sx = 0; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, yaxis);
            } else {
                sx = i; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = i; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -i; sy = -half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);

                sx = -i; sy = half_height; sz = 0;
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
                grid->AddVertex(sx, sy, sz, gridColor);
            }
        }

        if (pointToFront) {
            createdRotation = GetObjectScreenLocation().GetRotation();

            GetObjectScreenLocation().SetRotation(glm::vec3(-90.0f, -90.0f, -90.0f));

            // draw an arrow in the middle pointing to the front

            xlColor arrowColour = gridColor.ContrastColourNotBlack();

            sx = 0;
            sy = 0;
            sz = 0;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = 0.5 * line_spacing;
            sy = 0;
            sx = 1.5 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            sz = 0;
            sy = 0;
            sx = 2 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);
            sz = -0.5 * line_spacing;
            sy = 0;
            sx = 1.5 * line_spacing;
            GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
            grid->AddVertex(sx, sy, sz, arrowColour);

            GetObjectScreenLocation().SetRotation(createdRotation);
        }

        grid->Finalize(false, false);
        GetObjectScreenLocation().UpdateBoundingBox(width, height, 5.0f);
        uiObjectsInvalid = false;
    }
    
    solid->addStep([solid, this](xlGraphicsContext *ctx) {
        ctx->drawLines(grid);
    });
    
    if ((Selected() || Highlighted()) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}
