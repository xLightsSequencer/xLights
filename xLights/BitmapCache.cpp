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
#include "../include/toolbar/toolbarImages.h"

#include "../include/render-all-16.xpm"
#include "../include/render-all-24.xpm"
#include "../include/render-all-32.xpm"
#include "../include/render-all-48.xpm"
#include "../include/render-all-64.xpm"
#include "../include/paste-by-time-24.xpm"
#include "../include/paste-by-cell-24.xpm"
#include "../include/fforward10-24.xpm"
#include "../include/rewind10-24.xpm"


#define paste_by_time_16_xpm paste_by_time_24_xpm
#define paste_by_time_32_xpm paste_by_time_24_xpm
#define paste_by_time_48_xpm paste_by_time_24_xpm
#define paste_by_time_64_xpm paste_by_time_24_xpm
#define paste_by_cell_16_xpm paste_by_cell_24_xpm
#define paste_by_cell_32_xpm paste_by_cell_24_xpm
#define paste_by_cell_48_xpm paste_by_cell_24_xpm
#define paste_by_cell_64_xpm paste_by_cell_24_xpm

#include "../include/select_show_folder-24.xpm"
#include "../include/save-24.xpm"
#include "../include/save-as-24.xpm"
#include "../include/folder.xpm"
#include "../include/file_new-24.xpm"

#include "../include/Dice-24.xpm"
#include "../include/Dice-32.xpm"
#include "../include/Dice-48.xpm"
#include "../include/Dice-64.xpm"

#include "../include/group-open-24.xpm"
#include "../include/group-open-64.xpm"
#include "../include/group-closed-24.xpm"
#include "../include/group-closed-64.xpm"

#include "../include/arch-icon-16.xpm"
#include "../include/arch-icon-64.xpm"
#include "../include/cane-icon-16.xpm"
#include "../include/cane-icon-64.xpm"
#include "../include/channelblock-icon-16.xpm"
#include "../include/channelblock-icon-64.xpm"
#include "../include/circle-icon-16.xpm"
#include "../include/circle-icon-64.xpm"
#include "../include/custom-icon-16.xpm"
#include "../include/custom-icon-64.xpm"
#include "../include/dmx-icon-16.xpm"
#include "../include/dmx-icon-64.xpm"
#include "../include/icicle-icon-16.xpm"
#include "../include/icicle-icon-64.xpm"
#include "../include/line-icon-16.xpm"
#include "../include/line-icon-64.xpm"
#include "../include/matrix-icon-16.xpm"
#include "../include/matrix-icon-64.xpm"
#include "../include/poly-icon-16.xpm"
#include "../include/poly-icon-64.xpm"
#include "../include/spinner-icon-16.xpm"
#include "../include/spinner-icon-64.xpm"
#include "../include/star-icon-16.xpm"
#include "../include/star-icon-64.xpm"
#include "../include/submarine-16.xpm"
#include "../include/submarine-32.xpm"
#include "../include/tree-icon-16.xpm"
#include "../include/tree-icon-64.xpm"
#include "../include/window-icon-16.xpm"
#include "../include/window-icon-64.xpm"
#include "../include/wreath-icon-16.xpm"
#include "../include/wreath-icon-64.xpm"

#include "../include/link-48.xpm"
#include "../include/unlink-48.xpm"

#include "../include/point_1a-64.xpm"
#include "../include/point_1b-64.xpm"
#include "../include/point_2a-64.xpm"
#include "../include/point_2b-64.xpm"
#include "../include/point_1ab-64.xpm"
#include "../include/point_2ab-64.xpm"

#include "../include/papagayo-16.xpm"
#include "../include/papagayo-64.xpm"
#include "../include/papagayo_x-16.xpm"
#include "../include/papagayo_x-64.xpm"

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

        std::map<wxString, wxBitmap> *data;
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
        int origSize = size;
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
            } else if (!exact && image.GetHeight() == origSize*2) {
                (*data)[eff] = wxBitmap(image, -1, 2.0);
            } else if (!exact && image.GetHeight() == origSize*3) {
                (*data)[eff] = wxBitmap(image, -1, 3.0);
            } else if (!exact && image.GetHeight() == origSize*4) {
                (*data)[eff] = wxBitmap(image, -1, 4.0);
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
        return effectBitmaps.get(24, false, id, stop_sign_16_xpm, stop_sign_24_xpm, stop_sign_32_xpm, stop_sign_48_xpm, stop_sign_64_xpm);
    } else if ("xlART_OUTPUT_LIGHTS" == id) {
        return effectBitmaps.get(24, false, id, output_lights_off_16_xpm, output_lights_off_24_xpm, output_lights_off_32_xpm, output_lights_off_48_xpm, output_lights_off_64_xpm);
    } else if ("xlART_OUTPUT_LIGHTS_ON" == id) {
        return effectBitmaps.get(24, false, id, output_lights_on_16_xpm, output_lights_on_24_xpm, output_lights_on_32_xpm, output_lights_on_48_xpm, output_lights_on_64_xpm);
    } else if ("xlART_EFFECTS" == id) {
        return effectBitmaps.get(24, false, id, effects_16_xpm, effects_24_xpm, effects_32_xpm, effects_48_xpm, effects_64_xpm);
    } else if ("xlART_COLORS" == id) {
        return effectBitmaps.get(24, false, id, colors_16_xpm, colors_24_xpm, colors_32_xpm, colors_48_xpm, colors_64_xpm);
    } else if ("xlART_LAYERS" == id) {
        return effectBitmaps.get(24, false, id, layers_16_xpm, layers_24_xpm, layers_32_xpm, layers_48_xpm, layers_64_xpm);
    } else if ("xlART_LAYERS2" == id) {
        return effectBitmaps.get(24, false, id, layers2_16_xpm, layers2_24_xpm, layers2_32_xpm, layers2_48_xpm, layers2_64_xpm);
    } else if ("xlART_EFFECTSETTINGS" == id) {
        return effectBitmaps.get(24, false, id, effsettings_16_xpm, effsettings_24_xpm, effsettings_32_xpm, effsettings_48_xpm, effsettings_64_xpm);
    } else if ("xlART_MODEL_PREVIEW" == id) {
        return effectBitmaps.get(24, false, id, model_preview_16_xpm, model_preview_24_xpm, model_preview_32_xpm, model_preview_48_xpm, model_preview_64_xpm);
    } else if ("xlART_HOUSE_PREVIEW" == id) {
        return effectBitmaps.get(24, false, id, house_preview_16_xpm, house_preview_24_xpm, house_preview_32_xpm, house_preview_48_xpm, house_preview_64_xpm);
    } else if ("xlART_LIGHTS_OFF" == id) {
        return effectBitmaps.get(24, false, id, output_lights_16_xpm, output_lights_24_xpm, output_lights_32_xpm, output_lights_48_xpm, output_lights_64_xpm);
    } else if ("xlART_ZOOM_IN" == id) {
        return effectBitmaps.get(24, false, id, zoom_in_16_xpm, zoom_in_24_xpm, zoom_in_32_xpm, zoom_in_48_xpm, zoom_in_64_xpm);
    } else if ("xlART_ZOOM_OUT" == id) {
        return effectBitmaps.get(24, false, id, zoom_out_16_xpm, zoom_out_24_xpm, zoom_out_32_xpm, zoom_out_48_xpm, zoom_out_64_xpm);
    } else if ("xlART_SETTINGS" == id) {
        return effectBitmaps.get(24, false, id, settings_16_xpm, settings_24_xpm, settings_32_xpm, settings_48_xpm, settings_64_xpm);
    } else if ("xlART_PLAY" == id) {
        return effectBitmaps.get(24, false, id, play_16_xpm, play_24_xpm, play_32_xpm, play_48_xpm, play_64_xpm);
    } else if ("xlART_PAUSE" == id) {
        return effectBitmaps.get(24, false, id, pause_16_xpm, pause_24_xpm, pause_32_xpm, pause_48_xpm, pause_64_xpm);
    } else if ("xlART_BACKWARD" == id) {
        return effectBitmaps.get(24, false, id, backward_16_xpm, backward_24_xpm, backward_32_xpm, backward_48_xpm, backward_64_xpm);
    } else if ("xlART_REWIND10" == id) {
        return effectBitmaps.get(24, false, id, rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm);
    } else if ("xlART_FFORWARD10" == id) {
        return effectBitmaps.get(24, false, id, fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm);
    } else if ("xlART_FORWARD" == id) {
        return effectBitmaps.get(24, false, id, forward_16_xpm, forward_24_xpm, forward_32_xpm, forward_48_xpm, forward_64_xpm);
    } else if ("xlART_REPLAY" == id) {
        return effectBitmaps.get(24, false, id, replay_16_xpm, replay_24_xpm, replay_32_xpm, replay_48_xpm, replay_64_xpm);
    } else if ("xlART_STOP" == id) {
        return effectBitmaps.get(24, false, id, stop_16_xpm, stop_24_xpm, stop_32_xpm, stop_48_xpm, stop_64_xpm);
    } else if ("xlART_LINK" == id) {
        return effectBitmaps.get(48, false, id, link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm);
    } else if ("xlART_UNLINK" == id) {
        return effectBitmaps.get(48, false, id, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm);
    } else if ("xlART_SEQUENCE_ELEMENTS" == id) {
        return effectBitmaps.get(24, false, id, sequence_elements_16_xpm, sequence_elements_24_xpm, sequence_elements_32_xpm, sequence_elements_48_xpm, sequence_elements_64_xpm);
    } else if ("xlAC_ON" == id) {
        return effectBitmaps.get(24, false, id, ACon_16_xpm, ACon_24_xpm, ACon_32_xpm, ACon_48_xpm, ACon_64_xpm);
    }  else if ("xlAC_OFF" == id) {
        return effectBitmaps.get(24, false, id, ACoff_16_xpm, ACoff_24_xpm, ACoff_32_xpm, ACoff_48_xpm, ACoff_64_xpm);
    } else if ("xlAC_SELECT" == id) {
        return effectBitmaps.get(24, false, id, ACselect_16_xpm, ACselect_24_xpm, ACselect_32_xpm, ACselect_48_xpm, ACselect_64_xpm);
    } else if ("xlAC_DISABLED" == id) {
        return effectBitmaps.get(24, false, id, ACdisabled_16_xpm, ACdisabled_24_xpm, ACdisabled_32_xpm, ACdisabled_48_xpm, ACdisabled_64_xpm);
    } else if ("xlAC_ENABLED" == id) {
        return effectBitmaps.get(24, false, id, ACenabled_16_xpm, ACenabled_24_xpm, ACenabled_32_xpm, ACenabled_48_xpm, ACenabled_64_xpm);
    } else if ("xlAC_SHIMMER" == id) {
        return effectBitmaps.get(24, false, id, ACshimmer_16_xpm, ACshimmer_24_xpm, ACshimmer_32_xpm, ACshimmer_48_xpm, ACshimmer_64_xpm);
    } else if ("xlAC_TWINKLE" == id) {
        return effectBitmaps.get(24, false, id, ACtwinkle_16_xpm, ACtwinkle_24_xpm, ACtwinkle_32_xpm, ACtwinkle_48_xpm, ACtwinkle_64_xpm);
    } else if ("xlAC_INTENSITY" == id) {
        return effectBitmaps.get(24, false, id, ACintensity_16_xpm, ACintensity_24_xpm, ACintensity_32_xpm, ACintensity_48_xpm, ACintensity_64_xpm);
    } else if ("xlAC_RAMPUP" == id) {
        return effectBitmaps.get(24, false, id, ACrampup_16_xpm, ACrampup_24_xpm, ACrampup_32_xpm, ACrampup_48_xpm, ACrampup_64_xpm);
    } else if ("xlAC_RAMPDOWN" == id) {
        return effectBitmaps.get(24, false, id, ACrampdown_16_xpm, ACrampdown_24_xpm, ACrampdown_32_xpm, ACrampdown_48_xpm, ACrampdown_64_xpm);
    } else if ("xlAC_RAMPUPDOWN" == id) {
        return effectBitmaps.get(24, false, id, ACrampupdown_16_xpm, ACrampupdown_24_xpm, ACrampupdown_32_xpm, ACrampupdown_48_xpm, ACrampupdown_64_xpm);
    } else if ("xlAC_FILL" == id) {
        return effectBitmaps.get(24, false, id, ACfill_16_xpm, ACfill_24_xpm, ACfill_32_xpm, ACfill_48_xpm, ACfill_64_xpm);
    } else if ("xlAC_CASCADE" == id) {
        return effectBitmaps.get(24, false, id, ACcascade_16_xpm, ACcascade_24_xpm, ACcascade_32_xpm, ACcascade_48_xpm, ACcascade_64_xpm);
    } else if ("xlAC_FOREGROUND" == id) {
        return effectBitmaps.get(24, false, id, ACforeground_16_xpm, ACforeground_24_xpm, ACforeground_32_xpm, ACforeground_48_xpm, ACforeground_64_xpm);
    } else if ("xlAC_BACKGROUND" == id) {
        return effectBitmaps.get(24, false, id, ACbackground_16_xpm, ACbackground_24_xpm, ACbackground_32_xpm, ACbackground_48_xpm, ACbackground_64_xpm);
    } else if ("xlART_PASTE_BY_TIME" == id) {
        return effectBitmaps.get(24, false, id, paste_by_time_16_xpm, paste_by_time_24_xpm, paste_by_time_32_xpm, paste_by_time_48_xpm, paste_by_time_64_xpm);
    } else if ("xlART_PASTE_BY_CELL" == id) {
        return effectBitmaps.get(24, false, id, paste_by_cell_16_xpm, paste_by_cell_24_xpm, paste_by_cell_32_xpm, paste_by_cell_48_xpm, paste_by_cell_64_xpm);
    } else if ("xlART_RENDER_ALL" == id) {
        return effectBitmaps.get(24, false, id, render_all_16_xpm, render_all_24_xpm, render_all_32_xpm, render_all_48_xpm, render_all_64_xpm);
    } else if ("xlART_GROUP_CLOSED" == id) {
        return effectBitmaps.get(16, false, id, group_closed_24_xpm, group_closed_24_xpm, group_closed_64_xpm, group_closed_64_xpm, group_closed_64_xpm);
    } else if ("xlART_GROUP_OPEN" == id) {
        return effectBitmaps.get(16, false, id, group_open_24_xpm, group_open_24_xpm, group_open_64_xpm, group_open_64_xpm, group_open_64_xpm);
    } else if ("xlART_ARCH_ICON" == id) {
        return effectBitmaps.get(16, false, id, arch_icon_16_xpm, arch_icon_16_xpm, arch_icon_64_xpm, arch_icon_64_xpm, arch_icon_64_xpm);
    } else if ("xlART_CANE_ICON" == id) {
        return effectBitmaps.get(16, false, id, cane_icon_16_xpm, cane_icon_16_xpm, cane_icon_64_xpm, cane_icon_64_xpm, cane_icon_64_xpm);
    } else if ("xlART_CIRCLE_ICON" == id) {
        return effectBitmaps.get(16, false, id, circle_icon_16_xpm, circle_icon_16_xpm, circle_icon_64_xpm, circle_icon_64_xpm, circle_icon_64_xpm);
    } else if ("xlART_CHANNELBLOCK_ICON" == id) {
        return effectBitmaps.get(16, false, id, channelblock_icon_16_xpm, channelblock_icon_16_xpm, channelblock_icon_64_xpm, channelblock_icon_64_xpm, channelblock_icon_64_xpm);
    } else if ("xlART_CUSTOM_ICON" == id) {
        return effectBitmaps.get(16, false, id, custom_icon_16_xpm, custom_icon_16_xpm, custom_icon_64_xpm, custom_icon_64_xpm, custom_icon_64_xpm);
    } else if ("xlART_DMX_ICON" == id) {
        return effectBitmaps.get(16, false, id, dmx_icon_16_xpm, dmx_icon_16_xpm, dmx_icon_64_xpm, dmx_icon_64_xpm, dmx_icon_64_xpm);
    } else if ("xlART_ICICLE_ICON" == id) {
        return effectBitmaps.get(16, false, id, icicle_icon_16_xpm, icicle_icon_16_xpm, icicle_icon_64_xpm, icicle_icon_64_xpm, icicle_icon_64_xpm);
    } else if ("xlART_LINE_ICON" == id) {
        return effectBitmaps.get(16, false, id, line_icon_16_xpm, line_icon_16_xpm, line_icon_64_xpm, line_icon_64_xpm, line_icon_64_xpm);
    } else if ("xlART_MATRIX_ICON" == id) {
        return effectBitmaps.get(16, false, id, matrix_icon_16_xpm, matrix_icon_16_xpm, matrix_icon_64_xpm, matrix_icon_64_xpm, matrix_icon_64_xpm);
    } else if ("xlART_POLY_ICON" == id) {
        return effectBitmaps.get(16, false, id, poly_icon_16_xpm, poly_icon_16_xpm, poly_icon_64_xpm, poly_icon_64_xpm, poly_icon_64_xpm);
    } else if ("xlART_SPINNER_ICON" == id) {
        return effectBitmaps.get(16, false, id, spinner_icon_16_xpm, spinner_icon_16_xpm, spinner_icon_64_xpm, spinner_icon_64_xpm, spinner_icon_64_xpm);
    } else if ("xlART_STAR_ICON" == id) {
        return effectBitmaps.get(16, false, id, star_icon_16_xpm, star_icon_16_xpm, star_icon_64_xpm, star_icon_64_xpm, star_icon_64_xpm);
    } else if ("xlART_TREE_ICON" == id) {
        return effectBitmaps.get(16, false, id, tree_icon_16_xpm, tree_icon_16_xpm, tree_icon_64_xpm, tree_icon_64_xpm, tree_icon_64_xpm);
    } else if ("xlART_SUBMODEL_ICON" == id) {
        return effectBitmaps.get(16, false, id, submarine_16, submarine_32, submarine_32, submarine_32, submarine_32);
    } else if ("xlART_WINDOW_ICON" == id) {
        return effectBitmaps.get(16, false, id, window_icon_16_xpm, window_icon_16_xpm, window_icon_64_xpm, window_icon_64_xpm, window_icon_64_xpm);
    } else if ("xlART_WREATH_ICON" == id) {
        return effectBitmaps.get(16, false, id, wreath_icon_16_xpm, wreath_icon_16_xpm, wreath_icon_64_xpm, wreath_icon_64_xpm, wreath_icon_64_xpm);
    } else if ("xlART_DICE_ICON" == id) {
        return effectBitmaps.get(24, false, id, dice_24, dice_24, dice_32, dice_48, dice_64);
    } else if ("wxART_INFORMATION" == id) {
        return effectBitmaps.get(24, false, id, tips_16_xpm, tips_24_xpm, tips_32_xpm, tips_48_xpm, tips_64_xpm);
#ifndef __WXOSX__
        //don't use these on OSX as the OSX supplied Icons look MUCH better and more inline with expectations on a Mac
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
    } else if (wxART_TOOLBAR == client) {
        //sprintf("bmp:  %s   %s  %dx%d\n", (const char *)id.c_str(), (const char*)client.c_str(), size.x, size.y);
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

const wxBitmap &BitmapCache::GetPapgayoXIcon(wxString &toolTip, int size, bool exact) {
    toolTip = "Papagayo Voice";
    return effectBitmaps.get(size, exact, "PapagayoX", papagayo_x_16, papagayo_x_64, papagayo_x_64, papagayo_x_64, papagayo_x_64);
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

