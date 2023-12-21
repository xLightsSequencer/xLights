/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "ImageModel.h"
#include "ModelScreenLocation.h"
#include "../ModelPreview.h"
#include "../RenderBuffer.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

#include "../graphics/xlGraphicsAccumulators.h"
#include "../graphics/xlGraphicsContext.h"


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
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void ImageModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
	BufferHi = 1;
	BufferWi = 1;
}

void ImageModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
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

void ImageModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
	wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _imageFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
        "|All files (*.*)|*.*");

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
            if (!label.StartsWith("Single Color")) {
                ((wxEnumProperty*)p)->DeleteChoice(((wxEnumProperty*)p)->GetChoices().Index(label));
            }
        });
    }
}

int ImageModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    IncrementChangeCount();
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
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CandyCaneModel::OnPropertyGridChange::WhiteAsAlpha");
        return 0;
    } else if ("OffBrightness" == event.GetPropertyName()) {
        _offBrightness = event.GetValue().GetInteger();
        ModelXml->DeleteAttribute("OffBrightness");
        ModelXml->AddAttribute("OffBrightness", wxString::Format("%d", _offBrightness));
        IncrementChangeCount();
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
    if (_imageFile != ModelXml->GetAttribute("Image", "")) {
        ModelXml->DeleteAttribute("Image");
        ModelXml->AddAttribute("Image", _imageFile);
    }
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
    bool mustEnd = false;
    xlGraphicsContext *ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }
    if (ctx) {
        GetModelScreenLocation().PrepareToDraw(false, false);

        int w, h;
        preview->GetSize(&w, &h);

        xlTexture *texture = _images[preview->GetName().ToStdString()];
        if (texture == nullptr && FileExists(_imageFile)) {
            wxImage img(_imageFile);
            if (img.IsOk()) {
                bool mAlpha = img.HasAlpha();
                if (!mAlpha && _whiteAsAlpha) {
                    img.InitAlpha();
                    for (int x = 0; x < img.GetWidth(); x++) {
                        for (int y = 0; y < img.GetHeight(); y++) {
                            int r = img.GetRed(x,y);
                            if (r == img.GetGreen(x, y) && r == img.GetBlue(x, y)) {
                                img.SetAlpha(x, y, r);
                            }
                        }
                    }
                }
                width = img.GetWidth();
                height = img.GetHeight();
                texture = ctx->createTexture(img);
                texture->SetName(GetName());
                texture->Finalize();
            }
        }
        if (texture) {
            float scaleX = float(w) / float(width);
            float scaleY = float(h) / float(height);
            
            float scale = scaleY < scaleX ? scaleY : scaleX;
            
            float offX = (float(w) - float(width) * scale) / 2.0f;
            float offY = (float(h) - float(height) * scale) / 2.0f;

            float x1 = offX;
            float x2 = offX;
            float x3 = offX + float(width) * scale;
            float x4 = offX + float(width) * scale;
            float y1 = offY;
            float y2 = offY + float(height) * scale;
            float y3 = offY + float(height) * scale;
            float y4 = offY;
            
            int brightness = (100.0 - transparency) * 255.0 / 100.0;
            brightness = (float)_offBrightness + (float)(255 - _offBrightness) * (float)brightness / 255.0 * (float)GetChannelValue(0) / 255.0;
            
            xlVertexTextureAccumulator *va = ctx->createVertexTextureAccumulator();
            
            va->PreAlloc(6);
            va->AddVertex(x1, y1, 0, 0.0, 1.0);
            va->AddVertex(x4, y4, 0, 1.0, 1.0);
            va->AddVertex(x2, y2, 0, 0.0, 0.0);
            va->AddVertex(x2, y2, 0, 0.0, 0.0);
            va->AddVertex(x4, y4, 0, 1.0, 1.0);
            va->AddVertex(x3, y3, 0, 1.0, 0.0);

            preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext *ctx) {
                ctx->drawTexture(va, texture, brightness, 255, 0, va->getCount());
                delete va;
            });
        } else {
            
            xlGraphicsProgram *program = preview->getCurrentSolidProgram();
            xlVertexColorAccumulator *vac= program->getAccumulator();
            int start = vac->getCount();
            
            float offX = float(w) * 0.95;
            float offY = float(h) * 0.95;

            vac->AddVertex(offX, offY, 0, xlRED);
            vac->AddVertex(w - offX, offY, 0, xlRED);

            vac->AddVertex(w - offX, offY, 0, xlRED);
            vac->AddVertex(w - offX, h - offY, 0, xlRED);

            vac->AddVertex(w - offX, h - offY, 0, xlRED);
            vac->AddVertex(offX, h - offY, 0, xlRED);

            vac->AddVertex(offX, h - offY, 0, xlRED);
            vac->AddVertex(offX, offY, 0, xlRED);
            
            vac->AddVertex(offX, offY, 0, xlRED);
            vac->AddVertex(w - offX, h - offY, 0, xlRED);

            vac->AddVertex(w - offX, offY, 0, xlRED);
            vac->AddVertex(offX, h - offY, 0, xlRED);

            int count = vac->getCount();
            program->addStep([=](xlGraphicsContext *ctx) {
                ctx->drawLines(vac, start, count - start);
            });
        }

    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}


void ImageModel::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d,
                                      const xlColor* color, bool allowSelected, bool wiring,
                                      bool highlightFirst, int highlightpixel,
                                      float *boundingBox) {
    GetModelScreenLocation().PrepareToDraw(is_3d, allowSelected);

    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    
    xlTexture *texture = _images[preview->GetName().ToStdString()];
    if (texture == nullptr && FileExists(_imageFile)) {
        wxImage img(_imageFile);
        if (img.IsOk()) {
            bool mAlpha = img.HasAlpha();
            if (!mAlpha && _whiteAsAlpha) {
                img.InitAlpha();
                for (int x = 0; x < img.GetWidth(); x++) {
                    for (int y = 0; y < img.GetHeight(); y++) {
                        int r = img.GetRed(x,y);
                        if (r == img.GetGreen(x, y) && r == img.GetBlue(x, y)) {
                            img.SetAlpha(x, y, r);
                        }
                    }
                }
            }
            
            width = img.GetWidth();
            height = img.GetHeight();
            hasAlpha = img.HasAlpha();
            texture = ctx->createTexture(img);
            texture->SetName(GetName());
            texture->Finalize();
            _images[preview->GetName().ToStdString()] = texture;
        }
    }
    GetModelScreenLocation().UpdateBoundingBox(Nodes);  // FIXME: Modify to only call this when position changes
   
    
    xlGraphicsProgram *program = (transparency != 0 || hasAlpha) ? transparentProgram : solidProgram;
    if (texture) {
        xlVertexTextureAccumulator *va = ctx->createVertexTextureAccumulator();
        
        va->PreAlloc(6);
        va->AddVertex(-0.5, -0.5, 0, 0.0, 1.0);
        va->AddVertex(0.5, -0.5, 0, 1.0, 1.0);
        va->AddVertex(-0.5, 0.5, 0, 0.0, 0.0);
        va->AddVertex(-0.5, 0.5, 0, 0.0, 0.0);
        va->AddVertex(0.5, -0.5, 0, 1.0, 1.0);
        va->AddVertex(0.5, 0.5, 0, 1.0, 0.0);

        int alpha = (100.0 - transparency) / 100.0 * 255.0;
        int brightness = (float)_offBrightness + (float)(100 - _offBrightness) * (float)GetChannelValue(0) / 255.0;
        if (color) {
            brightness = color->red;
            brightness /= 2.55f;
        }

        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            if (!is_3d) {
                //not 3d, flatten to the 0 plane
                ctx->ScaleViewMatrix(1.0f, 1.0f, 0.0f);
            }
            GetModelScreenLocation().ApplyModelViewMatrices(ctx);
            ctx->drawTexture(va, texture, brightness, alpha, 0, va->getCount());
            ctx->PopMatrix();
            delete va;
        });
    } else {
        xlVertexColorAccumulator *vac= program->getAccumulator();
        int start = vac->getCount();
        vac->AddVertex(-0.5, -0.5, 0, xlRED);
        vac->AddVertex(0.5, -0.5, 0, xlRED);

        vac->AddVertex(0.5, -0.5, 0, xlRED);
        vac->AddVertex(0.5, 0.5, 0, xlRED);

        vac->AddVertex(0.5, 0.5, 0, xlRED);
        vac->AddVertex(-0.5, 0.5, 0, xlRED);

        vac->AddVertex(-0.5, 0.5, 0, xlRED);
        vac->AddVertex(-0.5, -0.5, 0, xlRED);

        vac->AddVertex(-0.5, -0.5, 0, xlRED);
        vac->AddVertex(0.5, 0.5, 0, xlRED);

        vac->AddVertex(0.5, -0.5, 0, xlRED);
        vac->AddVertex(-0.5, 0.5, 0, xlRED);

        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            GetModelScreenLocation().ApplyModelViewMatrices(ctx);
            ctx->drawLines(vac, start, count - start);
            ctx->PopMatrix();
        });
    }
    
    if ((Selected || (Highlighted && is_3d)) && color != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

bool ImageModel::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (FileExists(_imageFile)) {
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
    if (FileExists(_imageFile)) {
        res.push_back(_imageFile);
    }
    return res;
}

std::list<std::string> ImageModel::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !wxFile::Exists(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image model '%s' cant find image file '%s'", GetName(), _imageFile).ToStdString());
    } else if (!wxIsReadable(_imageFile) || !wxImage::CanRead(_imageFile)) {
        res.push_back(wxString::Format("    ERR: Image model '%s' cant load image file '%s'", GetName(), _imageFile).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile)) {
            res.push_back(wxString::Format("    WARN: Image model '%s' image file '%s' not under show/media/resource directories.", GetName(), _imageFile).ToStdString());
        }
    }
    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}


int ImageModel::GetChannelValue(int channel)
{
    wxASSERT(channel == 0);

    xlColor c;
    Nodes[channel]->GetColor(c);
    return std::max(c.red, std::max(c.green, c.blue));
}
