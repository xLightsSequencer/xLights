/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxGeneral.h"
#include "../ModelScreenLocation.h"
#include "../../controllers/ControllerCaps.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"
#include "DmxColorAbilityRGB.h"
#include "DmxPresetAbility.h"

DmxGeneral::DmxGeneral(const ModelManager &manager)
  : DmxModel(manager)
{
    color_ability = std::make_unique<DmxColorAbilityRGB>();
}

DmxGeneral::~DmxGeneral()
{
}

void DmxGeneral::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    DmxModel::AddTypeProperties(grid, outputManager);

    if (nullptr != color_ability) {
        ControllerCaps *caps = GetControllerCaps();
        color_ability->AddColorTypeProperties(grid, IsPWMProtocol() && caps && caps->SupportsPWM());
    }
}

int DmxGeneral::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (nullptr != color_ability && color_ability->OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxGeneral::InitModel()
{
    DmxModel::InitModel();
    screenLocation.SetRenderSize(1, 1, 1);
}

bool DmxGeneral::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) {
    if (root->GetName() == "dmxgeneral") {

        wxString name = root->GetAttribute("name");
        //wxString v = root->GetAttribute("SourceVersion");

        wxString sc = root->GetAttribute("DmxShutterChannel");
        wxString so = root->GetAttribute("DmxShutterOpen");
        wxString sov = root->GetAttribute("DmxShutterOnValue");
        wxString bl = root->GetAttribute("DmxBeamLimit");
        wxString dbl = root->GetAttribute("DmxBeamLength", "1");
        wxString dbw = root->GetAttribute("DmxBeamWidth", "1");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("DmxShutterChannel", sc);
        SetProperty("DmxShutterOpen", so);
        SetProperty("DmxShutterOnValue", sov);
        SetProperty("DmxBeamLimit", bl);
        SetProperty("DmxBeamLength", dbl);
        SetProperty("DmxBeamWidth", dbw);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        SetProperty("name", newname, true);

        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y, min_z, max_z);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxGeneral::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxGeneral::ImportXlightsModel");
        return true;
    } else {
        DisplayError("Failure loading DmxGeneral model file.");
        return false;
    }
}

void DmxGeneral::DrawModel(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is3d, bool active, const xlColor* c)
{
    size_t nodeCount = Nodes.size();

    if (!color_ability->IsValidModelSettings(this) || !preset_ability->IsValidModelSettings(this)) {
        DmxModel::DrawInvalid(sprogram, &(GetModelScreenLocation()), false, false);
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor black(xlBLACK);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int trans = color == xlBLACK ? blackTransparency : transparency;
    /*
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy = xlBLACK;
        if (white_channel > 0) {
            Nodes[white_channel - 1]->GetColor(proxy);
        }

        if (proxy == xlBLACK) {
            Nodes[red_channel - 1]->GetColor(proxy);
            Nodes[green_channel - 1]->GetColor(proxy);
            Nodes[blue_channel - 1]->GetColor(proxy);
        }
    } else if (white_channel > 0) {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
    }
    */
    ApplyTransparency(ccolor, trans, trans);

    // draw the bars
    xlColor proxy;
    xlColor red(xlRED);
    xlColor green(xlGREEN);
    xlColor blue(xlBLUE);
    xlColor white(xlWHITE);
    ApplyTransparency(red, trans, trans);
    ApplyTransparency(green, trans, trans);
    ApplyTransparency(blue, trans, trans);

    auto tvac = tprogram->getAccumulator();
    int tStart = tvac->getCount();
    auto vac = sprogram->getAccumulator();
    int startVert = vac->getCount();

    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->Translate(-0.25f, 0, 0);
        ctx->Scale(0.5f, 0.5f, 1.0f);
    });
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->Translate(-0.25f, 0, 0);
        ctx->Scale(0.5f, 0.5f, 1.0f);
    });

    vac->AddRectAsTriangles(-0.5f, 0.9f, -0.45f, -0.9f, ccolor); // left side
    vac->AddRectAsTriangles(1.5f, 0.9f, 1.45f, -0.9f, ccolor); // right side
    vac->AddRectAsTriangles(-0.5f, 0.9f, 1.5f, 0.85f, ccolor); // top side
    vac->AddRectAsTriangles(-0.5f, -0.9f, 1.5f, -0.85f, ccolor); // bottom side

    auto rgbColor = static_cast<DmxColorAbilityRGB*>(color_ability.get());

    float lineSize = 1.7f / ((float)nodeCount);
    float barSize = lineSize * 0.8f;
    float lineStart = 0.825f;
    for (int i = 1; i <= nodeCount; ++i) {
        Nodes[i - 1]->GetColor(proxy);
        float val = (float)proxy.red;
        float offsetx = val / 255.0f * 1.8f;
        if (i == rgbColor->GetRedChannel()) {
            proxy = red;
        } else if (i == rgbColor->GetGreenChannel()) {
            proxy = green;
        } else if (i == rgbColor->GetBlueChannel()) {
            proxy = blue;
        } else if (i == rgbColor->GetWhiteChannel()) {
            proxy = white;
        } else {
            proxy = ccolor;
        }
        vac->AddRectAsTriangles(-0.4f, lineStart, -0.4f + offsetx, lineStart - barSize, 0.3f, proxy);
        lineStart -= lineSize;
    }

    int tEnd = tvac->getCount();
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->drawTriangles(tvac, tStart, tEnd - tStart);
    });

    int end = vac->getCount();
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->drawTriangles(vac, startVert, end - startVert);
    });
}

void DmxGeneral::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                         xlGraphicsProgram* sprogram, xlGraphicsProgram* tprogram, bool is_3d,
                                         const xlColor* c, bool allowSelected, bool wiring,
                                         bool highlightFirst, int highlightpixel,
                                         float* boundingBox)
{
    if (!IsActive())
        return;

    screenLocation.PrepareToDraw(is_3d, allowSelected);
    screenLocation.UpdateBoundingBox(Nodes);
    if (boundingBox) {
        boundingBox[0] = -0.5;
        boundingBox[1] = -0.5;
        boundingBox[2] = -0.5;
        boundingBox[3] = 0.5;
        boundingBox[4] = 0.5;
        boundingBox[5] = 0.5;
    }

    sprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    tprogram->addStep([=, this](xlGraphicsContext* ctx) {
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0.5 plane
            ctx->Translate(0, 0, 0.5f);
            ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
    });
    DrawModel(preview, ctx, sprogram, tprogram, is_3d, !allowSelected, c);
    sprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    tprogram->addStep([=](xlGraphicsContext* ctx) {
        ctx->PopMatrix();
    });
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(tprogram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxGeneral::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
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
        int w, h;
        preview->GetSize(&w, &h);
        float scaleX = float(w) * 0.95f / float(GetModelScreenLocation().RenderWi);
        float scaleY = float(h) * 0.95f / float(GetModelScreenLocation().RenderHt);
        if (GetModelScreenLocation().RenderDp > 1) {
            float scaleZ = float(w) * 0.95f / float(GetModelScreenLocation().RenderDp);
            scaleX = std::min(scaleX, scaleZ);
        }

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

        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.5f);
            ctx->Scale(scaleX, scaleY, 0.001f);
        });
        DrawModel(preview, ctx, preview->getCurrentSolidProgram(), preview->getCurrentTransparentProgram(), false, true, nullptr);
        preview->getCurrentTransparentProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}
