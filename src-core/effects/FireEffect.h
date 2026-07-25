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

class FireEffect : public RenderableEffect
{
public:
    FireEffect(int id);
    virtual ~FireEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    // Left Stateful (the default): Fire's cost is ~86% its serial grid-diffusion
    // advance, so the tier-2 advance/draw split gives no speedup and measured
    // net-negative on whole-house buffers.  The advance/draw are still split into
    // Render + the DrawFire helper below, but frame-parallel does not engage.
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    // Cached from Fire.json by OnMetadataLoaded(). Exposed as statics so any
    // cold-path code (imports, Metal subclass, tests) can read them without
    // needing a pointer to the singleton effect instance.
    static int sHeightDefault;
    static int sHeightMin;
    static int sHeightMax;
    static int sHueShiftDefault;
    static int sHueShiftMin;
    static int sHueShiftMax;
    static double sGrowthCyclesDefault;
    static double sGrowthCyclesMin;
    static double sGrowthCyclesMax;
    static int sGrowthCyclesDivisor;
    static bool sGrowWithMusicDefault;
    static std::string sLocationDefault;
    static std::string sStyleDefault;

    // "New Render Method": the flame grid advances one step per frame instead of
    // being rebuilt bottom-up, so every cell depends only on the previous frame
    // and the whole grid updates at once (ISPC / GPU).  The classic method's
    // in-frame vertical dependency is strictly serial in y and cannot be
    // parallelised, which is why the fast paths hang off this setting.
    enum FireStyle { FIRE_STYLE_OLD = 0, FIRE_STYLE_NEW = 1 };
    static int GetStyle(const SettingsMap& settings);

    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

    // Everything one frame of the flame advance + draw needs, resolved by
    // Render() so the GPU subclass never has to restate the settings, value
    // curves and buffer geometry.
    struct FireFrameParams {
        std::vector<int>* grid;  // maxMWi * maxMHt palette indices
        const xlColor* lut;      // FirePaletteSize() entries, hue shift folded in
        int maxMWi;              // flame grid extent
        int maxMHt;
        int curWi;               // drawn extent
        int curHt;
        int loc;                 // 0 bottom, 1 top, 2 left, 3 right
        int step;                // per-row brightness change
        int liveRow;             // in/out: highest row known to be non-zero
        bool gridChangedOnCpu;   // CPU wrote the grid since the last GPU frame
        uint64_t frameSeed;      // RenderBuffer::hashRandomFrameSeed()
    };
    static int FirePaletteSize();

protected:
    virtual void OnMetadataLoaded() override;

    // Advance the flame one frame and rasterise it, both on the GPU, leaving the
    // grid GPU-resident for the next frame.  Returning false (the default) hands
    // the whole frame back to the CPU path.
    virtual bool RenderFireGPU(RenderBuffer& buffer, FireFrameParams& params) {
        return false;
    }
};
