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

#include "ImageObject.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "xLightsMain.h"
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

ImageObject::ImageObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _imageFile(""), width(1), height(1), transparency(0), brightness(100.0f)
{
    SetFromXml(node);
}

ImageObject::~ImageObject()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void ImageObject::InitModel() {
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

    screenLocation.SetRenderSize(width, height, 10.0f);
}

void ImageObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
	wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
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
}

int ImageObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
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
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Image");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ImageObject::OnPropertyGridChange::Image");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Image");
        return 0;
    } else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Transparency");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ImageObject::OnPropertyGridChange::Transparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Transparency");
        return 0;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ImageObject::OnPropertyGridChange::Brightness");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ImageObject::OnPropertyGridChange::Brightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ImageObject::OnPropertyGridChange::Brightness");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool ImageObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    if( !IsActive() ) { return true; }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
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

        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTexture(va, image, brightness, alpha, 0, va->getCount());
            delete va;
        });
    } else {
        auto vac = solid->getAccumulator();
        int startVert = vac->getCount();
        
        vac->AddVertex(x1, y1, z1, *wxRED);
        vac->AddVertex(x2, y2, z2, *wxRED);
        vac->AddVertex(x2, y2, z2, *wxRED);
        vac->AddVertex(x3, y3, z3, *wxRED);
        vac->AddVertex(x3, y3, z3, *wxRED);
        vac->AddVertex(x4, y4, z4, *wxRED);
        vac->AddVertex(x4, y4, z4, *wxRED);
        vac->AddVertex(x1, y1, z1, *wxRED);
        vac->AddVertex(x1, y1, z1, *wxRED);
        vac->AddVertex(x3, y3, z3, *wxRED);
        vac->AddVertex(x2, y2, z2, *wxRED);
        vac->AddVertex(x4, y4, z4, *wxRED);
        int end = vac->getCount();
        solid->addStep([solid, vac, startVert, end](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVert, end - startVert);
        });
    }

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}

std::list<std::string> ImageObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !FileExists(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image object '%s' cant find image file '%s'", GetName(), _imageFile).ToStdString());
    } else if (!wxIsReadable(_imageFile) || !wxImage::CanRead(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image object '%s' cant load image file '%s'", GetName(), _imageFile).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile)) {
            res.push_back(wxString::Format("    WARN: Image object '%s' image file '%s' not under show/media/resource directories.", GetName(), _imageFile).ToStdString());
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
            _imageFile = frame->MoveToShowFolder(_imageFile, wxString(wxFileName::GetPathSeparator()) + "Images");
            ModelXml->DeleteAttribute("Image");
            ModelXml->AddAttribute("Image", _imageFile);
            SetFromXml(ModelXml);
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
