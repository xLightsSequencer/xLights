/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Falcon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "ControllerUploadData.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

#pragma region FalconString Handling
class FalconString
{
public:
    int virtualStringIndex = -1;
    int protocol = -1;
    int universe = -1;
    int startChannel = -1;
    int pixels = -1;
    std::string description;
    int index = -1;
    int port = -1;
    float gamma = 1.0;
    int groupCount = 1;
    int smartRemote = 0;
    int nullPixels = 0;
    std::string colourOrder;
    std::string direction;
    int brightness = 100;
    void Dump() const {

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("    Index %02d Port %02d SmartRemote %d VirtualString %d Prot %d Desc '%s' Uni %d StartChan %d Pixels %d Group %d Direction %s ColorOrder %s Nulls %d Brightness %d Gamma %.1f",
            index,
            port + 1,
            smartRemote,
            virtualStringIndex,
            protocol,
            (const char*)description.c_str(),
            universe,
            startChannel,
            pixels,
            groupCount,
            (const char*)direction.c_str(),
            (const char*)colourOrder.c_str(),
            nullPixels,
            brightness,
            gamma);
    }
    const bool operator>(const FalconString& other) const
    {
        if (port / 4 == other.port / 4) {
            if (smartRemote > other.smartRemote) return false;
            if (smartRemote < other.smartRemote) return true;
        }

        if (port > other.port) return false;
        if (port < other.port) return true;

        if (virtualStringIndex > other.virtualStringIndex) return false;
        if (virtualStringIndex < other.virtualStringIndex) return true;

        return true;
    }
};

#define MINIMUMPIXELS 0
void Falcon::InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int minuniverse) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Filling in missing strings.");

    std::vector<FalconString*> newStringsData;

    int index = 0;
    for (int i = 0; i < max; i++) {
        bool added = false;
        for (const auto& sd : stringsData) {
            if (sd->port == i) {
                sd->index = index++;
                newStringsData.push_back(sd);
                added = true;
            }
        }
        if (!added) {
            FalconString* string = new FalconString();
            string->startChannel = 1;
            string->virtualStringIndex = 0;
            string->pixels = MINIMUMPIXELS;
            string->protocol = 0;
            string->universe = minuniverse;
            string->description = "";
            string->port = i;
            string->index = index++;
            string->brightness = 100;
            string->nullPixels = 0;
            string->gamma = 1.0;
            string->colourOrder = "RGB";
            string->direction = "Forward";
            string->groupCount = 1;
            string->smartRemote = 0x00;
            newStringsData.push_back(string);
            logger_base.debug("    Added default string to port %d.", i + 1);
        }
    }
    stringsData = newStringsData;
}

std::string Falcon::BuildStringPort(FalconString* string) const {

    return wxString::Format("&p%i=%i&x%i=%i&t%i=%i&u%i=%i&s%i=%i&c%i=%i&y%i=%s&b%i=%i&n%i=%i&G%i=%i&o%i=%i&d%i=%i&g%i=%i&w%i=%d",
        string->index, string->port,
        string->index, string->virtualStringIndex,
        string->index, string->protocol,
        string->index, string->universe,
        string->index, string->startChannel,
        string->index, string->pixels,
        string->index, string->description,
        string->index, EncodeBrightness(string->brightness),
        string->index, string->nullPixels,
        string->index, EncodeGamma(string->gamma),
        string->index, EncodeColourOrder(string->colourOrder),
        string->index, EncodeDirection(string->direction),
        string->index, string->groupCount,
        string->index, string->smartRemote
    ).ToStdString();
}

FalconString* Falcon::FindPort(const std::vector<FalconString*>& stringData, int port) const {

    for (const auto& it : stringData) {
        if (it->port == port)
        {
            return it;
        }
    }
    wxASSERT(false);
    return nullptr;
}

int Falcon::GetPixelCount(const std::vector<FalconString*>& stringData, int port) const {

    int count = 0;
    for (const auto& sd : stringData) {
        if (sd->port == port) {
            // have to include any null pixels in the count
            count += sd->pixels + sd->nullPixels;
        }
    }
    return count;
}

int Falcon::GetMaxPixelPort(const std::vector<FalconString*>& stringData) const {

    int max = 0;
    for (const auto& sd : stringData) {
        if (sd->port + 1 > max) {
            max = sd->port + 1;
        }
    }
    return max;
}

void Falcon::RemoveNonSmartRemote(std::vector<FalconString*>& stringData, int port)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto it = begin(stringData);
    while (it != end(stringData)) {
        if ((*it)->port == port && (*it)->smartRemote == 0) {
            logger_base.debug("Removing non smart remote port %d", port + 1);
            it = stringData.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Falcon::EnsureSmartStringExists(std::vector<FalconString*>& stringData, int port, int smartRemote, int minuniverse) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool found = false;
    for (const auto& it : stringData) {
        if (it->port == port && it->smartRemote == smartRemote) {
            found = true;
            break;
        }
    }

    if (!found) {
        logger_base.debug("Adding in dummy string for a smart remote Port: %d Smart Remote %d", port + 1, smartRemote + 1);
        FalconString* string = new FalconString();
        string->startChannel = 1;
        string->virtualStringIndex = 0;
        string->pixels = 0;
        string->protocol = 0;
        string->universe = minuniverse;
        string->description = "";
        string->port = port;
        string->index = stringData.size();
        string->brightness = 100;
        string->nullPixels = 0;
        string->gamma = 1.0;
        string->colourOrder = "RGB";
        string->direction = "Forward";
        string->groupCount = 1;
        string->smartRemote = smartRemote;
        stringData.push_back(string);
    }
}

void Falcon::DumpStringData(std::vector<FalconString*> stringData) const {

    for (const auto& sd : stringData) {
        sd->Dump();
    }
}
#pragma endregion

#pragma region strings.xml Handling
int Falcon::CountStrings(const wxXmlDocument& stringsDoc) const {

    if (stringsDoc.GetRoot() == nullptr) return 0;

    int count = 0;
    int last = -1;
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        int port = wxAtoi(e->GetAttribute("p"));
        if (port > last) {
            count++;
        }
        last = port;
    }
    return count;
}

void Falcon::ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (stringsDoc.GetRoot() == nullptr) return;

    int count = 0;
    for (auto n = stringsDoc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
        count++;
    }

    logger_base.debug("Strings.xml had %d entries.", count);
    if (count == 0) return;

    int oldCount = stringData.size();
    stringData.resize(count);
    for (int i = oldCount; i < count; ++i) {
        stringData[i] = nullptr;
    }

    int i = 0;
    int lastString = -1;
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        int port = wxAtoi(e->GetAttribute("p"));

        //<vs y="" p="7" u="2000" us="0" s="0" c="50" g="1" t="0" d="0" o="0" n="0" z="0" b="13" bl="0" ga="0"/>
        // y = description
        // u = universe
        // us = universe start channel - 1
        // s = absolute channel I think
        // c = pixel count
        // g = grouping
        // t = protocol
        // d = direction (index 0 - forward, 1 - reversed)
        // o = pixel order (index 0 - RGB, 1 - RBG, 2 - GRB, 3 - GBR, 4 - BRG, 5 - BGR)
        // n = null pixels
        // z = zig zag count - IGNORED
        // b = brightness (index 0 - 100, 1 - 95, 2 - 90, 3 - 85, 4 - 80, 5 - 75, 6 - 70, 7 - 65, 8 - 60, 9 - 50, 10 - 40, 11 - 30, 12 - 20, 13 - 10)
        // bl = blank (1 if checked) - IGNORED
        // ga = gamma (index 0 - none, 1 - 2.0, 2 - 2.3, 3 - 2.5, 4 - 2.8, 5 - 3.0
        FalconString* string = new FalconString();
        string->startChannel = wxAtoi(e->GetAttribute("us")) + 1;
        if (!_usingAbsolute) {
            if (string->startChannel < 1 || string->startChannel > 512) string->startChannel = 1;
        }
        string->pixels = wxAtoi(e->GetAttribute("c"));
        if (string->pixels < 0 || string->pixels > GetMaxPixels()) string->pixels = 0;
        string->protocol = wxAtoi(e->GetAttribute("t", "0"));
        string->universe = wxAtoi(e->GetAttribute("u"));
        if (string->universe <= 1 || string->universe > 64000) string->universe = 1;
        string->description = e->GetAttribute("y", "").ToStdString();
        string->port = wxAtoi(e->GetAttribute("p"));
        string->brightness = DecodeBrightness(wxAtoi(e->GetAttribute("b", "0")));
        string->nullPixels = wxAtoi(e->GetAttribute("n", "0"));
        string->gamma = DecodeGamma(wxAtoi(e->GetAttribute("ga", "0")));
        string->colourOrder = DecodeColourOrder(wxAtoi(e->GetAttribute("o", "0")));
        string->direction = DecodeDirection(wxAtoi(e->GetAttribute("d", "0")));
        string->groupCount = wxAtoi(e->GetAttribute("g", "1"));
        int sr = wxAtoi(e->GetAttribute("sr", "-1"));
        if (sr == -1) {
            string->smartRemote = wxAtoi(e->GetAttribute("x", "0"));
        }
        else {
            string->smartRemote = sr;
        }
        string->virtualStringIndex = wxAtoi(e->GetAttribute("si", "0"));
        string->index = i;
        stringData[i] = string;

        lastString = port;
        i++;
    }
}

int Falcon::MaxPixels(const wxXmlDocument& stringsDoc, int board) const {

    if (stringsDoc.GetRoot() == nullptr) return 0;

    switch (board) {
    case 0:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k0"));
    case 1:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k1"));
    case 2:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k2"));
    default:
        return 0;
    }
}
#pragma endregion

#pragma region Port Handling
void Falcon::ResetStringOutputs() {

    PutURL("/StringPorts.htm", "S=4&p0=0&p1=1&p2=2&p3=3");
}

void Falcon::UploadStringPort(const std::string& request, bool final) {

    std::string r = request;
    if (final) {
        r = "r=1&" + r;
    }
    else {
        r = "r=0&" + r;
    }

    PutURL("/StringPorts.htm", r);
}

void Falcon::UploadStringPorts(std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, int minuniverse) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int maxPort = 0;
    for (const auto& sd : stringData) {
        maxPort = std::max(maxPort, sd->port);
    }

    // fill in missing smart ports
    int quads = (maxPort + 3) / 4;
    for (int i = 0; i < quads; i++) {
        int maxRemote = 0;
        for (int j = 0; j < 4; j++) {
            for (const auto& it : stringData) {
                if (it->port == i * 4 + j) {
                    if (it->smartRemote != 0) {
                        maxRemote = std::max(maxRemote, it->smartRemote);
                    }
                }
            }
        }

        if (maxRemote > 0) {
            for (int k = 0; k < 4; k++) {
                RemoveNonSmartRemote(stringData, i * 4 + k);
                for (int j = 0; j < maxRemote; j++) {
                    EnsureSmartStringExists(stringData, i * 4 + k, j + 1, minuniverse);
                }
            }
        }
    }

    // Sort strings in the right order
    std::sort(begin(stringData), end(stringData), [](FalconString* a, FalconString* b) {
        return *a > *b;
        });

    // reindex the string data
    int i = 0;
    for (auto& it : stringData)         {
        it->index = i++;
    }

    logger_base.debug("Final string data for upload.");
    DumpStringData(stringData);

    int S = stringData.size();
    int m = 0;

    if (maxPort + 1 > GetDaughter2Threshold()) {
        m = 2;
    }
    else if (maxPort + 1 > GetDaughter1Threshold()) {
        m = 1;
    }

    std::string base = wxString::Format("m=%i&S=%i", m, S).ToStdString();

    if (SupportsVariableExpansions()) {
        base += wxString::Format("&k0=%i&k1=%i&k2=%i", maxMain, maxDaughter1, maxDaughter2).ToStdString();
    }

#define PACKETSIZE 40
    int packets = (stringData.size() + PACKETSIZE - 1) / PACKETSIZE;
    for (int p = 0; p < packets; p++) {
        std::string message = base + "&q=" + wxString::Format("%d", p).ToStdString();

        for (int i = p * PACKETSIZE; i < stringData.size() && i < (p + 1) * PACKETSIZE; ++i) {
            message += BuildStringPort(stringData[i]);
        }

        UploadStringPort(message, p == packets - 1);
    }
}

std::string Falcon::GetSerialOutputURI(ControllerCaps* caps, int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent) {

    if (output > caps->GetMaxSerialPort()) {
        DisplayError("Falcon " + GetModel() + " only supports " + wxString::Format("%d", caps->GetMaxSerialPort()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", parent);
        return "";
    }

    if (_usingAbsolute) {
        return wxString::Format("t%d=%d&s%d=%d",
            output - 1, protocol,
            output - 1, portstart).ToStdString();
    }
    else {
        int32_t sc;
        auto o = outputManager->GetOutput(portstart, sc);
        if (o != nullptr) {
            return wxString::Format("t%d=%d&u%d=%d&s%d=%d",
                output - 1, protocol,
                output - 1, o->GetUniverse(),
                output - 1, (int)sc).ToStdString();
        }
        else {
            DisplayError("Error uploading serial output to falcon. " + wxString::Format("%i", portstart) + " does not map to a universe.");
        }
    }

    return "";
}
#pragma endregion

#pragma region Encode and Decode
int Falcon::DecodeSerialOutputProtocol(std::string protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return 0;
    if (p == "pixelnet") return 1;
    if (p == "renard") return 2;
    return -1;
}

int Falcon::DecodeStringPortProtocol(std::string protocol) const {

    wxString p(protocol);
    p = p.Lower();
    if (p == "ws2811") return 0;
    if (p == "tm18xx") return 1;
    if (p == "lx1203") return 2;
    if (p == "ws2801") return 3;
    if (p == "tls3001") return 4;
    if (p == "lpd6803") return 5;
    if (p == "gece") return 6;
    if (p == "lpd8806") return 7;
    if (p == "apa102") return 8;

    return -1;
}

int Falcon::DecodeBrightness(int brightnessCode) const {

    switch (brightnessCode) {
    case 0: return 100;
    case 1: return 95;
    case 2: return 90;
    case 3: return 85;
    case 4: return 80;
    case 5: return 75;
    case 6: return 70;
    case 7: return 65;
    case 8: return 60;
    case 9: return 50;
    case 10: return 40;
    case 11: return 30;
    case 12: return 20;
    case 13: return 10;
    default: break;
    }
    return 100;
}

int Falcon::EncodeBrightness(int brightness) const {

    if (brightness < 15) return 13;
    if (brightness < 25) return 12;
    if (brightness < 35) return 11;
    if (brightness < 45) return 10;
    if (brightness < 55) return 9;
    if (brightness < 62.5) return 8;
    if (brightness < 67.5) return 7;
    if (brightness < 72.5) return 6;
    if (brightness < 77.5) return 5;
    if (brightness < 82.5) return 4;
    if (brightness < 87.5) return 3;
    if (brightness < 92.5) return 2;
    if (brightness < 97.5) return 1;
    return 0;
}

float Falcon::DecodeGamma(int gammaCode) const {

    switch (gammaCode) {
    case 0: return 1.0f;
    case 1: return 2.0f;
    case 2: return 2.3f;
    case 3: return 2.5f;
    case 4: return 2.8f;
    case 5: return 3.0f;
    default: break;
    }
    return 1.0f;
}

int Falcon::EncodeGamma(float gamma) const {

    if (gamma < 1.5) return 0;
    if (gamma < 2.15) return 1;
    if (gamma < 2.4) return 2;
    if (gamma < 2.65) return 3;
    if (gamma < 2.9) return 4;
    return 5;
}

std::string Falcon::DecodeColourOrder(int colourOrderCode) const {

    switch (colourOrderCode) {
    case 0: return "RGB";
    case 1: return "RBG";
    case 2: return "GRB";
    case 3: return "GBR";
    case 4: return "BRG";
    case 5: return "BGR";
    default: break;
    }
    return "RGB";
}

int Falcon::EncodeColourOrder(const std::string& colourOrder) const {

    if (colourOrder == "RGB") return 0;
    if (colourOrder == "RBG") return 1;
    if (colourOrder == "GRB") return 2;
    if (colourOrder == "GBR") return 3;
    if (colourOrder == "BRG") return 4;
    if (colourOrder == "BGR") return 5;
    return 0;
}

std::string Falcon::DecodeDirection(int directionCode) const {

    switch (directionCode) {
    case 0: return "Forward";
    case 1: return "Reverse";
    default: break;
    }
    return "Forward";
}

int Falcon::EncodeDirection(const std::string& direction) const {

    if (direction == "Forward") return 0;
    if (direction == "Reverse") return 1;
    return 0;
}
#pragma endregion

#pragma region Private Functions
std::string Falcon::SafeDescription(const std::string description) const {

    wxString desc(description);
    int replaced = desc.Replace("  ", " ");
    while (replaced != 0) {
        replaced = desc.Replace("  ", " ");
    }
    return desc.Left(25).ToStdString();
}

bool Falcon::IsEnhancedV2Firmware() const {

    if (_firmwareVersion == "") return false;

    auto fwv = wxSplit(_firmwareVersion, '.');
    int majorfw = 0;
    int minorfw = 0;

    if (fwv.size() > 0) {
        majorfw = wxAtoi(fwv[0]);
        if (fwv.size() > 1) {
            minorfw = wxAtoi(fwv[1]);
        }
    }

    if (majorfw < 2 || (majorfw == 2 && minorfw < 1)) {
        return false;
    }

    return true;
}

int Falcon::GetMaxPixels() const {

    if (IsV2() && !IsEnhancedV2Firmware()) {
        return 680;
    }
    else {
        return 1024;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
Falcon::Falcon(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _firmwareVersion = "";
    _model = "";
    _versionnum = 0;
    _modelnum = 0;
    _usingAbsolute = false;

    logger_base.debug("Connecting to Falcon on %s.", (const char*)_ip.c_str());

    _connected = true;
    int p = 0;
    std::string versionxml = GetURL("/status.xml");
    if (versionxml == "") {
        logger_base.error("    Error retrieving status.xml from falcon controller.");
        _connected = false;
        return;
    }
    logger_base.debug("status.xml:\n%s", (const char*)versionxml.c_str());

    wxStringInputStream stream(versionxml);
    wxXmlDocument xml(stream);
    if (!xml.IsOk() || xml.GetRoot() == nullptr) {
        logger_base.error("     Status XML parses as invalid.");
        _firmwareVersion = "UNKNOWN";
        _usingAbsolute = false;
        _name = "UNKNOWN";
        _model = "UNKNOWN";
        _connected = false;
    }
    else {
        wxXmlNode* node = xml.GetRoot()->GetChildren();
        while (node) {
            if (node->GetName() == "v" || node->GetName() == "fv") {
                _firmwareVersion = node->GetNodeContent();
            }
            else if (node->GetName() == "a") {
                _usingAbsolute = node->GetNodeContent() == "0";
            }
            else if (node->GetName() == "n") {
                _name = node->GetNodeContent().Trim();
            }
            else if (node->GetName() == "p") {
                DecodeModelVersion(wxAtoi(node->GetNodeContent()), _modelnum, _versionnum);
                _model = wxString::Format("F%dv%d", _modelnum, _versionnum).ToStdString();
            }
            node = node->GetNext();
        }
        if (_versionnum == 0 || _modelnum == 0 || _firmwareVersion == "") {
            std::string version = GetURL("/index.htm");
            if (version == "") {
                logger_base.error("    Error retrieving index.htm from falcon controller.");
                _connected = false;
                return;
            }

            if (_firmwareVersion == "") {
                //<title>F4V2            - v1.10</title>
                static wxRegEx firmwareversionregex("(title.*?v)([0-9]+\\.[0-9]+)\\<\\/title\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (firmwareversionregex.Matches(wxString(version))) {
                    _firmwareVersion = firmwareversionregex.GetMatch(wxString(version), 2).ToStdString();
                }
            }
        }

        logger_base.debug("Connected to falcon - p=%d Model: '%s' Firmware Version '%s'. F%d:V%d", p, (const char*)GetModel().c_str(), (const char*)_firmwareVersion.c_str(), _modelnum, _versionnum);
    }

    if (_versionnum == 0 || _modelnum == 0) {
        _connected = false;
        logger_base.error("Error connecting to falcon controller on %s. Unable to determine model/version.", (const char*)_ip.c_str());
    }
}
#pragma endregion

#pragma region Static Functions
void Falcon::DecodeModelVersion(int p, int& model, int& version) {

    switch (p) {
    case 1:
    case 2:
    case 3:
        model = 16;
        version = 2;
        break;
    case 4:
        model = 4;
        version = 2;
        break;
    case 5:
        model = 16;
        version = 3;
        break;
    case 6:
        model = 4;
        version = 3;
        break;
    case 7:
        model = 48;
        version = 3;
        break;
    default:
        model = 16;
        version = 3;
        break;
    }
}
#pragma endregion

#pragma region Getters and Setters
bool Falcon::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    SetInputUniverses(controller, parent);
    SetOutputs(allmodels, outputManager, controller, parent, false);
    return true;
}


bool Falcon::SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) {

    wxString request;
    int output = 0;

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    if (outputs.size() > 96) {
        DisplayError(wxString::Format("Attempt to upload %d universes to falcon controller but only 96 are supported.", outputs.size()).ToStdString());
        return false;
    }

    for (const auto& it : outputs) {
        int t = -1;
        if (it->GetType() == OUTPUT_E131) {
            t = 0;
        }
        else if (it->GetType() == OUTPUT_ARTNET) {
            t = 1;
        }
        request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
            output, it->GetUniverse(),
            output, (int)it->GetChannels(),
            output, (int)it->GetStartChannel(),
            output, t);
        output++;
    }

    request = wxString::Format("z=%d&a=%d", output, (_usingAbsolute ? 0 : 1)) + request;
    std::string response = PutURL("/E131.htm", request.ToStdString());
    return (response != "");
}

bool Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {
    return SetOutputs(allmodels, outputManager, controller, parent, true);
}
bool Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent, bool doProgress) {

    //ResetStringOutputs(); // this shouldnt be used normally

    std::unique_ptr<wxProgressDialog> progress;
    if (doProgress) {
        progress.reset(new wxProgressDialog("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
        progress->Show();
    }

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Falcon Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    if (doProgress) progress->Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, check, false);

    auto caps = ControllerCaps::GetControllerConfig(controller);
    bool success = true;

    success = cud.Check(caps, check);

    logger_base.debug(check);

    cud.Dump();

    bool fullcontrol = false;
    int currentStrings = 0;
    int mainPixels = 680;
    int daughter1Pixels = 0;
    int daughter2Pixels = 0;
    int minuniverse = controller->GetFirstOutput()->GetUniverse();

    if (caps != nullptr) {
        fullcontrol = caps->SupportsFullxLightsControl() && controller->IsFullxLightsControl();
        mainPixels = caps->GetMaxPixelPortChannels() / 3;
    }

    std::vector<FalconString*> stringData;

    if (!fullcontrol) {
        if (doProgress) progress->Update(10, "Retrieving string configuration from Falcon.");
        logger_base.info("Retrieving string configuration from Falcon.");

        // get the current config before I start
        std::string strings = GetURL("/strings.xml");
        if (strings == "") {
            DisplayError("Error occured trying to upload to Falcon. strings.xml could not be retrieved.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        wxStringInputStream strm(wxString(strings.c_str()));
        wxXmlDocument stringsDoc(strm);

        if (!stringsDoc.IsOk()) {
            DisplayError("Falcon Outputs Upload: Could not parse Falcon strings.xml.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        if (doProgress) progress->Update(40, "Processing current configuration data.");
        logger_base.info("Processing current configuration data.");

        currentStrings = CountStrings(stringsDoc);
        mainPixels = GetMaxPixels();
        daughter1Pixels = 0;
        daughter2Pixels = 0;
        if (SupportsVariableExpansions()) {
            mainPixels = MaxPixels(stringsDoc, 0);
            daughter1Pixels = MaxPixels(stringsDoc, 1);
            daughter2Pixels = MaxPixels(stringsDoc, 2);
        }
        else {
            if (currentStrings > GetBank1Threshold()) {
                mainPixels = mainPixels / 2;
                daughter1Pixels = mainPixels;
            }
        }

        logger_base.info("Current Falcon configuration split: Main = %d, Expansion1 = %d, Expansion2 = %d, Strings = %d", mainPixels, daughter1Pixels, daughter2Pixels, currentStrings);
        logger_base.info("Maximum string port configured in xLights: %d", cud.GetMaxPixelPort());

        ReadStringData(stringsDoc, stringData);

        logger_base.debug("Downloaded string data.");
        DumpStringData(stringData);
    }

    int maxPixels = GetMaxPixels();
    int totalPixelPorts = GetDaughter1Threshold();
    if (cud.GetMaxPixelPort() > GetDaughter2Threshold() ||
        currentStrings > GetDaughter2Threshold()) {
        logger_base.info("String port count needs to be %d.", caps->GetMaxPixelPort());
        totalPixelPorts = caps->GetMaxPixelPort();
    }
    else if (cud.GetMaxPixelPort() > GetDaughter1Threshold() ||
        currentStrings > GetDaughter1Threshold()) {
        logger_base.info("String port count needs to be %d.", GetDaughter2Threshold());
        totalPixelPorts = GetDaughter2Threshold();
    }
    else {
        logger_base.info("String port count needs to be %d.", GetDaughter1Threshold());
    }
    InitialiseStrings(stringData, totalPixelPorts, minuniverse);

    //logger_base.debug("Missing strings added.");
    //DumpStringData(stringData);

    logger_base.info("Falcon pixel split: Main = %d, Expansion1 = %d, Expansion2 = %d", mainPixels, daughter1Pixels, daughter2Pixels);

    if (doProgress) progress->Update(50, "Configuring string ports.");
    logger_base.info("Configuring string ports.");

    bool portdone[100];
    memset(&portdone, 0x00, sizeof(portdone)); // all false

    // break it up into virtual strings
    std::vector<FalconString*> newStringData;
    std::vector<FalconString*> toDelete;
    int index = 0;
    for (int pp = 1; pp <= totalPixelPorts; pp++) {
        if (cud.HasPixelPort(pp)) {
            UDControllerPort* port = cud.GetControllerPixelPort(pp);
            logger_base.info("Pixel Port %d Protocol %s.", pp, (const char*)port->GetProtocol().c_str());

            port->CreateVirtualStrings(true);

            FalconString* firstString = nullptr;
            for (const auto& sd : stringData) {
                if (sd->port == pp - 1) {
                    if (firstString == nullptr) {
                        firstString = sd;
                    }
                    else {
                        toDelete.push_back(sd);
                    }
                }
            }
            wxASSERT(firstString != nullptr);

            // need to add virtual strings
            bool first = true;
            for (const auto& vs : port->GetVirtualStrings()) {
                FalconString* fs;
                if (first) {
                    fs = firstString;
                    first = false;
                }
                else {
                    fs = new FalconString();
                }

                // ignore index ... we will fix them up when done
                fs->port = firstString->port;
                fs->index = index++;
                fs->virtualStringIndex = vs->_index;
                fs->protocol = DecodeStringPortProtocol(vs->_protocol);
                fs->universe = vs->_universe;
                if (_usingAbsolute) {
                    fs->startChannel = vs->_startChannel;
                }
                else {
                    fs->startChannel = vs->_universeStartChannel;
                }
                fs->pixels = vs->Channels() / 3;
                fs->description = SafeDescription(vs->_description);
                fs->smartRemote = vs->_smartRemote;
                if (vs->_brightnessSet) {
                    fs->brightness = vs->_brightness;
                }
                else {
                    fs->brightness = firstString->brightness;
                }
                if (vs->_nullPixelsSet) {
                    fs->nullPixels = vs->_nullPixels;
                }
                else {
                    fs->nullPixels = firstString->nullPixels;
                }
                if (vs->_gammaSet) {
                    fs->gamma = vs->_gamma;
                }
                else {
                    fs->gamma = firstString->gamma;
                }
                if (vs->_colourOrderSet) {
                    fs->colourOrder = vs->_colourOrder;
                }
                else {
                    fs->colourOrder = firstString->colourOrder;
                }
                if (vs->_reverseSet) {
                    fs->direction = vs->_reverse;
                }
                else {
                    fs->direction = firstString->direction;
                }
                if (vs->_groupCountSet) {
                    fs->groupCount = vs->_groupCount;
                    fs->pixels *= vs->_groupCount;
                }
                else {
                    fs->groupCount = firstString->groupCount;
                    fs->pixels *= firstString->groupCount;
                }
                newStringData.push_back(fs);
            }
        }
        else {
            int priorSmartRemote = 0;
            for (const auto& sd : stringData) {
                if (sd->port == pp - 1) {
                    sd->index = index++;
                    if (sd->port % 4 != 0 && priorSmartRemote != 0 && sd->smartRemote == 0) sd->smartRemote = 1;
                    newStringData.push_back(sd);
                }
                priorSmartRemote = sd->smartRemote;
            }
        }
    }
    stringData = newStringData;

    // delete any read strings we didnt keep
    for (const auto& d : toDelete) {
        delete d;
    }

    logger_base.debug("Virtual strings created.");
    DumpStringData(stringData);

    logger_base.info("Working out required pixel splits.");
    int maxMain = 0;
    int maxDaughter1 = 0;
    int maxDaughter2 = 0;

    for (auto pp = 0; pp < totalPixelPorts; ++pp) {
        int pixels = GetPixelCount(stringData, pp);
        if (pp < GetBank1Threshold()) {
            if (pixels > maxMain) maxMain = pixels;
        }
        else if (pp < GetDaughter2Threshold()) {
            if (pixels > maxDaughter1) maxDaughter1 = pixels;
        }
        else {
            if (pixels > maxDaughter2) maxDaughter2 = pixels;
        }
    }

    int maxPort = GetMaxPixelPort(stringData);

    if (!SupportsVariableExpansions()) {
        // minimum of main is 1
        if (maxMain == 0) {
            maxMain = 1;
        }

        if (maxDaughter1 > 0) {
            if (maxMain > maxPixels / 2 || maxDaughter1 > maxPixels / 2) {
                DisplayError(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports 340/340 pixel split with expansion board. (%d/%d)",
                    _ip, maxMain, maxDaughter1).ToStdString());
                success = false;
            }

            maxMain = maxPixels / 2;
            maxDaughter1 = maxPixels / 2;

            if (maxDaughter2 > 0) {
                DisplayError(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports one expansion board.",
                    _ip).ToStdString());
                success = false;
                maxDaughter2 = 0;
            }
        }

        logger_base.info("Falcon pixel fixed split: Main = %d, Expansion1 = %d", maxMain, maxDaughter1);
    }
    else {
        if (maxMain == 0) {
            maxMain = 1;
        }

        if (maxPort > GetDaughter2Threshold() && maxDaughter2 == 0) {
            maxDaughter2 = 1;
        }

        if (maxPort > GetBank1Threshold() && maxDaughter1 == 0) {
            maxDaughter1 = 1;
        }

        if (IsF4() && IsV3()) {
            // v3 supports 1024 on all outputs
            maxMain = maxPixels;
            maxDaughter1 = 0;
            maxDaughter2 = 0;
        }

        logger_base.info("Falcon pixel required split: Main = %d, Expansion1 = %d, Expansion2 = %d", maxMain, maxDaughter1, maxDaughter2);

        if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
            success = false;
            check += "ERROR: Total pixels exceeded maximum allowed on a pixel port: " + wxString::Format("%d", maxPixels).ToStdString() + "\n";

            logger_base.warn("ERROR: Total pixels exceeded maximum allowed on a pixel port: %d", maxPixels);

            if (_modelnum == 48) {
                check += "Trying to disable unused banks on the F48.\n";
                logger_base.debug("Trying to disable unused banks on the F48.");
                // if it looks like we arent using the last 16 ports and everything is still set to the default
                if (cud.GetMaxPixelPort() <= 16 && maxDaughter1 == 50 && maxDaughter2 == 50) {
                    int left = maxPixels - maxMain;
                    maxDaughter1 = left / 2;
                    maxDaughter1 = std::max(1, maxDaughter1);
                    maxDaughter2 = left - maxDaughter1;
                    maxDaughter2 = std::max(1, maxDaughter2);
                    success = true;
                    if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
                        success = false;
                        check += "ERROR: It looked like you were only using the first 16 outputs but even accounting for that there are too many pixels on this port.\n";
                        logger_base.error("It looked like you were only using the first 16 outputs but even accounting for that there are too many pixels on this port.");
                    }
                }
                else if (cud.GetMaxPixelPort() <= 32 && maxDaughter2 == 50) {
                    maxDaughter2 = maxPixels - maxMain - maxDaughter1;
                    maxDaughter2 = std::max(1, maxDaughter2);
                    success = true;
                    if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
                        success = false;
                        check += "ERROR: It looked like you were only using the first 32 outputs but even accounting for that there are too many pixels on this port.\n";
                        logger_base.error("It looked like you were only using the first 32 outputs but even accounting for that there are too many pixels on this port.");
                    }
                }
                else {
                    check += "ERROR: Unable to adjust banks. Resetting your controller to its defaults may help.\n";
                    logger_base.error("    Unable to adjust banks. Resetting your controller to its defaults may help.");
                }
            }
        }

        if (maxMain + maxDaughter1 + maxDaughter2 < maxPixels) {
            if (maxDaughter2 != 0) {
                maxDaughter2 = maxPixels - maxMain - maxDaughter1;
            }
            else if (maxDaughter1 != 0) {
                maxDaughter1 = maxPixels - maxMain;
            }
            else {
                maxMain = maxPixels;
            }
            logger_base.info("Falcon pixel split adjusted to add up to %d: Main = %d, Expansion1 = %d, Expansion2 = %d", maxPixels, maxMain, maxDaughter1, maxDaughter2);
        }
    }

    if (success && cud.GetMaxPixelPort() > 0) {
        for (int i = 1; i <= cud.GetMaxPixelPort(); i += 4) {
            bool smartRemoteFound = false;
            bool nonSmartRemoteFound = false;
            for (int j = 0; j < 4; j++) {
                if (cud.GetControllerPixelPort(i + j)->AtLeastOneModelIsUsingSmartRemote()) {
                    smartRemoteFound = true;
                }
                if (cud.GetControllerPixelPort(i + j)->AtLeastOneModelIsNotUsingSmartRemote()) {
                    nonSmartRemoteFound = true;
                }
            }

            if (smartRemoteFound && nonSmartRemoteFound) {
                success = false;
                check += wxString::Format("ERROR: Ports %d-%d have a mix of models using smart and non-smart remotes. This is not supported.\n", i, i + 3);
                logger_base.error("Ports %d-%d have a mix of models using smart and non-smart remotes. This is not supported.", i, i + 3);
            }
        }
    }

    if (success && cud.GetMaxPixelPort() > 0) {
        if (doProgress) progress->Update(60, "Uploading string ports.");

        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
            check = ""; // to suppress double display
        }

        logger_base.info("Uploading string ports.");
        UploadStringPorts(stringData, maxMain, maxDaughter1, maxDaughter2, minuniverse);
    }
    else {
        if (cud.GetMaxPixelPort() > 0 && caps->GetMaxPixelPort() > 0 && check != "") {
            DisplayError("Not uploaded due to errors.\n" + check);
            check = "";
        }
    }

    // delete all our string data
    while (stringData.size() > 0) {
        delete stringData.back();
        stringData.pop_back();
    }

    if (success && cud.GetMaxSerialPort() > 0) {
        if (doProgress) progress->Update(90, "Uploading serial ports.");

        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
        }

        std::string uri = "btnSave=Save";

        for (int sp = 1; sp <= cud.GetMaxSerialPort(); sp++) {
            if (cud.HasSerialPort(sp)) {
                UDControllerPort* port = cud.GetControllerSerialPort(sp);
                logger_base.info("Serial Port %d Protocol %s.", sp, (const char*)port->GetProtocol().c_str());

                int dmxOffset = 1;
                UDControllerPortModel* m = port->GetFirstModel();
                if (m != nullptr) {
                    dmxOffset = m->GetDMXChannelOffset();
                    if (dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
                }

                int sc = port->GetStartChannel() - dmxOffset + 1;
                logger_base.debug("    sc:%d - offset:%d -> %d", port->GetStartChannel(), dmxOffset, sc);

                uri += "&";
                uri += GetSerialOutputURI(caps, port->GetPort(), outputManager, DecodeSerialOutputProtocol(port->GetProtocol()), sc, parent);
            }
        }

        if (uri != "")
        {
            PutURL("/SerialOutputs.htm", uri);
        }
    }
    else {
        if (caps->GetMaxSerialPort() > 0 && check != "") {
            DisplayError("Not uploaded due to errors.\n" + check);
        }
    }

    if (doProgress) progress->Update(100, "Done.");
    logger_base.info("Falcon upload done.");

    return success;
}
#pragma endregion
