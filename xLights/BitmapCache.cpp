//
//  BitmapCache.cpp
//  xLights
//
//  Created by Daniel Kulp on 4/6/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#include <map>
#include "BitmapCache.h"

#include "../include/padlock16x16-blue.xpm"
#include "../include/padlock16x16-red.xpm"




/*-----  Toolbar Art ----- */
#include "../include/stop_sign_24.xpm"
#include "../include/output_lights_24.xpm"
#include "../include/output_lights_off_16.xpm"
#include "../include/output_lights_off_24.xpm"
#include "../include/output_lights_off_32.xpm"
#include "../include/output_lights_off_48.xpm"
#include "../include/output_lights_off_64.xpm"

#include "../include/output_lights_on_16.xpm"
#include "../include/output_lights_on_24.xpm"
#include "../include/output_lights_on_32.xpm"
#include "../include/output_lights_on_48.xpm"
#include "../include/output_lights_on_64.xpm"



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

#include "../include/link-48.xpm"
#include "../include/unlink-48.xpm"
#include "../include/models-16.xpm"
#include "../include/models-24.xpm"
#include "../include/models-32.xpm"
#include "../include/models-48.xpm"
#include "../include/models-64.xpm"

#include "../include/point_1a-64.xpm"
#include "../include/point_1b-64.xpm"
#include "../include/point_2a-64.xpm"
#include "../include/point_2b-64.xpm"
#include "../include/point_1ab-64.xpm"
#include "../include/point_2ab-64.xpm"

#include "../include/papagayo-16.xpm"
#include "../include/papagayo-64.xpm"

#include "../include/model-16.xpm"
#include "../include/model-64.xpm"

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
    const wxBitmap &GetOtherImage(const wxString &name,
                                  const char **data) {
        const wxBitmap &bmp = others[name];
        if (!bmp.IsOk()) {
            wxImage image(data);
            others[name] = wxBitmap(image);
        }
        return bmp;
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

    std::map<wxString, wxBitmap> others;
} effectBitmaps;



wxBitmap xlArtProvider::CreateBitmap(const wxArtID& id,
                                     const wxArtClient& client,
                                     const wxSize& size) {
    if ("xlART_STOP_NOW" == id) {
        return effectBitmaps.get(24, false, id, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm, stop_sign_24_xpm);
    } else if ("xlART_OUTPUT_LIGHTS" == id) {
        return effectBitmaps.get(24, false, id, output_lights_off_24, output_lights_off_24, output_lights_off_24, output_lights_off_24, output_lights_off_24);
    } else if ("xlART_OUTPUT_LIGHTS_ON" == id) {
        return effectBitmaps.get(24, false, id, output_lights_on_24, output_lights_on_24, output_lights_on_24, output_lights_on_24, output_lights_on_24);
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
    } else if ("xlART_LINK" == id) {
        return effectBitmaps.get(48, false, id, link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm);
    } else if ("xlART_UNLINK" == id) {
        return effectBitmaps.get(48, false, id, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm);
    } else if ("xlART_MODELS" == id) {
        return effectBitmaps.get(24, true, id, models_16_xpm, models_24_xpm, models_32_xpm, models_48_xpm, models_64_xpm);
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
    //printf("bmp:  %s   %s  %dx%d\n", (const char *)id.c_str(), (const char*)client.c_str(), size.x, size.y);
    return wxNullBitmap;
}
wxIconBundle xlArtProvider::CreateIconBundle(const wxArtID& id,
        const wxArtClient& client) {
    printf("ib:  %s   %s \n", (const char *)id.c_str(), (const char*)client.c_str());
    return wxNullIconBundle;
}




const wxBitmap &BitmapCache::GetPapgayoIcon(wxString &toolTip, int size, bool exact) {
    toolTip = "Papagayo Voice";
    return effectBitmaps.get(size, exact, "Papagayo", papagayo_16, papagayo_64, papagayo_64, papagayo_64, papagayo_64);
}

const wxBitmap &BitmapCache::GetModelGroupIcon(wxString &toolTip, int size, bool exact) {
    toolTip = "Model Group";
    return effectBitmaps.get(size, exact, "ModelGroup", model_16, model_64, model_64, model_64, model_64);
}

const wxBitmap &BitmapCache::GetCornerIcon(int position, wxString &toolTip, int size, bool exact) {
    switch(position) {
    case 0:
        toolTip = "Corner 1a";
        return effectBitmaps.get(size, exact, "Corner1a", point_1a_64, point_1a_64, point_1a_64, point_1a_64, point_1a_64);
    case 1:
        toolTip = "Corner 1b";
        return effectBitmaps.get(size, exact, "Corner1b", point_1b_64, point_1b_64, point_1b_64, point_1b_64, point_1b_64);
    case 2:
        toolTip = "Corner 2a";
        return effectBitmaps.get(size, exact, "Corner2a", point_2a_64, point_2a_64, point_2a_64, point_2a_64, point_2a_64);
    case 3:
        toolTip = "Corner 2b";
        return effectBitmaps.get(size, exact, "Corner2b", point_2b_64, point_2b_64, point_2b_64, point_2b_64, point_2b_64);
    case 4:
        toolTip = "Corner 1ab";
        return effectBitmaps.get(size, exact, "Corner1ab", point_1ab_64, point_1ab_64, point_1ab_64, point_1ab_64, point_1ab_64);
    case 5:
    default:
        toolTip = "Corner 2ab";
        return effectBitmaps.get(size, exact, "Corner2ab", point_2ab_64, point_2ab_64, point_2ab_64, point_2ab_64, point_2ab_64);
    }
}

const wxBitmap &BitmapCache::GetLockIcon(bool locked) {
    if (locked) {
        return effectBitmaps.GetOtherImage("Locked", padlock16x16_red_xpm);
    }
    return effectBitmaps.GetOtherImage("Unlocked", padlock16x16_blue_xpm);
}

