/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MeteorsEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "media/AudioManager.h"
#include "../models/Model.h"
#include "UtilFunctions.h"

#include <spdlog/fmt/fmt.h>

#include "../../include/meteors-16.xpm"
#include "../../include/meteors-24.xpm"
#include "../../include/meteors-32.xpm"
#include "../../include/meteors-48.xpm"
#include "../../include/meteors-64.xpm"
#include "UtilFunctions.h"

#include "Parallel.h"
#include "ispc/MeteorsFunctions.ispc.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>

std::string MeteorsEffect::sTypeDefault = "Rainbow";
std::string MeteorsEffect::sEffectDefault = "Down";
int MeteorsEffect::sCountDefault = 10;
int MeteorsEffect::sCountMin = 1;
int MeteorsEffect::sCountMax = 100;
int MeteorsEffect::sLengthDefault = 25;
int MeteorsEffect::sLengthMin = 1;
int MeteorsEffect::sLengthMax = 100;
int MeteorsEffect::sSwirlDefault = 0;
int MeteorsEffect::sSwirlMin = 0;
int MeteorsEffect::sSwirlMax = 20;
int MeteorsEffect::sSpeedDefault = 10;
int MeteorsEffect::sSpeedMin = 0;
int MeteorsEffect::sSpeedMax = 50;
int MeteorsEffect::sWarmupFramesDefault = 0;
int MeteorsEffect::sXOffsetDefault = 0;
int MeteorsEffect::sXOffsetMin = -100;
int MeteorsEffect::sXOffsetMax = 100;
int MeteorsEffect::sYOffsetDefault = 0;
int MeteorsEffect::sYOffsetMin = -100;
int MeteorsEffect::sYOffsetMax = 100;
bool MeteorsEffect::sUseMusicDefault = false;
bool MeteorsEffect::sFadeWithDistanceDefault = false;

MeteorsEffect::MeteorsEffect(int id) : RenderableEffect(id, "Meteors", meteors_16, meteors_24, meteors_32, meteors_48, meteors_64)
{
    //ctor
}

MeteorsEffect::~MeteorsEffect()
{
    //dtor
}

void MeteorsEffect::OnMetadataLoaded()
{
    sTypeDefault = GetStringDefault("Meteors_Type", sTypeDefault);
    sEffectDefault = GetStringDefault("Meteors_Effect", sEffectDefault);
    sCountDefault = GetIntDefault("Meteors_Count", sCountDefault);
    sCountMin = (int)GetMinFromMetadata("Meteors_Count", sCountMin);
    sCountMax = (int)GetMaxFromMetadata("Meteors_Count", sCountMax);
    sLengthDefault = GetIntDefault("Meteors_Length", sLengthDefault);
    sLengthMin = (int)GetMinFromMetadata("Meteors_Length", sLengthMin);
    sLengthMax = (int)GetMaxFromMetadata("Meteors_Length", sLengthMax);
    sSwirlDefault = GetIntDefault("Meteors_Swirl_Intensity", sSwirlDefault);
    sSwirlMin = (int)GetMinFromMetadata("Meteors_Swirl_Intensity", sSwirlMin);
    sSwirlMax = (int)GetMaxFromMetadata("Meteors_Swirl_Intensity", sSwirlMax);
    sSpeedDefault = GetIntDefault("Meteors_Speed", sSpeedDefault);
    sSpeedMin = (int)GetMinFromMetadata("Meteors_Speed", sSpeedMin);
    sSpeedMax = (int)GetMaxFromMetadata("Meteors_Speed", sSpeedMax);
    sWarmupFramesDefault = GetIntDefault("Meteors_WamupFrames", sWarmupFramesDefault);
    sXOffsetDefault = GetIntDefault("Meteors_XOffset", sXOffsetDefault);
    sXOffsetMin = (int)GetMinFromMetadata("Meteors_XOffset", sXOffsetMin);
    sXOffsetMax = (int)GetMaxFromMetadata("Meteors_XOffset", sXOffsetMax);
    sYOffsetDefault = GetIntDefault("Meteors_YOffset", sYOffsetDefault);
    sYOffsetMin = (int)GetMinFromMetadata("Meteors_YOffset", sYOffsetMin);
    sYOffsetMax = (int)GetMaxFromMetadata("Meteors_YOffset", sYOffsetMax);
    sUseMusicDefault = GetBoolDefault("Meteors_UseMusic", sUseMusicDefault);
    sFadeWithDistanceDefault = GetBoolDefault("FadeWithDistance", sFadeWithDistanceDefault);
}

std::list<std::string> MeteorsEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Meteors_UseMusic", sUseMusicDefault)) {
        res.push_back(fmt::format("    WARN: Meteors effect cant follow music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

//these must match list indexes in xLightsMain.h: -DJ
#define METEORS_DOWN  0
#define METEORS_UP  1
#define METEORS_LEFT  2
#define METEORS_RIGHT  3
#define METEORS_IMPLODE  4
#define METEORS_EXPLODE  5
#define METEORS_ICICLES  6 //random length drip effect -DJ
#define METEORS_ICICLES_BKG  7 //with bkg (dim) icicles -DJ


static inline int GetMeteorEffect(const std::string &dir) {
    if (dir == "Down") {
        return 0;
    } else if (dir == "Up") {
        return 1;
    } else if (dir == "Left") {
        return 2;
    } else if (dir == "Right") {
        return 3;
    } else if (dir == "Implode") {
        return 4;
    } else if (dir == "Explode") {
        return 5;
    } else if (dir == "Icicles") {
        return 6;
    } else if (dir == "Icicles + bkg") {
        return 7;
    }
    return 0; //down
}
static inline int GetMeteorColorScheme(const std::string &color) {
    if (color == "Rainbow") {
        return 0;
    } else if (color == "Range") {
        return 1;
    } else if (color == "Palette") {
        return 2;
    }

    return 0;
}

class MeteorClass {
public:

    int x,y;
    HSVValue hsv;
    int h = 0; //variable length; only used for icicle drip -DJ
};

// Contiguous, and append-only + order-preserving erase, so the draw order is
// exactly the creation order.  Overlapping meteors overwrite each other, so any
// reordering here changes pixels.
typedef std::vector<MeteorClass> MeteorList;

// NOTE: with hue and saturation fixed, fromHSV() is algebraically just
// `value * k` per component, so a fading trail looks like it could resolve the
// hue/saturation half once and spend three multiplies per step.  Don't: release
// builds are -ffast-math, and reassociating that multiply chain shifts channels
// by +-1 against fromHSV().  A sweep of ~1.8e9 (hue, sat, value, fade) points
// found ~14k mismatches with -ffast-math and zero without it.

class MeteorsRenderCache : public EffectRenderCache {
public:
    MeteorsRenderCache() {};
    virtual ~MeteorsRenderCache() {};

    float effectState = 0;
    MeteorList meteors;
    MeteorRadialList meteorsRadial;

    // One emit key per buffer pixel for drawRadialMeteors, kept across frames.
    // Sized by the buffer, not by the meteor count, so it does not grow with a
    // dense frame the way the old per-write plot array did.
    std::unique_ptr<std::atomic<uint64_t>[]> keys;
    size_t keyCount = 0;
};

// Implode/Explode draw overlapping trails, so serial order decides which meteor
// owns a shared pixel.  Neither SetPixel overload used here reads the old pixel,
// so "later write wins" is the ONLY thing that matters - and since the writes are
// emitted in (meteor, phase) order, the write that survives on a pixel is simply
// the one with the largest (n, ph).  So don't materialise the writes and sort
// them: record that key per pixel with an atomic max, then resolve the colour
// once per covered pixel.  Identical to drawing serially, and it drops the
// per-write plot array, both bucketing passes, and the colour conversions for
// every write that was going to be painted over (a whole-house Implode overdraws
// its buffer several times).
//
// `walk(meteor, n, emit)` calls emit(x, y, ph) for each in-bounds trail step, in
// emit order.  `shade(meteor, n, ph, x, y)` returns that step's colour and runs
// only for the steps that survived.  Splitting them this way also isolates the
// geometry, which is pure double->int and vectorises bit-exactly, from the
// colour, which does not (see the -ffast-math note above).
template <typename Walk, typename Shade>
static void drawRadialMeteors(RenderBuffer& buffer, MeteorsRenderCache* cache, const MeteorRadialList& meteors,
                              int tailLength, Walk&& walk, Shade&& shade) {
    const size_t meteorCount = meteors.size();
    if (meteorCount == 0) {
        return;
    }
    const int W = buffer.BufferWi;
    const int H = buffer.BufferHt;
    if (W <= 0 || H <= 0) {
        return;
    }

    // DMX buffers route SetPixel through SetPixelDMXModel, which is not a plain
    // store into the pixel grid, so a pixel's key would not decide its value.
    // Small draws are not worth the key buffer either.
    if (buffer.dmx_buffer || (int64_t)meteorCount * (tailLength + 1) < 20000) {
        for (size_t n = 0; n < meteorCount; ++n) {
            const MeteorRadialClass& m = meteors[n];
            walk(m, (int)n, [&](int x, int y, int ph) {
                buffer.SetPixel(x, y, shade(m, (int)n, ph, x, y));
            });
        }
        return;
    }

    const size_t pixels = (size_t)W * (size_t)H;
    if (cache->keyCount < pixels) {
        cache->keys.reset(new std::atomic<uint64_t>[pixels]);
        cache->keyCount = pixels;
    }
    std::atomic<uint64_t>* keys = cache->keys.get();
    parallel_for(0, H, [&](int y) {
        std::atomic<uint64_t>* row = keys + (size_t)y * W;
        for (int x = 0; x < W; ++x) {
            row[x].store(0, std::memory_order_relaxed);
        }
    });

    // n + 1 in the high half so that 0 can mean "no trail reached this pixel".
    parallel_for(0, (int)meteorCount, [&](int n) {
        const MeteorRadialClass& m = meteors[n];
        const uint64_t base = (uint64_t)(uint32_t)(n + 1) << 32;
        walk(m, n, [&](int x, int y, int ph) {
            std::atomic<uint64_t>& slot = keys[(size_t)y * W + x];
            const uint64_t k = base | (uint32_t)ph;
            uint64_t cur = slot.load(std::memory_order_relaxed);
            while (cur < k && !slot.compare_exchange_weak(cur, k, std::memory_order_relaxed)) {
            }
        });
    });

    // SetPixelDirect: not a DMX buffer (handled above) and walk() only emitted
    // in-bounds steps, so SetPixel's own range test is pure overhead here.
    parallel_for(0, H, [&](int y) {
        const std::atomic<uint64_t>* row = keys + (size_t)y * W;
        for (int x = 0; x < W; ++x) {
            const uint64_t k = row[x].load(std::memory_order_relaxed);
            if (k == 0) {
                continue;
            }
            const int n = (int)(uint32_t)(k >> 32) - 1;
            const int ph = (int)(uint32_t)k;
            buffer.SetPixelDirect(x, y, shade(meteors[n], n, ph, x, y));
        }
    });
}


static MeteorsRenderCache* GetCache(RenderBuffer &buffer, int id) {
    MeteorsRenderCache *cache = (MeteorsRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new MeteorsRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

// Tier-2 immutable per-frame draw state: the meteor snapshot + gather params.
struct MeteorsFrameState : public EffectFrameState {
    std::vector<MeteorSnapshot> parts;
    MeteorsGatherParams params;
};

// Build this frame's immutable draw snapshot from the advanced meteor list.
// The axis-aligned advance (AdvanceState) hands it back to Render, which draws
// it via GatherMeteors in both serial and frame-parallel rendering.
static std::unique_ptr<MeteorsFrameState> BuildMeteorsFrame(const MeteorsGatherParams& params, std::vector<MeteorSnapshot>& parts) {
    auto fs = std::make_unique<MeteorsFrameState>();
    fs->parts = std::move(parts);
    fs->params = params;
    return fs;
}

// Which of the two draw paths a style uses: the radial (Implode/Explode) trail
// walk, or the axis-aligned per-pixel gather.  Render, AdvanceState and the
// snapshot type all key off this, so they can never disagree about which
// EffectFrameState subclass is in flight.
bool MeteorsEffect::IsRadialStyle(const SettingsMap& settings) {
    int eff = GetMeteorEffect(settings.Get("CHOICE_Meteors_Effect", sEffectDefault));
    return eff == METEORS_IMPLODE || eff == METEORS_EXPLODE;
}

// The Implode trail stops at the first phase that lands on the centre.  The CPU
// walk just breaks out, but a GPU dispatches every phase as an independent
// thread and has to be told the cut up front.  Only a meteor whose trail passes
// within a few pixels of the centre can stop at all - the closest the trail ever
// gets is |R - ph| for ph in [0, TL], and int() truncation can shift a coord by
// at most 1 in each axis, so a closest approach beyond 3*sqrt(2) cannot satisfy
// the 2x2 test.  8.0 is that bound with margin, and it lets most meteors skip
// the scan entirely.
static int radialCutFor(const MeteorRadialClass& m, int centerX, int centerY, int TL) {
    const float ox = m.x - (float)centerX;
    const float oy = m.y - (float)centerY;
    if (sqrtf(ox * ox + oy * oy) - (float)TL > 8.0f) {
        return TL + 1;
    }
    for (int ph = 0; ph <= TL; ph++) {
        const int x = int(m.x - m.dx * float(ph));
        const int y = int(m.y - m.dy * float(ph));
        if ((abs(y - centerY) < 2) && (abs(x - centerX) < 2)) {
            return ph;
        }
    }
    return TL + 1;
}

void MeteorsEffect::ComputeRadialCuts(const MeteorsRadialFrameState& fs, std::vector<int>& cuts) {
    const int TL = fs.tailLength;
    cuts.assign(fs.meteors.size(), TL + 1);
    if (!fs.implode) {
        return;
    }
    int* out = cuts.data();
    parallel_for(0, (int)fs.meteors.size(), [&fs, TL, out](int n) {
        out[n] = radialCutFor(fs.meteors[n], fs.centerX, fs.centerY, TL);
    });
}

// Draw pass for the radial styles: a pure function of the snapshot and the
// buffer it writes into, so the engine can rasterise many frames of it at once.
// The two styles differ only in which way the trail walks away from the head,
// and in Implode stopping a trail once it reaches the centre.
void MeteorsEffect::DrawRadialSnapshot(RenderBuffer& buffer, const MeteorsRadialFrameState& fs) {
    MeteorsRenderCache* cache = GetCache(buffer, id);

    const int W = buffer.BufferWi;
    const int H = buffer.BufferHt;
    const bool allowAlpha = buffer.allowAlpha;
    const bool implode = fs.implode;
    const int centerX = fs.centerX;
    const int centerY = fs.centerY;
    const int maxdiag = fs.maxdiag;
    const int TailLength = fs.tailLength;
    const int ColorScheme = fs.colorScheme;
    const bool fadeWithDistance = fs.fadeWithDistance;
    // Rainbow re-rolls the hue every step and fade-with-distance rescales the
    // value per pixel.  Everything else walks the trail with one fixed HSV, and
    // when the alpha channel is carrying the fade the colour never changes at
    // all - so resolve it once per meteor instead of once per step.
    const bool solidTrail = (ColorScheme != 0) && !fadeWithDistance && allowAlpha;

    // Geometry only, through ISPC.  The scatter that consumes these cannot
    // vectorise (an atomic max per step), so the steps come back in small
    // batches that stay in L1 rather than through a per-meteor array.  Implode's
    // stop-at-centre is resolved up front by radialCutFor so the kernel needs no
    // early exit - and so the CPU, Metal and Vulkan paths all cut at the same
    // phase.  About 1e-6 of steps land a pixel off the scalar form; see the
    // kernel for why that is accepted.
    auto walk = [implode, centerX, centerY, TailLength, W, H](const MeteorRadialClass& meteor, int n, auto&& emit) {
        // if we were to swirl, it would need to alter the angle here
        const int cut = implode ? radialCutFor(meteor, centerX, centerY, TailLength) : TailLength + 1;
        constexpr int BATCH = 512;
        int xs[BATCH], ys[BATCH];
        for (int s = 0; s < cut; s += BATCH) {
            const int e = std::min(cut, s + BATCH);
            ispc::MeteorsRadialWalkISPC(meteor.x, meteor.y, meteor.dx, meteor.dy,
                                        implode ? 1 : 0, s, e, W, H, xs, ys);
            for (int i = 0; i < e - s; i++) {
                if (xs[i] >= 0) { // -1 marks a step that fell outside the buffer
                    emit(xs[i], ys[i], s + i);
                }
            }
        }
    };

    auto shade = [&buffer, fadeWithDistance, centerX, centerY, maxdiag, TailLength, ColorScheme, allowAlpha, solidTrail](const MeteorRadialClass& meteor, int n, int ph, int x, int y) -> xlColor {
        const float fade = float(ph) / float(TailLength);
        // Integer, not 255.0f*fade: the exact value is floor(255*ph/TailLength),
        // and computing it in float made the alpha channel the single largest
        // source of CPU-vs-GPU drift (all of it +-1).  Integer division is exact
        // and identical on every backend, so this channel simply cannot drift.
        const uint8_t alpha = (uint8_t)((255 * ph) / TailLength);
        if (solidTrail) {
            xlColor c(meteor.hsv);
            c.alpha = alpha;
            return c;
        }

        HSVValue hsv;
        switch (ColorScheme) {
        case 0:
            hsv.hue = buffer.hashRand01(uint32_t(n) * 131101u + uint32_t(ph));
            hsv.saturation = 1.0;
            hsv.value = 1.0;
            break;
        default:
            hsv = meteor.hsv;
            break;
        }

        if (fadeWithDistance) {
            // distance
            int distance = (int)sqrtf((float)((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY)));
            if (distance < 10) {
                distance = 10;
            }
            hsv.value *= float(distance) / float(maxdiag);
        }

        if (allowAlpha) {
            xlColor c(hsv);
            c.alpha = alpha;
            return c;
        }
        hsv.value *= fade;
        return xlColor(hsv);
    };

    drawRadialMeteors(buffer, cache, fs.meteors, TailLength, walk, shade);
}

RenderableEffect::FrameParallelism MeteorsEffect::GetFrameParallelism(const SettingsMap& settings) const {
    // Every style splits into a serial particle advance and a pure draw, so
    // AdvanceState always returns a snapshot - see the contract in
    // RenderableEffect.h.
    return FrameParallelism::Snapshottable;
}

float MeteorsEffect::calcEffectStateOffset(int mSpeed, RenderBuffer& buffer) {
    if (mSpeed == 0) {
        // at least advance a little bit
        return 0.1f;
    }
    return (float(mSpeed * buffer.frameTimeInMs)) / 50.0f;
}

// Without this the gather is O(pixels x meteors): a whole-house buffer carries tens of
// thousands of live meteors, so 800x286 x 63k is 1.4e10 inner iterations a frame.
void MeteorsEffect::BucketMeteorsByLine(const std::vector<MeteorSnapshot>& parts, int lineCount,
                                        std::vector<int>& lineStart, std::vector<int>& lineItems) {
    lineStart.assign(lineCount + 1, 0);
    for (const auto& p : parts) {
        if (p.a >= 0 && p.a < lineCount) {
            ++lineStart[p.a + 1];
        }
    }
    for (int i = 0; i < lineCount; i++) {
        lineStart[i + 1] += lineStart[i];
    }
    lineItems.assign(lineStart[lineCount], 0);
    std::vector<int> fill(lineStart.begin(), lineStart.end() - 1);
    for (int n = 0; n < (int)parts.size(); n++) {
        int a = parts[n].a;
        if (a >= 0 && a < lineCount) {
            lineItems[fill[a]++] = n;
        }
    }
}

// Base (CPU) gather: run the ISPC kernel. Each output pixel is scored against the
// meteors bucketed onto its line; the last one in draw order that covers it wins,
// reproducing the scalar SetPixel overwrite. Uncovered pixels are left at the
// buffer's cleared value (the render engine pre-clears each frame).
void MeteorsEffect::GatherMeteors(RenderBuffer& buffer, const MeteorsGatherParams& params, const std::vector<MeteorSnapshot>& parts) {
    ispc::MeteorsISPCData d;
    d.width = buffer.BufferWi;
    d.height = buffer.BufferHt;
    d.mode = params.mode;
    d.direction = params.direction;
    d.tailLength = params.tailLength;
    d.colorScheme = params.colorScheme;
    d.allowAlpha = params.allowAlpha;
    d.numMeteors = (int)parts.size();
    d.wantBkg = params.wantBkg;
    d.frameSeed = params.frameSeed;

    std::vector<ispc::MeteorParticle> ip(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        ip[i].a = parts[i].a;
        ip[i].base = parts[i].base;
        ip[i].h = parts[i].h;
        ip[i].hue = parts[i].hue;
        ip[i].sat = parts[i].sat;
        ip[i].val = parts[i].val;
    }

    xlColor* pixels = buffer.GetPixels();
    const ispc::MeteorParticle* pp = ip.data();
    int W = buffer.BufferWi;
    int H = buffer.BufferHt;
    if (W <= 0 || H <= 0) {
        return;
    }

    const int lineCount = (params.mode == 1) ? H : W;
    std::vector<int> lineStart;
    std::vector<int> lineItems;
    BucketMeteorsByLine(parts, lineCount, lineStart, lineItems);

    // an icicle background still paints lines that carry no meteor
    const bool allLines = (params.mode == 2 && params.wantBkg != 0);
    const int* items = lineItems.data();
    const int* starts = lineStart.data();
    auto renderLine = [&d, pp, pixels, items, starts, allLines](int line) {
        int s = starts[line];
        int e = starts[line + 1];
        if (s == e && !allLines) {
            return;
        }
        ispc::MeteorsEffectLineISPC(&d, pp, items, s, e, line, (ispc::uint8_t4*)pixels);
    };
    if ((size_t)W * H >= 20000) {
        parallel_for(0, lineCount, renderLine);
    } else {
        for (int line = 0; line < lineCount; line++) {
            renderLine(line);
        }
    }

    if (buffer.dmx_buffer) {
        // DMX fixtures need the colour routed through SetPixel(); the raw uint8_t4
        // writes above touched every channel in the buffer (sized to the DMX
        // channel count, not just node 0), so clear it back to transparent first —
        // otherwise leftover meteor/trail colors leak into unrelated channels
        // (pan/tilt, shutter, etc.) that SetPixelDMXModel doesn't itself set.
        xlColor c = buffer.GetPixel(0, 0);
        buffer.Clear();
        buffer.SetPixel(0, 0, c);
    }
}


// ColorScheme: 0=rainbow, 1=range, 2=palette
// MeteorsEffect: 0=down, 1=up, 2=left, 3=right, 4=implode, 5=explode
void MeteorsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    const bool radial = IsRadialStyle(SettingsMap);

    if (buffer.pendingSnapshot != nullptr) {
        // Draw pass: rasterise the snapshot AdvanceState produced; no sim advance.
        // This is the ONLY path the tier-2 engine reaches, in both serial and
        // frame-parallel rendering.
        if (radial) {
            DrawRadialSnapshot(buffer, static_cast<const MeteorsRadialFrameState&>(*buffer.pendingSnapshot));
        } else {
            const MeteorsFrameState& fs = static_cast<const MeteorsFrameState&>(*buffer.pendingSnapshot);
            GatherMeteors(buffer, fs.params, fs.parts);
        }
        return;
    }

    // Defensive fall-through for a caller that invokes Render without first
    // going through AdvanceState (the engine enters via the pendingSnapshot
    // branch above); the draw is a pure function of the snapshot.
    auto fs = AdvanceState(effect, SettingsMap, buffer);
    if (fs == nullptr) {
        return;
    }
    if (radial) {
        DrawRadialSnapshot(buffer, static_cast<const MeteorsRadialFrameState&>(*fs));
    } else {
        const MeteorsFrameState& mfs = static_cast<const MeteorsFrameState&>(*fs);
        GatherMeteors(buffer, mfs.params, mfs.parts);
    }
}

// Tier-2 advance: run this frame's particle simulation and return the immutable
// draw snapshot.  Every style is Snapshottable, so this always returns non-null
// (the contract GetFrameParallelism promises).  All stream RNG (add/randInt) is
// consumed here in serial; the draw is a pure function of the returned snapshot.
std::unique_ptr<EffectFrameState> MeteorsEffect::AdvanceState(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int MeteorsEffect = GetMeteorEffect(SettingsMap.Get("CHOICE_Meteors_Effect", sEffectDefault));

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Meteors_Count", sCountDefault, SettingsMap, oset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Length = GetValueCurveInt("Meteors_Length", sLengthDefault, SettingsMap, oset, sLengthMin, sLengthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SwirlIntensity = GetValueCurveInt("Meteors_Swirl_Intensity", sSwirlDefault, SettingsMap, oset, sSwirlMin, sSwirlMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mSpeed = GetValueCurveInt("Meteors_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int ColorScheme = GetMeteorColorScheme(SettingsMap.Get("CHOICE_Meteors_Type", sTypeDefault));
    int warmupFrames = SettingsMap.GetInt("SLIDER_Meteors_WamupFrames", sWarmupFramesDefault);

    if (SettingsMap.GetBool("CHECKBOX_Meteors_UseMusic", sUseMusicDefault)) {
        float f = 0.0;
        if (buffer.GetMedia() != nullptr) {
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                f = pf->max;
            }
        }
        Count = (float)Count * f;
    }

    MeteorsRenderCache *cache = GetCache(buffer, id);

    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->meteors.clear();
        cache->meteorsRadial.clear();
        cache->effectState = calcEffectStateOffset(mSpeed, buffer);
    }

    switch (MeteorsEffect) {
        case METEORS_DOWN: //0:
        case METEORS_UP: //1:
            return RenderMeteorsVertical(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
        case METEORS_LEFT: //2:
        case METEORS_RIGHT: //3:
            return RenderMeteorsHorizontal(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
        case METEORS_ICICLES: //6
            return RenderIcicleDrip(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
        case METEORS_ICICLES_BKG: //7
            return RenderIcicleDrip(buffer, ColorScheme, Count, -Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
        case METEORS_IMPLODE: //4:
        case METEORS_EXPLODE: { //5:
            int xoffset = GetValueCurveInt("Meteors_XOffset", sXOffsetDefault, SettingsMap, oset, sXOffsetMin, sXOffsetMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
            int yoffset = GetValueCurveInt("Meteors_YOffset", sYOffsetDefault, SettingsMap, oset, sYOffsetMin, sYOffsetMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
            bool fadeWithDistance = SettingsMap.GetBool("CHECKBOX_FadeWithDistance", sFadeWithDistanceDefault);
            if (MeteorsEffect == METEORS_IMPLODE) {
                return AdvanceMeteorsImplode(buffer, ColorScheme, Count, Length, mSpeed, xoffset, yoffset, fadeWithDistance, warmupFrames);
            }
            return AdvanceMeteorsExplode(buffer, ColorScheme, Count, Length, mSpeed, xoffset, yoffset, fadeWithDistance, warmupFrames);
        }
    }
    return nullptr;
}

/*
 * *************************************************************
 *  Horizontal
 * *************************************************************
 */

#pragma region Horizontal

class MeteorHasExpiredX
{
    int TailLength;
public:
    MeteorHasExpiredX(int t)
    : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj) {
        return obj.x + TailLength < 0;
    }
};

void MeteorsEffect::HorizontalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;
    for (int i = 0; i < buffer.BufferHt; i++) {
        if (buffer.randInt(0, 199) < Count) {
            m.x = buffer.BufferWi - 1;
            m.y = i;

            switch (ColorScheme) {
            case 1:
                buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                break;
            case 2:
                buffer.palette.GetHSV(buffer.randInt(0, (int)colorcnt - 1), m.hsv);
                break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::HorizontalMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    for (auto& meteor : cache->meteors) {
        meteor.x -= mspeed;
    }
}

void MeteorsEffect::HorizontalRemoveMeteors(RenderBuffer& buffer, int Length)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int TailLength = (buffer.BufferWi < 10) ? Length / 10 : buffer.BufferWi * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // delete old meteors
    std::erase_if(cache->meteors, MeteorHasExpiredX(TailLength));
}

std::unique_ptr<EffectFrameState> MeteorsEffect::RenderMeteorsHorizontal(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += calcEffectStateOffset(mSpeed, buffer);
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            HorizontalAddMeteors(buffer, ColorScheme, Count);
            HorizontalMoveMeteors(buffer, mspeed);
            HorizontalRemoveMeteors(buffer, Length);
        }
    }

    cache->effectState += calcEffectStateOffset(mSpeed, buffer);
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int TailLength = (buffer.BufferWi < 10) ? Length / 10 : buffer.BufferWi * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    HorizontalAddMeteors(buffer, ColorScheme, Count);

    // render meteors: snapshot the live list (swirl folded into the row) and gather per-pixel
    std::vector<MeteorSnapshot> parts;
    parts.reserve(cache->meteors.size());
    int n = 0;
    for (auto& meteor : cache->meteors) {
        double swirl_phase = double(meteor.x) / 5.0 + double(n) / 100.0;
        int dy = int(double(SwirlIntensity * buffer.BufferHt) / 80.0 * buffer.sin(swirl_phase));
        parts.push_back({ meteor.y + dy, meteor.x, 0, meteor.hsv.hue, meteor.hsv.saturation, meteor.hsv.value });
        ++n;
    }
    MeteorsGatherParams params{ 1, MeteorsEffect, TailLength, ColorScheme, buffer.allowAlpha ? 1 : 0, 0, buffer.hashRandomFrameSeed() };
    auto fs = BuildMeteorsFrame(params, parts);

    HorizontalMoveMeteors(buffer, speed);

    HorizontalRemoveMeteors(buffer, Length);
    return fs;
}

#pragma endregion

/*
 * *************************************************************
 *  Vertical
 * *************************************************************
 */

#pragma region Vertical

class MeteorHasExpiredY
{
    int TailLength;
public:
    MeteorHasExpiredY(int t)
    : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj) {
        return obj.y + TailLength < 0;
    }
};

void MeteorsEffect::VerticalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;

    for (int i = 0; i < buffer.BufferWi; i++) {
        if (buffer.randInt(0, 199) < Count) {
            m.x = i;
            m.y = buffer.BufferHt - 1;

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(buffer.randInt(0, (int)colorcnt - 1), m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::VerticalMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    for (auto& meteor : cache->meteors) {
        meteor.y -= mspeed;
    }
}

void MeteorsEffect::VerticalRemoveMeteors(RenderBuffer& buffer, int Length)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // delete old meteors
    std::erase_if(cache->meteors, MeteorHasExpiredY(TailLength));
}

std::unique_ptr<EffectFrameState> MeteorsEffect::RenderMeteorsVertical(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += calcEffectStateOffset(mSpeed, buffer);
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            VerticalAddMeteors(buffer, ColorScheme, Count);
            VerticalMoveMeteors(buffer, mspeed);
            VerticalRemoveMeteors(buffer, Length);
        }
    }

    cache->effectState += calcEffectStateOffset(mSpeed, buffer);
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    VerticalAddMeteors(buffer, ColorScheme, Count);

    // render meteors: snapshot the live list (swirl folded into the column) and gather per-pixel
    std::vector<MeteorSnapshot> parts;
    parts.reserve(cache->meteors.size());
    int n = 0;
    for (auto& meteor : cache->meteors) {
        // we adjust x axis with some sine function if swirl1 or swirl2
        // swirling more than 25% of the buffer width doesn't look good
        double swirl_phase = double(meteor.y) / 5.0 + double(n) / 100.0;
        int dx = int(double(SwirlIntensity * buffer.BufferWi) / 80.0 * buffer.sin(swirl_phase));
        parts.push_back({ meteor.x + dx, meteor.y, 0, meteor.hsv.hue, meteor.hsv.saturation, meteor.hsv.value });
        ++n;
    }
    MeteorsGatherParams params{ 0, MeteorsEffect, TailLength, ColorScheme, buffer.allowAlpha ? 1 : 0, 0, buffer.hashRandomFrameSeed() };
    auto fs = BuildMeteorsFrame(params, parts);

    VerticalMoveMeteors(buffer, speed);

    VerticalRemoveMeteors(buffer, Length);
    return fs;
}

#pragma endregion

/*
 * *************************************************************
 *  Icicle
 * *************************************************************
 */

#pragma region Icicle

// predicate to remove variable length meteors (icicles):
class IcicleHasExpired
{
public:
    bool operator()(const MeteorClass& obj)
    {
        return obj.y < -obj.h;
    }
};

void MeteorsEffect::IcicleAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;
    for (int i = 0; i < buffer.BufferWi; i++) {
        if (buffer.randInt(0, 199) < Count) {
            m.x = i;
            m.y = buffer.BufferHt - 1;
            m.h = buffer.randInt(0, 2 * buffer.BufferHt - 1) / 3; // somewhat variable length -DJ

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(buffer.randInt(0, (int)colorcnt - 1), m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::IcicleMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    for (auto& meteor : cache->meteors) {
        meteor.y -= mspeed;
    }
}

void MeteorsEffect::IcicleRemoveMeteors(RenderBuffer& buffer)
    {
    MeteorsRenderCache* cache = GetCache(buffer, id);

    // delete old meteors
    std::erase_if(cache->meteors, IcicleHasExpired());
}

//icicle drip effect, based on RenderMeteorsVertical: -DJ
std::unique_ptr<EffectFrameState> MeteorsEffect::RenderIcicleDrip(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += calcEffectStateOffset(mSpeed, buffer);
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            IcicleAddMeteors(buffer, ColorScheme, Count);
            IcicleMoveMeteors(buffer, mspeed);
            IcicleRemoveMeteors(buffer);
        }
    }

    cache->effectState += calcEffectStateOffset(mSpeed, buffer);
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    bool want_bkg = (Length < 0);
    if (want_bkg)
        Length = -Length; // kludge; too lazy to add another parameter ;)

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    IcicleAddMeteors(buffer, ColorScheme, Count);

    // render meteors: snapshot the live list (swirl folded into the column) and gather
    // per-pixel. The dim background icicles and the colored-tip/white-body/variable-length
    // drip logic all move into the kernel.
    std::vector<MeteorSnapshot> parts;
    parts.reserve(cache->meteors.size());
    int n = 0;
    for (auto& meteor : cache->meteors) {
        // we adjust x axis with some sine function if swirl1 or swirl2
        // swirling more than 25% of the buffer width doesn't look good
        float swirl_phase = float(meteor.y) / 5.0f + float(n) / 100.0f;
        int dx = int(float(SwirlIntensity * buffer.BufferWi) / 80.0f * buffer.sin(swirl_phase));
        parts.push_back({ meteor.x + dx, meteor.y, meteor.h, meteor.hsv.hue, meteor.hsv.saturation, meteor.hsv.value });
        ++n;
    }
    MeteorsGatherParams params{ 2, MeteorsEffect, TailLength, ColorScheme, buffer.allowAlpha ? 1 : 0, want_bkg ? 1 : 0, buffer.hashRandomFrameSeed() };
    auto fs = BuildMeteorsFrame(params, parts);

    IcicleMoveMeteors(buffer, speed);

    IcicleRemoveMeteors(buffer);
    return fs;
}

#pragma endregion

/*
 * *************************************************************
 *  Implode
 * *************************************************************
 */

#pragma region Implode

class MeteorHasExpiredImplode
{
    int cx, cy;

public:
    MeteorHasExpiredImplode(int centerX, int centerY)
    {
        cx = centerX;
        cy = centerY;
    }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return (std::abs(obj.y - cy) < 2) && (std::abs(obj.x - cx) < 2);
    }
};

void MeteorsEffect::ImplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    MeteorRadialClass m;
    m.cnt = 1;

    for (int i = 0; i < MinDimension; i++) {
        if (buffer.randInt(0, 199) < Count) {
            double angle;
            if (buffer.BufferHt == 1) {
                angle = double(buffer.randInt(0, 1)) * M_PI;
            } else if (buffer.BufferWi == 1) {
                angle = double(buffer.randInt(0, 1)) * M_PI - (M_PI / 2.0);
            } else {
                angle = buffer.rand01() * 2.0 * M_PI;
            }
            m.dx = buffer.cos(angle);
            m.dy = buffer.sin(angle);
            // m.x = centerX + double(halfdiag + TailLength)*m.dx;
            // m.y = centerY + double(halfdiag + TailLength)*m.dy;
            m.x = centerX + float(maxdiag + TailLength) * m.dx;
            m.y = centerY + float(maxdiag + TailLength) * m.dy;

            switch (ColorScheme) {
            case 1:
                buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                break;
            case 2:
                buffer.palette.GetHSV(buffer.randInt(0, (int)colorcnt - 1), m.hsv);
                break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }
}

void MeteorsEffect::ImplodeMoveMeteors(RenderBuffer& buffer, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    for (auto& meteor : cache->meteorsRadial) {
        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        meteor.x -= meteor.dx * mspeed * hdistance;
        meteor.y -= meteor.dy * mspeed * hdistance;
        meteor.cnt++;
    }
}

void MeteorsEffect::ImplodeRemoveMeteors(RenderBuffer& buffer, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // delete old meteors
    std::erase_if(cache->meteorsRadial, MeteorHasExpiredImplode(buffer.BufferWi / 2 + truexoffset, buffer.BufferHt / 2 + trueyoffset));
}

std::unique_ptr<EffectFrameState> MeteorsEffect::AdvanceMeteorsImplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += calcEffectStateOffset(mSpeed, buffer);
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            ImplodeAddMeteors(buffer, ColorScheme, Count, Length, xoffset, yoffset);
            ImplodeMoveMeteors(buffer, mspeed, xoffset, yoffset, fadeWithDistance);
            ImplodeRemoveMeteors(buffer, xoffset, yoffset);
        }
    }

    cache->effectState += calcEffectStateOffset(mSpeed, buffer);
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    ImplodeAddMeteors(buffer, ColorScheme, Count, Length, xoffset, yoffset);

    // Hand the live particle list to the draw pass.  Everything the trail needs
    // that is not a property of the buffer travels with it, so the draw is a pure
    // function of the snapshot and many frames of it can rasterise at once.
    auto fs = std::make_unique<MeteorsRadialFrameState>();
    fs->meteors = cache->meteorsRadial;
    fs->implode = true;
    fs->centerX = centerX;
    fs->centerY = centerY;
    fs->maxdiag = maxdiag;
    fs->tailLength = TailLength;
    fs->colorScheme = ColorScheme;
    fs->fadeWithDistance = fadeWithDistance;


    ImplodeMoveMeteors(buffer, speed, xoffset, yoffset, fadeWithDistance);

    ImplodeRemoveMeteors(buffer, xoffset, yoffset);
    return fs;
}
#pragma endregion

/*
 * *************************************************************
 *  Explode
 * *************************************************************
 */
#pragma region Explode

class MeteorHasExpiredExplode
{
    int ht, wi;

public:
    MeteorHasExpiredExplode(int h, int w)
    {
        ht = h;
        wi = w;
    }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return obj.y < 0 || obj.x < 0 || obj.y > ht || obj.x > wi;
    }
};

void MeteorsEffect::ExplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);
    size_t colorcnt = buffer.GetColorCount();

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);

    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;

    MeteorRadialClass m;
    m.x = buffer.BufferWi / 2 + truexoffset;
    m.y = buffer.BufferHt / 2 + trueyoffset;
    m.cnt = 1;
    for (int i = 0; i < MinDimension; i++) {
        if (buffer.randInt(0, 199) < Count) {
            double angle;
            if (buffer.BufferHt == 1) {
                    angle = double(buffer.randInt(0, 1)) * M_PI;
            } else if (buffer.BufferWi == 1) {
                    angle = double(buffer.randInt(0, 1)) * M_PI - (M_PI / 2.0);
            } else {
                    angle = buffer.rand01() * 2.0 * M_PI;
            }
            m.dx = buffer.cos(angle);
            m.dy = buffer.sin(angle);

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(buffer.randInt(0, (int)colorcnt - 1), m.hsv);
                    break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }
}

void MeteorsEffect::ExplodeMoveMeteors(RenderBuffer& buffer, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;

    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    for (auto& meteor : cache->meteorsRadial) {
        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        meteor.x += meteor.dx * mspeed * hdistance;
        meteor.y += meteor.dy * mspeed * hdistance;
        meteor.cnt++;
    }
}

void MeteorsEffect::ExplodeRemoveMeteors(RenderBuffer& buffer)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    // delete old meteors
    std::erase_if(cache->meteorsRadial, MeteorHasExpiredExplode(buffer.BufferHt, buffer.BufferWi));
}

std::unique_ptr<EffectFrameState> MeteorsEffect::AdvanceMeteorsExplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += calcEffectStateOffset(mSpeed, buffer);
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            ExplodeAddMeteors(buffer, ColorScheme, Count, xoffset, yoffset);
            ExplodeMoveMeteors(buffer, mspeed, xoffset, yoffset, fadeWithDistance);
            ExplodeRemoveMeteors(buffer);
        }
    }

    cache->effectState += calcEffectStateOffset(mSpeed, buffer);
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    HSVValue hsv, hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    ExplodeAddMeteors(buffer, ColorScheme, Count, xoffset, yoffset);

    // Hand the live particle list to the draw pass.  Everything the trail needs
    // that is not a property of the buffer travels with it, so the draw is a pure
    // function of the snapshot and many frames of it can rasterise at once.
    auto fs = std::make_unique<MeteorsRadialFrameState>();
    fs->meteors = cache->meteorsRadial;
    fs->implode = false;
    fs->centerX = centerX;
    fs->centerY = centerY;
    fs->maxdiag = maxdiag;
    fs->tailLength = TailLength;
    fs->colorScheme = ColorScheme;
    fs->fadeWithDistance = fadeWithDistance;


    ExplodeMoveMeteors(buffer, speed, xoffset, yoffset, fadeWithDistance);

    ExplodeRemoveMeteors(buffer);
    return fs;
}

#pragma endregion
