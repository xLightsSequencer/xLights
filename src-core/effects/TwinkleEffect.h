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

#include <atomic>
#include <cstdint>
#include <vector>

#include "RenderableEffect.h"

// Per-light strobe state. The ISPC/Metal kernels index this array as a flat
// int32 array with a stride of 6, so the layout must stay exactly six packed
// int32 fields (x, y, duration, colorindex, strobing, isByNode).
class StrobeClass {
public:
    int32_t x = 0, y = 0;
    int32_t duration = 0; // How many frames the strobe light stays on; decremented each frame
    int32_t colorindex = 0;
    int32_t strobing = 0;
    int32_t isByNode = 0;
};

// Output of the shared per-frame setup (parameter read + renewal/compaction),
// consumed by both the ISPC (CPU) and Metal (GPU) per-light dispatch paths.
struct TwinkleFrame {
    std::vector<StrobeClass>* states = nullptr; // the persistent per-light state array
    std::atomic_int* lightsToRenew = nullptr;   // points into the render cache
    int curNumStrobe = 0;
    int max_modulo = 2;
    int max_modulo2 = 1;
    int colorcnt = 1;
    uint64_t frameSeed = 0; // RenderBuffer::hashRandomFrameSeed() for this frame
    int width = 0;
    int npix = 0;           // pixel-write bound (GetPixelCount())
    bool new_algorithm = false;
    bool reRandomize = false;
    bool strobe = false;
    bool isByNode = false;
};

class TwinkleEffect : public RenderableEffect
{
public:
    TwinkleEffect(int id);
    virtual ~TwinkleEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual FrameParallelism GetFrameParallelism(const SettingsMap& settings) const override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        // Return 0xFFFF to opt out of UpgradeValueCurve auto-rescaling for
        // Twinkle_Steps. Old sequences stored Max=100; adjustSettings migrates
        // them to Max=400 while preserving the actual step values.
        if (name == "E_VALUECURVE_Twinkle_Steps")
            return 0xFFFF;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

    // Cached from Twinkle.json by OnMetadataLoaded().
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sStepsDefault;
    // Twinkle_Steps slider and VC both go 2..400 (vcMax=400 in JSON).
    // GetSettingVCDivisor returns 0xFFFF to disable UpgradeValueCurve
    // auto-rescaling; adjustSettings manually migrates old VCs (Max=100)
    // to the new limits (Max=400) without changing their stored values.
    static int sStepsVCMin;
    static int sStepsVCMax;
    static bool sStrobeDefault;
    static bool sReRandomDefault;
    static std::string sStyleDefault;

protected:
    virtual void OnMetadataLoaded() override;

    // Shared setup + dispatch helpers used by the CPU (ISPC) and Metal paths.
    // prepareTwinkleFrame mutates the persistent state (renewal/compaction) and
    // must run exactly once per Render call.
    TwinkleFrame prepareTwinkleFrame(const SettingsMap& settings, RenderBuffer& buffer);
    // Builds the per-(colorindex,duration) RGBA lookup table with the exact
    // scalar double-precision brightness/color math so the integer kernels are
    // byte-identical to the historical scalar renderer.
    void buildTwinkleLut(RenderBuffer& buffer, const TwinkleFrame& f, xlColorVector& lut);
    void dispatchTwinkleISPC(RenderBuffer& buffer, const TwinkleFrame& f, const xlColorVector& lut);
    void renderTwinkleByNode(RenderBuffer& buffer, const TwinkleFrame& f);
    void applyTwinkleFinishCount(const TwinkleFrame& f);
};
