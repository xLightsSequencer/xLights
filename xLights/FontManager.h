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

#include <vector>
#include "wx/wx.h"

#define XL_FONT_WIDTHS 128

class xlFont
{
    public:
        xlFont(wxBitmap& bitmap_);
        virtual ~xlFont();
        wxBitmap* get_bitmap() { return &bitmap; }
        int GetWidth() { return char_width; }
        int GetHeight() { return char_height; }
        int GetCharWidth(int ascii); 
        int GetCapsHeight() { return caps_height; }
        void SetCapsHeight( int height ) { caps_height = height; }
        void GatherInfo();

    protected:
        int char_width;   // the standard character width
        int char_height;  // the standard character height
        int caps_height;  // the capital letter height
        int widths[XL_FONT_WIDTHS];  // the trimmed width of each character
        wxBitmap& bitmap;
};

class FontManager
{
    public:
        static FontManager& instance()
        {
            static FontManager me;
            return me;
        }

        void init();

        virtual ~FontManager();

        static wxArrayString get_font_names();
        static xlFont* get_font(wxString font_name);
        static int get_length(xlFont* font, wxString& text);

        enum XLIGHTS_FONTS {
            FNT_6_5x6_THIN_SYSTEM,
            FNT_6_5x6_THIN_VERTICAL_SYSTEM,
            FNT_6_6x6_THIN_SYSTEM,
            FNT_6_6x6_THIN_VERTICAL_SYSTEM,
            FNT_8_8x8_THIN_SYSTEM,
            FNT_8_8x8_THIN_VERTICAL_SYSTEM,
            FNT_10_12x12_BOLD_SYSTEM,
            FNT_10_12x12_BOLD_VERTICAL_SYSTEM,
            FNT_10_12x12_THIN_SYSTEM,
            FNT_10_12x12_THIN_VERTICAL_SYSTEM,
            FNT_12_15x15_BOLD_SYSTEM,
            FNT_12_15x15_BOLD_VERTICAL_SYSTEM,
            NUM_XL_FONTS
        };

    protected:

    private:
        FontManager();
        FontManager(FontManager const&);     // Don't implement
        void operator=(FontManager const&);  // Don't implement

        static std::vector<wxBitmap> bitmaps;
        static std::vector<xlFont> fonts;
        static bool initialized;
        static wxArrayString names;
};
