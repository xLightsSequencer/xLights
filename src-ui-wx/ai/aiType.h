#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

namespace aiType {
	enum TYPE {
		PROMPT = 0x0000,
		COLORPALETTES = 0x0001,
        IMAGES = 0x0002,
        MAPPING = 0x0003
	};

    inline const char* TypeName(TYPE t) {
        switch (t) {
            case PROMPT: return "Prompt";
            case COLORPALETTES: return "Color Palette";
            case IMAGES: return "Images";
            case MAPPING: return "Mapping";
            default: break;
        }
        return "Unknown";
    }

    inline const char* TypeSettingsSuffix(TYPE t) {
        switch (t) {
            case PROMPT: return "Prompt";
            case COLORPALETTES: return "ColorPalette";
            case IMAGES: return "Images";
            case MAPPING: return "Mapping";
            default: break;
        }
        return "Unknown";
    }
};
