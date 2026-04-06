/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxFloodlight.h"
#include "DmxBeamAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxPresetAbility.h"
#include "DmxShutterAbility.h"
#include "../../graphics/IModelPreview.h"
#include "../ModelManager.h"
#include "../../graphics/xlGraphicsContext.h"
#include "../../graphics/xlGraphicsAccumulators.h"
#include "UtilFunctions.h"
#include "xLightsVersion.h"
#include "../../XmlSerializer/XmlNodeKeys.h"

DmxFloodlight::DmxFloodlight(const ModelManager &manager)
    : DmxModel(manager)
{
    DisplayAs = DisplayAsType::DmxFloodlight;
    color_ability = std::make_unique<DmxColorAbilityRGB>();
    shutter_ability = std::make_unique<DmxShutterAbility>();
    beam_ability = std::make_unique<DmxBeamAbility>();
}

DmxFloodlight::~DmxFloodlight()
{
}

void DmxFloodlight::InitModel()
{
    DmxModel::InitModel();
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

void DmxFloodlight::DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext* ctx,
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
    bool shutter_open = allowSelected || shutter_ability->IsShutterOpen(Nodes);

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
            DrawModel(vac, center, edge, is_3d ? beam_ability->GetBeamLength() * min_size : 0);
            int end = vac->getCount();
            transparentProgram->addStep([=, this](xlGraphicsContext* ctx) {
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
    if ((Selected() || (Highlighted() && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted(), IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

void DmxFloodlight::DisplayEffectOnWindow(IModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool shutter_open = shutter_ability->IsShutterOpen(Nodes);
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
        w = preview->getWidth(); h = preview->getHeight();
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

void DmxFloodlight::EnableFixedChannels(xlColorVector& pixelVector) const
{
    if (shutter_ability->GetShutterChannel() != 0 && shutter_ability->GetShutterOnValue() != 0) {
        if ((int)Nodes.size() > shutter_ability->GetShutterChannel() - 1) {
            xlColor c(shutter_ability->GetShutterOnValue(), shutter_ability->GetShutterOnValue(), shutter_ability->GetShutterOnValue());
            pixelVector[shutter_ability->GetShutterChannel() - 1] = c;
        }
    }
    DmxModel::EnableFixedChannels(pixelVector);
}

std::vector<std::string> DmxFloodlight::GenerateNodeNames() const
{
    std::vector<std::string> names = DmxModel::GenerateNodeNames();

    if (0 != shutter_ability->GetShutterChannel() && shutter_ability->GetShutterChannel() < (int)names.size()) {
        names[shutter_ability->GetShutterChannel() - 1] = "Shutter";
    }
    return names;
}


void DmxFloodlight::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    DmxModel::GetPWMOutputs(channels);
    if (shutter_ability->GetShutterChannel() > 0) {
        channels[shutter_ability->GetShutterChannel()] = PWMOutput(shutter_ability->GetShutterChannel(), PWMOutput::Type::LED, 1, "Shutter");
    }
}
