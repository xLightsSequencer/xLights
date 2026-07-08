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
#include "../utils/Color.h"

#include <cstdint>
#include <vector>

#define WAVE_FILL_NONE       0
#define WAVE_FILL_RAINBOW    1
#define WAVE_FILL_PALETTE    2
#define MAX_WAVE_COLORS      8

// Shared render config the ISPC/Metal kernels need. The per-column vertical band
// itself is passed separately (2 ints per column: [2x]=y1, [2x+1]=y2).
struct WaveKernelConfig {
    int width = 0;
    int height = 0;
    int fillColor = WAVE_FILL_NONE;
    int mirror = 0;
    int yoffset = 0;
    int numColors = 1;
    xlColor noneColor;
    xlColor palColors[MAX_WAVE_COLORS];
};

class WaveEffect : public RenderableEffect
{
public:
    WaveEffect(int id);
    virtual ~WaveEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanBeRandom() override
    {
        return false;
    }

    // Cached from Wave.json by OnMetadataLoaded().
    static std::string sWaveTypeDefault;
    static std::string sFillColorsDefault;
    static bool sMirrorWaveDefault;
    // Number_Waves is now a float "number of cycles" with divisor 360. The
    // underlying storage stays in degrees (pre-divisor 180..3600) so old
    // sequence value curves keep working; Render multiplies the divided
    // float back up to get the raw degree count used by the wave math.
    static double sNumberWavesDefault;
    static double sNumberWavesMin;
    static double sNumberWavesMax;
    static int sNumberWavesDivisor;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sWaveHeightDefault;
    static int sWaveHeightMin;
    static int sWaveHeightMax;
    static double sWaveSpeedDefault;
    static double sWaveSpeedMin;
    static double sWaveSpeedMax;
    static int sWaveSpeedDivisor;
    static std::string sWaveDirectionDefault;
    static int sYOffsetDefault;
    static int sYOffsetMin;
    static int sYOffsetMax;

protected:
    virtual void OnMetadataLoaded() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

    // Advances the render-cache phase accumulator, (re)builds the Fractal/ivy
    // branch buffer (init-time RNG stays on CPU), and computes the per-column
    // wave band [y1,y2] with the exact scalar double math (incl. std::sin). A
    // column that draws nothing is stored as y1 > y2. cols holds 2 ints per
    // column: cols[2x]=y1, cols[2x+1]=y2. Shared by the ISPC (CPU) and Metal
    // (GPU) render entry points so the phase advances exactly once per frame.
    void BuildWaveColumns(const SettingsMap& settings, RenderBuffer& buffer,
                          WaveKernelConfig& cfg, std::vector<int32_t>& cols);
    // Fills the precomputed bands with the ISPC kernel (the CPU path; handles
    // None/Rainbow/Palette fills).
    void RenderWaveISPC(const WaveKernelConfig& cfg, const std::vector<int32_t>& cols, RenderBuffer& buffer);
};
