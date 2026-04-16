/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define MAX_ISPC_FAN_COLORS 8

#include "FanEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"

#include "ispc/FanFunctions.ispc.h"
#include "Parallel.h"


#include "../../include/fan-16.xpm"
#include "../../include/fan-24.xpm"
#include "../../include/fan-32.xpm"
#include "../../include/fan-48.xpm"
#include "../../include/fan-64.xpm"

int FanEffect::sCenterXDefault = 50;
int FanEffect::sCenterXMin = 0;
int FanEffect::sCenterXMax = 100;
int FanEffect::sCenterYDefault = 50;
int FanEffect::sCenterYMin = 0;
int FanEffect::sCenterYMax = 100;
int FanEffect::sStartRadiusDefault = 1;
int FanEffect::sStartRadiusMin = 0;
int FanEffect::sStartRadiusMax = 2500;
int FanEffect::sStartAngleDefault = 0;
int FanEffect::sStartAngleMin = 0;
int FanEffect::sStartAngleMax = 360;
int FanEffect::sEndRadiusDefault = 10;
int FanEffect::sEndRadiusMin = 0;
int FanEffect::sEndRadiusMax = 2500;
int FanEffect::sRevolutionsDefault = 720; // pre-divisor (= JSON 2.0 * divisor 360)
int FanEffect::sRevolutionsMin = 0;
int FanEffect::sRevolutionsMax = 3600;
int FanEffect::sRevolutionsDivisor = 360;
int FanEffect::sNumBladesDefault = 3;
int FanEffect::sNumBladesMin = 1;
int FanEffect::sNumBladesMax = 16;
int FanEffect::sBladeWidthDefault = 50;
int FanEffect::sBladeWidthMin = 5;
int FanEffect::sBladeWidthMax = 100;
int FanEffect::sBladeAngleDefault = 90;
int FanEffect::sBladeAngleMin = -360;
int FanEffect::sBladeAngleMax = 360;
int FanEffect::sNumElementsDefault = 1;
int FanEffect::sNumElementsMin = 1;
int FanEffect::sNumElementsMax = 4;
int FanEffect::sElementWidthDefault = 100;
int FanEffect::sElementWidthMin = 5;
int FanEffect::sElementWidthMax = 100;
int FanEffect::sDurationDefault = 80;
int FanEffect::sDurationMin = 0;
int FanEffect::sDurationMax = 100;
int FanEffect::sAccelDefault = 0;
int FanEffect::sAccelMin = -10;
int FanEffect::sAccelMax = 10;
bool FanEffect::sReverseDefault = false;
bool FanEffect::sBlendEdgesDefault = true;
bool FanEffect::sScaleDefault = true;

FanEffect::FanEffect(int id) : RenderableEffect(id, "Fan", fan_16, fan_24, fan_32, fan_48, fan_64)
{
    //ctor
}

FanEffect::~FanEffect()
{
    //dtor
}

void FanEffect::OnMetadataLoaded()
{
    sCenterXDefault = GetIntDefault("Fan_CenterX", sCenterXDefault);
    sCenterXMin = (int)GetMinFromMetadata("Fan_CenterX", sCenterXMin);
    sCenterXMax = (int)GetMaxFromMetadata("Fan_CenterX", sCenterXMax);
    sCenterYDefault = GetIntDefault("Fan_CenterY", sCenterYDefault);
    sCenterYMin = (int)GetMinFromMetadata("Fan_CenterY", sCenterYMin);
    sCenterYMax = (int)GetMaxFromMetadata("Fan_CenterY", sCenterYMax);
    sStartRadiusDefault = GetIntDefault("Fan_Start_Radius", sStartRadiusDefault);
    sStartRadiusMin = (int)GetMinFromMetadata("Fan_Start_Radius", sStartRadiusMin);
    sStartRadiusMax = (int)GetMaxFromMetadata("Fan_Start_Radius", sStartRadiusMax);
    sStartAngleDefault = GetIntDefault("Fan_Start_Angle", sStartAngleDefault);
    sStartAngleMin = (int)GetMinFromMetadata("Fan_Start_Angle", sStartAngleMin);
    sStartAngleMax = (int)GetMaxFromMetadata("Fan_Start_Angle", sStartAngleMax);
    sEndRadiusDefault = GetIntDefault("Fan_End_Radius", sEndRadiusDefault);
    sEndRadiusMin = (int)GetMinFromMetadata("Fan_End_Radius", sEndRadiusMin);
    sEndRadiusMax = (int)GetMaxFromMetadata("Fan_End_Radius", sEndRadiusMax);
    // Fan_Revolutions default in JSON is post-divisor (2.0), but Render uses
    // the pre-divisor tick count — multiply it back.
    sRevolutionsDivisor = GetDivisorFromMetadata("Fan_Revolutions", sRevolutionsDivisor);
    sRevolutionsDefault = (int)(GetDoubleDefault("Fan_Revolutions", (double)sRevolutionsDefault / sRevolutionsDivisor) * sRevolutionsDivisor);
    sRevolutionsMin = (int)GetMinFromMetadata("Fan_Revolutions", sRevolutionsMin);
    sRevolutionsMax = (int)GetMaxFromMetadata("Fan_Revolutions", sRevolutionsMax);
    sNumBladesDefault = GetIntDefault("Fan_Num_Blades", sNumBladesDefault);
    sNumBladesMin = (int)GetMinFromMetadata("Fan_Num_Blades", sNumBladesMin);
    sNumBladesMax = (int)GetMaxFromMetadata("Fan_Num_Blades", sNumBladesMax);
    sBladeWidthDefault = GetIntDefault("Fan_Blade_Width", sBladeWidthDefault);
    sBladeWidthMin = (int)GetMinFromMetadata("Fan_Blade_Width", sBladeWidthMin);
    sBladeWidthMax = (int)GetMaxFromMetadata("Fan_Blade_Width", sBladeWidthMax);
    sBladeAngleDefault = GetIntDefault("Fan_Blade_Angle", sBladeAngleDefault);
    sBladeAngleMin = (int)GetMinFromMetadata("Fan_Blade_Angle", sBladeAngleMin);
    sBladeAngleMax = (int)GetMaxFromMetadata("Fan_Blade_Angle", sBladeAngleMax);
    sNumElementsDefault = GetIntDefault("Fan_Num_Elements", sNumElementsDefault);
    sNumElementsMin = (int)GetMinFromMetadata("Fan_Num_Elements", sNumElementsMin);
    sNumElementsMax = (int)GetMaxFromMetadata("Fan_Num_Elements", sNumElementsMax);
    sElementWidthDefault = GetIntDefault("Fan_Element_Width", sElementWidthDefault);
    sElementWidthMin = (int)GetMinFromMetadata("Fan_Element_Width", sElementWidthMin);
    sElementWidthMax = (int)GetMaxFromMetadata("Fan_Element_Width", sElementWidthMax);
    sDurationDefault = GetIntDefault("Fan_Duration", sDurationDefault);
    sDurationMin = (int)GetMinFromMetadata("Fan_Duration", sDurationMin);
    sDurationMax = (int)GetMaxFromMetadata("Fan_Duration", sDurationMax);
    sAccelDefault = GetIntDefault("Fan_Accel", sAccelDefault);
    sAccelMin = (int)GetMinFromMetadata("Fan_Accel", sAccelMin);
    sAccelMax = (int)GetMaxFromMetadata("Fan_Accel", sAccelMax);
    sReverseDefault = GetBoolDefault("Fan_Reverse", sReverseDefault);
    sBlendEdgesDefault = GetBoolDefault("Fan_Blend_Edges", sBlendEdgesDefault);
    sScaleDefault = GetBoolDefault("Fan_Scale", sScaleDefault);
}

int FanEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                    xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps) {
    int head_duration = e->GetSettings().GetInt("E_SLIDER_Fan_Duration", sDurationDefault);
    int num_colors = e->GetPalette().size();
    int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
    int head_length;
    int color_length;
    if( num_colors > 1 ) {
        head_length = (x_mid - x1) / (num_colors-1);
        color_length = (x2 - x_mid) / (num_colors-1);
    } else {
        head_length = (x_mid - x1);
        color_length = (x2 - x_mid);
    }
    for(int i = 0; i < num_colors; i++ ) {
        int cx = x1 + (i*head_length);
        int cx1 = x_mid + (i*color_length);
        if (i == (num_colors-1)) { // fix any roundoff error for last color
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangleAsTriangles(cx, y1+1, x_mid, y2-1, c1, c1);
            backgrounds.AddHBlendedRectangleAsTriangles(cx1, y1+4, x2, y2-4, c1, c1);
        } else {
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            xlColor c2 = e->GetPalette()[i + 1];
            c2.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangleAsTriangles(cx, y1+1, cx+head_length, y2-1, c1, c2);
            backgrounds.AddHBlendedRectangleAsTriangles(cx1, y1+4, cx1+color_length, y2-4, c1, c2);
        }
    }
    return 2;
}

bool FanEffect::needToAdjustSettings(const std::string& version) {
    return IsVersionOlder("2025.04", version);
}

void FanEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap& settings = effect->GetSettings();

    if (IsVersionOlder("2025.04", version)) {
        settings["E_CHECKBOX_Fan_Scale"] = "0";
    }
}

void FanEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Fan_CenterX", sCenterXDefault, SettingsMap, eff_pos, sCenterXMin, sCenterXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Fan_CenterY", sCenterYDefault, SettingsMap, eff_pos, sCenterYMin, sCenterYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Fan_Start_Radius", sStartRadiusDefault, SettingsMap, eff_pos, sStartRadiusMin, sStartRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Fan_End_Radius", sEndRadiusDefault, SettingsMap, eff_pos, sEndRadiusMin, sEndRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle = GetValueCurveInt("Fan_Start_Angle", sStartAngleDefault, SettingsMap, eff_pos, sStartAngleMin, sStartAngleMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions = GetValueCurveInt("Fan_Revolutions", sRevolutionsDefault, SettingsMap, eff_pos, sRevolutionsMin, sRevolutionsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sRevolutionsDivisor);
    int num_blades = GetValueCurveInt("Fan_Num_Blades", sNumBladesDefault, SettingsMap, eff_pos, sNumBladesMin, sNumBladesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_width = GetValueCurveInt("Fan_Blade_Width", sBladeWidthDefault, SettingsMap, eff_pos, sBladeWidthMin, sBladeWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_angle = GetValueCurveInt("Fan_Blade_Angle", sBladeAngleDefault, SettingsMap, eff_pos, sBladeAngleMin, sBladeAngleMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int num_elements = GetValueCurveInt("Fan_Num_Elements", sNumElementsDefault, SettingsMap, eff_pos, sNumElementsMin, sNumElementsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int element_width = GetValueCurveInt("Fan_Element_Width", sElementWidthDefault, SettingsMap, eff_pos, sElementWidthMin, sElementWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration = GetValueCurveInt("Fan_Duration", sDurationDefault, SettingsMap, eff_pos, sDurationMin, sDurationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = GetValueCurveInt("Fan_Accel", sAccelDefault, SettingsMap, eff_pos, sAccelMin, sAccelMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Fan_Reverse", sReverseDefault);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Fan_Blend_Edges", sBlendEdgesDefault);
    bool scale = SettingsMap.GetBool("CHECKBOX_Fan_Scale", sScaleDefault);

    HSVValue hsv, hsv1;
    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color;
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double effect_duration = duration/100.0;    // time the head is in the frame
    double radius_rampup = (1.0 - effect_duration)/2.0;

    double radius1 = start_radius;
    double radius2 = end_radius;
    // Full (pre-ramp) outer radius in pixel units. Used by the twist
    // calculation (r/max_radius)*blade_angle — which defines the full-fan
    // spiral, independent of the ramp-up/ramp-down region. Must stay a
    // double: on small buffers (bufferMax<200) scaling can produce
    // fractional values that would otherwise int-truncate to 0 and trip
    // the blade_angle=0 safety clamp, silently killing the twist.
    double max_radius_d = std::max(radius1, radius2);

    if (scale) { // convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0); // 200 bc radius is half of the width
        radius2 = radius2 * (bufferMax / 200.0);
        max_radius_d = max_radius_d * (bufferMax / 200.0);
    }

    int xc_adj = (center_x-50)*buffer.BufferWi / 100;
    int yc_adj = (center_y-50)*buffer.BufferHt / 100;

    double blade_div_angle = 360.0 / (double)num_blades;
    double blade_width_angle = blade_div_angle * (double)blade_width / 100.0;
    double color_angle = blade_width_angle / (double)num_colors;
    double angle_offset = eff_pos_adj * revs;
    double element_angle = color_angle / (double)num_elements;
    double element_size = element_angle * (double)element_width/ 100.0;

    if( effect_duration < 1.0 )
    {
        double radius_delta = std::abs(radius2 - radius1);
        if( eff_pos_adj < radius_rampup )  // blade growing
        {
            double pct = 1.0 - (eff_pos_adj / radius_rampup);
            if( radius2 > radius1 )
                radius2 = radius2 - radius_delta * pct;
            else
                radius2 = radius2 + radius_delta * pct;
        }
        else if( eff_pos_adj > (1.0 - radius_rampup) )  // blade shrinking
        {
            double pct = (1.0 - eff_pos_adj) / radius_rampup;
            if( radius2 > radius1 )
                radius1 = radius2 - radius_delta * pct;
            else
                radius1 = radius2 + radius_delta * pct;
        }
    }

    if( radius1 > radius2 )
    {
        std::swap(radius1, radius2);
    }

    double max_radius = max_radius_d;
    if (max_radius <= 0.0)
    {
        // A non-positive max radius would be used as a divisor in both the CPU
        // and ISPC twist calculations. Treat this as "no twist" and use a safe
        // positive denominator so both paths remain well-defined.
        blade_angle = 0;
        max_radius = 1.0;
    }

    do {
        // Spatial palette entries need per-pixel position data — fall through to CPU.
        bool hasSpatial = false;
        for (int i = 0; i < num_colors && !hasSpatial; i++)
            hasSpatial = buffer.palette.IsSpatial(i);
        if (hasSpatial) break;

        if (num_colors > MAX_ISPC_FAN_COLORS) break;

        ispc::FanData fdata;
        fdata.width             = buffer.BufferWi;
        fdata.height            = buffer.BufferHt;
        fdata.xc_adj            = xc_adj;
        fdata.yc_adj            = yc_adj;
        fdata.radius1           = (float)radius1;
        fdata.radius2           = (float)radius2;
        fdata.max_radius        = (float)max_radius;
        fdata.blade_div_angle   = (float)blade_div_angle;
        fdata.blade_width_angle = (float)blade_width_angle;
        fdata.color_angle       = (float)color_angle;
        fdata.element_angle     = (float)element_angle;
        fdata.element_size      = (float)element_size;
        fdata.angle_offset      = (float)angle_offset;
        fdata.start_angle       = (float)start_angle;
        fdata.blade_angle       = (float)blade_angle;
        fdata.reverse_dir       = reverse_dir ? 1 : 0;
        fdata.blend_edges       = blend_edges ? 1 : 0;
        fdata.allowAlpha        = buffer.allowAlpha ? 1 : 0;
        fdata.num_colors        = num_colors;

        for (int i = 0; i < num_colors; i++) {
            xlColor c;
            buffer.palette.GetColor(i, c);
            fdata.colorsAsRGBA[i].v[0] = c.red;
            fdata.colorsAsRGBA[i].v[1] = c.green;
            fdata.colorsAsRGBA[i].v[2] = c.blue;
            fdata.colorsAsRGBA[i].v[3] = c.alpha;
            HSVValue hsvC = c.asHSV();
            fdata.colorsH[i] = (float)hsvC.hue;
            fdata.colorsS[i] = (float)hsvC.saturation;
            fdata.colorsV[i] = (float)hsvC.value;
        }

        int ispcMax = buffer.BufferWi * buffer.BufferHt;
        constexpr int bfBlockSize = 4096;
        int blocks = ispcMax / bfBlockSize + 1;
        parallel_for(0, blocks, [&fdata, &buffer, ispcMax](int blk) {
            int start = blk * bfBlockSize;
            int end = start + bfBlockSize;
            if (end > ispcMax) end = ispcMax;
            ispc::FanEffectISPC(&fdata, start, end, (ispc::uint8_t4*)buffer.GetPixels());
        });
        return;
    } while (false);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        int x1 = x - xc_adj - (buffer.BufferWi / 2);
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            int y1 = y - yc_adj - (buffer.BufferHt / 2);
            double r = std::hypot(x1, y1);
            if( r >= radius1 && r <= radius2 ) {
                double degrees_twist = (r / max_radius)*blade_angle;
                double theta = ((std::atan2(x1, y1) * 180.0 / PI)) + degrees_twist + start_angle;
                if (reverse_dir == 1)
                {
                    theta = angle_offset - theta + 180.0;
                } else {
                    theta = theta + 180.0 + angle_offset;
                }
                if( theta < 0.0 ) { theta += 360.0; }
                double current_blade = theta / blade_div_angle;
                double current_blade_angle = theta - (double)((int)current_blade * blade_div_angle);

                if( current_blade_angle <= blade_width_angle ) {

                    double current_element = current_blade_angle / element_angle;
                    double current_element_angle = current_blade_angle - (double)((int)current_element * element_angle);

                    if( current_element_angle <= element_size ) {
                        int color_index = (int)(current_blade_angle / color_angle);
                        buffer.palette.GetColor(color_index, color);

                        double round = (float)current_element_angle / (float)element_size;
                        double color_pct = 1.0 - ((std::abs(current_element_angle - (element_size/2)) * 2) / element_size);

                        if (buffer.palette.IsSpatial(color_index))
                        {
                            buffer.palette.GetSpatialColor(color_index, xc_adj + (buffer.BufferWi / 2), yc_adj + (buffer.BufferHt / 2), x, y, round, max_radius, color);
                            hsv = color.asHSV();
                        }
                        else
                        {
                            hsv = color;
                        }
                        if( blend_edges )
                        {
                            if (buffer.allowAlpha) {
                                color.alpha = 255.0 * color_pct;
                            }
                            else {
                                hsv.value = hsv.value * color_pct;
                                color = hsv;
                            }
                        }
                        buffer.SetPixel(x, y, color);
                    }
                }
            }
        }
    }
}

