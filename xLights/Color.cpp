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

#include <wx/colour.h>
#include "Color.h"

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
        //need to do the slower lookups
        wxColor c(str);
        red = c.Red();
        green = c.Green();
        blue = c.Blue();
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