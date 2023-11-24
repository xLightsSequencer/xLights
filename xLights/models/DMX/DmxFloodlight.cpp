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
#include "DmxColorAbilityRGB.h"
#include "DmxPresetAbility.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodlight::DmxFloodlight(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), beam_length(1)
{
    SetFromXml(node, zeroBased);
}

DmxFloodlight::~DmxFloodlight()
{
    //dtor
}

void DmxFloodlight::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    DmxModel::AddTypeProperties(grid, outputManager);
    if (nullptr != color_ability) {
        color_ability->AddColorTypeProperties(grid);
    }
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

    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
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

    color_ability = std::make_unique<DmxColorAbilityRGB>(ModelXml);

    shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
    shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    shutter_on_value = wxAtoi(ModelXml->GetAttribute("DmxShutterOnValue", "0"));
    beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "1.0"));
    screenLocation.SetRenderSize(1, 1, 1);
}

void DmxFloodlight::GetColors(xlColor& center, xlColor& edge, bool allowSelected, const xlColor* c)
{
    color_ability->GetColor(center, transparency, blackTransparency, allowSelected, c, Nodes);
    edge = center;
    if (_pixelStyle != PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE) {
        edge.alpha = 0;
    }
}

void DmxFloodlight::DrawModel(xlVertexColorAccumulator* vac, xlColor& center, xlColor& edge, float beam_length)
{
    vac->AddCircleAsTriangles(0, 0, 0, 0.5, center, edge, beam_length);
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
    if (boundingBox) {
        boundingBox[0] = -0.5;
        boundingBox[1] = -0.5;
        boundingBox[2] = -0.5;
        boundingBox[3] = 0.5;
        boundingBox[4] = 0.5;
        boundingBox[5] = 0.5;
    }

    // determine if shutter is open for floods that support it
    bool shutter_open = allowSelected || IsShutterOpen(Nodes);

    if (!color_ability->IsValidModelSettings(this) || !preset_ability->IsValidModelSettings(this)) {
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
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
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


    wxString dbl = ModelXml->GetAttribute("DmxBeamLength", "1");

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    ExportBaseParameters(f);
    color_ability->ExportParameters(f,ModelXml);
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
    //ExportDimensions(f);
    f.Write("</dmxmodel>");
    f.Close();
}

void DmxFloodlight::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "dmxmodel") {
        ImportBaseParameters(root);

        wxString name = root->GetAttribute("name");
        wxString v = root->GetAttribute("SourceVersion");

        wxString dbl = root->GetAttribute("DmxBeamLength", "1");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        color_ability->ImportParameters(root, this);
        SetProperty("DmxBeamLength", dbl);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxFloodlight::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxFloodlight::ImportXlightsModel");
    } else {
        DisplayError("Failure loading DmxFloodlight model file.");
    }
}

std::vector<std::string> DmxFloodlight::GenerateNodeNames() const
{
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_channel && shutter_channel < names.size()) {
        names[shutter_channel - 1] = "Shutter";
    }
    return names;
}
