//
//  BitmapCache.cpp
//  xLights
//
//  Created by Daniel Kulp on 4/6/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#include <map>
#include "BitmapCache.h"

//#include "../include/bars.xpm"
#include "../include/bars-16.xpm"
#include "../include/bars-24.xpm"
#include "../include/bars-32.xpm"
#include "../include/bars-48.xpm"
#include "../include/bars-64.xpm"

//#include "../include/butterfly.xpm"
#include "../include/butterfly-16.xpm"
#include "../include/butterfly-24.xpm"
#include "../include/butterfly_32.xpm"
#include "../include/butterfly-48.xpm"
#include "../include/butterfly-64.xpm"

//#include "../include/circles.xpm"
#include "../include/circles-16.xpm"
#include "../include/circles-24.xpm"
#include "../include/circles-32.xpm"
#include "../include/circles-48.xpm"
#include "../include/circles-64.xpm"

#include "../include/ColorWash.xpm"
#include "../include/corofaces.xpm"

//#include "../include/curtain.xpm"
#include "../include/curtain-16.xpm"
#include "../include/curtain-24.xpm"
#include "../include/curtain-32.xpm"
#include "../include/curtain-48.xpm"
#include "../include/curtain-64.xpm"

#include "../include/faces.xpm"

//#include "../include/fire.xpm"
#include "../include/fire-16.xpm"
#include "../include/fire-24.xpm"
#include "../include/fire-32.xpm"
#include "../include/fire-48.xpm"
#include "../include/fire-64.xpm"


//#include "../include/fireworks.xpm"
#include "../include/fireworks-16.xpm"
#include "../include/fireworks-24.xpm"
#include "../include/fireworks-32.xpm"
#include "../include/fireworks-48.xpm"

//#include "../include/garlands.xpm"
#include "../include/garlands-16.xpm"
#include "../include/garlands-24.xpm"
#include "../include/garlands-32.xpm"
#include "../include/garlands-48.xpm"

// Galaxy Icons
#include "../include/galaxy-16.xpm"
#include "../include/galaxy-24.xpm"
#include "../include/galaxy-32.xpm"
#include "../include/galaxy-48.xpm"
#include "../include/galaxy-64.xpm"

#include "../include/glediator.xpm"
//#include "../include/life.xpm"
#include "../include/life-16.xpm"
#include "../include/life-24.xpm"
#include "../include/life-32.xpm"
#include "../include/life-48.xpm"

//#include "../include/meteors.xpm"
#include "../include/meteors-16.xpm"
#include "../include/meteors-24.xpm"
#include "../include/meteors-32.xpm"
#include "../include/meteors-48.xpm"

#include "../include/morph.xpm"

#include "../include/Off.xpm"
#include "../include/On.xpm"
#include "../include/piano.xpm"

//#include "../include/pictures.xpm"
#include "../include/pictures-16.xpm"
#include "../include/pictures-24.xpm"
#include "../include/pictures-32.xpm"
#include "../include/pictures-48.xpm"

//#include "../include/pinwheel.xpm"
#include "../include/pinwheel-16.xpm"
#include "../include/pinwheel-24.xpm"
#include "../include/pinwheel-32.xpm"
#include "../include/pinwheel-48.xpm"


//#include "../include/ripple.xpm"
#include "../include/ripple-16.xpm"
#include "../include/ripple-24.xpm"
#include "../include/ripple-32.xpm"
#include "../include/ripple-48.xpm"

#include "../include/shimmer.xpm"
#include "../include/singleStrand.xpm"

//#include "../include/snowflakes.xpm"
#include "../include/snowflakes-16.xpm"
#include "../include/snowflakes-24.xpm"
#include "../include/snowflakes-32.xpm"
#include "../include/snowflakes-48.xpm"

//#include "../include/snowstorm.xpm"
#include "../include/snowstorm-16.xpm"
#include "../include/snowstorm-24.xpm"
#include "../include/snowstorm-32.xpm"
#include "../include/snowstorm-48.xpm"

//#include "../include/spirals.xpm"
#include "../include/spirals-16.xpm"
#include "../include/spirals-24.xpm"
#include "../include/spirals-32.xpm"
#include "../include/spirals-48.xpm"

//#include "../include/spirograph.xpm"
#include "../include/spirograph-16.xpm"
#include "../include/spirograph-24.xpm"
#include "../include/spirograph-32.xpm"
#include "../include/spirograph-48.xpm"

#include "../include/strobe.xpm"

//#include "../include/text.xpm"
#include "../include/text-16.xpm"
#include "../include/text-24.xpm"
#include "../include/text-32.xpm"
#include "../include/text-48.xpm"

//#include "../include/tree.xpm"
#include "../include/tree-16.xpm"
#include "../include/tree-24.xpm"
#include "../include/tree-32.xpm"
#include "../include/tree-48.xpm"

//#include "../include/twinkle.xpm"
#include "../include/twinkle-16.xpm"
#include "../include/twinkle-24.xpm"
#include "../include/twinkle-32.xpm"
#include "../include/twinkle-48.xpm"

//#include "../include/wave.xpm"
#include "../include/wave-16.xpm"
#include "../include/wave-24.xpm"
#include "../include/wave-32.xpm"
#include "../include/wave-48.xpm"


/*-----  Toolbar Art ----- */
#include "../include/stop_sign_24.xpm"
#include "../include/output_lights_24.xpm"
#include "../include/effects.xpm"
#include "../include/colors.xpm"
#include "../include/layers.xpm"
#include "../include/model_preview_24.xpm"
#include "../include/house_preview_24.xpm"
#include "../include/lights_off_24.xpm"
#include "../include/graceful_stop_24.xpm"
#include "../include/zoom-out-24.xpm"
#include "../include/zoom-in-24.xpm"
#include "../include/settings-24.xpm"

#include "../include/green-gear-24.xpm"
#include "../include/select_show_folder-24.xpm"
#include "../include/save-24.xpm"
#include "../include/save-as-24.xpm"
#include "../include/folder.xpm"
#include "../include/file_new-24.xpm"

#include "../include/stop-24.xpm"
#include "../include/stop-24_off.xpm"
#include "../include/trash-24.xpm"
#include "../include/search-24.xpm"
#include "../include/replay-24.xpm"
#include "../include/play-24.xpm"
#include "../include/play-24_off.xpm"
#include "../include/pause-24.xpm"
#include "../include/pause-24_off.xpm"
#include "../include/move-24.xpm"
#include "../include/home-24.xpm"
#include "../include/gears-24.xpm"
#include "../include/forward-24.xpm"
#include "../include/forward-24_off.xpm"
#include "../include/eye-open-24.xpm"
#include "../include/backward-24.xpm"
#include "../include/backward-24_off.xpm"


#include "wx/artprov.h"

#ifdef __WXOSX__
double xlOSXGetMainScreenContentScaleFactor();
#endif


class xlArtProvider : public wxArtProvider {
public:
    xlArtProvider() {}
    virtual ~xlArtProvider() {};

    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size);
    virtual wxIconBundle CreateIconBundle(const wxArtID& id,
                                          const wxArtClient& client);

};

class EffectBitmapCache {
public:
    EffectBitmapCache() {
        wxArtProvider::PushBack(new xlArtProvider());
    }
    const wxBitmap &get(int size, bool exact,
                        const wxString &eff,
                        const char **data) {
        return get(size, exact, eff, data, data, data, data, data);
    }

    const wxBitmap &get(int size, bool exact,
                        const wxString &eff,
                        const char **data16,
                        const char **data24,
                        const char **data32,
                        const char **data48,
                        const char **data64) {

        std::map<wxString, wxBitmap> *data = &size16;
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
        } else if (size <= 48) {
            data = &size48;
            size = 48;
            dc = data48;
        } else {
            data = &size64;
            size = 64;
            dc = data64;
        }

#ifdef __WXOSX__
        double scale = 1.0;
        //Retina Display, use the larger icons with the scale factor set
        if (exact) {
            if (size == 16) {
                data = &size16e;
            } else if (size == 24) {
                data = &size24e;
            } else if (size == 32) {
                data = &size32e;
            }
        } else if (xlOSXGetMainScreenContentScaleFactor() > 1.9) {
            if (size == 16 && (data16 != data32)) {
                size = 32;
                scale = 2.0;
                dc = data32;
            } else if (size == 24 && (data24 != data48)) {
                size = 48;
                scale = 2.0;
                dc = data48;
            } else if (size == 32 && (data64 != data48) && (data64 != data32)) {
                size = 64;
                scale = 2.0;
                dc = data64;
            }
        }
        const wxBitmap &bmp = (*data)[eff];
        if (!bmp.IsOk()) {
            wxImage image(dc);
            if (image.GetHeight() == size) {
                (*data)[eff] = wxBitmap(image, -1, scale);
            } else {
                wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
                (*data)[eff] = wxBitmap(scaled, -1, scale);
            }
        }
#else
        const wxBitmap &bmp = (*data)[eff];
        if (!bmp.IsOk()) {
            wxImage image(dc);
            if (image.GetHeight() == size) {
                (*data)[eff] = wxBitmap(image);
            } else {
                wxImage scaled = image.Scale(size, size, wxIMAGE_QUALITY_HIGH);
                (*data)[eff] = wxBitmap(scaled);
            }
        }
#endif
        return (*data)[eff];
    }

    std::map<wxString, wxBitmap> size16;
    std::map<wxString, wxBitmap> size24;
    std::map<wxString, wxBitmap> size32;
    std::map<wxString, wxBitmap> size48;
    std::map<wxString, wxBitmap> size64;

#ifdef __WXOSX__
    std::map<wxString, wxBitmap> size16e;
    std::map<wxString, wxBitmap> size24e;
    std::map<wxString, wxBitmap> size32e;
#endif
} effectBitmaps;



wxBitmap xlArtProvider::CreateBitmap(const wxArtID& id,
                                     const wxArtClient& client,
                                     const wxSize& size) {
    if ("xlART_STOP_NOW" == id) {
        return effectBitmaps.get(24, false, id, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm);
    } else if ("xlART_OUTPUT_LIGHTS" == id) {
        return effectBitmaps.get(24, false, id, output_lights_24_xpm, output_lights_24_xpm, output_lights_24_xpm, output_lights_24_xpm, output_lights_24_xpm);
    } else if ("xlART_EFFECTS" == id) {
        return effectBitmaps.get(24, false, id, effects_xpm, effects_xpm, effects_xpm, effects_xpm, effects_xpm);
    } else if ("xlART_COLORS" == id) {
        return effectBitmaps.get(24, false, id, colors_xpm, colors_xpm, colors_xpm, colors_xpm, colors_xpm);
    } else if ("xlART_LAYERS" == id) {
        return effectBitmaps.get(24, false, id, layers_xpm, layers_xpm, layers_xpm, layers_xpm, layers_xpm);
    } else if ("xlART_MODEL_PREVIEW" == id) {
        return effectBitmaps.get(24, false, id, model_preview_xpm, model_preview_xpm, model_preview_xpm, model_preview_xpm, model_preview_xpm);
    } else if ("xlART_HOUSE_PREVIEW" == id) {
        return effectBitmaps.get(24, false, id, house_preview_xpm, house_preview_xpm, house_preview_xpm, house_preview_xpm, house_preview_xpm);
    } else if ("xlART_GRACEFUL_STOP" == id) {
        return effectBitmaps.get(24, false, id, graceful_stop_24_xpm, graceful_stop_24_xpm, graceful_stop_24_xpm, graceful_stop_24_xpm, graceful_stop_24_xpm);
    } else if ("xlART_LIGHTS_OFF" == id) {
        return effectBitmaps.get(24, false, id, lights_off_24_xpm, lights_off_24_xpm, lights_off_24_xpm, lights_off_24_xpm, lights_off_24_xpm);
    } else if ("xlART_ZOOM_IN" == id) {
        return effectBitmaps.get(24, false, id, zoom_in_24_xpm, zoom_in_24_xpm, zoom_in_24_xpm, zoom_in_24_xpm, zoom_in_24_xpm);
    } else if ("xlART_ZOOM_OUT" == id) {
        return effectBitmaps.get(24, false, id, zoom_out_24_xpm, zoom_out_24_xpm, zoom_out_24_xpm, zoom_out_24_xpm, zoom_out_24_xpm);
    } else if ("xlART_SETTINGS" == id) {
        return effectBitmaps.get(24, false, id, settings_24_xpm, settings_24_xpm, settings_24_xpm, settings_24_xpm, settings_24_xpm);
    } else if ("xlART_PLAY" == id) {
        return effectBitmaps.get(24, false, id, play_24_xpm, play_24_xpm, play_24_xpm, play_24_xpm, play_24_xpm);
    } else if ("xlART_PAUSE" == id) {
        return effectBitmaps.get(24, false, id, pause_24_xpm, pause_24_xpm, pause_24_xpm, pause_24_xpm, pause_24_xpm);
    } else if ("xlART_BACKWARD" == id) {
        return effectBitmaps.get(24, false, id, backward_24_xpm, backward_24_xpm, backward_24_xpm, backward_24_xpm, backward_24_xpm);
    } else if ("xlART_FORWARD" == id) {
        return effectBitmaps.get(24, false, id, forward_24_xpm, forward_24_xpm, forward_24_xpm, forward_24_xpm, forward_24_xpm);
    } else if ("xlART_REPLAY" == id) {
        return effectBitmaps.get(24, false, id, replay_24_xpm, replay_24_xpm, replay_24_xpm, replay_24_xpm, replay_24_xpm);
    } else if ("xlART_STOP" == id) {
        return effectBitmaps.get(24, false, id, stop_24_xpm, stop_24_xpm, stop_24_xpm, stop_24_xpm, stop_24_xpm);
#ifndef __WXOSX__
    //don't use these on OSX as the OSX supplied Icons look MUCH better and more inline with expectations on a Mac
    } else if ("xlART_RENDER_ALL" == id) {
        return effectBitmaps.get(24, false, id, green_gear_24_xpm, green_gear_24_xpm, green_gear_24_xpm, green_gear_24_xpm, green_gear_24_xpm);
    } else if (wxART_FOLDER_OPEN == id) {
        return effectBitmaps.get(24, false, id, select_show_folder_24_xpm, select_show_folder_24_xpm, select_show_folder_24_xpm, select_show_folder_24_xpm, select_show_folder_24_xpm);
    } else if (wxART_NEW == id) {
        return effectBitmaps.get(24, false, id, file_new_24_xpm, file_new_24_xpm, file_new_24_xpm, file_new_24_xpm, file_new_24_xpm);
    } else if (wxART_FILE_OPEN == id) {
        return effectBitmaps.get(24, false, id, folder_xpm, folder_xpm, folder_xpm, folder_xpm, folder_xpm);
    } else if (wxART_FILE_SAVE == id) {
        return effectBitmaps.get(24, false, id, save_24_xpm, save_24_xpm, save_24_xpm, save_24_xpm, save_24_xpm);
    } else if (wxART_FILE_SAVE_AS == id) {
        return effectBitmaps.get(24, false, id, save_as_24_xpm, save_as_24_xpm, save_as_24_xpm, save_as_24_xpm, save_as_24_xpm);
#else
    } else if ("xlART_RENDER_ALL" == id) {
        return wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FIND_AND_REPLACE")), wxART_OTHER);
    } else if (wxART_TOOLBAR == client) {
        return wxArtProvider::GetBitmap(id, wxART_OTHER);
#endif
    }
    printf("bmp:  %s   %s  %dx%d\n", (const char *)id.c_str(), (const char*)client.c_str(), size.x, size.y);
    return wxNullBitmap;
}
wxIconBundle xlArtProvider::CreateIconBundle(const wxArtID& id,
                                              const wxArtClient& client)
{
    printf("ib:  %s   %s \n", (const char *)id.c_str(), (const char*)client.c_str());
    return wxNullIconBundle;
}



static const wxBitmap tmp;
const wxBitmap &BitmapCache::GetToolbarIcon(const wxString &name, int size) {
    return tmp;
}

const wxBitmap &BitmapCache::GetEffectIcon(int effectID, wxString &toolTip, int size, bool exact) {
    switch(effectID)
    {
        case BitmapCache::RGB_EFFECTS_e::eff_OFF:
            toolTip = "Off";
            return effectBitmaps.get(size, exact, "Off", Off, Off, Off, Off, Off);
        case BitmapCache::RGB_EFFECTS_e::eff_ON:
            toolTip = "On";
            return effectBitmaps.get(size, exact, "On", On, On, On, On, On);
        case BitmapCache::RGB_EFFECTS_e::eff_BARS:
            toolTip = "Bars";
            return effectBitmaps.get(size, exact, "Bars", bars_16, bars_24, bars_32, bars_48, bars_64);
        case BitmapCache::RGB_EFFECTS_e::eff_BUTTERFLY:
            toolTip = "Butterfly";
            return effectBitmaps.get(size, exact, "Butterfly", butterfly_16, butterfly_24, butterfly_32, butterfly_48, butterfly_64);
        case BitmapCache::RGB_EFFECTS_e::eff_CIRCLES:
            toolTip = "Circles";
            return effectBitmaps.get(size, exact, "Circles", circles_16, circles_24, circles_32, circles_48, circles_64);
        case BitmapCache::RGB_EFFECTS_e::eff_COLORWASH:
            toolTip = "ColorWash";
            return effectBitmaps.get(size, exact, "ColorWash", ColorWash, ColorWash, ColorWash, ColorWash, ColorWash);
        case BitmapCache::RGB_EFFECTS_e::eff_COROFACES:
            toolTip = "Coro Faces";
            return effectBitmaps.get(size, exact, "Coro Faces", corofaces, corofaces, corofaces, corofaces, corofaces);
        case BitmapCache::RGB_EFFECTS_e::eff_CURTAIN:
            toolTip = "Curtain";
            return effectBitmaps.get(size, exact, "Curtain", curtain_16, curtain_24, curtain_32, curtain_48, curtain_64);
        case BitmapCache::RGB_EFFECTS_e::eff_FACES:
            toolTip = "Matrix Faces";
            return effectBitmaps.get(size, exact, "Matrix Faces", faces, faces, faces, faces, faces);
        case BitmapCache::RGB_EFFECTS_e::eff_FIRE:
            toolTip = "Fire";
            return effectBitmaps.get(size, exact, "Fire", fire_16, fire_24, fire_32, fire_48, fire_64);
        case BitmapCache::RGB_EFFECTS_e::eff_FIREWORKS:
            toolTip = "Fireworks";
            return effectBitmaps.get(size, exact, "Fireworks", fireworks_16, fireworks_24, fireworks_32, fireworks_48, fireworks_48);
        case BitmapCache::RGB_EFFECTS_e::eff_GALAXY:
            toolTip = "Galaxy";
            return effectBitmaps.get(size, exact, "Galaxy", galaxy_16, galaxy_24, galaxy_32, galaxy_48, galaxy_64);
        case BitmapCache::RGB_EFFECTS_e::eff_GARLANDS:
            toolTip = "Garlands";
            return effectBitmaps.get(size, exact, "Garlands", garlands_16, garlands_24, garlands_32, garlands_48, garlands_48);
        case BitmapCache::RGB_EFFECTS_e::eff_GLEDIATOR:
            toolTip = "Glediator";
            return effectBitmaps.get(size, exact, "Glediator", glediator, glediator, glediator, glediator, glediator);
        case BitmapCache::RGB_EFFECTS_e::eff_LIFE:
            toolTip = "Life";
            return effectBitmaps.get(size, exact, "Life", life_16, life_24, life_32, life_48, life_48);
        case BitmapCache::RGB_EFFECTS_e::eff_METEORS:
            toolTip = "Meteors";
            return effectBitmaps.get(size, exact, "Meteors", meteors_16, meteors_24, meteors_32, meteors_48, meteors_48);
        case BitmapCache::RGB_EFFECTS_e::eff_MORPH:
            toolTip = "Morph";
            return effectBitmaps.get(size, exact, "Morph", morph, morph, morph, morph, morph);
        case BitmapCache::RGB_EFFECTS_e::eff_PIANO:
            toolTip = "Piano";
            return effectBitmaps.get(size, exact, "Piano", piano, piano, piano, piano, piano);
        case BitmapCache::RGB_EFFECTS_e::eff_PICTURES:
            toolTip = "Pictures";
            return effectBitmaps.get(size, exact, "Pictures", pictures_16, pictures_24, pictures_32, pictures_48, pictures_48);
        case BitmapCache::RGB_EFFECTS_e::eff_PINWHEEL:
            toolTip = "Pinwheel";
            return effectBitmaps.get(size, exact, "Pinwheel", pinwheel_16, pinwheel_24, pinwheel_32, pinwheel_48, pinwheel_48);
        case BitmapCache::RGB_EFFECTS_e::eff_RIPPLE:
            toolTip = "Ripple";
            return effectBitmaps.get(size, exact, "Ripple", ripple_16, ripple_24, ripple_32, ripple_48, ripple_48);
        case BitmapCache::RGB_EFFECTS_e::eff_SHIMMER:
            toolTip = "Shimmer";
            return effectBitmaps.get(size, exact, "Shimmer", shimmer, shimmer, shimmer, shimmer, shimmer);
        case BitmapCache::RGB_EFFECTS_e::eff_SINGLESTRAND:
            toolTip = "Single Strand";
            return effectBitmaps.get(size, exact, "Single Strand", singleStrand, singleStrand, singleStrand, singleStrand, singleStrand);
        case BitmapCache::RGB_EFFECTS_e::eff_SNOWFLAKES:
            toolTip = "Snow Flakes";
            return effectBitmaps.get(size, exact, "Snow Flakes", snowflakes_16, snowflakes_24, snowflakes_32, snowflakes_48, snowflakes_48);
        case BitmapCache::RGB_EFFECTS_e::eff_SNOWSTORM:
            toolTip = "Snow Storm";
            return effectBitmaps.get(size, exact, "Snow Storm", snowstorm_16, snowstorm_24, snowstorm_32, snowstorm_48, snowstorm_48);
        case BitmapCache::RGB_EFFECTS_e::eff_SPIRALS:
            toolTip = "Spirals";
            return effectBitmaps.get(size, exact, "Sprials", spirals_16, spirals_24, spirals_32, spirals_48, spirals_48);
        case BitmapCache::RGB_EFFECTS_e::eff_SPIROGRAPH:
            toolTip = "Spirograph";
            return effectBitmaps.get(size, exact, "Spirograph", spirograph_16, spirograph_24, spirograph_32, spirograph_48, spirograph_48);
        case BitmapCache::RGB_EFFECTS_e::eff_STROBE:
            toolTip = "Strobe";
            return effectBitmaps.get(size, exact, "Strobe", strobe, strobe, strobe, strobe, strobe);
        case BitmapCache::RGB_EFFECTS_e::eff_TEXT:
            toolTip = "Text";
            return effectBitmaps.get(size, exact, "Text", text_16, text_24, text_32, text_48, text_48);
        case BitmapCache::RGB_EFFECTS_e::eff_TREE:
            toolTip = "Tree";
            return effectBitmaps.get(size, exact, "Tree", tree_16, tree_24, tree_32, tree_48, tree_48);
        case BitmapCache::RGB_EFFECTS_e::eff_TWINKLE:
            toolTip = "Twinkle";
            return effectBitmaps.get(size, exact, "Twinkle", twinkle_16, twinkle_24, twinkle_32, twinkle_48, twinkle_48);
        case BitmapCache::RGB_EFFECTS_e::eff_WAVE:
            toolTip = "Wave";
            return effectBitmaps.get(size, exact, "Wave", wave_16, wave_24, wave_32, wave_48, wave_48);
        default:
            break;
    }
    toolTip = "Off";
    return effectBitmaps.get(size, exact, "Off", Off, Off, Off, Off, Off);
}

