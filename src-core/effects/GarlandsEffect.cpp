/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GarlandsEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "Parallel.h"

#include "ispc/GarlandsFunctions.ispc.h"

#include "../../include/garlands-16.xpm"
#include "../../include/garlands-24.xpm"
#include "../../include/garlands-32.xpm"
#include "../../include/garlands-48.xpm"
#include "../../include/garlands-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Garlands.json values).
int GarlandsEffect::sTypeDefault = 0;
int GarlandsEffect::sTypeMin = 0;
int GarlandsEffect::sTypeMax = 4;
int GarlandsEffect::sSpacingDefault = 10;
int GarlandsEffect::sSpacingMin = 1;
int GarlandsEffect::sSpacingMax = 100;
double GarlandsEffect::sCyclesDefault = 1.0;
double GarlandsEffect::sCyclesMin = 0;
double GarlandsEffect::sCyclesMax = 200;
int GarlandsEffect::sCyclesDivisor = 10;
std::string GarlandsEffect::sDirectionDefault = "Up";

GarlandsEffect::GarlandsEffect(int id) : RenderableEffect(id, "Garlands", garlands_16, garlands_24, garlands_32, garlands_48, garlands_64)
{
    //ctor
}

GarlandsEffect::~GarlandsEffect()
{
    //dtor
}

void GarlandsEffect::OnMetadataLoaded()
{
    sTypeDefault = GetIntDefault("Garlands_Type", sTypeDefault);
    sTypeMin = (int)GetMinFromMetadata("Garlands_Type", sTypeMin);
    sTypeMax = (int)GetMaxFromMetadata("Garlands_Type", sTypeMax);
    sSpacingDefault = GetIntDefault("Garlands_Spacing", sSpacingDefault);
    sSpacingMin = (int)GetMinFromMetadata("Garlands_Spacing", sSpacingMin);
    sSpacingMax = (int)GetMaxFromMetadata("Garlands_Spacing", sSpacingMax);
    sCyclesDefault = GetDoubleDefault("Garlands_Cycles", sCyclesDefault);
    sCyclesMin = GetMinFromMetadata("Garlands_Cycles", sCyclesMin);
    sCyclesMax = GetMaxFromMetadata("Garlands_Cycles", sCyclesMax);
    sCyclesDivisor = GetDivisorFromMetadata("Garlands_Cycles", sCyclesDivisor);
    sDirectionDefault = GetStringDefault("Garlands_Direction", sDirectionDefault);
}

bool GarlandsEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2026.05.2", version);
}

void GarlandsEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    if (IsVersionOlder("2026.05.2", version)) {
        // Garlands_Cycles moved from post-divisor min/max (0..20, no divisor)
        // to the JSON convention (pre-divisor 0..200, divisor 10). Active VCs
        // are rescaled automatically by UpgradeValueCurve at load, but any
        // plain slider stored as E_SLIDER_Garlands_Cycles=X (raw int in old
        // [0..20] space) needs to move to E_TEXTCTRL_Garlands_Cycles=X.X as a
        // post-divisor float — same displayed value, just in the new key.
        SettingsMap& settings = effect->GetSettings();
        std::string cycles = settings.Get("E_SLIDER_Garlands_Cycles", "");
        if (!cycles.empty()) {
            settings.erase("E_SLIDER_Garlands_Cycles");
            settings["E_TEXTCTRL_Garlands_Cycles"] = cycles;
        }
    }
}

int GarlandsEffect::GetDirection(const std::string &direction) {
    if ("Up" == direction) {
        return 0;
    } else if ("Down" == direction) {
        return 1;
    } else if ("Left" == direction) {
        return 2;
    } else if ("Right" == direction) {
        return 3;
    } else if ("Up then Down" == direction) {
        return 4;
    } else if ("Down then Up" == direction) {
        return 5;
    } else if ("Left then Right" == direction) {
        return 6;
    } else if ("Right then Left" == direction) {
        return 7;
    }
    return 0;
}

void GarlandsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int GarlandType = SettingsMap.GetInt("SLIDER_Garlands_Type", sTypeDefault);
    int Spacing = GetValueCurveInt("Garlands_Spacing", sSpacingDefault, SettingsMap, oset, sSpacingMin, sSpacingMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Garlands_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);

    if (Spacing < 1) {
        Spacing = 1;
    }
    int dir = GetDirection(SettingsMap.Get("CHOICE_Garlands_Direction", sDirectionDefault));
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = buffer.BufferHt;
    int garlandWid = buffer.BufferWi;
    if (dir > 1) {
        buffMax = buffer.BufferWi;
        garlandWid = buffer.BufferHt;
    }
    if (buffMax < 1 || garlandWid < 1) {
        return;
    }
    double PixelSpacing = Spacing * buffMax / 100.0;
    if (PixelSpacing < 2.0) PixelSpacing = 2.0;

    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;

    // The scalar renderer scattered one ring at a time; because the per-column
    // ring->pixel map is strictly increasing (injective), the ISPC kernel inverts
    // it per pixel. The per-ring blend colors and truncated y offsets are computed
    // here with the exact scalar double math so the kernel is byte-identical.
    std::vector<ispc::uint8_t4> colors(buffMax);
    std::vector<int32_t> yb(buffMax);
    xlColor color;
    for (int ring = 0; ring < buffMax; ring++) {
        double ratio = double(buffMax - ring - 1) / double(buffMax);
        buffer.GetMultiColorBlend(ratio, false, color);
        colors[ring].v[0] = color.red;
        colors[ring].v[1] = color.green;
        colors[ring].v[2] = color.blue;
        colors[ring].v[3] = color.alpha;
        yb[ring] = (int)(1.0 + ring * PixelSpacing - positionOffset);
    }

    ispc::GarlandsData gdata;
    gdata.width = buffer.BufferWi;
    gdata.height = buffer.BufferHt;
    gdata.buffMax = buffMax;
    gdata.garlandType = GarlandType;
    gdata.dir = dir;
    gdata.invPS = (float)(1.0 / PixelSpacing);
    gdata.posOffOverPS = (float)(positionOffset / PixelSpacing);

    if (buffer.dmx_buffer) {
        // DMX fixtures need the colour routed through SetPixel()
        ispc::uint8_t4 single = {};
        ispc::GarlandsEffectISPC(&gdata, 0, 1, colors.data(), yb.data(), &single);
        buffer.SetPixel(0, 0, xlColor(single.v[0], single.v[1], single.v[2], single.v[3]));
        return;
    }

    // Bound the ISPC writes by the actual pixel allocation, not the logical
    // dimensions: a variable sub-buffer can leave GetPixelCount() < BufferWi*BufferHt
    // and the kernel writes result[index] with no bounds check.
    int max = std::min<int>(buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    constexpr int glBlockSize = 4096;
    int blocks = max / glBlockSize + 1;
    parallel_for(0, blocks, [&gdata, &colors, &yb, &buffer, max](int blk) {
        int start = blk * glBlockSize;
        int end = start + glBlockSize;
        if (end > max) end = max;
        ispc::GarlandsEffectISPC(&gdata, start, end, colors.data(), yb.data(), (ispc::uint8_t4*)buffer.GetPixels());
    });
}
