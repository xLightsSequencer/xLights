/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CandleEffect.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <spdlog/fmt/fmt.h>
#include <map>
#include <vector>

#include "ispc/CandleFunctions.ispc.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "Parallel.h"
#include "../utils/xlSize.h"

#include "../../include/candle-16.xpm"
#include "../../include/candle-24.xpm"
#include "../../include/candle-32.xpm"
#include "../../include/candle-48.xpm"
#include "../../include/candle-64.xpm"

int CandleEffect::sFlameAgilityDefault = 2;
int CandleEffect::sFlameAgilityMin = 1;
int CandleEffect::sFlameAgilityMax = 10;
int CandleEffect::sWindBaselineDefault = 30;
int CandleEffect::sWindBaselineMin = 0;
int CandleEffect::sWindBaselineMax = 255;
int CandleEffect::sWindVariabilityDefault = 5;
int CandleEffect::sWindVariabilityMin = 0;
int CandleEffect::sWindVariabilityMax = 10;
int CandleEffect::sWindCalmnessDefault = 2;
int CandleEffect::sWindCalmnessMin = 0;
int CandleEffect::sWindCalmnessMax = 10;
bool CandleEffect::sPerNodeDefault = false;
bool CandleEffect::sUsePaletteDefault = false;

CandleEffect::CandleEffect(int id) : RenderableEffect(id, "Candle", candle_16, candle_24, candle_32, candle_48, candle_64)
{
    //ctor
}

CandleEffect::~CandleEffect()
{
    //dtor
}

void CandleEffect::OnMetadataLoaded()
{
    sFlameAgilityDefault = GetIntDefault("Candle_FlameAgility", sFlameAgilityDefault);
    sFlameAgilityMin = (int)GetMinFromMetadata("Candle_FlameAgility", sFlameAgilityMin);
    sFlameAgilityMax = (int)GetMaxFromMetadata("Candle_FlameAgility", sFlameAgilityMax);
    sWindBaselineDefault = GetIntDefault("Candle_WindBaseline", sWindBaselineDefault);
    sWindBaselineMin = (int)GetMinFromMetadata("Candle_WindBaseline", sWindBaselineMin);
    sWindBaselineMax = (int)GetMaxFromMetadata("Candle_WindBaseline", sWindBaselineMax);
    sWindVariabilityDefault = GetIntDefault("Candle_WindVariability", sWindVariabilityDefault);
    sWindVariabilityMin = (int)GetMinFromMetadata("Candle_WindVariability", sWindVariabilityMin);
    sWindVariabilityMax = (int)GetMaxFromMetadata("Candle_WindVariability", sWindVariabilityMax);
    sWindCalmnessDefault = GetIntDefault("Candle_WindCalmness", sWindCalmnessDefault);
    sWindCalmnessMin = (int)GetMinFromMetadata("Candle_WindCalmness", sWindCalmnessMin);
    sWindCalmnessMax = (int)GetMaxFromMetadata("Candle_WindCalmness", sWindCalmnessMax);
    sPerNodeDefault = GetBoolDefault("PerNode", sPerNodeDefault);
    sUsePaletteDefault = GetBoolDefault("UsePalette", sUsePaletteDefault);
}

std::list<std::string> CandleEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Candle_GrowWithMusic", false)) {
        res.push_back(fmt::format("    WARN: Candle effect cant grow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

// the ISPC/Metal kernels index the state array as raw bytes with a stride of 5
static_assert(sizeof(CandleState) == 5, "CandleState must stay 5 packed bytes");

void CandleState::init(RenderBuffer& buffer) {
    flamer = buffer.rand01() * 255;
    flameprimer = buffer.rand01() * 255;
    flameg = buffer.rand01() * flamer;
    flameprimeg = buffer.rand01() * flameprimer;
    wind = buffer.rand01() * 255;
}

class CandleRenderCache : public EffectRenderCache
{
public:
    std::vector<CandleState> _states;
    int maxWid;

    CandleRenderCache(){};
    virtual ~CandleRenderCache() {
        _states.clear();
    };
};

static CandleRenderCache* GetCache(RenderBuffer& buffer, int id)
{
    CandleRenderCache* cache = (CandleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CandleRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}


void CandleEffect::Update(RenderBuffer& buffer, uint32_t seed, uint8_t& flameprime, uint8_t& flame, uint8_t& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline)
{
    // Uses the stateless hash RNG keyed on distinct seeds rather than the serial
    // per-buffer stream so the result is reproducible regardless of call order.
    // We simulate a gust of wind by setting the wind var to a random value
    if (uint8_t(buffer.hashRand01(seed) * 255.0) < windVariability) {
        wind = uint8_t(buffer.hashRand01(seed + 1u) * 255.0);
    }

    // The wind constantly settles towards its baseline value
    if (wind > windBaseline) {
        wind--;
    }

    // The flame constantly gets brighter till the wind knocks it down
    if (flame < 255) {
        flame++;
    }

    // Depending on the wind strength and the calmness modifier we calculate the odds
    // of the wind knocking down the flame by setting it to random values
    if (uint8_t(buffer.hashRand01(seed + 2u) * 255) < (wind >> windCalmness)) {
        flame = uint8_t(buffer.hashRand01(seed + 3u) * 255);
    }

    // Real flames ook like they have inertia so we use this constant-aproach-rate filter
    // To lowpass the flame height
    if (flame > flameprime) {
        if (flameprime < (255 - flameAgility)) {
            flameprime += flameAgility;
        }
    } else {
        if (flameprime > flameAgility) {
            flameprime -= flameAgility;
        }
    }

    // How do we prevent jittering when the two are equal?
    // We don't. It adds to the realism.
}

std::vector<CandleState>* CandleEffect::getPerNodeStates(RenderBuffer& buffer, const SettingsMap& SettingsMap, int& maxWid)
{
    CandleRenderCache* cache = GetCache(buffer, id);
    std::vector<CandleState>& states = cache->_states;

    if (buffer.needToInit) {
        buffer.needToInit = false;

        xlSize maxBuffer = buffer.GetMaxBuffer(SettingsMap);
        int maxMWi = maxBuffer.width == -1 ? buffer.BufferWi : maxBuffer.width;
        int maxMHt = maxBuffer.height == -1 ? buffer.BufferHt : maxBuffer.height;
        cache->maxWid = maxMWi;
        int numStates = maxMWi * maxMHt;
        if (numStates > (int)states.size()) {
            states.resize(numStates);
        }
        for (int x = 0; x < numStates; x++) {
            states[x].init(buffer);
        }
    }
    maxWid = cache->maxWid;
    return &states;
}

// Tier-2 immutable per-frame draw state: the whole per-node flame/wind state
// vector plus the buffer's needToInit flag, captured BEFORE this frame's
// advance.  A frame-parallel draw pass restores these onto a clone and re-runs
// the normal render (advance + draw fused, per-frame-seeded hash RNG),
// reproducing the serial frame byte-for-byte.
struct CandleFrameState : public EffectFrameState {
    std::vector<CandleState> states;
    int maxWid = 0;
    bool needToInit = false;
};

RenderableEffect::FrameParallelism CandleEffect::GetFrameParallelism(const SettingsMap& settings) const {
    // All cross-frame state is CandleRenderCache (per-node 5-byte flame/wind
    // random-walk states), which CandleFrameState snapshots in full; the walk
    // uses the per-frame-seeded hash RNG, so a draw pass that restores the
    // pre-frame state and re-runs the advance reproduces the serial frame.
    return FrameParallelism::Snapshottable;
}

// 10 <= HeightPct <= 100
void CandleEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    // Tier-2 draw pass: the engine runs AdvanceState first (serially advancing the
    // simulation and capturing the pre-frame snapshot) and then calls Render with
    // buffer.pendingSnapshot set, in BOTH serial and frame-parallel rendering.  We
    // restore that snapshot and re-run the fused advance+draw from it.
    std::unique_ptr<EffectFrameState> owned;
    const EffectFrameState* snap = buffer.pendingSnapshot;
    if (snap == nullptr) {
        // Defensive fall-through: a direct caller invoked Render without first going
        // through AdvanceState (e.g. a preview).  Advance + capture here, then draw
        // the snapshot - a pure function of it, so this stays byte-identical.
        owned = AdvanceState(effect, SettingsMap, buffer);
        snap = owned.get();
    }
    int maxW = 0;
    RestoreSnapshotStates(buffer, *snap, maxW);
    RenderDraw(effect, SettingsMap, buffer);
}

std::unique_ptr<EffectFrameState> CandleEffect::AdvanceState(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    CandleRenderCache* cache = GetCache(buffer, id);

    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", sFlameAgilityDefault, SettingsMap, oset, sFlameAgilityMin, sFlameAgilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", sWindCalmnessDefault, SettingsMap, oset, sWindCalmnessMin, sWindCalmnessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", sWindVariabilityDefault, SettingsMap, oset, sWindVariabilityMin, sWindVariabilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", sWindBaselineDefault, SettingsMap, oset, sWindBaselineMin, sWindBaselineMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", sPerNodeDefault);

    auto fs = std::make_unique<CandleFrameState>();

    if (perNode) {
        // getPerNodeStates seeds every per-node state (stream RNG via
        // CandleState::init) on the first frame and clears needToInit.  Seeding MUST
        // happen here in the serial advance, never in Render, so a parallel draw
        // clone never touches the stream RNG.
        int maxW = 0;
        std::vector<CandleState>& states = *getPerNodeStates(buffer, SettingsMap, maxW);

        // Snapshot the post-init, pre-advance state; Render restores this and draws.
        fs->states = states;
        fs->maxWid = maxW;
        fs->needToInit = false;

        // Advance the live cache exactly as the fused ISPC/Metal/Vulkan kernel does
        // for state - same node->pixel mapping, same clamped params, same
        // hashRand01 seeds and call order, minus all colour/pixel work.  The scalar
        // Update() is the reference the kernels are bit-identical to (see
        // ispc/CandleFunctions.ispc), so re-running it here reproduces the kernels'
        // per-node state bit-for-bit.
        if (!states.empty()) {
            int wi = buffer.BufferWi;
            uint32_t numStates = (uint32_t)states.size();
            int npix = (int)std::min<size_t>((size_t)buffer.BufferWi * buffer.BufferHt, buffer.GetPixelCount());
            // Match the kernel's clamps (windCalmness feeds a right shift; a value
            // curve can exceed the slider bounds).
            size_t cWindVariability = (size_t)std::clamp(windVariability, 0, 255);
            size_t cFlameAgility = (size_t)std::clamp(flameAgility, 0, 254);
            size_t cWindCalmness = (size_t)std::clamp(windCalmness, 0, 31);
            size_t cWindBaseline = (size_t)std::clamp(windBaseline, 0, 255);
            for (int index = 0; index < npix; index++) {
                int x = index % wi;
                int y = index / wi;
                uint32_t stateIdx = (uint32_t)(y * maxW + x);
                if (stateIdx < numStates) {
                    CandleState& st = states[stateIdx];
                    uint32_t seed = stateIdx * 131101u;
                    Update(buffer, seed, st.flameprimer, st.flamer, st.wind, cWindVariability, cFlameAgility, cWindCalmness, cWindBaseline);
                    Update(buffer, seed + 4u, st.flameprimeg, st.flameg, st.wind, cWindVariability, cFlameAgility, cWindCalmness, cWindBaseline);
                    if (st.flameprimeg > st.flameprimer) {
                        st.flameprimeg = st.flameprimer;
                    }
                    if (st.flameg > st.flamer) {
                        st.flameprimeg = st.flameprimer;
                    }
                }
            }
        }
    } else {
        std::vector<CandleState>& states = cache->_states;
        if (buffer.needToInit) {
            buffer.needToInit = false;
            if (states.empty()) {
                states.resize(1);
            }
            states[0].init(buffer);
        }

        // Snapshot the post-init, pre-advance state; Render restores this and draws.
        fs->states = states;
        fs->maxWid = cache->maxWid;
        fs->needToInit = false;

        // Single-flame advance mirrors the scalar draw path, which uses UNCLAMPED
        // params (this mode never runs on the GPU/ISPC clamped kernel).
        CandleState* state = &states[0];
        Update(buffer, 0u, state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
        Update(buffer, 4u, state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
        if (state->flameprimeg > state->flameprimer) {
            state->flameprimeg = state->flameprimer;
        }
        if (state->flameg > state->flamer) {
            state->flameprimeg = state->flameprimer;
        }
    }
    return fs;
}

std::vector<CandleState>* CandleEffect::RestoreSnapshotStates(RenderBuffer& buffer, const EffectFrameState& snap, int& maxWid)
{
    CandleRenderCache* cache = GetCache(buffer, id);
    const CandleFrameState& fs = static_cast<const CandleFrameState&>(snap);
    cache->_states = fs.states;
    cache->maxWid = fs.maxWid;
    // AdvanceState always seeds before snapshotting, so the draw pass never inits
    // (which would consume stream RNG on a parallel clone).  Guard the invariant.
    assert(!fs.needToInit);
    buffer.needToInit = false;
    maxWid = cache->maxWid;
    return &cache->_states;
}

void CandleEffect::RenderDraw(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", sFlameAgilityDefault, SettingsMap, oset, sFlameAgilityMin, sFlameAgilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", sWindCalmnessDefault, SettingsMap, oset, sWindCalmnessMin, sWindCalmnessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", sWindVariabilityDefault, SettingsMap, oset, sWindVariabilityMin, sWindVariabilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", sWindBaselineDefault, SettingsMap, oset, sWindBaselineMin, sWindBaselineMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", sPerNodeDefault);
    bool usePalette = SettingsMap.GetBool("CHECKBOX_UsePalette", sUsePaletteDefault);

    const auto& pal = effect->GetPalette();
    xlColor c1, c2;
    if (usePalette){  // We're using the palette.
        if (pal.empty()) {
            // No colors selected. Default to white. Set black as second color.
            c1 = xlWHITE;
            c2 = xlBLACK;
        } else {
            // One color selected, set black as second color.
            c1 = pal[0];
            c2 = (pal.size() > 1 ? pal[1] : xlBLACK);
        }
    }

    if (perNode) {
        int maxW = 0;
        std::vector<CandleState>& states = *getPerNodeStates(buffer, SettingsMap, maxW);
        if (states.empty()) {
            return;
        }

        // ISPC gather: one lane per pixel runs both flame Updates (bit-identical
        // hashRand01 emulation) against the shared per-node state cache and writes
        // the color.
        ispc::CandleISPCData cd;
        cd.width = buffer.BufferWi;
        cd.height = buffer.BufferHt;
        cd.maxWid = maxW;
        cd.numStates = (uint32_t)states.size();
        cd.frameSeed = buffer.hashRandomFrameSeed();
        // Clamp to the ranges the kernel emulates bit-identically to the old scalar
        // loop; windCalmness feeds a right shift, so a value >= 32 would be UB. A
        // value curve or hand-edited setting can exceed the slider metadata bounds,
        // so clamp the values feeding the kernel rather than keep a scalar fallback.
        cd.windVariability = std::clamp(windVariability, 0, 255);
        cd.flameAgility = std::clamp(flameAgility, 0, 254);
        cd.windCalmness = std::clamp(windCalmness, 0, 31);
        cd.windBaseline = std::clamp(windBaseline, 0, 255);
        cd.usePalette = usePalette ? 1 : 0;
        cd.c1r = c1.red; cd.c1g = c1.green; cd.c1b = c1.blue;
        cd.c2r = c2.red; cd.c2g = c2.green; cd.c2b = c2.blue;
        xlColor* pixels = buffer.GetPixels();
        uint8_t* statesPtr = reinterpret_cast<uint8_t*>(states.data());
        int wi = buffer.BufferWi;
        // Bound writes by the real pixel allocation: a variable sub-buffer can leave
        // GetPixelCount() below BufferWi*BufferHt and the kernel writes result[index]
        // unguarded.
        int npix = (int)std::min<size_t>((size_t)buffer.BufferWi * buffer.BufferHt, buffer.GetPixelCount());
        if (npix >= 20000) {
            parallel_for(0, buffer.BufferHt, [&cd, pixels, statesPtr, wi, npix](int y) {
                int start = y * wi;
                int end = std::min(start + wi, npix);
                if (start < end) {
                    ispc::CandleEffectISPC(&cd, start, end, statesPtr, (ispc::uint8_t4*)pixels);
                }
            });
        } else {
            ispc::CandleEffectISPC(&cd, 0, npix, statesPtr, (ispc::uint8_t4*)pixels);
        }
    } else {
        CandleRenderCache* cache = GetCache(buffer, id);
        std::vector<CandleState>& states = cache->_states;

        if (buffer.needToInit) {
            buffer.needToInit = false;
            if (states.empty()) {
                states.resize(1);
            }
            states[0].init(buffer);
        }

        CandleState* state = &states[0];

        Update(buffer, 0u, state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
        Update(buffer, 4u, state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);

        if (state->flameprimeg > state->flameprimer)
            state->flameprimeg = state->flameprimer;
        if (state->flameg > state->flamer)
            state->flameprimeg = state->flameprimer;

        //  Now play Candle
        xlColor c;
        if (usePalette) {
            float t = float(state->flameprimer) / 255.0f;
            c.red = uint8_t(c1.red * (1.0f - t) + c2.red * t);
            c.green = uint8_t(c1.green * (1.0f - t) + c2.green * t);
            c.blue = uint8_t(c1.blue * (1.0f - t) + c2.blue * t);
        } else {
            c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
        }
        for (int y = 0; y < buffer.BufferHt; y++) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                buffer.SetPixel(x, y, c);
            }
        }
    }
}
