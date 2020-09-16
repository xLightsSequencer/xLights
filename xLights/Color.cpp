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
