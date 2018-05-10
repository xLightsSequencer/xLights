

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




const std::string NodeBaseClass::EMPTY_STR("");


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

void NodeClassRGBW::SetFromChannels(const unsigned char *buf) {
    switch (rgbwHandling) {
        case RGB_HANDLING_RGB:
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    c[x] = buf[offsets[x] + wOffset];
                }
            }
            break;
        case RGB_HANDLING_WHITE:
            c[0] = c[1] = c[2] = buf[wIndex];
            break;
        default: //RGB_HANDLING_NORMAL
            if (buf[wIndex] != 0) {
                c[0] = c[1] = c[2] = buf[wIndex];
            } else {
                for (int x = 0; x < 3; x++) {
                    if (offsets[x] != 255) {
                        c[x] = buf[offsets[x] + wOffset];
                    }
                }
            }
            break;
    }
}
void NodeClassRGBW::GetForChannels(unsigned char *buf) const {
    switch (rgbwHandling) {
        case RGB_HANDLING_RGB:
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x] + wOffset] = c[x];
                }
            }
            break;
        case RGB_HANDLING_WHITE:
            if (c[0] == c[1] && c[1] == c[2]) {
                buf[wIndex] = c[0];
            }
            break;
        default: //RGB_HANDLING_NORMAL
            if (c[0] == c[1] && c[1] == c[2]) {
                buf[0 + wOffset] = buf[1 + wOffset] = buf[2 + wOffset] = 0;
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
