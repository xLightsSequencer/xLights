/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
//  Created by Daniel Kulp on 4/6/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.

#include <map>
#include <array>
#include <wx/mstream.h>
#include "BitmapCache.h"
#include "UtilFunctions.h"

#include "../include/padlock_open_14.xpm"
#include "../include/padlock_open_28.xpm"
#include "../include/padlock_close_14.xpm"
#include "../include/padlock_close_28.xpm"



/*-----  Toolbar Art ----- */
#include "../include/toolbar/toolbarImages.h"


#include "../include/fforward10-24.xpm"
#include "../include/rewind10-24.xpm"

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
#include "../include/cube-icon-16.xpm"
#include "../include/cube-icon-64.xpm"
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
#include "../include/sphere-icon-16.xpm"
#include "../include/sphere-icon-64.xpm"
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
#include "../include/models/image_model.xpm"

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

// used on the Color Panel
#include "../include/cc_time.xpm"
#include "../include/cc_timelocked.xpm"
#include "../include/cc_left.xpm"
#include "../include/cc_right.xpm"
#include "../include/cc_up.xpm"
#include "../include/cc_down.xpm"
#include "../include/cc_na.xpm"
#include "../include/cc_ccw.xpm"
#include "../include/cc_cw.xpm"
#include "../include/cc_radialin.xpm"
#include "../include/cc_radialout.xpm"
#include "../include/save.xpm"
#include "../include/delete.xpm"
#include "../include/switch.xpm"


#include "../include/valuecurvenotselected.xpm"
#include "../include/valuecurveselected.xpm"

#include "Images_png.h"

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"
#include "../include/splashimage.h"

#include "../include/fpp_icon.h"

#include "wx/artprov.h"


class xlArtProvider : public wxArtProvider {
public:
    xlArtProvider() {}
    virtual ~xlArtProvider() {};

    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size) override;
    
    virtual wxIconBundle CreateIconBundle(const wxArtID& id,
                                          const wxArtClient& client) override;
    
    virtual wxBitmapBundle CreateBitmapBundle(const wxArtID& id,
                                              const wxArtClient& client,
                                              const wxSize& size) override;
};

void BitmapCache::SetupArtProvider() {
    wxArtProvider::Push(new xlArtProvider());
}


class xlBitmapBundleImplSet : public wxBitmapBundleImpl
{
public:
    xlBitmapBundleImplSet(const wxSize &ps, const wxString &i, std::vector<const char **> d): preferredSize(ps), data(d), id(i) {
    }

    ~xlBitmapBundleImplSet() {}

    virtual wxSize GetDefaultSize() const override {
        return preferredSize;
    }
    virtual wxSize GetPreferredBitmapSizeAtScale(double scale) const override {
        return wxSize(scale * (double)preferredSize.GetX(), scale * (double)preferredSize.GetY());
    }
    virtual wxBitmap GetBitmap(const wxSize& bsize) override {
        if (bsize != lastSize) {
            lastSize = bsize;
            int size = bsize.GetHeight();
            if (data.size() == 5) {
                const char ** dc = data[0];
                if (size <= 16) {
                    dc = data[0];
                } else if (size <= 24) {
                    dc = data[1];
                } else if (size <= 32) {
                    dc = data[2];
                } else if (size <= 48) {
                    dc = data[3];
                } else {
                    dc = data[4];
                }
                wxImage image(dc);
                if (image.HasMask() && !image.HasAlpha()) {
                    image.InitAlpha();
                }
                if (image.GetHeight() != size) {
                    image.Rescale(bsize.GetWidth(),  bsize.GetHeight());
                }
                lastBitmap = wxBitmap(image);
            } else {
                for (int x = 0; x < data.size(); x++) {
                    wxImage image(data[x]);
                    if (image.GetHeight() == size) {
                        if (image.HasMask() && !image.HasAlpha()) {
                            image.InitAlpha();
                        }
                        lastBitmap = wxBitmap(image);
                        break;
                    } else if (image.GetHeight() > size || x == (data.size()-1)) {
                        if (image.HasMask() && !image.HasAlpha()) {
                            image.InitAlpha();
                        }
                        image.Rescale(bsize.GetWidth(),  bsize.GetHeight());
                        lastBitmap = wxBitmap(image);
                        break;
                    }
                }
            }
        }
        return lastBitmap;
    }
    
    wxSize preferredSize;
    std::vector<const char **> data;
    wxString id;
    
    wxSize lastSize;
    wxBitmap lastBitmap;
};

static std::list<std::string> NAMED;

xlNamedBitmapBundleImpl::xlNamedBitmapBundleImpl(const std::string &n, int i, const wxVector<wxBitmap>& b) : name(n), size(i, i), bitmaps(b) {
    NAMED.push_back(name);
}
xlNamedBitmapBundleImpl::xlNamedBitmapBundleImpl(const std::string &n, const wxSize &sz, const wxVector<wxBitmap>& b) : name(n), size(sz), bitmaps(b)  {
    NAMED.push_back(name);
}
xlNamedBitmapBundleImpl::~xlNamedBitmapBundleImpl() {
    const auto &idx = std::find(NAMED.begin(), NAMED.end(), name);
    if (idx == NAMED.end()) {
        printf("Not found\n");
    } else {
        NAMED.erase(idx);
    }
}


wxSize xlNamedBitmapBundleImpl::GetDefaultSize() const {
    return size;
}
wxSize xlNamedBitmapBundleImpl::GetPreferredBitmapSizeAtScale(double scale) const {
    return wxSize(scale * (double)size.GetX(), scale * (double)size.GetY());
}
wxBitmap xlNamedBitmapBundleImpl::GetBitmap(const wxSize& size) {
    wxSize newSize = size;
    if (newSize == lastSize) {
        return lastBitmap;
    }
    int idx = 0;
    for (int x = 0; x < bitmaps.size(); x++) {
        if (newSize == bitmaps[x].GetSize()) {
            lastSize = newSize;
            lastBitmap = bitmaps[x];
            return lastBitmap;
        }
        if (newSize.GetY() > bitmaps[x].GetHeight()) {
            idx = x;
        }
    }
    if (idx < (bitmaps.size() - 1)) {
        idx++;
    }
    // don't have an exact match size, but idx is pointing to the next largest so we'll
    // rescale that one down
    wxImage i = bitmaps[idx].ConvertToImage();
    i.Rescale(newSize.GetX(), newSize.GetY());
    lastBitmap = wxBitmap(i);
    if (idx == (bitmaps.size() - 1) && newSize.GetY() > bitmaps[idx].GetHeight()) {
        // this is bigger than the last one in the list, we'll keep it
        bitmaps.push_back(lastBitmap);
    }
    lastSize = newSize;
    return lastBitmap;
}


static wxBitmapBundle CreateBitmapBundleFromXPMs(int defSize, const wxString &id, std::vector<const char **> data) {
    if (defSize != -1) {
        return wxBitmapBundle::FromImpl(new xlBitmapBundleImplSet(wxSize(defSize, defSize), id, data));
    }
    
    wxVector<wxBitmap> bitmaps;
    const char **last = nullptr;
    wxSize sz(-1, -1);
    for (auto d : data) {
        if (d != last) {
            last = d;
            wxImage image(d);
            if (image.HasMask() && !image.HasAlpha()) {
                image.InitAlpha();
            }
            if (sz.GetX() == -1) {
                sz = image.GetSize();
            }
            bitmaps.push_back(wxBitmap(image));
        }
    }
    return wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(id, sz, bitmaps));
}

static wxBitmapBundle CreateBitmapBundleFromPNGs(const wxString &id,
                                                 const unsigned char *data, int size,
                                                 const unsigned char *dataDouble = nullptr, int sizeDoube = -1) {
    wxVector<wxBitmap> bitmaps;
    bitmaps.push_back(wxBitmap::NewFromPNGData(data, size));
    if (dataDouble != nullptr) {
        bitmaps.push_back(wxBitmap::NewFromPNGData(dataDouble, sizeDoube));
    } else {
        wxMemoryInputStream stream(data, size);
        wxImage img(stream, wxBITMAP_TYPE_PNG);
        img.Rescale(img.GetWidth() * 2, img.GetHeight() * 2);
        bitmaps.push_back(wxBitmap(img));
    }
    return wxBitmapBundle::FromImpl(new xlNamedBitmapBundleImpl(id, bitmaps[0].GetSize(), bitmaps));
}


wxBitmapBundle xlArtProvider::CreateBitmapBundle(const wxArtID& id,
                                                 const wxArtClient& client,
                                                 const wxSize& size) {
    int sz = size.GetX();
    if (sz == -1) {
        if (client == wxART_TOOLBAR) {
            sz = 24;
        } else if (client == wxART_LIST) {
            sz = 16;
        } else if (client == wxART_MENU) {
            sz = 16;
        }
    }
    /*
    if (sz == -1 || size.GetX() != -1) {
        printf("%s (%s):  %d -> %d\n", id.ToStdString().c_str(), client.ToStdString().c_str(), size.GetX(), sz);
    }
    */
    
    if ("xlART_STOP_NOW" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {stop_sign_16, stop_sign_24, stop_sign_32, stop_sign_48, stop_sign_64});
    } else if ("xlART_LIGHTS_OFF" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {output_lights_off_16, output_lights_off_24, output_lights_off_32, output_lights_off_48, output_lights_off_64});
    } else if ("xlART_OUTPUT_LIGHTS_ON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {output_lights_on_16, output_lights_on_24, output_lights_on_32, output_lights_on_48, output_lights_on_64});
    } else if ("xlART_OUTPUT_LIGHTS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {output_lights_16, output_lights_24, output_lights_32, output_lights_48, output_lights_64});
    } else if ("xlART_EFFECTS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {effects_16, effects_24, effects_32, effects_48, effects_64});
    } else if ("xlART_COLORS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {colors_16, colors_24, colors_32, colors_48, colors_64});
    } else if ("xlART_LAYERS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {layers_16, layers_24, layers_32, layers_48, layers_64});
    } else if ("xlART_LAYERS2" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {layers2_16, layers2_24, layers2_32, layers2_48, layers2_64});
    } else if ("xlART_EFFECTSETTINGS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {effsettings_16, effsettings_24, effsettings_32, effsettings_48, effsettings_64});
    } else if ("xlART_EFFECTASSISTANT" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {effassistant_16, effassistant_24, effassistant_32, effassistant_48, effassistant_64});
    } else if ("xlART_MODEL_PREVIEW" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {model_preview_16, model_preview_24, model_preview_32, model_preview_48, model_preview_64});
    } else if ("xlART_HOUSE_PREVIEW" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {house_preview_16, house_preview_24, house_preview_32, house_preview_48, house_preview_64});
    } else if ("xlART_ZOOM_IN" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {zoom_in_16, zoom_in_24, zoom_in_32, zoom_in_48, zoom_in_64});
    } else if ("xlART_ZOOM_OUT" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {zoom_out_16, zoom_out_24, zoom_out_32, zoom_out_48, zoom_out_64});
    } else if ("xlART_SETTINGS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {settings_16, settings_24, settings_32, settings_48, settings_64});
    } else if ("xlART_PLAY" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {play_16_xpm, play_24_xpm, play_32_xpm, play_48_xpm, play_64_xpm});
    } else if ("xlART_PAUSE" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {pause_16_xpm, pause_24_xpm, pause_32_xpm, pause_48_xpm, pause_64_xpm});
    } else if ("xlART_BACKWARD" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {backward_16_xpm, backward_24_xpm, backward_32_xpm, backward_48_xpm, backward_64_xpm});
    } else if ("xlART_REWIND10" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm, rewind10_24_xpm});
    } else if ("xlART_FFORWARD10" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm, fforward10_24_xpm});
    } else if ("xlART_FORWARD" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {forward_16_xpm, forward_24_xpm, forward_32_xpm, forward_48_xpm, forward_64_xpm});
    } else if ("xlART_REPLAY" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {replay_16_xpm, replay_24_xpm, replay_32_xpm, replay_48_xpm, replay_64_xpm});
    } else if ("xlART_STOP" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {stop_16_xpm, stop_24_xpm, stop_32_xpm, stop_48_xpm, stop_64_xpm});
    } else if ("xlART_LINK" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm, link_48_xpm});
    } else if ("xlART_UNLINK" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm, unlink_48_xpm});
    } else if ("xlART_SEQUENCE_ELEMENTS" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {sequence_elements_16, sequence_elements_24, sequence_elements_32, sequence_elements_48, sequence_elements_64});
    } else if ("xlAC_ON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACon_16, ACon_24, ACon_32, ACon_48, ACon_64});
    }  else if ("xlAC_OFF" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACoff_16, ACoff_24, ACoff_32, ACoff_48, ACoff_64});
    } else if ("xlAC_SELECT" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACselect_16, ACselect_24, ACselect_32, ACselect_48, ACselect_64});
    } else if ("xlAC_DISABLED" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACdisabled_16, ACdisabled_24, ACdisabled_32, ACdisabled_48, ACdisabled_64});
    } else if ("xlAC_ENABLED" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACenabled_16, ACenabled_24, ACenabled_32, ACenabled_48, ACenabled_64});
    } else if ("xlAC_SHIMMER" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACshimmer_16, ACshimmer_24, ACshimmer_32, ACshimmer_48, ACshimmer_64});
    } else if ("xlAC_TWINKLE" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACtwinkle_16, ACtwinkle_24, ACtwinkle_32, ACtwinkle_48, ACtwinkle_64});
    } else if ("xlAC_INTENSITY" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACintensity_16, ACintensity_24, ACintensity_32, ACintensity_48, ACintensity_64});
    } else if ("xlAC_RAMPUP" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACrampup_16, ACrampup_24, ACrampup_32, ACrampup_48, ACrampup_64});
    } else if ("xlAC_RAMPDOWN" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACrampdown_16, ACrampdown_24, ACrampdown_32, ACrampdown_48, ACrampdown_64});
    } else if ("xlAC_RAMPUPDOWN" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACrampupdown_16, ACrampupdown_24, ACrampupdown_32, ACrampupdown_48, ACrampupdown_64});
    } else if ("xlAC_FILL" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACfill_16, ACfill_24, ACfill_32, ACfill_48, ACfill_64});
    } else if ("xlAC_CASCADE" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACcascade_16, ACcascade_24, ACcascade_32, ACcascade_48, ACcascade_64});
    } else if ("xlAC_FOREGROUND" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACforeground_16, ACforeground_24, ACforeground_32, ACforeground_48, ACforeground_64});
    } else if ("xlAC_BACKGROUND" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {ACbackground_16, ACbackground_24, ACbackground_32, ACbackground_48, ACbackground_64});
    } else if ("xlART_PASTE_BY_TIME" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {paste_by_time_16_xpm, paste_by_time_24_xpm, paste_by_time_32_xpm, paste_by_time_48_xpm, paste_by_time_64_xpm});
    } else if ("xlART_PASTE_BY_CELL" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {paste_by_cell_16_xpm, paste_by_cell_24_xpm, paste_by_cell_32_xpm, paste_by_cell_48_xpm, paste_by_cell_64_xpm});
    } else if ("xlART_RENDER_ALL" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {render_all_16_xpm, render_all_24_xpm, render_all_32_xpm, render_all_48_xpm, render_all_64_xpm});
    } else if ("xlART_GROUP_CLOSED" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {group_closed_24_xpm, group_closed_24_xpm, group_closed_64_xpm, group_closed_64_xpm, group_closed_64_xpm});
    } else if ("xlART_GROUP_OPEN" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {group_open_24_xpm, group_open_24_xpm, group_open_64_xpm, group_open_64_xpm, group_open_64_xpm});
    } else if ("wxART_INFORMATION" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {tips_16, tips_24, tips_32, tips_48, tips_64});
    } else if (wxART_FOLDER_OPEN == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {select_show_folder_16_xpm, select_show_folder_24_xpm, select_show_folder_32_xpm, select_show_folder_48_xpm, select_show_folder_64_xpm});
    } else if (wxART_NEW == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {file_new_16_xpm, file_new_24_xpm, file_new_32_xpm, file_new_48_xpm, file_new_64_xpm});
    } else if (wxART_FILE_OPEN == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {open_16_xpm, open_24_xpm, open_32_xpm, open_48_xpm, open_64_xpm});
    } else if (wxART_FILE_SAVE == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {save_16_xpm, save_24_xpm, save_32_xpm, save_48_xpm, save_64_xpm});
    } else if (wxART_FILE_SAVE_AS == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {save_as_16_xpm, save_as_24_xpm, save_as_32_xpm, save_as_48_xpm, save_as_64_xpm});
    } else if ("xlART_PADLOCK_OPEN" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {padlock_open_14, padlock_open_28});
    } else if ("xlART_PADLOCK_CLOSED" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {padlock_close_14, padlock_close_28});
    } else if ("xlART_ARCH_ICON" == id || "xlART_Arches_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {arch_icon_16_xpm, arch_icon_16_xpm, arch_icon_64_xpm, arch_icon_64_xpm, arch_icon_64_xpm});
    } else if ("xlART_CANE_ICON" == id || "xlART_Candy Canes_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {cane_icon_16_xpm, cane_icon_16_xpm, cane_icon_64_xpm, cane_icon_64_xpm, cane_icon_64_xpm});
    } else if ("xlART_CIRCLE_ICON" == id || "xlART_Circle_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {circle_icon_16_xpm, circle_icon_16_xpm, circle_icon_64_xpm, circle_icon_64_xpm, circle_icon_64_xpm});
    } else if ("xlART_CHANNELBLOCK_ICON" == id || "xlART_Channel Block_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {channelblock_icon_16_xpm, channelblock_icon_16_xpm, channelblock_icon_64_xpm, channelblock_icon_64_xpm, channelblock_icon_64_xpm});
    } else if ("xlART_CUBE_ICON" == id || "xlART_Cube_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {cube_icon_16_xpm, cube_icon_16_xpm, cube_icon_64_xpm, cube_icon_64_xpm, cube_icon_64_xpm});
    } else if ("xlART_CUSTOM_ICON" == id || "xlART_Custom_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {custom_icon_16_xpm, custom_icon_16_xpm, custom_icon_64_xpm, custom_icon_64_xpm, custom_icon_64_xpm});
    } else if ("xlART_DMX_ICON" == id || "xlART_DMXFloodlight_ICON" == id || "xlART_DMXMovingHead_ICON" == id || "xlART_DMXMovingHead3D_ICON" == id || "xlART_DMXSkull_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {dmx_icon_16_xpm, dmx_icon_16_xpm, dmx_icon_64_xpm, dmx_icon_64_xpm, dmx_icon_64_xpm});
    } else if ("xlART_ICICLE_ICON" == id || "xlART_Icicles_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {icicle_icon_16_xpm, icicle_icon_16_xpm, icicle_icon_64_xpm, icicle_icon_64_xpm, icicle_icon_64_xpm});
    } else if ("xlART_LINE_ICON" == id || "xlART_Single Line_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {line_icon_16_xpm, line_icon_16_xpm, line_icon_64_xpm, line_icon_64_xpm, line_icon_64_xpm});
    } else if ("xlART_MATRIX_ICON" == id || "xlART_Matrix_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {matrix_icon_16_xpm, matrix_icon_16_xpm, matrix_icon_64_xpm, matrix_icon_64_xpm, matrix_icon_64_xpm});
    } else if ("xlART_POLY_ICON" == id || "xlART_Poly Line_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {poly_icon_16_xpm, poly_icon_16_xpm, poly_icon_64_xpm, poly_icon_64_xpm, poly_icon_64_xpm});
    } else if ("xlART_SPHERE_ICON" == id || "xlART_Sphere_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {sphere_icon_16_xpm, sphere_icon_16_xpm, sphere_icon_64_xpm, sphere_icon_64_xpm, sphere_icon_64_xpm});
    } else if ("xlART_SPINNER_ICON" == id || "xlART_Spinner_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {spinner_icon_16_xpm, spinner_icon_16_xpm, spinner_icon_64_xpm, spinner_icon_64_xpm, spinner_icon_64_xpm});
    } else if ("xlART_STAR_ICON" == id || "xlART_Star_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {star_icon_16_xpm, star_icon_16_xpm, star_icon_64_xpm, star_icon_64_xpm, star_icon_64_xpm});
    } else if ("xlART_TREE_ICON" == id || "xlART_Tree_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {tree_icon_16_xpm, tree_icon_16_xpm, tree_icon_64_xpm, tree_icon_64_xpm, tree_icon_64_xpm});
    } else if ("xlART_SUBMODEL_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {submarine_16, submarine_32, submarine_32, submarine_32, submarine_32});
    } else if ("xlART_WINDOW_ICON" == id || "xlART_Window Frame_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {window_icon_16_xpm, window_icon_16_xpm, window_icon_64_xpm, window_icon_64_xpm, window_icon_64_xpm});
    } else if ("xlART_WREATH_ICON" == id || "xlART_Wreath_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {wreath_icon_16_xpm, wreath_icon_16_xpm, wreath_icon_64_xpm, wreath_icon_64_xpm, wreath_icon_64_xpm});
    } else if ("xlART_IMAGE_ICON" == id || "xlART_Image_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {image_xpm, image_xpm, image_xpm, image_xpm, image_xpm});
    } else if ("xlART_DICE_ICON" == id) {
        return CreateBitmapBundleFromXPMs(sz, id, {dice_24, dice_24, dice_32, dice_48, dice_64});
    } else if ("xlART_cc_time_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_time_xpm", {cc_time_xpm, cc_time_xpm});
    } else if ("xlART_cc_timelocked_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_timelocked_xpm", {cc_timelocked_xpm, cc_timelocked_xpm});
    } else if ("xlART_cc_left_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_left_xpm", {cc_left_xpm, cc_left_xpm});
    } else if ("xlART_cc_right_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_right_xpm", {cc_right_xpm, cc_right_xpm});
    } else if ("xlART_cc_up_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_up_xpm", {cc_up_xpm, cc_up_xpm});
    } else if ("xlART_cc_down_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_down_xpm", {cc_down_xpm, cc_down_xpm});
    } else if ("xlART_cc_na_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_na_xpm", {cc_na_xpm, cc_na_xpm});
    } else if ("xlART_cc_ccw_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_ccw_xpm", {cc_ccw_xpm, cc_ccw_xpm});
    } else if ("xlART_cc_cw_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_cw_xpm", {cc_cw_xpm, cc_cw_xpm});
    } else if ("xlART_cc_radialin_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_radialin_xpm", {cc_radialin_xpm, cc_radialin_xpm});
    } else if ("xlART_cc_radialout_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "cc_radialout_xpm", {cc_radialout_xpm, cc_radialout_xpm});
    } else if ("xlART_colorpanel_switch_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "colorpanel_switch_xpm", {switch_xpm, switch_xpm});
    } else if ("xlART_colorpanel_delete_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "colorpanel_delete_xpm", {delete_xpm, delete_xpm});
    } else if ("xlART_colorpanel_save_xpm" == id) {
        return CreateBitmapBundleFromXPMs(sz, "colorpanel_save_xpm", {save_xpm, save_xpm});
    } else if ("xlART_valuecurve_selected" == id) {
        return CreateBitmapBundleFromXPMs(sz, "valuecurve_selected_xpm", {valuecurveselected_24, valuecurveselected_24});
    } else if ("xlART_valuecurve_notselected" == id) {
        return CreateBitmapBundleFromXPMs(sz, "valuecurve_notselected_xpm", {valuecurvenotselected_24, valuecurvenotselected_24});
    } else if ("xlART_musical_seq" == id) {
        return CreateBitmapBundleFromPNGs(id, musical_seq_png, sizeof(musical_seq_png));
    } else if ("xlART_musical_seq_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, musical_seq_pressed_png, sizeof(musical_seq_pressed_png));
    } else if ("xlART_animation_seq" == id) {
        return CreateBitmapBundleFromPNGs(id, animation_seq_png, sizeof(animation_seq_png));
    } else if ("xlART_animation_seq_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, animation_seq_pressed_png, sizeof(animation_seq_pressed_png));
    } else if ("xlART_time_25ms" == id) {
        return CreateBitmapBundleFromPNGs(id, time_25ms_png, sizeof(time_25ms_png));
    } else if ("xlART_time_25ms_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, time_25ms_pressed_png, sizeof(time_25ms_pressed_png));
    } else if ("xlART_time_custom" == id) {
        return CreateBitmapBundleFromPNGs(id, time_custom_png, sizeof(time_custom_png));
    } else if ("xlART_time_custom_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, time_custom_pressed_png, sizeof(time_custom_pressed_png));
    } else if ("xlART_time_50ms" == id) {
        return CreateBitmapBundleFromPNGs(id, time_50ms_png, sizeof(time_50ms_png));
    } else if ("xlART_time_50ms_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, time_50ms_pressed_png, sizeof(time_50ms_pressed_png));
    } else if ("xlART_quick_start" == id) {
        return CreateBitmapBundleFromPNGs(id, quick_start_png, sizeof(quick_start_png));
    } else if ("xlART_quick_start_pressed" == id) {
        return CreateBitmapBundleFromPNGs(id, quick_start_pressed_png, sizeof(quick_start_pressed_png));
    } else if ("xlART_xlights_logo" == id) {
        return CreateBitmapBundleFromPNGs(id, xlights_logo_png, sizeof(xlights_logo_png));
    } else if ("xlART_lightorama" == id) {
        return CreateBitmapBundleFromPNGs(id, lightorama_png, sizeof(lightorama_png));
    } else if ("xlART_vixen" == id) {
        return CreateBitmapBundleFromPNGs(id, vixen_png, sizeof(vixen_png));
    } else if ("xlART_glediator" == id) {
        return CreateBitmapBundleFromPNGs(id, glediator_png, sizeof(glediator_png));
    } else if ("xlART_hls" == id) {
        return CreateBitmapBundleFromPNGs(id, hls_png, sizeof(hls_png));
    } else if ("xlART_lynx" == id) {
        return CreateBitmapBundleFromPNGs(id, lynx_png, sizeof(lynx_png));
    } else if ("xlART_xLights_SlashImage" == id) {
        return CreateBitmapBundleFromPNGs(id, xl_splashimage_png, sizeof(xl_splashimage_png));
    }
    return wxBitmapBundle();
}

wxBitmap xlArtProvider::CreateBitmap(const wxArtID& id,
                                     const wxArtClient& client,
                                     const wxSize& size) {
    //printf("%s (%s):  %d\n", id.ToStdString().c_str(), client.ToStdString().c_str(), size.GetX());
    
    //printf("bmp:  %s   %s  %dx%d\n", (const char *)id.c_str(), (const char*)client.c_str(), size.x, size.y);
    return wxNullBitmap;
}
wxIconBundle xlArtProvider::CreateIconBundle(const wxArtID& id,
        const wxArtClient& client) {
    
    if (id == "xlART_xLights_Icons") {
        static wxIconBundle icons;
        if (icons.IsEmpty()) {
            icons.AddIcon(wxIcon(xlights_16_xpm));
            icons.AddIcon(wxIcon(xlights_32_xpm));
            icons.AddIcon(wxIcon(xlights_64_xpm));
            icons.AddIcon(wxIcon(xlights_128_xpm));
            icons.AddIcon(wxIcon(xlights_xpm));
        }
        return icons;
    }
    return wxNullIconBundle;
}




wxBitmapBundle BitmapCache::GetPapgayoIcon() {
    return CreateBitmapBundleFromXPMs(16, "Papagayo", {papagayo_16, papagayo_64, papagayo_64, papagayo_64, papagayo_64});
}

wxBitmapBundle BitmapCache::GetPapgayoXIcon() {
    return CreateBitmapBundleFromXPMs(16, "PapagayoX", {papagayo_x_16, papagayo_x_64, papagayo_x_64, papagayo_x_64, papagayo_x_64});
}

wxBitmapBundle BitmapCache::GetModelGroupIcon() {
    return CreateBitmapBundleFromXPMs(16, "ModelGroup", {model_16, model_64, model_64, model_64, model_64});
}
wxBitmapBundle BitmapCache::GetFPPIcon() {
    return wxBitmapBundle::FromSVG(fpp_app_icon_svg, sizeof(fpp_app_icon_svg), wxSize(16, 16));
}

const wxImage &BitmapCache::GetCornerIcon(int position, int size) {
    static std::array<wxImage, 6> images64 = {
        wxImage(point_1a_64),
        wxImage(point_1b_64),
        wxImage(point_2a_64),
        wxImage(point_2b_64),
        wxImage(point_1ab_64),
        wxImage(point_2ab_64)
    };
    static std::array<wxImage, 6> images32 = {
        wxImage(point_1a_64).Rescale(32, 32),
        wxImage(point_1b_64).Rescale(32, 32),
        wxImage(point_2a_64).Rescale(32, 32),
        wxImage(point_2b_64).Rescale(32, 32),
        wxImage(point_1ab_64).Rescale(32, 32),
        wxImage(point_2ab_64).Rescale(32, 32)
    };
    static std::array<wxImage, 6> images16 = {
        wxImage(point_1a_64).Rescale(16, 16),
        wxImage(point_1b_64).Rescale(16, 16),
        wxImage(point_2a_64).Rescale(16, 16),
        wxImage(point_2b_64).Rescale(16, 16),
        wxImage(point_1ab_64).Rescale(16, 16),
        wxImage(point_2ab_64).Rescale(16, 16)
    };
    if (size == 64) {
        return images64[position];
    }
    if (size == 32) {
        return images32[position];
    }
    return images16[position];
}

wxBitmapBundle BitmapCache::GetLockIcon(bool locked) {
    static wxBitmapBundle lockedIcon = CreateBitmapBundleFromXPMs(14, "Locked", {padlock_close_14, padlock_close_28});
    static wxBitmapBundle unlockedIcon = CreateBitmapBundleFromXPMs(14, "Unlocked", {padlock_open_14, padlock_open_28});
    return locked ? lockedIcon : unlockedIcon;
}

