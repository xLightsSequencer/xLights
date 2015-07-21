
#ifndef XLIGHTS_COLOR_H
#define XLIGHTS_COLOR_H

#include <vector>

#include <wx/image.h>

class xlColor : public wxImage::RGBValue {
public:
    unsigned char alpha;
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
    xlColor(const wxColor &c) {
        red = c.Red();
        green = c.Green();
        blue = c.Blue();
        alpha = 255;
    }

    xlColor(unsigned char r, unsigned char g, unsigned char b) {
        red = r;
        green = g;
        blue = b;
        alpha = 255;
    }
    xlColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
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
    xlColor(const wxString &str) {
        SetFromString(str);
    }
    xlColor(const wxImage::RGBValue &rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        alpha = 255;
    }
    xlColor(const wxImage::HSVValue &hsv) {
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        alpha = 255;
    }
    unsigned char Red() const { return red; }
    unsigned char Blue() const { return blue; };
    unsigned char Green() const { return green; };
    unsigned char Alpha() const { return alpha; };

    void Set(unsigned char r, unsigned char g, unsigned char b) {
        red = r;
        green = g;
        blue = b;
        alpha = 255;
    }
    void Set(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }
    xlColor&operator=(const wxImage::HSVValue& hsv) {
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        alpha = 255;
        return *this;
    }
    xlColor&operator=(const wxImage::RGBValue& rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        alpha = 255;
        return *this;
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
    wxColor asWxColor() const {
        return wxColor(red, green, blue, alpha);
    }
    wxImage::HSVValue asHSV() const {
        return wxImage::RGBtoHSV(*this);
    }
    wxUint32 GetRGB(bool BBGGRR = true) const
    {
        if (BBGGRR) {
            return Red() | (Green() << 8) | (Blue() << 16);
        } else {
            return Blue() | (Green() << 8) | (Red() << 16);
        }
    }
    operator wxString() const {
        return wxString::Format("#%02x%02x%02x", red, green, blue);
    }
    /** AlphaBlend this color onto the background **/
    xlColor AlphaBlend(const xlColor &bc) const {
        if (alpha == 0) return bc;
        if (alpha == 255) return *this;
        double a = alpha;
        a /= 255; // 0 (transparent) - 1.0 (opague)
        double dr = red * a + bc.red * (1.0 - a);
        double dg = green * a + bc.green * (1.0 - a);
        double db = blue * a + bc.blue * (1.0 - a);
        return xlColor(dr, dg, db);
    }
    void SetFromString(const wxString &str) {
        alpha = 255;
        if (str.empty()) {
            red = blue = green = 0;
            return;
        } else if ( str[0] == wxT('#') && wxStrlen(str) == 7 ) {
            // hexadecimal prefixed with # (HTML syntax)
            unsigned long tmp;
            if (wxSscanf(str.wx_str() + 1, wxT("%lx"), &tmp) != 1) {
                red = blue = green = 0;
                return;
            }

            Set((unsigned char)(tmp >> 16),
                (unsigned char)(tmp >> 8),
                (unsigned char)tmp);
        } else if (str[0] == wxT('0') && str[1] == wxT('x') && wxStrlen(str) == 8 ) {
            // hexadecimal prefixed with 0x
            unsigned long tmp;
            if (wxSscanf(str.wx_str() + 2, wxT("%lx"), &tmp) != 1) {
                red = blue = green = 0;
                return;
            }

            Set((unsigned char)(tmp >> 16),
                (unsigned char)(tmp >> 8),
                (unsigned char)tmp);
        } else {
            //need to do the slower lookups
            wxColor c(str);
            red = c.Red();
            green = c.Green();
            blue = c.Blue();
        }
    }
};

static const xlColor xlBLUE(0, 0, 255);
static const xlColor xlRED(255, 0, 0);
static const xlColor xlGREEN(0, 255, 0);
static const xlColor xlBLACK(0, 0, 0);
static const xlColor xlWHITE(255, 255, 255);
static const xlColor xlYELLOW(255, 255, 0);
static const xlColor xlLIGHT_GREY(211, 211, 211);
static const xlColor xlCYAN(0, 255, 255);

typedef xlColor xlColour;
typedef std::vector<xlColor> xlColorVector;
typedef std::vector<xlColor> xlColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;
typedef wxImage::HSVValue HSVValue;

enum ColorDisplayMode
{
    MODE_HUE,
    MODE_SATURATION,
    MODE_BRIGHTNESS,
    MODE_RED,
    MODE_GREEN,
    MODE_BLUE
};

#endif


