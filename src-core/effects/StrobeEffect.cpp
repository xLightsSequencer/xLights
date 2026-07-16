/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StrobeEffect.h"

#include <spdlog/fmt/fmt.h>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "media/AudioManager.h"
#include "../models/Model.h"
#include "../../include/strobe.xpm"
#include "UtilFunctions.h"

// Fallback defaults (used until OnMetadataLoaded replaces them with Strobe.json values).
int StrobeEffect::sNumberStrobesDefault = 3;
int StrobeEffect::sStrobeDurationDefault = 10;
int StrobeEffect::sStrobeTypeDefault = 1;
bool StrobeEffect::sStrobeMusicDefault = false;

StrobeEffect::StrobeEffect(int id) : RenderableEffect(id, "Strobe", strobe, strobe, strobe, strobe, strobe)
{
    //ctor
}

StrobeEffect::~StrobeEffect()
{
    //dtor
}

void StrobeEffect::OnMetadataLoaded()
{
    sNumberStrobesDefault = GetIntDefault("Number_Strobes", sNumberStrobesDefault);
    sStrobeDurationDefault = GetIntDefault("Strobe_Duration", sStrobeDurationDefault);
    sStrobeTypeDefault = GetIntDefault("Strobe_Type", sStrobeTypeDefault);
    sStrobeMusicDefault = GetBoolDefault("Strobe_Music", sStrobeMusicDefault);
}

std::list<std::string> StrobeEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);
    if (media == nullptr && settings.GetBool("E_CHECKBOX_Strobe_Music", false)) {
        res.push_back(fmt::format("    WARN: Strobe effect cant follow music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }
    return res;
}

class StrobeClass
{
public:

    int x,y;
    int duration; // How frames strobe light stays on. Will be decremented each frame
    HSVValue hsv;
    xlColor color;

    bool operator==(const StrobeClass& r)
    {
        return x == r.x && y == r.y && hsv.hue == r.hsv.hue && hsv.saturation == r.hsv.saturation && hsv.value == r.hsv.value && color == r.color && duration == r.duration;
    }
    StrobeClass(int _x, int _y, int _duration, HSVValue _hsv, xlColor _color)
    {
        x = _x;
        y = _y;
        duration = _duration;
        hsv = _hsv;
        color = _color;
    }
};

class StrobeRenderCache : public EffectRenderCache {
public:
    StrobeRenderCache() {};
    virtual ~StrobeRenderCache() {};

    std::list<StrobeClass> strobe;
};

// Tier-2 immutable per-frame draw state.  `flip` is the type-2/4 orientation
// coin-flip that the old code rolled *in the draw* - moved into the advance so
// the draw is RNG-free (required for the parallel draw to be byte-identical).
struct StrobeDrawItem {
    int x;
    int y;
    int duration;
    xlColor color;
    HSVValue hsv;
    uint8_t flip;
};
struct StrobeFrameState : public EffectFrameState {
    std::vector<StrobeDrawItem> items;
    int strobeType = 1;
};

static void DrawStrobes(RenderBuffer& buffer, const std::vector<StrobeDrawItem>& items, int Strobe_Type) {
    for (const auto& sit : items) {
        HSVValue hsv = sit.hsv;
        xlColor color(sit.color);
        int x = sit.x;
        int y = sit.y;
        if (sit.duration > 0) {
            buffer.SetPixel(x, y, color);
        }

        double v = 1.0;
        if (sit.duration == 1) {
            v = 0.5;
        } else if (sit.duration == 2) {
            v = 0.75;
        }
        if (buffer.allowAlpha) {
            color.alpha = 255.0 * v;
        } else {
            hsv.value *= v;
            color = hsv;
        }

        if (Strobe_Type == 2) {
            if (sit.flip == 0) {
                buffer.SetPixel(x, y - 1, color);
                buffer.SetPixel(x, y + 1, color);
            } else {
                buffer.SetPixel(x - 1, y, color);
                buffer.SetPixel(x + 1, y, color);
            }
        }
        if (Strobe_Type == 3) {
            buffer.SetPixel(x, y - 1, color);
            buffer.SetPixel(x, y + 1, color);
            buffer.SetPixel(x - 1, y, color);
            buffer.SetPixel(x + 1, y, color);
        }
        if (Strobe_Type == 4) {
            if (sit.flip == 0) {
                buffer.SetPixel(x, y - 1, color);
                buffer.SetPixel(x, y + 1, color);
                buffer.SetPixel(x - 1, y, color);
                buffer.SetPixel(x + 1, y, color);
            } else {
                buffer.SetPixel(x + 1, y - 1, color);
                buffer.SetPixel(x + 1, y + 1, color);
                buffer.SetPixel(x - 1, y - 1, color);
                buffer.SetPixel(x - 1, y + 1, color);
            }
        }
    }
}

void StrobeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (buffer.pendingSnapshot != nullptr) {
        // Frame-parallel draw pass: rasterise the snapshot the serial capture
        // pass advanced and stored; no sim advance here.
        const StrobeFrameState& fs = static_cast<const StrobeFrameState&>(*buffer.pendingSnapshot);
        DrawStrobes(buffer, fs.items, fs.strobeType);
        return;
    }
    int Number_Strobes = SettingsMap.GetInt("SLIDER_Number_Strobes", sNumberStrobesDefault);
    int StrobeDuration = SettingsMap.GetInt("SLIDER_Strobe_Duration", sStrobeDurationDefault);
    int Strobe_Type = SettingsMap.GetInt("SLIDER_Strobe_Type", sStrobeTypeDefault);
    bool reactToMusic = SettingsMap.GetBool("CHECKBOX_Strobe_Music", sStrobeMusicDefault);

    if (reactToMusic) {
        float f = 0.0;
        if (buffer.GetMedia() != nullptr) {
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                f = pf->max;
            }
        }
        Number_Strobes *= f;
    }

    StrobeRenderCache *cache = (StrobeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new StrobeRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::list<StrobeClass> &strobe = cache->strobe;

    if (StrobeDuration == 0) {
        StrobeDuration = 1;
    }

    int ColorIdx;
    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();

    if (buffer.needToInit) {
        buffer.needToInit = false;
        strobe.clear();

        // prepopulate first frame
        for (int i = 0; i < Number_Strobes * StrobeDuration; i++) {
            xlColor color;
            ColorIdx = buffer.randInt(0, (int)colorcnt - 1);
            buffer.palette.GetHSV(ColorIdx, hsv); // take first checked color as color of flash
            buffer.palette.GetColor(ColorIdx, color); // take first checked color as color of flash
            strobe.push_back(StrobeClass(buffer.randInt(0, buffer.BufferWi - 1),
                buffer.randInt(0, buffer.BufferHt - 1), i % StrobeDuration, hsv, color));
        }
    }

    // create new strobe, randomly place a strobe
    while ((int)strobe.size() < Number_Strobes * StrobeDuration) {
        HSVValue hsv;
        xlColor color;
        ColorIdx = buffer.randInt(0, (int)colorcnt - 1);
        buffer.palette.GetHSV(ColorIdx, hsv); // take first checked color as color of flash
        buffer.palette.GetColor(ColorIdx, color); // take first checked color as color of flash
        strobe.push_back(StrobeClass(buffer.randInt(0, buffer.BufferWi - 1),
            buffer.randInt(0, buffer.BufferHt - 1), StrobeDuration, hsv, color));
    }

    // Advance: roll the type-2/4 orientation flip (was in the draw), snapshot the
    // drawable state, decrement, cull.  The flip is the only RNG here and is drawn
    // once per strobe in list order - same order the old interleaved loop rolled
    // it - so advance-then-draw is byte-identical.
    std::vector<StrobeDrawItem> drawItems;
    drawItems.reserve(strobe.size());
    std::list<StrobeClass>::iterator it = strobe.begin();
    while (it != strobe.end()) {
        uint8_t flip = 0;
        if (Strobe_Type == 2 || Strobe_Type == 4) {
            flip = (uint8_t)buffer.randInt(0, 1);
        }
        drawItems.push_back({ it->x, it->y, it->duration, it->color, it->hsv, flip });

        it->duration--;  // decrease the frame counter on this strobe, when it gets to zero we no longer will turn it on

        if (it->duration <= 0) {
            std::list<StrobeClass>::iterator del = it;
            ++it;
            strobe.erase(del);
        } else {
            ++it;
        }
    }

    // draw the snapshot, or (tier-2) hand it to a parallel draw pass
    if (buffer.captureSnapshot != nullptr) {
        auto fs = std::make_unique<StrobeFrameState>();
        fs->items = std::move(drawItems);
        fs->strobeType = Strobe_Type;
        *buffer.captureSnapshot = std::move(fs);
    } else {
        DrawStrobes(buffer, drawItems, Strobe_Type);
    }
}

RenderableEffect::FrameParallelism StrobeEffect::GetFrameParallelism(const SettingsMap& settings) const {
    return FrameParallelism::Snapshottable;
}

