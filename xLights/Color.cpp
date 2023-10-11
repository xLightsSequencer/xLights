/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
//  Created by Daniel Kulp on 1/12/16.
//  Copyright © 2016 Daniel Kulp. All rights reserved.

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <map>

#include <wx/colour.h>
#include "Color.h"

static const std::map<std::string, xlColor> NAME_MAP = {
    {"White", xlWHITE},
    {"WHITE", xlWHITE},
    {"Black", xlBLACK},
    {"BLACK", xlBLACK},
    {"Blue", xlBLUE},
    {"BLUE", xlBLUE},
    {"Cyan", xlCYAN},
    {"CYAN", xlCYAN},
    {"Green", xlGREEN},
    {"GREEN", xlGREEN},
    {"MAGENTA", xlMAGENTA},
    {"Magenta", xlMAGENTA},
    {"RED", xlRED},
    {"Red", xlRED},
    {"YELLOW", xlYELLOW},
    {"Yellow", xlYELLOW},

    {"AQUAMARINE", xlColor(112, 219, 147)},
    {"BLUE VIOLET", xlColor( 159, 95, 159)},
    {"BROWN", xlColor( 165, 42, 42)},
    {"CADET BLUE", xlColor( 95, 159, 159)},
    {"CORAL", xlColor( 255, 127, 0)},
    {"CORNFLOWER BLUE", xlColor( 66, 66, 111)},
    {"DARK GREY", xlColor( 47, 47, 47)},   // ?
    {"DARK GREEN", xlColor( 47, 79, 47)},
    {"DARK OLIVE GREEN", xlColor( 79, 79, 47)},
    {"DARK ORCHID", xlColor( 153, 50, 204)},
    {"DARK SLATE BLUE", xlColor( 107, 35, 142)},
    {"DARK SLATE GREY", xlColor( 47, 79, 79)},
    {"DARK TURQUOISE", xlColor( 112, 147, 219)},
    {"DIM GREY", xlColor( 84, 84, 84)},
    {"FIREBRICK", xlColor( 142, 35, 35)},
    {"FOREST GREEN", xlColor( 35, 142, 35)},
    {"GOLD", xlColor( 204, 127, 50)},
    {"GOLDENROD", xlColor( 219, 219, 112)},
    {"GREEN YELLOW", xlColor( 147, 219, 112)},
    {"GREY", xlColor( 128, 128, 128)},
    {"INDIAN RED", xlColor( 79, 47, 47)},
    {"KHAKI", xlColor( 159, 159, 95)},
    {"LIGHT BLUE", xlColor( 191, 216, 216)},
    {"LIGHT GREY", xlColor( 192, 192, 192)},
    {"LIGHT STEEL BLUE", xlColor( 143, 143, 188)},
    {"LIME GREEN", xlColor( 50, 204, 50)},
    {"LIGHT MAGENTA", xlColor( 255, 119, 255)},
    {"MAROON", xlColor( 142, 35, 107)},
    {"MEDIUM AQUAMARINE", xlColor( 50, 204, 153)},
    {"MEDIUM GREY", xlColor( 100, 100, 100)},
    {"MEDIUM BLUE", xlColor( 50, 50, 204)},
    {"MEDIUM FOREST GREEN", xlColor( 107, 142, 35)},
    {"MEDIUM GOLDENROD", xlColor( 234, 234, 173)},
    {"MEDIUM ORCHID", xlColor( 147, 112, 219)},
    {"MEDIUM SEA GREEN", xlColor( 66, 111, 66)},
    {"MEDIUM SLATE BLUE", xlColor( 127, 0, 255)},
    {"MEDIUM SPRING GREEN", xlColor( 127, 255, 0)},
    {"MEDIUM TURQUOISE", xlColor( 112, 219, 219)},
    {"MEDIUM VIOLET RED", xlColor( 219, 112, 147)},
    {"MIDNIGHT BLUE", xlColor( 47, 47, 79)},
    {"NAVY", xlColor( 35, 35, 142)},
    {"ORANGE", xlColor( 204, 50, 50)},
    {"ORANGE RED", xlColor( 255, 0, 127)},
    {"ORCHID", xlColor( 219, 112, 219)},
    {"PALE GREEN", xlColor( 143, 188, 143)},
    {"PINK", xlColor( 255, 192, 203)},
    {"PLUM", xlColor( 234, 173, 234)},
    {"PURPLE", xlColor( 176, 0, 255)},
    {"SALMON", xlColor( 111, 66, 66)},
    {"SEA GREEN", xlColor( 35, 142, 107)},
    {"SIENNA", xlColor( 142, 107, 35)},
    {"SKY BLUE", xlColor( 50, 153, 204)},
    {"SLATE BLUE", xlColor( 0, 127, 255)},
    {"SPRING GREEN", xlColor( 0, 255, 127)},
    {"STEEL BLUE", xlColor( 35, 107, 142)},
    {"TAN", xlColor( 219, 147, 112)},
    {"THISTLE", xlColor( 216, 191, 216)},
    {"TURQUOISE", xlColor( 173, 234, 234)},
    {"VIOLET", xlColor( 79, 47, 79)},
    {"VIOLET RED", xlColor( 204, 50, 153)},
    {"WHEAT", xlColor( 216, 216, 191)},
    {"YELLOW GREEN", xlColor( 153, 204, 50)}
};

xlColor::operator std::string() const {
    std::stringstream stream;
    stream << "#"
        << std::setfill ('0') << std::setw(6)
        << std::hex << GetRGB(false);
    return stream.str();
}

void xlColor::SetFromString(const std::string &str) {
    alpha = 255;
    if (str.empty()) {
        red = blue = green = 0;
        return;
    } else if ( str[0] == '#' && str.size() == 7 ) {
        // hexadecimal prefixed with # (HTML syntax)
        unsigned long tmp = strtoul(&str.c_str()[1], nullptr, 16);

        Set((uint8_t)(tmp >> 16),
            (uint8_t)(tmp >> 8),
            (uint8_t)tmp);
    } else if ( str[0] == '#' && str.size() == 9 ) {
        // hexadecimal prefixed with # (HTML syntax)
        unsigned long tmp = strtoul(&str.c_str()[1], nullptr, 16);
        
        Set((uint8_t)(tmp >> 16),
            (uint8_t)(tmp >> 8),
            (uint8_t)tmp,
            (uint8_t)(tmp >> 24));
    } else if (str[0] == '0' && str[1] == 'x' && str.size() == 8 ) {
        // hexadecimal prefixed with 0x
        unsigned long tmp = strtoul(&str.c_str()[2], nullptr, 16);

        Set((uint8_t)(tmp >> 16),
            (uint8_t)(tmp >> 8),
            (uint8_t)tmp);
    } else if (str[0] == '0' && str[1] == 'x' && str.size() == 10 ) {
        // hexadecimal prefixed with 0x
        unsigned long tmp = strtoul(&str.c_str()[2], nullptr, 16);
        
        Set((uint8_t)(tmp >> 16),
            (uint8_t)(tmp >> 8),
            (uint8_t)tmp,
            (uint8_t)(tmp >> 24));
    } else if (str.size() > 5 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && (str[3] == '(' || str[4] == '(')) {
        int start = 4;
        if (str[3] == 'a') {
            start++;
        }
        std::string::size_type sz;
        std::string val = str.substr(start);
        red = std::stoi(val, &sz);
        val = val.substr(sz + 1);
        green = std::stoi(val, &sz);
        val = val.substr(sz + 1);
        blue = std::stoi(val, &sz);
        val = val.substr(sz + 1);
        alpha = 255;
        if (str[3] == 'a') {
            float f = std::stof(val) * 255.0;
            alpha = f;
        }
    } else {
        auto c = NAME_MAP.find(str);
        if (c != NAME_MAP.end()) {
            red = c->second.red;
            green = c->second.green;
            blue = c->second.blue;
            alpha = c->second.alpha;
        } else {
        
            //need to do the slower lookups
            wxColor c(str);
            red = c.Red();
            green = c.Green();
            blue = c.Blue();
        }
    }
}
static void fromHSV(xlColor & rgb, const HSVValue &hsv) {
    double red, green, blue;

    if (0.0f == hsv.saturation) {
        // Grey
        red = hsv.value;
        green = hsv.value;
        blue = hsv.value;
    } else { // not grey
        double hue = hsv.hue * 6.0;      // sector 0 to 5
        int i = (int)std::floor(hue);
        double f = hue - i;          // fractional part of h
        double p = hsv.value * (1.0 - hsv.saturation);

        switch (i) {
            case 0:
                red = hsv.value;
                green = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                blue = p;
                break;

            case 1:
                red = hsv.value * (1.0 - hsv.saturation * f);
                green = hsv.value;
                blue = p;
                break;

            case 2:
                red = p;
                green = hsv.value;
                blue = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                break;

            case 3:
                red = p;
                green = hsv.value * (1.0 - hsv.saturation * f);
                blue = hsv.value;
                break;

            case 4:
                red = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                green = p;
                blue = hsv.value;
                break;

            default:    // case 5:
                red = hsv.value;
                green = p;
                blue = hsv.value * (1.0 - hsv.saturation * f);
                break;
        }
    }
    rgb.Set((uint8_t)(red * 255.0),
            (uint8_t)(green * 255.0),
            (uint8_t)(blue * 255.0));
}

void xlColor::fromHSV(const HSVValue &hsv) {
    ::fromHSV(*this, hsv);
}
//optimized algorithm from http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
static void toHSV(const xlColor &c, HSVValue &v)
{
    double r = c.red / 255.0;
    double g = c.green / 255.0;
    double b = c.blue / 255.0;

    double K = 0.0;
    if (g < b)
    {
        std::swap(g, b);
        K = -1.0;
    }
    double min_gb = b;
    if (r < g)
    {
        std::swap(r, g);
        K = -2.0 / 6.0 - K;
        min_gb = std::min(g, b);
    }
    double chroma = r - min_gb;
    v.hue = std::abs(K + (g - b) / (6.0 * chroma + 1e-20));
    v.saturation = chroma / (r + 1e-20);
    v.value = r;
}

void xlColor::toHSV(HSVValue &v) const {
    ::toHSV(*this, v);
}


HSVValue::HSVValue(const xlColor &c) {
    toHSV(c, *this);
}


HSVValue& HSVValue::operator=(const xlColor& c) {
    toHSV(c, *this);
    return *this;
}


void xlColor::SetFromString(const wxString &str) {
    SetFromString(str.ToStdString());
}
xlColor::operator wxString() const {
    return wxString::Format("#%02x%02x%02x", red, green, blue);
}

wxColor xlColor::asWxColor() const {
    return wxColor(red, green, blue, alpha);
}

// HSL functions
double Hue2RGB(double v1, double v2, double H) {
   if(H < 0.0) H += 1.0;
   if(H > 1.0) H -= 1.0;
   if((6.0*H) < 1.0) return (v1 + (v2 - v1) * 6.0 * H);
   if((2.0*H) < 1.0) return v2;
   if((3.0*H) < 2.0) return (v1 + (v2 - v1) * ((2.0/3.0) - H) * 6.0);
   return v1;
}

static void fromHSL(xlColor & rgb, const HSLValue &hsl) {
    if(hsl.saturation == 0) {
      rgb.red = (int)(hsl.lightness * 255.0);
      rgb.green = rgb.red;
      rgb.blue = rgb.red;
      return;
   }

   double v2;
   if(hsl.lightness < 0.5) v2 = hsl.lightness * (1.0 + hsl.saturation);
   else                    v2 = (hsl.lightness + hsl.saturation) - (hsl.saturation * hsl.lightness);

   double v1 = 2.0 * hsl.lightness - v2;

   rgb.red =   (int)(255.0 * Hue2RGB(v1, v2, hsl.hue + (1.0/3.0)) + 0.5);
   rgb.green = (int)(255.0 * Hue2RGB(v1, v2, hsl.hue) + 0.5);
   rgb.blue =  (int)(255.0 * Hue2RGB(v1, v2, hsl.hue - (1.0/3.0)) + 0.5);
}

void xlColor::fromHSL(const HSLValue &hsl) {
    ::fromHSL(*this, hsl);
}

static void toHSL(const xlColor &c, HSLValue &hsl)
{
   double rgb[3];
   rgb[0] = (double)c.red / 255.0;
   rgb[1] = (double)c.green / 255.0;
   rgb[2] = (double)c.blue / 255.0;

   double min = std::min(rgb[0], std::min(rgb[1], rgb[2]));
   double max = std::max(rgb[0], std::max(rgb[1], rgb[2]));
   double delta = max - min;

   hsl.lightness = (max + min) / 2.0;
   if(delta == 0) { // Grey
      hsl.hue = -1.0;
      hsl.saturation = 0;
      return;
   }

   if(hsl.lightness <= 0.5) hsl.saturation = delta / (max + min);
   else                    hsl.saturation = delta / (2.0 - max - min);

   double dr = (((max - rgb[0]) / 6.0) + (delta / 2.0)) / delta;
   double dg = (((max - rgb[1]) / 6.0) + (delta / 2.0)) / delta;
   double db = (((max - rgb[2]) / 6.0) + (delta / 2.0)) / delta;

   if     (rgb[0] == max) hsl.hue = db - dg;
   else if(rgb[1] == max) hsl.hue = (1.0/3.0) + dr - db;
   else if(rgb[2] == max) hsl.hue = (2.0/3.0) + dg - dr;

   if(hsl.hue < 0.0) hsl.hue += 1.0;
   if(hsl.hue > 1.0) hsl.hue -= 1.0;
}

void xlColor::toHSL(HSLValue &v) const {
    ::toHSL(*this, v);
}

HSLValue::HSLValue(const xlColor &c) {
    toHSL(c, *this);
}


HSLValue& HSLValue::operator=(const xlColor& c) {
    toHSL(c, *this);
    return *this;
}

int xlColor::Brightness() const
{
    return std::max((int)red, std::max((int)blue, (int)green)) * (int)alpha / 255;
}


inline float GetSRGB(float r) {
    r /= 255.0f;
    return r <= 0.03928f ? r / 12.92f : std::pow((r + 0.055f) / 1.055f, 2.4f);
}
inline float GetRelativeLuminance(const xlColor& c) {
    // based on https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
    return
        0.2126f * GetSRGB(c.red) +
        0.7152f * GetSRGB(c.green) +
        0.0722f * GetSRGB(c.blue);
}
inline float GetColourContrast(const xlColor& c1, const xlColor& c2) {
    // based on https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast7.html
    float L1 = GetRelativeLuminance(c1);
    float L2 = GetRelativeLuminance(c2);
    return L1 > L2 ? (L1 + 0.05f) / (L2 + 0.05f) : (L2 + 0.05f) / (L1 + 0.05f);
}

bool xlColor::HasSufficientContrast(const xlColor& bg) const {
    return GetColourContrast(bg, *this) >= 4.5f;
}

typedef struct NAMED_COLOUR {
    std::string name;
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    float HowClose(wxColour c) const
    {
      return std::sqrt(((int)c.Red() - (int)red) * ((int)c.Red() - (int)red) +
                       ((int)c.Green() - (int)green) * ((int)c.Green() - (int)green) +
                       ((int)c.Blue() - (int)blue) * ((int)c.Blue() - (int)blue));
    }
} NAMED_COLOUR;

static NAMED_COLOUR namedColours[] = {
    { "Indian Red", 0xCD, 0x5C, 0x5C },
    { "Light Coral", 0xF0, 0x80, 0x80 },
    { "Salmon", 0xFA, 0x80, 0x72 },
    { "Dark Salmon", 0xE9, 0x96, 0x7A },
    { "Light Salmon", 0xFF, 0xA0, 0x7A },
    { "Crimson", 0xDC, 0x14, 0x3C },
    { "Red", 0xFF, 0x00, 0x00 },
    { "Fire Brick", 0xB2, 0x22, 0x22 },
    { "Dark Red", 0x8B, 0x00, 0x00 },
    { "Pink", 0xFF, 0xC0, 0xCB },
    { "Light Pink", 0xFF, 0xB6, 0xC1 },
    { "Hot Pink", 0xFF, 0x69, 0xB4 },
    { "Deep Pink", 0xFF, 0x14, 0x93 },
    { "Medium Violet Red", 0xC7, 0x15, 0x85 },
    { "Pale Violet Red", 0xDB, 0x70, 0x93 },
    { "Light Salmon", 0xFF, 0xA0, 0x7A },
    { "Coral", 0xFF, 0x7F, 0x50 },
    { "Tomato", 0xFF, 0x63, 0x47 },
    { "Orange Red", 0xFF, 0x45, 0x00 },
    { "Dark Orange", 0xFF, 0x8C, 0x00 },
    { "Orange", 0xFF, 0xA5, 0x00 },
    { "Gold", 0xFF, 0xD7, 0x00 },
    { "Yellow", 0xFF, 0xFF, 0x00 },
    { "Light Yellow", 0xFF, 0xFF, 0xE0 },
    { "Lemon Chiffon", 0xFF, 0xFA, 0xCD },
    { "Light Goldenrod Yellow", 0xFA, 0xFA, 0xD2 },
    { "Papaya Whip", 0xFF, 0xEF, 0xD5 },
    { "Moccasin", 0xFF, 0xE4, 0xB5 },
    { "PeachPuff", 0xFF, 0xDA, 0xB9 },
    { "Pale Goldenrod", 0xEE, 0xE8, 0xAA },
    { "Khaki", 0xF0, 0xE6, 0x8C },
    { "Dark Khaki", 0xBD, 0xB7, 0x6B },
    { "Lavender", 0xE6, 0xE6, 0xFA },
    { "Thistle", 0xD8, 0xBF, 0xD8 },
    { "Plum", 0xDD, 0xA0, 0xDD },
    { "Violet", 0xEE, 0x82, 0xEE },
    { "Orchid", 0xDA, 0x70, 0xD6 },
    // { "Fuchsia", 0xFF, 0x00, 0xFF },
    { "Magenta", 0xFF, 0x00, 0xFF },
    { "Medium Orchid", 0xBA, 0x55, 0xD3 },
    { "Medium Purple", 0x93, 0x70, 0xDB },
    { "Rebecca Purple", 0x66, 0x33, 0x99 },
    { "Blue Violet", 0x8A, 0x2B, 0xE2 },
    { "Dark Violet", 0x94, 0x00, 0xD3 },
    { "Dark Orchid", 0x99, 0x32, 0xCC },
    { "Dark Magenta", 0x8B, 0x00, 0x8B },
    { "Purple", 0x80, 0x00, 0x80 },
    { "Indigo", 0x4B, 0x00, 0x82 },
    { "Slate Blue", 0x6A, 0x5A, 0xCD },
    { "Dark Slate Blue", 0x48, 0x3D, 0x8B },
    { "Medium Slate Blue", 0x7B, 0x68, 0xEE },
    { "Green Yellow", 0xAD, 0xFF, 0x2F },
    { "Chartreuse", 0x7F, 0xFF, 0x00 },
    { "Lawn Green", 0x7C, 0xFC, 0x00 },
    { "Green", 0x00, 0xFF, 0x00 }, // Lime
    { "Lime Green", 0x32, 0xCD, 0x32 },
    { "Pale Green", 0x98, 0xFB, 0x98 },
    { "Light Green", 0x90, 0xEE, 0x90 },
    { "Medium Spring Green", 0x00, 0xFA, 0x9A },
    { "Spring Green", 0x00, 0xFF, 0x7F },
    { "Medium Sea Green", 0x3C, 0xB3, 0x71 },
    { "Sea Green", 0x2E, 0x8B, 0x57 },
    { "Forest Green", 0x22, 0x8B, 0x22 },
    { "Dark Green", 0x00, 0x80, 0x00 }, // Green
    { "Darker Green", 0x00, 0x64, 0x00 }, // Dark Green
    { "Yellow Green", 0x9A, 0xCD, 0x32 },
    { "Olive Drab", 0x6B, 0x8E, 0x23 },
    { "Olive", 0x80, 0x80, 0x00 },
    { "Dark Olive Green", 0x55, 0x6B, 0x2F },
    { "Medium Aquamarine", 0x66, 0xCD, 0xAA },
    { "Dark Sea Green", 0x8F, 0xBC, 0x8B },
    { "Light Sea Green", 0x20, 0xB2, 0xAA },
    { "Dark Cyan", 0x00, 0x8B, 0x8B },
    { "Teal", 0x00, 0x80, 0x80 },
    // { "Aqua", 0x00, 0xFF, 0xFF },
    { "Cyan", 0x00, 0xFF, 0xFF },
    { "Light Cyan", 0xE0, 0xFF, 0xFF },
    { "Pale Turquoise", 0xAF, 0xEE, 0xEE },
    { "Aquamarine", 0x7F, 0xFF, 0xD4 },
    { "Turquoise", 0x40, 0xE0, 0xD0 },
    { "Medium Turquoise", 0x48, 0xD1, 0xCC },
    { "Dark Turquoise", 0x00, 0xCE, 0xD1 },
    { "Cadet Blue", 0x5F, 0x9E, 0xA0 },
    { "Steel Blue", 0x46, 0x82, 0xB4 },
    { "Light Steel Blue", 0xB0, 0xC4, 0xDE },
    { "Powder Blue", 0xB0, 0xE0, 0xE6 },
    { "Light Blue", 0xAD, 0xD8, 0xE6 },
    { "Sky Blue", 0x87, 0xCE, 0xEB },
    { "Light Sky Blue", 0x87, 0xCE, 0xFA },
    { "Deep Sky Blue", 0x00, 0xBF, 0xFF },
    { "Dodger Blue", 0x1E, 0x90, 0xFF },
    { "Cornflower Blue", 0x64, 0x95, 0xED },
    { "Medium Slate Blue", 0x7B, 0x68, 0xEE },
    { "Royal Blue", 0x41, 0x69, 0xE1 },
    { "Blue", 0x00, 0x00, 0xFF },
    { "Medium Blue", 0x00, 0x00, 0xCD },
    { "Dark Blue", 0x00, 0x00, 0x8B },
    { "Navy", 0x00, 0x00, 0x80 },
    { "Midnight Blue", 0x19, 0x19, 0x70 },
    { "Cornsilk", 0xFF, 0xF8, 0xDC },
    { "Blanched Almond", 0xFF, 0xEB, 0xCD },
    { "Bisque", 0xFF, 0xE4, 0xC4 },
    { "Navajo White", 0xFF, 0xDE, 0xAD },
    { "Wheat", 0xF5, 0xDE, 0xB3 },
    { "Burly Wood", 0xDE, 0xB8, 0x87 },
    { "Tan", 0xD2, 0xB4, 0x8C },
    { "Rosy Brown", 0xBC, 0x8F, 0x8F },
    { "Sandy Brown", 0xF4, 0xA4, 0x60 },
    { "Goldenrod", 0xDA, 0xA5, 0x20 },
    { "Dark Goldenrod", 0xB8, 0x86, 0x0B },
    { "Peru", 0xCD, 0x85, 0x3F },
    { "Chocolate", 0xD2, 0x69, 0x1E },
    { "Saddle Brown", 0x8B, 0x45, 0x13 },
    { "Sienna", 0xA0, 0x52, 0x2D },
    { "Brown", 0xA5, 0x2A, 0x2A },
    { "Maroon", 0x80, 0x00, 0x00 },
    { "White", 0xFF, 0xFF, 0xFF },
    { "Snow", 0xFF, 0xFA, 0xFA },
    { "Honey Dew", 0xF0, 0xFF, 0xF0 },
    { "Mint Cream", 0xF5, 0xFF, 0xFA },
    { "Azure", 0xF0, 0xFF, 0xFF },
    { "Alice Blue", 0xF0, 0xF8, 0xFF },
    { "Ghost White", 0xF8, 0xF8, 0xFF },
    { "White Smoke", 0xF5, 0xF5, 0xF5 },
    { "Sea Shell", 0xFF, 0xF5, 0xEE },
    { "Beige", 0xF5, 0xF5, 0xDC },
    { "Old Lace", 0xFD, 0xF5, 0xE6 },
    { "Floral White", 0xFF, 0xFA, 0xF0 },
    { "Ivory", 0xFF, 0xFF, 0xF0 },
    { "Antique White", 0xFA, 0xEB, 0xD7 },
    { "Linen", 0xFA, 0xF0, 0xE6 },
    { "Lavender Blush", 0xFF, 0xF0, 0xF5 },
    { "Misty Rose", 0xFF, 0xE4, 0xE1 },
    { "Gainsboro", 0xDC, 0xDC, 0xDC },
    { "Light Gray", 0xD3, 0xD3, 0xD3 },
    { "Silver", 0xC0, 0xC0, 0xC0 },
    { "Dark Gray", 0xA9, 0xA9, 0xA9 },
    { "Gray", 0x80, 0x80, 0x80 },
    { "Dim Gray", 0x69, 0x69, 0x69 },
    { "Light Slate Gray", 0x77, 0x88, 0x99 },
    { "Slate Gray", 0x70, 0x80, 0x90 },
    { "Dark Slate Gray", 0x2F, 0x4F, 0x4F },
    { "Black", 0x00, 0x00, 0x00 }
};

const std::string& GetColourName(const wxColour& c)
{
    const NAMED_COLOUR* nc = nullptr;
    float min = 99999999999.0f;

    for (const auto& it : namedColours) {
        if (nc == nullptr) {
            nc = &it;
            min = it.HowClose(c);
        } else if (it.HowClose(c) < min) {
            nc = &it;
            min = it.HowClose(c);
        }
    }

    return nc->name;
}