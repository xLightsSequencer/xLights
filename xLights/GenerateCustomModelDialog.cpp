/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(GenerateCustomModelDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include <wx/generic/statbmpg.h>

#include <map>
#include <list>

#include "GenerateCustomModelDialog.h"
#include "effects/VideoPanel.h"
#include "xLightsVersion.h"
#include "xLightsXmlFile.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "ExternalHooks.h"
#include "Parallel.h"

#include <log4cpp/Category.hh>

#define GCM_DISPLAYIMAGEWIDTH 800
#define GCM_DISPLAYIMAGEHEIGHT 600

#define PAGE_PREPARE 0
#define PAGE_GENERATE 1

#define PAGE_MODELTYPE 0
#define PAGE_CHOOSEVIDEO 1
#define PAGE_STARTFRAME 2
#define PAGE_MANUALIDENTIFY 3
#define PAGE_BULBIDENTIFY 4
#define PAGE_REVIEWMODEL 5

// making this smaller speeds the start scan but makes it more sensitive to when the user takes a while to start the video
#define STARTSCANSECS 15
#define FRAMEMS 50

#define LEADOFF 3000 // time after start before anything happens

#define FLAGON 600
#define FLAGOFF 400

#define NODEON 500

#define DELAYMSUNTILSAMPLE (NODEON / 2)

#define PIXCOL wxRED
#define LINECOL wxYELLOW

#define MANUAL_IMAGE_SUBTRACT (0.8f)
//#define ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
#define CUSTOM_MODEL_GENERATOR_PARALLEL
//#define EXTRA_CHECKS
//#define SHOW_PROCESSED_IMAGE
#define MODEL_SIZE_MULTIPLER (3 - 1)

// These are extra rows/columns added to ensure scaled model fits
#define MATRIX_FUDGE (MODEL_SIZE_MULTIPLER * 2)

#pragma region Flicker Free Static Bitmap

class MyGenericStaticBitmap : public wxGenericStaticBitmap {
    bool _doerasebackground;
public:
    MyGenericStaticBitmap(wxWindow *parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxStaticBitmapNameStr)
        : wxGenericStaticBitmap(parent, id, bitmap, pos, size, style, name)
    {
        _doerasebackground = true;
    };

    void OnEraseBackGround(wxEraseEvent& event)
    {
        if (_doerasebackground)
        {
            event.Skip();
        }
    };
    void SetEraseBackground(bool f)
    {
        _doerasebackground = f;
    }
    DECLARE_EVENT_TABLE()
};

// this groups all the image manipulation functions into a common class
class ProcessedImage : public wxImage {

public:
    enum class P_IMG_FRAME_TYPE {
        P_IMG_IMAGE_COLOUR,
        P_IMG_IMAGE_GREYSCALE,
        P_IMG_IMAGE_RED,
        P_IMG_IMAGE_GREEN,
        P_IMG_IMAGE_BLUE,
        P_IMG_IMAGE_MONO,
        P_IMG_IMAGE_UNKNOWN
    };

    [[nodiscard]]
    bool IsSingleChannel() const
    {
        return _imageType != P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR && _imageType != P_IMG_FRAME_TYPE::P_IMG_IMAGE_UNKNOWN;
    }

    ProcessedImage(ProcessedImage* image) :
        wxImage(image->Copy())
    {
        _imageType = image->GetImageType();
        SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
    }

    ProcessedImage(ProcessedImage* image, P_IMG_FRAME_TYPE imageType) :
        wxImage(image->Copy()), _imageType(imageType)
    {
        SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
    }

    ProcessedImage(const wxImage &image, P_IMG_FRAME_TYPE imageType) :
        wxImage(image.Copy()), _imageType(imageType) // we have to use copy or it creates a reference to the data
    {
        SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
    }

    ProcessedImage(uint32_t width, uint32_t height, unsigned char* data, P_IMG_FRAME_TYPE imageType = P_IMG_FRAME_TYPE::P_IMG_IMAGE_UNKNOWN) :
        wxImage((int)width, (int)height, data, true), _imageType(imageType)
    {
        SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
    }

    ProcessedImage(uint32_t width, uint32_t height, P_IMG_FRAME_TYPE imageType = P_IMG_FRAME_TYPE::P_IMG_IMAGE_UNKNOWN) :
        wxImage((int)width, (int)height, true), _imageType(imageType)
    {
        SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
    }

    virtual ~ProcessedImage()
    {

    }

    void UpdateFrom(const wxImage &image)
    {
        wxASSERT(GetWidth() == image.GetWidth() && GetHeight() == image.GetHeight());
        if (GetWidth() != image.GetWidth() || GetHeight() != image.GetHeight())
            return;

        memcpy(GetData(), image.GetData(), GetPixels() * (HasAlpha() ? 4 : 3));
    }

    [[nodiscard]]
    inline uint8_t GetPixel(uint32_t x, uint32_t y, uint32_t width, uint8_t channels, uint8_t* data)
    {
#ifdef EXTRA_CHECKS
        wxASSERT(x >= 0 && x < GetWidth() && width == GetWidth());
        wxASSERT(y >= 0 && y < GetHeight());
#endif
        return *(data + (y * width + x) * channels);
    }

    [[nodiscard]] inline uint8_t GetPixelC(uint32_t x, uint32_t y, uint32_t width, uint8_t channels, uint8_t* data, uint8_t ch)
    {
        #ifdef EXTRA_CHECKS
        wxASSERT(x >= 0 && x < GetWidth() && width == GetWidth());
        wxASSERT(y >= 0 && y < GetHeight());
        #endif
        return *(data + (y * width + x) * channels + ch);
    }

    inline void SetPixelC(uint32_t x, uint32_t y, uint32_t width, uint8_t channels, uint8_t* data, uint8_t r, uint8_t g, uint8_t b)
    {
#ifdef EXTRA_CHECKS
        wxASSERT(x >= 0 && x < GetWidth() && width == GetWidth());
        wxASSERT(y >= 0 && y < GetHeight());
#endif
        *(data + (y * width + x) * channels) = r;
        *(data + (y * width + x) * channels + 1) = g;
        *(data + (y * width + x) * channels + 2) = b;
    }

    inline void SetPixel(uint32_t x, uint32_t y, uint32_t width, uint8_t channels, uint8_t* data, uint8_t c)
    {
#ifdef EXTRA_CHECKS
        wxASSERT(x >= 0 && x < GetWidth() && width == GetWidth());
        wxASSERT(y >= 0 && y < GetHeight());
#endif
        *(data + (y * width + x) * channels) = c;
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
        *(data + (y * width + x) * channels + 1) = c;
        *(data + (y * width + x) * channels + 2) = c;
#endif
    }

    // gets data from a single channel greyscale image
    // this is used when i need a copy of the image as the modification changes data i later need to refer to
    uint8_t getData(uint8_t* buffer, uint32_t x, uint32_t y)
    {
#ifdef EXTRA_CHECKS
        wxASSERT(x >= 0 && x < GetWidth());
        wxASSERT(y >= 0 && y < GetHeight());
#endif
        return *(buffer + y * GetWidth() + x);
    }

    // used with a black and white image to set all pixels black unless they are white and surrounded by 8 white pixels
    void Erode()
    {
        // assumes we are working with a black/white image
        wxASSERT(_imageType == P_IMG_FRAME_TYPE::P_IMG_IMAGE_MONO);

        // take a copy of the data as this damages the data we need
        uint8_t* orig = (uint8_t*)malloc(GetPixels());

        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, orig](uint32_t i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            orig[i] = *(data);
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
        // abc
        // def
        // ghi
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, orig, incr](uint32_t j) {
#else
        for (uint32_t j = 0; j < GetPixels(); ++j) {
#endif
            uint32_t y = j / GetWidth();
            uint32_t x = j % GetWidth();
            uint8_t a = x == 0 || y == 0 ? 255 : getData(orig, x - 1, y - 1);
            uint8_t b = y == 0 ? 255 : getData(orig, x, y - 1);
            uint8_t c = x == GetWidth() - 1 || y == 0 ? 255 : getData(orig, x + 1, y - 1);
            uint8_t d = x == 0 ? 255 : getData(orig, x - 1, y);
            uint8_t e = getData(orig, x, y);
            uint8_t f = x == GetWidth() - 1 ? 255 : getData(orig, x + 1, y);
            uint8_t g = x == 0 || y == GetHeight() - 1 ? 255 : getData(orig, x - 1, y + 1);
            uint8_t h = y == GetHeight() - 1 ? 255 : getData(orig, x, y + 1);
            uint8_t i = x == GetWidth() - 1 || y == GetHeight() - 1 ? 255 : getData(orig, x + 1, y + 1);
            uint8_t* data = GetData() + j * incr;
            if (a == 0 || b == 0 || c == 0 || d == 0 || e == 0 || f == 0 || g == 0 || h == 0 || i == 0) {
                *(data) = 0;
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
                *(data + 1) = 0;
                *(data + 2) = 0;
#endif
            } else {
                *(data) = 255;
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
                *(data + 1) = 255;
                *(data + 2) = 255;
#endif
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
        free(orig);
    }

    // used with a black and white picture to set all the pixels surrounding a pixel white only if it is white.
    void Dilate()
    {
        // assumes we are working with a black/white image
        wxASSERT(_imageType == P_IMG_FRAME_TYPE::P_IMG_IMAGE_MONO);

        // take a copy of the data as this damages the data we need
        // take a copy of the data as this damages the data we need
        uint8_t* orig = (uint8_t*)malloc(GetPixels());

        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, orig](uint32_t i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            orig[i] = *(data);
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif

#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, orig](uint32_t i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint32_t y = i / GetWidth();
            uint32_t x = i % GetWidth();
            if (getData(orig, x, y) == 255) {
                if (x != 0 && y != 0)
                    SetRGB(x - 1, y - 1, 255, 255, 255);
                if (y != 0)
                    SetRGB(x, y - 1, 255, 255, 255);
                if (x != GetWidth() - 1 && y != GetHeight() - 1)
                    SetRGB(x + 1, y + 1, 255, 255, 255);
                if (x != 0)
                    SetRGB(x - 1, y, 255, 255, 255);
                SetRGB(x, y, 255, 255, 255);
                if (x != GetWidth() - 1)
                    SetRGB(x + 1, y, 255, 255, 255);
                if (x != 0 && y != GetHeight() - 1)
                    SetRGB(x - 1, y + 1, 255, 255, 255);
                if (y != GetHeight() - 1)
                    SetRGB(x, y + 1, 255, 255, 255);
                if (x != GetWidth() - 1 && y != GetHeight() - 1)
                    SetRGB(x + 1, y + 1, 255, 255, 255);
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
        free(orig);
    }

    // used with a greyscale image to generate a black and white image based on a threshold
    void Threshold(uint8_t threshold)
    {
        wxASSERT(IsSingleChannel());

        uint8_t thresholdTable[256];
        for (uint32_t i = 0; i < 256; ++i) {
            thresholdTable[i] = i >= threshold ? 255 : 0;
        }

        // assumes this is a greyscale image
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, thresholdTable](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            *(data) = thresholdTable[*data];
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
            *(data + 1) = *data;
            *(data + 2) = *data;
#endif
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif

        _imageType = P_IMG_FRAME_TYPE::P_IMG_IMAGE_MONO;
    }

    // Used with a colour image to generate a greyscale image with the colour taken from 0 - red, 1 - green or 2 - blue
    void IsolateColour(uint8_t rgb)
    {
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, rgb](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            switch (rgb) {
            case 0:
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
                *(data + 1) = *(data + 0);
                *(data + 2) = *(data + 0);
#endif
                break;
            case 1:
                *(data + 0) = *(data + 1);
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
                *(data + 2) = *(data + 1);
#endif
                break;
            case 2:
                *(data + 0) = *(data + 2);
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
                *(data + 1) = *(data + 2);
#endif
                break;
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    // if one colour is brightest then only leave that colour ... otherwise blank
    inline void ForceToRGorBPixel(uint8_t* data)
    {
        uint8_t minv = 0;
        uint8_t maxv = 0;

        for (uint8_t i = 1; i < 3; i++) {
            if (*(data + i) < *(data + minv))
                minv = i;
            if (*(data + i) > *(data + maxv))
                maxv = i;
        }
        uint8_t midv = 3 - minv - maxv;
        if (minv != maxv && *(data+minv) != *(data+midv) && *(data+maxv) != *(data+midv)) {
            // experiment ... this maximises the colour
            //*(data + maxv) = 255;
            *(data + minv) = 0;
            *(data + midv) = 0;
        }
        else {
            // if multiple values are the same then just make them all zero
            *(data + 0) = 0;
            *(data + 1) = 0;
            *(data + 2) = 0;
        }
    }

    [[nodiscard]] inline bool IsBlack(uint8_t* data, uint8_t incr, uint32_t i)
    {
        return *(data + i * incr) == 0 && *(data + i * incr + 1) == 0 && *(data + i * incr + 2) == 0;
    }

    void FillInRGB()
    {
        // now go through the same data and remove any black pixels largely surrounded by another colour
        ProcessedImage temp(this);
        uint32_t width = GetWidth();
        uint8_t* data = GetData();
        uint8_t* tempData = temp.GetData();
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, data, tempData, width](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            // if pixel is black
            if (IsBlack(data, incr, i)) {
                int32_t y = i / width;
                int32_t x = i % width;

                // we only do this for pixels not on the outside
                if (x > 0 && x < width - 1 && y > 0 && y < GetHeight() - 1) {
                    // look at the surrounding pixels
                    uint8_t r = 0;
                    uint8_t g = 0;
                    uint8_t b = 0;
                    for (int8_t x1 = -1; x1 <= 1; ++x1) {
                        for (int8_t y1 = -1; y1 <= 1; ++y1) {
                            r += GetPixelC(x + x1, y + y1, width, incr, tempData, 0);
                            g += GetPixelC(x + x1, y + y1, width, incr, tempData, 1);
                            b += GetPixelC(x + x1, y + y1, width, incr, tempData, 2);
                        }
                    }

                    if (r > g && r > b)
                        SetPixelC(x, y, width, incr, data, 255, 0, 0);
                    else if (g > r && g > b)
                        SetPixelC(x, y, width, incr, data, 0, 255, 0);
                    else if (b > r && b > g)
                        SetPixelC(x, y, width, incr, data, 0, 0, 255);
                }
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    #define MAXIMUM_LOOK_RGB 20
    void FillInRGBA()
    {
        // now go through the same data and remove any black pixels largely surrounded by another colour
        uint8_t* data = GetData();

        ProcessedImage temp(this);
        uint8_t* tempData = temp.GetData();

        uint32_t width = GetWidth();
        uint32_t height = GetHeight();

        uint8_t incr = HasAlpha() ? 4 : 3;

#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, data, tempData, width, height](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            if (IsBlack(data, incr, i)) {
                uint32_t y = i / width;
                uint32_t x = i % width;

                uint8_t n = 0x00;
                uint16_t nv = 0x00;
                uint8_t s = 0x00;
                uint16_t sv = 0x00;
                uint8_t e = 0x00;
                uint16_t ev = 0x00;
                uint8_t w = 0x00;
                uint16_t wv = 0x00;
                uint8_t all = 0x00;
                uint8_t allor = 0x00;

                for (uint8_t j = 0; all == 0 && (allor & 0x08) == 0 && j <= MAXIMUM_LOOK_RGB; ++j) {
                    // west x-
                    if (w == 0x00) {
                        if (x >= j) {
                            w |= (GetPixelC(x - j, y, width, incr, tempData, 0)) != 0 ? 0x01 : 0x00;
                            w |= (GetPixelC(x - j, y, width, incr, tempData, 1)) != 0 ? 0x02 : 0x00;
                            w |= (GetPixelC(x - j, y, width, incr, tempData, 2)) != 0 ? 0x04 : 0x00;
                            if (w == 0x01)
                                wv = GetPixelC(x - j, y, width, incr, tempData, 0);
                            else if (w == 0x02)
                                wv = GetPixelC(x - j, y, width, incr, tempData, 1);
                            else if (w == 0x02)
                                wv = GetPixelC(x - j, y, width, incr, tempData, 2);
                        } else {
                            w |= 0x08;
                        }
                    }

                    // east x+
                    if (e == 0x00) {
                        if (x < width - j - 1) {
                            e |= (GetPixelC(x + j, y, width, incr, tempData, 0)) != 0 ? 0x01 : 0x00;
                            e |= (GetPixelC(x + j, y, width, incr, tempData, 1)) != 0 ? 0x02 : 0x00;
                            e |= (GetPixelC(x + j, y, width, incr, tempData, 2)) != 0 ? 0x04 : 0x00;
                            if (e == 0x01)
                                ev = GetPixelC(x + j, y, width, incr, tempData, 0);
                            else if (e == 0x02)
                                ev = GetPixelC(x + j, y, width, incr, tempData, 1);
                            else if (e == 0x02)
                                ev = GetPixelC(x + j, y, width, incr, tempData, 2);
                        } else {
                            e |= 0x08;
                        }
                    }

                    // south y-
                    if (s == 0x00) {
                        if (y >= j) {
                            s |= (GetPixelC(x, y - j, width, incr, tempData, 0)) != 0 ? 0x01 : 0x00;
                            s |= (GetPixelC(x, y - j, width, incr, tempData, 1)) != 0 ? 0x02 : 0x00;
                            s |= (GetPixelC(x, y - j, width, incr, tempData, 2)) != 0 ? 0x04 : 0x00;
                            if (s == 0x01)
                                sv = GetPixelC(x, y - j, width, incr, tempData, 0);
                            else if (s == 0x02)
                                sv = GetPixelC(x, y - j, width, incr, tempData, 1);
                            else if (s == 0x02)
                                sv = GetPixelC(x, y - j, width, incr, tempData, 2);
                        } else {
                            s |= 0x08;
                        }
                    }

                    // north y+
                    if (n == 0x00) {
                        if (y < height - j - 1) {
                            n |= (GetPixelC(x, y + j, width, incr, tempData, 0)) != 0 ? 0x01 : 0x00;
                            n |= (GetPixelC(x, y + j, width, incr, tempData, 1)) != 0 ? 0x02 : 0x00;
                            n |= (GetPixelC(x, y + j, width, incr, tempData, 2)) != 0 ? 0x04 : 0x00;
                            if (n == 0x01)
                                nv = GetPixelC(x, y + j, width, incr, tempData, 0);
                            else if (n == 0x02)
                                nv = GetPixelC(x, y + j, width, incr, tempData, 1);
                            else if (n == 0x02)
                                nv = GetPixelC(x, y + j, width, incr, tempData, 2);
                        } else {
                            n |= 0x08;
                        }
                    }

                    allor = (n | s | e | w);
                    all = (n & s & e & w);
                }

                // done and pixel colour should be set
                uint8_t val = (ev + wv + sv + nv) / 4;
                if (all == 0x01) {
                    SetPixelC(x, y, width, incr, data, val, 0, 0);
                } else if (all == 0x02) {
                    SetPixelC(x, y, width, incr, data, 0, val, 0);
                } else if (all == 0x04) {
                    SetPixelC(x, y, width, incr, data, 0, 0, val);
                }
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif

        // need to copy the current image again
        ProcessedImage temp2(this);
        tempData = temp2.GetData();

        // go back through everything and dilate a pixel if only touches one colour n,s,e,w
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, data, tempData, width, height](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            if (IsBlack(data, incr, i)) {
                uint32_t y = i / width;
                uint32_t x = i % width;

                if (x > 1 && y > 1 && x < width - 1 && y < height - 1) {
                    uint8_t c = (GetPixelC(x - 1, y, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x + 1, y, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x, y - 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x, y + 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x - 1, y - 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x - 1, y + 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x + 1, y - 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x + 1, y + 1, width, incr, tempData, 0) != 0 ? 0x01 : 0x00) |
                                (GetPixelC(x - 1, y, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x + 1, y, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x, y - 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x, y + 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x - 1, y - 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x - 1, y + 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x + 1, y - 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x + 1, y + 1, width, incr, tempData, 1) != 0 ? 0x02 : 0x00) |
                                (GetPixelC(x - 1, y, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x + 1, y, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x, y - 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x, y + 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x - 1, y - 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x - 1, y + 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x + 1, y - 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00) |
                                (GetPixelC(x + 1, y + 1, width, incr, tempData, 2) != 0 ? 0x04 : 0x00);

                    if (c == 0x01) {
                        SetPixelC(x, y, width, incr, data, 255, 0, 0);
                    } else if (c == 0x02) {
                        SetPixelC(x, y, width, incr, data, 0, 255, 0);
                    } else if (c == 0x04) {
                        SetPixelC(x, y, width, incr, data, 0, 0, 255);
                    }
                }
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    void ForceToRGorB()
    {
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            ForceToRGorBPixel(data);
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif

        //FillInRGB();
        FillInRGBA();
    }

    inline void SaturatePixel(uint8_t* data, uint8_t sat)
    {
        uint8_t minv = 0;
        uint8_t maxv = 0;

        for (uint8_t i = 1; i < 3; i++) {
        if (*(data + i) < *(data + minv)) minv = i;
        if (*(data + i) > *(data + maxv)) maxv = i;
        }
        if (minv != maxv) {
            uint8_t midv = 3 - minv - maxv;
            if (*(data + midv) != *(data + minv)) { // this would cause divide by zero
                float r = ((int)*(data + maxv) - (int)*(data + midv)) / ((int)*(data + midv) - (int)*(data + minv));
                float s = (float)sat / 100.0;
                //*(data+max) = *(data+max);
                *(data + minv) = std::round(*(data + maxv) * (1 - s));
                *(data + midv) = std::round(*(data + maxv) / (r + 1) * s + *(data + maxv) * (1 - s));
            }
        }
    }

    // 1.0+
    void Gamma(float gamma)
    {
        wxASSERT(gamma >= 1.0);
        if (gamma == 1.0)
            return;

        uint8_t gammaTable[256];
        for (uint32_t i = 0; i < 256; ++i) {
            gammaTable[i] = (uint8_t)(std::pow(((float)i / 255.0), (1.0 / gamma)) * 255.0);
        }

        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels() * incr, [this, gammaTable](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i;
            *data = gammaTable[*data];
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

        void Saturate(uint8_t saturate)
    {
        wxASSERT(saturate >= 0 && saturate <= 100);
        if (saturate == 0)
            return;
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, saturate](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            SaturatePixel(data, saturate);
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    // used with any image to subtract a background
    void Subtract(ProcessedImage* image, float howMuch = 1.0)
    {
        if (image == nullptr || howMuch == 0)
            return;

        wxASSERT(howMuch <= 1.0 && howMuch > 0.0);

        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, image, howMuch, incr](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            uint8_t* imagedata = image->GetData() + i * incr;
            if (*(imagedata) * howMuch <= *(data)) {
                *(data) = *(data) - (*(imagedata) * howMuch);
            } else {
                *(data) = 0;
            }
            if (*(imagedata + 1) *howMuch <= *(data + 1)) {
                *(data + 1) = *(data + 1) - (*(imagedata + 1) * howMuch);
            } else {
                *(data + 1) = 0;
            }
            if (*(imagedata + 2) * howMuch <= *(data + 2)) {
                *(data + 2) = *(data + 2) - (*(imagedata + 2) * howMuch);
            } else {
                *(data + 2) = 0;
            }
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    // used with black and white images and only leaves set pixels that are white in both images
    void And(ProcessedImage* image)
    {
        wxASSERT(_imageType == P_IMG_FRAME_TYPE::P_IMG_IMAGE_MONO);

        uint8_t andTable[256] = { 0 };
        andTable[255] = 255;

        // assumes image is already black and white
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, image, incr, andTable](int i) {
            #else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
        #endif
            uint8_t* data = GetData() + i * incr;
            uint8_t* imagedata = image->GetData() + i * incr;
            *(data) = andTable[*(data) & *(imagedata)];
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
            *(data + 1) = *data;
            *(data + 2) = *data;
#endif
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
        #endif
    }

    // used with single channel images only and returns the minimum intensity
    void Min(ProcessedImage* image)
    {
        wxASSERT(IsSingleChannel());

        // assumes image is already black and white
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, image, incr](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            uint8_t* imagedata = image->GetData() + i * incr;
            *(data) = std::min(*(data), *(imagedata));
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
            *(data + 1) = *data;
            *(data + 2) = *data;
#endif
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    // used with single channel images only and returns the maximum intensity
    void Max(ProcessedImage* image)
    {
        wxASSERT(IsSingleChannel());

        // assumes image is already black and white
        uint8_t incr = HasAlpha() ? 4 : 3;
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, image, incr](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            uint8_t* imagedata = image->GetData() + i * incr;
            *(data) = std::max(*(data), *(imagedata));
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
            *(data + 1) = *data;
            *(data + 2) = *data;
#endif
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

    // applies contrast to the image
    void ApplyContrast(int contrast)
    {
        wxASSERT(IsSingleChannel());

        // Dont need to do anything if zero
        if (contrast == 0) {
            return;
        }

        float factor = (259.0 * ((float)contrast + 255.0)) / (255.0 * (259.0 - (float)contrast));
        uint8_t contrastTable[256];
        for (uint32_t i = 0; i < 256; ++i) {
            contrastTable[i] = (uint8_t)std::clamp(factor * ((float)(i) - 128.0) + 128.0, 0.0, 255.0);
        }

        uint8_t incr = HasAlpha() ? 4 : 3;

#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, GetPixels(), [this, incr, contrastTable](int i) {
#else
        for (uint32_t i = 0; i < GetPixels(); ++i) {
#endif
            uint8_t* data = GetData() + i * incr;
            *(data) = contrastTable[*data];
#ifndef ONLY_SET_RED_CHANNEL_FOR_GREYSCALE_AND_BW_IMAGES
            *(data + 1) = *data;
            *(data + 2) = *data;
#endif
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
#endif
    }

        [[nodiscard]] uint32_t GetPixels()
    {
        return GetWidth() * GetHeight();
    }

    // finding the largest difference quantum can determine when a big change happened -ve numbers means dimmer and +ve numbers means brighter
    [[nodiscard]] int32_t DifferenceQuantum(ProcessedImage* image)
    {
        wxASSERT(IsSingleChannel());

        int32_t diff = 0;

        uint8_t incr = HasAlpha() ? 4 : 3;
        for (uint32_t i = 0; i < GetPixels(); ++i) {
            diff += (int)(*(GetData() + i * incr)) - (int)(*(image->GetData() + i * incr));
        }

        return diff;
    }

    // provides a single value representing how bright the image is
    [[nodiscard]] float CalcBrightness()
    {
        wxASSERT(IsSingleChannel());

        uint8_t incr = HasAlpha() ? 4 : 3;
        unsigned char* data = GetData();
        int64_t total = 0;
        for (uint32_t i = 0; i < GetPixels() * incr; i += incr) {
            total += *(data + i);
        }

        return (float)((double)total /
                       ((double)GetPixels()) / 255.0);
    }

    // creates a new image with reduce size with the unwanted pixels removed
    // left right top bottom is how many rows/columns of pixels to remove from that edge
    [[nodiscard]] ProcessedImage* Clip(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
    {
        long width = (long)right - (long)left;
        long height = (long)top - (long)bottom;
        long adjY = GetHeight() - top;

        if (width < 0 || height < 0 || (left == 0 && right == GetWidth() && top == GetHeight() && bottom == 0))
            return new ProcessedImage(this);

        ProcessedImage* clipped = new ProcessedImage(width, height, _imageType);

#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        parallel_for(0, width * height, [this, clipped, left, adjY, width](int i) {
            #else
        for (uint32_t i = 0; i < (uint32_t)(width * height); ++i) {
#endif
            uint32_t y = i / width;
            uint32_t x = i % width;
            clipped->SetRGB(x, y, GetRed(left + x, adjY + y), GetGreen(left + x, adjY + y), GetBlue(left + x, adjY + y));
        }
#ifdef CUSTOM_MODEL_GENERATOR_PARALLEL
        );
        #endif

        return clipped;
    }

    // useful on a black and white image to count the number of white pixels
    [[nodiscard]] uint32_t CountWhite(uint32_t stopAfter)
    {
        wxASSERT(_imageType == P_IMG_FRAME_TYPE::P_IMG_IMAGE_MONO);

        // only usefull on a black and white image
        uint32_t res = 0;
        uint8_t incr = HasAlpha() ? 4 : 3;
        int b = GetPixels() * incr;
        unsigned char* data = GetData();
        for (int i = 0; i < b; i += incr) {
            if (*(data + i) == 255) {
                ++res;
                if (res > stopAfter)
                    return res;
            }
        }

        return res;
    }

    void ProcessB(uint8_t erode_dilate, uint8_t threshold, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
//#ifdef SHOW_PROCESSED_IMAGE
//        if (displayCallback != nullptr) {
//            displayCallback(this);
//        }
//#endif
        Threshold(threshold);
//#ifdef SHOW_PROCESSED_IMAGE
//        if (displayCallback != nullptr) {
//            displayCallback(this);
//        }
//#endif
        for (uint8_t i = 0; i < erode_dilate; ++i) {
            Erode();
//#ifdef SHOW_PROCESSED_IMAGE
//            if (displayCallback != nullptr) {
//                displayCallback(this);
//            }
//#endif
        }
        for (uint8_t i = 0; i < erode_dilate; ++i) {
            Dilate();
//#ifdef SHOW_PROCESSED_IMAGE
//            if (displayCallback != nullptr) {
//                displayCallback(this);
//            }
//#endif
        }
    }

    void ProcessA(int contrast, uint8_t blur, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        if (blur > 1) {
            UpdateFrom(Blur(blur));
//#ifdef SHOW_PROCESSED_IMAGE
//            if (displayCallback != nullptr) {
//                displayCallback(this);
//            }
//#endif
        }
        if (contrast != 0) {
            ApplyContrast(contrast);
//#ifdef SHOW_PROCESSED_IMAGE
//            if (displayCallback != nullptr) {
//                displayCallback(this);
//            }
//#endif
        }
    }

    // Blur is built into wxImage
    // ConvertToGreyscale is built into wxImage
    // ChangeBrightness is built into wxImage

        [[nodiscard]] P_IMG_FRAME_TYPE GetImageType() const
    {
        return _imageType;
    }

    // returns false if too many pixels are identified
    [[nodiscard]] bool WalkPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* data, uint32_t& totalX, uint32_t& totalY, uint32_t& pixelCount)
    {
        bool res = true;

        std::list<wxPoint> pixels;
        pixels.push_back(wxPoint(x, y));
        uint8_t incr = HasAlpha() ? 4 : 3;

        while (pixels.size() != 0 && pixels.size() < 1000) {
            std::list<wxPoint>::iterator it = pixels.begin();

            if (GetPixel(it->x, it->y, width, incr, data) > 0) {
                SetPixel(it->x, it->y, width, incr, data, 0);
                ++pixelCount;
                totalX += it->x;
                totalY += it->y;

                if (it->x > 0 && it->y > 0 && GetPixel(it->x - 1, it->y - 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x - 1, it->y - 1));
                }
                if (it->x > 0 && it->y < height - 1 && GetPixel(it->x - 1, it->y + 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x - 1, it->y + 1));
                }
                if (it->x < width - 1 && it->y > 0 && GetPixel(it->x + 1, it->y - 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x + 1, it->y - 1));
                }
                if (it->x < width - 1 && it->y < height - 1 && GetPixel(it->x + 1, it->y + 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x + 1, it->y + 1));
                }
                if (it->y > 0 && GetPixel(it->x, it->y - 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x, it->y - 1));
                }
                if (it->y < height - 1 && GetPixel(it->x, it->y + 1, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x, it->y + 1));
                }
                if (it->x > 0 && GetPixel(it->x - 1, it->y, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x - 1, it->y));
                }
                if (it->x < width - 1 && GetPixel(it->x + 1, it->y, width, incr, data) > 0) {
                    pixels.push_back(wxPoint(it->x + 1, it->y));
                }
            }
            pixels.pop_front();
        }

        if (pixels.size() != 0) {
            res = false;
        }

        return res;
    }

    [[nodiscard]] static uint32_t PixelDistance(const wxPoint& pt1, const wxPoint& pt2)
    {
       return sqrt((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
    }

    // finds all possible pixels in frame
    [[nodiscard]] std::list<std::pair<wxPoint, uint32_t>> FindPixels(uint32_t pixel, uint32_t minSeparation, std::function<void(float)> progressCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

        std::list<std::pair<wxPoint, uint32_t>> res;
        uint32_t width = GetWidth();
        uint8_t incr = HasAlpha() ? 4 : 3;

        logger_gcm.debug("Found pixels:");

        for (uint32_t x = 0; x < width; ++x) {
            for (uint32_t y = 0; y < (uint32_t)GetHeight(); ++y) {
                if (GetPixel(x, y, width, incr, GetData()) == 255) {
                    uint32_t totalX = 0;
                    uint32_t totalY = 0;
                    uint32_t pixelCount = 0;
                    if (!WalkPixels(x, y, width, GetHeight(), GetData(), totalX, totalY, pixelCount)) {
                        // we are not going to find anything
                        res.clear();
                        return res;
                    }

                    bool okToAdd = true;
                    wxPoint pt(totalX / pixelCount,totalY / pixelCount);
                    if (minSeparation > 0) {
                        for (const auto& it : res) {
                            if (ProcessedImage::PixelDistance(it.first, pt) < minSeparation) {
                                okToAdd = false;
                                break;
                            }
                        }
                    }

                    if (okToAdd) {
                        res.push_back({ pt, pixel });
                        logger_gcm.debug("   %d: %d, %d", res.back().second, res.back().first.x, res.back().first.y);
                    }
                }
            }
            if (progressCallback != nullptr) {
                progressCallback((float)(x + 1) / (float)width);
            }
        }

        return res;
    }

    // returns -1, -1 if nothing is found
    // Finds the centre of the largest white patch in the image
    [[nodiscard]] wxPoint FindPixel()
    {
        wxPoint res = wxPoint(-1, -1);
        uint32_t maxSize = 0;
        uint32_t width = GetWidth();
        uint8_t incr = HasAlpha() ? 4 : 3;

        for (uint32_t x = 0; x < width; ++x) {
            for (uint32_t y = 0; y < (uint32_t)GetHeight(); ++y) {
                if (GetPixel(x, y, width, incr, GetData()) == 255) {
                    uint32_t totalX = 0;
                    uint32_t totalY = 0;
                    uint32_t pixelCount = 0;
                    if (!WalkPixels(x, y, width, GetHeight(), GetData(), totalX, totalY, pixelCount)) {
                        // we are not going to find anything
                        return wxPoint(-1, -1);
                    }

                    if (pixelCount > maxSize) {
                        maxSize = pixelCount;
                        res = wxPoint(totalX / pixelCount, totalY / pixelCount);
                    }
                }
            }
        }

        return res;
    }

    // Finds the centre of all white patches
    [[nodiscard]] wxPoint FindPixelA()
    {
        wxPoint res = wxPoint(-1, -1);
        uint32_t width = GetWidth();
        uint8_t incr = HasAlpha() ? 4 : 3;

        uint32_t totalX = 0;
        uint32_t totalY = 0;
        uint32_t count = 0;

        for (uint32_t x = 0; x < width; ++x) {
            for (uint32_t y = 0; y < (uint32_t)GetHeight(); ++y) {
                if (GetPixel(x, y, width, incr, GetData()) == 255) {
                    count++;
                    totalX += x;
                    totalY += y;
                }
            }
        }

        if (count != 0) {
            res = wxPoint(totalX / count, totalY / count);
        }

        return res;
    }

    protected:

        P_IMG_FRAME_TYPE _imageType = P_IMG_FRAME_TYPE::P_IMG_IMAGE_UNKNOWN;
};

BEGIN_EVENT_TABLE(MyGenericStaticBitmap, wxGenericStaticBitmap)
EVT_ERASE_BACKGROUND(MyGenericStaticBitmap::OnEraseBackGround)
END_EVENT_TABLE()

#pragma endregion Flicker Free Static Bitmap

class VideoFrame
{
public:
    enum class VIDEO_FRAME_TYPE {
        VFT_IMAGE_OFF,
        VFT_IMAGE_START1,
        VFT_IMAGE_START2,
        VFT_IMAGE_PIXEL,
        VFT_IMAGE_MULTI,
        VFT_IMAGE_TEMP
    };

    void PrepareImages(bool processRGB, float gamma, uint8_t saturate, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        if (gamma != 1.0) {
            _rawFrame->Gamma(gamma);
        }
        if (saturate != 0) {
            _rawFrame->Saturate(saturate);
        }

        if (processRGB) {
            //            if (displayCallback != nullptr) {
            //                displayCallback(_rawFrame);
            //            }
            _rawFrame->ForceToRGorB();
            //            if (displayCallback != nullptr) {
            //                displayCallback(_rawFrame);
            //            }

            _redFrame = new ProcessedImage(_rawFrame, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_RED);
            _redFrame->IsolateColour(0);
            _greenFrame = new ProcessedImage(_rawFrame, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_GREEN);
            _greenFrame->IsolateColour(1);
            _blueFrame = new ProcessedImage(_rawFrame, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_BLUE);
            _blueFrame->IsolateColour(2);
        } else {
            _greyscaleFrame = new ProcessedImage(_rawFrame, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_GREYSCALE);
            _greyscaleFrame->ConvertToGreyscale();
        }
    }

    VideoFrame(ProcessedImage* image, uint32_t timestamp, bool isRGB, VIDEO_FRAME_TYPE frameType = VIDEO_FRAME_TYPE::VFT_IMAGE_TEMP) :
        _frameType(frameType), _isRGB(isRGB), _timestamp(timestamp)
    {
        _rawFrame = new ProcessedImage(image);
    }

    VideoFrame(uint32_t width, uint32_t height)
    {
        _rawFrame = new ProcessedImage(800, 600);
    }

    virtual ~VideoFrame()
    {
        if (_rawFrame != nullptr)
            delete _rawFrame;
        if (_greyscaleFrame != nullptr)
            delete _greyscaleFrame;
        if (_redFrame != nullptr)
            delete _redFrame;
        if (_greenFrame != nullptr)
            delete _greenFrame;
        if (_blueFrame != nullptr)
            delete _blueFrame;
    }

    [[nodiscard]] ProcessedImage* GetColourImage() const
    {
        return _rawFrame;
    }

    [[nodiscard]] ProcessedImage* GetGreyscaleImage() const
    {
        wxASSERT(!_isRGB);
        return _greyscaleFrame;
    }

    [[nodiscard]] ProcessedImage* GetRedImage() const
    {
        wxASSERT(_isRGB);
        return _redFrame;
    }

    [[nodiscard]] ProcessedImage* GetGreenImage() const
    {
        wxASSERT(_isRGB);
        return _greenFrame;
    }

    [[nodiscard]] ProcessedImage* GetBlueImage() const
    {
        wxASSERT(_isRGB);
        return _blueFrame;
    }

    void RemoveBackground(VideoFrame* offFrame, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        //#ifdef SHOW_PROCESSED_IMAGE
        //        if (displayCallback != nullptr)
        //            displayCallback(offFrame->GetColourImage());
        //#endif

        //#ifdef SHOW_PROCESSED_IMAGE
        //        if (displayCallback != nullptr)
        //            displayCallback(_rawFrame);
        //#endif
        _rawFrame->Subtract(offFrame->GetColourImage());
        //#ifdef SHOW_PROCESSED_IMAGE
        //        if (displayCallback != nullptr)
        //            displayCallback(_rawFrame);
        //#endif
    }

    [[nodiscard]] bool IsOk() const
    {
        return _rawFrame != nullptr && _rawFrame->IsOk();
    }

    [[nodiscard]] uint32_t GetWidth() const
    {
        if (_rawFrame == nullptr)
            return 0;
        return _rawFrame->GetWidth();
    }

    [[nodiscard]] uint32_t GetHeight() const
    {
        if (_rawFrame == nullptr)
            return 0;
        return _rawFrame->GetHeight();
    }

    [[nodiscard]] uint32_t GetTimestamp() const
    {
        return _timestamp;
    }

    uint32_t SetFrameDelta(VideoFrame* other)
    {
        _frameDelta = _greyscaleFrame->DifferenceQuantum(other->GetGreyscaleImage());
        return _frameDelta;
    }

    [[nodiscard]] int32_t GetFrameDelta() const
    {
        return _frameDelta;
    }

    void ProcessImage(ProcessedImage* img, int contrast, uint8_t blur, uint8_t erode_dilate, uint8_t threshold, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        img->ProcessA(contrast, blur, displayCallback);
        img->ProcessB(erode_dilate, threshold, displayCallback);
    }

    void ProcessImageA(ProcessedImage* img, int contrast, uint8_t blur, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        img->ProcessA(contrast, blur, displayCallback);
    }

    void ProcessImageB(ProcessedImage* img, uint8_t erode_dilate, uint8_t threshold, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        img->ProcessB(erode_dilate, threshold, displayCallback);
    }

    // returns a new frame with the processing done
    [[nodiscard]] VideoFrame* Process(uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, uint8_t erode_dilate, uint8_t threshold, float gamma, uint8_t saturate, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        auto frame = new VideoFrame(_rawFrame->Clip(cropLeft, cropRight, cropTop, cropBottom), _timestamp, _isRGB, _frameType);
        frame->PrepareImages(_isRGB, gamma, saturate, displayCallback);

        if (_isRGB) {
            ProcessImage(frame->_redFrame, contrast, blur, erode_dilate, threshold, displayCallback);
            ProcessImage(frame->_greenFrame, contrast, blur, erode_dilate, threshold, displayCallback);
            ProcessImage(frame->_blueFrame, contrast, blur, erode_dilate, threshold, displayCallback);
        } else {
            ProcessImage(frame->_greyscaleFrame, contrast, blur, erode_dilate, threshold, displayCallback);
        }
        return frame;
    }

    [[nodiscard]] VideoFrame* ProcessA(uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, float gamma, uint8_t saturate, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        auto frame = new VideoFrame(_rawFrame->Clip(cropLeft, cropRight, cropTop, cropBottom), _timestamp, _isRGB, _frameType);
        frame->PrepareImages(_isRGB, gamma, saturate, displayCallback);
        if (_isRGB) {
            ProcessImageA(frame->_redFrame, contrast, blur, displayCallback);
            ProcessImageA(frame->_greenFrame, contrast, blur, displayCallback);
            ProcessImageA(frame->_blueFrame, contrast, blur, displayCallback);
        } else {
            ProcessImageA(frame->_greyscaleFrame, contrast, blur, displayCallback);
        }
        return frame;
    }

    void ProcessB(uint8_t erode_dilate, uint8_t threshold, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        if (_isRGB) {
            ProcessImageB(_redFrame, erode_dilate, threshold, displayCallback);
            ProcessImageB(_greenFrame, erode_dilate, threshold, displayCallback);
            ProcessImageB(_blueFrame, erode_dilate, threshold, displayCallback);
        } else {
            ProcessImageB(_greyscaleFrame, erode_dilate, threshold, displayCallback);
        }
    }

protected:
    ProcessedImage* _rawFrame = nullptr;
    ProcessedImage* _greyscaleFrame = nullptr;
    ProcessedImage* _redFrame = nullptr;
    ProcessedImage* _greenFrame = nullptr;
    ProcessedImage* _blueFrame = nullptr;
    VIDEO_FRAME_TYPE _frameType = VIDEO_FRAME_TYPE::VFT_IMAGE_TEMP;
    bool _isRGB = false;
    uint32_t _timestamp = 0;
    int32_t _frameDelta = 0;
};

[[nodiscard]] bool VideoFrameDeltaCompare(const VideoFrame* v1, const VideoFrame* v2)
{
    return std::abs(v1->GetFrameDelta()) > std::abs(v2->GetFrameDelta());
}

[[nodiscard]] bool VideoFrameTimestampCompare(const VideoFrame* v1, const VideoFrame* v2)
{
    return v1->GetTimestamp() < v2->GetTimestamp();
}

[[nodiscard]] bool LightsCompare(const std::pair<wxPoint, uint32_t>& v1, const std::pair<wxPoint, uint32_t>& v2)
{
    return v1.second < v2.second;
}

class CustomModelGenerator
{
protected:
    std::string _filename;
    VideoReader* _vr = nullptr;
    uint32_t _startMS = 0;              // this is the timestamp of the _start1 image
    VideoFrame* _offFrame = nullptr;    // used to hold an image of all lights off ... this can be subtracted by future images
    VideoFrame* _startFrame1 = nullptr; // these are the two start images
    VideoFrame* _startFrame2 = nullptr;
    VideoFrame* _firstFrame = nullptr;
    std::list<VideoFrame*> _frames; // this is a collection of raw video stills containing the snapshots of the pixels
                                    // while this uses some memory to hold it is faster than continually re-reading the
                                    // video as the video is often processed many times. The uint32_t is the video
                                    // timestamp of the frame
    std::vector<VideoFrame*> _processedFrames;

    // used during video creation
    wxDateTime _startOutputTime;
    bool _outputting = false;

public:
    // this is used when you just need the generator to run the sequence
    CustomModelGenerator()
    {
    }

    CustomModelGenerator(const std::string& filename) :
        _filename(filename)
    {
        SetVideo(filename);
    }

    void Reset()
    {
        if (_firstFrame != nullptr) {
            delete _firstFrame;
            _firstFrame = nullptr;
        }
        if (_offFrame != nullptr) {
            delete _offFrame;
            _offFrame = nullptr;
        }
        if (_startFrame1 != nullptr) {
            delete _startFrame1;
            _startFrame1 = nullptr;
        }
        if (_startFrame2 != nullptr) {
            delete _startFrame2;
            _startFrame2 = nullptr;
        }

        while (_processedFrames.size() > 0) {
            delete _processedFrames.back();
            _processedFrames.pop_back();
        }

        while (_frames.size() > 0) {
            delete _frames.back();
            _frames.pop_back();
        }

        if (_vr != nullptr) {
            delete _vr;
            _vr = nullptr;
        }
    }

    void SetVideo(const std::string& filename)
    {
        Reset();
        _filename = filename;
        if (_filename != "") {
            _vr = new VideoReader(_filename, 800, 600, true, false, false);
            _firstFrame = ReadFrame(_vr->GetNextFrame(0), 0, false);
        } else {
            _firstFrame = new VideoFrame(800, 600);
        }
    }

    void SetImage(const std::string& filename)
    {
        Reset();
        _filename = filename;
        if (_filename != "") {
            _firstFrame = new VideoFrame(new ProcessedImage(wxImage(_filename), ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR), 0, false);
        } else {
            _firstFrame = new VideoFrame(800, 600);
        }
    }

    virtual ~CustomModelGenerator()
    {
        Reset();
    }

    [[nodiscard]] VideoFrame* GetFirstFrame() const
    {
        return _firstFrame;
    }

    [[nodiscard]] VideoFrame* GetActualStartFrame() const
    {
        return _startFrame1;
    }

    [[nodiscard]] VideoFrame* GetStartFrame() const
    {
        if (_startFrame1 != nullptr)
            return _startFrame1;
        return _firstFrame;
    }

    [[nodiscard]] uint32_t GetFirstNodeFrameTime() const
    {
        if (_startFrame1 == nullptr)
            return 0;
        return _startFrame1->GetTimestamp() + FLAGON + FLAGOFF + FLAGON + FLAGOFF; // we dont need to add node/2 as this was done then grabbing start frame
    }

#pragma region Run Sequence
    [[nodiscard]] uint32_t GetBits(uint32_t numPixels) const
    {
        uint32_t count = 0;
        uint32_t p = numPixels;
        while (p != 0) {
            p = p / 3;
            ++count;
        }
        return count + 2;
    }

    [[nodiscard]] uint32_t GetSequenceRunTime(uint32_t numPixels) const
    {
        return LEADOFF + FLAGON + FLAGOFF + FLAGON + FLAGOFF + GetBits(numPixels) * NODEON;
    }

    [[nodiscard]] uint32_t GetManualSequenceRunTime(uint32_t numPixels) const
    {
        return LEADOFF + FLAGON + FLAGOFF + FLAGON + FLAGOFF + numPixels * NODEON;
    }

    void StartBulbOutput(OutputManager* outputManager, xLightsFrame* frame)
    {
        _startOutputTime = wxDateTime::UNow();

        // Remember our outputting state
        _outputting = outputManager->IsOutputting();
        if (!_outputting) {
            frame->EnableOutputs();
        }
    }

    void EndBulbOutput(OutputManager* outputManager, xLightsFrame* frame)
    {
        outputManager->AllOff();
        if (!_outputting) {
            frame->DisableOutputs();
        }
    }

    // turns on the nominated bulbs
    void SetBulbs(OutputManager* outputManager, bool nodes, int count, int startch, int node, int ms, uint8_t intensity)
    {
        static log4cpp::Category& logger_pcm = log4cpp::Category::getInstance(std::string("log_prepcustommodel"));

        // node is out of range ... odd
        if (node > count) {
            logger_pcm.debug("SetBulbs failed. Node %d is greater than number of nodes %d", node, count);
            return;
        }

        wxTimeSpan ts = wxDateTime::UNow() - _startOutputTime;
        long curtime = ts.GetMilliseconds().ToLong();
        outputManager->StartFrame(curtime);

        if (node != -1) {
            if (nodes) {
                for (uint32_t j = 0; j < count; ++j) {
                    if (node == j) {
                        for (uint32_t i = 0; i < 3; ++i) {
                            outputManager->SetOneChannel(startch + j * 3 + i - 1, intensity);
                        }
                    } else {
                        for (uint32_t i = 0; i < 3; ++i) {
                            outputManager->SetOneChannel(startch + j * 3 + i - 1, 0);
                        }
                    }
                }
            } else {
                for (uint32_t j = 0; j < count; ++j) {
                    if (j == node) {
                        outputManager->SetOneChannel(startch + j - 1, intensity);
                    } else {
                        outputManager->SetOneChannel(startch + j - 1, 0);
                    }
                }
            }
        } else {
            if (nodes) {
                for (uint32_t j = 0; j < count; ++j) {
                    for (uint32_t i = 0; i < 3; ++i) {
                        outputManager->SetOneChannel(startch + j * 3 + i - 1, intensity);
                    }
                }
            } else {
                for (uint32_t j = 0; j < count; ++j) {
                    outputManager->SetOneChannel(startch + j - 1, intensity);
                }
            }
        }

        outputManager->EndFrame();

        wxTimeSpan tsx = wxDateTime::UNow() - _startOutputTime;
        long curtimex = tsx.GetMilliseconds().ToLong();
        while (curtimex - curtime < ms) {
            wxYield();
            wxMicroSleep(5000);
            tsx = wxDateTime::UNow() - _startOutputTime;
            curtimex = tsx.GetMilliseconds().ToLong();
        }
    }

    [[nodiscard]] std::string convertToBase3(uint32_t number, uint32_t min_digits)
    {
        std::string res;
        uint32_t total = 0;
        while (number > 0) {
            uint32_t r = number % 3;
            res = std::to_string(r) + res;
            total += r;
            number = number / 3;
        }

        uint32_t check = 2 - (total % 3);
        res = res + std::to_string(check);
        check = (check + 1) % 3;
        res = res + std::to_string(check);

        while (res.size() < min_digits) {
            res = "0" + res;
        }

        return res;
    }

    // turns on the nominated bulbs using a base ... so when the bulb number has that bit set then the bulb turns on
    void SetBulbsUsingBase3(OutputManager* outputManager, bool nodes, int count, int startch, uint32_t digit, uint32_t bits, int ms, uint8_t intensity)
    {
        wxASSERT(digit < bits);

        wxTimeSpan ts = wxDateTime::UNow() - _startOutputTime;
        long curtime = ts.GetMilliseconds().ToLong();
        outputManager->StartFrame(curtime);

        if (nodes) {
            for (uint32_t j = 0; j < count; ++j) {
                auto value = convertToBase3(j + 1, bits)[digit];
                switch (value) {
                case '0':
                    outputManager->SetOneChannel(startch + j * 3 + 0 - 1, intensity);
                    outputManager->SetOneChannel(startch + j * 3 + 1 - 1, 0);
                    outputManager->SetOneChannel(startch + j * 3 + 2 - 1, 0);
                    break;
                case '1':
                    outputManager->SetOneChannel(startch + j * 3 + 0 - 1, 0);
                    outputManager->SetOneChannel(startch + j * 3 + 1 - 1, intensity);
                    outputManager->SetOneChannel(startch + j * 3 + 2 - 1, 0);
                    break;
                case '2':
                    outputManager->SetOneChannel(startch + j * 3 + 0 - 1, 0);
                    outputManager->SetOneChannel(startch + j * 3 + 1 - 1, 0);
                    outputManager->SetOneChannel(startch + j * 3 + 2 - 1, intensity);
                    break;
                default:
                    break;
                }
            }
        } else {
            wxASSERT(false);
        }

        outputManager->EndFrame();

        wxTimeSpan tsx = wxDateTime::UNow() - _startOutputTime;
        long curtimex = tsx.GetMilliseconds().ToLong();
        while (curtimex - curtime < ms) {
            wxYield();
            wxMicroSleep(5000);
            tsx = wxDateTime::UNow() - _startOutputTime;
            curtimex = tsx.GetMilliseconds().ToLong();
        }
    }

    void RunSequence(xLightsFrame* frame, OutputManager* outputManager, bool nodes, uint32_t startChannel, uint32_t numPixels, uint8_t intensity, std::function<void(float)> progressCallback = nullptr)
    {
        StartBulbOutput(outputManager, frame);

        auto totalTime = GetSequenceRunTime(numPixels);

        // 3.0 seconds off 0.5 seconds on ... 0.5 seconds off ... 0.5 second on ... 0.5 seconds off
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, LEADOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, FLAGON, intensity);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, FLAGOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, FLAGON, intensity);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, FLAGOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);

        // then in turn each node on for 0.5 seconds
        for (uint32_t i = 0; i < GetBits(numPixels) && !wxGetKeyState(WXK_ESCAPE); ++i) {
            // logger_pcm.debug("%d of %d", i, count);
            SetBulbsUsingBase3(outputManager, nodes, numPixels, startChannel, i, GetBits(numPixels), NODEON, intensity);
            if (progressCallback != nullptr)
                progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        }
        SetBulbs(outputManager, nodes, numPixels, startChannel, -1, 0, 0);

        if (progressCallback != nullptr)
            progressCallback(1.0);

        EndBulbOutput(outputManager, frame);
    }

    void RunManualSequence(xLightsFrame* frame, OutputManager* outputManager, uint32_t startChannel, uint32_t numPixels, uint8_t intensity, std::function<void(float)> progressCallback = nullptr)
    {
        StartBulbOutput(outputManager, frame);

        auto totalTime = GetManualSequenceRunTime(numPixels);

        // 3.0 seconds off 0.5 seconds on ... 0.5 seconds off ... 0.5 second on ... 0.5 seconds off
        SetBulbs(outputManager, true, numPixels, startChannel, -1, LEADOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, true, numPixels, startChannel, -1, FLAGON, intensity);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, true, numPixels, startChannel, -1, FLAGOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, true, numPixels, startChannel, -1, FLAGON, intensity);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        SetBulbs(outputManager, true, numPixels, startChannel, -1, FLAGOFF, 0);
        if (progressCallback != nullptr)
            progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);

        // then in turn each node on for 0.5 seconds
        for (uint32_t i = 0; i < numPixels && !wxGetKeyState(WXK_ESCAPE); ++i) {
            // logger_pcm.debug("%d of %d", i, count);
            SetBulbs(outputManager, true, numPixels, startChannel, i, NODEON, intensity);
            if (progressCallback != nullptr)
                progressCallback((float)(wxDateTime::UNow() - _startOutputTime).GetMilliseconds().ToLong() / (float)totalTime);
        }
        SetBulbs(outputManager, true, numPixels, startChannel, -1, 0, 0);

        if (progressCallback != nullptr)
            progressCallback(1.0);

        EndBulbOutput(outputManager, frame);
    }
#pragma endregion

    void AddFrame(const wxImage& img, uint32_t timestamp, VideoFrame::VIDEO_FRAME_TYPE type = VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_PIXEL)
    {
        switch (type) {
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_OFF:
            _offFrame = new VideoFrame(new ProcessedImage(img, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR), timestamp, true, type);
            break;
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_START1:
            _startFrame1 = new VideoFrame(new ProcessedImage(img, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR), timestamp, true, type);
            _startMS = timestamp;
            break;
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_START2:
            _startFrame2 = new VideoFrame(new ProcessedImage(img, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR), timestamp, true, type);
            break;
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_PIXEL:
            _frames.push_back(new VideoFrame(new ProcessedImage(img, ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_COLOUR), timestamp, true, type));
            break;
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_MULTI:
        case VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_TEMP:
            break;
        }
    }

    void RemoveBackgroundFromFrames(std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        for (auto& it : _frames) {
            it->RemoveBackground(_offFrame, displayCallback);
        }
    }

    [[nodiscard]] VideoFrame* ReadFrame(AVFrame* frame, uint32_t timestamp, bool processRGB)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

        ProcessedImage* img = nullptr;
        if (frame != nullptr) {
            img = new ProcessedImage(frame->width, frame->height, (unsigned char*)frame->data[0]);
        } else {
            logger_gcm.info("Video returned no frame.");
            if (_startFrame1 != nullptr && _startFrame1->IsOk()) {
                img = new ProcessedImage(_startFrame1->GetWidth(), _startFrame1->GetHeight());
            } else {
                img = new ProcessedImage(800, 600);
            }
        }

        VideoFrame* videoFrame = new VideoFrame(img, timestamp, processRGB, VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_TEMP);

        delete img;

        return videoFrame;
    }

    // call back used whenever we have an image the UI might want to display
    [[nodiscard]] bool FindStartFrames(std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr, std::function<void(float)> progressCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        bool res = false;
        bool abort = false;

        // read the first STARTSCANSECS seconds of video looking for 2 frames a frame apart with large changes
        std::list<VideoFrame*> startScan;
        for (uint32_t ms = 0; ms < STARTSCANSECS * 1000 && !abort && ms < _vr->GetLengthMS(); ms += FRAMEMS) {
            auto img = ReadFrame(_vr->GetNextFrame(ms), ms, false);
            img->PrepareImages(false, 1.0, 0); // prepare as greyscale
            startScan.push_back(img);
            if (displayCallback != nullptr)
                displayCallback(img->GetColourImage(), nullptr);
            if (progressCallback != nullptr)
                progressCallback(((float)ms * 0.7f) / ((float)STARTSCANSECS * 1000.0f));
            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        if (!abort) {
            // work out all the frame deltas ... we want the biggest with the right gap
            logger_gcm.debug("Working out frame deltas");
            auto it1 = startScan.begin();
            auto it2 = it1;
            ++it2;
            ++it2;
            uint32_t cnt = 0;
            while (it2 != startScan.end()) {
                (*it2)->SetFrameDelta(*it1);
                logger_gcm.debug("Frame %u delta %d", (*it2)->GetTimestamp(), (*it2)->GetFrameDelta());
                ++it1;
                ++it2;
                ++cnt;
                if (progressCallback != nullptr)
                    progressCallback(0.7f + (0.2f * (float)cnt) / (float)startScan.size());
            }
            startScan.sort(VideoFrameDeltaCompare);

            // find 4 high events separated by flag on duration
            // find 2 high events separated by flag off duration
            // only look through first 20 items as the frames should be there

            logger_gcm.debug("Looking through the largest deltas");
            std::vector<VideoFrame*> candidates;
            it1 = startScan.begin();
            for (uint32_t j = 0; j < 20 && !abort && (*it1)->GetFrameDelta() != 0; ++j) {
                // logger_gcm.debug("Frame %u delta %u", (*it1)->GetTimestamp(), (*it1)->GetFrameDelta());
                it2 = it1;
                ++it2;
                for (uint32_t i = 0; i < 20 && !abort && (*it2)->GetFrameDelta() != 0; ++i) {
                    // two signs must be different and separation must be right
                    auto diff = std::abs(std::abs((long)(*it1)->GetTimestamp() - (long)(*it2)->GetTimestamp()) - FLAGON);
                    auto sign = ((*it1)->GetFrameDelta() / std::abs((*it1)->GetFrameDelta())) *
                                ((*it2)->GetFrameDelta() / std::abs((*it2)->GetFrameDelta()));
                    if (sign < 0 && diff <= FRAMEMS) {
                        auto cand = it2;
                        if ((*it1)->GetTimestamp() < (*it2)->GetTimestamp()) {
                            cand = it1;
                        }
                        // candidate frames must increase in brightness
                        if ((*cand)->GetFrameDelta() > 0) {
                            // check no close candidate frame is already in our list
                            bool present = false;
                            for (const auto& it : candidates) {
                                if (std::abs((long)it->GetTimestamp() - (long)(*cand)->GetTimestamp()) < 150) {
                                    present = true;
                                    break;
                                }
                            }
                            if (!present) {
                                logger_gcm.debug("Candidate %u - %u, %ld", (*it1)->GetTimestamp(), (*it2)->GetTimestamp(), std::abs((long)(*it1)->GetTimestamp() - (long)(*it2)->GetTimestamp()) - FLAGON);
                                candidates.push_back(*cand);
                            }
                        }
                    }
                    ++it2;
                    abort |= wxGetKeyState(WXK_ESCAPE);
                }
                ++it1;
            }

            if (!abort) {
                logger_gcm.info("We found %lu start flashes.", candidates.size());

                std::sort(candidates.begin(), candidates.end(), VideoFrameTimestampCompare);
                startScan.sort(VideoFrameTimestampCompare);

                if (candidates.size() >= 2) {
                    // make sure that the first and second flash are FLASHON + FLASHOFF separated and are both increases in brightness
                    if (candidates[0]->GetFrameDelta() > 0 && candidates[1]->GetFrameDelta() > 0 && std::abs((long)candidates[1]->GetTimestamp() - (long)candidates[0]->GetTimestamp() - FLAGON - FLAGOFF) <= FRAMEMS) {
                        // now find the blank frame
                        uint32_t blankFrameTime = candidates[0]->GetTimestamp() + FLAGON + (FLAGOFF / 2);

                        // I dont actually want to add the frame itself as it is when the flash started ... I really want to find the frame NODEON/2 further along ... so lets go find it
                        bool repl1 = false;
                        bool repl2 = false;
                        for (const auto& it : startScan) {
                            if (!repl1 && it->GetTimestamp() >= candidates[0]->GetTimestamp() + DELAYMSUNTILSAMPLE) {
                                repl1 = true;
                                candidates[0] = it;
                            }
                            if (!repl2 && it->GetTimestamp() >= candidates[1]->GetTimestamp() + DELAYMSUNTILSAMPLE) {
                                repl2 = true;
                                candidates[1] = it;
                            }
                        }

                        AddFrame(*candidates[0]->GetColourImage(), candidates[0]->GetTimestamp(), VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_START1);
                        AddFrame(*candidates[1]->GetColourImage(), candidates[1]->GetTimestamp(), VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_START2);

                        for (const auto& it : startScan) {
                            if (it->GetTimestamp() >= blankFrameTime) {
                                AddFrame(*it->GetColourImage(), it->GetTimestamp(), VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_OFF);

                                //#ifdef SHOW_PROCESSED_IMAGE
                                //                        if (displayCallback != nullptr)
                                //                            displayCallback(_offFrame->GetColourImage());
                                //#endif

                                res = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        while (startScan.size() > 0) {
            delete startScan.back();
            startScan.pop_back();
        }

        return res;
    }

    [[nodiscard]] VideoFrame* GetBlankFrame() const
    {
        return _offFrame;
    }

    [[nodiscard]] ProcessedImage* ReadVideoFrame(uint32_t timeMS)
    {
        auto frame = _vr->GetNextFrame(timeMS);
        ProcessedImage* img = nullptr;
        if (frame != nullptr) {
            img = new ProcessedImage(frame->width, frame->height, (unsigned char*)frame->data[0]);
        } else {
            if (_startFrame1 != nullptr && _startFrame1->IsOk()) {
                img = new ProcessedImage(_startFrame1->GetWidth(), _startFrame1->GetHeight());
            } else {
                img = new ProcessedImage(800, 600);
            }
        }
        return img;
    }

    // watch the video from the start recording all the frames that should have pixels ... dont apply any fancy processing
    [[nodiscard]] bool ReadVideo(uint32_t maxPixels, bool steady, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr, std::function<void(float)> progressCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

        if (_startFrame1 == nullptr)
            return false;

        uint32_t framesToRead = GetBits(maxPixels);
        uint32_t currentTime = GetFirstNodeFrameTime();

        bool abort = false;

        while (_frames.size() < framesToRead && currentTime < (uint32_t)_vr->GetLengthMS() && !abort) {
            if (progressCallback != nullptr)
                progressCallback((float)(currentTime * 100) / (float)_vr->GetLengthMS());

            logger_gcm.debug("Reading frame %u at %ums", (uint32_t)_frames.size() + 1, currentTime);
            auto img = ReadFrame(_vr->GetNextFrame(currentTime), currentTime, true);
            _frames.push_back(img);
            if (displayCallback != nullptr)
                displayCallback(img->GetColourImage(), nullptr);
            currentTime += NODEON;
            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        if (!abort && steady) {
            // now remove all the backgrounds - we do this once regardless of other processing
            RemoveBackgroundFromFrames(displayCallback);
        }

        return framesToRead == _frames.size();
    }

    void ProcessFrames(uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, uint8_t erode_dilate, uint8_t threshold, float gamma, uint8_t saturate, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        wxASSERT(_frames.size() > 0);

        while (_processedFrames.size() > 0) {
            delete _processedFrames.back();
            _processedFrames.pop_back();
        }

        for (auto& it : _frames) {
            _processedFrames.push_back(it->Process(cropLeft, cropRight, cropTop, cropBottom, contrast, blur, erode_dilate, threshold, gamma, saturate, displayCallback));

            if (wxGetKeyState(WXK_ESCAPE))
                break;
        }
    }

    void ProcessFramesA(uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, float gamma, uint8_t saturate, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr, std::function<void(float)> progressCallback = nullptr)
    {
        wxASSERT(_frames.size() > 0);

        while (_processedFrames.size() > 0) {
            delete _processedFrames.back();
            _processedFrames.pop_back();
        }

        float cnt = 0;
        for (auto& it : _frames) {
            _processedFrames.push_back(it->ProcessA(cropLeft, cropRight, cropTop, cropBottom, contrast, blur, gamma, saturate, displayCallback));
            if (progressCallback != nullptr) {
                ++cnt;
                progressCallback((0.5 * cnt) / (float)_frames.size());
            }
            if (wxGetKeyState(WXK_ESCAPE))
                break;
        }
    }

    void ProcessFramesB(uint8_t erode_dilate, uint8_t threshold, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        wxASSERT(_processedFrames.size() > 0);

        for (auto& it : _processedFrames) {
            it->ProcessB(erode_dilate, threshold, displayCallback);
            if (wxGetKeyState(WXK_ESCAPE))
                break;
        }
    }

    // lights are 1 based
    [[nodiscard]] wxPoint FindLight(uint32_t pixel, uint32_t numPixels, std::map<std::string, ProcessedImage*>& cache, ProcessedImage** ppi, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        wxASSERT(_processedFrames.size() > 0);

        auto bits = GetBits(numPixels);
        auto value = convertToBase3(pixel, bits);

        logger_gcm.debug("Finding pixel %u : %s", pixel, (const char*)value.c_str());

        ProcessedImage* img = nullptr;

        // find the longest image in the cache that is helpful
        uint32_t startat = 0;
        for (uint32_t i = bits - 1; i > 1; --i) {
            std::string key = value.substr(0, i);
            if (cache.find(key) != cache.end()) {
                img = new ProcessedImage(cache[key]);
                startat = key.size();
                logger_gcm.debug("   starting with image from cache ... key %s", (const char*)key.c_str());
                break;
            }
        }

        bool abort = false;

        for (uint32_t i = startat; i < bits && !abort; ++i) {
            switch (value[i]) {
            case '0':
                logger_gcm.debug("   Applying red frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetRedImage());
                } else {
                    img->And(_processedFrames[i]->GetRedImage());
                }
                break;
            case '1':
                logger_gcm.debug("   Applying green frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetGreenImage());
                } else {
                    img->And(_processedFrames[i]->GetGreenImage());
                }
                break;
            case '2':
                logger_gcm.debug("   Applying blue frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetBlueImage());
                } else {
                    img->And(_processedFrames[i]->GetBlueImage());
                }
                break;
            }

            if (displayCallback != nullptr) {
                displayCallback(img, nullptr);
            }

            std::string key = value.substr(0, i);
            if (cache.find(key) == cache.end()) {
                cache[key] = new ProcessedImage(img);
            }

            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        if (!abort) {
            auto res = img->FindPixel();

            delete img;

            // now only one key pixel should be in the image so find the largest connected white area
            return res;
        } else {
            delete img;
            return wxPoint(-1, -1);
        }
    }

    // lights are 1 based
    [[nodiscard]] wxPoint FindLightA(uint32_t pixel, uint32_t numPixels, uint8_t erode_dilate, uint8_t threshold, std::map<std::string, ProcessedImage*>& cache, ProcessedImage** ppi, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        wxASSERT(_processedFrames.size() > 0);

        auto bits = GetBits(numPixels);
        auto value = convertToBase3(pixel, bits);

        logger_gcm.debug("Finding pixel %u : %s", pixel, (const char*)value.c_str());

        ProcessedImage* img = nullptr;

        // find the longest image in the cache that is helpful
        uint32_t startat = 0;
        for (uint32_t i = bits - 1; i > 1; --i) {
            std::string key = value.substr(0, i);
            if (cache.find(key) != cache.end()) {
                img = new ProcessedImage(cache[key]);
                startat = key.size();
                logger_gcm.debug("   starting with image from cache ... key %s", (const char*)key.c_str());
                break;
            }
        }

        bool abort = false;
        for (uint32_t i = startat; i < bits && !abort; ++i) {
            switch (value[i]) {
            case '0':
                logger_gcm.debug("   Applying red frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetRedImage());
                } else {
                    img->Min(_processedFrames[i]->GetRedImage());
                }
                break;
            case '1':
                logger_gcm.debug("   Applying green frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetGreenImage());
                } else {
                    img->Min(_processedFrames[i]->GetGreenImage());
                }
                break;
            case '2':
                logger_gcm.debug("   Applying blue frame %d", i + 1);
                if (img == nullptr) {
                    img = new ProcessedImage(_processedFrames[i]->GetBlueImage());
                } else {
                    img->Min(_processedFrames[i]->GetBlueImage());
                }
                break;
            }

            if (displayCallback != nullptr) {
                displayCallback(img, nullptr);
            }

            std::string key = value.substr(0, i + 1);
            if (cache.find(key) == cache.end()) {
                cache[key] = new ProcessedImage(img);
            }

            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        if (!abort) {
            img->ProcessB(erode_dilate, threshold, displayCallback);

            if (displayCallback != nullptr) {
                displayCallback(img, nullptr);
            }

            // auto res = img->FindPixel(); // find the largest white area
            auto res = img->FindPixelA(); // find the centre of all white areas
            delete img;

            // now only one key pixel should be in the image so find the largest connected white area
            return res;
        } else {
            delete img;
            return wxPoint(-1, -1);
        }
    }

    // turns rgb into b&w images then tries to find them
    [[nodiscard]] std::list<std::pair<wxPoint, uint32_t>> FindLights(uint32_t maxPixels, uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, uint8_t erode_dilate, uint8_t threshold, float gamma, uint8_t saturate, ProcessedImage** ppi, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        std::list<std::pair<wxPoint, uint32_t>> res;

        if (_startFrame1 == nullptr || _frames.size() == 0)
            return res;

        // prepare the images
        ProcessFrames(cropLeft, cropRight, cropTop, cropBottom, contrast, blur, erode_dilate, threshold, gamma, saturate, displayCallback);

        std::map<std::string, ProcessedImage*> cache;

        logger_gcm.debug("Found pixels:");
        bool abort = false;
        for (uint32_t p = 0; p < maxPixels && !abort; ++p) {
            auto pt = FindLight(p + 1, maxPixels, cache, ppi, displayCallback);
            if (pt.x != -1) {
                logger_gcm.debug("   %d: %d, %d", p + 1, pt.x, pt.y);
                res.push_back({ pt, p + 1 });
            }
            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        for (const auto& it : cache) {
            delete it.second;
        }

        // I am deliberately not clearing when we abort as the user may want to see what was found

        return res;
    }

    [[nodiscard]] std::list<std::pair<wxPoint, uint32_t>> FindLightsA(uint32_t maxPixels, uint32_t cropLeft, uint32_t cropRight, uint32_t cropTop, uint32_t cropBottom, int contrast, uint8_t blur, uint8_t erode_dilate, uint8_t threshold, float gamma, uint8_t saturate, ProcessedImage** ppi, std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> displayCallback = nullptr, std::function<void(float)> progressCallback = nullptr)
    {
        static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
        std::list<std::pair<wxPoint, uint32_t>> res;

        if (_startFrame1 == nullptr || _frames.size() == 0)
            return res;

        // prepare the images
        ProcessFramesA(cropLeft, cropRight, cropTop, cropBottom, contrast, blur, gamma, saturate, displayCallback, progressCallback);

        if (displayCallback != nullptr) {
            for (const auto& it : _processedFrames) {
                displayCallback(it->GetColourImage(), nullptr);
            }
        }

        std::map<std::string, ProcessedImage*> cache;

        logger_gcm.debug("Found pixels:");
        bool abort = false;
        for (uint32_t p = 0; p < maxPixels && !abort; ++p) {
            auto pt = FindLightA(p + 1, maxPixels, erode_dilate, threshold, cache, ppi, displayCallback);
            if (pt.x != -1) {
                logger_gcm.debug("   %d: %d, %d", p + 1, pt.x, pt.y);
                res.push_back({ pt, p + 1 });
            }
            if (progressCallback != nullptr) {
                progressCallback(0.5 + ((float)(p + 1) * 0.5) / (float)maxPixels);
            }
            if (displayCallback != nullptr)
                displayCallback(GetStartFrame()->GetColourImage(), &res);
            abort |= wxGetKeyState(WXK_ESCAPE);
        }

        for (const auto& it : cache) {
            delete it.second;
        }

        // I am deliberately not deleting the points when aborted as the user is likely to want to see what was found before the abort

        return res;
    }
};

#pragma region Constructor

const long GenerateCustomModelDialog::ID_STATICBITMAP_Preview = wxNewId();

//(*IdInit(GenerateCustomModelDialog)
const long GenerateCustomModelDialog::ID_RADIOBUTTON1 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON4 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON2 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_NC_Count = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_StartChannel = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_Intensity = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_PCM_Run = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Prepare = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON3 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON6 = wxNewId();
const long GenerateCustomModelDialog::ID_RADIOBUTTON5 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_MT_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL1 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT10 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_GCM_Filename = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_GCM_SelectFile = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT13 = wxNewId();
const long GenerateCustomModelDialog::ID_SPINCTRL_PROCESSNODECOUNT = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX_BI_IsSteady = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX3 = wxNewId();
const long GenerateCustomModelDialog::ID_GAUGE2 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CV_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_ChooseVideo = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT3 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartFrameOk = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT_StartTime = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_SF_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_StartFrame = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT14 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT15 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL4 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT16 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER4 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL5 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON5 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON1 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON2 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON3 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON4 = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL2 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT5 = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX2 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_AdjustBlur = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BC_Blur = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT8 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_Sensitivity = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Sensitivity = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT6 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MinSeparation = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_MinSeparation = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT4 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER1 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL1 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT2 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_Contrast = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Contrast = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT11 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER2 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL2 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT12 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER3 = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL3 = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT7 = wxNewId();
const long GenerateCustomModelDialog::ID_SLIDER_BI_MinScale = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_MinScale = wxNewId();
const long GenerateCustomModelDialog::ID_CHECKBOX1 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CB_RestoreDefault = wxNewId();
const long GenerateCustomModelDialog::ID_TEXTCTRL_BI_Status = wxNewId();
const long GenerateCustomModelDialog::ID_GAUGE1 = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_BI_Next = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_BulbIdentify = wxNewId();
const long GenerateCustomModelDialog::ID_STATICTEXT9 = wxNewId();
const long GenerateCustomModelDialog::ID_GRID_CM_Result = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Shrink = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_Grow = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CM_Back = wxNewId();
const long GenerateCustomModelDialog::ID_BUTTON_CM_Save = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_CustomModel = wxNewId();
const long GenerateCustomModelDialog::ID_AUINOTEBOOK_ProcessSettings = wxNewId();
const long GenerateCustomModelDialog::ID_PANEL_Generate = wxNewId();
const long GenerateCustomModelDialog::ID_AUINOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GenerateCustomModelDialog,wxDialog)
	//(*EventTable(GenerateCustomModelDialog)
	//*)
END_EVENT_TABLE()

GenerateCustomModelDialog::GenerateCustomModelDialog(xLightsFrame* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _parent = parent;
    _outputManager = outputManager;
    _busy = false;

	//(*Initialize(GenerateCustomModelDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer15;
	wxFlexGridSizer* FlexGridSizer16;
	wxFlexGridSizer* FlexGridSizer17;
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer20;
	wxFlexGridSizer* FlexGridSizer21;
	wxFlexGridSizer* FlexGridSizer22;
	wxFlexGridSizer* FlexGridSizer23;
	wxFlexGridSizer* FlexGridSizer24;
	wxFlexGridSizer* FlexGridSizer25;
	wxFlexGridSizer* FlexGridSizer26;
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticText* StaticText14;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;

	Create(parent, wxID_ANY, _("Generate Custom Models"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	AuiNotebook1 = new wxAuiNotebook(this, ID_AUINOTEBOOK1, wxDefaultPosition, wxSize(1400,700), wxTAB_TRAVERSAL);
	Panel_Prepare = new wxPanel(AuiNotebook1, ID_PANEL_Prepare, wxPoint(63,54), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Prepare"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	StaticText1 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Use this page to generate the lights that you need to video before you can proceed to the processing tab to create your custom model."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Using a steady video camera to record a clear video showing all the lights running this test pattern."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(Panel_Prepare, wxID_ANY, _("The test pattern must start within the first 30 seconds of the video."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText4 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Model details:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	wxFont StaticText4Font(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText4->SetFont(StaticText4Font);
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText14 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodesRadioButton = new wxRadioButton(Panel_Prepare, ID_RADIOBUTTON1, _("Nodes - Automatic"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	NodesRadioButton->SetValue(true);
	FlexGridSizer6->Add(NodesRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ManualNodesRadioButton = new wxRadioButton(Panel_Prepare, ID_RADIOBUTTON4, _("Nodes - Manual"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	FlexGridSizer6->Add(ManualNodesRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SingleChannelRadioButton = new wxRadioButton(Panel_Prepare, ID_RADIOBUTTON2, _("Single Channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer6->Add(SingleChannelRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Node/Channel Count"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_NC_Count = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_NC_Count, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 99999, 100, _T("ID_SPINCTRL_NC_Count"));
	SpinCtrl_NC_Count->SetValue(_T("100"));
	FlexGridSizer6->Add(SpinCtrl_NC_Count, 1, wxALL|wxEXPAND, 2);
	StaticText6 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_StartChannel = new wxSpinCtrl(Panel_Prepare, ID_SPINCTRL_StartChannel, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 999999, 1, _T("ID_SPINCTRL_StartChannel"));
	SpinCtrl_StartChannel->SetValue(_T("1"));
	FlexGridSizer6->Add(SpinCtrl_StartChannel, 1, wxALL|wxEXPAND, 2);
	StaticText8 = new wxStaticText(Panel_Prepare, wxID_ANY, _("Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Intensity = new wxSlider(Panel_Prepare, ID_SLIDER_Intensity, 255, 1, 255, wxDefaultPosition, wxSize(300,-1), 0, wxDefaultValidator, _T("ID_SLIDER_Intensity"));
	FlexGridSizer6->Add(Slider_Intensity, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PCM_Run = new wxButton(Panel_Prepare, ID_BUTTON_PCM_Run, _("Run Capture Pattern"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PCM_Run"));
	FlexGridSizer2->Add(Button_PCM_Run, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Prepare->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Prepare);
	FlexGridSizer2->SetSizeHints(Panel_Prepare);
	Panel_Generate = new wxPanel(AuiNotebook1, ID_PANEL_Generate, wxPoint(59,17), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Generate"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	FlexGridSizer14 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer14->AddGrowableCol(0);
	FlexGridSizer14->AddGrowableRow(0);
	FlexGridSizer5->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer7 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(0);
	AuiNotebook_ProcessSettings = new wxAuiNotebook(Panel_Generate, ID_AUINOTEBOOK_ProcessSettings, wxDefaultPosition, wxDefaultSize, 0);
	Panel1 = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL1, wxPoint(73,8), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer24 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer24->AddGrowableCol(0);
	FlexGridSizer24->AddGrowableRow(1);
	FlexGridSizer25 = new wxFlexGridSizer(0, 1, 0, 0);
	NodesRadioButtonPg2 = new wxRadioButton(Panel1, ID_RADIOBUTTON3, _("Nodes - Automatic"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	NodesRadioButtonPg2->SetValue(true);
	FlexGridSizer25->Add(NodesRadioButtonPg2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodesManualRadioButtonPg2 = new wxRadioButton(Panel1, ID_RADIOBUTTON6, _("Nodes - Manual"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON6"));
	FlexGridSizer25->Add(NodesManualRadioButtonPg2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SLRadioButton = new wxRadioButton(Panel1, ID_RADIOBUTTON5, _("Static Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON5"));
	FlexGridSizer25->Add(SLRadioButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(FlexGridSizer25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer26 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_MT_Next = new wxButton(Panel1, ID_BUTTON_MT_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MT_Next"));
	FlexGridSizer26->Add(Button_MT_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(FlexGridSizer26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(FlexGridSizer24);
	FlexGridSizer24->Fit(Panel1);
	FlexGridSizer24->SetSizeHints(Panel1);
	Panel_ChooseVideo = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_ChooseVideo, wxPoint(18,15), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_ChooseVideo"));
	FlexGridSizer21 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer21->AddGrowableCol(0);
	FlexGridSizer21->AddGrowableRow(4);
	StaticText_CM_Request = new wxStaticText(Panel_ChooseVideo, ID_STATICTEXT10, _("Insert text here"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT10"));
	FlexGridSizer21->Add(StaticText_CM_Request, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer22 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer22->AddGrowableCol(1);
	StaticText3 = new wxStaticText(Panel_ChooseVideo, wxID_ANY, _("Media File"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_GCM_Filename = new wxTextCtrl(Panel_ChooseVideo, ID_TEXTCTRL_GCM_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_GCM_Filename"));
	FlexGridSizer22->Add(TextCtrl_GCM_Filename, 1, wxALL|wxEXPAND, 2);
	Button_GCM_SelectFile = new wxButton(Panel_ChooseVideo, ID_BUTTON_GCM_SelectFile, _("..."), wxDefaultPosition, wxSize(29,28), 0, wxDefaultValidator, _T("ID_BUTTON_GCM_SelectFile"));
	FlexGridSizer22->Add(Button_GCM_SelectFile, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText19 = new wxStaticText(Panel_ChooseVideo, ID_STATICTEXT13, _("Node/Channel Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer22->Add(StaticText19, 1, wxALL|wxEXPAND, 2);
	SpinCtrl_ProcessNodeCount = new wxSpinCtrl(Panel_ChooseVideo, ID_SPINCTRL_PROCESSNODECOUNT, _T("100"), wxDefaultPosition, wxSize(100,-1), 0, 1, 99999, 100, _T("ID_SPINCTRL_PROCESSNODECOUNT"));
	SpinCtrl_ProcessNodeCount->SetValue(_T("100"));
	FlexGridSizer22->Add(SpinCtrl_ProcessNodeCount, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	CheckBox_BI_IsSteady = new wxCheckBox(Panel_ChooseVideo, ID_CHECKBOX_BI_IsSteady, _("Video is steady"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BI_IsSteady"));
	CheckBox_BI_IsSteady->SetValue(true);
	FlexGridSizer22->Add(CheckBox_BI_IsSteady, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_AdvancedStartScan = new wxCheckBox(Panel_ChooseVideo, ID_CHECKBOX3, _("Preview video during scan (slower)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_AdvancedStartScan->SetValue(false);
	FlexGridSizer22->Add(CheckBox_AdvancedStartScan, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21->Add(FlexGridSizer22, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Gauge_Progress1 = new wxGauge(Panel_ChooseVideo, ID_GAUGE2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE2"));
	FlexGridSizer21->Add(Gauge_Progress1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer23 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_CV_Back = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Back"));
	FlexGridSizer23->Add(Button_CV_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_CV_Next = new wxButton(Panel_ChooseVideo, ID_BUTTON_CV_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CV_Next"));
	FlexGridSizer23->Add(Button_CV_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer21->Add(FlexGridSizer23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_ChooseVideo->SetSizer(FlexGridSizer21);
	FlexGridSizer21->Fit(Panel_ChooseVideo);
	FlexGridSizer21->SetSizeHints(Panel_ChooseVideo);
	Panel_StartFrame = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_StartFrame, wxPoint(43,126), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_StartFrame"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(4);
	StaticText11 = new wxStaticText(Panel_StartFrame, ID_STATICTEXT3, _("This is the frame the scan has identified as being the most likely to show all your bulbs. \nIt should be a frame showing all the bulbs on."), wxDefaultPosition, wxSize(658,99), wxALIGN_LEFT, _T("ID_STATICTEXT3"));
	FlexGridSizer10->Add(StaticText11, 1, wxALL|wxEXPAND, 2);
	StaticText_StartFrameOk = new wxStaticText(Panel_StartFrame, ID_STATICTEXT_StartFrameOk, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT_StartFrameOk"));
	wxFont StaticText_StartFrameOkFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText_StartFrameOk->SetFont(StaticText_StartFrameOkFont);
	FlexGridSizer10->Add(StaticText_StartFrameOk, 1, wxALL|wxEXPAND, 2);
	StaticText_StartTime = new wxStaticText(Panel_StartFrame, ID_STATICTEXT_StartTime, wxEmptyString, wxDefaultPosition, wxSize(95,16), 0, _T("ID_STATICTEXT_StartTime"));
	FlexGridSizer10->Add(StaticText_StartTime, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_SF_Back = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Back"));
	FlexGridSizer12->Add(Button_SF_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SF_Next = new wxButton(Panel_StartFrame, ID_BUTTON_SF_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SF_Next"));
	FlexGridSizer12->Add(Button_SF_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_StartFrame->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(Panel_StartFrame);
	FlexGridSizer10->SetSizeHints(Panel_StartFrame);
	Panel_ManualIdentify = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL2, wxPoint(281,15), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	StaticText9 = new wxStaticText(Panel_ManualIdentify, ID_STATICTEXT14, _("Click on the image to identify the location of the current bulb.\n\nOnce happy click next bulb to move to the next lit bulb.\nUse bump > and bump < to jump forward back video frames if you think the video has lost its place. This can happen in long videos.\n\nPress next when you are done identifying bulbs."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer4->Add(StaticText9, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(1);
	FlexGridSizer8->AddGrowableRow(4);
	StaticText10 = new wxStaticText(Panel_ManualIdentify, ID_STATICTEXT15, _("Current Bulb:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer8->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_CurrentBulb = new wxTextCtrl(Panel_ManualIdentify, ID_TEXTCTRL4, _("1"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer8->Add(TextCtrl_CurrentBulb, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(Panel_ManualIdentify, ID_STATICTEXT16, _("Model Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
	FlexGridSizer8->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	Slider_MI_ModelScale = new wxSlider(Panel_ManualIdentify, ID_SLIDER4, 1, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER4"));
	FlexGridSizer11->Add(Slider_MI_ModelScale, 1, wxALL|wxEXPAND, 5);
	TextCtrl_MI_ModelScale = new wxTextCtrl(Panel_ManualIdentify, ID_TEXTCTRL5, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer11->Add(TextCtrl_MI_ModelScale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MI_Back1Bulb = new wxButton(Panel_ManualIdentify, ID_BUTTON5, _("Back 1 Bulb"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer8->Add(Button_MI_Back1Bulb, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_BumpBack = new wxButton(Panel_ManualIdentify, ID_BUTTON1, _("Bump <"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer8->Add(Button_BumpBack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonBumpFwd = new wxButton(Panel_ManualIdentify, ID_BUTTON2, _("Bump >"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer8->Add(ButtonBumpFwd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_MI_Back = new wxButton(Panel_ManualIdentify, ID_BUTTON3, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer9->Add(Button_MI_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MI_Next = new wxButton(Panel_ManualIdentify, ID_BUTTON4, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer9->Add(Button_MI_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	Panel_ManualIdentify->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel_ManualIdentify);
	FlexGridSizer4->SetSizeHints(Panel_ManualIdentify);
	Panel_BulbIdentify = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_BulbIdentify, wxPoint(176,18), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_BulbIdentify"));
	FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer15->AddGrowableCol(0);
	FlexGridSizer15->AddGrowableRow(5);
	StaticText_BI = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT5, _("The red circles on the image show the bulbs we have identify.\nAdjust the sensitivity if there are bulbs missing or phantom bulbs identified.\n\nClick next when you are happy that all bulbs have been detected."), wxDefaultPosition, wxSize(-1,100), wxALIGN_LEFT, _T("ID_STATICTEXT5"));
	FlexGridSizer15->Add(StaticText_BI, 1, wxALL|wxEXPAND, 5);
	CheckBox_Advanced = new wxCheckBox(Panel_BulbIdentify, ID_CHECKBOX2, _("Advanced Controls"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_Advanced->SetValue(false);
	FlexGridSizer15->Add(CheckBox_Advanced, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer16 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer16->AddGrowableCol(1);
	StaticText_Blur = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT1, _("Blur"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer16->Add(StaticText_Blur, 1, wxALL|wxEXPAND, 2);
	Slider_AdjustBlur = new wxSlider(Panel_BulbIdentify, ID_SLIDER_AdjustBlur, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_AdjustBlur"));
	FlexGridSizer16->Add(Slider_AdjustBlur, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BC_Blur = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BC_Blur, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BC_Blur"));
	FlexGridSizer16->Add(TextCtrl_BC_Blur, 1, wxALL|wxEXPAND, 2);
	StaticText_Sensitivity = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT8, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer16->Add(StaticText_Sensitivity, 1, wxALL|wxEXPAND, 2);
	Slider_BI_Sensitivity = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_Sensitivity, 127, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_Sensitivity"));
	FlexGridSizer16->Add(Slider_BI_Sensitivity, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_Sensitivity = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Sensitivity, _("127"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Sensitivity"));
	FlexGridSizer16->Add(TextCtrl_BI_Sensitivity, 1, wxALL|wxEXPAND, 5);
	StaticText_MinSeparation = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT6, _("Minimum Separation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer16->Add(StaticText_MinSeparation, 1, wxALL|wxEXPAND, 2);
	Slider_BI_MinSeparation = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MinSeparation, 1, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MinSeparation"));
	FlexGridSizer16->Add(Slider_BI_MinSeparation, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_MinSeparation = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_MinSeparation, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_MinSeparation"));
	FlexGridSizer16->Add(TextCtrl_BI_MinSeparation, 1, wxALL|wxEXPAND, 5);
	StaticTextDespeckle = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT4, _("Despeckle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer16->Add(StaticTextDespeckle, 1, wxALL|wxEXPAND, 2);
	Slider_Despeckle = new wxSlider(Panel_BulbIdentify, ID_SLIDER1, 0, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
	FlexGridSizer16->Add(Slider_Despeckle, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Despeckle = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL1, _("0"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer16->Add(TextCtrl_Despeckle, 1, wxALL|wxEXPAND, 5);
	StaticText_Contrast = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT2, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer16->Add(StaticText_Contrast, 1, wxALL|wxEXPAND, 2);
	Slider_BI_Contrast = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_Contrast, 0, -20, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_Contrast"));
	FlexGridSizer16->Add(Slider_BI_Contrast, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_Contrast = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Contrast, _("0"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Contrast"));
	FlexGridSizer16->Add(TextCtrl_BI_Contrast, 1, wxALL|wxEXPAND, 2);
	StaticText_Gamma = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT11, _("Gamma"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer16->Add(StaticText_Gamma, 1, wxALL|wxEXPAND, 2);
	Slider_Gamma = new wxSlider(Panel_BulbIdentify, ID_SLIDER2, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER2"));
	FlexGridSizer16->Add(Slider_Gamma, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Gamma = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL2, _("1.0"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer16->Add(TextCtrl_Gamma, 1, wxALL|wxEXPAND, 2);
	StaticText_Saturation = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT12, _("Saturation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer16->Add(StaticText_Saturation, 1, wxALL|wxEXPAND, 2);
	Slider_Saturation = new wxSlider(Panel_BulbIdentify, ID_SLIDER3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER3"));
	FlexGridSizer16->Add(Slider_Saturation, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Saturation = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL3, _("0"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer16->Add(TextCtrl_Saturation, 1, wxALL|wxEXPAND, 2);
	StaticText_ModelScale = new wxStaticText(Panel_BulbIdentify, ID_STATICTEXT7, _("Model Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer16->Add(StaticText_ModelScale, 1, wxALL|wxEXPAND, 2);
	Slider_BI_MinScale = new wxSlider(Panel_BulbIdentify, ID_SLIDER_BI_MinScale, 1, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BI_MinScale"));
	FlexGridSizer16->Add(Slider_BI_MinScale, 1, wxALL|wxEXPAND, 2);
	TextCtrl_BI_MinScale = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_MinScale, _("1"), wxDefaultPosition, wxSize(40,24), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_MinScale"));
	FlexGridSizer16->Add(TextCtrl_BI_MinScale, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer15->Add(FlexGridSizer16, 1, wxALL|wxEXPAND, 2);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	CheckBox_GuessSingle = new wxCheckBox(Panel_BulbIdentify, ID_CHECKBOX1, _("Guess location of missing nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_GuessSingle->SetValue(true);
	BoxSizer1->Add(CheckBox_GuessSingle, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer15->Add(BoxSizer1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer27 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_CB_RestoreDefault = new wxButton(Panel_BulbIdentify, ID_BUTTON_CB_RestoreDefault, _("Restore Default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CB_RestoreDefault"));
	FlexGridSizer27->Add(Button_CB_RestoreDefault, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer15->Add(FlexGridSizer27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_BI_Status = new wxTextCtrl(Panel_BulbIdentify, ID_TEXTCTRL_BI_Status, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_BI_Status"));
	FlexGridSizer15->Add(TextCtrl_BI_Status, 1, wxALL|wxEXPAND, 2);
	Gauge_Progress2 = new wxGauge(Panel_BulbIdentify, ID_GAUGE1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GAUGE1"));
	FlexGridSizer15->Add(Gauge_Progress2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer17 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_BI_Back = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Back"));
	FlexGridSizer17->Add(Button_BI_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_BI_Next = new wxButton(Panel_BulbIdentify, ID_BUTTON_BI_Next, _("Next"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BI_Next"));
	FlexGridSizer17->Add(Button_BI_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer15->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_BulbIdentify->SetSizer(FlexGridSizer15);
	FlexGridSizer15->Fit(Panel_BulbIdentify);
	FlexGridSizer15->SetSizeHints(Panel_BulbIdentify);
	Panel_CustomModel = new wxPanel(AuiNotebook_ProcessSettings, ID_PANEL_CustomModel, wxPoint(259,19), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CustomModel"));
	FlexGridSizer18 = new wxFlexGridSizer(4, 1, 0, 0);
	FlexGridSizer18->AddGrowableCol(0);
	FlexGridSizer18->AddGrowableRow(1);
	StaticText17 = new wxStaticText(Panel_CustomModel, ID_STATICTEXT9, _("This is the new custom model. Click save to create a model file that you can then import into your layout."), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT9"));
	FlexGridSizer18->Add(StaticText17, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer19 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer19->AddGrowableCol(0);
	FlexGridSizer19->AddGrowableRow(0);
	Grid_CM_Result = new wxGrid(Panel_CustomModel, ID_GRID_CM_Result, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_CM_Result"));
	FlexGridSizer19->Add(Grid_CM_Result, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(FlexGridSizer19, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer18->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_Shrink = new wxButton(Panel_CustomModel, ID_BUTTON_Shrink, _("-"), wxDefaultPosition, wxSize(26,28), 0, wxDefaultValidator, _T("ID_BUTTON_Shrink"));
	FlexGridSizer20->Add(Button_Shrink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Grow = new wxButton(Panel_CustomModel, ID_BUTTON_Grow, _("+"), wxDefaultPosition, wxSize(26,28), 0, wxDefaultValidator, _T("ID_BUTTON_Grow"));
	FlexGridSizer20->Add(Button_Grow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CM_Back = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Back, _("Back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Back"));
	FlexGridSizer20->Add(Button_CM_Back, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CM_Save = new wxButton(Panel_CustomModel, ID_BUTTON_CM_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CM_Save"));
	FlexGridSizer20->Add(Button_CM_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer18->Add(FlexGridSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_CustomModel->SetSizer(FlexGridSizer18);
	FlexGridSizer18->Fit(Panel_CustomModel);
	FlexGridSizer18->SetSizeHints(Panel_CustomModel);
	AuiNotebook_ProcessSettings->AddPage(Panel1, _("Model Type"));
	AuiNotebook_ProcessSettings->AddPage(Panel_ChooseVideo, _("Choose Media"));
	AuiNotebook_ProcessSettings->AddPage(Panel_StartFrame, _("Start Frame"));
	AuiNotebook_ProcessSettings->AddPage(Panel_ManualIdentify, _("Manual Identify"));
	AuiNotebook_ProcessSettings->AddPage(Panel_BulbIdentify, _("Bulb Identify"));
	AuiNotebook_ProcessSettings->AddPage(Panel_CustomModel, _("Custom Model"));
	FlexGridSizer7->Add(AuiNotebook_ProcessSettings, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer5->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
	Panel_Generate->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_Generate);
	FlexGridSizer3->SetSizeHints(Panel_Generate);
	AuiNotebook1->AddPage(Panel_Prepare, _("Prepare"), true);
	AuiNotebook1->AddPage(Panel_Generate, _("Process"));
	FlexGridSizer1->Add(AuiNotebook1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	SetSizer(FlexGridSizer1);
	FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_PCM_Run,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_PCM_RunClick);
	Connect(ID_BUTTON_MT_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MT_NextClick);
	Connect(ID_TEXTCTRL_GCM_Filename,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText);
	Connect(ID_BUTTON_GCM_SelectFile,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GCM_SelectFileClick);
	Connect(ID_CHECKBOX_BI_IsSteady,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnCheckBox_BI_IsSteadyClick);
	Connect(ID_BUTTON_CV_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_BackClick);
	Connect(ID_BUTTON_CV_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CV_NextClick);
	Connect(ID_BUTTON_SF_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_BackClick);
	Connect(ID_BUTTON_SF_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_SF_NextClick);
	Connect(ID_SLIDER4,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_MI_ModelScaleCmdScrollThumbRelease);
	Connect(ID_SLIDER4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_MI_ModelScaleCmdSliderUpdated);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_Back1BulbClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BumpBackClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButtonBumpFwdClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_BackClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_MI_NextClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnCheckBox_AdvancedClick);
	Connect(ID_SLIDER_AdjustBlur,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScrollChanged);
	Connect(ID_SLIDER_AdjustBlur,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll);
	Connect(ID_SLIDER_BI_Sensitivity,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdScrollChanged);
	Connect(ID_SLIDER_BI_Sensitivity,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated);
	Connect(ID_SLIDER_BI_MinSeparation,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdScrollChanged);
	Connect(ID_SLIDER_BI_MinSeparation,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdSliderUpdated);
	Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_DespeckleCmdScrollChanged);
	Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_DespeckleCmdSliderUpdated);
	Connect(ID_SLIDER_BI_Contrast,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_ContrastCmdScrollChanged);
	Connect(ID_SLIDER_BI_Contrast,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_ContrastCmdSliderUpdated);
	Connect(ID_SLIDER2,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_GammaCmdScrollChanged);
	Connect(ID_SLIDER2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_GammaCmdSliderUpdated);
	Connect(ID_SLIDER3,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_SaturationCmdScrollChanged);
	Connect(ID_SLIDER3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_SaturationCmdSliderUpdated);
	Connect(ID_SLIDER_BI_MinScale,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdScrollChanged);
	Connect(ID_SLIDER_BI_MinScale,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdSliderUpdated);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnCheckBox_GuessSingleClick);
	Connect(ID_BUTTON_CB_RestoreDefault,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_RestoreDefaultClick);
	Connect(ID_BUTTON_BI_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_BackClick);
	Connect(ID_BUTTON_BI_Next,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_BI_NextClick);
	Connect(ID_BUTTON_Shrink,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_ShrinkClick);
	Connect(ID_BUTTON_Grow,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_GrowClick);
	Connect(ID_BUTTON_CM_Back,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_BackClick);
	Connect(ID_BUTTON_CM_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateCustomModelDialog::OnButton_CM_SaveClick);
	Connect(ID_AUINOTEBOOK_ProcessSettings,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&GenerateCustomModelDialog::OnAuiNotebook_ProcessSettingsPageChanging);
	//*)

    _displaybmp = wxImage(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, true);

    StaticBitmap_Preview = new MyGenericStaticBitmap(Panel_Generate, ID_STATICBITMAP_Preview, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP_Preview"));
    StaticBitmap_Preview->SetScaleMode(wxStaticBitmapBase::ScaleMode::Scale_AspectFit);
    StaticBitmap_Preview->SetSizeHints(wxSize(800, 600), wxSize(800, 600));
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    FlexGridSizer14->Insert(0, StaticBitmap_Preview, 1, wxALL | wxEXPAND, 2);
    FlexGridSizer14->Layout();
    FlexGridSizer5->Layout();

    FlexGridSizer19->Fit(Grid_CM_Result);
    FlexGridSizer19->SetSizeHints(Grid_CM_Result);
    FlexGridSizer19->Layout();

    Slider_Intensity->SetValue(10);

    StaticBitmap_Preview->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapLeftDown, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapLeftUp, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapLeftDClick, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_MOTION, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseMove, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseLeave, 0, this);
    StaticBitmap_Preview->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&GenerateCustomModelDialog::OnStaticBitmapMouseEnter, 0, this);

    _draggingedge = -1;

    SetBIDefault();

    MTTabEntry();

    _generator = new CustomModelGenerator();

    ValidateWindow();
}

GenerateCustomModelDialog::~GenerateCustomModelDialog()
{
	//(*Destroy(GenerateCustomModelDialog)
	FileDialog1->Destroy();
	//*)

    ClearLights();

    if (_generator != nullptr)
    {
        delete _generator;
    }

    if (_detectedImage != nullptr)
        delete _detectedImage;

    if (StaticBitmap_Preview != nullptr)
        delete StaticBitmap_Preview;
}
#pragma endregion Constructor

#pragma region Global Functions

void GenerateCustomModelDialog::OnResize(wxSizeEvent& event)
{
    event.Skip();
}

void GenerateCustomModelDialog::ValidateWindow()
{
    if (_state == VideoProcessingStates::CHOOSE_VIDEO)
    {
        wxString file = TextCtrl_GCM_Filename->GetValue();
        if (FileExists(file)) {
            TextCtrl_GCM_Filename->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            Button_CV_Next->Enable();
        } else {
            TextCtrl_GCM_Filename->SetBackgroundColour(*wxRED);
            Button_CV_Next->Disable();
        }
        Button_GCM_SelectFile->Enable();
    }
    else if (_state == VideoProcessingStates::CHOOSE_MODELTYPE)
    {
    }
    else if (_state == VideoProcessingStates::FINDING_START_FRAME)
    {
    }
    else if (_state == VideoProcessingStates::CIRCLING_BULBS)
    {
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
    } else if (_state == VideoProcessingStates::IDENTIFYING_MANUAL) {
        if (_lights.size() > 0) {
            Button_MI_Back1Bulb->Enable();
            Button_MI_Next->Enable();
        } else {
            Button_MI_Back1Bulb->Disable();
            Button_MI_Next->Disable();
        }
    }
    else if (_state == VideoProcessingStates::REVIEW_CUSTOM_MODEL)
    {
    }
}

#pragma endregion Global Functions

#pragma region Image Functions

void GenerateCustomModelDialog::ShowImage(const ProcessedImage* image)
{
    if (image != nullptr && image->IsOk())
    {
        wxSize s = StaticBitmap_Preview->GetSize();
        _displaybmp = image->Scale(s.GetWidth() , s.GetHeight(), wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
    }
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    wxYield();
}

void GenerateCustomModelDialog::CreateDetectedImage(ProcessedImage* pi, bool drawLines, std::list<std::pair<wxPoint, uint32_t>>* points)
{
    if (_generator == nullptr)
        return;

    // updates the _detectedImage with the clip rectangle and the detected pixels

    wxBitmap bmp(_generator->GetFirstFrame()->GetWidth(), _generator->GetFirstFrame()->GetHeight());
    wxMemoryDC dc(bmp);
    if (pi != nullptr) {
        dc.DrawBitmap(*pi, wxPoint(_clip.GetLeft(), _clip.GetTop()), false);
    }
    else if (_generator != nullptr)
    {
        dc.DrawBitmap(*_generator->GetStartFrame()->GetColourImage(), wxPoint(0, 0), false);
    }

    wxSize displaysize = StaticBitmap_Preview->GetSize();
    float factor = std::max((float)_generator->GetStartFrame()->GetWidth() / (float)displaysize.GetWidth(),
                            (float)_generator->GetStartFrame()->GetHeight() / (float)displaysize.GetHeight());

    // draw grey out clipped area
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxBrush shade(*wxLIGHT_GREY, wxBRUSHSTYLE_BDIAGONAL_HATCH);
    dc.SetBrush(shade);
    if (_clip.GetLeft() > 0)
    {
        dc.DrawRectangle(wxRect(0, 0, _clip.GetLeft(), _generator->GetStartFrame()->GetHeight()));
    }
    if (_clip.GetRight() < _generator->GetStartFrame()->GetWidth())
    {
        dc.DrawRectangle(wxRect(_clip.GetRight(), 0, _generator->GetStartFrame()->GetWidth() - _clip.GetRight(), _generator->GetStartFrame()->GetHeight()));
    }
    if (_clip.GetTop() > 0)
    {
        dc.DrawRectangle(wxRect(0, 0, _generator->GetStartFrame()->GetWidth(), _clip.GetTop()));
    }
    if (_clip.GetBottom() < _generator->GetStartFrame()->GetHeight())
    {
        dc.DrawRectangle(wxRect(0, _clip.GetBottom(), _generator->GetStartFrame()->GetWidth(), _generator->GetStartFrame()->GetHeight() - _clip.GetBottom()));
    }

    // Draw clip rectangle
    int penw = 2 * factor;
    wxPen p2(*wxGREEN, penw, wxPENSTYLE_LONG_DASH);
    dc.SetPen(p2);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(_clip);

    TextCtrl_BI_Status->SetValue(GenerateStats());

    std::list<std::pair<wxPoint, uint32_t>>* lights = &_lights;
    if (points != nullptr)
        lights = points;

    if (lights != nullptr) {
        if (lights->size() > 0) {
            if (drawLines) {
                wxPen p(*LINECOL, 1);
                dc.SetPen(p);
                auto it1 = lights->begin();
                auto it2 = it1;
                ++it2;
                while (it2 != lights->end()) {
                    wxPoint pt1 = wxPoint(_clip.GetLeft() + it1->first.x, it1->first.y + (_clip.GetTop()));
                    wxPoint pt2 = wxPoint(_clip.GetLeft() + it2->first.x, it2->first.y + (_clip.GetTop()));
                    dc.DrawLine(pt1, pt2);
                    ++it1;
                    ++it2;
                }
            }

            {
                int diameter = 2 * factor;
                wxBrush b(*PIXCOL, wxBrushStyle::wxBRUSHSTYLE_SOLID);
                dc.SetBrush(b);
                wxPen p = wxPen(*PIXCOL, 1);
                dc.SetPen(p);
                for (const auto& c : *lights) {
                    wxPoint pt = wxPoint(_clip.GetLeft() + c.first.x, c.first.y + (_clip.GetTop()));
                    dc.DrawCircle(pt, diameter);
                }
            }
        }
    }

    if (_detectedImage != nullptr)
        delete _detectedImage;

    _detectedImage = new ProcessedImage(bmp.ConvertToImage(), ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_UNKNOWN);

    ShowImage(_detectedImage);
}

#pragma endregion Image Functions

// ***********************************************************
//
// Prepare tab methods
//
// ***********************************************************

#pragma region Prepare

void GenerateCustomModelDialog::UpdateProgressCallback(float progress)
{
    _pd->Update(std::min(100, (int)(100.0 * progress)));
}

void GenerateCustomModelDialog::UpdateProgress(float progress)
{
    Gauge_Progress1->SetValue(std::min(100, (int)(100.0 * progress)));
    Gauge_Progress2->SetValue(std::min(100, (int)(100.0 * progress)));
    wxYield();
}

void GenerateCustomModelDialog::OnButton_PCM_RunClick(wxCommandEvent& event)
{
    DisplayInfo("Please prepare to video the model ... press ok when ready to start.", this);

    static log4cpp::Category &logger_pcm = log4cpp::Category::getInstance(std::string("log_prepcustommodel"));
    logger_pcm.info("Running lights to be videoed.");

    _pd = new wxProgressDialog("Running light patterns", "", 100, this);

    int count = SpinCtrl_NC_Count->GetValue();
    int startch = SpinCtrl_StartChannel->GetValue();
    int intensity = Slider_Intensity->GetValue();
    bool nodes = NodesRadioButton->GetValue();
    bool manual = ManualNodesRadioButton->GetValue();

    logger_pcm.info("   Count: %d.", count);
    logger_pcm.info("   Start Channel: %d.", startch);
    logger_pcm.info("   Intensity: %d.", intensity);
    if (nodes || manual)
    {
        if (nodes) {
        logger_pcm.info("   Nodes.");
        }
        else {
            logger_pcm.info("   Manual Nodes.");
        }
        logger_pcm.info("   Channels that will be affected %ld-%ld of %ld channels", startch, startch + (count * 3) - 1, _outputManager->GetTotalChannels());
    }
    else
    {
        logger_pcm.info("   Channels.");
        logger_pcm.info("   Channels that will be affected %ld-%ld of %ld channels", startch, startch + count - 1, _outputManager->GetTotalChannels());
    }

    if (manual) {
        _generator->RunManualSequence(_parent, _outputManager, startch, count, intensity, std::bind(&GenerateCustomModelDialog::UpdateProgressCallback, this, std::placeholders::_1));
    } else {
        _generator->RunSequence(_parent, _outputManager, nodes, startch, count, intensity, std::bind(&GenerateCustomModelDialog::UpdateProgressCallback, this, std::placeholders::_1));
    }

    delete _pd;
    _pd = nullptr;

    SetFocus();

    logger_pcm.info("   Done.");

    DisplayInfo("Please stop the video.", this);
    ValidateWindow();
}

#pragma endregion Prepare

// ***********************************************************
//
// Generate tab methods
//
// ***********************************************************

#pragma region Generate

#pragma region Generate Tab General Methods

void GenerateCustomModelDialog::OnAuiNotebook_ProcessSettingsPageChanging(wxAuiNotebookEvent& event)
{
    int page = event.GetSelection();
    if (_state == VideoProcessingStates::CHOOSE_MODELTYPE && page != PAGE_MODELTYPE)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::CHOOSE_VIDEO && page != PAGE_CHOOSEVIDEO)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::FINDING_START_FRAME && page != PAGE_STARTFRAME)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::IDENTIFYING_BULBS && page != PAGE_BULBIDENTIFY)
    {
        event.Veto();
    }
    else if (_state == VideoProcessingStates::REVIEW_CUSTOM_MODEL && page != PAGE_REVIEWMODEL)
    {
        event.Veto();
    }
}

void GenerateCustomModelDialog::SwapPage(int oldp, const int newp)
{
    AuiNotebook_ProcessSettings->ChangeSelection(newp);
}

#pragma endregion Generate Tab General Methods

// ***********************************************************
// Model Type tab methods
// ***********************************************************

#pragma region Model Type

void GenerateCustomModelDialog::MTTabEntry()
{
    _state = VideoProcessingStates::CHOOSE_MODELTYPE;
    _displaybmp = wxImage(GCM_DISPLAYIMAGEWIDTH, GCM_DISPLAYIMAGEHEIGHT, true);
    StaticBitmap_Preview->SetBitmap(_displaybmp);
    _draggingedge = -1;
}

void GenerateCustomModelDialog::OnButton_MT_NextClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Generating custom model.");

    TextCtrl_GCM_Filename->SetValue(""); // clear the filename in case the type has changed
    CVTabEntry();
    SwapPage(PAGE_MODELTYPE, PAGE_CHOOSEVIDEO);
    ValidateWindow();
}

#pragma endregion Model Type

// ***********************************************************
// Choose Video tab methods
// ***********************************************************

#pragma region Choose Video

void GenerateCustomModelDialog::CVTabEntry()
{
    ClearLights();
    Button_CV_Next->Enable();
    Button_CV_Back->Enable();
    Button_GCM_SelectFile->Enable();
    TextCtrl_GCM_Filename->Enable();
    SpinCtrl_ProcessNodeCount->Enable();
    CheckBox_BI_IsSteady->Enable();
    CheckBox_AdvancedStartScan->Enable();

    if (SLRadioButton->GetValue())
    {
        StaticText_CM_Request->SetLabel("Select a picture of your static lights model.");
        CheckBox_BI_IsSteady->Hide();
        CheckBox_AdvancedStartScan->Hide();
        SpinCtrl_ProcessNodeCount->Hide();
        StaticText19->Hide();
    }
    else if (NodesManualRadioButtonPg2->GetValue()) {
        SpinCtrl_ProcessNodeCount->Hide();
        StaticText19->Hide();
    }
    else
    {
        StaticText_CM_Request->SetLabel("Select the video you recorded of your model using the prepare tab.");
        CheckBox_BI_IsSteady->Show();
        CheckBox_AdvancedStartScan->Show();
        SpinCtrl_ProcessNodeCount->Show();
        StaticText19->Show();
    }
    _state = VideoProcessingStates::CHOOSE_VIDEO;
    if (FileExists(std::string(TextCtrl_GCM_Filename->GetValue().c_str())))
    {
        if (SLRadioButton->GetValue())
        {
            _generator->SetImage(TextCtrl_GCM_Filename->GetValue());
            ShowImage(_generator->GetFirstFrame()->GetColourImage());
        }
        else
        {
            StaticBitmap_Preview->SetEraseBackground(false);
            _generator->SetVideo(TextCtrl_GCM_Filename->GetValue());
            ShowImage(_generator->GetFirstFrame()->GetColourImage());
        }
        _clip = wxRect(0, 0, _generator->GetFirstFrame()->GetWidth() - 1, _generator->GetFirstFrame()->GetHeight() - 1);
    }
    Panel_ChooseVideo->Layout();
}

void GenerateCustomModelDialog::OnButton_GCM_SelectFileClick(wxCommandEvent& event)
{
    if (SLRadioButton->GetValue())
    {
        FileDialog1->SetWildcard(wxImage::GetImageExtWildcard());
    }
    else
    {
        FileDialog1->SetWildcard(VIDEOWILDCARD);
    }
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        TextCtrl_GCM_Filename->SetValue(FileDialog1->GetDirectory() + "/" + FileDialog1->GetFilename());
        CVTabEntry();
    }

    ValidateWindow();
}

void GenerateCustomModelDialog::OnTextCtrl_GCM_FilenameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void GenerateCustomModelDialog::DisplayImageCallbackCMG(ProcessedImage* image, std::list<std::pair<wxPoint, uint32_t>>* points)
{
    if (points != nullptr) {
        if (_cachedPoints != nullptr) {
            delete _cachedPoints;
            _cachedPoints = nullptr;
        }
        _cachedPoints = new std::list<std::pair<wxPoint, uint32_t>>(*points);
    }

    // add the current points
    CreateDetectedImage(image, true, _cachedPoints);

    //ShowImage(image);
}

std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)> GenerateCustomModelDialog::DisplayImage(bool show)
{
    return show ? std::bind(&GenerateCustomModelDialog::DisplayImageCallbackCMG, this, std::placeholders::_1, std::placeholders::_2) : (std::function<void(ProcessedImage*, std::list<std::pair<wxPoint, uint32_t>>*)>)nullptr;
}

std::function<void(float)> GenerateCustomModelDialog::Progress()
{
    return std::bind(&GenerateCustomModelDialog::UpdateProgress, this, std::placeholders::_1);
}

void GenerateCustomModelDialog::OnButton_CV_NextClick(wxCommandEvent& event)
{
    Button_CV_Next->Disable();
    Button_GCM_SelectFile->Disable();
    Button_CV_Back->Disable();
    TextCtrl_GCM_Filename->Disable();
    SpinCtrl_ProcessNodeCount->Disable();
    CheckBox_BI_IsSteady->Disable();
    CheckBox_AdvancedStartScan->Disable();
    ClearLights();

    static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("File: %s.", (const char*)TextCtrl_GCM_Filename->GetValue().c_str());

    if (SLRadioButton->GetValue()) {
        // static
        StaticText_MinSeparation->Show();
        Slider_BI_MinSeparation->Show();
        TextCtrl_BI_MinSeparation->Show();

        CheckBox_GuessSingle->Hide();

        BITabEntry(true);
        SwapPage(PAGE_CHOOSEVIDEO, PAGE_BULBIDENTIFY);
        DoBulbIdentify();
    } else {
        CheckBox_GuessSingle->Show();

        StaticText_MinSeparation->Hide();
        Slider_BI_MinSeparation->Hide();
        TextCtrl_BI_MinSeparation->Hide();

        ShowProgress(true);

        SetCursor(wxCURSOR_WAIT);
        if (_generator->FindStartFrames(DisplayImage(CheckBox_AdvancedStartScan->IsChecked()), Progress())) {
            ShowImage(_generator->GetStartFrame()->GetColourImage());

            if (!_generator->ReadVideo(SpinCtrl_ProcessNodeCount->GetValue(), CheckBox_BI_IsSteady->IsChecked(), DisplayImage(true), nullptr)) {
                logger_gcm.warn("Video reading generated an error ... but lets keep going.");
            }

            ShowImage(_generator->GetStartFrame()->GetColourImage());

            SFTabEntry();
            SwapPage(PAGE_CHOOSEVIDEO, PAGE_STARTFRAME);
        }
        else {
            wxMessageBox("Start frame could not be found.");
            CVTabEntry();
        }
        ShowProgress(false);
        SetCursor(wxCURSOR_ARROW);
    }
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_CV_BackClick(wxCommandEvent& event)
{
    MTTabEntry();
    SwapPage(PAGE_CHOOSEVIDEO, PAGE_MODELTYPE);
    ValidateWindow();
}

#pragma endregion Choose Video

// ***********************************************************
// Start Frame tab methods
// ***********************************************************

#pragma region Start Frame

void GenerateCustomModelDialog::SFTabEntry()
{
    _state = VideoProcessingStates::FINDING_START_FRAME;
    ClearLights();
    ShowImage(_generator->GetStartFrame()->GetColourImage());
    Button_SF_Next->Enable();
    Button_SF_Back->Enable();
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_SF_NextClick(wxCommandEvent& event)
{
    Button_SF_Next->Disable();
    Button_SF_Back->Disable();

    if (NodesManualRadioButtonPg2->GetValue()) {
        MITabEntry(true);
        SwapPage(PAGE_STARTFRAME, PAGE_MANUALIDENTIFY);
    } else {
        BITabEntry(true);
        SwapPage(PAGE_STARTFRAME, PAGE_BULBIDENTIFY);
        DoBulbIdentify();
    }
    ValidateWindow();
}

void GenerateCustomModelDialog::OnButton_SF_BackClick(wxCommandEvent& event)
{
    SwapPage(PAGE_STARTFRAME, PAGE_CHOOSEVIDEO);
    CVTabEntry();
    ValidateWindow();
}

#pragma endregion Start Frame

// ***********************************************************
// Bulb Identify tab methods
// ***********************************************************

float SliderToGamma(int slider)
{
    return 1.0 + (5 * (float)slider) / 100.0;
}

int GammaToSlider(float gamma)
{
    return (100.0 * (gamma - 1.0)) / 5.0;
}

#pragma region Bulb Identify
void GenerateCustomModelDialog::ShowProgress(bool show)
{

    if (show) {
        Gauge_Progress1->SetValue(0);
        Gauge_Progress2->SetValue(0);
        Gauge_Progress1->Show();
        Gauge_Progress2->Show();
    } else {
        Gauge_Progress1->Hide();
        Gauge_Progress2->Hide();
        Gauge_Progress1->SetValue(0);
        Gauge_Progress2->SetValue(0);
    }
    Panel_BulbIdentify->Layout();
    Panel_ChooseVideo->Layout();
}

void GenerateCustomModelDialog::ClearLights()
{
    _lights.clear();
    if (_cachedPoints != nullptr) {
        delete _cachedPoints;
        _cachedPoints = nullptr;
    }
}

void GenerateCustomModelDialog::DoBulbIdentify()
{
    static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

    if (!_busy) {
        _busy = true;

        CheckBox_GuessSingle->Disable();
        CheckBox_Advanced->Disable();
        Slider_AdjustBlur->Disable();
        Slider_BI_Sensitivity->Disable();
        Slider_BI_MinSeparation->Disable();
        Slider_BI_MinScale->Disable();
        Slider_BI_Contrast->Disable();
        Slider_Saturation->Disable();
        Slider_Gamma->Disable();
        Slider_Despeckle->Disable();
        Button_CB_RestoreDefault->Disable();
        Button_BI_Next->Disable();
        Button_BI_Back->Disable();
        Panel_BulbIdentify->Layout();
        SetCursor(wxCURSOR_WAIT);
        StaticBitmap_Preview->SetEraseBackground(false);

        ShowProgress(true);

        ClearLights();
        CreateDetectedImage(nullptr, false, &_lights);

        wxYield(); // let them update

        logger_gcm.info("Executing bulb identify.");
        logger_gcm.info("   Image Size: %dx%d.", _generator->GetFirstFrame()->GetWidth(), _generator->GetFirstFrame()->GetHeight());
        logger_gcm.info("   Blur: %d.", Slider_AdjustBlur->GetValue());
        logger_gcm.info("   Sensitivity: %d.", Slider_BI_Sensitivity->GetValue());
        logger_gcm.info("   Contrast: %d.", Slider_BI_Contrast->GetValue());
        logger_gcm.info("   Erode/Dilate: %d.", Slider_Despeckle->GetValue());
        logger_gcm.info("   Gamma: %s.", (const char*)TextCtrl_Gamma->GetValue().c_str());
        logger_gcm.info("   Saturation: %d.", Slider_Saturation->GetValue());
        logger_gcm.info("   Minimum Separation: %d.", Slider_BI_MinSeparation->GetValue());
        logger_gcm.info("   Minimum Scale: %d.", Slider_BI_MinScale->GetValue());
        logger_gcm.info("   Clip Rectangle: (%d,%d)-(%d,%d).", _clip.GetLeft(), _clip.GetTop(), _clip.GetRight(), _clip.GetBottom());

        if (SLRadioButton->GetValue()) {
            VideoFrame* vf = new VideoFrame(_generator->GetFirstFrame()->GetColourImage(), 0, false, VideoFrame::VIDEO_FRAME_TYPE::VFT_IMAGE_MULTI);
            auto nvf = vf->Process(_clip.GetLeft(), _clip.GetRight(), vf->GetHeight() - _clip.GetTop(), vf->GetHeight() - _clip.GetBottom(), Slider_BI_Contrast->GetValue(), Slider_AdjustBlur->GetValue(), Slider_Despeckle->GetValue(), Slider_BI_Sensitivity->GetValue(), SliderToGamma(Slider_Gamma->GetValue()), Slider_Saturation->GetValue());
            delete vf;

            ProcessedImage* pi = nullptr;
            if (CheckBox_Advanced->IsChecked()) {
                pi = new ProcessedImage(nvf->GetGreyscaleImage(), ProcessedImage::P_IMG_FRAME_TYPE::P_IMG_IMAGE_GREYSCALE);
            }

            _lights = nvf->GetGreyscaleImage()->FindPixels(1, Slider_BI_MinSeparation->GetValue(), Progress());
            delete nvf;

            CreateDetectedImage(pi, false);
            if (pi != nullptr) {
                delete pi;
            }
        } else {
            ProcessedImage* pi = nullptr;
            //_lights = _generator->FindLights(SpinCtrl_ProcessNodeCount->GetValue(), _clip.GetLeft(), _clip.GetRight(), _generator->GetFirstFrame()->GetHeight() - _clip.GetTop(), _generator->GetFirstFrame()->GetHeight() - _clip.GetBottom(),
            //                                 Slider_BI_Contrast->GetValue(), Slider_AdjustBlur->GetValue(), Slider_Despeckle->GetValue(), Slider_BI_Sensitivity->GetValue(),
            //                                 SliderToGamma(Slider_Gamma->GetValue()), Slider_Saturation->GetValue(), &pi, std::bind(&GenerateCustomModelDialog::DisplayImageCallbackCMG, this, std::placeholders::_1));
            _lights = _generator->FindLightsA(SpinCtrl_ProcessNodeCount->GetValue(), _clip.GetLeft(), _clip.GetRight(), _generator->GetFirstFrame()->GetHeight() - _clip.GetTop(), _generator->GetFirstFrame()->GetHeight() - _clip.GetBottom(),
                                              Slider_BI_Contrast->GetValue(), Slider_AdjustBlur->GetValue(), Slider_Despeckle->GetValue(), Slider_BI_Sensitivity->GetValue(),
                                              SliderToGamma(Slider_Gamma->GetValue()), Slider_Saturation->GetValue(), &pi, DisplayImage(CheckBox_Advanced->IsChecked()), Progress());

            if (CheckBox_GuessSingle->IsChecked()) {
                GuessMissingBulbs();
            }

            CreateDetectedImage(pi, true);
            if (pi != nullptr)
                delete pi;
        }
        StaticBitmap_Preview->SetEraseBackground(true);
        CheckBox_Advanced->Enable();
        CheckBox_GuessSingle->Enable();
        Slider_AdjustBlur->Enable();
        Slider_BI_Sensitivity->Enable();
        Slider_BI_MinSeparation->Enable();
        Slider_BI_MinScale->Enable();
        Slider_BI_Contrast->Enable();
        Slider_Despeckle->Enable();
        Slider_Gamma->Enable();
        Slider_Saturation->Enable();
        Button_CB_RestoreDefault->Enable();
        Button_BI_Next->Enable();
        Button_BI_Back->Enable();
        Panel_BulbIdentify->Layout();

        if (_lights.size() > 0) {
            _minimumSeparation = 99999999;
            auto it1 = _lights.begin();
            auto it2 = it1;
            ++it2;
            while (it2 != _lights.end()) {
                uint32_t separation = ProcessedImage::PixelDistance(it1->first, it2->first);
                if (separation < _minimumSeparation)
                    _minimumSeparation = separation;
                ++it1;
                ++it2;
            }
        }

        ShowProgress(false);

        SetCursor(wxCURSOR_ARROW);
        logger_gcm.info("Result: %s.", (const char*)TextCtrl_BI_Status->GetValue().c_str());
        _busy = false;
    }
}

void GenerateCustomModelDialog::BITabEntry(bool setdefault)
{
    _state = VideoProcessingStates::IDENTIFYING_BULBS;
    if (setdefault)
    {
        ClearLights();
        TextCtrl_BI_Status->SetValue("");
        SetBIDefault();
    }
    CreateDetectedImage(nullptr, ShowPixelLines());
    StaticText_BI->SetLabel("The red circles on the image show the bulbs we have identified. Adjust the sensitivity if there are bulbs missing or phantom bulbs identified.\n\nClick next when you are happy that all bulbs have been detected.");
    CheckBox_Advanced->Enable();
    CheckBox_GuessSingle->Enable();
    Slider_BI_Sensitivity->Enable();
    Slider_BI_MinSeparation->Enable();
    Slider_BI_MinScale->Enable();
    if (CheckBox_Advanced->IsChecked()) {
        Slider_AdjustBlur->Show();
        Slider_BI_Contrast->Show();
        Slider_Saturation->Show();
        Slider_Gamma->Show();
        Slider_Despeckle->Show();

        TextCtrl_BC_Blur->Show();
        TextCtrl_BI_Contrast->Show();
        TextCtrl_Saturation->Show();
        TextCtrl_Gamma->Show();
        TextCtrl_Despeckle->Show();

        StaticTextDespeckle->Show();
        StaticText_Blur->Show();
        StaticText_Contrast->Show();
        StaticText_Gamma->Show();
        StaticText_Saturation->Show();

        Slider_AdjustBlur->Enable();
        Slider_BI_Contrast->Enable();
        Slider_Saturation->Enable();
        Slider_Gamma->Enable();
        Slider_Despeckle->Enable();
        Button_CB_RestoreDefault->Enable();
    }
    else {
        Slider_AdjustBlur->Hide();
        Slider_BI_Contrast->Hide();
        Slider_Saturation->Hide();
        Slider_Gamma->Hide();
        Slider_Despeckle->Hide();

        TextCtrl_BC_Blur->Hide();
        TextCtrl_BI_Contrast->Hide();
        TextCtrl_Saturation->Hide();
        TextCtrl_Gamma->Hide();
        TextCtrl_Despeckle->Hide();

        StaticTextDespeckle->Hide();
        StaticText_Blur->Hide();
        StaticText_Contrast->Hide();
        StaticText_Gamma->Hide();
        StaticText_Saturation->Hide();
    }
    Button_BI_Next->Enable();
    Button_BI_Back->Enable();
    Panel_BulbIdentify->Layout();
    Layout();
}

int GenerateCustomModelDialog::GetMaxNum()
{
    uint32_t max = 0;
    for (const auto& it : _lights) {
        if (it.second > max) {
            max = it.second;
        }
    }

    return max;
}

int GenerateCustomModelDialog::GetBulbCount()
{
    return _lights.size();
}

// Assumes nodes are in order
wxString GenerateCustomModelDialog::GetMissingNodes()
{
    wxString res;

    std::list<uint32_t> used;
    for (const auto& it : _lights) {
        used.push_back(it.second);
    }

    used.sort();

    uint32_t upTo = 0;
    for (const auto& it : used) {
        ++upTo;
        if (it > upTo) {
            for (uint32_t i = upTo; i < it; ++i) {
                if (res != "") {
                    res += ", ";
                }
                res += std::to_string(i);
            }
        }
        upTo = it;
    }

    return res;
}

void GenerateCustomModelDialog::GuessMissingBulbs()
{
    static log4cpp::Category& logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));

    // make sure lights are in order
    _lights.sort(LightsCompare);

    uint32_t next = 1;
    wxPoint last;
    for (auto it = _lights.begin(); it != _lights.end(); ++it) {
        if (it->second > next) {
            uint32_t missing = it->second - next;
            float distance = ProcessedImage::PixelDistance(last, it->first);
            if (distance < 1)
                distance = 1;
            float incr = distance / (missing + 1);
            logger_gcm.debug("%u Bulbs missing %u-%u", missing, next, next + missing - 1);
            for (uint32_t i = 0; i < missing; ++i) {
                uint32_t x = (float)last.x + (((float)(i + 1) * incr) / distance) * (float)(it->first.x - last.x);
                uint32_t y = (float)last.y + (((float)(i + 1) * incr) / distance) * (float)(it->first.y - last.y);
                logger_gcm.debug("  Bulb missing for node %u ... added at %u,%u", next + i, x, y);
                _lights.insert(it, std::pair<wxPoint, uint32_t>(wxPoint(x, y), next + i));
            }
        }
        next = it->second + 1;
        last = it->first;
    }
}

wxString GenerateCustomModelDialog::GenerateStats()
{
    wxString res;

    int n = GetMaxNum();
    if (n < 1) {
        res += "Bulbs: " + std::to_string(GetBulbCount()) + "\n";
    } else {
        res += "Nodes: " + std::to_string(n) + "\n";
    }

    // wxString mn = GetMissingNodes();
    // if (mn == "")
    //{
    //     res += "Missing Nodes: N/A\n";
    // }
    // else
    //{
    //     res += wxString::Format("Missing Nodes: %s\n", mn);
    // }

    wxSize p = CalcSize();
    res += "Model size: " + std::to_string(p.x) + "x" + std::to_string(p.y) + "\n";

    return res;
}

void GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_Sensitivity->SetValue(std::to_string(Slider_BI_Sensitivity->GetValue()));
}

void GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScroll(wxScrollEvent& event)
{
    TextCtrl_BC_Blur->SetValue(std::to_string(Slider_AdjustBlur->GetValue()));
}

void GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_MinSeparation->SetValue(std::to_string(Slider_BI_MinSeparation->GetValue()));
}

void GenerateCustomModelDialog::SetBIDefault()
{
    Slider_BI_MinSeparation->SetValue(20);
    TextCtrl_BI_MinSeparation->SetValue("20");
    Slider_BI_MinScale->SetValue(100);
    TextCtrl_BI_MinScale->SetValue("100");
    Slider_AdjustBlur->SetValue(1);
    TextCtrl_BC_Blur->SetValue("1");
    Slider_BI_Sensitivity->SetValue(100);
    TextCtrl_BI_Sensitivity->SetValue("100");
    Slider_BI_Contrast->SetValue(0);
    TextCtrl_BI_Contrast->SetValue("0");
    Slider_Despeckle->SetValue(0);
    TextCtrl_Despeckle->SetValue("0");
    if (NodesRadioButtonPg2->GetValue()) {
        Slider_Gamma->SetValue(GammaToSlider(1.0f)); // 2.2
        TextCtrl_Gamma->SetValue("1.0");
        Slider_Saturation->SetValue(20); // 100
        TextCtrl_Saturation->SetValue("20");
    } else {
        Slider_Gamma->SetValue(GammaToSlider(1.0f));
        TextCtrl_Gamma->SetValue("1.0");
        Slider_Saturation->SetValue(0);
        TextCtrl_Saturation->SetValue("0");
    }
}

void GenerateCustomModelDialog::OnButton_BI_RestoreDefaultClick(wxCommandEvent& event)
{
    if (!_busy) {
        SetBIDefault();
        DoBulbIdentify();
    }
}

void GenerateCustomModelDialog::OnSlider_AdjustBlurCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_BI_SensitivityCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_BI_MinSeparationCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_BI_ContrastCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnCheckBox_BI_IsSteadyClick(wxCommandEvent& event)
{
}

void GenerateCustomModelDialog::OnSlider_BI_ContrastCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_Contrast->SetValue(std::to_string(Slider_BI_Contrast->GetValue()));
}

void GenerateCustomModelDialog::OnButton_BI_NextClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        DoGenerateCustomModel();
        CMTabEntry();
        SwapPage(PAGE_BULBIDENTIFY, PAGE_REVIEWMODEL);
    }
}

void GenerateCustomModelDialog::OnButton_BI_BackClick(wxCommandEvent& event)
{
    if (!_busy)
    {
        if (SLRadioButton->GetValue())
        {
            CVTabEntry();
            SwapPage(PAGE_BULBIDENTIFY, PAGE_CHOOSEVIDEO);
        }
        else
        {
            SFTabEntry();
            SwapPage(PAGE_BULBIDENTIFY, PAGE_STARTFRAME);
        }
    }
}

#pragma endregion Bulb Identify

// ***********************************************************
// Custom Model tab methods
// ***********************************************************

#pragma region Custom Model

bool GenerateCustomModelDialog::ShowPixelLines()
{
    return NodesRadioButtonPg2->GetValue();
}

void GenerateCustomModelDialog::CMTabEntry()
{
    _state = VideoProcessingStates::REVIEW_CUSTOM_MODEL;
    CreateDetectedImage(nullptr, ShowPixelLines());
}

wxSize GenerateCustomModelDialog::CalcSize(wxPoint* offset, float* multiplier)
{
    if (_lights.size() == 0)
    {
        return wxSize(0, 0);
    }

    if (_minimumSeparation == 0)
        _minimumSeparation = 1;
    float mult = 1.0 / (float)_minimumSeparation;

    uint32_t minX = 9999999;
    uint32_t minY = 9999999;
    for (const auto& it : _lights) {
        if (it.first.x < minX)
            minX = it.first.x;
        if (it.first.y < minY)
            minY = it.first.y;
    }

    if (offset != nullptr) {
        *offset = wxPoint(minX, minY);
    }

    float maxX = 0;
    float maxY = 0;

    // rescale everything and zero base it
    for (auto& it : _lights) {
        uint32_t x = (float)(it.first.x - minX) * mult;
        uint32_t y = (float)(it.first.y - minY) * mult;
        if (x > maxX)
            maxX = x;
        if (y > maxY)
            maxY = y;
    }

    maxX = maxX + ((maxX * (Slider_BI_MinScale->GetValue() - 1) * MODEL_SIZE_MULTIPLER) / 100);
    maxY = maxY + ((maxY * (Slider_BI_MinScale->GetValue() - 1) * MODEL_SIZE_MULTIPLER) / 100);

    if (multiplier != nullptr) {
        *multiplier = mult + ((mult * (Slider_BI_MinScale->GetValue() - 1.0) * MODEL_SIZE_MULTIPLER) / 100.0);
    }

    return wxSize(maxX + MATRIX_FUDGE, maxY + MATRIX_FUDGE);
}

// this will find the best scale to 1/100th of the imput size
void GenerateCustomModelDialog::DoGenerateCustomModel()
{
    if (_lights.size() == 0) {
        return;
    }

    wxPoint offset = wxPoint(0, 0);
    float multiplier = 1.0;
    auto size = CalcSize(&offset, &multiplier);
    Grid_CM_Result->ClearGrid();
    if (Grid_CM_Result->GetNumberCols() > 0) {
        Grid_CM_Result->DeleteCols(0, Grid_CM_Result->GetNumberCols());
        if (Grid_CM_Result->GetNumberRows() > 0) {
            Grid_CM_Result->DeleteRows(0, Grid_CM_Result->GetNumberRows());
        }
        Grid_CM_Result->AppendCols(size.x);
        Grid_CM_Result->AppendRows(size.y);
    } else {
        Grid_CM_Result->CreateGrid(size.y, size.x);
    }

    for (const auto& it : _lights) {
        wxPoint p = wxPoint((it.first.x - offset.x) * multiplier, (it.first.y - offset.y) * multiplier);
        Grid_CM_Result->SetCellValue(p.y, p.x, std::to_string(it.second));
        if (IsDarkMode()) {
            Grid_CM_Result->SetCellBackgroundColour(p.y, p.x, wxColor(0, 128, 0));
        } else {
            Grid_CM_Result->SetCellBackgroundColour(p.y, p.x, *wxGREEN);
        }
    }

    wxFont font = Grid_CM_Result->GetDefaultCellFont();
    SetGridSizeForFont(font);
    FlexGridSizer19->Layout();
    Layout();
}

void GenerateCustomModelDialog::SetGridSizeForFont(const wxFont& font)
{
    Grid_CM_Result->Freeze();
    Grid_CM_Result->BeginBatch();
    Grid_CM_Result->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    Grid_CM_Result->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < Grid_CM_Result->GetNumberCols(); ++c) {
        Grid_CM_Result->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
    }
    for (int r = 0; r < Grid_CM_Result->GetNumberRows(); ++r) {
        Grid_CM_Result->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
    }
    Grid_CM_Result->SetDefaultRowSize(int(1.5 * (float)font.GetPixelSize().y));
    Grid_CM_Result->SetDefaultColSize(2 * font.GetPixelSize().y);
    Grid_CM_Result->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
    Grid_CM_Result->SetRowLabelSize(int(2.5 * font.GetPixelSize().y));

    Grid_CM_Result->EndBatch();
    Grid_CM_Result->Thaw();
}

void GenerateCustomModelDialog::OnButton_CM_BackClick(wxCommandEvent& event)
{
    if (NodesManualRadioButtonPg2->GetValue()) {
        MITabEntry(false);
        SwapPage(PAGE_REVIEWMODEL, PAGE_MANUALIDENTIFY);
    } else {
        BITabEntry(false);
        SwapPage(PAGE_REVIEWMODEL, PAGE_BULBIDENTIFY);
    }
}

wxString GenerateCustomModelDialog::CreateCustomModelData()
{
    wxString res = "";
    for (int y = 0; y < Grid_CM_Result->GetNumberRows(); ++y)
    {
        for (int x = 0; x < Grid_CM_Result->GetNumberCols(); ++x)
        {
            res += Grid_CM_Result->GetCellValue(y, x);

            if (x + 1 != Grid_CM_Result->GetNumberCols())
            {
                res += ",";
            }
        }

        if (y + 1 != Grid_CM_Result->GetNumberRows())
        {
            res += ";";
        }
    }

    return res;
}

void GenerateCustomModelDialog::OnButton_CM_SaveClick(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "NewCustomModel", wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    static log4cpp::Category &logger_gcm = log4cpp::Category::getInstance(std::string("log_generatecustommodel"));
    logger_gcm.info("Saving to xmodel file %s.", (const char *)filename.c_str());
    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError("Unable to create file "+filename+". Error "+std::to_string(f.GetLastError())+"\n");
        return;
    }
    wxString name = wxFileName(filename).GetName();
    wxString cm = CreateCustomModelData();
    wxString p1 = std::to_string(Grid_CM_Result->GetNumberCols());
    wxString p2 = std::to_string(Grid_CM_Result->GetNumberRows());
    wxString st;
    if (SLRadioButton->GetValue())
    {
        st = "Single Color White";
    }
    else
    {
        st = "RGB Nodes";
    }
    wxString ps = "2";
    wxString t = "0";
    wxString mb = "0";
    wxString a = "1";
    wxString sn = "";
    wxString nn = "";
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    f.Write("</custommodel>");
    f.Close();
}

void GenerateCustomModelDialog::OnButton_GrowClick(wxCommandEvent& event)
{
    Grid_CM_Result->BeginBatch();
    wxFont font = Grid_CM_Result->GetLabelFont();
    font.MakeLarger();
    Grid_CM_Result->SetLabelFont(font);
    auto fs = font.GetPointSize();
    font = Grid_CM_Result->GetDefaultCellFont();
    font.SetPointSize(fs);
    Grid_CM_Result->SetDefaultCellFont(font);
    Grid_CM_Result->EndBatch();

    font = Grid_CM_Result->GetDefaultCellFont();
    SetGridSizeForFont(font);
}

void GenerateCustomModelDialog::OnButton_ShrinkClick(wxCommandEvent& event)
{
    Grid_CM_Result->BeginBatch();
    wxFont font = Grid_CM_Result->GetLabelFont();
    auto fs = font.GetPointSize();
    font.MakeSmaller();
    if (fs != font.GetPointSize()) {
        fs = font.GetPointSize();
        Grid_CM_Result->SetLabelFont(font);
        font = Grid_CM_Result->GetDefaultCellFont();
        font.SetPointSize(fs);
        Grid_CM_Result->SetDefaultCellFont(font);
    }

    Grid_CM_Result->EndBatch();

    font = Grid_CM_Result->GetDefaultCellFont();
    SetGridSizeForFont(font);
}
#pragma endregion Custom Model

#pragma endregion Generate

int GenerateCustomModelDialog::GetEdge(int x, int y)
{
    wxSize displaysize = StaticBitmap_Preview->GetSize();
    float xf = (float)_generator->GetFirstFrame()->GetWidth() / (float)displaysize.GetWidth();
    float yf = (float)_generator->GetFirstFrame()->GetHeight() / (float)displaysize.GetHeight();
    int edge = -1;

    if (std::abs(xf*x - _clip.GetLeft()) < 3 * xf)
    {
        if (yf*y >= _clip.GetTop() && yf*y <= _clip.GetBottom())
        {
            edge = 3;
        }
    }
    else if (std::abs(xf*x - _clip.GetRight()) < 5 * xf)
    {
        if (yf*y >= _clip.GetTop() && yf*y <= _clip.GetBottom())
        {
            edge = 1;
        }
    }
    else if (std::abs(yf*y - _clip.GetTop()) < 5*yf)
    {
        if (xf*x >= _clip.GetLeft() && xf*x <= _clip.GetRight())
        {
            edge = 0;
        }
    }
    else if (std::abs(yf*y - _clip.GetBottom()) < 5*yf)
    {
        if (xf*x >= _clip.GetLeft() && xf*x <= _clip.GetRight())
        {
            edge = 2;
        }
    }

    return edge;
}

wxPoint GenerateCustomModelDialog::NormalisePoint(const wxPoint& pt) const
{
    wxSize displaysize = StaticBitmap_Preview->GetSize();
    int w = _generator->GetFirstFrame()->GetWidth();
    int h = _generator->GetFirstFrame()->GetHeight();
    int xf = (float)w / (float)displaysize.GetWidth() * (float)pt.x;
    if (xf < 0) {
        xf = 0;
    } else if (xf > w - 1) {
        xf = w - 1;
    }
    int yf = (float)h / (float)displaysize.GetHeight() * (float)pt.y;
    if (yf < 0) {
        yf = 0;
    } else if (yf > h - 1) {
        yf = h - 1;
    }

    return { xf, yf };
}

void GenerateCustomModelDialog::ResizeClip(int x, int y)
{
    if (_draggingedge == -1)
    {
        return;
    }
    else
    {
        wxPoint pt = NormalisePoint({ x, y });
        if (_draggingedge == 0)
        {
            int b = _clip.GetBottom();
            _clip.SetTop(pt.y);
            _clip.SetBottom(b);
        }
        else if (_draggingedge == 1)
        {
            _clip.SetRight(pt.x);
        }
        else if (_draggingedge == 2)
        {
            _clip.SetBottom(pt.y);
        }
        else if (_draggingedge == 3)
        {
            int r = _clip.GetRight();
            _clip.SetLeft(pt.x);
            _clip.SetRight(r);
        }
    }
    StaticBitmap_Preview->SetEraseBackground(false);
    CreateDetectedImage(nullptr, ShowPixelLines());
    StaticBitmap_Preview->SetEraseBackground(true);
}

void GenerateCustomModelDialog::OnStaticBitmapLeftDown(wxMouseEvent& event)
{
    // only doing dragging on one page
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        _draggingedge = GetEdge(event.GetX(), event.GetY());
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
            if (_draggingedge % 2 == 0)
            {
                SetCursor(wxCURSOR_SIZENS);
            }
            else
            {
                SetCursor(wxCURSOR_SIZEWE);
            }
        }
    } else if (_state == VideoProcessingStates::IDENTIFYING_MANUAL) {
        _lights.push_back({ NormalisePoint({ event.GetX(), event.GetY() }), (uint32_t)_lights.size() + 1 });
        _MIVideoTime += NODEON;
        ProcessedImage* pi = _generator->ReadVideoFrame(_MIVideoTime);
        pi->Subtract(_generator->GetBlankFrame()->GetColourImage(), MANUAL_IMAGE_SUBTRACT);
        CreateDetectedImage(pi, true);
        delete pi;
        TextCtrl_CurrentBulb->SetValue(std::to_string(_lights.size() + 1));
        if (_lights.size() > 0) {
            _minimumSeparation = 99999999;
            auto it1 = _lights.begin();
            auto it2 = it1;
            ++it2;
            while (it2 != _lights.end()) {
                uint32_t separation = ProcessedImage::PixelDistance(it1->first, it2->first);
                if (separation < _minimumSeparation)
                    _minimumSeparation = separation;
                ++it1;
                ++it2;
            }
        }
        ValidateWindow();
    }
}

void GenerateCustomModelDialog::OnStaticBitmapLeftDClick(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS) {
        _clip = wxRect(0, 0, _generator->GetFirstFrame()->GetWidth() - 1, _generator->GetFirstFrame()->GetHeight() - 1);
        DoBulbIdentify();
        CreateDetectedImage(nullptr, ShowPixelLines());
    }
}

void GenerateCustomModelDialog::OnStaticBitmapLeftUp(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
            DoBulbIdentify();
            CreateDetectedImage(nullptr, ShowPixelLines());
        }
        _draggingedge = -1;
        SetCursor(wxCURSOR_ARROW);
    }
}

void GenerateCustomModelDialog::OnStaticBitmapMouseLeave(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(std::min(event.GetX(), (int) _generator->GetFirstFrame()->GetWidth() - 1), std::min(event.GetY(), (int)_generator->GetFirstFrame()->GetHeight() - 1));
        }
        _draggingedge = -1;
        SetCursor(wxCURSOR_ARROW);
    }
}

void GenerateCustomModelDialog::OnStaticBitmapMouseEnter(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_MANUAL)
    {
        SetCursor(wxCURSOR_CROSS);
    }
}

void GenerateCustomModelDialog::OnStaticBitmapMouseMove(wxMouseEvent& event)
{
    if (_state == VideoProcessingStates::IDENTIFYING_BULBS)
    {
        if (_draggingedge >= 0)
        {
            ResizeClip(event.GetX(), event.GetY());
        }
        else
        {
            int e = GetEdge(event.GetX(), event.GetY());
            if (e == -1)
            {
                SetCursor(wxCURSOR_ARROW);
            }
            else if (e % 2 == 0)
            {
                SetCursor(wxCURSOR_SIZENS);
            }
            else
            {
                SetCursor(wxCURSOR_SIZEWE);
            }
        }
    }
}

void GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_BI_MinScaleCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_BI_MinScale->SetValue(std::to_string(Slider_BI_MinScale->GetValue()));
}

void GenerateCustomModelDialog::OnSlider_DespeckleCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_DespeckleCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Despeckle->SetValue(std::to_string(Slider_Despeckle->GetValue()));
}

void GenerateCustomModelDialog::OnCheckBox_GuessSingleClick(wxCommandEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_GammaCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_GammaCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Gamma->SetValue(wxString::Format(wxT("%.2f"), SliderToGamma(Slider_Gamma->GetValue())));
}

void GenerateCustomModelDialog::OnSlider_SaturationCmdScrollChanged(wxScrollEvent& event)
{
    DoBulbIdentify();
}

void GenerateCustomModelDialog::OnSlider_SaturationCmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Saturation->SetValue(std::to_string(Slider_Saturation->GetValue()));
}

void GenerateCustomModelDialog::OnCheckBox_AdvancedClick(wxCommandEvent& event)
{
    BITabEntry(!CheckBox_Advanced->GetValue());
    // re-identify if the defaults were possibly set
    if (!CheckBox_Advanced->GetValue()) {
        DoBulbIdentify();
    }
}

void GenerateCustomModelDialog::OnButton_MI_NextClick(wxCommandEvent& event)
{
    DoGenerateCustomModel();
    CMTabEntry();
    SwapPage(PAGE_MANUALIDENTIFY, PAGE_REVIEWMODEL);
}

void GenerateCustomModelDialog::OnButton_MI_BackClick(wxCommandEvent& event)
{
    SFTabEntry();
    SwapPage(PAGE_MANUALIDENTIFY, PAGE_STARTFRAME);
}

void GenerateCustomModelDialog::OnButtonBumpFwdClick(wxCommandEvent& event)
{
    _MIVideoTime += 50;
    ProcessedImage* pi = _generator->ReadVideoFrame(_MIVideoTime);
    pi->Subtract(_generator->GetBlankFrame()->GetColourImage(), MANUAL_IMAGE_SUBTRACT);
    CreateDetectedImage(pi, true);
    delete pi;
}

void GenerateCustomModelDialog::OnButton_BumpBackClick(wxCommandEvent& event)
{
    if (_MIVideoTime > 50) {
        _MIVideoTime -= 50;
    }
    else {
        _MIVideoTime = 0;
    }
    ProcessedImage* pi = _generator->ReadVideoFrame(_MIVideoTime);
    pi->Subtract(_generator->GetBlankFrame()->GetColourImage(), MANUAL_IMAGE_SUBTRACT);
    CreateDetectedImage(pi, true);
    delete pi;
}

void GenerateCustomModelDialog::OnButton_MI_Back1BulbClick(wxCommandEvent& event)
{
    wxASSERT(_lights.size() > 0);
    _lights.pop_back();
    TextCtrl_CurrentBulb->SetValue(std::to_string(_lights.size() + 1));

    _MIVideoTime -= NODEON;

    ProcessedImage* pi = _generator->ReadVideoFrame(_MIVideoTime);
    pi->Subtract(_generator->GetBlankFrame()->GetColourImage(), MANUAL_IMAGE_SUBTRACT);
    CreateDetectedImage(pi, true);
    delete pi;

    if (_lights.size() > 0) {
        _minimumSeparation = 99999999;
        auto it1 = _lights.begin();
        auto it2 = it1;
        ++it2;
        while (it2 != _lights.end()) {
            uint32_t separation = ProcessedImage::PixelDistance(it1->first, it2->first);
            if (separation < _minimumSeparation)
                _minimumSeparation = separation;
            ++it1;
            ++it2;
        }
    }

    ValidateWindow();
}

void GenerateCustomModelDialog::MITabEntry(bool setdefault)
{
    _state = VideoProcessingStates::IDENTIFYING_MANUAL;
    if (setdefault) {
        ClearLights();
        _MIVideoTime = _generator->GetFirstNodeFrameTime();
        Slider_BI_MinScale->SetValue(100);
        Slider_MI_ModelScale->SetValue(100);
        TextCtrl_MI_ModelScale->SetValue("100");
    }
    ProcessedImage* pi = _generator->ReadVideoFrame(_MIVideoTime);
    pi->Subtract(_generator->GetBlankFrame()->GetColourImage(), MANUAL_IMAGE_SUBTRACT);
    CreateDetectedImage(pi, true);
    delete pi;
    Button_MI_Next->Enable();
    Button_MI_Back->Enable();
    ValidateWindow();
}

void GenerateCustomModelDialog::OnSlider_MI_ModelScaleCmdScrollThumbRelease(wxScrollEvent& event)
{
    // copy value to the BI equivalent as the model generation routine looks at that value
    Slider_BI_MinScale->SetValue(Slider_MI_ModelScale->GetValue());
}

void GenerateCustomModelDialog::OnSlider_MI_ModelScaleCmdSliderUpdated(wxScrollEvent& event)
{
    // copy value to the BI equivalent as the model generation routine looks at that value
    Slider_BI_MinScale->SetValue(Slider_MI_ModelScale->GetValue());
    TextCtrl_MI_ModelScale->SetValue(std::to_string(Slider_MI_ModelScale->GetValue()));
}
