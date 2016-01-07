#include "RenderableEffect.h"
#include "../sequencer/Effect.h"

RenderableEffect::RenderableEffect(int i, std::string n,
                                   const char **data16,
                                   const char **data24,
                                   const char **data32,
                                   const char **data48,
                                   const char **data64)
    : id(i), name(n), panel(nullptr), mSequenceElements(nullptr)
{
    initBitmaps(data16, data24, data32, data48, data64);
}

RenderableEffect::~RenderableEffect()
{
    //dtor
}

#ifdef __WXOSX__
double xlOSXGetMainScreenContentScaleFactor();
#endif

const wxBitmap &RenderableEffect::GetEffectIcon(int size, bool exact) const {
#ifdef __WXOSX__
    if (exact || xlOSXGetMainScreenContentScaleFactor() < 1.9) {
        if (size <= 16) {
            return icon16e;
        } else if (size <= 24) {
            return icon24e;
        } else if (size <= 32) {
            return icon32e;
        }
    }
#endif
    if (size <= 16) {
        return icon16;
    } else if (size <= 24) {
        return icon24;
    } else if (size <= 32) {
        return icon32;
    } else if (size <= 48) {
        return icon48;
    } else {
        return icon64;
    }
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




void AdjustAndSetBitmap(int size, wxImage &image, wxImage &dbl, wxBitmap&bitmap) {
#ifdef __WXOSX__
    if (dbl.GetHeight() == (2 * size)) {
        bitmap = wxBitmap(dbl, -1, 2.0);
    } else if (dbl.GetHeight() > (2*size)) {
        wxImage scaled = image.Scale(size*2, size*2, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled, -1, 2.0);
    } else if (image.GetHeight() == size) {
        bitmap = wxBitmap(image, -1, 1.0);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled, -1, 1.0);
    }
#else
    if (image.GetHeight() == size) {
        bitmap = wxBitmap(image);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled);
    }
#endif
}

void AdjustAndSetBitmap(int size, wxImage &image, wxBitmap&bitmap) {
#ifdef __WXOSX__
    if (image.GetHeight() == size) {
        bitmap = wxBitmap(image, -1, 1.0);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled, -1, 1.0);
    }
#else
    if (image.GetHeight() == size) {
        bitmap = wxBitmap(image);
    } else {
        wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
        bitmap = wxBitmap(scaled);
    }
#endif

}

void RenderableEffect::initBitmaps(const char **data16,
                                   const char **data24,
                                   const char **data32,
                                   const char **data48,
                                   const char **data64) {
    wxImage image16(data16);
    wxImage image24(data24);
    wxImage image32(data32);
    wxImage image48(data48);
    wxImage image64(data64);
    AdjustAndSetBitmap(16, image16, image32, icon16);
    AdjustAndSetBitmap(24, image24, image48, icon24);
    AdjustAndSetBitmap(32, image32, image64, icon32);
    AdjustAndSetBitmap(48, image48, icon48);
    AdjustAndSetBitmap(64, image64, icon64);
#ifdef __WXOSX__
    AdjustAndSetBitmap(16, image16, icon16e);
    AdjustAndSetBitmap(24, image24, icon24e);
    AdjustAndSetBitmap(32, image32, icon32e);
#endif

}
