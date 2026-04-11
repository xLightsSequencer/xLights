
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <regex>

#include "../render/UICallbacks.h"

#include <cassert>

#include "AlphaPix.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"
#include "UtilFunctions.h"
#include "../utils/string_utils.h"

#include <log.h>

#include <chrono>
#include <curl/curl.h>
#include <thread>

#include <spdlog/fmt/fmt.h>

#pragma region Output Classes
class AlphaPixOutput
{
public:
    const int output;
    int universe{ 1 };
    int startChannel{ 1 };
    int pixels{ 1 };
    int nullPixel{ 0 };
    int colorOrder{ 0 };
    bool reverse{ false };
    int brightness{ 100 };
    int zigZag{ 0 };
    bool upload{ false };

    explicit AlphaPixOutput(int output_) : output(output_) { }
    void Dump() const {
        spdlog::debug("    Output {} Uni {} StartChan {} Pixels {} Rev {} Nulls {} Brightness {} ZigZag {} ColorOrder {} Upload {}",
            output,
            universe,
            startChannel,
            pixels,
            toStr(reverse),
            nullPixel,
            brightness,
            zigZag,
            colorOrder,
            toStr(upload)
        );
    }
};

class AlphaPixSerial
{
public:
    const int output;
    int universe{ 1 };
    bool enabled{ false };
    bool upload{ false };

    explicit AlphaPixSerial(int output_) : output(output_) { }
    void Dump() const {
        spdlog::debug("    Output {} Uni {} Enabled {} Upload {}",
            output,
            universe,
            toStr(enabled),
            toStr(upload)
        );
    }
};

class AlphaPixData
{
public:
    std::string name;
    int protocol{ 0 };
    int inputMode{ 0 };
    AlphaPixData() = default;
    void Dump() const {
        spdlog::debug("    Name {} Protocol {} InputMode {}",
            name,
            protocol,
            inputMode
        );
    }
};
#pragma endregion

#pragma region Constructors and Destructors
AlphaPix::AlphaPix(const std::string& ip, const std::string &proxy) : BaseController(ip, proxy) {
    _page = APGetURL("/");
    if (!_page.empty()) {
        if (Contains(_page, "Existing user login")) {
            spdlog::error("AlphaPix Webpage locked out by another computer");
        }
        //AlphaPix 4 V2/V3 Classic
        //AlphaPix Flex Lighting Controller
        static std::regex modelregex("(\\d+) Port Ethernet to SPI Controller");
        static std::regex modelregex2("AlphaPix (\\d+) ");
        std::smatch modm;
        if (std::regex_search(_page, modm, modelregex)) {
            _modelnum = (int)std::strtol(modm[1].str().c_str(), nullptr, 10);
            _connected = true;
        }
        else if (std::regex_search(_page, modm, modelregex2)) {
            _modelnum = (int)std::strtol(modm[1].str().c_str(), nullptr, 10);
            _connected = true;
        } else if (Contains(_page, "AlphaPix Flex Lighting Controller") || Contains(_page, "AlphaPix Evolution Lighting Controller")) {
            _modelnum = 48;
            _connected = true;
        }
        else {
            spdlog::error("Error Determining AlphaPix controller Type.");
            _connected = false;
        }

        //Currently Installed Firmware Version:  2.08
        static std::regex firmwareregex("(Currently Installed Firmware Version:  ([0-9]+.[0-9]+))");
        std::smatch fwm;
        if (std::regex_search(_page, fwm, firmwareregex)) {
            _version = fwm[2].str();
        }

        if (Contains(_page, "name=\"U01\"")) { // look for certain web element. Fix for new webUI on firmware 2.16, 2.18 and maybe 2.12,2.13. Firmware has the same format as Flex Controller
            _revision = 2;
            spdlog::debug("v2 WebPage format, AlphaPix 4/16 Firmware 2.16+ or AlphaPix Flex/Evolution Firmware 4.3+");
        } else {
            _revision = 1;
            spdlog::debug("v1 WebPage format, AlphaPix 4/16 Firmware 2.08 and below or AlphaPix Flex/Evolution Firmware 4.2");
        }

        if (_modelnum == 48) {
            _model = fmt::format("AlphaPix Flex v{}", _revision);
        }
        else { 
            _model = fmt::format("AlphaPix {} v{}", _modelnum, _revision);
        }

        if(_connected)
            spdlog::debug("Connected to AlphaPix controller model {}.", GetFullName());
    }
    else {
        _connected = false;
        spdlog::error("Error connecting to AlphaPix controller on {}.", _ip);
    }
}

AlphaPix::~AlphaPix() {

    for (const auto& it : _pixelOutputs)
    {
        delete it;
    }
    _pixelOutputs.clear();

    for (const auto& it : _serialOutputs)
    {
        delete it;
    }
    _serialOutputs.clear();
}
#pragma endregion

#pragma region Private Functions
bool AlphaPix::ParseWebpage(std::string const& page, AlphaPixData& data) {

    _pixelOutputs.clear();
    _serialOutputs.clear();
    data.name = ExtractName(page);
    data.inputMode = ExtractInputType(page);
    data.protocol = ExtractProtocol(page);

    for (int i = 1; i <= GetNumberOfOutputs(); i++) {
        AlphaPixOutput* output;
        if (_revision == 2) {
            output = ExtractOutputDataV2(page, i);
        } else {
            output = ExtractOutputData(page, i);
        }
        output->Dump();
        _pixelOutputs.push_back(output);
    }

    for (int i = 1; i <= GetNumberOfSerial(); i++) {
        AlphaPixSerial* serial = ExtractSerialData(page, i);
        serial->Dump();
        _serialOutputs.push_back(serial);
    }

    const int colorType = ExtractColorType(page);
    //if single color order, set all to same color
    if (colorType == 0) {
        const int color = ExtractColor(page);
        for (auto& pixelPort : _pixelOutputs) {
            pixelPort->colorOrder = color;
        }
    }
    else {
        //Load advance color order page
        const std::string colorPage = APPutURL(GetColorOrderURL(), "RGBORD=1");
        for (auto& pixelPort : _pixelOutputs) {
            pixelPort->colorOrder = ExtractSingleColor(colorPage, pixelPort->output);
        }
    }
    return true;
}

AlphaPixOutput* AlphaPix::ExtractOutputData(std::string const& page, int port) {

    AlphaPixOutput* output = new AlphaPixOutput(port);

    output->universe = ExtractIntFromPage(page, fmt::format("SU{}", port), "input", 1);
    output->startChannel = ExtractIntFromPage(page, fmt::format("SC{}", port), "input", 1);
    output->pixels = ExtractIntFromPage(page, fmt::format("PC{}", port), "input", 1);
    output->nullPixel = ExtractIntFromPage(page, fmt::format("NP{}", port), "input", 0);
    output->zigZag = ExtractIntFromPage(page, fmt::format("RA{}", port), "input", 0);
    output->brightness = ExtractIntFromPage(page, fmt::format("LM{}", port), "input", 100);
    output->reverse = ExtractIntFromPage(page, fmt::format("RV{}", port), "checkbox", 0);

    return output;
}

AlphaPixOutput* AlphaPix::ExtractOutputDataV2(std::string const& page, int port) {

    AlphaPixOutput* output = new AlphaPixOutput(port);
    output->universe = ExtractIntFromPage(page, fmt::format("U{:02d}", port), "input", 1);
    output->startChannel = ExtractIntFromPage(page, fmt::format("C{:02d}", port), "input", 1);
    output->pixels = ExtractIntFromPage(page, fmt::format("P{:02d}", port), "input", 1);
    output->nullPixel = ExtractIntFromPage(page, fmt::format("N{:02d}", port), "input", 0);
    output->zigZag = ExtractIntFromPage(page, fmt::format("R{:02d}", port), "input", 0);
    output->brightness = ExtractIntFromPage(page, fmt::format("L{:02d}", port), "input", 100);
    output->reverse = ExtractIntFromPage(page, fmt::format("V{:02d}", port), "checkbox", 0);

    return output;
}

AlphaPixSerial* AlphaPix::ExtractSerialData(std::string const& page, int port) {
    AlphaPixSerial* serial = new AlphaPixSerial(port);
    if (_modelnum == 4) {
        serial->enabled = ExtractDMXEnabled(page, "Rever5");
        serial->universe = ExtractDMXUniverse(page, "DMX512");
    }
    else {
        serial->enabled = ExtractDMXEnabled(page, fmt::format("Rever{}", port));
        serial->universe = ExtractDMXUniverse(page, fmt::format("DMX512_{}", port));
    }
    return serial;
}

std::string AlphaPix::ExtractName(std::string const& page) {
    int const start = page.find("Device name:");
    std::string const name = ExtractFromPage(page, "name", "input", start);
    return name;
}

int AlphaPix::ExtractInputType(std::string const& page) {
    int const start = page.find("Ethernet Protocol:");
    int const value = ExtractIntFromPage(page, "EP", "radio", 0, start);
    return value;
}

int AlphaPix::ExtractDMXUniverse(std::string const& page, std::string const& name) {
    const int start = page.find("DMX Universe:");
    const int value = ExtractIntFromPage(page, name, "input", 0, start);
    return value;
}

int AlphaPix::ExtractProtocol(std::string const& page) {
    const int start = page.find("Pixel IC Protocol:");
    const int protocol = ExtractIntFromPage(page, "IC", "select", 0, start);
    return protocol;
}

int AlphaPix::ExtractColor(std::string const& page) {
    const int start = page.find("Pixel Chip Color Output Order:");
    const int colorOrder = ExtractIntFromPage(page, "RGBS", "select", 0, start);
    return colorOrder;
}

int AlphaPix::ExtractColorType(std::string const& page) {
    int const start = page.find("Pixel Chip Color Output Order:");
    int const value = ExtractIntFromPage(page, "RGBORD", "radio", 0, start);
    return value;
}

int AlphaPix::ExtractSingleColor(std::string const& page, const int output) {
    const int start = page.find(fmt::format("Output {}:", output));
    const int colorOrder = ExtractIntFromPage(page, fmt::format("{}_RGB", output), "select", 0, start);
    return colorOrder;
}

bool AlphaPix::ExtractDMXEnabled(std::string const& page, std::string const& name) {
    const int start = page.find("Enabled:");
    const bool useDMX = ExtractBoolFromPage(page, name, "checkbox", false, start);
    return useDMX;
}

void AlphaPix::UpdatePortData(AlphaPixOutput* pd, UDControllerPort* stringData, bool& changeColor) const {

    if (pd != nullptr) {
        const int nullPix = stringData->GetFirstModel()->GetStartNullPixels(-1);
        if (nullPix != -1 && pd->nullPixel != nullPix) {
            pd->nullPixel = nullPix;
            pd->upload = true;
        }

        const int brightness = stringData->GetFirstModel()->GetBrightness(-1);
        if (brightness != -1 && pd->brightness != brightness) {
            pd->brightness = brightness;
            pd->upload = true;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown" && pd->reverse != EncodeDirection(direction)) {
            pd->reverse = EncodeDirection(direction);
            pd->upload = true;
        }

        const std::string color = stringData->GetFirstModel()->GetColourOrder("");
        if (!color.empty()) {
            int newcolor = EncodeColorOrder(color);
            if (pd->colorOrder != newcolor) {
                pd->colorOrder = newcolor;
                changeColor = true;
            }
        }

        if (pd->universe != stringData->GetUniverse()) {
            pd->universe = stringData->GetUniverse();
            pd->upload = true;
        }

        if (pd->startChannel != stringData->GetUniverseStartChannel()) {
            pd->startChannel = stringData->GetUniverseStartChannel();
            pd->upload = true;
        }

        if (pd->pixels != stringData->Pixels()) {
            pd->pixels = stringData->Pixels();
            pd->upload = true;
        }
    }
}

void AlphaPix::UpdateSerialData(AlphaPixSerial* pd, UDControllerPort* serialData) const {

    if (pd != nullptr) {
        if (pd->universe != (serialData->GetUniverse())) {
            pd->universe = serialData->GetUniverse();
            pd->upload = true;
        }

        if (!pd->enabled) {
            pd->enabled = true;
            pd->upload = true;
        }
    }
}

std::string AlphaPix::ExtractFromPage(std::string const& page, const std::string& parameter, const std::string& type, int start) {
    const std::string p = page.substr(start);
    if (type == "input") {
        //<input  style = " width: 80px ;TEXT-ALIGN: center" type="text" value="1" name="DMX512"/>
        //<input\s+style\s=\s\".*\"\stype="text"\s+value=\")([0-9\\.]*?)\"
        //<input\s+style\s=\s\".*\"\stype=\"text\"\s+value=\"(.*)\"\s+name=\"SU1\"
        std::regex inputregex("<input\\s+style\\s?=\\s?\".*\"\\stype=\"text\"\\s+value=\"(.*)\"\\s+name=\"" + parameter + "\"");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[1].str();
        }
    }
    else if (type == "select") {
        size_t startSel = p.find("<select name=\"" + parameter + "\"");
        std::string pSel = (startSel != std::string::npos) ? p.substr(startSel) : p;
        //<select name="RGBS"
        //<option value="([0-9])\"\sselected=\"selected\"
        std::regex inputregex("<option\\s+value=\"([0-9])\"\\sselected=\"selected\"");
        std::smatch m;
        if (std::regex_search(pSel, m, inputregex)) {
            return m[1].str();
        }
    }
    else if (type == "checkbox") {
        //<input\s+(?:style\s=\s\".*\"\s+)?type=\"checkbox\"\s+name=\"(\w+)\"\s+(checked=\"checked\"\s+)?value=\"[0-9]\"
        std::regex inputregex("<input\\s+(?:style\\s?=\\s?\".*\"\\s+)?type=\"checkbox\"\\s+name=\"" + parameter + "\"\\s+(checked=\"checked\"\\s+)?value=\"[0-9]\"");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            const std::string res2 = m[1].str();
            if (!res2.empty())
                return "1";
            return "0";
        }
    }
    else if (type == "radio") {
        // <input\s+type="radio"\s+(?:id="\w+")?\s+name=\"\w+"\s+value=\"([0-9])\"\s+checked="checked"
        std::regex inputregex("<input\\s+type=\"radio\"\\s+(?:id=\"\\w+\")?\\s+name=\"" + parameter + "\"\\s+value=\"([0-9])\"\\s+checked=\"checked\"");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[1].str();
        }
    }
    else {
        spdlog::error("AlphaPix::ExtractFromPage   Invalid Regex Type:{}", type);
        assert(false);
    }

    return "";
}

int AlphaPix::ExtractIntFromPage(std::string const& page, const std::string& parameter, const std::string& type, int defaultValue, int start) {
    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return (int)strtol(value.c_str(), nullptr, 10);
    }
    return defaultValue;
}

bool AlphaPix::ExtractBoolFromPage(std::string const& page, const std::string& parameter, const std::string& type, bool defaultValue, int start) {
    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return value == "1";
    }
    return defaultValue;
}

int AlphaPix::EncodeStringPortProtocol(const std::string& protocol) const {

    std::string const p = Lower(protocol);

    if (p == "ws2811") return 0;
    if (p == "ws2801") return 1;
    if (p == "lpd6803") return 2;
    if (p == "tls3001" && _modelnum != 48) return 4;
    if (p == "tm18xx" && _modelnum != 48) return 6;
    if (p == "tm18xx" && _modelnum == 48) return 4;
    assert(false);
    return 0;
}

int AlphaPix::EncodeColorOrder(const std::string& colorOrder) const {

    std::string const c = Lower(colorOrder);
    if (c == "rgb") return 0;
    if (c == "rbg") return 1;
    if (c == "grb") return 2;
    if (c == "gbr") return 3;
    if (c == "brg") return 4;
    if (c == "bgr") return 5;
    assert(false);
    return 0;
}

bool AlphaPix::EncodeDirection(const std::string& direction) const {

    return direction == "Reverse";
}

AlphaPixOutput* AlphaPix::FindPortData(int port) {

    for (const auto& sd : _pixelOutputs) {
        if (sd->output == port) {
            return sd;
        }
    }
    assert(false);
    return nullptr;
}

AlphaPixSerial* AlphaPix::FindSerialData(int port) {

    for (const auto& sd : _serialOutputs) {
        if (sd->output == port) {
            return sd;
        }
    }
    assert(false);
    return nullptr;
}

std::string AlphaPix::BuildStringPortRequest(AlphaPixOutput* po) const {   
    spdlog::debug("     Output String {}, Universe {} StartChannel {} Pixels {}",
        po->output, po->universe, po->startChannel, po->pixels);

    std::string reverseAdd;
    if (po->reverse) {
        reverseAdd = fmt::format("&RV{}=1", po->output);
    }

    return fmt::format("SU{}={}&SC{}={}&PC{}={}&NP{}={}&RA{}={}&LM{}={}{}",
        po->output, po->universe,
        po->output, po->startChannel,
        po->output, po->pixels,
        po->output, po->nullPixel,
        po->output, po->zigZag,
        po->output, po->brightness,
        reverseAdd);
}

std::string AlphaPix::BuildStringPortRequestV2(AlphaPixOutput* po) const { 
    spdlog::debug("     Output String {}, Universe {} StartChannel {} Pixels {}",
        po->output, po->universe, po->startChannel, po->pixels);

    std::string reverseAdd;
    if (po->reverse) {
        reverseAdd = fmt::format("&V{:02d}=1", po->output);
    }

    return fmt::format("U{:02d}={}&C{:02d}={}&P{:02d}={}&N{:02d}={}&R{:02d}={}&L{:02d}={}{}",
        po->output, po->universe,
        po->output, po->startChannel,
        po->output, po->pixels,
        po->output, po->nullPixel,
        po->output, po->zigZag,
        po->output, po->brightness,
        reverseAdd);
}

std::string AlphaPix::SafeDescription(const std::string& description) const {
    std::string desc(description);
    if (desc.length() > 16) {
        return desc.substr(0, 16);
    }
    return desc;
}

std::string AlphaPix::APGetURL(const std::string& url) const
{    
    std::string res;
    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;

    CURL* curl = curl_easy_init();
    if (curl) {
        auto u = std::string("http://" + baseIP + _baseUrl + url);
        spdlog::debug("Curl GET: {}", u);
        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
        curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        /* Perform the request, res will get the return code */
        CURLcode r = curl_easy_perform(curl);

        if (r != CURLE_OK) {
            spdlog::error("Failure to access {}: {}.", url, curl_easy_strerror(r));
        } else {
            res = response_string;
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return res;
}

std::string AlphaPix::APPutURL(const std::string& url, const std::string& request) const
{
    

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    spdlog::debug("Making request to Controller '{}'.", url);
    spdlog::debug("    With data '{}'.", request);

    CURL* curl = curl_easy_init();
    if (curl != nullptr) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        auto const u = std::string("http://" + baseIP + _baseUrl + url);
        spdlog::debug("Curl POST: {}", u);
        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "content-type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)request.c_str());
        // curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
        std::string buffer = "";
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode ret = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (ret == CURLE_OK) {
            return buffer;
        }
        spdlog::error("Failure to access {}: {}.", url, curl_easy_strerror(ret));
    }

    return "";
}
#pragma endregion

#pragma region Getters and Setters
bool AlphaPix::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {

    auto progressTk = ui->BeginProgress("Uploading ...", 100);

    spdlog::debug("AlphaPix Outputs Upload: Uploading to {}", _ip);

    ui->UpdateProgress(progressTk, 0, "Scanning models");
    spdlog::info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    //first check rules
    auto caps = ControllerCaps::GetControllerConfig(controller);
    const bool success = cud.Check(caps, check);

    spdlog::debug(check);

    cud.Dump();
    if (!success) {
        ui->ShowMessage("AlphaPix Upload Error:\n" + check, "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }
    //get current config Page
    const std::string page = _page;

    if (page.empty()) {
        ui->ShowMessage("AlphaPix Upload Error:\nWebpage was empty", "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    bool worked = true;
    AlphaPixData controllerData = AlphaPixData();

    _connected = ParseWebpage(_page, controllerData);
    if (!_connected) {
        ui->ShowMessage("Unable to Parse Main Webpage.", "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    spdlog::info("Figuring Out Pixel Output Information.");
    ui->UpdateProgress(progressTk,10, "Figuring Out Pixel Output Information.");

    bool uploadColor = false;
    std::vector<int> colorOrder;
    std::string pixelType;

    //loop to setup string outputs
    for (int port = 1; port <= GetNumberOfOutputs(); port++) {
        if (cud.HasPixelPort(port)) {
            UDControllerPort* portData = cud.GetControllerPixelPort(port);
            AlphaPixOutput* pixOut = FindPortData(port);
            UpdatePortData(pixOut, portData, uploadColor);
            if (pixelType.empty())
                pixelType = portData->GetFirstModel()->GetProtocol();

            colorOrder.push_back(pixOut->colorOrder);
        }
    }

    spdlog::info("Uploading String Output Information.");
    ui->UpdateProgress(progressTk,20, "Uploading String Output Information.");
    if (_modelnum == 48)
        UploadFlexPixelOutputs(worked);
    else
        UploadPixelOutputs(worked);

    spdlog::info("Figuring Out DMX Output Information.");
    ui->UpdateProgress(progressTk,30, "Figuring Out DMX Output Information.");
    for (int port = 1; port <= GetNumberOfSerial(); port++) {
        if (cud.HasSerialPort(port)) {
            UDControllerPort* portData = cud.GetControllerSerialPort(port);
            AlphaPixSerial* serialOut = FindSerialData(port);
            UpdateSerialData(serialOut, portData);
        }
    }

    spdlog::info("Uploading DMX Output Information.");
    ui->UpdateProgress(progressTk,40, "Uploading DMX Output Information.");
    for (const auto& serial : _serialOutputs) {
        serial->Dump();
        if (serial->upload) {
            if (_modelnum == 4) {
                const std::string serialRequest = fmt::format("Rever5=1&DMX512={}", serial->universe);
                const std::string res = APPutURL(GetDMXURL(), serialRequest);
                if (res.empty())
                    worked = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            else {
                const std::string serialRequest = fmt::format("Rever{}=1&DMX512_{}={}",
                    serial->output, serial->output, serial->universe);
                const std::string res = APPutURL(GetDMXURL(serial->output), serialRequest);
                if (res.empty())
                    worked = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    }

    spdlog::info("Uploading Protocol Type.");
    ui->UpdateProgress(progressTk,50, "Uploading Protocol Type.");
    const int newProtocol = EncodeStringPortProtocol(pixelType);
    if (newProtocol != -1 && controllerData.protocol != newProtocol) {
        const std::string res = APPutURL(GetProtocolURL(), fmt::format("IC={}", newProtocol));
        if (res.empty())
            worked = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    spdlog::info("Uploading Color Order.");
    ui->UpdateProgress(progressTk,60, "Uploading Color Order.");

    if (uploadColor) {
        std::sort(colorOrder.begin(), colorOrder.end());
        colorOrder.erase(std::unique(colorOrder.begin(), colorOrder.end()), colorOrder.end());
        if (colorOrder.size() == 1) {
            //all the same color order, "simple mode" will do
            const std::string res = APPutURL(GetColorOrderURL(), fmt::format("RGBORD=0&RGBS={}", colorOrder[0]));
            if (res.empty())
                worked = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else {
            // different color orders, "advance mode" needed
            const std::string res = APPutURL(GetColorOrderURL(), "RGBORD=1");
            if (res.empty())
                worked = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::string colorRequestString;
            for (const auto& pixelPort : _pixelOutputs) {
                if (colorRequestString != "")
                    colorRequestString += "&";
                colorRequestString += fmt::format("{}_RGB={}",
                    pixelPort->output, pixelPort->colorOrder);
            }

            const std::string res2 = APPutURL(GetIndvColorOrderURL(), colorRequestString);
            if (res2.empty())
                worked = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    spdlog::info("Uploading Output Description.");
    ui->UpdateProgress(progressTk,70, "Uploading Output Description.");
    const std::string outName = SafeDescription(controller->GetName());
    if (!outName.empty() && controllerData.name != outName) {
        const std::string res = APPutURL(GetNameURL(), "name=" + outName);
        if (res.empty())
            worked = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //upload Input Type
    auto o = controller->GetFirstOutput();
    std::string requestInputString;
    if (o->GetType() == OUTPUT_E131) {
        if (controllerData.inputMode != 0)
            requestInputString = "EP=0";
    }
    else if (o->GetType() == OUTPUT_ARTNET) {
        if (controllerData.inputMode != 1)
            requestInputString = "EP=1";
    }

    spdlog::info("Uploading Output Type.");
    ui->UpdateProgress(progressTk,80, "Updating Output Type.");
    if (!requestInputString.empty()) {
        const std::string res = APPutURL(GetInputTypeURL(), requestInputString);
        if (res.empty())
            worked = false;
        //wait for reboot
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    if(!worked)
        spdlog::error("Error Uploading to AlphaPix controller, Page HTML:{}.", (const char*)_page.c_str());

    ui->EndProgress(progressTk);
    return worked;
}

void AlphaPix::UploadPixelOutputs(bool& worked) {

    
    spdlog::debug("Building pixel upload:");
    std::string requestString;
    for (const auto& pixelPort : _pixelOutputs) {
        if (requestString != "")
            requestString += "&";
        if(_revision == 2)
            requestString += BuildStringPortRequestV2(pixelPort);
        else
            requestString += BuildStringPortRequest(pixelPort);
    }

    spdlog::info("PUT String Output Information.");

    if (!requestString.empty()) {
        const std::string res = APPutURL(GetOutputURL(), requestString);
        if (res.empty())
            worked = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

void AlphaPix::UploadFlexPixelOutputs(bool& worked) {

    
    spdlog::debug("Building pixel upload:");

    for (int i = 0; i < 3; i++) {
        const int startPort = (i * 16) + 1;
        const int endPort = (i * 16) + 16;
        std::string requestString;
        bool upload = false;
        for (int port = startPort; port <= endPort; port++) {
            AlphaPixOutput* pixelPort = FindPortData(port);
            if (requestString != "")
                requestString += "&";
            if (_revision == 2)
                requestString += BuildStringPortRequestV2(pixelPort);
            else
                requestString += BuildStringPortRequest(pixelPort);
            upload |= pixelPort->upload;
        }

        spdlog::info("PUT String Output Information.");
        if (!requestString.empty() && upload) {
            const std::string res = APPutURL(GetOutputURL(i + 1), requestString);
            if (res.empty())
                worked = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
}
#pragma endregion
