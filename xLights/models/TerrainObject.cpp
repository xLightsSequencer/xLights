/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <vector>
#include "TerrainObject.h"
#include "ModelPreview.h"
#include "Model.h"
#include "RulerObject.h"
#include "../ExternalHooks.h"
#include <log4cpp/Category.hh>

TerrainObject::TerrainObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
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

void TerrainObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxImageFileProperty("Image",
        "Image",
        _imageFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
                                        "|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", transparency));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", (int)brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Line Spacing", "TerrianLineSpacing", spacing));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1024);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Terrian Width", "TerrianWidth", width));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Terrian depth", "TerrianDepth", depth));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxColourProperty("Grid Color", "gridColor", gridColor.asWxColor()));

    p = grid->Append(new wxBoolProperty("Hide Grid", "HideGrid", hide_grid));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxBoolProperty("Hide Image", "HideImage", hide_image));
    p->SetAttribute("UseCheckbox", true);

    p = grid->Append(new wxUIntProperty("Brush Size", "TerrianBrushSize", brush_size));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Edit Terrian", "TerrianEdit", editTerrian));

    p->SetAttribute("UseCheckbox", true);

    if (RulerObject::GetRuler() != nullptr) {
        p = grid->Append(new wxStringProperty("Terrian Spacing", "RealSpacing",
            RulerObject::PrescaledMeasureDescription(RulerObject::Measure(spacing))
        ));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

int TerrainObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("Image" == event.GetPropertyName()) {
        for (auto it = _images.begin(); it != _images.end(); ++it) {
            delete it->second;
        }
        _images.clear();
        _imageFile = event.GetValue().GetString();
        ObtainAccessToURL(_imageFile);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Image");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Image");
        return 0;
    } else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("TerrianLineSpacing" == event.GetPropertyName()) {
        spacing = (int)event.GetPropertyValue().GetLong();
        UpdateSize();
        if (grid->GetPropertyByName("RealSpacing") != nullptr && RulerObject::GetRuler() != nullptr) {
            grid->GetPropertyByName("RealSpacing")->SetValueFromString(RulerObject::PrescaledMeasureDescription(RulerObject::Measure(spacing)));
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrianLineSpacing");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrianLineSpacing");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrianLineSpacing");
        return 0;
    } else if ("TerrianWidth" == event.GetPropertyName()) {
        width = (int)event.GetPropertyValue().GetLong();
        UpdateSize();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrianWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrianWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrianWidth");
        return 0;
    } else if ("TerrianDepth" == event.GetPropertyName()) {
        depth = (int)event.GetPropertyValue().GetLong();
        UpdateSize();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrianDepth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrainObject::OnPropertyGridChange::TerrianDepth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::TerrianDepth");
        return 0;
    } else if ("gridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("gridColor");
        wxColour c;
        c << p->GetValue();
        gridColor = c;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::gridColor");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::gridColor");
        return 0;
    } else if ("HideGrid" == event.GetPropertyName()) {
        hide_grid = event.GetValue().GetBool();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::HideGrid");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::HideGrid");
        return 0;
    } else if ("HideImage" == event.GetPropertyName()) {
        hide_image = event.GetValue().GetBool();
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::HideImage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrainObject::OnPropertyGridChange::HideImage");
        return 0;
    } else if ("TerrianBrushSize" == event.GetPropertyName()) {
        brush_size = (int)event.GetPropertyValue().GetLong();
        screenLocation.SetToolSize(brush_size);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrainObject::OnPropertyGridChange::TerrianBrushSize");
        return 0;
    } else if (event.GetPropertyName() == "TerrianEdit") {
        editTerrian = event.GetValue().GetBool();
        if (editTerrian) {
            screenLocation.SetActiveHandle(NO_HANDLE);
            screenLocation.SetEdit(true);
        } else {
            screenLocation.SetActiveHandle(0);
            screenLocation.SetAxisTool(ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE);
            screenLocation.SetEdit(false);
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool TerrainObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool exists = false;

    mtx.lock();
    screenLocation.PrepareToDraw(true, allowSelected);

    int num_points_wide = screenLocation.GetNumPointsWide();
    int num_points_deep = screenLocation.GetNumPointsDeep();
    int num_points = screenLocation.GetNumPoints();

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char *)GetName().c_str(),
                (const char *)_imageFile.c_str(),
                (const char *)preview->GetName().c_str());
            wxImage image(_imageFile);
            if (image.IsOk()) {
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

    if ((Selected || Highlighted) && allowSelected) {
        screenLocation.DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }

    mtx.unlock();
    return true;
}
