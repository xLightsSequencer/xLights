

#include "Node.h"


const std::string NodeBaseClass::RED("R");
const std::string NodeBaseClass::GREEN("G");
const std::string NodeBaseClass::BLUE("B");
const std::string NodeBaseClass::WHITE("W");
const std::string NodeBaseClass::RGBW("RGBW");
const std::string NodeBaseClass::WRGB("WRGB");

const std::string NodeBaseClass::RGB("RGB");
const std::string NodeBaseClass::RBG("RBG");
const std::string NodeBaseClass::GBR("GBR");
const std::string NodeBaseClass::GRB("GRB");
const std::string NodeBaseClass::BRG("BRG");
const std::string NodeBaseClass::BGR("BGR");

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
