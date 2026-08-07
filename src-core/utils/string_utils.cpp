/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "string_utils.h"

#include <random>

const std::string xlEMPTY_STRING{ "" };

std::string GenerateGuid()
{
    static constexpr char HEX[] = "0123456789abcdef";

    std::random_device rd;
    std::uniform_int_distribution<int> nibble(0, 15);

    std::string guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (char& c : guid) {
        if (c == 'x') {
            c = HEX[nibble(rd)];
        } else if (c == 'y') {
            // Variant field: 8, 9, a or b.
            c = HEX[(nibble(rd) & 0x3) | 0x8];
        }
    }
    return guid;
}
