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

#include <memory>
#include <string>
#include <vector>

#include "../utils/xlImage.h"

#define XL_FONT_WIDTHS 128

class xlFont {
public:
    xlFont(const uint8_t* pngData, size_t pngLen);
    ~xlFont();
    const xlImage* get_image() const { return &image; }
    int GetWidth() const { return char_width; }
    int GetHeight() const { return char_height; }
    int GetCharWidth(int ascii) const;
    int GetCapsHeight() const { return caps_height; }
    void SetCapsHeight(int height) { caps_height = height; }

private:
    void GatherInfo();

    int char_width;
    int char_height;
    int caps_height;
    int widths[XL_FONT_WIDTHS];
    xlImage image;
};

class FontManager {
public:
    static FontManager& instance() {
        static FontManager me;
        return me;
    }

    void init();

    ~FontManager();

    static const std::vector<std::string>& get_font_names();
    static xlFont* get_font(const std::string& font_name);
    static int get_length(xlFont* font, const std::string& text);

private:
    FontManager();
    FontManager(FontManager const&) = delete;
    void operator=(FontManager const&) = delete;

    static std::vector<std::unique_ptr<xlFont>> fonts;
    static bool initialized;
    static std::vector<std::string> names;
};
