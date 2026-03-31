/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>
#include "TerrainObject.h"
#include "ModelPreview.h"
#include "Model.h"
#include "../ExternalHooks.h"
#include "UtilFunctions.h"
#include "../utils/xlImage.h"
#include <log.h>

TerrainObject::TerrainObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Terrain;
    screenLocation.SetSupportsZScaling(true);
    UpdateSize();
}

TerrainObject::~TerrainObject()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }

    if (texture != nullptr) {
        delete texture;
        texture = nullptr;
    }
    if (grid != nullptr) {
        delete grid;
        grid = nullptr;
    }
}

void TerrainObject::UpdateSize()
{
    // Prevent number of points from changing while drawing the object
    mtx.lock();
    int num_points_wide = width / spacing + 1;
    int num_points_deep = depth / spacing + 1;
    int num_points = num_points_wide * num_points_deep;
    screenLocation.UpdateSize(num_points_wide, num_points_deep, num_points);
    mtx.unlock();
}

void TerrainObject::InitModel() {
    screenLocation.SetToolSize(brush_size);
}

void TerrainObject::SetImageFile(const std::string & imageFile)
{
    ObtainAccessToURL(imageFile);
    _imageFile = FixFile("", imageFile);
}

void TerrainObject::ClearImages() {
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
    _images.clear();
}

bool TerrainObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }

    
    bool exists = false;

    mtx.lock();
    screenLocation.PrepareToDraw(true, allowSelected);

    int num_points_wide = screenLocation.GetNumPointsWide();
    int num_points_deep = screenLocation.GetNumPointsDeep();
    int num_points = screenLocation.GetNumPoints();

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            spdlog::debug("Loading image model {} file {} for preview {}.",
                GetName(),
                _imageFile,
                preview->GetName().ToStdString());
            xlImage image;
            if (image.LoadFromFile(_imageFile) && image.IsOk()) {
                xlTexture *t = ctx->createTexture(image, GetName(), true);
                _images[preview->GetName().ToStdString()] = t;
                img_width = image.GetWidth();
                img_height = image.GetHeight();
                screenLocation.SetRenderSize(width, height, 10.0f);
                exists = true;
            } else {
                exists = false;
            }
        }
    } else {
        exists = true;
    }

    if (uiObjectsInvalid || texture == nullptr) {
        if (texture != nullptr) {
            delete texture;
            texture = nullptr;
        }
        if (grid != nullptr) {
            delete grid;
            grid = nullptr;
        }

        float sx,sy,sz;
        screenLocation.SetRenderSize(width, height, depth);

        std::vector<float>& mPos = *reinterpret_cast<std::vector<float>*>(screenLocation.GetRawData());
        std::vector<glm::vec3> pos;
        pos.resize(num_points);
        float x_offset = (num_points_wide - 1) * spacing / 2;
        float z_offset = (num_points_deep - 1) * spacing / 2;
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                int abs_point = j * num_points_wide + i;
                sx = i * spacing - x_offset;
                sz = j * spacing - z_offset;
                sy = mPos[abs_point];
                screenLocation.TranslatePoint(sx, sy, sz);
                pos[abs_point] = glm::vec3(sx, sy, sz);
            }
        }
        
        if (!hide_grid) {
            grid = ctx->createVertexAccumulator();
            grid->SetName(GetName() + "_GridLines");
            grid->PreAlloc(width / spacing * 12 + depth / spacing * 12);
            // Draw the Gridlines
            for (int j = 0; j < num_points_deep; ++j) {
                for (int i = 0; i < num_points_wide - 1; ++i) {
                    int abs_point = j * num_points_wide + i;
                    grid->AddVertex(pos[abs_point].x, pos[abs_point].y, pos[abs_point].z);
                    grid->AddVertex(pos[abs_point + 1].x, pos[abs_point + 1].y, pos[abs_point + 1].z);
                }
            }

            for (int i = 0; i < num_points_wide; ++i) {
                for (int j = 0; j < num_points_deep - 1; ++j) {
                    int abs_point = j * num_points_wide + i;
                    grid->AddVertex(pos[abs_point].x, pos[abs_point].y, pos[abs_point].z);
                    grid->AddVertex(pos[abs_point + num_points_wide].x, pos[abs_point + num_points_wide].y, pos[abs_point + num_points_wide].z);
                }
            }
            grid->Finalize(false);
        }

        if (exists && !hide_image) {
            // Draw the Texture
            if (transparency == 0) {
                texture = ctx->createVertexTextureAccumulator();
            } else {
                texture = ctx->createVertexTextureAccumulator();
            }
            texture->SetName(GetName() + "_Image");
            texture->PreAlloc(width / spacing * 12 + depth / spacing * 12);

            float x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
            float delta_x = 1.0f / (num_points_wide - 1);
            float delta_y = 1.0f / (num_points_deep - 1);

            float tx1, tx2, ty1, ty2;
            texture->PreAlloc((num_points_deep-1)*(num_points_wide-1)*6);

            for (int j = 0; j < num_points_deep - 1; ++j) {
                for (int i = 0; i < num_points_wide - 1; ++i) {
                    int abs_point = j * num_points_wide + i;
                    x1 = pos[abs_point].x;
                    x2 = pos[abs_point + num_points_wide].x;
                    x3 = pos[abs_point + num_points_wide + 1].x;
                    x4 = pos[abs_point + 1].x;
                    y1 = pos[abs_point].y;
                    y2 = pos[abs_point + num_points_wide].y;
                    y3 = pos[abs_point + num_points_wide + 1].y;
                    y4 = pos[abs_point + 1].y;
                    z1 = pos[abs_point].z;
                    z2 = pos[abs_point + num_points_wide].z;
                    z3 = pos[abs_point + num_points_wide + 1].z;
                    z4 = pos[abs_point + 1].z;
                    tx1 = i * delta_x;
                    tx2 = (i + 1) * delta_x;
                    ty1 = j * delta_y;
                    ty2 = (j + 1) * delta_y;
                    texture->AddVertex(x1, y1, z1, tx1, ty1);
                    texture->AddVertex(x4, y4, z4, tx2, ty1);
                    texture->AddVertex(x2, y2, z2, tx1, ty2);
                    texture->AddVertex(x2, y2, z2, tx1, ty2);
                    texture->AddVertex(x4, y4, z4, tx2, ty1);
                    texture->AddVertex(x3, y3, z3, tx2, ty2);
                }
            }
            texture->Finalize(false, false);
        }
    }
    if (grid) {
        solid->addStep([=, this](xlGraphicsContext *ctx) {
            ctx->drawLines(grid, gridColor);
        });
    }
    if (texture) {
        xlTexture *image = _images[preview->GetName().ToStdString()];
        if (transparency == 0) {
            solid->addStep([=, this](xlGraphicsContext *ctx) {
                ctx->drawTexture(texture, image, brightness, 255, 0, texture->getCount());
            });
        } else {
            transparent->addStep([=, this](xlGraphicsContext *ctx) {
                int alpha = (100.0 - transparency) * 255.0 / 100.0;
                ctx->drawTexture(texture, image, brightness, alpha, 0, texture->getCount());
            });
        }
    }
    screenLocation.UpdateBoundingBox(width, height, depth);

    if ((Selected() || Highlighted()) && allowSelected) {
        screenLocation.DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }

    mtx.unlock();
    return true;
}
