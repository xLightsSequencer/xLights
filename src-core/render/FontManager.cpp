/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FontManager.h"
#include "../../include/xLightsFontImages.h"

#include <cstdlib>

#define FONT_BITMAP_COLUMNS 8
#define FONT_BITMAP_ROWS 16

xlFont::xlFont(const uint8_t* pngData, size_t pngLen) {
    image.LoadFromMemory(pngData, pngLen);
    char_width = ((image.GetWidth() - 1) / FONT_BITMAP_COLUMNS) - 1;
    char_height = ((image.GetHeight() - 1) / FONT_BITMAP_ROWS) - 1;
    caps_height = char_height;
    GatherInfo();
}

xlFont::~xlFont() {
}

int xlFont::GetCharWidth(int ascii) const {
    if (ascii < 0 || ascii >= XL_FONT_WIDTHS) {
        return 0;
    }
    return widths[ascii];
}

void xlFont::GatherInfo() {
    int index = 0;
    for (int i = 0; i < XL_FONT_WIDTHS; i++) {
        widths[i] = char_width;
    }
    for (int y = 0; y < FONT_BITMAP_ROWS; y++) {
        int y_pos = (y * (char_height + 1)) + 1;
        for (int x = 0; x < FONT_BITMAP_COLUMNS; x++) {
            int x_pos = (x * (char_width + 1)) + 1;
            for (int z = 0; z < char_width; z++) {
                int red = image.GetRed(x_pos + z, y_pos);
                int green = image.GetGreen(x_pos + z, y_pos);
                int blue = image.GetBlue(x_pos + z, y_pos);
                if (red == 0 && green == 255 && blue == 255) {
                    widths[index] = z;
                    break;
                }
            }
            index++;
        }
    }
}

FontManager::FontManager() {
}

std::vector<std::unique_ptr<xlFont>> FontManager::fonts;
bool FontManager::initialized = false;
std::vector<std::string> FontManager::names;

FontManager::~FontManager() {
}

void FontManager::init() {
    if (!initialized) {
        get_font_names(); // ensure names are populated

        fonts.push_back(std::make_unique<xlFont>(font_5_5x5_thin_system_png, sizeof(font_5_5x5_thin_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_5_5x5_full_system_png, sizeof(font_5_5x5_full_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_6_5x6_thin_system_png, sizeof(font_6_5x6_thin_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_6_5x6_thin_vertical_system_png, sizeof(font_6_5x6_thin_vertical_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_6_6x6_thin_system_png, sizeof(font_6_6x6_thin_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_6_6x6_thin_vertical_system_png, sizeof(font_6_6x6_thin_vertical_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_7_7x9_thin_png, sizeof(font_7_7x9_thin_png)));
        fonts.push_back(std::make_unique<xlFont>(font_7_7x9_thinnarrow_png, sizeof(font_7_7x9_thinnarrow_png)));
        fonts.push_back(std::make_unique<xlFont>(font_7_7x9_bold_png, sizeof(font_7_7x9_bold_png)));
        fonts.push_back(std::make_unique<xlFont>(font_8_8x8_thin_system_png, sizeof(font_8_8x8_thin_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_8_8x8_thin_vertical_system_png, sizeof(font_8_8x8_thin_vertical_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_10_12x12_bold_system_png, sizeof(font_10_12x12_bold_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_10_12x12_bold_vertical_system_png, sizeof(font_10_12x12_bold_vertical_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_10_12x12_thin_system_png, sizeof(font_10_12x12_thin_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_10_12x12_thin_vertical_system_png, sizeof(font_10_12x12_thin_vertical_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_12_15x15_bold_system_png, sizeof(font_12_15x15_bold_system_png)));
        fonts.push_back(std::make_unique<xlFont>(font_12_15x15_bold_vertical_system_png, sizeof(font_12_15x15_bold_vertical_system_png)));

        for (size_t i = 0; i < fonts.size(); i++) {
            auto dash = names[i].find('-');
            if (dash != std::string::npos) {
                fonts[i]->SetCapsHeight(std::strtol(names[i].c_str(), nullptr, 10));
            }
        }

        initialized = true;
    }
}

const std::vector<std::string>& FontManager::get_font_names() {
    if (names.empty()) {
        names.push_back("5-5x5 Thin");
        names.push_back("5-5x5 Mono");
        names.push_back("6-5x6 Thin");
        names.push_back("6-5x6 Thin Vertical");
        names.push_back("6-6x6 Thin");
        names.push_back("6-6x6 Thin Vertical");
        names.push_back("7-7x9 Thin");
        names.push_back("7-7x9 Thin Narrow");
        names.push_back("7-7x9 Bold");
        names.push_back("8-8x8 Thin");
        names.push_back("8-8x8 Thin Vertical");
        names.push_back("10-12x12 Bold");
        names.push_back("10-12x12 Bold Vertical");
        names.push_back("10-12x12 Thin");
        names.push_back("10-12x12 Thin Vertical");
        names.push_back("12-15x15 Bold");
        names.push_back("12-15x15 Bold Vertical");
    }

    return names;
}

xlFont* FontManager::get_font(const std::string& font_name) {
    for (size_t i = 0; i < fonts.size(); i++) {
        if (names[i] == font_name) {
            return fonts[i].get();
        }
    }
    return nullptr;
}

int FontManager::get_length(xlFont* font, const std::string& text) {
    if (text.empty()) return 0;
    int length = 0;
    for (size_t i = 0; i < text.length(); i++) {
        char ascii = text[i];
        length += font->GetCharWidth(ascii);
    }
    return length;
}
