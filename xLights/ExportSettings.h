#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

// free functions to create a CSV/Printing export configuration
class wxWindow;

namespace ExportSettings {

    enum SETTINGS {
        SETTINGS_NONE = 0x0000,
        SETTINGS_PORT_ABSADDRESS = 0x0001,
        SETTINGS_PORT_UNIADDRESS = 0x0002,
        SETTINGS_PORT_CHANNELS = 0x0004,
        SETTINGS_PORT_PIXELS = 0x008,
        SETTINGS_MODEL_DESCRIPTIONS = 0x0010,
        SETTINGS_MODEL_ABSADDRESS = 0x0020,
        SETTINGS_MODEL_UNIADDRESS = 0x0040,
        SETTINGS_MODEL_CHANNELS = 0x0080,
        SETTINGS_MODEL_PIXELS = 0x0100
    };
    inline SETTINGS operator|(SETTINGS const a, SETTINGS const b) {
        return static_cast<SETTINGS>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline SETTINGS& operator|=(SETTINGS& a, SETTINGS b) {
        return a = a | b;
    }

    SETTINGS GetSettings(wxWindow* parent);

};