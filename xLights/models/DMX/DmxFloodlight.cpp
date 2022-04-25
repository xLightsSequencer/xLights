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

#include "DmxFloodlight.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodlight::DmxFloodlight(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), beam_length(1)
{
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxFloodlight::~DmxFloodlight()
{
    //dtor
}

void DmxFloodlight::AddTypeProperties(wxPropertyGridInterface* grid) {

    DmxModel::AddTypeProperties(grid);
    AddColorTypeProperties(grid);
    AddShutterTypeProperties(grid);

    auto p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

void DmxFloodlight::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    // disable these because the size of the object is determined by the size of the bounding box
    wxPGProperty* p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) {
        p->Enable(false);
    }

    DmxModel::DisableUnusedProperties(grid);
}

int DmxFloodlight::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        return 0;
    }

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnShutterPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxFloodlight::InitModel()
{
    DmxModel::InitModel();
    DisplayAs = "DmxFloodlight";
    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "1"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "2"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "3"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
    shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
    shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "1.0"));
    screenLocation.SetRenderSize(1, 1, 1);
}

void DmxFloodlight::GetColors(xlColor& center, xlColor& edge, bool allowSelected, const xlColor* c)
{
    GetColor(center, transparency, blackTransparency, allowSelected, c, Nodes);
    edge = center;
    if (_pixelStyle != PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE) {
        edge.alpha = 0;
    }
}

void DmxFloodlight::DrawModel(xlVertexColorAccumulator* vac, xlColor& center, xlColor& edge, float beam_length)
{
    vac->AddCircleAsTriangles(0, 0, 0, 0.5, center, edge, beam_length);
}

std::list<std::string> DmxFloodlight::CheckModelSettings()
{
    std::list<std::string> res;

    int nodeCount = Nodes.size();

    if (red_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s red channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), red_channel, nodeCount));
    }
    if (green_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s green channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), green_channel, nodeCount));
    }
    if (blue_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s blue channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), blue_channel, nodeCount));
    }
    if (white_channel > nodeCount) {
        res.push_back(wxString::Format("    ERR: Model %s white channel refers to a channel (%d) not present on the model which only has %d channels.", GetName(), white_channel, nodeCount));
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxFloodlight::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                         xlGraphicsProgram* solidProgram, xlGraphicsProgram* transparentProgram, bool is_3d,
                                         const xlColor* c, bool allowSelected, bool wiring,
                                         bool highlightFirst, int highlightpixel,
                                         float* boundingBox)
{
    if (!IsActive())
        return;

    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(1, 1, 1);

    // determine if shutter is open for floods that support it
    bool shutter_open = allowSelected || IsShutterOpen(Nodes);

    size_t NodeCount = Nodes.size();
    if (red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount) {
        DmxModel::DrawInvalid(solidProgram, &(GetModelScreenLocation()), is_3d, true);
    } else {
        xlColor center, edge;
        GetColors(center, edge, allowSelected, c);

        // beam length doesn't use the zscale, it draws out of our normal bounding box
        // we need to calculate a length
        float rh = ((BoxedScreenLocation)screenLocation).GetMWidth();
        float rw = ((BoxedScreenLocation)screenLocation).GetMHeight();
        float min_size = (float)(std::min(rh, rw));

        if (shutter_open) {
            auto* vac = transparentProgram->getAccumulator();
            int start = vac->getCount();
            DrawModel(vac, center, edge, is_3d ? beam_length * min_size : 0);
            int end = vac->getCount();
            transparentProgram->addStep([=](xlGraphicsContext* ctx) {
                ctx->PushMatrix();
                if (!is_3d) {
                    //not 3d, flatten to the 0 plane
                    ctx->Scale(1.0, 1.0, 0.0);
                }
                GetModelScreenLocation().ApplyModelViewMatrices(ctx);
                ctx->drawTriangles(vac, start, end - start);
                ctx->PopMatrix();
            });
        }
    }
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted);
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
        }
    }
}

void DmxFloodlight::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool shutter_open = IsShutterOpen(Nodes);
    if (!shutter_open) {
        return;
    }
    bool mustEnd = false;
    xlGraphicsContext* ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }
    if (ctx) {
        xlColor center, edge;
        GetColors(center, edge, false, nullptr);
        xlGraphicsProgram* p = preview->getCurrentTransparentProgram();
        auto vac = p->getAccumulator();
        int start = vac->getCount();
        DrawModel(vac, center, edge, 0);
        int end = vac->getCount();

        int w, h;
        preview->GetSize(&w, &h);
        float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;

        float aspect = screenLocation.GetScaleX();
        aspect /= screenLocation.GetScaleY();
        if (scaleY < scaleX) {
            scaleX = scaleY * aspect;
        } else {
            scaleY = scaleX / aspect;
        }
        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        p->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->Scale(scaleX, scaleY, 1.0);

            ctx->drawTriangles(vac, start, end - start);
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

void DmxFloodlight::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    ExportBaseParameters(f);

    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    wxString dbl = ModelXml->GetAttribute("DmxBeamLength", "1");

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
    f.Write(wxString::Format("DmxBeamLength=\"%s\" ", dbl));

    f.Write(" >\n");
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    f.Write("</dmxmodel>");
    f.Close();
}

void DmxFloodlight::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "dmxmodel") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");

        wxString rc = root->GetAttribute("DmxRedChannel");
        wxString gc = root->GetAttribute("DmxGreenChannel");
        wxString bc = root->GetAttribute("DmxBlueChannel");
        wxString wc = root->GetAttribute("DmxWhiteChannel");
        wxString dbl = root->GetAttribute("DmxBeamLength", "1");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("DmxRedChannel", rc);
        SetProperty("DmxGreenChannel", gc);
        SetProperty("DmxBlueChannel", bc);
        SetProperty("DmxWhiteChannel", wc);
        SetProperty("DmxBeamLength", dbl);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportModelChildren(root, xlights, newname);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxFloodlight::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxFloodlight::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxFloodlight model file.");
    }
}
