/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TwinkleEffect.h"

#include "ispc/TwinkleFunctions.ispc.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/twinkle-16.xpm"
#include "../../include/twinkle-24.xpm"
#include "../../include/twinkle-32.xpm"
#include "../../include/twinkle-48.xpm"
#include "../../include/twinkle-64.xpm"
#include "render/ValueCurve.h"

#include "Parallel.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <cmath>

// Fallback defaults (used until OnMetadataLoaded replaces them with Twinkle.json values).
int TwinkleEffect::sCountDefault = 3;
int TwinkleEffect::sCountMin = 2;
int TwinkleEffect::sCountMax = 100;
int TwinkleEffect::sStepsDefault = 30;
int TwinkleEffect::sStepsVCMin = 2;
int TwinkleEffect::sStepsVCMax = 100;
bool TwinkleEffect::sStrobeDefault = false;
bool TwinkleEffect::sReRandomDefault = false;
std::string TwinkleEffect::sStyleDefault = "Old Render Method";

TwinkleEffect::TwinkleEffect(int id) : RenderableEffect(id, "Twinkle", twinkle_16, twinkle_24, twinkle_32, twinkle_48, twinkle_64)
{
    //ctor
}

TwinkleEffect::~TwinkleEffect()
{
    //dtor
}

void TwinkleEffect::OnMetadataLoaded()
{
    sCountDefault = GetIntDefault("Twinkle_Count", sCountDefault);
    sCountMin = (int)GetMinFromMetadata("Twinkle_Count", sCountMin);
    sCountMax = (int)GetMaxFromMetadata("Twinkle_Count", sCountMax);
    sStepsDefault = GetIntDefault("Twinkle_Steps", sStepsDefault);
    sStepsVCMin = (int)GetVCMinFromMetadata("Twinkle_Steps", sStepsVCMin);
    sStepsVCMax = (int)GetVCMaxFromMetadata("Twinkle_Steps", sStepsVCMax);
    sStrobeDefault = GetBoolDefault("Twinkle_Strobe", sStrobeDefault);
    sReRandomDefault = GetBoolDefault("Twinkle_ReRandom", sReRandomDefault);
    sStyleDefault = GetStringDefault("Twinkle_Style", sStyleDefault);
}
class TwinkleRenderCache : public EffectRenderCache {
public:
    TwinkleRenderCache() {};
    virtual ~TwinkleRenderCache() {};
    
    std::vector<StrobeClass> strobe;
    int num_lights = 0;
    int curNumStrobe = 0;
    std::atomic_int lights_to_renew = 0;
};

// Tier-2 immutable per-frame draw state: the entire TwinkleRenderCache plus the
// buffer's needToInit flag, captured by AdvanceState BEFORE any of this frame's
// mutation (the renewal bookkeeping, init, placement, and the kernel's in-place
// advance). The draw pass restores these and re-runs the normal render, which
// advances from this exact pre-frame state and draws - reproducing the advanced
// frame byte-for-byte (serial and frame-parallel alike).
struct TwinkleFrameState : public EffectFrameState {
    std::vector<StrobeClass> strobe;
    int num_lights = 0;
    int curNumStrobe = 0;
    int lights_to_renew = 0;
    bool needToInit = false;
};

RenderableEffect::FrameParallelism TwinkleEffect::GetFrameParallelism(const SettingsMap& settings) const {
    // All cross-frame state lives in TwinkleRenderCache (per-light StrobeClass
    // array + renewal counters) plus buffer.needToInit, which TwinkleFrameState
    // snapshots in full, and every random draw (init shuffle, placement,
    // reRandomize) is a stateless hash keyed on the frame - so a draw pass that
    // restores the pre-frame state and re-runs the normal render reproduces the
    // serial frame exactly. This holds for both render methods and the by-node
    // scalar tail alike.
    return FrameParallelism::Snapshottable;
}

bool TwinkleEffect::needToAdjustSettings(const std::string& version) {
    // give the base class a chance to adjust any settings
    return RenderableEffect::needToAdjustSettings(version) || IsVersionOlder("2020.57", version) || IsVersionOlder("2026.06", version);
}

void TwinkleEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    if (IsVersionOlder("2020.57", version)) {
        SettingsMap& settings = effect->GetSettings();
        settings["E_CHOICE_Twinkle_Style"] = "Old Render Method";
    }
    if (IsVersionOlder("2026.06", version)) {
        // TWINKLE_STEPS_MAX was raised from 100 to 400. UpgradeValueCurve is
        // disabled for this setting (GetSettingVCDivisor returns 0xFFFF) to
        // prevent it from rescaling old values into the new range. Instead,
        // migrate here: preserve the stored values but widen the Max to 400.
        SettingsMap& settings = effect->GetSettings();
        std::string vc = settings.Get("E_VALUECURVE_Twinkle_Steps", "");
        if (vc.find("VALUECURVE") != std::string::npos && vc.find("Max=100.00") != std::string::npos) {
            ValueCurve v;
            v.Deserialise(vc, true); // holdminmax=true: keep values, don't rescale
            v.SetLimits(sStepsVCMin, sStepsVCMax); // extend to new limits (2..400)
            settings["E_VALUECURVE_Twinkle_Steps"] = v.Serialise();
        }
        // Twinkle_Style default changed from "Old Render Method" to "New Render
        // Method". Preserve the prior rendering for sequences saved before this
        // change — if the setting wasn't explicitly stored, pin it to the old
        // default so behavior doesn't silently change on load.
        if (!settings.Contains("E_CHOICE_Twinkle_Style")) {
            settings["E_CHOICE_Twinkle_Style"] = "Old Render Method";
        }
    }
}

static inline void addLineAsTriangles(xlVertexColorAccumulator &bg,
                                      float x1, float y1,
                                      float x2, float y2,
                                      const xlColor &c,
                                      bool h = false) {

    float diffy = std::abs(y2 - y1);
    float diffx = std::abs(x2 - x1);

    if (h || (diffy < diffx)) {
        bg.AddVertex(x1, y1, c);
        bg.AddVertex(x1, y1 + 0.5f, c);
        bg.AddVertex(x2, y2, c);

        bg.AddVertex(x1, y1 + 0.5f, c);
        bg.AddVertex(x2, y2, c);
        bg.AddVertex(x2, y2 + 0.5f, c);
    } else {
        bg.AddVertex(x1, y1, c);
        bg.AddVertex(x1 + 0.5f, y1, c);
        bg.AddVertex(x2, y2, c);

        bg.AddVertex(x1 + 0.5f, y1, c);
        bg.AddVertex(x2, y2, c);
        bg.AddVertex(x2 + 0.5f, y2, c);
    }
}

int TwinkleEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                        xlVertexColorAccumulator &bg, xlColor* colorMask, bool ramp)
{
    if (ramp) {
        float endi;
        float starti;
        std::string vcs = e->GetPaletteMap().Get("C_VALUECURVE_Brightness", "");
        if (vcs == "") {
            starti = e->GetPaletteMap().GetInt("C_SLIDER_Brightness", 100);
            if (starti > 100) starti = 100;
            endi = starti;
        } else {
            ValueCurve vc(vcs);
            starti = vc.GetOutputValueAt(0.0, e->GetStartTimeMS(), e->GetEndTimeMS());
            if (starti > 100) starti = 100;
            endi = vc.GetOutputValueAt(1.0, e->GetStartTimeMS(), e->GetEndTimeMS());
            if (endi > 100) endi = 100;
        }

        xlColor color = e->GetPalette()[0];
        color.ApplyMask(colorMask);

        int height = y2 - y1;
        float starty = (float)y2 - starti * (float)height / 100.0f;
        float endy = (float)y2 - endi * (float)height / 100.0f;


        addLineAsTriangles(bg, x1, starty, x1, y2, color);
        addLineAsTriangles(bg, x2 - 0.25f, y2, x2 - 0.25f, endy, color);
        addLineAsTriangles(bg, x1, y2, x2, y2, color, true);
        addLineAsTriangles(bg, x1, starty, x2 + 0.25f, endy, color, true);

        float m = float(endy - starty) / float(x2 - x1);
        const int gap = 10;
        int lastx = x1;

        for (int x = x1 + gap; x < x2; x += gap) {
            float newY = m * (x - x1 - gap) + starty;
            float newY2 = m * (x - x1) + starty;
            addLineAsTriangles(bg, x-gap, newY, x, y2, color);
            addLineAsTriangles(bg, x, newY2, x-gap, y2, color);
            lastx = x;
        }

        // fill in the end
        if (lastx != x2) {
            float newY = m * (lastx - x1) + starty;
            float newY2 = m * (x2-x1) + starty;
            addLineAsTriangles(bg, lastx, newY, x2 - 0.5, y2, color);
            addLineAsTriangles(bg, x2 - 0.5, newY2, lastx, y2, color);
        }
        return 2;
    }

    return 1;
}

static void place_twinkles(int lights_to_place, int &curIndex, std::vector<StrobeClass>& strobe, RenderBuffer& buffer,
                           int max_modulo, size_t colorcnt) {
    // Placement uses the stateless hash RNG (hashRand01, keyed on the slot being
    // filled with distinct salts per draw) rather than the serial rand stream, so
    // the result is reproducible regardless of call order and matches the GPU/ISPC
    // paths. hashRand01 folds in curPeriod, so the same slot re-randomizes each frame.
    while (lights_to_place > 0 && (curIndex < (int)strobe.size())) {
        int span = (int)strobe.size() - curIndex;
        int idx = curIndex + std::min(span - 1, (int)(buffer.hashRand01(0x40000000u + (uint32_t)curIndex) * span));
        if (idx != curIndex) {
            std::swap(strobe[idx], strobe[curIndex]);
        }
        strobe[curIndex].duration = std::min(max_modulo - 1, (int)(buffer.hashRand01(0x50000000u + (uint32_t)curIndex) * max_modulo));
        strobe[curIndex].colorindex = std::min((int)colorcnt - 1, (int)(buffer.hashRand01(0x60000000u + (uint32_t)curIndex) * colorcnt));
        strobe[curIndex].strobing = true;
        curIndex++;
        lights_to_place--;
    }
}

void TwinkleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    // Tier-2 draw pass: when the engine set buffer.pendingSnapshot, prepareTwinkleFrame
    // restores this frame's pre-frame cache from it and we re-advance+draw from that
    // exact state. Serially this restore-and-re-advance is idempotent (the stateless
    // hash RNG plus the same pre-frame state reproduce the post-frame state AdvanceState
    // already wrote), so it stays byte-identical to a plain advance+draw. When there is
    // no snapshot (a caller invoked Render directly), prepareTwinkleFrame leaves the live
    // cache in place and this is the defensive fused advance+draw fall-through.
    TwinkleFrame f = prepareTwinkleFrame(SettingsMap, buffer);
    if (f.isByNode) {
        // The node-indirection path (SetNodePixel maps one node to many buffer
        // coords, and several nodes can share a coord in a group buffer) can't
        // be a clean per-pixel kernel, so it stays on a serial scalar tail.
        renderTwinkleByNode(buffer, f);
        return;
    }
    // Per-pixel path: ISPC is the sole CPU implementation. Brightness/color is
    // baked into a CPU-built double-precision LUT; the kernel is pure integer
    // state evolution + LUT lookup.
    xlColorVector lut;
    buildTwinkleLut(buffer, f, lut);
    dispatchTwinkleISPC(buffer, f, lut);
    applyTwinkleFinishCount(f);
}

// Tier-2 state advance: snapshot the PRE-frame cache, then advance the live cache
// exactly as this frame's draw would, and return the snapshot unconditionally
// (Twinkle is always Snapshottable). The engine calls this once serially per frame
// and then draws via Render(pendingSnapshot) in both serial and frame-parallel
// rendering, so the two paths are byte-identical. This mirrors the old serial
// capture pre-pass: the ISPC-eligible config advances state-only with npix=0 (no
// pixels, no LUT touched - the kernel's only guarded access), while the by-node
// config has no advance-only kernel and so runs the fused advance+draw whose pixels
// the engine discards (Render redraws them from the restored snapshot).
std::unique_ptr<EffectFrameState> TwinkleEffect::AdvanceState(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    auto snap = std::make_unique<TwinkleFrameState>();
    TwinkleFrame f = prepareTwinkleFrame(SettingsMap, buffer, snap.get());
    if (f.isByNode) {
        renderTwinkleByNode(buffer, f);
        return snap;
    }
    TwinkleFrame fc = f;
    fc.npix = 0;
    xlColorVector unusedLut(1);
    dispatchTwinkleISPC(buffer, fc, unusedLut);
    applyTwinkleFinishCount(fc);
    return snap;
}

// Reads the effect parameters and evolves the persistent per-light state array
// (init + renewal/compaction). Mutating; must run exactly once per Render/
// AdvanceState call. captureInto (AdvanceState) receives a deep copy of the
// pre-frame cache before any mutation; otherwise buffer.pendingSnapshot (the
// Render draw pass) restores the pre-frame cache before advancing.
TwinkleFrame TwinkleEffect::prepareTwinkleFrame(const SettingsMap &SettingsMap, RenderBuffer &buffer, TwinkleFrameState *captureInto) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Twinkle_Count", sCountDefault, SettingsMap, oset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Steps = GetValueCurveInt("Twinkle_Steps", sStepsDefault, SettingsMap, oset, sStepsVCMin, sStepsVCMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Strobe = SettingsMap.GetBool("CHECKBOX_Twinkle_Strobe", sStrobeDefault);
    bool reRandomize = SettingsMap.GetBool("CHECKBOX_Twinkle_ReRandom", sReRandomDefault);
    const std::string& twinkle_style = SettingsMap.Get("CHOICE_Twinkle_Style", sStyleDefault);
    bool new_algorithm = false;
    bool isByNode = false;
    if (twinkle_style == "New Render Method") {
        new_algorithm = true;
    }
    int strobeCount = (buffer.BufferHt * buffer.BufferWi);
    if ((buffer.GetNodeCount() * 5) < strobeCount) {
        // less than 20% of the buffer has lights, twinkling buffer coordinates won't work well
        // at all so we'll twinkle by node instead
        isByNode = true;
        strobeCount = buffer.GetNodeCount();
    }

    int lights = std::round(((float)(strobeCount * Count)) / 100.0); // Count is in range of 1-100 from slider bar
    if (strobeCount == 1) {
        lights = 1;
    }
    int step = 1;
    if (lights > 0) {
        step = strobeCount / lights;
    }
    int max_modulo = Steps;
    if (max_modulo < 2) {
        max_modulo = 2;  // scm  could we be getting 0 passed in?
    }
    int max_modulo2 = max_modulo / 2;
    if (max_modulo2 < 1) {
        max_modulo2 = 1;
    }

    if (step < 1) {
        step = 1;
    }

    TwinkleRenderCache *cache = (TwinkleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TwinkleRenderCache();
        buffer.infoCache[id] = cache;
        cache->num_lights = lights;
        cache->lights_to_renew = lights;
        cache->curNumStrobe = 0;
    }
    std::vector<StrobeClass> &strobe = cache->strobe;

    // Tier-2 Snapshottable fork (see GetFrameParallelism / AdvanceState).
    // AdvanceState (captureInto set): store the pre-frame state before anything
    // mutates, then advance as usual. Render draw pass (buffer.pendingSnapshot
    // set): restore the pre-frame state AdvanceState stored (including needToInit -
    // true only when AdvanceState entered this frame needing init) and fall through
    // to the NORMAL flow below, which re-runs this frame's advance (bookkeeping/
    // init/renewal + kernel) from that exact state, reproducing the advanced frame.
    if (captureInto != nullptr) {
        captureInto->strobe = strobe;
        captureInto->num_lights = cache->num_lights;
        captureInto->curNumStrobe = cache->curNumStrobe;
        captureInto->lights_to_renew = cache->lights_to_renew.load();
        captureInto->needToInit = buffer.needToInit;
    } else if (buffer.pendingSnapshot != nullptr) {
        const TwinkleFrameState &fs = static_cast<const TwinkleFrameState&>(*buffer.pendingSnapshot);
        strobe = fs.strobe;
        cache->num_lights = fs.num_lights;
        cache->curNumStrobe = fs.curNumStrobe;
        cache->lights_to_renew.store(fs.lights_to_renew);
        buffer.needToInit = fs.needToInit;
    }

    if (new_algorithm) {
        cache->lights_to_renew += lights - cache->num_lights;
    } else {
        if (lights != cache->num_lights) {
            buffer.needToInit = true;
        }
    }
    cache->num_lights = lights;

    size_t colorcnt = buffer.GetColorCount();
    if (colorcnt == 0) {
        // guards the kernel's `% colorcnt` and the placement modulo against an
        // empty palette (does not happen for a normally-rendered effect)
        colorcnt = 1;
    }

    int i = 0;

    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->lights_to_renew = lights;
        cache->curNumStrobe = 0;
        if (new_algorithm) {
            strobe.clear();
            strobe.resize(strobeCount);
            if (isByNode) {
                for (int s = 0; s < strobeCount; s++) {
                    strobe[s].x = s;
                    strobe[s].y = 0;
                    strobe[s].duration = 0;
                    strobe[s].strobing = -1;
                    strobe[s].isByNode = true;
                }
            } else {
                int s = 0;
                for (int x = 0; x < buffer.BufferWi; x++) {
                    for (int y = 0; y < buffer.BufferHt; y++) {
                        strobe[s].x = x;
                        strobe[s].y = y;
                        strobe[s].duration = 0;
                        strobe[s].strobing = -1;
                        strobe[s].isByNode = false;
                        s++;
                    }
                }
            }
            //randomize the locations
            for (int s = 0; s < (int)strobe.size(); ++s) {
                int r = std::min((int)strobe.size() - 1, (int)(buffer.hashRand01(0x10000000u + (uint32_t)s) * strobe.size()));
                if (r != s) {
                    std::swap(strobe[r], strobe[s]);
                }
            }
        } else {
            strobe.clear();
            cache->curNumStrobe = 0;
            if (isByNode) {
                for (int i = 0; i < strobeCount; i++) {
                    if (i % step == 1 || step == 1) {
                        int s = strobe.size();
                        strobe.resize(s + 1);
                        strobe[s].duration = std::min(max_modulo - 1, (int)(buffer.hashRand01(0x20000000u + (uint32_t)s) * max_modulo));

                        strobe[s].x = i;
                        strobe[s].y = 0;
                        strobe[s].isByNode = true;

                        strobe[s].colorindex = std::min((int)colorcnt - 1, (int)(buffer.hashRand01(0x30000000u + (uint32_t)s) * colorcnt));
                        cache->curNumStrobe++;
                    }
                }
            } else {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    for (int x = 0; x < buffer.BufferWi; x++) {
                        i++;
                        if (i % step == 1 || step == 1) {
                            int s = strobe.size();
                            strobe.resize(s + 1);
                            strobe[s].duration = std::min(max_modulo - 1, (int)(buffer.hashRand01(0x20000000u + (uint32_t)s) * max_modulo));

                            strobe[s].x = x;
                            strobe[s].y = y;
                            strobe[s].isByNode = false;

                            strobe[s].colorindex = std::min((int)colorcnt - 1, (int)(buffer.hashRand01(0x30000000u + (uint32_t)s) * colorcnt));
                            cache->curNumStrobe++;
                        }
                    }
                }
            }
        }
    }

    if (new_algorithm) {
        if (cache->lights_to_renew > 0) {
            while (cache->curNumStrobe && !strobe[cache->curNumStrobe-1].strobing) {
                cache->curNumStrobe--;
            }
            for (int x = 0; x < cache->curNumStrobe; x++) {
                if (!strobe[x].strobing) {
                    cache->curNumStrobe--;
                    if (x != cache->curNumStrobe) {
                        std::swap(strobe[x], strobe[cache->curNumStrobe]);
                    }
                    while (cache->curNumStrobe && !strobe[cache->curNumStrobe-1].strobing) {
                        cache->curNumStrobe--;
                    }
                }
            }
            place_twinkles(cache->lights_to_renew, cache->curNumStrobe, strobe, buffer, max_modulo, colorcnt);
            cache->lights_to_renew = 0;
        }
    }

    TwinkleFrame f;
    f.states = &strobe;
    f.lightsToRenew = &cache->lights_to_renew;
    f.curNumStrobe = cache->curNumStrobe;
    f.max_modulo = max_modulo;
    f.max_modulo2 = max_modulo2;
    f.colorcnt = (int)colorcnt;
    f.frameSeed = buffer.hashRandomFrameSeed();
    f.width = buffer.BufferWi;
    f.npix = (int)buffer.GetPixelCount();
    f.new_algorithm = new_algorithm;
    f.reRandomize = reRandomize;
    f.strobe = Strobe;
    f.isByNode = isByNode;
    return f;
}

// Precompute one RGBA per (colorindex, duration) using the exact scalar double
// math so the integer ISPC/Metal kernels are byte-identical to the historical
// renderer. Indexed [colorindex * (max_modulo+1) + clamp(i7, 0, max_modulo)].
void TwinkleEffect::buildTwinkleLut(RenderBuffer &buffer, const TwinkleFrame &f, xlColorVector &lut) {
    int stride = f.max_modulo + 1;
    lut.resize((size_t)f.colorcnt * (size_t)stride);
    bool allowAlpha = buffer.allowAlpha;
    for (int ci = 0; ci < f.colorcnt; ci++) {
        xlColor baseColor;
        HSVValue baseHsv;
        if (allowAlpha) {
            buffer.palette.GetColor(ci, baseColor);
        } else {
            buffer.palette.GetHSV(ci, baseHsv);
        }
        for (int i7 = 0; i7 <= f.max_modulo; i7++) {
            double v;
            if (i7 <= f.max_modulo2) {
                if (f.max_modulo2 > 0) v = (1.0 * i7) / f.max_modulo2;
                else v = 0;
            } else {
                if (f.max_modulo2 > 0) v = (f.max_modulo - i7) * 1.0 / (f.max_modulo2);
                else v = 0;
            }
            if (v < 0.0) v = 0.0;
            if (f.strobe) {
                if (i7 == f.max_modulo2) v = 1.0;
                else v = 0.0;
            }
            xlColor out;
            if (allowAlpha) {
                out = baseColor;
                out.alpha = 255.0 * v;
            } else {
                HSVValue hsv = baseHsv;
                hsv.value = v;
                out = hsv;
            }
            lut[(size_t)ci * (size_t)stride + (size_t)i7] = out;
        }
    }
}

// ISPC per-light dispatch (the CPU path). Each lane owns one strobe entry: it
// advances the state (duration++, renewal/reRandomize) and writes its unique
// pixel from the LUT. States are read-modify-written in place.
void TwinkleEffect::dispatchTwinkleISPC(RenderBuffer &buffer, const TwinkleFrame &f, const xlColorVector &lut) {
    if (f.curNumStrobe <= 0) {
        return;
    }
    ispc::TwinkleISPCData d;
    d.width = (unsigned int)f.width;
    d.npix = (unsigned int)f.npix;
    d.max_modulo = f.max_modulo;
    d.max_modulo2 = f.max_modulo2;
    d.colorcnt = f.colorcnt;
    d.lutStride = f.max_modulo + 1;
    d.lutSize = (int)lut.size();
    d.new_algorithm = f.new_algorithm ? 1 : 0;
    d.reRandomize = f.reRandomize ? 1 : 0;
    d.frameSeed = f.frameSeed;

    std::vector<StrobeClass> &strobe = *f.states;
    int32_t *statePtr = reinterpret_cast<int32_t*>(strobe.data());
    const ispc::uint8_t4 *lutPtr = (const ispc::uint8_t4*)lut.data();
    ispc::uint8_t4 *pixels = (ispc::uint8_t4*)buffer.GetPixels();

    int total = f.curNumStrobe;
    constexpr int blockSize = 2048;
    if (total >= blockSize * 2) {
        int blocks = (total + blockSize - 1) / blockSize;
        parallel_for(0, blocks, [&d, statePtr, lutPtr, pixels, total](int block) {
            int start = block * blockSize;
            int end = std::min(start + blockSize, total);
            ispc::TwinkleEffectISPC(&d, start, end, statePtr, lutPtr, pixels);
        });
    } else {
        ispc::TwinkleEffectISPC(&d, 0, total, statePtr, lutPtr, pixels);
    }

    if (buffer.dmx_buffer) {
        // DMX fixtures need the colour routed through SetPixel(); the raw uint8_t4
        // writes above (each lane targets its own strobe's buffer index) bypass the
        // DMX channel mapping.
        buffer.SetPixel(0, 0, buffer.GetPixel(0, 0));
    }
}

// new_algorithm: count lights that finished this frame (kernel cleared their
// strobing flag) and schedule them for renewal next frame. Matches the inline
// lights_to_renew++ the scalar renderer did per finishing light.
void TwinkleEffect::applyTwinkleFinishCount(const TwinkleFrame &f) {
    if (!f.new_algorithm) {
        return;
    }
    const std::vector<StrobeClass> &strobe = *f.states;
    int count = 0;
    for (int x = 0; x < f.curNumStrobe; x++) {
        if (strobe[x].strobing == 0) {
            count++;
        }
    }
    *f.lightsToRenew += count;
}

// Serial scalar tail for the by-node path (SetNodePixel can't be a clean pixel
// kernel). Keeps the exact original double math and RNG; the last strobe in
// index order wins for coords shared between nodes in a group buffer.
void TwinkleEffect::renderTwinkleByNode(RenderBuffer &buffer, const TwinkleFrame &f) {
    std::vector<StrobeClass> &strobe = *f.states;
    int max_modulo = f.max_modulo;
    int max_modulo2 = f.max_modulo2;
    int colorcnt = f.colorcnt;
    bool reRandomize = f.reRandomize;
    bool Strobe = f.strobe;
    bool new_algorithm = f.new_algorithm;
    for (int x = 0; x < f.curNumStrobe; x++) {
        strobe[x].duration++;
        if (new_algorithm) {
            if (!strobe[x].strobing) {
                continue;
            }
        }
        if (strobe[x].duration < 0) {
            continue;
        }
        if (strobe[x].duration == max_modulo) {
            strobe[x].duration = 0;
            if (new_algorithm) {
                (*f.lightsToRenew)++;
                strobe[x].strobing = false;
            } else if (reRandomize) {
                strobe[x].duration -= buffer.hashRandom(uint32_t(x) * 131101u) % max_modulo2;
                strobe[x].colorindex = buffer.hashRandom(uint32_t(x) * 131101u + 1u) % colorcnt;
            }
        }
        int i7 = strobe[x].duration;
        HSVValue hsv;
        buffer.palette.GetHSV(strobe[x].colorindex, hsv);
        double v;
        if (i7 <= max_modulo2) {
            if (max_modulo2 > 0) v = (1.0 * i7) / max_modulo2;
            else v = 0;
        } else {
            if (max_modulo2 > 0) v = (max_modulo - i7) * 1.0 / (max_modulo2);
            else v = 0;
        }
        if (v < 0.0) v = 0.0;

        if (Strobe) {
            if (i7 == max_modulo2) v = 1.0;
            else v = 0.0;
        }
        if (buffer.allowAlpha) {
            xlColor color;
            buffer.palette.GetColor(strobe[x].colorindex, color);
            color.alpha = 255.0 * v;
            buffer.SetNodePixel(strobe[x].x, color); // Turn pixel on
        } else {
            buffer.palette.GetHSV(strobe[x].colorindex, hsv);
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            hsv.value = v;
            buffer.SetNodePixel(strobe[x].x, hsv); // Turn pixel on
        }
    }
}
