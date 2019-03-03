#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ImageObject.h"
#include "DrawGLUtils.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "../xLightsMain.h"

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

    if (ModelXml->HasAttribute("Transparency")) {
        transparency = wxAtoi(ModelXml->GetAttribute("Transparency"));
    }
    if (ModelXml->HasAttribute("Brightness")) {
        brightness = wxAtoi(ModelXml->GetAttribute("Brightness"));
    }

    screenLocation.SetRenderSize(width, height, 10.0f);
}

void ImageObject::AddTypeProperties(wxPropertyGridInterface *grid) {
	wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _imageFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*");

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
        ModelXml->DeleteAttribute("Image");
        ModelXml->AddAttribute("Image", _imageFile);
        SetFromXml(ModelXml);
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH;
    } else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        return GRIDCHANGE_MARK_DIRTY_AND_REFRESH | GRIDCHANGE_REBUILD_MODEL_LIST;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

void ImageObject::Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, DrawGLUtils::xl3Accumulator &tva3, bool allowSelected)
{
    if( !active ) { return; }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool exists = false;

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (wxFileExists(_imageFile)) {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char *)GetName().c_str(),
                (const char *)_imageFile.c_str(),
                (const char *)preview->GetName().c_str());
            _images[preview->GetName().ToStdString()] = new Image(_imageFile);

            width = (_images[preview->GetName().ToStdString()])->width;
            height = (_images[preview->GetName().ToStdString()])->height;
            screenLocation.SetRenderSize(width, height, 10.0f);
            exists = true;
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

    GetObjectScreenLocation().UpdateBoundingBox(width, height);  // FIXME: Modify to only call this when position changes

    if (exists) {
        Image* image = _images[preview->GetName().ToStdString()];

        float tx1 = 0;
        float tx2 = image->tex_coord_x;

        DrawGLUtils::xl3Accumulator &va = transparency == 0 ? va3 : tva3;
        
        va.PreAllocTexture(6);
        va.AddTextureVertex(x1, y1, z1, tx1, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);
        va.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);
        va.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x3, y3, z3, tx2, image->tex_coord_y);
        int alpha = (100.0 - transparency) * 255.0 / 100.0;
        va.FinishTextures(GL_TRIANGLES, image->getID(), alpha, brightness);
    } else {
        va3.AddVertex(x1, y1, z1, *wxRED);
        va3.AddVertex(x2, y2, z2, *wxRED);
        va3.AddVertex(x2, y2, z2, *wxRED);
        va3.AddVertex(x3, y3, z3, *wxRED);
        va3.AddVertex(x3, y3, z3, *wxRED);
        va3.AddVertex(x4, y4, z4, *wxRED);
        va3.AddVertex(x4, y4, z4, *wxRED);
        va3.AddVertex(x1, y1, z1, *wxRED);
        va3.AddVertex(x1, y1, z1, *wxRED);
        va3.AddVertex(x3, y3, z3, *wxRED);
        va3.AddVertex(x2, y2, z2, *wxRED);
        va3.AddVertex(x4, y4, z4, *wxRED);
        va3.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
    }

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(va3);
    }
}

std::list<std::string> ImageObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !wxFile::Exists(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image object '%s' cant find image file '%s'", GetName(), _imageFile).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile)) {
            res.push_back(wxString::Format("    WARN: Image object '%s' image file '%s' not under show directory.", GetName(), _imageFile).ToStdString());
        }
    }
    return res;
}

bool ImageObject::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (wxFile::Exists(_imageFile))
    {
        if (!frame->IsInShowFolder(_imageFile))
        {
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
    if (wxFile::Exists(_imageFile))
    {
        res.push_back(_imageFile);
    }
    return res;
}
