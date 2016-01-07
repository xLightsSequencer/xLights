#include "RenderableEffect.h"
#include "../BitmapCache.h"
#include "../sequencer/Effect.h"

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
int RenderableEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2) {
    if (e->HasBackgroundDisplayList()) {
        DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList());
        return e->GetBackgroundDisplayList().iconSize;
    }
    return 1;
}
