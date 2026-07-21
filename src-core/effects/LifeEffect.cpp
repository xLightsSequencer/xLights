/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LifeEffect.h"

#include <algorithm>

#include "ispc/LifeFunctions.ispc.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "Parallel.h"

#include "../../include/life-16.xpm"
#include "../../include/life-24.xpm"
#include "../../include/life-32.xpm"
#include "../../include/life-48.xpm"
#include "../../include/life-64.xpm"
#include "UtilFunctions.h"

int LifeEffect::sCountDefault = 50;
int LifeEffect::sSeedDefault = 0;
int LifeEffect::sSpeedDefault = 10;

LifeEffect::LifeEffect(int id) : RenderableEffect(id, "Life", life_16, life_24, life_32, life_48, life_48)
{
    //ctor
}

LifeEffect::~LifeEffect()
{
    //dtor
}

void LifeEffect::OnMetadataLoaded()
{
    sCountDefault = GetIntDefault("Life_Count", sCountDefault);
    sSeedDefault = GetIntDefault("Life_Seed", sSeedDefault);
    sSpeedDefault = GetIntDefault("Life_Speed", sSpeedDefault);
}

class LifeRenderCache : public EffectRenderCache {
public:
    LifeRenderCache() : LastLifeCount(0), LastLifeType(0), LastLifeState(0) {};
    virtual ~LifeRenderCache() {};
    int LastLifeCount;
    int LastLifeType;
    int LastLifeState;
};

void LifeEffect::BuildLifePalette(RenderBuffer& buffer, std::vector<uint32_t>& palette)
{
    int n = (int)buffer.palette.Size(); // >= 1
    palette.resize(n);
    xlColor c;
    for (int i = 0; i < n; i++) {
        buffer.palette.GetColor(i, c); // white if i >= explicit size
        palette[i] = uint32_t(c.red) | (uint32_t(c.green) << 8) | (uint32_t(c.blue) << 16) | (uint32_t(c.alpha) << 24);
    }
}

bool LifeEffect::PrepareLifeGeneration(RenderBuffer& buffer, const SettingsMap& SettingsMap, int& outType)
{
    int Count = SettingsMap.GetInt("SLIDER_Life_Count", sCountDefault);
    int Type = SettingsMap.GetInt("SLIDER_Life_Seed", sSeedDefault);
    int lspeed = SettingsMap.GetInt("SLIDER_Life_Speed", sSpeedDefault);
    outType = Type;

    LifeRenderCache* cache = (LifeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new LifeRenderCache();
        buffer.infoCache[id] = cache;
    }

    int BufferHt = buffer.BufferHt;
    int BufferWi = buffer.BufferWi;
    if (BufferHt < 1)
        BufferHt = 1;
    Count = BufferWi * BufferHt * Count / 200 + 1;
    if (buffer.needToInit || Count != cache->LastLifeCount || Type != cache->LastLifeType) {
        buffer.needToInit = false;
        // seed tempbuf
        cache->LastLifeCount = Count;
        cache->LastLifeType = Type;
        buffer.ClearTempBuf();
        xlColor color;
        for (int i = 0; i < Count; i++) {
            int x = buffer.randInt(0, BufferWi - 1);
            int y = buffer.randInt(0, BufferHt - 1);
            buffer.GetMultiColorBlend(buffer.rand01(), false, color);
            buffer.SetTempPixel(x, y, color);
        }
    }
    int effectState = (buffer.curPeriod - buffer.curEffStartPer) * lspeed * buffer.frameTimeInMs / 50;

    long TempState = effectState % 400 / 20;
    if (TempState == cache->LastLifeState) {
        buffer.CopyTempBufToPixels();
        return false;
    }
    cache->LastLifeState = TempState;
    return true;
}

void LifeEffect::RenderLifeGenerationISPC(RenderBuffer& buffer, int type)
{
    int width = buffer.BufferWi;
    int height = buffer.BufferHt;
    if (width < 1 || height < 1) {
        return;
    }
    // Bound writes by the real pixel allocation: a variable sub-buffer can leave
    // GetPixelCount() below BufferWi*BufferHt and the kernel writes result[index]
    // unguarded.
    int npix = std::min<int>(buffer.GetPixelCount(), width * height);
    if (npix < 1) {
        return;
    }

    std::vector<uint32_t> palette;
    BuildLifePalette(buffer, palette);

    ispc::LifeISPCData ld;
    ld.width = width;
    ld.height = height;
    ld.npix = npix;
    ld.type = type;
    ld.numColors = (int)palette.size();
    ld.frameSeed = buffer.hashRandomFrameSeed();

    // The kernel reads the previous generation (tempbuf) and writes the new
    // generation (pixels); the two buffers are distinct so lanes never read a
    // half-written cell. State advances by copying pixels back to tempbuf after.
    const uint32_t* prev = reinterpret_cast<const uint32_t*>(buffer.GetTempBuf());
    uint32_t* result = reinterpret_cast<uint32_t*>(buffer.GetPixels());
    const uint32_t* pal = palette.data();

    constexpr int lifeBlockSize = 4096;
    int blocks = npix / lifeBlockSize + 1;
    parallel_for(0, blocks, [&ld, prev, pal, result, npix](int block) {
        int start = block * lifeBlockSize;
        int end = std::min(start + lifeBlockSize, npix);
        if (start < end) {
            ispc::LifeEffectISPC(&ld, start, end, prev, pal, result);
        }
    });

    // Snapshot the clean generation for the next frame before the DMX routing
    // below repurposes pixelVector entries as channel values.
    buffer.CopyPixelsToTempBuf();

    if (buffer.dmx_buffer) {
        // DMX fixtures need the colour routed through SetPixel(); the raw uint32_t
        // write above bypasses the DMX channel mapping.
        buffer.SetPixel(0, 0, buffer.GetPixel(0, 0));
    }
}

void LifeEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    int Type = 0;
    if (!PrepareLifeGeneration(buffer, SettingsMap, Type)) {
        return;
    }
    RenderLifeGenerationISPC(buffer, Type);
}
