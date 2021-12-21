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

#include "DmxGeneral.h"
#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxGeneral::DmxGeneral(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
  : DmxModel(node, manager, zeroBased)
{
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxGeneral::~DmxGeneral()
{
    //dtor
}

void DmxGeneral::AddTypeProperties(wxPropertyGridInterface *grid) {

    DmxModel::AddTypeProperties(grid);

    AddColorTypeProperties(grid);
}

int DmxGeneral::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxGeneral::InitModel() {
    DmxModel::InitModel();

    DisplayAs = "DmxGeneral";

    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;

    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "0"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "0"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "0"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));

    screenLocation.SetRenderSize(1, 1, 1);
}


void DmxGeneral::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d,
                                      const xlColor* c, bool allowSelected, bool wiring,
                                      bool highlightFirst, int highlightpixel,
                                      float *boundingBox)  {
    if (!IsActive()) return;

    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(Nodes);

    size_t NodeCount = Nodes.size();
    if (red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount) {
        return;
    }
    auto vac = solidProgram->getAccumulator();
    int start = vac->getCount();
    DrawModel(vac, c, allowSelected);
    int end = vac->getCount();
    solidProgram->addStep([=](xlGraphicsContext*ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->Scale(1.0, 1.0, 0.0);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
        ctx->drawTriangles(vac, start, end - start);

        ctx->PopMatrix();
    });
    
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted);
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
        }
    }
}
void DmxGeneral::DisplayEffectOnWindow(ModelPreview* preview, double pointSize) {
    if (!IsActive() && preview->IsNoCurrentModel()) { return; }
    
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
        xlGraphicsProgram *p = preview->getCurrentSolidProgram();
        auto vac = p->getAccumulator();
        int start = vac->getCount();
        DrawModel(vac, nullptr, false);
        int end = vac->getCount();
        
        int w, h;
        preview->GetSize(&w, &h);
        float scaleX = float(w) * 0.95;
        float scaleY = float(h) * 0.95;
        float ml, mb;
        GetMinScreenXY(ml, mb);
        
        p->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            ctx->Translate(w/2.0f - (ml < 0.0f ? ml : 0.0f),
                           h/2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->Scale(scaleX, scaleY, 1.0);

            ctx->drawTriangles(vac, start, end - start);
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}
void DmxGeneral::DrawModel(xlVertexColorAccumulator *vac, const xlColor *c, bool allowSelected) {
    size_t nodeCount = Nodes.size();

    xlColor ccolor(xlWHITE);
    xlColor black(xlBLACK);
    xlColor color;
    GetColor(color, transparency, blackTransparency, allowSelected, c, Nodes);

    float bar_gap = (0.2 / (nodeCount));
    float bar_height = (0.8 / (nodeCount + 1));

    // draw the bars
	xlColor proxy;
	xlColor red(xlRED);
	xlColor green(xlGREEN);
	xlColor blue(xlBLUE);
	xlColor white(xlWHITE);
    
    vac->AddRectAsTriangles(-0.5, -0.5, 0.5, -0.46, ccolor);
    vac->AddRectAsTriangles(-0.5, 0.5, 0.5, 0.46, ccolor);
    
    vac->AddRectAsTriangles(-0.5, -0.5, -0.46, 0.5, ccolor);
    vac->AddRectAsTriangles(0.5, -0.5, 0.46, 0.5, ccolor);

    float y = 0.45;
    for (int i = 1; i <= nodeCount; ++i) {
		Nodes[i - 1]->GetColor(proxy);
		float val = (float)proxy.red;
		float offsetx = val / 255.0 * 0.90 - 0.45;
		if (i == red_channel) {
			proxy = red;
		} else if (i == green_channel) {
			proxy = green;
		} else if (i == blue_channel) {
			proxy = blue;
		} else if (i == white_channel) {
			proxy = white;
		} else {
			proxy = ccolor;
		}
        vac->AddRectAsTriangles(-0.45, y, offsetx, y - bar_height, proxy);
        y -= bar_height;
        y -= bar_gap;
	}
}

void DmxGeneral::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxgeneral \n");

    ExportBaseParameters(f);

    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");

    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));

    f.Write(" >\n");

    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    f.Write("</dmxgeneral>");
    f.Close();
}

void DmxGeneral::ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) {
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxgeneral")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");

            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");
            wxString sc = root->GetAttribute("DmxShutterChannel");
            wxString so = root->GetAttribute("DmxShutterOpen");
            wxString bl = root->GetAttribute("DmxBeamLimit");
            wxString dbl = root->GetAttribute("DmxBeamLength", "1");
            wxString dbw = root->GetAttribute("DmxBeamWidth", "1");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            ImportModelChildren(root, xlights, newname);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxGeneral::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxGeneral::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxGeneral model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxGeneral model file.");
    }
}
