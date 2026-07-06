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

#include <cstdint>
#include <vector>

#include "RenderableEffect.h"

class LifeEffect : public RenderableEffect
{
public:
    LifeEffect(int id);
    virtual ~LifeEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override { return false; }

    // Cached from Life.json by OnMetadataLoaded().
    static int sCountDefault;
    static int sSeedDefault;
    static int sSpeedDefault;

protected:
    virtual void OnMetadataLoaded() override;

    // Shared CPU bookkeeping used by both the ISPC (base) and Metal render paths:
    // seeds the board on init and gates generation advance on the sub-frame state.
    // Returns true if a new generation should be computed this frame (tempbuf holds
    // the previous generation); false if the frame was satisfied by copying tempbuf
    // to the pixels. outType receives the ruleset.
    bool PrepareLifeGeneration(RenderBuffer& buffer, const SettingsMap& settings, int& outType);

    // Computes one generation with the ISPC kernel (tempbuf must already hold the
    // previous generation) and advances tempbuf. Used as the base render path and
    // the Metal wrapper's fallback after PrepareLifeGeneration has run.
    void RenderLifeGenerationISPC(RenderBuffer& buffer, int type);

    // Snapshots palette.GetColor(i) into packed little-endian RGBA (matching xlColor
    // memory layout) for the ISPC/Metal birth-colour blend.
    static void BuildLifePalette(RenderBuffer& buffer, std::vector<uint32_t>& palette);
};
