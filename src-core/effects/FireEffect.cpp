/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FireEffect.h"
#include "render/ValueCurve.h"

#include <spdlog/fmt/fmt.h>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "media/AudioManager.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "../utils/xlSize.h"
#include "ispc/FireFunctions.ispc.h"

#include "../../include/fire-16.xpm"
#include "../../include/fire-24.xpm"
#include "../../include/fire-32.xpm"
#include "../../include/fire-48.xpm"
#include "../../include/fire-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Fire.json values).
int FireEffect::sHeightDefault = 50;
int FireEffect::sHeightMin = 1;
int FireEffect::sHeightMax = 100;
int FireEffect::sHueShiftDefault = 0;
int FireEffect::sHueShiftMin = 0;
int FireEffect::sHueShiftMax = 100;
double FireEffect::sGrowthCyclesDefault = 0.0;
double FireEffect::sGrowthCyclesMin = 0;
double FireEffect::sGrowthCyclesMax = 200;
int FireEffect::sGrowthCyclesDivisor = 10;
bool FireEffect::sGrowWithMusicDefault = false;
std::string FireEffect::sLocationDefault = "Bottom";
std::string FireEffect::sStyleDefault = "New Render Method";

FireEffect::FireEffect(int id) : RenderableEffect(id, "Fire", fire_16, fire_24, fire_32, fire_48, fire_64)
{
    //ctor
}

FireEffect::~FireEffect()
{
    //dtor
}

void FireEffect::OnMetadataLoaded()
{
    sHeightDefault = GetIntDefault("Fire_Height", sHeightDefault);
    sHeightMin = (int)GetMinFromMetadata("Fire_Height", sHeightMin);
    sHeightMax = (int)GetMaxFromMetadata("Fire_Height", sHeightMax);
    sHueShiftDefault = GetIntDefault("Fire_HueShift", sHueShiftDefault);
    sHueShiftMin = (int)GetMinFromMetadata("Fire_HueShift", sHueShiftMin);
    sHueShiftMax = (int)GetMaxFromMetadata("Fire_HueShift", sHueShiftMax);
    sGrowthCyclesDefault = GetDoubleDefault("Fire_GrowthCycles", sGrowthCyclesDefault);
    sGrowthCyclesMin = GetMinFromMetadata("Fire_GrowthCycles", sGrowthCyclesMin);
    sGrowthCyclesMax = GetMaxFromMetadata("Fire_GrowthCycles", sGrowthCyclesMax);
    sGrowthCyclesDivisor = GetDivisorFromMetadata("Fire_GrowthCycles", sGrowthCyclesDivisor);
    sGrowWithMusicDefault = GetBoolDefault("Fire_GrowWithMusic", sGrowWithMusicDefault);
    sLocationDefault = GetStringDefault("Fire_Location", sLocationDefault);
    sStyleDefault = GetStringDefault("Fire_Style", sStyleDefault);
}

std::list<std::string> FireEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Fire_GrowWithMusic", sGrowWithMusicDefault)) {
        res.push_back(fmt::format("    WARN: Fire effect cant grow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

class FirePaletteClass {
public:
    FirePaletteClass()
    {
        HSVValue hsv;
        // calc 100 reds, black to bright red
        hsv.hue = 0.0;
        hsv.saturation = 1.0;
        for (int i = 0; i < 100; ++i) {
            hsv.value = double(i) / 100.0;
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(xlColor(255, 0, 0, i * 255 / 100));
        }

        // gives 100 hues red to yellow
        hsv.value = 1.0;
        for (int i = 0; i < 100; ++i) {
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(hsv);
            hsv.hue += 0.00166666;
        }
    }
    int size() const {
        return firePalette.size();
    }
    const HSVValue &operator[](int x) const {
        return firePalette[x];
    }
    const xlColor &asColor(int x) const {
        return firePaletteColors[x];
    }
    const xlColor &asAlphaColor(int x) const {
        return firePaletteColorsAlpha[x];
    }

private:
    hsvVector firePalette;
    xlColorVector firePaletteColors;
    xlColorVector firePaletteColorsAlpha;
};
static const FirePaletteClass FirePalette;

// 0 <= x < BufferWi
// 0 <= y < BufferHt
void SetFireBuffer(int x, int y, int PaletteIdx, std::vector<int>& FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi) {
        FireBuffer[y * maxWi + x] = PaletteIdx;
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
int GetFireBuffer(int x, int y, const std::vector<int>& FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi) {
        return FireBuffer[y * maxWi + x];
    }
    return -1;
}

int FireEffect::FirePaletteSize() {
    return FirePalette.size();
}

int FireEffect::GetStyle(const SettingsMap& settings) {
    return settings.Get("CHOICE_Fire_Style", sStyleDefault) == "New Render Method" ? FIRE_STYLE_NEW : FIRE_STYLE_OLD;
}

bool FireEffect::needToAdjustSettings(const std::string& version) {
    return RenderableEffect::needToAdjustSettings(version) || IsVersionOlder("2026.15", version);
}

void FireEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    if (IsVersionOlder("2026.15", version)) {
        // Fire_Style is new. Sequences saved before it existed must keep the
        // classic look, so pin them explicitly rather than relying on the
        // default - which lets the default be flipped later without silently
        // restyling every old sequence.
        SettingsMap& settings = effect->GetSettings();
        if (!settings.Contains("E_CHOICE_Fire_Style")) {
            settings["E_CHOICE_Fire_Style"] = "Old Render Method";
        }
    }
}

static int GetLocation(const std::string &location) {
    if (location == "Bottom") {
        return 0;
    } else if (location == "Top") {
        return 1;
    } else if (location == "Left") {
        return 2;
    } else if (location == "Right") {
        return 3;
    }
    return 0;
}



class FireRenderCache : public EffectRenderCache {
public:
    FireRenderCache() {};
    virtual ~FireRenderCache() {};

    std::vector<int> FireBuffer;
    xlSize maxBuffer;

    // Per-frame palette-index -> colour table (see BuildFireLUT).  Rebuilt only
    // when the hue shift or the alpha mode actually changes, which for a static
    // Hue Shift means once per effect.
    std::vector<xlColor> lut;
    int lutHueShift = -1;
    bool lutAllowAlpha = false;

    // New Render Method only: the temporal advance needs a populated grid to
    // carry forward, so the first frame runs the classic advance instead.
    bool primed = false;
    // Highest row known to be non-zero, so the next advance can skip the dead
    // region above the flame.
    int liveRow = 0;
    // FireBuffer has been written on the CPU since the GPU last ran, so a GPU
    // path must re-upload it before advancing.
    bool cpuDirty = true;
};

static FireRenderCache* GetCache(RenderBuffer &buffer, int id) {
    FireRenderCache *cache = (FireRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FireRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

// One colour per fire-palette index, folding in the hue shift and the alpha
// mode.  The hue shift is constant across a frame, so this 200-entry table
// replaces a per-pixel HSV->RGB conversion in DrawFire - exactly, because both
// xlColor's HSVValue constructor and its assignment operator go through
// fromHSV(), and SetPixel stores an xlColor verbatim when useAlpha is false.
static void BuildFireLUT(int HueShift, bool allowAlpha, std::vector<xlColor>& lut) {
    lut.resize(FirePalette.size());
    for (int i = 0; i < FirePalette.size(); ++i) {
        if (HueShift > 0) {
            HSVValue hsv = FirePalette[i];
            hsv.hue = hsv.hue + (HueShift / 100.0);
            if (hsv.hue > 1.0)
                hsv.hue = 1.0;
            xlColor c(hsv);
            if (allowAlpha) {
                c.alpha = FirePalette.asAlphaColor(i).Alpha();
            }
            lut[i] = c;
        } else {
            lut[i] = allowAlpha ? FirePalette.asAlphaColor(i) : FirePalette.asColor(i);
        }
    }
}

// Classic advance: rebuild the whole grid bottom-up from a fresh random seed row.
// Nothing survives from the previous frame - every row is derived from rows this
// same call already wrote - so the flame flickers in place with no temporal
// coherence.  Strictly serial in y, which is why it has no ISPC/GPU path.
// Returns the first row that is provably all zero (see the comment inside).
static int AdvanceFireClassic(RenderBuffer& buffer, std::vector<int>& fire, int maxMWi, int maxMHt, int step) {
    for (int x = 0; x < maxMWi; ++x) {
        int r = 150 + buffer.randInt(0, 49);
        SetFireBuffer(x, 0, r, fire, maxMWi, maxMHt);
    }
    // Rows [firstDeadRow, maxMHt) are provably all zero once two consecutive rows
    // come out all zero: row y+1's taps are rows y and y-1, so a zero sum gives
    // new_index == 0 and the `new_index > 0` gate below suppresses the noise term.
    // One all-zero row is NOT enough - row y-1 still feeds row y+1 through the
    // (x, y-2) tap.  randInt() is only called inside that gate, so stopping early
    // consumes no randomness and leaves the RNG stream identical.
    int firstDeadRow = maxMHt;
    int zeroRun = 0;
    for (int y = 1; y < maxMHt; ++y) {
        bool allZero = true;
        for (int x = 0; x < maxMWi; ++x) {
            int v1 = GetFireBuffer(x - 1, y - 1, fire, maxMWi, maxMHt);
            int v2 = GetFireBuffer(x, y - 1, fire, maxMWi, maxMHt);
            int v3 = GetFireBuffer(x + 1, y - 1, fire, maxMWi, maxMHt);
            int v4 = GetFireBuffer(x, y - 2, fire, maxMWi, maxMHt);
            int n = 0;
            int sum = 0;
            if (v1 >= 0) {
                sum += v1;
                n++;
            }
            if (v2 >= 0) {
                sum += v2;
                n++;
            }
            if (v3 >= 0) {
                sum += v3;
                n++;
            }
            if (v4 >= 0) {
                sum += v4;
                n++;
            }
            int new_index = n > 0 ? sum / n : 0;
            if (new_index > 0) {
                new_index += (buffer.randInt(0, 99) < 20) ? step : -step;
                if (new_index < 0)
                    new_index = 0;
                if (new_index >= FirePalette.size())
                    new_index = FirePalette.size() - 1;
            }
            SetFireBuffer(x, y, new_index, fire, maxMWi, maxMHt);
            if (new_index != 0) {
                allZero = false;
            }
        }
        zeroRun = allZero ? zeroRun + 1 : 0;
        if (zeroRun == 2) {
            firstDeadRow = y + 1;
            break;
        }
    }
    return firstDeadRow;
}

// Advance/draw split of the fire grid render: Render rebuilds the grid, DrawFire
// rasterises it.  (Kept separate for clarity; frame-parallel does not engage -
// see FireEffect.h - because the advance dominates Fire's cost.)
static void DrawFire(RenderBuffer& buffer, const std::vector<int>& fireBuffer, int maxMWi, int maxMHt, int curWi, int curHt, int loc, const std::vector<xlColor>& lut) {
    for (int y = 0; y < curHt; ++y) {
        for (int x = 0; x < curWi; ++x) {
            int xp = x;
            int yp = y;
            if (loc == 1 || loc == 3) {
                yp = curHt - y - 1;
            }
            if (loc == 2 || loc == 3) {
                std::swap(xp, yp);
            }
            // GetFireBuffer returns -1 outside the grid, which the drawn area can
            // reach when the effect's max buffer is smaller than the current one.
            int idx = GetFireBuffer(x, y, fireBuffer, maxMWi, maxMHt);
            buffer.SetPixel(xp, yp, lut[idx < 0 ? 0 : idx]);
        }
    }
}

// 10 <= HeightPct <= 100
void FireEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float offset = buffer.GetEffectTimeIntervalPosition();
    int HeightPct = GetValueCurveInt("Fire_Height", sHeightDefault, SettingsMap, offset, sHeightMin, sHeightMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int HueShift = GetValueCurveInt("Fire_HueShift", sHueShiftDefault, SettingsMap, offset, sHueShiftMin, sHueShiftMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Fire_GrowthCycles", sGrowthCyclesDefault, SettingsMap, offset, sGrowthCyclesMin, sGrowthCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sGrowthCyclesDivisor);
    bool withMusic = SettingsMap.GetBool("CHECKBOX_Fire_GrowWithMusic", sGrowWithMusicDefault);
    int loc = GetLocation(SettingsMap.Get("CHOICE_Fire_Location", sLocationDefault));
    int style = GetStyle(SettingsMap);

    if (withMusic) {
        HeightPct = 10;
        if (buffer.GetMedia() != nullptr) {
            float f = 0.0;
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                f = pf->max;
            }
            HeightPct += 90 * f;
        }
    } else {
        // cycles is 0 - 200 representing growth cycle count of 0 - 20
        if (cycles > 0) {
            double adjust = buffer.GetEffectTimeIntervalPosition(cycles);
            adjust = 0.5 - std::abs(adjust - 0.5);
            HeightPct += adjust * 100;
        }
    }
    if (HeightPct < 1)
        HeightPct = 1;


    int curWi = buffer.BufferWi;
    int curHt = buffer.BufferHt;
    if (loc == 2 || loc == 3) {
        std::swap(curHt, curWi);
    }
    if (curHt < 1) {
        curHt = 1;
    }

    FireRenderCache* cache = GetCache(buffer, id);

    if (buffer.needToInit) {
        buffer.needToInit = false;

        cache->maxBuffer = buffer.GetMaxBuffer(SettingsMap);
        int w = std::max(buffer.BufferWi, cache->maxBuffer.width);
        int h = std::max(buffer.BufferHt, cache->maxBuffer.height);

        cache->FireBuffer.resize(w * h);
        for (size_t i = 0; i < cache->FireBuffer.size(); ++i) {
            cache->FireBuffer[i] = 0;
        }
        cache->primed = false;
        cache->liveRow = 0;
        cache->cpuDirty = true;
    }
    int maxMWi = cache->maxBuffer.width == -1 ? buffer.BufferWi : cache->maxBuffer.width;
    int maxMHt = cache->maxBuffer.height == -1 ? buffer.BufferHt : cache->maxBuffer.height;
    if (loc == 2 || loc == 3) {
        std::swap(maxMHt, maxMWi);
    }
    if (maxMHt < 1) {
        maxMHt = 1;
    }

    if ((maxMHt * maxMWi) > (int)cache->FireBuffer.size()) {
        // this shouldn't happen, but just in case we'll do this as a safety measure
        cache->FireBuffer.resize(maxMHt * maxMWi);
    }
    
    int step = std::max(1, 255 * 100 / curHt / HeightPct);

    if (cache->lut.empty() || cache->lutHueShift != HueShift || cache->lutAllowAlpha != buffer.allowAlpha) {
        BuildFireLUT(HueShift, buffer.allowAlpha, cache->lut);
        cache->lutHueShift = HueShift;
        cache->lutAllowAlpha = buffer.allowAlpha;
    }

    // build fire
    if (style == FIRE_STYLE_NEW && cache->primed) {
        FireFrameParams params;
        params.grid = &cache->FireBuffer;
        params.lut = cache->lut.data();
        params.maxMWi = maxMWi;
        params.maxMHt = maxMHt;
        params.curWi = curWi;
        params.curHt = curHt;
        params.loc = loc;
        params.step = step;
        params.liveRow = cache->liveRow;
        params.gridChangedOnCpu = cache->cpuDirty;
        params.frameSeed = buffer.hashRandomFrameSeed();
        if (RenderFireGPU(buffer, params)) {
            // The GPU advanced AND drew, and keeps the grid resident.
            cache->liveRow = params.liveRow;
            cache->cpuDirty = false;
            return;
        }
        ispc::FireData fd;
        fd.width = maxMWi;
        fd.height = maxMHt;
        fd.step = step;
        // +3, not +2: row y reads rows y-1 AND y-2, so the row two above the
        // flame's top can be lit by the (x, y-2) tap alone in a single frame.
        fd.maxRow = std::min(maxMHt, cache->liveRow + 3);
        fd.frameSeed = params.frameSeed;
        cache->liveRow = ispc::FireAdvanceTemporalISPC(&fd, cache->FireBuffer.data());
        cache->cpuDirty = true;
    } else {
        // The classic advance also primes the New Render Method's first frame:
        // it reaches the steady-state flame in one pass, where stepping the
        // temporal advance up from an empty grid would take maxMHt frames.
        int firstDeadRow = AdvanceFireClassic(buffer, cache->FireBuffer, maxMWi, maxMHt, step);
        if (firstDeadRow < maxMHt) {
            std::fill(cache->FireBuffer.begin() + (size_t)firstDeadRow * maxMWi,
                      cache->FireBuffer.begin() + (size_t)maxMHt * maxMWi, 0);
        }
        cache->liveRow = std::max(0, firstDeadRow - 1);
        cache->primed = true;
        cache->cpuDirty = true;
    }

    //  Now play fire
    DrawFire(buffer, cache->FireBuffer, maxMWi, maxMHt, curWi, curHt, loc, cache->lut);
}
