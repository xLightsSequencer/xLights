//
//  Color.cpp
//  xLights
//
//  Created by Daniel Kulp on 1/12/16.
//  Copyright © 2016 Daniel Kulp. All rights reserved.
//

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <cmath>

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
    } else if (str[0] == '0' && str[1] == 'x' && str.size() == 8 ) {
        // hexadecimal prefixed with 0x
        unsigned long tmp = strtoul(&str.c_str()[2], nullptr, 16);
        
        Set((uint8_t)(tmp >> 16),
            (uint8_t)(tmp >> 8),
            (uint8_t)tmp);
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
xlColor::xlColor(const wxColor &c) {
    red = c.Red();
    green = c.Green();
    blue = c.Blue();
    alpha = 255;
}

