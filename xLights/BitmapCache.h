//
//  BitmapCache.h
//  xLights
//

#ifndef xLights_BitmapCache_h
#define xLights_BitmapCache_h

#include "wx/bitmap.h"

class BitmapCache {
public:

    enum RGB_EFFECTS_e {
        eff_OFF,
        eff_ON,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_CURTAIN,
        eff_DMX,
        eff_FACES,
        eff_FAN,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GALAXY,
        eff_GARLANDS,
        eff_GLEDIATOR,
        eff_LIFE,
        eff_LIGHTNING,
        eff_MARQUEE,
        eff_METEORS,
        eff_MORPH,
        eff_PIANO,
        eff_PICTURES,
        eff_PINWHEEL,
        eff_PLASMA,
        eff_RIPPLE,
        eff_SHIMMER,
        eff_SHOCKWAVE,
        eff_SINGLESTRAND,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_STROBE,
        eff_TEXT,
        eff_TREE,
        eff_TWINKLE,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };

    static const wxBitmap &GetEffectIcon(int effectID, wxString &toolTip, int size = 16, bool exact = false);
    static const wxBitmap &GetToolbarIcon(const wxString &name, int size);
    static const wxBitmap &GetPapgayoIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetCornerIcon(int position, wxString &toolTip, int size, bool exact);
};


#endif
