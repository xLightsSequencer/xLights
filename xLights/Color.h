
#ifndef XLIGHTS_COLOR_H
#define XLIGHTS_COLOR_H

#include <vector>

#include <wx/image.h>

class xlColor : public wxImage::RGBValue {
public:
    xlColor() {
        red = green = blue = 0;
    }
    xlColor(int r, int g, int b) {
        red = r;
        green = g;
        blue = b;
    }
    xlColor(const xlColor &rgb) {
        red = rgb.red;
        blue = rgb.blue;
        green = rgb.green;
    }
    xlColor(const wxColour &rgb) {
        red = rgb.Red();
        blue = rgb.Blue();
        green = rgb.Green();
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
    xlColor&operator=(const wxColor&c) {
        red = c.Red();
        blue = c.Blue();
        green = c.Green();
        return *this;
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
    operator wxColor() const {
        return wxColor(red, green, blue);
    }
    wxUint32 GetRGB() const
    { return Red() | (Green() << 8) | (Blue() << 16); }
};

typedef xlColor xlColour;
typedef std::vector<xlColor> xlColorVector;
typedef std::vector<xlColor> xlColourVector;
typedef std::vector<wxColor> wxColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;
typedef wxImage::HSVValue HSVValue;

#endif


