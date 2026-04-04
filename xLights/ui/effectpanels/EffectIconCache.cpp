/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectIconCache.h"
#include "../../effects/RenderableEffect.h"
#include "../../ui/shared/utils/BitmapCache.h"

#include <wx/image.h>

std::map<std::pair<int, int>, wxBitmapBundle> EffectIconCache::cache;

static wxBitmapBundle emptyBundle;

static int SizeBucket(int sz) {
    if (sz >= 48) return 48;
    if (sz >= 32) return 32;
    if (sz >= 24) return 24;
    return 16;
}

void EffectIconCache::BuildBundles(const RenderableEffect* effect) {
    static constexpr int sizes[] = {16, 24, 32, 48, 64};
    wxVector<wxBitmap> bitmaps;

    for (int i = 0; i < 5; i++) {
        const char* const* data = effect->GetIconData(i);
        wxImage image(data);
        if (image.GetHeight() != sizes[i]) {
            wxImage scaled = image.Scale(sizes[i], sizes[i], wxIMAGE_QUALITY_HIGH);
            bitmaps.push_back(wxBitmap(scaled));
        } else {
            bitmaps.push_back(wxBitmap(image));
        }
    }

    // If the original 64px xpm was larger than 64, keep the original too
    {
        const char* const* data64 = effect->GetIconData(4);
        wxImage image(data64);
        if (image.GetHeight() > 64) {
            bitmaps.push_back(wxBitmap(image));
        }
    }

    int effectId = effect->GetId();
    const std::string& name = effect->Name();
    cache[{effectId, 16}] = wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(name, 16, bitmaps));
    cache[{effectId, 24}] = wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(name, 24, bitmaps));
    cache[{effectId, 32}] = wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(name, 32, bitmaps));
    cache[{effectId, 48}] = wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(name, 48, bitmaps));
}

const wxBitmapBundle& EffectIconCache::GetEffectIcon(const RenderableEffect* effect, int defSize) {
    if (!effect) {
        return emptyBundle;
    }

    int bucket = SizeBucket(defSize);
    int effectId = effect->GetId();
    auto key = std::make_pair(effectId, bucket);

    auto it = cache.find(key);
    if (it == cache.end()) {
        BuildBundles(effect);
        it = cache.find(key);
    }
    return it->second;
}

void EffectIconCache::Clear() {
    cache.clear();
}
