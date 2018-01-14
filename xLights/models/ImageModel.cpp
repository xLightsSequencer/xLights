#include "ImageModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "ModelScreenLocation.h"
#include "../ModelPreview.h"
#include "../RenderBuffer.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

ImageModel::ImageModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    _whiteAsAlpha = false;
    _imageFile = "";
    SetFromXml(node, zeroBased);
}

ImageModel::ImageModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
    _imageFile = "";
    _whiteAsAlpha = false;
}

ImageModel::~ImageModel()
{
    //dtor
    for (auto it = _images.begin(); it != _images.end(); ++it)
    {
        delete it->second;
    }
}

void ImageModel::GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const {
	BufferHi = 1;
	BufferWi = 1;
}

void ImageModel::InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                     std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    BufferHi = 1;
    BufferWi = 1;

	newNodes.push_back(NodeBaseClassPtr(Nodes[0]->clone()));
	newNodes[0]->Coords[0].bufX=0;
	newNodes[0]->Coords[0].bufY=0;
}

void ImageModel::AddTypeProperties(wxPropertyGridInterface *grid) {

	wxPGProperty *p = grid->Append(new wxImageFileProperty("Image",
                                             "Image",
                                             _imageFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*");

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
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    else if ("WhiteAsAlpha" == event.GetPropertyName()) {
        _whiteAsAlpha = event.GetValue();
        for (auto it = _images.begin(); it != _images.end(); ++it)
        {
            delete it->second;
        }
        _images.clear();
        ModelXml->DeleteAttribute("WhiteAsAlpha");
        ModelXml->AddAttribute("WhiteAsAlpha", _whiteAsAlpha ? "True" : "False");
        SetFromXml(ModelXml, zeroBased);
        return 3;
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

    SetNodeCount(1, 1, rgbOrder);
	Nodes[0]->ActChan = stringStartChan[0];
	Nodes[0]->StringNum = 0;
    // the screenx/screeny positions are used to fake it into giving a bigger selection area
    Nodes[0]->Coords[0].screenX = -0.5f;
    Nodes[0]->Coords[0].screenY = -0.5f;
    Nodes[0]->AddBufCoord(0, 0);
    Nodes[0]->Coords[1].screenX = 0.5f;
    Nodes[0]->Coords[1].screenY = 0.5f;

    SetBufferSize(1, 1);
    screenLocation.SetRenderSize(1, 1);
}

void ImageModel::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!wxFileExists(_imageFile)) return;

    bool success = preview->StartDrawing(pointSize);

    if(success) {
        DrawGLUtils::xlAccumulator va(maxVertexCount);

        GetModelScreenLocation().PrepareToDraw();

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

		DrawModelOnWindow(preview, va, nullptr, x1, y1, x2, y2, x3, y3, x4, y4, true);

        DrawGLUtils::Draw(va);

        preview->EndDrawing();
    }
}

// display model using colors
void ImageModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, bool allowSelected) 
{
    GetModelScreenLocation().PrepareToDraw();

    if (wxFileExists(_imageFile))
    {
        int w, h;
        preview->GetVirtualCanvasSize(w, h);

        float x1 = -0.5;
        float x2 = -0.5;
        float x3 = 0.5;
        float x4 = 0.5;
        float y1 = -0.5;
        float y2 = 0.5;
        float y3 = 0.5;
        float y4 = -0.5;

        GetModelScreenLocation().TranslatePoint(x1, y1);
        GetModelScreenLocation().TranslatePoint(x2, y2);
        GetModelScreenLocation().TranslatePoint(x3, y3);
        GetModelScreenLocation().TranslatePoint(x4, y4);

        DrawModelOnWindow(preview, va, c, x1, y1, x2, y2, x3, y3, x4, y4, !allowSelected);
    }

    if (Selected && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va);
    }
}

std::list<std::string> ImageModel::GetFileReferences()
{
    std::list<std::string> res;
    res.push_back(_imageFile);
    return res;
}

std::list<std::string> ImageModel::CheckModelSettings()
{
    std::list<std::string> res;

    if (_imageFile == "" || !wxFile::Exists(_imageFile))
    {
        res.push_back(wxString::Format("    ERR: Image model '%s' cant find image file '%s'", GetName(), _imageFile).ToStdString());
    }
    else
    {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _imageFile))
        {
            res.push_back(wxString::Format("    WARN: Image model '%s' image file '%s' not under show directory.", GetName(), _imageFile).ToStdString());
        }
    }
    return res;
}

void ImageModel::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &x1, float &y1, float&x2, float&y2, float& x3, float& y3, float& x4, float& y4, bool active)
{
    if (!wxFileExists(_imageFile)) return;

    if (_images.find(preview->GetName().ToStdString()) == _images.end())
    {
        _images[preview->GetName().ToStdString()] = new Image(_imageFile, _whiteAsAlpha);
    }

    Image* image = _images[preview->GetName().ToStdString()];

    va.PreAllocTexture(65536);
    float tx1 = 0;
    float tx2 = image->tex_coord_x;

    va.AddTextureVertex(x1, y1, tx1, -0.5 / (image->textureHeight));
    va.AddTextureVertex(x4, y4, tx2, -0.5 / (image->textureHeight));
    va.AddTextureVertex(x2, y2, tx1, image->tex_coord_y);

    va.AddTextureVertex(x2, y2, tx1, image->tex_coord_y);
    va.AddTextureVertex(x4, y4, tx2, -0.5 / (image->textureHeight));
    va.AddTextureVertex(x3, y3, tx2, image->tex_coord_y);

    int brightness = (100.0 - transparency) * 255.0 / 100.0;

    if (active)
    {
        brightness = (float)brightness * (float)GetChannelValue(0) / 255.0;
    }

    va.FinishTextures(GL_TRIANGLES, image->getID(), brightness);
    va.Finish(GL_TRIANGLES);
}

int ImageModel::GetChannelValue(int channel)
{
    wxASSERT(channel == 0);

    xlColor c;
    Nodes[channel]->GetColor(c);
    return std::max(c.red, std::max(c.green, c.blue));
}