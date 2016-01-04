#include "RenderableEffect.h"
#include "../BitmapCache.h"

RenderableEffect::RenderableEffect(int i, std::string n) : id(i), name(n), panel(nullptr), mSequenceElements(nullptr)
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


wxPanel *RenderableEffect::GetPanel(wxWindow *parent) {
    if (panel == nullptr) {
        panel = CreatePanel(parent);
    }
    return panel;
}
