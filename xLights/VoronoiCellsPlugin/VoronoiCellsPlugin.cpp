/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Sample effect plugin: builds effect_plugins/VoronoiCellsPlugin.dll, which
// EffectManager::loadEffectPlugins() picks up at xLights startup. Renders a
// drifting, pulsing Voronoi-cell pattern with 6 color modes.
//
// Determinism note: RenderableEffect subclasses are singletons - EffectManager
// creates exactly one VoronoiCellsEffect shared by every placed copy of this
// effect across the whole timeline, rendered via parallel_for across threads
// with frames possibly re-rendered out of order (scrubbing, cache
// invalidation, headless re-render). So seed *positions* are computed as a
// closed-form function of absolute time (buffer.curPeriod * frameTimeInMs),
// not by mutating state each Render() call - the latter would make a frame's
// appearance depend on how many times Render() happened to run before it.
// Only the seeds' fixed initial conditions (generated once via a seeded RNG
// when CellCount changes) are cached, and that cache is thread_local because
// concurrent renders of *differently configured* placed copies of this
// effect (different CellCount) can land on different threads.

#include "effects/RenderableEffect.h"
#include "effects/effectPlugin.h"
#include "render/RenderBuffer.h"
#include "utils/UtilClasses.h"
#include "utils/Color.h"
#include "utils/FloatChecks.h"

#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

namespace {

/* XPM */
static const char *voronoi_16[] = {
"16 16 7 1",
"1 c #1A3AA8",
"2 c #D62828",
"3 c #2A9D3C",
"4 c #E8C400",
"5 c #6A2C91",
"6 c #1FB6C9",
"7 c #0A0A0F",
"5555555776666666",
"5555555776666666",
"5555555776666666",
"5555555776666666",
"7555555776677777",
"7777775777777777",
"4777777777711111",
"4444447771111111",
"4444447771111111",
"4444477377771111",
"4447773777777777",
"4477733772227777",
"4773333772222222",
"7733333772222222",
"7333333772222222",
"3333333772222222"
};

/* XPM */
static const char *voronoi_24[] = {
"24 24 7 1",
"1 c #1A3AA8",
"2 c #D62828",
"3 c #2A9D3C",
"4 c #E8C400",
"5 c #6A2C91",
"6 c #1FB6C9",
"7 c #0A0A0F",
"555555555557766666666666",
"555555555557766666666666",
"555555555557766666666666",
"555555555557766666666666",
"555555555557766666666666",
"555555555557766666666666",
"555555555557766666666666",
"777555555557767777777777",
"777777775557777777777777",
"444777777777771111111111",
"444444447777111111111111",
"444444444477111111111111",
"444444444477711111111111",
"444444444777711111111111",
"444444447733777771111111",
"444444777333777777777111",
"444447773333772227777777",
"444477333337722222222777",
"447773333337722222222222",
"477733333337722222222222",
"773333333337722222222222",
"733333333337722222222222",
"333333333337722222222222",
"333333333377222222222222"
};

/* XPM */
static const char *voronoi_32[] = {
"32 32 7 1",
"1 c #1A3AA8",
"2 c #D62828",
"3 c #2A9D3C",
"4 c #E8C400",
"5 c #6A2C91",
"6 c #1FB6C9",
"7 c #0A0A0F",
"55555555555555557766666666666666",
"55555555555555557766666666666666",
"55555555555555557766666666666666",
"55555555555555557766666666666666",
"55555555555555577666666666666666",
"55555555555555577666666666666666",
"55555555555555577666666666666666",
"55555555555555577666666666666666",
"55555555555555577666666666666666",
"55555555555555577666666666677777",
"77777555555555577777777777777777",
"77777777775555577777777777711111",
"44444777777777777111111111111111",
"44444444447777771111111111111111",
"44444444444444771111111111111111",
"44444444444444771111111111111111",
"44444444444444777111111111111111",
"44444444444447777111111111111111",
"44444444444477337711111111111111",
"44444444447773337777771111111111",
"44444444477733337777777777111111",
"44444444773333337722227777777111",
"44444477733333337722222222777777",
"44444777333333337722222222222777",
"44447733333333377222222222222222",
"44777333333333377222222222222222",
"47773333333333377222222222222222",
"77333333333333377222222222222222",
"73333333333333377222222222222222",
"33333333333333377222222222222222",
"33333333333333772222222222222222",
"33333333333333772222222222222222"
};

/* XPM */
static const char *voronoi_48[] = {
"48 48 7 1",
"1 c #1A3AA8",
"2 c #D62828",
"3 c #2A9D3C",
"4 c #E8C400",
"5 c #6A2C91",
"6 c #1FB6C9",
"7 c #0A0A0F",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666666666",
"555555555555555555555555776666666666666666677777",
"777555555555555555555555776666667777777777777777",
"777777775555555555555555777777777777777777711111",
"444777777777775555555557777777771111111111111111",
"444444447777777777755557711111111111111111111111",
"444444444444447777777777111111111111111111111111",
"444444444444444444477777111111111111111111111111",
"444444444444444444444477111111111111111111111111",
"444444444444444444444477111111111111111111111111",
"444444444444444444444477111111111111111111111111",
"444444444444444444444477111111111111111111111111",
"444444444444444444444777711111111111111111111111",
"444444444444444444447737711111111111111111111111",
"444444444444444444777333771111111111111111111111",
"444444444444444447773333377711111111111111111111",
"444444444444444477333333377777771111111111111111",
"444444444444447773333333377277777771111111111111",
"444444444444477733333333772222227777777111111111",
"444444444444773333333333772222222227777777711111",
"444444444477733333333333772222222222222777777771",
"444444444777333333333333772222222222222222277777",
"444444447733333333333333772222222222222222222227",
"444444777333333333333333772222222222222222222222",
"444447773333333333333337722222222222222222222222",
"444477333333333333333337722222222222222222222222",
"447773333333333333333337722222222222222222222222",
"477733333333333333333337722222222222222222222222",
"773333333333333333333337722222222222222222222222",
"733333333333333333333337722222222222222222222222",
"333333333333333333333337722222222222222222222222",
"333333333333333333333377222222222222222222222222",
"333333333333333333333377222222222222222222222222",
"333333333333333333333377222222222222222222222222",
"333333333333333333333377222222222222222222222222"
};

/* XPM */
static const char *voronoi_64[] = {
"64 64 7 1",
"1 c #1A3AA8",
"2 c #D62828",
"3 c #2A9D3C",
"4 c #E8C400",
"5 c #6A2C91",
"6 c #1FB6C9",
"7 c #0A0A0F",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666666666",
"5555555555555555555555555555555577666666666666666666666666677777",
"7755555555555555555555555555555577666666666666667777777777777777",
"7777777555555555555555555555555577666677777777777777777777711111",
"4477777777775555555555555555555577777777777777771111111111111111",
"4444444777777777755555555555555777777711111111111111111111111111",
"4444444444447777777777555555555771111111111111111111111111111111",
"4444444444444444477777777777557711111111111111111111111111111111",
"4444444444444444444444777777777111111111111111111111111111111111",
"4444444444444444444444444444777111111111111111111111111111111111",
"4444444444444444444444444444477111111111111111111111111111111111",
"4444444444444444444444444444477111111111111111111111111111111111",
"4444444444444444444444444444447711111111111111111111111111111111",
"4444444444444444444444444444447711111111111111111111111111111111",
"4444444444444444444444444444447711111111111111111111111111111111",
"4444444444444444444444444444477711111111111111111111111111111111",
"4444444444444444444444444444777771111111111111111111111111111111",
"4444444444444444444444444447733377111111111111111111111111111111",
"4444444444444444444444444777333377111111111111111111111111111111",
"4444444444444444444444447773333337711111111111111111111111111111",
"4444444444444444444444477333333337777711111111111111111111111111",
"4444444444444444444447773333333337777777771111111111111111111111",
"4444444444444444444477733333333337722277777771111111111111111111",
"4444444444444444444773333333333337722222227777777111111111111111",
"4444444444444444477733333333333337722222222227777777711111111111",
"4444444444444444777333333333333337722222222222222777777771111111",
"4444444444444447733333333333333337722222222222222222277777777111",
"4444444444444777333333333333333377222222222222222222222227777777",
"4444444444447773333333333333333377222222222222222222222222222777",
"4444444444477333333333333333333377222222222222222222222222222222",
"4444444447773333333333333333333377222222222222222222222222222222",
"4444444477733333333333333333333377222222222222222222222222222222",
"4444444773333333333333333333333377222222222222222222222222222222",
"4444477733333333333333333333333772222222222222222222222222222222",
"4444777333333333333333333333333772222222222222222222222222222222",
"4447733333333333333333333333333772222222222222222222222222222222",
"4777333333333333333333333333333772222222222222222222222222222222",
"7773333333333333333333333333333772222222222222222222222222222222",
"7333333333333333333333333333333772222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333337722222222222222222222222222222222",
"3333333333333333333333333333377222222222222222222222222222222222"
};

// One Voronoi cell's fixed initial conditions, generated once via a seeded
// RNG (42) whenever CellCount changes. Position at a given time is derived
// from these (closed-form) rather than by mutating x0/y0 - see file header.
struct CellSeed {
    double x0, y0;     // initial position, [0,1]
    double vx, vy;     // drift velocity per unit "frame", roughly [-0.003, 0.003]
    double hueDeg;     // base hue, [0,360)
    double phase;      // pulse phase offset, [0, 2*pi)
    double pulseSpeed; // pulse rate multiplier, [0.5, 2.0]
};

// Reflects v (which may have drifted arbitrarily far from [0,1]) back into
// [0,1] as if it had bounced elastically off both bounds - a closed-form
// equivalent of iteratively clamping+reversing velocity every step.
double ReflectUnit(double v) {
    double m = std::fmod(v, 2.0);
    if (m < 0.0) m += 2.0;
    return (m <= 1.0) ? m : (2.0 - m);
}

constexpr double kTwoPi = 6.283185307179586476925286766559;

std::vector<CellSeed> GenerateSeeds(int cellCount) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> pos01(0.0, 1.0);
    std::uniform_real_distribution<double> vel(-0.003, 0.003);
    std::uniform_real_distribution<double> phase01(0.0, kTwoPi);
    std::uniform_real_distribution<double> pulseSpeed01(0.5, 2.0);

    std::vector<CellSeed> seeds;
    seeds.reserve(cellCount);
    for (int i = 0; i < cellCount; i++) {
        CellSeed s;
        s.x0 = pos01(rng);
        s.y0 = pos01(rng);
        s.vx = vel(rng);
        s.vy = vel(rng);
        s.hueDeg = (double)i / (double)cellCount * 360.0;
        s.phase = phase01(rng);
        s.pulseSpeed = pulseSpeed01(rng);
        seeds.push_back(s);
    }
    return seeds;
}

// thread_local: RenderableEffect subclasses are singletons shared by every
// placed copy of the effect, rendered via parallel_for across threads.
// Concurrent renders of differently-configured copies (different CellCount)
// can land on different threads, so a shared (non-thread_local) cache would
// race between threads rebuilding it for different counts.
struct SeedCache {
    int cellCount = -1;
    std::vector<CellSeed> seeds;
};

const std::vector<CellSeed>& GetSeeds(int cellCount) {
    thread_local SeedCache cache;
    if (cache.cellCount != cellCount) {
        cache.seeds = GenerateSeeds(cellCount);
        cache.cellCount = cellCount;
    }
    return cache.seeds;
}

double Clamp01(double v) {
    return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v);
}

// Smooth 0->1 transition, used to anti-alias the cell/edge blend instead of
// a hard binary cutoff.
double Smoothstep(double edge0, double edge1, double x) {
    if (edge1 <= edge0) return x < edge0 ? 0.0 : 1.0;
    double t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

// Per-color-mode fill/edge treatment. Modes differ in more than just hue -
// Neon Grid wants dark cells with bright glowing borders (the opposite of
// Stained Glass's dark borders on bright cells), so each mode returns its
// own (fill, edge, edgeBlendWidthMultiplier) rather than one shared
// "edge = dark" rule.
struct ModeColors {
    xlColor fill;
    xlColor edge;
    double edgeBlendWidthMultiplier; // >1 = softer/wider transition (Lava Lamp)
};

xlColor FromHSV(double hueDeg, double sat, double val) {
    HSVValue hsv(hueDeg / 360.0, Clamp01(sat), Clamp01(val));
    return xlColor(hsv);
}

ModeColors ComputeModeColors(int colorMode, const CellSeed& seed, double pulseVal, double timeSeconds, RenderBuffer& buffer) {
    ModeColors mc;
    switch (colorMode) {
    case 5: { // Color Palette - cells blend continuously across the effect's
              // selected palette (same GetMultiColorBlend used by ColorWash
              // etc.) instead of a fixed hue scheme; edges are a darker
              // shade of the same blended color so any palette looks sane.
        xlColor blended;
        buffer.GetMultiColorBlend(seed.hueDeg / 360.0, true, blended);
        mc.fill = blended.ApplyBrightness((float)(0.35 + 0.5 * pulseVal));
        mc.edge = blended.ApplyBrightness(0.08f);
        mc.edgeBlendWidthMultiplier = 1.0;
        break;
    }
    case 1: { // Cellular / Organic - warm ambers, glowing orange/white edges
        double hue = 25.0 + 20.0 * (seed.hueDeg / 360.0); // narrow warm amber band
        mc.fill = FromHSV(hue, 0.75, 0.35 + 0.45 * pulseVal);
        mc.edge = FromHSV(35.0, 0.35, 1.0); // glowing near-white/orange
        mc.edgeBlendWidthMultiplier = 1.4;
        break;
    }
    case 2: { // Neon Grid - dark cells, bright glowing colored borders
        mc.fill = FromHSV(seed.hueDeg, 0.6, 0.06 + 0.05 * pulseVal);
        mc.edge = FromHSV(seed.hueDeg, 0.9, 1.0);
        mc.edgeBlendWidthMultiplier = 1.0;
        break;
    }
    case 3: { // Ice Crystals - cool blues/whites, bright crystalline edges
        double hue = 190.0 + 30.0 * (seed.hueDeg / 360.0);
        mc.fill = FromHSV(hue, 0.45, 0.45 + 0.4 * pulseVal);
        mc.edge = FromHSV(hue, 0.15, 1.0);
        mc.edgeBlendWidthMultiplier = 1.2;
        break;
    }
    case 4: { // Lava Lamp - slowly morphing warm/cool hues, soft edges
        double hue = std::fmod(seed.hueDeg + timeSeconds * 8.0 * (seed.pulseSpeed * 0.5 + 0.25), 360.0);
        mc.fill = FromHSV(hue, 0.85, 0.4 + 0.4 * pulseVal);
        mc.edge = FromHSV(std::fmod(hue + 20.0, 360.0), 0.6, 0.6 + 0.3 * pulseVal);
        mc.edgeBlendWidthMultiplier = 2.2; // soft edges - wide blend, no hard line
        break;
    }
    default: { // 0: Stained Glass - rich saturated hue per cell, dark edges
        mc.fill = FromHSV(seed.hueDeg, 0.9, 0.35 + 0.5 * pulseVal);
        mc.edge = FromHSV(seed.hueDeg, 0.9, 0.04);
        mc.edgeBlendWidthMultiplier = 1.0;
        break;
    }
    }
    return mc;
}

} // namespace

class VoronoiCellsEffect : public RenderableEffect
{
public:
    explicit VoronoiCellsEffect(int id)
        : RenderableEffect(id, "Voronoi Cells", voronoi_16, voronoi_24, voronoi_32, voronoi_48, voronoi_64)
    {
    }

    void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override
    {
        // Runtime SettingsMap keys drop the "E_" prefix the JSON-driven UI
        // uses for widget IDs (see BarsEffect.cpp reading "CHOICE_Bars_Direction",
        // not "E_CHOICE_Bars_Direction") - keys here must match
        // resources/effectmetadata/VoronoiCells.json's controlType-derived prefixes.
        int cellCount = settings.GetInt("SLIDER_Voronoi_CellCount", 12);
        cellCount = std::min(30, std::max(3, cellCount));

        float speed = settings.GetFloat("TEXTCTRL_Voronoi_Speed", 0.30f);
        speed = (float)Clamp01(speed);
        float pulse = settings.GetFloat("TEXTCTRL_Voronoi_Pulse", 0.50f);
        pulse = (float)Clamp01(pulse);
        float edgeWidthSetting = settings.GetFloat("TEXTCTRL_Voronoi_EdgeWidth", 0.25f);
        edgeWidthSetting = (float)Clamp01(edgeWidthSetting);

        static const char* kColorModeNames[6] = {
            "Stained Glass", "Cellular / Organic", "Neon Grid", "Ice Crystals", "Lava Lamp", "Color Palette"
        };
        std::string colorModeName = settings.Get("CHOICE_Voronoi_ColorMode", "Stained Glass");
        int colorMode = 0;
        for (int i = 0; i < 6; i++) {
            if (colorModeName == kColorModeNames[i]) { colorMode = i; break; }
        }

        bool showEdges = settings.GetBool("CHECKBOX_Voronoi_ShowEdges", true);

        const std::vector<CellSeed>& seeds = GetSeeds(cellCount);

        // Absolute (order-independent) time basis - see file header comment.
        double absoluteFrames = (double)buffer.curPeriod;
        double driftFrames = absoluteFrames * speed;
        double timeSeconds = absoluteFrames * (double)buffer.frameTimeInMs / 1000.0;

        // Reflected seed positions for THIS frame - computed once per Render()
        // call, not per pixel.
        std::vector<std::pair<double, double>> pos(seeds.size());
        for (size_t i = 0; i < seeds.size(); i++) {
            pos[i] = { ReflectUnit(seeds[i].x0 + seeds[i].vx * driftFrames),
                       ReflectUnit(seeds[i].y0 + seeds[i].vy * driftFrames) };
        }

        double edgeThreshold = std::max(0.001, edgeWidthSetting * 0.08);

        for (int y = 0; y < buffer.BufferHt; y++) {
            double ny = ((double)y + 0.5) / (double)buffer.BufferHt;
            for (int x = 0; x < buffer.BufferWi; x++) {
                double nx = ((double)x + 0.5) / (double)buffer.BufferWi;

                int nearest = 0;
                double d1 = -1.0, d2 = -1.0;
                for (size_t i = 0; i < pos.size(); i++) {
                    double dx = nx - pos[i].first;
                    double dy = ny - pos[i].second;
                    double d = dx * dx + dy * dy;
                    if (d1 < 0.0 || d < d1) {
                        d2 = d1;
                        d1 = d;
                        nearest = (int)i;
                    } else if (d2 < 0.0 || d < d2) {
                        d2 = d;
                    }
                }
                if (d2 < 0.0) d2 = d1;

                double edge = std::sqrt(d2) - std::sqrt(d1);
                if (!xl::isfinite(edge)) edge = edgeThreshold; // degenerate: treat as "well inside cell"

                const CellSeed& s = seeds[nearest];
                double pulseVal = 0.5 + 0.5 * std::sin(s.phase + timeSeconds * s.pulseSpeed * (0.3 + pulse * 1.7));
                ModeColors mc = ComputeModeColors(colorMode, s, pulseVal, timeSeconds, buffer);

                xlColor color;
                if (showEdges) {
                    double blendEnd = edgeThreshold * mc.edgeBlendWidthMultiplier;
                    double t = Smoothstep(0.0, blendEnd, edge); // 0 at/near boundary, 1 well inside the cell
                    color.red = (uint8_t)(mc.edge.red + (mc.fill.red - mc.edge.red) * t);
                    color.green = (uint8_t)(mc.edge.green + (mc.fill.green - mc.edge.green) * t);
                    color.blue = (uint8_t)(mc.edge.blue + (mc.fill.blue - mc.edge.blue) * t);
                    color.alpha = 255;
                } else {
                    color = mc.fill;
                }

                buffer.SetPixel(x, y, color);
            }
        }
    }
};

XL_EFFECT_PLUGIN_EXPORT(VoronoiCellsEffect)
