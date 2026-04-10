/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShockwaveEffect.h"
#include <cmath>

#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"
#include "../render/Effect.h"

#include "../../include/shockwave-16.xpm"
#include "../../include/shockwave-24.xpm"
#include "../../include/shockwave-32.xpm"
#include "../../include/shockwave-48.xpm"
#include "../../include/shockwave-64.xpm"

#include "ispc/ShockwaveFunctions.ispc.h"
#include "Parallel.h"

// Fallback defaults (replaced from Shockwave.json in OnMetadataLoaded).
int ShockwaveEffect::sCenterXDefault = 50;
int ShockwaveEffect::sCenterXMin = 0;
int ShockwaveEffect::sCenterXMax = 100;
int ShockwaveEffect::sCenterYDefault = 50;
int ShockwaveEffect::sCenterYMin = 0;
int ShockwaveEffect::sCenterYMax = 100;
int ShockwaveEffect::sStartRadiusDefault = 1;
int ShockwaveEffect::sStartRadiusMin = 0;
int ShockwaveEffect::sStartRadiusMax = 750;
int ShockwaveEffect::sEndRadiusDefault = 10;
int ShockwaveEffect::sEndRadiusMin = 0;
int ShockwaveEffect::sEndRadiusMax = 750;
int ShockwaveEffect::sStartWidthDefault = 5;
int ShockwaveEffect::sStartWidthMin = 0;
int ShockwaveEffect::sStartWidthMax = 255;
int ShockwaveEffect::sEndWidthDefault = 10;
int ShockwaveEffect::sEndWidthMin = 0;
int ShockwaveEffect::sEndWidthMax = 255;
int ShockwaveEffect::sAccelDefault = 0;
int ShockwaveEffect::sCyclesDefault = 1;
bool ShockwaveEffect::sScaleDefault = true;
bool ShockwaveEffect::sBlendEdgesDefault = true;

ShockwaveEffect::ShockwaveEffect(int id) :
    RenderableEffect(id, "Shockwave", shockwave_16, shockwave_24, shockwave_32, shockwave_48, shockwave_64)
{
    // ctor
}

ShockwaveEffect::~ShockwaveEffect()
{
    // dtor
}

void ShockwaveEffect::OnMetadataLoaded()
{
    sCenterXDefault = GetIntDefault("Shockwave_CenterX", sCenterXDefault);
    sCenterXMin = (int)GetMinFromMetadata("Shockwave_CenterX", sCenterXMin);
    sCenterXMax = (int)GetMaxFromMetadata("Shockwave_CenterX", sCenterXMax);
    sCenterYDefault = GetIntDefault("Shockwave_CenterY", sCenterYDefault);
    sCenterYMin = (int)GetMinFromMetadata("Shockwave_CenterY", sCenterYMin);
    sCenterYMax = (int)GetMaxFromMetadata("Shockwave_CenterY", sCenterYMax);
    sStartRadiusDefault = GetIntDefault("Shockwave_Start_Radius", sStartRadiusDefault);
    sStartRadiusMin = (int)GetMinFromMetadata("Shockwave_Start_Radius", sStartRadiusMin);
    sStartRadiusMax = (int)GetMaxFromMetadata("Shockwave_Start_Radius", sStartRadiusMax);
    sEndRadiusDefault = GetIntDefault("Shockwave_End_Radius", sEndRadiusDefault);
    sEndRadiusMin = (int)GetMinFromMetadata("Shockwave_End_Radius", sEndRadiusMin);
    sEndRadiusMax = (int)GetMaxFromMetadata("Shockwave_End_Radius", sEndRadiusMax);
    sStartWidthDefault = GetIntDefault("Shockwave_Start_Width", sStartWidthDefault);
    sStartWidthMin = (int)GetMinFromMetadata("Shockwave_Start_Width", sStartWidthMin);
    sStartWidthMax = (int)GetMaxFromMetadata("Shockwave_Start_Width", sStartWidthMax);
    sEndWidthDefault = GetIntDefault("Shockwave_End_Width", sEndWidthDefault);
    sEndWidthMin = (int)GetMinFromMetadata("Shockwave_End_Width", sEndWidthMin);
    sEndWidthMax = (int)GetMaxFromMetadata("Shockwave_End_Width", sEndWidthMax);
    sAccelDefault = GetIntDefault("Shockwave_Accel", sAccelDefault);
    sCyclesDefault = GetIntDefault("Shockwave_Cycles", sCyclesDefault);
    sScaleDefault = GetBoolDefault("Shockwave_Scale", sScaleDefault);
    sBlendEdgesDefault = GetBoolDefault("Shockwave_Blend_Edges", sBlendEdgesDefault);
}

int ShockwaveEffect::DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                          xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps)
{
    int cycles = e->GetSettings().GetInt("E_SLIDER_Shockwave_Cycles", sCyclesDefault);
    int totalsize = x2 - x1;
    double x_size = totalsize / (double)cycles;
    x_size = std::max(x_size, 0.01);
    for (int i = 0; i < cycles; ++i) {
        backgrounds.AddHBlendedRectangleAsTriangles(x1 + (i * x_size), y1, x1 + (i * x_size) + x_size, y2, colorMask, 0, e->GetPalette());
    }

    return 2;
}

bool ShockwaveEffect::needToAdjustSettings(const std::string& version) {
    return IsVersionOlder("2025.04", version);
}

void ShockwaveEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap& settings = effect->GetSettings();

    if (IsVersionOlder("2025.04", version)) {
        settings["E_CHECKBOX_Shockwave_Scale"] = "0";
    }
}

#define ToRadians(x) ((double)x * PI / (double)180.0)

void ShockwaveEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    int cycles = SettingsMap.GetInt("SLIDER_Shockwave_Cycles", sCyclesDefault);
    double eff_pos = buffer.GetEffectTimeIntervalPosition(cycles);
    int center_x = GetValueCurveInt("Shockwave_CenterX", sCenterXDefault, SettingsMap, eff_pos, sCenterXMin, sCenterXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Shockwave_CenterY", sCenterYDefault, SettingsMap, eff_pos, sCenterYMin, sCenterYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Shockwave_Start_Radius", sStartRadiusDefault, SettingsMap, eff_pos, sStartRadiusMin, sStartRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Shockwave_End_Radius", sEndRadiusDefault, SettingsMap, eff_pos, sEndRadiusMin, sEndRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_width = GetValueCurveInt("Shockwave_Start_Width", sStartWidthDefault, SettingsMap, eff_pos, sStartWidthMin, sStartWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Shockwave_End_Width", sEndWidthDefault, SettingsMap, eff_pos, sEndWidthMin, sEndWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = SettingsMap.GetInt("SLIDER_Shockwave_Accel", sAccelDefault);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Shockwave_Blend_Edges", sBlendEdgesDefault);
    bool scale = SettingsMap.GetBool("CHECKBOX_Shockwave_Scale", sScaleDefault);

    int num_colors = buffer.palette.Size();
    if (num_colors == 0)
        num_colors = 1;

    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    HSVValue hsv, hsv1;
    xlColor color;
    double blend_pct = 1.0;
    if (num_colors > 1) {
        blend_pct = 1.0 / (num_colors - 1);
    }
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(std::min(color_index, num_colors - 1), std::min(color_index + 1, num_colors - 1), std::min(blend_pct, 1.0), color);

    int xc_adj = center_x * buffer.BufferWi / 100;
    int yc_adj = center_y * buffer.BufferHt / 100;

    double radius1 = start_radius;
    double radius2 = end_radius;
    if (scale) { // convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0); // 200 bc radius is half of the width
        radius2 = radius2 * (bufferMax / 200.0);
        start_width = start_width * (bufferMax / 100.0);
        end_width = end_width * (bufferMax / 100.0);
    }
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    if (half_width < 0.25) {
        half_width = 0.25;
    }
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;
    radius1 = std::max(0.0, radius1);

    if (!buffer.palette.IsSpatial(color_index)) {
        HSVValue colorHsv = color.asHSV();
        ispc::ShockwaveData sdata;
        sdata.width         = buffer.BufferWi;
        sdata.height        = buffer.BufferHt;
        sdata.xc_adj        = (float)xc_adj;
        sdata.yc_adj        = (float)yc_adj;
        sdata.radius1       = (float)radius1;
        sdata.radius2       = (float)radius2;
        sdata.radius_center = (float)radius_center;
        sdata.half_width    = (float)half_width;
        sdata.blend_edges   = blend_edges ? 1 : 0;
        sdata.allowAlpha    = buffer.allowAlpha ? 1 : 0;
        sdata.color.v[0]    = color.red;
        sdata.color.v[1]    = color.green;
        sdata.color.v[2]    = color.blue;
        sdata.color.v[3]    = color.alpha;
        sdata.colorH        = (float)colorHsv.hue;
        sdata.colorS        = (float)colorHsv.saturation;
        sdata.colorV        = (float)colorHsv.value;

        int max = buffer.BufferWi * buffer.BufferHt;
        constexpr int bfBlockSize = 4096;
        int blocks = max / bfBlockSize + 1;
        parallel_for(0, blocks, [&sdata, &buffer, max](int blk) {
            int start = blk * bfBlockSize;
            int end = start + bfBlockSize;
            if (end > max) end = max;
            ispc::ShockwaveEffectISPC(&sdata, start, end, (ispc::uint8_t4*)buffer.GetPixels());
        });
        return;
    }

    for (int x = 0; x < buffer.BufferWi; x++) {
        int x1 = x - xc_adj;
        for (int y = 0; y < buffer.BufferHt; y++) {
            int y1 = y - yc_adj;
            double r = std::hypot(x1, y1);
            if (r >= radius1 && r <= radius2) {
                if (buffer.palette.IsSpatial(color_index)) {
                    double theta = (((std::atan2(x1, y1) * 180.0 / PI)) + 180.0) / 360.0;
                    buffer.palette.GetSpatialColor(color_index, radius1, 0, r, 0, theta, radius2, color);
                    hsv = color.asHSV();
                } else {
                    hsv = color;
                }
                if (blend_edges) {
                    double color_pct = 1.0 - std::abs(r - radius_center) / half_width;
                    xlColor ncolor(color);
                    if (buffer.allowAlpha) {
                        ncolor.alpha = 255.0 * color_pct;
                    } else {
                        hsv.value = hsv.value * color_pct;
                        ncolor = hsv;
                    }
                    buffer.SetPixel(x, y, ncolor);
                } else {
                    buffer.SetPixel(x, y, color);
                }
            }
        }
    }
}
