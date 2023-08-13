/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Pixels.h"

#include <algorithm>

// ***********************************************************************************************
// *                                                                                             *
// * The following table may not be accurate but it is my best understanding of protocol         *
// * compatibility. This does not mean that controllers wont show them as separate or even that  *
// * some controllers having timing that means that things which could be equivalent with the    *
// * righting timing choice are not in reality. This makes things messy but I do think having    *
// * this documented somewhere is essential.                                                     *
// *                                                                                             *
// * I have arbitrarily chosen one of the pixel types as the group name. Typically because this  *
// * is the most common or just because that is the one I randomly chose ... live with it        *
// *                                                                                             *
// * If you make changes please add a note explaining why as I am sure there is going to be      *
// * disagreement.                                                                               *
// *                                                                                             *
// ***********************************************************************************************
//
// ----------------------------------------------------------------------------------------------------------------------------------------
// | Group Name     | Characteristics        | Equivalent                                        | Controller Support                     |
// |======================================================================================================================================|
// | 9PDOT-8Bit     | 8 Bit                  |                                                   | Entec                                  |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | 9PDOT-16Bit    | 16 Bit                 |                                                   | Entec                                  |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | APA102         | RGB, 8bit, 4 wire      | APA101, APA102, APA102C, HD107S, SK9822           |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | APA109         | RGBW, 8bit, 3 wire     | APA109, SK6812RGBW, SK6818, SM16704, UCS2904      |                                        |
// |                |                        | WS2814                                            |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | DM412          | ?, ? bit, ? wire       |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | DMX512P        | RGB, 8 bit, 3 wire     | DMX512, UCS512, WS2822                            |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | DMX512P-4      | RGBW, 8 bit, 3 wire    | UCS512C4                                          |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | GECE           | RGB, 5 bit, 3 wire     |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | GW6205         | ?, 12 bit, ? wire      |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | LPD6803        | RGB, 5bit, 4 wire      | D705, LPD1101, LPD6803, UCS6909, UCS6912S         |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | LPD8806        | RGB, 7 bit, 4 wire     | LPD8803, LPD8806, LPD8809, UCS8812                |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | MBI6020        | ?, 10 bit, 4 wire      |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | MY9221         | RGB, 16 bit, 4 wire    |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | MY9231         |                        |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | MY9291         | RGBW, 16 bit, 4 wire   |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | P9813          | RGB, 6 bit, 4 wire     |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | RM2021         |                        |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | SJ1221         | RGB, 12 bit, 3 wire    | RGB+2 (minleon)                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | SM16716        | RGB, 8 bit, 4 wire     | SM16716, SM16726                                  |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | SPXL-8Bit      | 8 Bit                  |                                                   | Entec                                  |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | SPXL-16Bit     | 16 Bit                 |                                                   | Entec                                  |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | TLC5973        | RGB, 12 bit, 3 wire    |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | TLS3001        | RGB, 12 bit, 3 wire    | CY3005, TLS3001, TLS3002, QED3001                 |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | TLS3008        | RGB, 8 bit, 3 wire     |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | TM1814         | RGBW, 8 bit, 3 wire    |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | TM1829         | RGB, 5 bit, 3 wire     |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | UCS8903        | RGB, 16 bit, 3 wire    |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | UCS8904        | RGBW, 16 bit, 3 wire   |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | UCS9812        |                        |                                                   |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | WS2801         | RGB, 8 bit, 4 wire     | WS2801, WS2803                                    |                                        |
// |--------------------------------------------------------------------------------------------------------------------------------------|
// | WS2811         | RGB, 8 bit, 3 wire     | APA104, APA106, CS8812, GS8202, GS8206,           |                                        |
// |                |                        | GS8208, INK1002, INK1003, LPD1882,                |                                        |
// |                |                        | LX1203, P9883, SK6812, SK6813, SK6822, SM16703,   |                                        |
// |                |                        | SM16711, SM16712, TM1803, TM1804, TM1809,         |                                        |
// |                |                        | TM1812, TM1914, UCS1903, UCS1904, UCS1909,        |                                        |
// |                |                        | UCS1912, UCS2903, UCS2909, UCS2912, UCS9812,      |                                        |
// |                |                        | WS2811, WS2812, WS2813, WS2815, WS2818            |                                        |
// ----------------------------------------------------------------------------------------------------------------------------------------
// 
// I am undecided whether making this generally available to the user helps or hinders.
// The problem is if certain pixels are not equivalent on their controller and we said they were  then we have issues
//

// This table includes some artificial types such as tm18xx which don't exist but are themselves a group within a group of compatible pixels
// These are required because some controllers list them as a type

// the "preferred" pixel type should be at the start of each list
const static std::vector<std::vector<std::string>> __equivalentPixels =
{
    { "" }, // No protocol
    { "ws2811", // preferred & default
      "apa104", "apa106", "cs8812", "gs8202", "gs8206", "gs8208", "ink1002", "ink1003", "lpd1882",
      "lx1203", "p9883", "sk6812", "sk6813", "sk6822", "sm16703", "sm16711", "sm16712", "tm1803", "tm1804", "tm1809",
      "tm1812", "tm1914", "ucs1903", "ucs1904", "ucs1909", "ucs1912", "ucs2903", "ucs2909", "ucs2912", "ucs9812",
      "ws2812", "ws2812b", "ws2813", "ws2815", "ws2818",
      // artificial
      "tm18xx", "ws281x", " ws2811v1.4", " ws2811 slow", "gs820x"
    },
    { "9pdot" },
    { "9pdot (16)" },
    { "apa101", "apa102", "apa102c", "hd107s", "sk9822" },
    { "apa109", "sk6812rgbw", "sk6818", "sm16704", "ucs2904", "ws2814" },
    { "dm412" },
    { "dmx512p", "dmx512", "ucs512", "ws2822" },
    { "dmx512p-4", "ucs512c4" },
    { "gece" },
    { "gw6205" },
    { "d705", "lpd1101", "lpd6803", "ucs6909", "ucs6912s" },
    { "lpd8803", "lpd8806", "lpd8809", "ucs8812" },
    { "mib6020" },
    { "my9221" },
    { "my9291" },
    { "p9813" },
    { "sj1221", "rgb+2" },
    { "sm16716", "sm16726" },
    { "sm16714" }, // I have not researched these
    { "spxl" },
    { "spxl (16)" },
    { "cy3005", "tls3001", "tls3002", "qed3001" },
    { "tlc5973" },
    { "tls3008" },
    { "tm1814", "tm1814a" },
    { "tm1829" },
    { "ucs8903", "ucs8903 (16)" },
    { "ucs8904", "ucs8904 (16)" },
    { "ucs9812" }, // I have not researched these
    { "my9231" },  // I have not researched these ... they may fit into an existing category
    { "ws2801", "ws2803" },
    { "rgb+" } // not sure if these have a functional equivalent ... if they do these should be moved to artificial
};

const static std::vector<std::vector<std::string>> __equivalentSerial = {
    { "dmx", "dmx512" },
    { "dmx-open", "opendmx" },
    { "dmx-pro" },
    { "lor" },
    { "renard" },
    { "genericserial" },
    { "pixelnet" },
    { "pixelnet-lynx" },
    { "pixelnet-open" }
};

const static std::vector<std::string> __artificalTypes =
{
    "tm18xx", "ws281x", " ws2811v1.4", " ws2811 slow", "ucs8903 (16)", "ucs8904 (16)", "gs820x", "rgb+2", "dmx512p", "dmx512p-4"
};

bool IsArtificialPixelType(const std::string& p)
{
    for (const auto& it : __artificalTypes) {
        if (p == it) return true;
    }
    return false;
}

std::string ChooseBestControllerPixel(const std::vector<std::string>& controllerPixels, const std::string& selectedPixel)
{
    // first look for an exact match in controller pixels ... if that is there return that
    if (std::find(begin(controllerPixels), end(controllerPixels), selectedPixel) != end(controllerPixels)) {
        return selectedPixel;
    }

    // if not look for the pool this type is in and look for an exact match in that group
    for (const auto& it : __equivalentPixels) {
        if (std::find(begin(it), end(it), selectedPixel) != end(it)) {
            // we found our pool
            for (const auto& it2 : it) {
                if (std::find(begin(controllerPixels), end(controllerPixels), it2) != end(controllerPixels)) {
                    return it2;
                }
            }
        }
    }
    std::string p1 = selectedPixel;
    std::transform(p1.begin(), p1.end(), p1.begin(), ::tolower);
    if (p1 != selectedPixel) {
        return ChooseBestControllerSerial(controllerPixels, p1);
    }

    // no good match
    return "";
}

std::vector<std::string> GetAllPixelTypes(const std::vector<std::string>& controllerPixels, bool includeSerial, bool includeArtificial, bool includeMatrices)
{
    auto superset = GetAllPixelTypes(includeSerial, includeArtificial, includeMatrices);

    std::vector<std::string> res;

    for (const auto& it : controllerPixels) {
        if (std::find(begin(superset), end(superset), it) != end(superset)) {
            res.push_back(it);
        } else {
            std::string p1 = it;
            std::transform(p1.begin(), p1.end(), p1.begin(), ::tolower);
            if (std::find(begin(superset), end(superset), p1) != end(superset)) {
                res.push_back(it);
            }
        }
    }

    return res;
}

std::vector<std::string> GetAllPixelTypes(bool includeSerial, bool includeArtificial, bool includeMatrices)
{
    std::vector<std::string> res;

    for (const auto& it : __equivalentPixels) {
        for (const auto& it2 : it) {
            if (includeArtificial || !IsArtificialPixelType(it2)) {
                res.push_back(it2);
            }
        }
    }

    if (includeSerial) {
        for (const auto& it : __equivalentSerial) {
            for (const auto& it2 : it) {
                res.push_back(it2);
            }
        }
    }
    if (includeMatrices) {
        res.push_back("Virtual Matrix");
        res.push_back("LED Panel Matrix");
    }

    return res;
}

std::vector<std::string> GetAllSerialTypes()
{
    std::vector<std::string> res;

    for (const auto& it : __equivalentSerial) {
        for (const auto& it2 : it) {
            res.push_back(it2);
        }
    }
    
    return res;
}

std::vector<std::string> GetAllSerialTypes(const std::vector<std::string>& controllerSerial)
{
    auto superset = GetAllSerialTypes();

    std::vector<std::string> res;

    for (const auto& it : controllerSerial) {
        if (std::find(begin(superset), end(superset), it) != end(superset)) {
            res.push_back(it);
        } else {
            std::string p1 = it;
            std::transform(p1.begin(), p1.end(), p1.begin(), ::tolower);
            if (std::find(begin(superset), end(superset), p1) != end(superset)) {
                res.push_back(it);
            }
        }
    }

    return res;
}

std::string ChooseBestControllerSerial(const std::vector<std::string>& controllerSerial, const std::string& selectedSerial)
{
    // first look for an exact match in controller pixels ... if that is there return that
    if (std::find(begin(controllerSerial), end(controllerSerial), selectedSerial) != end(controllerSerial)) {
        return selectedSerial;
    }

    // if not look for the pool this type is in and look for an exact match in that group
    for (const auto& it : __equivalentSerial) {
        if (std::find(begin(it), end(it), selectedSerial) != end(it)) {
            // we found our pool
            for (const auto& it2 : it) {
                if (std::find(begin(controllerSerial), end(controllerSerial), it2) != end(controllerSerial)) {
                    return it2;
                }
            }
        }
    }
    std::string p1 = selectedSerial;
    std::transform(p1.begin(), p1.end(), p1.begin(), ::tolower);
    if (p1 != selectedSerial) {
        return ChooseBestControllerSerial(controllerSerial, p1);
    }
    // no good match
    return "";
}

bool IsPixelProtocol(const std::string& p1)
{
    auto pt = GetAllPixelTypes(false, true);
    return std::find_if(begin(pt), end(pt), [&p1](auto const& pix) {
            std::string p = p1;
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            return pix == p;
        }) != end(pt);
}

bool IsSerialProtocol(const std::string& p1)
{
    auto st = GetAllSerialTypes();
    return std::find_if(begin(st), end(st), [&p1](auto const& ser) {
               std::string p = p1;
               std::transform(p.begin(), p.end(), p.begin(), ::tolower);
               return ser == p;
           }) != end(st);
}
bool IsMatrixProtocol(const std::string& p1)
{
    return (p1 == "LED Panel Matrix") || (p1 == "Virtual Matrix");
}
bool IsLEDPanelMatrixProtocol(const std::string& p1)
{
    return p1 == "LED Panel Matrix";
}
bool IsVirtualMatrixProtocol(const std::string& p1)
{
    return p1 == "Virtual Matrix";
}

int GetChannelsPerPixel(const std::string& p)
{
    const static std::vector<std::string> fourChanPixels = { "apa109", "sk6812rgbw", "sk6818", "sm16704", "ucs2904", "ws2814", "dmx512p-4", "dmx512", "my9291", "tm1814", "tm1814a", "ucs8904", "ucs8904 (16)" };

    if (std::find(begin(fourChanPixels), end(fourChanPixels), p) != end(fourChanPixels)) {
        return 4;
    }
    return 3;
}
