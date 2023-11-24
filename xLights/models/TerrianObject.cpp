/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <vector>
#include "TerrianObject.h"
#include "ModelPreview.h"
#include "Model.h"
#include "RulerObject.h"
#include "../ExternalHooks.h"
#include <log4cpp/Category.hh>

TerrianObject::TerrianObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _imageFile(""), spacing(50), gridColor(xlColor(0,128, 0)),
   width(1000.0f), height(10.0f), depth(1000.0f), editTerrian(false), hide_image(false),
   hide_grid(false), brush_size(1), img_width(1), img_height(1), 
   transparency(0), brightness(100.0f), grid(nullptr), texture(nullptr)
{
    screenLocation.SetSupportsZScaling(true);
    SetFromXml(node);
}

TerrianObject::~TerrianObject()
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

void TerrianObject::InitModel() {
    _imageFile = FixFile("", ModelXml->GetAttribute("Image", ""));
    if (_imageFile != ModelXml->GetAttribute("Image", "")) {
        ModelXml->DeleteAttribute("Image");
        ModelXml->AddAttribute("Image", _imageFile);
    }

    ObtainAccessToURL(_imageFile);

    if (ModelXml->HasAttribute("Transparency")) {
        transparency = wxAtoi(ModelXml->GetAttribute("Transparency"));
    }
    if (ModelXml->HasAttribute("Brightness")) {
        brightness = wxAtoi(ModelXml->GetAttribute("Brightness"));
    }
    if (ModelXml->HasAttribute("TerrianLineSpacing")) {
        spacing = wxAtoi(ModelXml->GetAttribute("TerrianLineSpacing"));
    }
    if (ModelXml->HasAttribute("TerrianWidth")) {
        width = wxAtoi(ModelXml->GetAttribute("TerrianWidth"));
    }
    if (ModelXml->HasAttribute("TerrianDepth")) {
        depth = wxAtoi(ModelXml->GetAttribute("TerrianDepth"));
    }
    hide_grid = ModelXml->GetAttribute("HideGrid", "0") == "1";
    hide_image = ModelXml->GetAttribute("HideImage", "0") == "1";
    if (ModelXml->HasAttribute("gridColor")) {
        gridColor = xlColor(ModelXml->GetAttribute("gridColor", "#000000").ToStdString());
    }
    if (ModelXml->HasAttribute("TerrianBrushSize")) {
        brush_size = wxAtoi(ModelXml->GetAttribute("TerrianBrushSize"));
        GetObjectScreenLocation().SetToolSize(brush_size);
    }
    num_points_wide = width / spacing + 1;
    num_points_deep = depth / spacing + 1;
    num_points = num_points_wide * num_points_deep;
}

void TerrianObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
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
        p->ChangeFlag(wxPG_PROP_READONLY, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

int TerrianObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("Image" == event.GetPropertyName()) {
        for (auto it = _images.begin(); it != _images.end(); ++it) {
            delete it->second;
        }
        _images.clear();
        _imageFile = event.GetValue().GetString();
        ObtainAccessToURL(_imageFile);
        ModelXml->DeleteAttribute("Image");
        ModelXml->AddAttribute("Image", _imageFile);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::Image");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrianObject::OnPropertyGridChange::Image");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::Image");
        return 0;
    } else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("TerrianLineSpacing" == event.GetPropertyName()) {
        spacing = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("TerrianLineSpacing");
        ModelXml->AddAttribute("TerrianLineSpacing", wxString::Format("%d", spacing));
        if (grid->GetPropertyByName("RealSpacing") != nullptr && RulerObject::GetRuler() != nullptr) {
            grid->GetPropertyByName("RealSpacing")->SetValueFromString(RulerObject::PrescaledMeasureDescription(RulerObject::Measure(spacing)));
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::TerrianLineSpacing");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrianObject::OnPropertyGridChange::TerrianLineSpacing");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::TerrianLineSpacing");
        return 0;
    } else if ("TerrianWidth" == event.GetPropertyName()) {
        width = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("TerrianWidth");
        ModelXml->AddAttribute("TerrianWidth", wxString::Format("%d", width));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::TerrianWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrianObject::OnPropertyGridChange::TerrianWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::TerrianWidth");
        return 0;
    } else if ("TerrianDepth" == event.GetPropertyName()) {
        depth = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("TerrianDepth");
        ModelXml->AddAttribute("TerrianDepth", wxString::Format("%d", depth));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::TerrianDepth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TerrianObject::OnPropertyGridChange::TerrianDepth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::TerrianDepth");
        return 0;
    } else if ("gridColor" == event.GetPropertyName()) {
        wxPGProperty *p = grid->GetPropertyByName("gridColor");
        wxColour c;
        c << p->GetValue();
        gridColor = c;
        ModelXml->DeleteAttribute("gridColor");
        ModelXml->AddAttribute("gridColor", gridColor);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::gridColor");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::gridColor");
        return 0;
    } else if ("HideGrid" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("HideGrid");
        hide_grid = event.GetValue().GetBool();
        if (hide_grid) {
            ModelXml->AddAttribute("HideGrid", "1");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::HideGrid");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::HideGrid");
        return 0;
    } else if ("HideImage" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("HideImage");
        hide_image = event.GetValue().GetBool();
        if (hide_image) {
            ModelXml->AddAttribute("HideImage", "1");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::HideImage");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TerrianObject::OnPropertyGridChange::HideImage");
        return 0;
    } else if ("TerrianBrushSize" == event.GetPropertyName()) {
        brush_size = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("TerrianBrushSize");
        ModelXml->AddAttribute("TerrianBrushSize", wxString::Format("%d", brush_size));
        GetObjectScreenLocation().SetToolSize(brush_size);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TerrianObject::OnPropertyGridChange::TerrianBrushSize");
        return 0;
    } else if (event.GetPropertyName() == "TerrianEdit") {
        editTerrian = event.GetValue().GetBool();
        if (editTerrian) {
            GetObjectScreenLocation().SetActiveHandle(NO_HANDLE);
            GetObjectScreenLocation().SetEdit(true);
        } else {
            GetObjectScreenLocation().SetActiveHandle(0);
            GetObjectScreenLocation().SetAxisTool(ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE);
            GetObjectScreenLocation().SetEdit(false);
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "GridlinesObject::OnPropertyGridChange::TerrianEdit");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool TerrianObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if (!IsActive()) { return true; }

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool exists = false;

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char *)GetName().c_str(),
                (const char *)_imageFile.c_str(),
                (const char *)preview->GetName().c_str());
            wxImage image(_imageFile);
            if (image.IsOk()) {
                xlTexture *t = ctx->createTexture(image);
                t->SetName(GetName());
                t->Finalize();
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

        std::vector<float>& mPos = *reinterpret_cast<std::vector<float>*>(GetObjectScreenLocation().GetRawData());
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
                GetObjectScreenLocation().TranslatePoint(sx, sy, sz);
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
        solid->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(grid, gridColor);
        });
    }
    if (texture) {
        xlTexture *image = _images[preview->GetName().ToStdString()];
        if (transparency == 0) {
            solid->addStep([=](xlGraphicsContext *ctx) {
                ctx->drawTexture(texture, image, brightness, 255, 0, texture->getCount());
            });
        } else {
            transparent->addStep([=](xlGraphicsContext *ctx) {
                int alpha = (100.0 - transparency) * 255.0 / 100.0;
                ctx->drawTexture(texture, image, brightness, alpha, 0, texture->getCount());
            });
        }
    }
    GetObjectScreenLocation().UpdateBoundingBox(width, height, depth);

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}
