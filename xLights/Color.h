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

#include <wx/colour.h>
#include <vector>
#include <cstdint>
#include <string>

#if __has_include(<simd/simd.h>)
#include <simd/simd.h>
#endif

class xlColor;
class wxString;
class ColorCurve;

class HSVValue
{
public:
    HSVValue(double h=0.0, double s=0.0, double v=0.0)
        : hue(h), saturation(s), value(v) {}
    HSVValue(const xlColor &c);
    HSVValue& operator=(const xlColor& hsv);

    double hue;
    double saturation;
    double value;
};

class HSLValue
{
public:
    HSLValue(double h=0.0, double s=0.0, double l=0.0)
        : hue(h), saturation(s), lightness(l) {}
    HSLValue(const xlColor &c);
    HSLValue& operator=(const xlColor& hsl);

    double hue;
    double saturation;
    double lightness;
};

class xlColor {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    xlColor() {
        red = green = blue = 0;
        alpha = 255;
    }
    xlColor(unsigned int rgb, bool BBGGRR = false) {
        if (BBGGRR) {
            red = rgb & 0xff;
            green = (rgb >> 8) & 0xff;
            blue = (rgb >> 16) & 0xff;
        } else {
            blue = rgb & 0xff;
            green = (rgb >> 8) & 0xff;
            red = (rgb >> 16) & 0xff;
        }
        alpha = 255;
    }
    xlColor(uint8_t r, uint8_t g, uint8_t b) {
        red = r;
        green = g;
        blue = b;
        alpha = 255;
    }
    xlColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }
    xlColor(const xlColor &rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        alpha = rgb.alpha;
    }
    xlColor(const wxColor &rgb) {
        red = rgb.Red();
        blue = rgb.Blue();
        green = rgb.Green();
        alpha = 255;
    }
    xlColor(const std::string &str) {
        SetFromString(str);
    }
    xlColor(const HSVValue &hsv) {
        fromHSV(hsv);
    }

    uint8_t Red() const { return red; }
    uint8_t Blue() const { return blue; };
    uint8_t Green() const { return green; };
    uint8_t Alpha() const { return alpha; };

    inline void ApplyMask(xlColor* mask) {
        if (mask != nullptr)
        {
            red = red & mask->red;
            green = green & mask->green;
            blue = blue & mask->blue;
        }
    }

    void Set(uint8_t r, uint8_t g, uint8_t b) {
        red = r;
        green = g;
        blue = b;
        alpha = 255;
    }
    void Set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }

    void SetAlpha(uint8_t a)
    {
        alpha = a;
    }

    bool IsNilColor() const
    {
        return (red == 0 && green == 0 && blue == 0 && alpha == 0);
    }

    bool operator==(const xlColor &rgb) const {
        return (red == rgb.red)
            && (blue == rgb.blue)
            && (green == rgb.green);  //don't compare alpha
    }
    bool operator!=(const xlColor &rgb) const {
        return (red != rgb.red)
            || (blue != rgb.blue)
            || (green != rgb.green); //don't compare alpha
    }

    xlColor&operator=(const HSVValue& hsv) {
        fromHSV(hsv);
        return *this;
    }

    xlColor&operator=(const HSLValue& hsl) {
        fromHSL(hsl);
        return *this;
    }

    HSVValue asHSV() const {
        HSVValue v;
        toHSV(v);
        return v;
    }
    void fromHSV(const HSVValue &v);
    void toHSV(HSVValue &v) const;

    HSLValue asHSL() const {
        HSLValue v;
        toHSL(v);
        return v;
    }
    void fromHSL(const HSLValue &v);
    void toHSL(HSLValue &v) const;

    xlColor ContrastColourNotBlack() const
    {
        xlColor c;
        HSLValue hsl = asHSL();
        hsl.hue += 0.5;
        if (hsl.hue > 1.0)
            hsl.hue -= 1.0;

        if (hsl.lightness > 0.8 || hsl.lightness < 0.2)
            hsl.lightness = 0.5;

        c.fromHSL(hsl);

        return c;
    }

    static xlColor NilColor()
    {
        return xlColor(0, 0, 0, 0);
    }

    xlColor ApplyBrightness(float b)
    {
        wxASSERT(b >= 0.0 && b <= 1.0);
        return xlColor(b * red, b * green, b * blue);
    }

    int Brightness() const;

    uint32_t GetRGB(bool BBGGRR = true) const
    {
        if (BBGGRR) {
            return Red() | (Green() << 8) | (Blue() << 16);
        } else {
            return Blue() | (Green() << 8) | (Red() << 16);
        }
    }
    uint32_t GetRGBA(bool BBGGRR = true) const {
        if (BBGGRR) {
            return Red() | (Green() << 8) | (Blue() << 16) | (alpha << 24);
        } else {
            return Blue() | (Green() << 8) | (Red() << 16) | (alpha << 24);
        }
    }
    /** AlphaBlend this color onto the background **/
    xlColor AlphaBlend(const xlColor &bc) const {
        if (alpha == 0) return bc;
        if (alpha == 255) return *this;
        float a = alpha;
        a /= 255; // 0 (transparent) - 1.0 (opague)
        float dr = red * a + bc.red * (1.0f - a);
        float dg = green * a + bc.green * (1.0f - a);
        float db = blue * a + bc.blue * (1.0f - a);
        return xlColor((uint8_t)dr, (uint8_t)dg, (uint8_t)db);
    }
    
    /** AlphaBlend the fg color onto this color **/
    void AlphaBlendForgroundOnto(const xlColor &fc) {
        if (fc.alpha == 0) return;
        if (fc.alpha == 255) {
            red = fc.red;
            blue = fc.blue;
            green = fc.green;
            alpha = 255;
            return;
        }
        float a = fc.alpha;
        a /= 255; // 0 (transparent) - 1.0 (opague)
        float dr = fc.red * a + red * (1.0f - a);
        float dg = fc.green * a + green * (1.0f - a);
        float db = fc.blue * a + blue * (1.0f - a);
        red = (uint8_t)dr;
        green = (uint8_t)dg;
        blue = (uint8_t)db;
    }

    /** Does this color have enough contrast to display OK on the given background **/
    bool HasSufficientContrast(const xlColor& bg) const;
    
    void SetFromString(const std::string &str);
    operator std::string() const;

    void SetFromString(const wxString &str);
    operator wxString() const;
    wxColor asWxColor() const;
    xlColor(const wxString &str) {
        SetFromString(str);
    }

#if __has_include(<simd/simd.h>)
    simd_uchar4 asChar4() const {
        return {red, green, blue, alpha};
    }
    simd::float4 asFloat4() const {
        float r = red;
        float g = green;
        float b = blue;
        float a = alpha;
        r /= 255.0f;
        g /= 255.0f;
        b /= 255.0f;
        a /= 255.0f;
        return {r, g, b, a};
    }
#endif

};

static const xlColor xlBLUE(0, 0, 255);
static const xlColor xlBLUETRANSLUCENT(0, 0, 255, 180); // higher alpha because blue/black lacks contrast
static const xlColor xlRED(255, 0, 0);
static const xlColor xlREDTRANSLUCENT(255, 0, 0, 150);
static const xlColor xlGREEN(0, 255, 0);
static const xlColor xlGREENTRANSLUCENT(0, 255, 0, 150);
static const xlColor xlBLACK(0, 0, 0);
static const xlColor xlWHITE(255, 255, 255);
static const xlColor xlWHITETRANSLUCENT(255, 255, 255, 150);
static const xlColor xlYELLOW(255, 255, 0);
static const xlColor xlYELLOWTRANSLUCENT(255, 255, 0, 150);
static const xlColor xlORANGE(255, 128, 0);
static const xlColor xlORANGETRANSLUCENT(255, 128, 0, 150);
static const xlColor xlLIGHT_GREY(211, 211, 211);
static const xlColor xlDARK_GREY(96, 96, 96);
static const xlColor xlCYAN(0, 255, 255);
static const xlColor xlCYANTRANSLUCENT(0, 255, 255, 150);
static const xlColor xlMAGENTA(255, 0, 255);
static const xlColor xlMAGENTATRANSLUCENT(255, 0, 255, 150);
static const xlColor xlCLEAR(0, 0, 0, 0);
static const xlColor xlPURPLETRANSLUCENT(0xC0, 0, 0xC0, 180);

typedef std::vector<xlColor> xlColorVector;
typedef std::vector<ColorCurve> xlColorCurveVector;
typedef std::vector<HSVValue> hsvVector;

enum ColorDisplayMode
{
    MODE_HUE,
    MODE_SATURATION,
    MODE_BRIGHTNESS,
    MODE_RED,
    MODE_GREEN,
    MODE_BLUE
};

const std::string& GetColourName(const wxColour& c);
