/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Node.h"

const std::string NodeBaseClass::RED("R");
const std::string NodeBaseClass::GREEN("G");
const std::string NodeBaseClass::BLUE("B");
const std::string NodeBaseClass::WHITE("W");

const std::string NodeBaseClass::RGB("RGB");
const std::string NodeBaseClass::RBG("RBG");
const std::string NodeBaseClass::GBR("GBR");
const std::string NodeBaseClass::GRB("GRB");
const std::string NodeBaseClass::BRG("BRG");
const std::string NodeBaseClass::BGR("BGR");

const std::string NodeBaseClass::WRGB("WRGB");
const std::string NodeBaseClass::WRBG("WRBG");
const std::string NodeBaseClass::WGBR("WGBR");
const std::string NodeBaseClass::WGRB("WGRB");
const std::string NodeBaseClass::WBRG("WBRG");
const std::string NodeBaseClass::WBGR("WBGR");

const std::string NodeBaseClass::RGBW("RGBW");
const std::string NodeBaseClass::RBGW("RBGW");
const std::string NodeBaseClass::GBRW("GBRW");
const std::string NodeBaseClass::GRBW("GRBW");
const std::string NodeBaseClass::BRGW("BRGW");
const std::string NodeBaseClass::BGRW("BGRW");


const std::string &NodeBaseClass::GetNodeType() const {
    switch (offsets[0]) {
        case 0:
            if (offsets[1] == 1) {
                return RGB;
            }
            return RBG;
        case 1:
            if (offsets[1] == 0) {
                return GRB;
            }
            return BRG;
        default:
            break;
    }
    if (offsets[1] == 0) {
        return GBR;
    }
    return BGR;
}


const std::string &NodeClassRGBW::GetNodeType() const {
    switch (offsets[0]) {
        case 0:
            if (offsets[1] == 1) {
                return wIndex == 0 ? WRGB : RGBW;
            }
            return RBG;
        case 1:
            if (offsets[1] == 0) {
                return wIndex == 0 ? WGRB : GRBW;
            }
            return wIndex == 0 ? WBRG : GRBW;
        default:
            break;
    }
    if (offsets[1] == 0) {
        return wIndex == 0 ? WGBR : GBRW;
    }
    return wIndex == 0 ? WBGR : BGRW;
}

#define RGB_HANDLING_NORMAL   0
#define RGB_HANDLING_RGB      1
#define RGB_HANDLING_WHITE    2
#define RGB_HANDLING_ADVANCED 3
#define RGB_HANDLING_ALL      4

void NodeClassRGBW::SetFromChannels(const unsigned char* buf)
{
    switch (rgbwHandling) {
    case RGB_HANDLING_RGB:
    case RGB_HANDLING_ALL:
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                c[x] = buf[offsets[x] + wOffset];
            }
        }
        break;
    case RGB_HANDLING_WHITE:
        c[0] = c[1] = c[2] = buf[wIndex];
        break;
    case RGB_HANDLING_ADVANCED:

        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                c[x] = buf[offsets[x] + wOffset] + buf[wIndex];
            }
        }

        break;

    default: //RGB_HANDLING_NORMAL
        if (buf[wIndex] != 0) {
            c[0] = c[1] = c[2] = buf[wIndex];
        }
        else {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    c[x] = buf[offsets[x] + wOffset];
                }
            }
        }
        break;
    }
}

void NodeClassRGBW::GetForChannels(unsigned char* buf) const
{
    switch (rgbwHandling) {
    case RGB_HANDLING_RGB:
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                buf[offsets[x] + wOffset] = c[x];
            }
        }
        buf[wIndex] = 0;
        break;
    case RGB_HANDLING_WHITE:
        if (c[0] == c[1] && c[1] == c[2]) {
            buf[wIndex] = c[0];
        }
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                buf[offsets[x] + wOffset] = 0;
            }
        }
        break;
    case RGB_HANDLING_ALL:
        if (c[0] == c[1] && c[1] == c[2]) {
            buf[wIndex] = c[0];
        }

        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                buf[offsets[x] + wOffset] = c[x];
            }
        }
        break;
    case RGB_HANDLING_ADVANCED: {
        uint8_t maxc = std::max(c[0], std::max(c[1], c[2]));
        if (maxc == 0) {
            buf[wIndex] = 0;
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x] + wOffset] = 0;
                }
            }
        } else {
            uint8_t minc = std::min(c[0], std::min(c[1], c[2]));
            // find colour with 100% hue
            float multiplier = 255.0f / maxc;
            float h0 = c[0] * multiplier;
            float h1 = c[1] * multiplier;
            float h2 = c[2] * multiplier;

            float maxW = std::max(h0, std::max(h1, h2));
            float minW = std::min(h0, std::min(h1, h2));
            uint8_t whiteness = ((maxW + minW) / 2.0f - 127.5f) * (255.0f / 127.5f) / multiplier;
            if (whiteness < 0)
                whiteness = 0;
            else if (whiteness > minc)
                whiteness = minc;

            buf[wIndex] = whiteness;
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x] + wOffset] = c[x] - whiteness;
                }
            }
        }
    } break;
    default: // RGB_HANDLING_NORMAL
        if (c[0] == c[1] && c[1] == c[2]) {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x] + wOffset] = 0;
                }
            }
            buf[wIndex] = c[0];
        } else {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x] + wOffset] = c[x];
                }
            }
            buf[wIndex] = 0;
        }
        break;
    }
}

void NodeClassSuperString::SetFromChannels(const unsigned char* buf)
{
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
    for (int i = 0; i < _superStringColours.size(); i++) {
        xlColor cc = xlColor(_superStringColours[i].red * buf[i] / 255, _superStringColours[i].green * buf[i] / 255, _superStringColours[i].blue * buf[i] / 255);
        if (c[0] < cc.red)
            c[0] = cc.red;
        if (c[1] < cc.green)
            c[1] = cc.green;
        if (c[2] < cc.blue)
            c[2] = cc.blue;
    }
}

void NodeClassSuperString::GetForChannels(unsigned char* buf) const
{
    bool r = c[0] > 0 && c[1] == 0 && c[2] == 0;
    bool g = c[0] == 0 && c[1] > 0 && c[2] == 0;
    bool b = c[0] == 0 && c[1] == 0 && c[2] > 0;
    bool w = c[0] > 0 && c[0] == c[1] && c[0] == c[2];
    bool y = c[0] > 0 && c[0] == c[1] && c[2] == 0;
    bool cy = c[1] > 0 && c[1] == c[2] && c[0] == 0;
    bool m = c[0] > 0 && c[0] == c[2] && c[1] == 0;
    uint8_t common = std::min(c[0], std::min(c[1], c[2]));

    bool primary = r || g || b || y || w || cy || m;
    int singleColour = -1;
    if (primary) {
        for (int i = 0; singleColour == -1 && i < _superStringColours.size(); i++) {
            xlColor cc = _superStringColours[i];
            if ((r && cc.red > 0 && cc.green == 0 && cc.blue == 0) ||
                (g && cc.red == 0 && cc.green > 0 && cc.blue == 0) ||
                (b && cc.red == 0 && cc.green == 0 && cc.blue > 0) ||
                (w && cc.red > 0 && cc.red == cc.green && cc.red == cc.blue) ||
                (y && cc.red > 0 && cc.red == cc.green && cc.blue == 0) ||
                (cy && cc.green > 0 && cc.green == cc.blue && cc.red == 0) ||
                (m && cc.red > 0 && cc.red == cc.blue && cc.green == 0)) {
                singleColour = i;
            }
        }
    }

    switch (rgbwHandling) {
    case RGB_HANDLING_RGB:
        // only update rgb
        for (int i = 0; i < _superStringColours.size(); i++) {
            // if this is a white super string
            if (_superStringColours[i].red != _superStringColours[i].green || _superStringColours[i].red != _superStringColours[i].blue) {
                    // this needs work
                    xlColor cc = _superStringColours[i];
                    float r = cc.red == 0 ? 1 : (float)c[0] / cc.red;
                    float g = cc.green == 0 ? 1 : (float)c[1] / cc.green;
                    float b = cc.blue == 0 ? 1 : (float)c[2] / cc.blue;
                    float in = std::min(r, std::min(g, std::min(1.0f, b)));
                    buf[i] = in * 255;
            }
        }
        break;

    case RGB_HANDLING_WHITE:
        // only populate the white channel

        for (int i = 0; i < _superStringColours.size(); i++) {
            if (_superStringColours[i].red == _superStringColours[i].green && _superStringColours[i].red == _superStringColours[i].blue) {
                buf[i] = common;
            }
        }
        break;

    case RGB_HANDLING_ALL:
        // this is how it used to work
        if (singleColour == -1) {
            for (int i = 0; i < _superStringColours.size(); i++) {
                // this needs work
                xlColor cc = _superStringColours[i];
                float r = cc.red == 0 ? 1 : (float)c[0] / cc.red;
                float g = cc.green == 0 ? 1 : (float)c[1] / cc.green;
                float b = cc.blue == 0 ? 1 : (float)c[2] / cc.blue;
                float in = std::min(r, std::min(g, std::min(1.0f, b)));
                buf[i] = in * 255;
            }
        } else {
            for (int i = 0; i < _superStringColours.size(); i++) {
                if (i == singleColour) {
                    buf[i] = std::max(c[0], std::max(c[1], c[2]));
                } else {
                    buf[i] = 0;
                }
            }
        }
        break;
    case RGB_HANDLING_ADVANCED: 
        // set white the common amount

        for (int i = 0; i < _superStringColours.size(); i++) {
            // if this is a white super string
            if (_superStringColours[i].red != _superStringColours[i].green || _superStringColours[i].red != _superStringColours[i].blue) {
                if (singleColour == -1) {
                    // this needs work
                    xlColor cc = _superStringColours[i];
                    float r = cc.red == 0 ? 1 : (float)(c[0] - common) / cc.red;
                    float g = cc.green == 0 ? 1 : (float)(c[1] - common) / cc.green;
                    float b = cc.blue == 0 ? 1 : (float)(c[2] - common) / cc.blue;
                    float in = std::min(r, std::min(g, std::min(1.0f, b)));
                    buf[i] = in * 255;
                } else {
                    if (i == singleColour) {
                        buf[i] = std::max(c[0], std::max(c[1], c[2]));
                    } else {
                        buf[i] = 0;
                    }
                }
            }
            else
            {
                buf[i] = common;
            }
        }

        break;

    default: // RGB_HANDLING_NORMAL

        // when r==g==b only light up white
        if (w)
        {
            for (int i = 0; i < _superStringColours.size(); i++) {
                // if this is a white super string
                if (_superStringColours[i].red == _superStringColours[i].green && _superStringColours[i].red == _superStringColours[i].blue)
                {
                    xlColor cc = _superStringColours[i];
                    float r = cc.red == 0 ? 1 : (float)c[0] / cc.red;
                    float g = cc.green == 0 ? 1 : (float)c[1] / cc.green;
                    float b = cc.blue == 0 ? 1 : (float)c[2] / cc.blue;
                    float in = std::min(r, std::min(g, std::min(1.0f, b)));
                    buf[i] = in * 255;
                }
            }
        } else {
            for (int i = 0; i < _superStringColours.size(); i++) {
                // if this is a white super string
                if (_superStringColours[i].red != _superStringColours[i].green || _superStringColours[i].red != _superStringColours[i].blue) {
                    if (singleColour == -1) {
                        // this needs work
                        xlColor cc = _superStringColours[i];
                        float r = cc.red == 0 ? 1 : (float)c[0] / cc.red;
                        float g = cc.green == 0 ? 1 : (float)c[1] / cc.green;
                        float b = cc.blue == 0 ? 1 : (float)c[2] / cc.blue;
                        float in = std::min(r, std::min(g, std::min(1.0f, b)));
                        buf[i] = in * 255;
                    } else {
                        if (i == singleColour) {
                            buf[i] = std::max(c[0], std::max(c[1], c[2]));
                        } else {
                            buf[i] = 0;
                        }
                    }
                }
            }
        }
        break;
    }
}
