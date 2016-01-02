#include "RenderableEffect.h"
#include "../BitmapCache.h"

RenderableEffect::RenderableEffect(int i, std::string n) : id(i), name(n)
{
    //ctor
}

RenderableEffect::~RenderableEffect()
{
    //dtor
}

const wxBitmap &RenderableEffect::GetEffectIcon(int size, bool exact) const {
    wxString tt;
    return BitmapCache::GetEffectIcon(id, tt, size, exact);
}
