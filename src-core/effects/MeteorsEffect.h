#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#include <vector>
#include <cstdint>

// One live meteor snapshotted for the per-pixel gather kernel (ISPC/Metal). The
// swirl is already folded into the axis coordinate on the CPU so the kernel needs
// no trig; hue/sat/val carry meteor.hsv (range/palette + the icicle color).
struct MeteorSnapshot {
    int a;       // primary axis coord: column (vertical/icicle) or row (horizontal)
    int base;    // meteor.y (vertical/icicle) or meteor.x (horizontal)
    int h;       // icicle drip length (meteor.h); 0 otherwise
    double hue;
    double sat;
    double val;
};

struct MeteorsGatherParams {
    int mode;         // 0 = vertical, 1 = horizontal, 2 = icicle
    int direction;    // raw MeteorsEffect: 0 Down, 1 Up, 2 Left, 3 Right, 6/7 Icicle
    int tailLength;
    int colorScheme;  // 0 = rainbow, 1 = range, 2 = palette
    int allowAlpha;
    int wantBkg;      // icicle: draw dim background icicles
    uint64_t frameSeed;
};

// One live meteor of a radial (Implode/Explode) trail.  float, not double: a
// meteor lives ~60 frames over a buffer at most a couple of thousand pixels
// wide, and replaying a full lifetime both ways puts the accumulated float
// position error at 0.0025 px - so double bought no accuracy, only half the SIMD
// lanes and twice the memory traffic.  Matching the GPU's precision is the other
// half of the reason: Metal and Vulkan have no double, so this is what lets the
// CPU and GPU trails agree instead of drifting a pixel apart.
class MeteorRadialClass
{
public:
    float x, y, dx, dy;
    int cnt;
    HSVValue hsv;
};

typedef std::vector<MeteorRadialClass> MeteorRadialList;

// Tier-2 immutable per-frame draw state for the radial styles.  These keep the
// live particles rather than a flattened snapshot: the trail is generated from
// each meteor's position and direction, not from a per-pixel axis coordinate.
struct MeteorsRadialFrameState : public EffectFrameState {
    MeteorRadialList meteors;
    bool implode = true;
    int centerX = 0;
    int centerY = 0;
    int maxdiag = 0;
    int tailLength = 1;
    int colorScheme = 0;
    bool fadeWithDistance = false;
};

class MeteorsEffect : public RenderableEffect
{
public:
    MeteorsEffect(int id);
    virtual ~MeteorsEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    // Tier-2: every style splits into a cheap serial particle advance
    // (AdvanceState) and a pure draw.  The axis-aligned styles
    // (vertical/horizontal/icicle) draw through GatherMeteors, the radial ones
    // (Implode/Explode) through DrawRadialSnapshot; IsRadialStyle picks between
    // them and is the single predicate all three entry points share.
    virtual std::unique_ptr<EffectFrameState> AdvanceState(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual FrameParallelism GetFrameParallelism(const SettingsMap& settings) const override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    // Cached from Meteors.json by OnMetadataLoaded().
    static std::string sTypeDefault;
    static std::string sEffectDefault;
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sLengthDefault;
    static int sLengthMin;
    static int sLengthMax;
    static int sSwirlDefault;
    static int sSwirlMin;
    static int sSwirlMax;
    static int sSpeedDefault;
    static int sSpeedMin;
    static int sSpeedMax;
    static int sWarmupFramesDefault;
    static int sXOffsetDefault;
    static int sXOffsetMin;
    static int sXOffsetMax;
    static int sYOffsetDefault;
    static int sYOffsetMin;
    static int sYOffsetMax;
    static bool sUseMusicDefault;
    static bool sFadeWithDistanceDefault;

    static bool IsRadialStyle(const SettingsMap& settings);

protected:
    virtual void OnMetadataLoaded() override;

    // Per-pixel gather of the axis-aligned styles (vertical/horizontal/icicle).
    // Base implementation runs ISPC (the CPU path); MetalMeteorsEffect overrides
    // to dispatch the GPU kernel and falls back here when Metal isn't viable.
    virtual void GatherMeteors(RenderBuffer& buffer, const MeteorsGatherParams& params, const std::vector<MeteorSnapshot>& parts);

    // A meteor's axis coord is the only line of the buffer it can ever cover (the
    // column it falls down, or the row it crosses), so bucket the snapshot by line and
    // a pixel need only scan its own line's meteors instead of all of them. Counting
    // sort, so a line's meteors stay in ascending snapshot order and the last one still
    // wins the pixel. lineStart holds lineCount+1 prefix offsets into lineItems, which
    // holds global indices into parts. Meteors whose line falls outside the buffer are
    // dropped: no pixel can match their axis coord. Shared by the ISPC, Metal and
    // Vulkan gathers so all three bucket identically.
    static void BucketMeteorsByLine(const std::vector<MeteorSnapshot>& parts, int lineCount,
                                    std::vector<int>& lineStart, std::vector<int>& lineItems);

    // Draw pass for the radial styles (Implode/Explode) - a pure function of the
    // snapshot.  Base implementation is the CPU key-max scatter; MetalMeteors
    // Effect overrides to dispatch the GPU kernels and falls back here when Metal
    // isn't viable.
    virtual void DrawRadialSnapshot(RenderBuffer& buffer, const MeteorsRadialFrameState& fs);

    // Per-meteor phase at which an Implode trail stops (Explode never stops).
    // Shared by the CPU and Metal paths so both cut identically.
    static void ComputeRadialCuts(const MeteorsRadialFrameState& fs, std::vector<int>& cuts);

private:
    std::unique_ptr<EffectFrameState> RenderMeteorsVertical(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed, int warmupFrames);
        void VerticalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void VerticalMoveMeteors(RenderBuffer& buffer, int speed);
        void VerticalRemoveMeteors(RenderBuffer& buffer, int Length);
    std::unique_ptr<EffectFrameState> RenderMeteorsHorizontal(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed, int warmupFrames);
        void HorizontalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void HorizontalMoveMeteors(RenderBuffer& buffer, int speed);
        void HorizontalRemoveMeteors(RenderBuffer& buffer, int Length);
    std::unique_ptr<EffectFrameState> AdvanceMeteorsImplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int mspeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames);
        void ImplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int xoffset, int yoffset);
        void ImplodeMoveMeteors(RenderBuffer& buffer, int speed, int xoffset, int yoffset, bool fadeWithDistance);
        void ImplodeRemoveMeteors(RenderBuffer& buffer, int xoffset, int yoffset);
    std::unique_ptr<EffectFrameState> RenderIcicleDrip(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames);
        void IcicleAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count);
        void IcicleMoveMeteors(RenderBuffer& buffer, int mSpeed);
        void IcicleRemoveMeteors(RenderBuffer& buffer);
    std::unique_ptr<EffectFrameState> AdvanceMeteorsExplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames);
        void ExplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int xoffset, int yoffset);
        void ExplodeMoveMeteors(RenderBuffer& buffer, int speed, int xoffset, int yoffset, bool fadeWithDistance);
        void ExplodeRemoveMeteors(RenderBuffer& buffer);

    float calcEffectStateOffset(int mSpeed, RenderBuffer& buffer);
};
