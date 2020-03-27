#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "ImageModel.h"
#include "ModelScreenLocation.h"
#include "../ModelPreview.h"
#include "../RenderBuffer.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

ImageModel::ImageModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    _whiteAsAlpha = false;
    _offBrightness = 80;
    _imageFile = "";
    SetFromXml(node, zeroBased);
}

ImageModel::ImageModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
    _imageFile = "";
    _whiteAsAlpha = false;
    _offBrightness = 80;
}

ImageModel::~ImageModel()
{
    //dtor
    for (auto it = _images.begin(); it != _images.end(); ++it)
    {
        delete it->second;
    }
}

void ImageModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const {
	BufferHi = 1;
	BufferWi = 1;
}

void ImageModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    BufferHi = 1;
    BufferWi = 1;

    //newNodes.push_back(NodeBaseClassPtr(Nodes[0]->clone()));
    NodeBaseClass* node = Nodes[0]->clone();

    // remove one of the coordinates
    node->Coords.erase(node->Coords.begin());

    // set it to zero zero
    node->Coords[0].bufX = 0;
    node->Coords[0].bufY = 0;
    node->Coords[0].bufZ = 0;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    GetModelScreenLocation().TranslatePoint(x, y, z);
    node->Coords[0].screenX = x;
    node->Coords[0].screenY = y;
    node->Coords[0].screenZ = z;

    newNodes.push_back(NodeBaseClassPtr(node));
}

void ImageModel::AddTypeProperties(wxPropertyGridInterface *grid) {

	wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _imageFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Off Brightness", "OffBrightness", _offBrightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Read White As Alpha",
        "WhiteAsAlpha",
        _whiteAsAlpha));
    p->SetAttribute("UseCheckbox", true);
}

void ImageModel::DisableUnusedProperties(wxPropertyGridInterface *grid)
{
    wxPGProperty* p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStates");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStrandNodeNames");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("SubModels");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelBlackTransparency");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        wxArrayString labels = ((wxEnumProperty*)p)->GetChoices().GetLabels();
        std::for_each(labels.begin(), labels.end(), [&p](wxString label) {
            if (!label.Contains("Single Color")) {
                ((wxEnumProperty*)p)->DeleteChoice(((wxEnumProperty*)p)->GetChoices().Index(label));
            }
        });
    }
}

int ImageModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("Image" == event.GetPropertyName()) {
        for (auto it = _images.begin(); it != _images.end(); ++it)
        {
            delete it->second;
        }
        _images.clear();
        _imageFile = event.GetValue().GetString();
        ModelXml->DeleteAttribute("Image");
        ModelXml->AddAttribute("Image", _imageFile);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::Image");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::Image");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::Image");
        return 0;
    } else if ("WhiteAsAlpha" == event.GetPropertyName()) {
        _whiteAsAlpha = event.GetValue();
        for (auto it = _images.begin(); it != _images.end(); ++it) {
            delete it->second;
        }
        _images.clear();
        ModelXml->DeleteAttribute("WhiteAsAlpha");
        ModelXml->AddAttribute("WhiteAsAlpha", _whiteAsAlpha ? "True" : "False");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        return 0;
    } else if ("OffBrightness" == event.GetPropertyName()) {
        _offBrightness = event.GetValue().GetInteger();
        ModelXml->DeleteAttribute("OffBrightness");
        ModelXml->AddAttribute("OffBrightness", wxString::Format("%d", _offBrightness));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::OffBrightness");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::OffBrightness");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::OffBrightness");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void ImageModel::InitModel()
{
    DisplayAs = "Image";
    parm2 = 1;
    parm3 = 1;

	_imageFile = FixFile("", ModelXml->GetAttribute("Image", ""));
	_whiteAsAlpha = ModelXml->GetAttribute("WhiteAsAlpha", "False") == "True";
	_offBrightness = wxAtoi(ModelXml->GetAttribute("OffBrightness", "80"));

    SetNodeCount(1, 1, rgbOrder);
	Nodes[0]->ActChan = stringStartChan[0];
	Nodes[0]->StringNum = 0;
    // the screenx/screeny positions are used to fake it into giving a bigger selection area
    Nodes[0]->Coords[0].screenX = -0.5f;
    Nodes[0]->Coords[0].screenY = -0.5f;
    Nodes[0]->Coords[0].screenZ = -0.5f;
    Nodes[0]->AddBufCoord(0, 0);
    Nodes[0]->Coords[1].screenX = 0.5f;
    Nodes[0]->Coords[1].screenY = 0.5f;
    Nodes[0]->Coords[1].screenZ = 0.5f;

    SetBufferSize(1, 1);
    screenLocation.SetRenderSize(1, 1);
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

void ImageModel::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    bool success = preview->StartDrawing(pointSize);

    if (success) {
        DrawGLUtils::xlAccumulator va(maxVertexCount);
        DrawGLUtils::xlAccumulator tva(maxVertexCount);

        GetModelScreenLocation().PrepareToDraw(false, false);

        int w, h;
        preview->GetSize(&w, &h);

        float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;
        float scale = scaleY < scaleX ? scaleY : scaleX;
        scale = 1;

        float x1 = 0;
        float x2 = 0;
        float x3 = w * scale;
        float x4 = w * scale;
        float y1 = 0;
        float y2 = h * scale;
        float y3 = h * scale;
        float y4 = 0;

		DrawModelOnWindow(preview, va, tva, nullptr, x1, y1, x2, y2, x3, y3, x4, y4, true);

        DrawGLUtils::Draw(va);
        DrawGLUtils::Draw(tva);
        maxVertexCount = std::max(va.count, tva.count);
        preview->EndDrawing();
    }
}

// display model using colors
void ImageModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, DrawGLUtils::xlAccumulator &tva, bool is_3d, const xlColor *c, bool allowSelected)
{
    GetModelScreenLocation().PrepareToDraw(is_3d, allowSelected);

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    float x1 = -0.5f;
    float x2 = -0.5f;
    float x3 = 0.5f;
    float x4 = 0.5f;
    float y1 = -0.5f;
    float y2 = 0.5f;
    float y3 = 0.5f;
    float y4 = -0.5f;
    float z1 = 0.0f;
    float z2 = 0.0f;
    float z3 = 0.0f;
    float z4 = 0.0f;

    GetModelScreenLocation().TranslatePoint(x1, y1, z1);
    GetModelScreenLocation().TranslatePoint(x2, y2, z2);
    GetModelScreenLocation().TranslatePoint(x3, y3, z3);
    GetModelScreenLocation().TranslatePoint(x4, y4, z4);

    GetModelScreenLocation().UpdateBoundingBox(Nodes);  // FIXME: Modify to only call this when position changes
    DrawModelOnWindow(preview, va, tva, c, x1, y1, x2, y2, x3, y3, x4, y4, !allowSelected);

    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

void ImageModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va,
                                      DrawGLUtils::xl3Accumulator &tva, DrawGLUtils::xl3Accumulator& lva, bool is_3d, const xlColor *c, bool allowSelected, bool wiring, bool highlightFirst, int highlightpixel)
{
    GetModelScreenLocation().PrepareToDraw(is_3d, allowSelected);

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    float x1 = -0.5f;
    float x2 = -0.5f;
    float x3 = 0.5f;
    float x4 = 0.5f;
    float y1 = -0.5f;
    float y2 = 0.5f;
    float y3 = 0.5f;
    float y4 = -0.5f;
    float z1 = 0.0f;
    float z2 = 0.0f;
    float z3 = 0.0f;
    float z4 = 0.0f;

    GetModelScreenLocation().TranslatePoint(x1, y1, z1);
    GetModelScreenLocation().TranslatePoint(x2, y2, z2);
    GetModelScreenLocation().TranslatePoint(x3, y3, z3);
    GetModelScreenLocation().TranslatePoint(x4, y4, z4);

    GetModelScreenLocation().UpdateBoundingBox(Nodes);  // FIXME: Modify to only call this when position changes
    DrawModelOnWindow(preview, va, tva, c, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, !allowSelected);

    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

bool ImageModel::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (wxFile::Exists(_imageFile)) {
        if (!frame->IsInShowFolder(_imageFile)) {
            _imageFile = frame->MoveToShowFolder(_imageFile, wxString(wxFileName::GetPathSeparator()) + "Images");
            ModelXml->DeleteAttribute("Image");
            ModelXml->AddAttribute("Image", _imageFile);
            SetFromXml(ModelXml, zeroBased);
            rc = true;
        }
    }

    return Model::CleanupFileLocations(frame) || rc;
}

std::list<std::string> ImageModel::GetFileReferences()
{
    std::list<std::string> res;
    if (wxFile::Exists(_imageFile)) {
        res.push_back(_imageFile);
    }
    return res;
}

std::list<std::string> ImageModel::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !wxFile::Exists(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image model '%s' cant find image file '%s'", GetName(), _imageFile).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile)) {
            res.push_back(wxString::Format("    WARN: Image model '%s' image file '%s' not under show directory.", GetName(), _imageFile).ToStdString());
        }
    }
    return res;
}

void ImageModel::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, DrawGLUtils::xlAccumulator &tva, const xlColor *c,
                                   float &x1, float &y1, float&x2, float&y2, float& x3, float& y3, float& x4, float& y4, bool active)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool exists = false;
    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (!wxFileExists(_imageFile)) {
            va.AddVertex(x1, y1, *wxRED);
            va.AddVertex(x2, y2, *wxRED);
            va.AddVertex(x2, y2, *wxRED);
            va.AddVertex(x3, y3, *wxRED);
            va.AddVertex(x3, y3, *wxRED);
            va.AddVertex(x4, y4, *wxRED);
            va.AddVertex(x4, y4, *wxRED);
            va.AddVertex(x1, y1, *wxRED);
            va.AddVertex(x1, y1, *wxRED);
            va.AddVertex(x3, y3, *wxRED);
            va.AddVertex(x2, y2, *wxRED);
            va.AddVertex(x4, y4, *wxRED);
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
        } else {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                              (const char *)GetName().c_str(),
                              (const char *)_imageFile.c_str(),
                              (const char *)preview->GetName().c_str());
            _images[preview->GetName().ToStdString()] = new Image(_imageFile, _whiteAsAlpha);
            exists = true;
        }
    } else {
        exists = true;
    }

    if (exists) {
        Image* image = _images[preview->GetName().ToStdString()];

        DrawGLUtils::xlAccumulator &va2 = transparency == 0 ? va : tva;
        
        float tx1 = 0;
        float tx2 = image->tex_coord_x;

        va2.PreAllocTexture(6);
        va2.AddTextureVertex(x1, y1, tx1, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x4, y4, tx2, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x2, y2, tx1, image->tex_coord_y);

        va2.AddTextureVertex(x2, y2, tx1, image->tex_coord_y);
        va2.AddTextureVertex(x4, y4, tx2, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x3, y3, tx2, image->tex_coord_y);

        int brightness = (100.0 - transparency) * 255.0 / 100.0;

        if (active) {
            brightness = (float)_offBrightness + (float)(255 - _offBrightness) * (float)brightness / 255.0 * (float)GetChannelValue(0) / 255.0;
        }

        va2.FinishTextures(GL_TRIANGLES, image->getID(), brightness, 100.0f);
    }

    if (c != nullptr && (c->red != c->green || c->red != c->blue)) {
        va.AddVertex(x1 - 2, y1 - 2, *c);
        va.AddVertex(x2 - 2, y2 + 2, *c);
        va.AddVertex(x2 - 2, y2 + 2, *c);
        va.AddVertex(x3 + 2, y3 + 2, *c);
        va.AddVertex(x3 + 2, y3 + 2, *c);
        va.AddVertex(x4 + 2, y4 - 2, *c);
        va.AddVertex(x4 + 2, y4 - 2, *c);
        va.AddVertex(x1 - 2, y1 - 2, *c);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }
}

void ImageModel::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, DrawGLUtils::xl3Accumulator &tva,
                                   const xlColor *c,
                                   float &x1, float &y1, float &z1,
                                   float &x2, float &y2, float &z2,
                                   float &x3, float &y3, float &z3,
                                   float &x4, float &y4, float &z4, bool active)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool exists = false;
    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (!wxFileExists(_imageFile)) {
            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x1, y1, z1, *wxRED);
            va.AddVertex(x3, y3, z3, *wxRED);
            va.AddVertex(x2, y2, z2, *wxRED);
            va.AddVertex(x4, y4, z4, *wxRED);
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
        } else {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char *)GetName().c_str(),
                (const char *)_imageFile.c_str(),
                (const char *)preview->GetName().c_str());
            _images[preview->GetName().ToStdString()] = new Image(_imageFile, _whiteAsAlpha);
            exists = true;
        }
    } else {
        exists = true;
    }

    if (exists) {
        Image* image = _images[preview->GetName().ToStdString()];
        DrawGLUtils::xlAccumulator &va2 = transparency == 0 ? va : tva;

        va2.PreAllocTexture(6);
        float tx1 = 0;
        float tx2 = image->tex_coord_x;

        va2.AddTextureVertex(x1, y1, z1, tx1, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);

        va2.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);
        va2.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va2.AddTextureVertex(x3, y3, z3, tx2, image->tex_coord_y);

        int brightness = (100.0 - transparency) * 255.0 / 100.0;

        if (active) {
            brightness = (float)_offBrightness + (float)(255 - _offBrightness) * (float)brightness / 255.0 * (float)GetChannelValue(0) / 255.0;
        }

        va2.FinishTextures(GL_TRIANGLES, image->getID(), brightness, 100.0f);
    }

    if (c != nullptr && (c->red != c->green || c->red != c->blue)) {
        va.AddVertex(x1 - 2, y1 - 2, z1, *c);
        va.AddVertex(x2 - 2, y2 + 2, z2, *c);
        va.AddVertex(x2 - 2, y2 + 2, z2, *c);
        va.AddVertex(x3 + 2, y3 + 2, z3, *c);
        va.AddVertex(x3 + 2, y3 + 2, z3, *c);
        va.AddVertex(x4 + 2, y4 - 2, z4, *c);
        va.AddVertex(x4 + 2, y4 - 2, z4, *c);
        va.AddVertex(x1 - 2, y1 - 2, z1, *c);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }
}
int ImageModel::GetChannelValue(int channel)
{
    wxASSERT(channel == 0);

    xlColor c;
    Nodes[channel]->GetColor(c);
    return std::max(c.red, std::max(c.green, c.blue));
}
