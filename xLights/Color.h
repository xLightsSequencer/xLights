
#ifndef XLIGHTS_COLOR_H
#define XLIGHTS_COLOR_H

#include <vector>

#include <wx/image.h>

class xlColor : public wxImage::RGBValue {
public:
    xlColor() {
        red = green = blue = 0;
    }
    xlColor(unsigned char r, unsigned char g, unsigned char b) {
        red = r;
        green = g;
        blue = b;
    }
    xlColor(const xlColor &rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
    }
    xlColor(const wxString &str) {
        SetFromString(str);
    }
    xlColor(const wxImage::RGBValue &rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
    }
    xlColor(const wxImage::HSVValue &hsv) {
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
    }
    unsigned char Red() const { return red; }
    unsigned char Blue() const { return blue; };
    unsigned char Green() const { return green; };
    
    void Set(unsigned char r, unsigned char g, unsigned char b) {
        red = r;
        green = g;
        blue = b;
    }
    xlColor&operator=(const wxImage::HSVValue& hsv) {
        wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
        return *this;
    }
    xlColor&operator=(const wxImage::RGBValue& rgb) {
        return operator=(rgb);
    }
    bool operator==(const xlColor &rgb) const {
        return (red == rgb.red)
            && (blue == rgb.blue)
            && (green == rgb.green);
    }
    bool operator!=(const xlColor &rgb) const {
        return (red != rgb.red)
            || (blue != rgb.blue)
            || (green != rgb.green);
    }
    wxColor asWxColor() const {
        return wxColor(red, green, blue);
    }
    wxUint32 GetRGB() const
    { return Red() | (Green() << 8) | (Blue() << 16); }
    
    void SetFromString(const wxString &str) {
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


typedef xlColor xlColour;
typedef std::vector<xlColor> xlColorVector;
typedef std::vector<xlColor> xlColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;
typedef wxImage::HSVValue HSVValue;

#endif


