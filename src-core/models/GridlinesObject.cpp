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

#include <algorithm>

namespace {
// A contrasting (hue rotated 180 degrees) colour that is never black.  Was
// xlColor::ContrastColourNotBlack(), moved here as the only caller when the HSL
// helpers were removed from Color.h; the RGB<->HSL round-trip is inlined.
xlColor contrastColour(const xlColor& in) {
    double r = in.red / 255.0, g = in.green / 255.0, b = in.blue / 255.0;
    double mn = std::min(r, std::min(g, b));
    double mx = std::max(r, std::max(g, b));
    double delta = mx - mn;
    double hue = 0.0, sat = 0.0, light = (mx + mn) / 2.0;
    if (delta != 0.0) {
        sat = (light <= 0.5) ? delta / (mx + mn) : delta / (2.0 - mx - mn);
        double dr = (((mx - r) / 6.0) + (delta / 2.0)) / delta;
        double dg = (((mx - g) / 6.0) + (delta / 2.0)) / delta;
        double db = (((mx - b) / 6.0) + (delta / 2.0)) / delta;
        if (r == mx)      hue = db - dg;
        else if (g == mx) hue = (1.0 / 3.0) + dr - db;
        else              hue = (2.0 / 3.0) + dg - dr;
        if (hue < 0.0) hue += 1.0;
        if (hue > 1.0) hue -= 1.0;
    }
    hue += 0.5;
    if (hue > 1.0) hue -= 1.0;
    if (light > 0.8 || light < 0.2) light = 0.5;

    xlColor c;
    if (sat == 0.0) {
        c.red = c.green = c.blue = (int)(light * 255.0);
    } else {
        double v2 = (light < 0.5) ? light * (1.0 + sat) : (light + sat) - (sat * light);
        double v1 = 2.0 * light - v2;
        auto hue2rgb = [](double v1, double v2, double H) {
            if (H < 0.0) H += 1.0;
            if (H > 1.0) H -= 1.0;
            if ((6.0 * H) < 1.0) return v1 + (v2 - v1) * 6.0 * H;
            if ((2.0 * H) < 1.0) return v2;
            if ((3.0 * H) < 2.0) return v1 + (v2 - v1) * ((2.0 / 3.0) - H) * 6.0;
            return v1;
        };
        c.red   = (int)(255.0 * hue2rgb(v1, v2, hue + (1.0 / 3.0)) + 0.5);
        c.green = (int)(255.0 * hue2rgb(v1, v2, hue) + 0.5);
        c.blue  = (int)(255.0 * hue2rgb(v1, v2, hue - (1.0 / 3.0)) + 0.5);
    }
    return c;
}
}

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

            xlColor arrowColour = contrastColour(gridColor);

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
