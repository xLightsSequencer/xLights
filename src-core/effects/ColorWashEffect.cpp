/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ColorWashEffect.h"
#include "../render/Effect.h"
#include "../render/EffectLayer.h"
#include "../render/Element.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../../include/ColorWash.xpm"

#include "ispc/ColorWashFunctions.ispc.h"
#include "Parallel.h"

#include <format>

static const std::string CHECKBOX_ColorWash_HFade("CHECKBOX_ColorWash_HFade");
static const std::string CHECKBOX_ColorWash_VFade("CHECKBOX_ColorWash_VFade");
static const std::string CHECKBOX_ColorWash_ReverseFades("CHECKBOX_ColorWash_ReverseFades");
static const std::string TEXTCTRL_ColorWash_Cycles("TEXTCTRL_ColorWash_Cycles");
static const std::string CHECKBOX_ColorWash_Shimmer("CHECKBOX_ColorWash_Shimmer");
static const std::string CHECKBOX_ColorWash_CircularPalette("CHECKBOX_ColorWash_CircularPalette");


// Cycles: JSON min=1, max=200, divisor=10 → renders 0.1..20 cycles.
double ColorWashEffect::sCyclesDefault = 1.0;
double ColorWashEffect::sCyclesMin = 1;
double ColorWashEffect::sCyclesMax = 200;
int ColorWashEffect::sCyclesDivisor = 10;
bool ColorWashEffect::sVFadeDefault = false;
bool ColorWashEffect::sHFadeDefault = false;
bool ColorWashEffect::sReverseFadesDefault = false;
bool ColorWashEffect::sShimmerDefault = false;
bool ColorWashEffect::sCircularPaletteDefault = false;

ColorWashEffect::ColorWashEffect(int i) : RenderableEffect(i, "Color Wash", ColorWash, ColorWash, ColorWash, ColorWash, ColorWash)
{
    //ctor
}

ColorWashEffect::~ColorWashEffect()
{
    //dtor
}

void ColorWashEffect::OnMetadataLoaded()
{
    sCyclesDefault = GetDoubleDefault("ColorWash_Cycles", sCyclesDefault);
    sCyclesMin = GetMinFromMetadata("ColorWash_Cycles", sCyclesMin);
    sCyclesMax = GetMaxFromMetadata("ColorWash_Cycles", sCyclesMax);
    sCyclesDivisor = GetDivisorFromMetadata("ColorWash_Cycles", sCyclesDivisor);
    sVFadeDefault = GetBoolDefault("ColorWash_VFade", sVFadeDefault);
    sHFadeDefault = GetBoolDefault("ColorWash_HFade", sHFadeDefault);
    sReverseFadesDefault = GetBoolDefault("ColorWash_ReverseFades", sReverseFadesDefault);
    sShimmerDefault = GetBoolDefault("ColorWash_Shimmer", sShimmerDefault);
    sCircularPaletteDefault = GetBoolDefault("ColorWash_CircularPalette", sCircularPaletteDefault);
}

int ColorWashEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                          xlVertexColorAccumulator &bg, xlColor* colorMask, bool ramps) {
    if (e->HasBackgroundDisplayList()) {
        e->GetBackgroundDisplayList().addToAccumulator(x1, y1, x2-x1, y2-y1, bg);
        return e->GetBackgroundDisplayList().iconSize;
    }
    if (e->GetSettings().GetBool("E_CHECKBOX_ColorWash_CircularPalette")) {
        xlColorVector map(e->GetPalette());
        map.push_back(map[0]);
        bg.AddHBlendedRectangleAsTriangles(x1, y1, x2, y2, colorMask, 0, map);
    } else {
        bg.AddHBlendedRectangleAsTriangles(x1, y1, x2, y2, colorMask, 0, e->GetPalette());
    }
    return 2;
}

void ColorWashEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    float cycles = GetValueCurveDouble("ColorWash_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);

    bool HorizFade = SettingsMap.GetBool(CHECKBOX_ColorWash_HFade, sHFadeDefault);
    bool VertFade = SettingsMap.GetBool(CHECKBOX_ColorWash_VFade, sVFadeDefault);
    bool reverseFades = SettingsMap.GetBool(CHECKBOX_ColorWash_ReverseFades, sReverseFadesDefault);
    bool shimmer = SettingsMap.GetBool(CHECKBOX_ColorWash_Shimmer, sShimmerDefault);
    bool circularPalette = SettingsMap.GetBool(CHECKBOX_ColorWash_CircularPalette, sCircularPaletteDefault);

    xlColor color, orig;

    const int StartX = 0;
    const int StartY = 0;
    int endX = buffer.BufferWi - 1;
    int endY = buffer.BufferHt - 1;

    int tot = buffer.curPeriod - buffer.curEffStartPer;
    bool shimmerBlack = shimmer && ((tot % 2) != 0);

    ispc::ColorWashData cwdata;
    cwdata.width        = buffer.BufferWi;
    cwdata.height       = buffer.BufferHt;
    cwdata.horizFade    = HorizFade    ? 1 : 0;
    cwdata.vertFade     = VertFade     ? 1 : 0;
    cwdata.reverseFades = reverseFades ? 1 : 0;
    cwdata.allowAlpha   = buffer.allowAlpha ? 1 : 0;
    cwdata.shimmerBlack = shimmerBlack  ? 1 : 0;

    if (shimmerBlack) {
        orig = xlBLACK;
        cwdata.color.v[0] = cwdata.color.v[1] = cwdata.color.v[2] = cwdata.color.v[3] = 0;
        cwdata.colorH = cwdata.colorS = cwdata.colorV = 0.0f;
    } else {
        double position = buffer.GetEffectTimeIntervalPosition(cycles);
        buffer.GetMultiColorBlend(position, circularPalette, color);
        orig = color;
        cwdata.color.v[0] = color.red;
        cwdata.color.v[1] = color.green;
        cwdata.color.v[2] = color.blue;
        cwdata.color.v[3] = color.alpha;
        HSVValue hsvOrig = color.asHSV();
        cwdata.colorH = (float)hsvOrig.hue;
        cwdata.colorS = (float)hsvOrig.saturation;
        cwdata.colorV = (float)hsvOrig.value;
    }

    int max = buffer.BufferWi * buffer.BufferHt;
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&cwdata, &buffer, max](int blk) {
        int start = blk * bfBlockSize;
        int end = start + bfBlockSize;
        if (end > max) end = max;
        ispc::ColorWashEffectISPC(&cwdata, start, end, (ispc::uint8_t4*)buffer.GetPixels());
    });

    if (effect->IsBackgroundDisplayListEnabled() && buffer.perModelIndex == 0) {
        std::unique_lock<std::recursive_mutex> lock(effect->GetBackgroundDisplayList().lock);
        if (VertFade || HorizFade) {
            effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6 * 2);
            int total = buffer.curEffEndPer - buffer.curEffStartPer + 1;
            double x1 = double(buffer.curPeriod - buffer.curEffStartPer) / double(total);
            double x2 = (buffer.curPeriod - buffer.curEffStartPer + 1.0) / double(total);
            int idx = (buffer.curPeriod - buffer.curEffStartPer) * 12;
            buffer.SetDisplayListVRect(effect, idx, x1, 0.0, x2, 0.5,
                                       xlBLACK, orig);
            buffer.SetDisplayListVRect(effect, idx + 6, x1, 0.5, x2, 1.0,
                                       orig, xlBLACK);
        } else {
            effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
            int midX = (StartX + endX) / 2;
            int midY = (StartY + endY) / 2;
            buffer.CopyPixelsToDisplayListX(effect, midY, midX, midX);
        }
    }
}
