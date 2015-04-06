//
//  BitmapCache.cpp
//  xLights
//
//  Created by Daniel Kulp on 4/6/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#include "xLightsMain.h"


#include "../include/bars.xpm"
#include "../include/butterfly.xpm"
#include "../include/circles.xpm"
#include "../include/ColorWash.xpm"
#include "../include/corofaces.xpm"
#include "../include/curtain.xpm"
#include "../include/faces.xpm"
#include "../include/fire.xpm"
#include "../include/fireworks.xpm"
#include "../include/garlands.xpm"
#include "../include/glediator.xpm"
#include "../include/life.xpm"
#include "../include/meteors.xpm"
#include "../include/morph.xpm"

#include "../include/Off.xpm"
#include "../include/On.xpm"
#include "../include/piano.xpm"
#include "../include/pictures.xpm"

#include "../include/pinwheel.xpm"
#include "../include/ripple.xpm"
#include "../include/shimmer.xpm"
#include "../include/singleStrand.xpm"
#include "../include/snowflakes.xpm"
#include "../include/snowstorm.xpm"
#include "../include/spirals.xpm"
#include "../include/spirograph.xpm"
#include "../include/strobe.xpm"
#include "../include/text.xpm"
#include "../include/tree.xpm"
#include "../include/twinkle.xpm"
#include "../include/wave.xpm"


class EffectBitmapCache {
public:
    EffectBitmapCache() {
    }
    
    const wxBitmap &get(int size,
                        int eff,
                        const char **data16,
                        const char **data24,
                        const char **data32,
                        const char **data48) {
        
        std::map<int, wxBitmap> *data = &size16;
        const char ** dc = data16;
        if (size <= 16) {
            data = &size16;
            size = 16;
            dc = data16;
        } else if (size <= 24) {
            data = &size24;
            size = 24;
            dc = data24;
        } else if (size <= 32) {
            data = &size32;
            size = 32;
            dc = data32;
        } else {
            data = &size48;
            size = 48;
            dc = data48;
        }
        
        
        const wxBitmap &bmp = (*data)[eff];
        if (!bmp.IsOk()) {
            wxImage image(dc);
            if (image.GetSize() == wxSize(size, size)) {
                (*data)[eff] = wxBitmap(image);
            } else {
                wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
                (*data)[eff] = wxBitmap(scaled);
            }
        }
        return (*data)[eff];
    }
    
    std::map<int, wxBitmap> size16;
    std::map<int, wxBitmap> size24;
    std::map<int, wxBitmap> size32;
    std::map<int, wxBitmap> size48;
    
} effectBitmaps;


const wxBitmap &xLightsFrame::GetIcon(int effectID, wxString &toolTip, int size)
{
    switch(effectID)
    {
        case xLightsFrame::RGB_EFFECTS_e::eff_OFF:
            toolTip = "Off";
            return effectBitmaps.get(size, effectID, Off, Off, Off, Off);
        case xLightsFrame::RGB_EFFECTS_e::eff_ON:
            toolTip = "On";
            return effectBitmaps.get(size, effectID, On, On, On, On);
        case xLightsFrame::RGB_EFFECTS_e::eff_BARS:
            toolTip = "Bars";
            return effectBitmaps.get(size, effectID, bars, bars, bars, bars);
        case xLightsFrame::RGB_EFFECTS_e::eff_BUTTERFLY:
            toolTip = "Butterfly";
            return effectBitmaps.get(size, effectID, butterfly, butterfly, butterfly, butterfly);
        case xLightsFrame::RGB_EFFECTS_e::eff_CIRCLES:
            toolTip = "Circles";
            return effectBitmaps.get(size, effectID, circles, circles, circles, circles);
        case xLightsFrame::RGB_EFFECTS_e::eff_COLORWASH:
            toolTip = "ColorWash";
            return effectBitmaps.get(size, effectID, ColorWash, ColorWash, ColorWash, ColorWash);
        case xLightsFrame::RGB_EFFECTS_e::eff_COROFACES:
            toolTip = "Coro Faces";
            return effectBitmaps.get(size, effectID, corofaces, corofaces, corofaces, corofaces);
        case xLightsFrame::RGB_EFFECTS_e::eff_CURTAIN:
            toolTip = "Curtain";
            return effectBitmaps.get(size, effectID, curtain, curtain, curtain, curtain);
        case xLightsFrame::RGB_EFFECTS_e::eff_FACES:
            toolTip = "Matrix Faces";
            return effectBitmaps.get(size, effectID, faces, faces, faces, faces);
        case xLightsFrame::RGB_EFFECTS_e::eff_FIRE:
            toolTip = "Fire";
            return effectBitmaps.get(size, effectID, fire, fire, fire, fire);
        case xLightsFrame::RGB_EFFECTS_e::eff_FIREWORKS:
            toolTip = "Fireworks";
            return effectBitmaps.get(size, effectID, fireworks, fireworks, fireworks, fireworks);
        case xLightsFrame::RGB_EFFECTS_e::eff_GARLANDS:
            toolTip = "Garlands";
            return effectBitmaps.get(size, effectID, garlands, garlands, garlands, garlands);
        case xLightsFrame::RGB_EFFECTS_e::eff_GLEDIATOR:
            toolTip = "Glediator";
            return effectBitmaps.get(size, effectID, glediator, glediator, glediator, glediator);
        case xLightsFrame::RGB_EFFECTS_e::eff_LIFE:
            toolTip = "Life";
            return effectBitmaps.get(size, effectID, life, life, life, life);
        case xLightsFrame::RGB_EFFECTS_e::eff_METEORS:
            toolTip = "Meteors";
            return effectBitmaps.get(size, effectID, meteors, meteors, meteors, meteors);
        case xLightsFrame::RGB_EFFECTS_e::eff_MORPH:
            toolTip = "Morph";
            return effectBitmaps.get(size, effectID, morph, morph, morph, morph);
        case xLightsFrame::RGB_EFFECTS_e::eff_PIANO:
            toolTip = "Piano";
            return effectBitmaps.get(size, effectID, piano, piano, piano, piano);
        case xLightsFrame::RGB_EFFECTS_e::eff_PICTURES:
            toolTip = "Pictures";
            return effectBitmaps.get(size, effectID, pictures, pictures, pictures, pictures);
        case xLightsFrame::RGB_EFFECTS_e::eff_PINWHEEL:
            toolTip = "Pinwheel";
            return effectBitmaps.get(size, effectID, pinwheel, pinwheel, pinwheel, pinwheel);
        case xLightsFrame::RGB_EFFECTS_e::eff_RIPPLE:
            toolTip = "Ripple";
            return effectBitmaps.get(size, effectID, ripple, ripple, ripple, ripple);
        case xLightsFrame::RGB_EFFECTS_e::eff_SHIMMER:
            toolTip = "Shimmer";
            return effectBitmaps.get(size, effectID, shimmer, shimmer, shimmer, shimmer);
        case xLightsFrame::RGB_EFFECTS_e::eff_SINGLESTRAND:
            toolTip = "Single Strand";
            return effectBitmaps.get(size, effectID, singleStrand, singleStrand, singleStrand, singleStrand);
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWFLAKES:
            toolTip = "Snow Flakes";
            return effectBitmaps.get(size, effectID, snowflakes, snowflakes, snowflakes, snowflakes);
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWSTORM:
            toolTip = "Snow Storm";
            return effectBitmaps.get(size, effectID, snowstorm, snowstorm, snowstorm, snowstorm);
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIRALS:
            toolTip = "Spirals";
            return effectBitmaps.get(size, effectID, spirals, spirals, spirals, spirals);
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIROGRAPH:
            toolTip = "Spirograph";
            return effectBitmaps.get(size, effectID, spirograph, spirograph, spirograph, spirograph);
        case xLightsFrame::RGB_EFFECTS_e::eff_STROBE:
            toolTip = "Strobe";
            return effectBitmaps.get(size, effectID, strobe, strobe, strobe, strobe);
        case xLightsFrame::RGB_EFFECTS_e::eff_TEXT:
            toolTip = "Text";
            return effectBitmaps.get(size, effectID, text, text, text, text);
        case xLightsFrame::RGB_EFFECTS_e::eff_TREE:
            toolTip = "Tree";
            return effectBitmaps.get(size, effectID, tree, tree, tree, tree);
        case xLightsFrame::RGB_EFFECTS_e::eff_TWINKLE:
            toolTip = "Twinkle";
            return effectBitmaps.get(size, effectID, twinkle, twinkle, twinkle, twinkle);
        case xLightsFrame::RGB_EFFECTS_e::eff_WAVE:
            toolTip = "Wave";
            return effectBitmaps.get(size, effectID, wave, wave, wave, wave);
        default:
            break;
    }
    toolTip = "Off";
    return effectBitmaps.get(size, effectID, Off, Off, Off, Off);
}

