/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <filesystem>
#include <format>
#include "ImageObject.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "xLightsMain.h"
#include "../ExternalHooks.h"

#include <log.h>

ImageObject::ImageObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Image;
}

ImageObject::~ImageObject()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void ImageObject::SetImageFile(const std::string & imageFile)
{
    ObtainAccessToURL(imageFile);
    _imageFile = FixFile("", imageFile);
}

void ImageObject::InitModel()
{
    screenLocation.SetRenderSize(width, height, 10.0f);
}

void ImageObject::ClearImages() {
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
    _images.clear();
}

bool ImageObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if( !IsActive() ) { return true; }

    
    bool exists = false;

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            spdlog::debug("Loading image model {} file {} for preview {}.",
                (const char *)GetName().c_str(),
                (const char *)_imageFile.c_str(),
                (const char *)preview->GetName().c_str());
            wxImage image(_imageFile);
            if (image.IsOk()) {
                xlTexture *t = ctx->createTexture(image, GetName(), true);
                _images[preview->GetName().ToStdString()] = t;
                width = image.GetWidth();
                height = image.GetHeight();
                screenLocation.SetRenderSize(width, height, 10.0f);
                exists = true;
            } else {
                exists = false;
            }
        }
    } else {
        exists = true;
    }

    float x1 = -0.5f * width;
    float x2 = -0.5f * width;
    float x3 = 0.5f * width;
    float x4 = 0.5f * width;
    float y1 = -0.5f * height;
    float y2 = 0.5f * height;
    float y3 = 0.5f * height;
    float y4 = -0.5f * height;
    float z1 = 0.0f;
    float z2 = 0.0f;
    float z3 = 0.0f;
    float z4 = 0.0f;

    GetObjectScreenLocation().TranslatePoint(x1, y1, z1);
    GetObjectScreenLocation().TranslatePoint(x2, y2, z2);
    GetObjectScreenLocation().TranslatePoint(x3, y3, z3);
    GetObjectScreenLocation().TranslatePoint(x4, y4, z4);

    GetObjectScreenLocation().UpdateBoundingBox(width, height, 5.0f);

    if (exists) {
        xlTexture* image = _images[preview->GetName().ToStdString()];
        xlGraphicsProgram *program = transparency == 0 ? solid : transparent;
        xlVertexTextureAccumulator *va = ctx->createVertexTextureAccumulator();
        
        va->PreAlloc(6);
        va->AddVertex(x1, y1, z1, 0.0, 1.0);
        va->AddVertex(x4, y4, z4, 1.0, 1.0);
        va->AddVertex(x2, y2, z2, 0.0, 0.0);
        va->AddVertex(x2, y2, z2, 0.0, 0.0);
        va->AddVertex(x4, y4, z4, 1.0, 1.0);
        va->AddVertex(x3, y3, z3, 1.0, 0.0);
        float a = (100.0 - transparency) * 255.0 / 100.0;
        uint8_t alpha = a;

        program->addStep([=, this](xlGraphicsContext *ctx) {
            ctx->drawTexture(va, image, brightness, alpha, 0, va->getCount());
            delete va;
        });
    } else {
        auto vac = solid->getAccumulator();
        int startVert = vac->getCount();
        
        vac->AddVertex(x1, y1, z1, xlRED);
        vac->AddVertex(x2, y2, z2, xlRED);
        vac->AddVertex(x2, y2, z2, xlRED);
        vac->AddVertex(x3, y3, z3, xlRED);
        vac->AddVertex(x3, y3, z3, xlRED);
        vac->AddVertex(x4, y4, z4, xlRED);
        vac->AddVertex(x4, y4, z4, xlRED);
        vac->AddVertex(x1, y1, z1, xlRED);
        vac->AddVertex(x1, y1, z1, xlRED);
        vac->AddVertex(x3, y3, z3, xlRED);
        vac->AddVertex(x2, y2, z2, xlRED);
        vac->AddVertex(x4, y4, z4, xlRED);
        int end = vac->getCount();
        solid->addStep([solid, vac, startVert, end](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVert, end - startVert);
        });
    }

    if ((Selected() || Highlighted()) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}

std::list<std::string> ImageObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !FileExists(_imageFile)) {
        res.push_back(std::format("    ERR: Image object '{}' cant find image file '{}'", GetName(), _imageFile));
    } else if (!wxIsReadable(_imageFile) || !wxImage::CanRead(_imageFile)) {
        res.push_back(std::format("    ERR: Image object '{}' cant load image file '{}'", GetName(), _imageFile));
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile)) {
            res.push_back(std::format("    WARN: Image object '{}' image file '{}' not under show/media/resource directories.", GetName(), _imageFile));
        }
    }
    res.splice(res.end(), BaseObject::CheckModelSettings());
    return res;
}

bool ImageObject::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (FileExists(_imageFile)) {
        if (!frame->IsInShowFolder(_imageFile)) {
            _imageFile = frame->MoveToShowFolder(_imageFile, std::string(1, std::filesystem::path::preferred_separator) + "Images");
            rc = true;
        }
    }
    return BaseObject::CleanupFileLocations(frame) || rc;
}

std::list<std::string> ImageObject::GetFileReferences()
{
    std::list<std::string> res;
    if (FileExists(_imageFile)) {
        res.push_back(_imageFile);
    }
    return res;
}
