
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "SanDevices.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "UtilFunctions.h"
#include "../utils/string_utils.h"
#include "../utils/DisplayMessages.h"

#include "../render/UICallbacks.h"

#include <regex>

#include <cassert>
#include <chrono>
#include <format>
#include <thread>

#include <log.h>

#pragma region Dumps
void SanDevicesOutput::Dump() const {
    spdlog::debug("    Group {} Output {} Port {} Uni {} StartChan {} Pixels {} GroupCount {} Rev {} ColorOrder {} Nulls {} Brightness {} Chase {} firstZig {} thenEvery {} Upload {}",
        group,
        output,
        stringport,
        universe,
        startChannel,
        pixels,
        groupCount,
        toStr(reverse),
        colorOrder,
        nullPixel,
        brightness,
        toStr(chase),
        firstZig,
        thenEvery,
        toStr(upload)
        );
}

void SanDevicesProtocol::Dump() const {
    spdlog::debug("    Group {} Protocol {} Timing {} Upload {}",
        getGroup(),
        getProtocol(),
        getTiming(),
        toStr(shouldUpload()));
}

void SanDevicesOutputV4::Dump() const {
    spdlog::debug("    Group {} outputSize {} Uni {} StartChan {} Pixels {} GroupCount {} Rev {},{},{},{} ColorOrder {} Nulls {},{},{},{} ZigZag {} Upload {}",
        group,
        outputSize,
        universe,
        startChannel,
        pixels,
        groupCount,
        toStr(reverse[0]),
        toStr(reverse[1]),
        toStr(reverse[2]),
        toStr(reverse[3]),
        colorOrder,
        nullPixel[0],
        nullPixel[1],
        nullPixel[2],
        nullPixel[3],
        zigzag,
        toStr(upload)
    );
}
#pragma endregion


#pragma region Private Functions
bool SanDevices::SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {

    auto progressTk = ui->BeginProgress("Uploading ...", 100);

    spdlog::debug("SanDevices Outputs Upload: Uploading to {}", _ip);

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    ui->UpdateProgress(progressTk, 0, "Scanning models");
    spdlog::info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    //first check rules
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool const success = cud.Check(rules, check);

    spdlog::debug(check);

    cud.Dump();

    if (!success) {
        ui->ShowMessage("SanDevices Upload Error:\n" + check, "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    //get current config Page
    const std::string page = _page;

    if (page.empty()) {
        ui->EndProgress(progressTk);
        return false;
    }

    ui->UpdateProgress(progressTk, 30, "Reading Protocol Data from Controller.");
    spdlog::info("Reading Protocol Data from Controller.");

    _connected = ParseV4Webpage(_page);
    if (!_connected) {
        ui->ShowMessage("Unable to Parse Webpage.", "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    ui->UpdateProgress(progressTk, 50, "Figuring Out Protocol and Output Information.");
    spdlog::info("Figuring Out Protocol and Output Information.");
    const int totalOutputGroups = GetNumberOfOutputGroups();
    const int outputPerGroups = GetOutputsPerGroup();

    //loop to setup protocol setting and string outputs
    for (int i = 1; i <= totalOutputGroups; i++) {
        bool first = true;
        SanDevicesOutputV4* newPort = FindPortDataV4(i);
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerPixelPort(outputNumber);
                if (first) {
                    UpdatePortDataV4(newPort, port);
                    first = false;
                }

                const char protocol = EncodeStringPortProtocolV4(port->GetProtocol());

                if (newPort->protocol != protocol) {
                    spdlog::warn("SanDevices Outputs Upload: All The Protocols must be the same across a Output Group. Check Port {}-1 to {}-4", i, i);
                    ui->ShowMessage(std::format("All The Protocols must be the same across a Output Group. Check Port {}-1 to {}-4", i, i), "Error");
                    ui->EndProgress(progressTk);
                    return false;
                }
                if (newPort->pixels != port->Pixels()) {
                    spdlog::warn("SanDevices Outputs Upload: All The Pixel Lengths must be the same across a Output Group. Check Port {}-1 to {}-4", i, i);
                    ui->ShowMessage(std::format("All The Pixel Lengths must be the same across a Output Group. Check Port {}-1 to {}-4", i, i), "Error");
                    ui->EndProgress(progressTk);
                    return false;
                }
                UpdateSubPortDataV4(newPort, j, port);
            }
        }
        if (first) {
            newPort->outputSize = 0;
            newPort->upload = true;
        }
    }
    int p = 60;
    spdlog::info("Sending Output Data to Controller.");
    //spam the controller with web requests
    for (const auto& outputD : _outputDataV4) {
        ui->UpdateProgress(progressTk, p, "Sending Output Data to Controller.");
        outputD->Dump();
        if (outputD->upload) {
            const std::string url = GenerateOutputURLV4(outputD);
            SDGetURL(url);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        p += 10;
    }
    ui->EndProgress(progressTk);
    return true;
}

bool SanDevices::SetOutputsV5(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {

    auto progressTk = ui->BeginProgress("Uploading ...", 100);

    //bool success = true;

    spdlog::debug("SanDevices Outputs Upload: Uploading to {}", _ip);

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    ui->UpdateProgress(progressTk, 0, "Scanning models");
    spdlog::info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    //first check rules
    auto rules = ControllerCaps::GetControllerConfig(controller);
    const bool success = cud.Check(rules, check);

    spdlog::debug(check);

    cud.Dump();
    if (!success) {
        ui->ShowMessage("SanDevices Upload Error:\n" + check, "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }
    //get current config Page
    const std::string page = _page;

    if (page.empty()) {
        ui->EndProgress(progressTk);
        return false;
    }

    ui->UpdateProgress(progressTk, 10, "Reading Protocol Data from Controller.");
    spdlog::info("Reading Protocol Data from Controller.");

    _connected = ParseV5MainWebpage(_page);
    if (!_connected) {
        ui->ShowMessage("Unable to Parse Main Webpage.", "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    const int totalOutputGroups = GetNumberOfOutputGroups();
    const int outputPerGroups = GetOutputsPerGroup();

    auto getControllerPort = [&cud](int output, bool isSerial) {
            if (isSerial) {
                return cud.GetControllerSerialPort(output);
            }
            return cud.GetControllerPixelPort(output);
        };

    ui->UpdateProgress(progressTk, 20, "Figuring Out Protocol Information.");
    spdlog::info("Figuring Out Protocol Information.");
    //loop to setup protocol setting
    for (int i = 1; i <= totalOutputGroups; i++) {
        SanDevicesProtocol* firstPort = nullptr;
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber) || cud.HasSerialPort(outputNumber)) {
                if (cud.HasPixelPort(outputNumber) && cud.HasSerialPort(outputNumber)) {
                    spdlog::warn("SanDevices Outputs Upload: Serial and Pixel Port on same output are Currently used, this in not Valid. Check Port {}-1 to {}-4", i, i);
                    ui->ShowMessage(std::format("SanDevices Outputs Upload: Serial and Pixel Port on same output are Currently used, this in not Valid. Check Port {}-1 to {}-4", i, i), "Error");
                    ui->EndProgress(progressTk);
                    return false;
                }

                // Check if the port is a serial port or pixel port
                UDControllerPort* port = getControllerPort(outputNumber, cud.HasSerialPort(outputNumber));
                if (firstPort == nullptr) {
                    firstPort = new SanDevicesProtocol(i, EncodeStringPortProtocolV5(port->GetProtocol()));
                }
                SanDevicesProtocol* newPort = new SanDevicesProtocol(i, EncodeStringPortProtocolV5(port->GetProtocol()));
                if (newPort->getProtocol() != firstPort->getProtocol()) {
                    delete newPort;
                    spdlog::warn("SanDevices Outputs Upload: All The Protocols must be the same across a Output Group. Check Port {}-1 to {}-4", i, i);
                    ui->ShowMessage(std::format("All The Protocols must be the same across a Output Group. Check Port {}-1 to {}-4", i, i), "Error");
                    ui->EndProgress(progressTk);
                    return false;
                }
                delete newPort;
                if (_protocolData[i - 1]->getProtocol() != firstPort->getProtocol()) {
                    _protocolData[i - 1]->setProtocol(firstPort->getProtocol());
                }
            }
        }
    }

    ui->UpdateProgress(progressTk, 30, "Sending Protocol Data to Controller.");
    spdlog::info("Sending Protocol Data to Controller.");

    for (const auto& proro : _protocolData) {
        proro->Dump();
        if (proro->shouldUpload()) {
            const std::string url = GenerateProtocolURLV5(proro);
            SDGetURL(url);
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
    }

    ui->UpdateProgress(progressTk, 50, "Getting Output Data from Controller.");
    spdlog::info("Getting Output Data from Controller.");

    const std::string page2 = SDGetURL("/H?");
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    if (page2.empty()) {
        spdlog::error("SanDevices Outputs Upload: SanDevices would not return current configuration.");
        ui->ShowMessage("Error occurred trying to upload to SanDevices.", "Error");
        ui->EndProgress(progressTk);
        return false;
    }

    _connected = ParseV5OutputWebpage(page2);
    if (!_connected) {
        ui->ShowMessage("Unable to Parse Main Webpage.", "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    ui->UpdateProgress(progressTk, 60, "Figuring Out Output Information.");
    spdlog::info("Figuring Out Output Information.");
    //loop to setup string outputs
    for (int i = 1; i <= totalOutputGroups; i++) {
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerPixelPort(outputNumber);
                UpdatePortDataV5(i, j, port, false);
            } else if(cud.HasSerialPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerSerialPort(outputNumber);
                UpdatePortDataV5(i, j, port, true);
            } else {
                SanDevicesOutput* sd = FindPortDataV5(i, j);
                if (sd != nullptr) {
                    sd->pixels = 0;
                    sd->upload = true;
                }
            }
        }
    }
    spdlog::info("Sending Output Data to Controller.");
    int p = 66;
    //spam the controller with web requests
    for (const auto& outputD : _outputData) {
        p += 2;
        ui->UpdateProgress(progressTk, p, "Sending Output Data to Controller.");
        outputD->Dump();
        if (outputD->upload) {
            const std::string url = GenerateOutputURLV5(outputD);
            SDGetURL(url);
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
    }
    ui->EndProgress(progressTk);
    return true;
}

bool SanDevices::ParseV4Webpage(const std::string& page) {
    // 1-1 thru 1-4
    _outputDataV4.clear();
    _universes.clear();

    int fieldStart = 65; // char of 'A'
    const std::string p(page);
    const int start = p.find(" Universe");
    for (int i = 0; i < 12; i++) {
        // extract the universes
        const int univers = ExtractIntFromPage(page, std::format("{:c}", fieldStart++), "input", 1, start);
        _universes.push_back(univers);
    }

    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        SanDevicesOutputV4* group = ExtractOutputDataV4(page, i);
        group->Dump();
        _outputDataV4.push_back(group);
    }
    if (_outputDataV4.size() == 4 && _universes.size() == 12) {
        return true;
    }
    assert(false);
    return true;
}

bool SanDevices::ParseV5MainWebpage(const std::string& page) {
    // 1-1 thru 1-4
    _protocolData.clear();
    _universes.clear();

    int fieldStart = 65; // char of 'A'
    const std::string p(page);
    const int start = p.find(" Universe");
    for (int i = 0; i < 12; i++) {
        // extract the universes
        const int univers = ExtractIntFromPage(page, std::format("{:c}", fieldStart++), "input", 1, start);
        _universes.push_back(univers);
    }

    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        SanDevicesProtocol* group = ExtractProtocalDataV5(page, i);
        group->Dump();
        _protocolData.push_back(group);
    }
    if (_protocolData.size() == 4 && _universes.size() == 12) {
        return true;
    }
    assert(false);
    return true;
}

bool SanDevices::ParseV5OutputWebpage(const std::string& page) {
    _outputData.clear();
    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        for (int j = 1; j <= GetOutputsPerGroup(); j++) {
            SanDevicesOutput* group = ExtractOutputDataV5(page, i, j);
            group->Dump();
            _outputData.push_back(group);
        }
    }
    if ((int)_outputData.size() == GetMaxStringOutputs()) {
        return true;
    }
    assert(false);
    return true;
}

std::string SanDevices::SDGetURL(const std::string& url, bool logresult) {
    std::string res = GetURL(url);
    spdlog::debug("Making request to SanDevices {} '{}'", _ip, url);

    if (res.empty()) {
        spdlog::error("Unable to connect to SanDevices {} '{}'.", _ip, url);
        DisplayError(std::format("Unable to connect to SanDevices {}.", _ip));
    } else if (logresult) {
        spdlog::debug("Response from SanDevices '{}'.", res);
    }

    return res;
}

void SanDevices::ResetStringOutputs() {

    SDGetURL("/4?A=0");
    SDGetURL("/5?A=0");
    SDGetURL("/6?A=0");
    SDGetURL("/7?A=0");
}

inline int SanDevices::GetOutputsPerGroup() const {
    if (IsE682()) { return 4; }
    return 1;
}

std::string SanDevices::ExtractFromPage(const std::string& page, const std::string& parameter, const std::string& type, int start) {

    const std::string p = page.substr(start);
    if (type == "input") {
        std::regex inputregex("(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')([0-9\\.]*?)\\'");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[2].str();
        }
    }
    else if (type == "inputText") {
        std::regex inputregex("(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')(\\w+)(\\s+)?\\'");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[2].str();
        }
    }
    else if (type == "select") {
        std::regex inputregex("(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z0-9])\\'selected");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[2].str();
        }
    }
    else if (type == "selectLetter") {
        std::regex inputregex("(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z])\\'selected");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            return m[2].str();
        }
    }
    else if (type == "checkbox") {
        std::regex inputregex("(\\<input type=\\'checkbox\\' name=\\'" + parameter + "\\' value=\\'[0-9]\\')([^\\>]*)\\>");
        std::smatch m;
        if (std::regex_search(p, m, inputregex)) {
            std::string res = m[2].str();
            auto start_it = std::find_if_not(res.begin(), res.end(), ::isspace);
            auto end_it = std::find_if_not(res.rbegin(), res.rend(), ::isspace).base();
            res = (start_it < end_it) ? std::string(start_it, end_it) : "";
            if (res == "checked") {
                return "1";
            }
            return "0";
        }
    }

    return "";
}

int SanDevices::ExtractIntFromPage(const std::string& page, const std::string& parameter, const std::string& type, int defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return (int)strtol(value.c_str(), nullptr, 10);
    }
    return defaultValue;
}

char SanDevices::ExtractCharFromPage(const std::string& page, const std::string& parameter, const std::string& type, char defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return value[0];
    }
    return defaultValue;
}

bool SanDevices::ExtractBoolFromPage(const std::string& page, const std::string& parameter, const std::string& type, bool defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return value == "1";
    }
    return defaultValue;
}

SanDevicesProtocol* SanDevices::ExtractProtocalDataV5(const std::string& page, int group) {

    const std::string p(page);
    int start = p.find("Output Group Configuration:");

    const std::string tofind = "<td>" + std::format("{}-1 thru {}-4", group, group) + "</td>";

    start = p.find(tofind, start);

    // extract the Protocol
    const char proto = ExtractCharFromPage(page, "E", "select", 'A', start);

    // extract the timing
    const char timing = ExtractCharFromPage(page, "K", "select", 'A', start);

    return new SanDevicesProtocol(group, proto, timing);
}

SanDevicesOutput* SanDevices::ExtractOutputDataV5(const std::string& page, int group, int port) {

    const std::string p(page);
    std::string tofind;

    if (IsE682()) {
        tofind = "<td>" + std::format("{}-{}", group, port) + "</td>";
    }
    else {
        tofind = "<td>" + std::to_string(group) + "</td>";
    }
    const int start = p.find(tofind);

    SanDevicesOutput* output = new SanDevicesOutput(group, port, EncodeXlightsOutput(group, port));

    // extract the pixel
    output->pixels = ExtractIntFromPage(page, "A", "input", 0, start);

    output->colorOrder = ExtractCharFromPage(page, "E", "select", 'A', start);
    output->universe = ExtractCharFromPage(page, "Z", "select", 'A', start);

    output->startChannel = ExtractIntFromPage(page, "G", "input", 1, start);

    output->reverse = ExtractBoolFromPage(page, "N", "checkbox", false, start);
    output->nullPixel = ExtractIntFromPage(page, "H", "input", 0, start);

    output->groupCount = ExtractIntFromPage(page, "B", "input", 1, start);

    output->chase = ExtractBoolFromPage(page, "F", "checkbox", false, start);
    output->firstZig = ExtractIntFromPage(page, "I", "input", 0, start);
    output->thenEvery = ExtractIntFromPage(page, "J", "input", 0, start);

    output->brightness = ExtractCharFromPage(page, "D", "selectLetter", 'A', start);

    return output;
}

SanDevicesOutputV4* SanDevices::ExtractOutputDataV4(const std::string& page, int group) {

    const std::string p(page);
    int start = p.find("Output Configuration:");

    std::string tofind;

    if (IsE682()) {
        tofind = "<td>" + std::format("{}-1 to {}-4", group, group) + "</td>";
    }
    else {
        tofind = "<td>" + std::to_string(group) + "</td>";
    }
    start = p.find(tofind, start);

    SanDevicesOutputV4* output = new SanDevicesOutputV4(group);

    if (IsE682()) {
        output->outputSize = ExtractIntFromPage(page, "A", "input", 0, start);
    }
    else {
        output->outputSize = ExtractIntFromPage(page, "A", "checkbox", 0, start);
    }

    output->protocol = ExtractCharFromPage(page, "B", "select", 'A', start);

    // extract the pixel
    output->pixels = ExtractIntFromPage(page, "C", "input", 0, start);
    output->groupCount = ExtractIntFromPage(page, "D", "input", 1, start);

    output->colorOrder = ExtractIntFromPage(page, "E", "select", 0, start);
    output->universe = ExtractCharFromPage(page, "F", "select", 'A', start);

    output->startChannel = ExtractIntFromPage(page, "G", "input", 1, start);
    if (IsE682()) {
        output->reverse[0] = ExtractBoolFromPage(page, "H", "checkbox", false, start);
        output->reverse[1] = ExtractBoolFromPage(page, "I", "checkbox", false, start);
        output->reverse[2] = ExtractBoolFromPage(page, "J", "checkbox", false, start);
        output->reverse[3] = ExtractBoolFromPage(page, "K", "checkbox", false, start);
    }
    else {
        output->reverse[0] = ExtractIntFromPage(page, "H", "checkbox", 0, start);
    }
    output->zigzag = ExtractIntFromPage(page, "L", "input", 0, start);
    if (IsE682()) {
        output->nullPixel[0] = ExtractIntFromPage(page, "M", "input", 0, start);
        output->nullPixel[1] = ExtractIntFromPage(page, "N", "input", 0, start);
        output->nullPixel[2] = ExtractIntFromPage(page, "O", "input", 0, start);
        output->nullPixel[3] = ExtractIntFromPage(page, "P", "input", 0, start);
    }
    else {
        output->nullPixel[0] = ExtractIntFromPage(page, "M", "checkbox", 0, start);
    }

    return output;
}

void SanDevices::UpdatePortDataV5(int group, int output, UDControllerPort* stringData, bool serial) {
    SanDevicesOutput* sd = FindPortDataV5(group, output);
    if (sd != nullptr) {
        if (stringData->GetPort() != sd->stringport) {
            spdlog::warn("SanDevices Outputs Upload: Error {} outputs. Attempt to upload to output {}.", stringData->GetPort(), sd->stringport);
        }
        const char newUniv = EncodeUniverse(stringData->GetUniverse());
        if (newUniv != sd->universe) {
            sd->upload = true;
        }
        sd->universe = newUniv;

        const int startChan = stringData->GetUniverseStartChannel();
        if (startChan != sd->startChannel) {
            sd->upload = true;
        }
        sd->startChannel = startChan;

        const int pixels = serial ? stringData->Channels() : stringData->Pixels();
        if (pixels != sd->pixels) {
            sd->upload = true;
        }
        sd->pixels = pixels;

        const int nullPix = stringData->GetFirstModel()->GetStartNullPixels(-1);
        if (nullPix != -1) {
            if (nullPix != sd->nullPixel) {
                sd->upload = true;
            }
            sd->nullPixel = nullPix;
        }
        const int groupCount = stringData->GetFirstModel()->GetGroupCount(-1);
        if (groupCount != -1) {
            if (groupCount != sd->groupCount) {
                sd->upload = true;
            }
            sd->groupCount = groupCount;
        }
        const int brightness = stringData->GetFirstModel()->GetBrightness(-1);
        if (brightness != -1) {
            const char brightn = EncodeBrightness(brightness);
            sd->upload = true;
            sd->brightness = brightn;
        }

        const std::string colorOrder = stringData->GetFirstModel()->GetColourOrder("unknown");
        if (colorOrder != "unknown") {
            const char co = EncodeColorOrderV5(colorOrder);
            if (co != sd->colorOrder) {
                sd->upload = true;
            }
            sd->colorOrder = co;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown") {
            const bool rev = EncodeDirection(direction);
            if (rev != sd->reverse) {
                sd->upload = true;
            }
            sd->reverse = rev;
        }

        if (serial != sd->serial)
        {
            sd->upload = true;
            sd->serial = serial;
        }
    }
}

SanDevicesOutput* SanDevices::FindPortDataV5(int group, int output) {
    for (const auto& sd : _outputData) {
        if (sd->group == group && sd->output == output) {
            return sd;
        }
    }
    assert(false);
    return nullptr;
}

void SanDevices::UpdatePortDataV4(SanDevicesOutputV4* pd, UDControllerPort* stringData) const {

    if (pd != nullptr) {
        const char protocol = EncodeStringPortProtocolV4(stringData->GetProtocol());
        if (protocol != pd->protocol) {
            pd->upload = true;
        }
        pd->protocol = protocol;

        const char newUniv = EncodeUniverse(stringData->GetUniverse());
        if (newUniv != pd->universe) {
            pd->upload = true;
        }
        pd->universe = newUniv;

        const int startChan = stringData->GetUniverseStartChannel();
        if (startChan != pd->startChannel) {
            pd->upload = true;
        }
        pd->startChannel = startChan;

        const int pixels = stringData->Pixels();
        if (pixels != pd->pixels) {
            pd->upload = true;
        }
        pd->pixels = pixels;

        const int groupCount = stringData->GetFirstModel()->GetGroupCount(-1);
        if (groupCount != -1) {
            if (groupCount != pd->groupCount) {
               pd->upload = true;
            }
            pd->groupCount = groupCount;
        }

        const std::string colorOrder = stringData->GetFirstModel()->GetColourOrder("unknown");
        if (colorOrder != "unknown") {
            const char co = EncodeColorOrderV4(colorOrder);
            if (co != pd->colorOrder) {
                pd->upload = true;
            }
            pd->colorOrder = co;
        }
    }
}

void SanDevices::UpdateSubPortDataV4(SanDevicesOutputV4* pd, int subport, UDControllerPort* stringData) const {

    if (pd != nullptr) {
        if (subport != pd->outputSize) {
            pd->upload = true;
        }
        pd->outputSize = subport;

        const int nullPix = stringData->GetFirstModel()->GetStartNullPixels(-1);
        if (nullPix != -1) {
            if (nullPix != pd->nullPixel[subport - 1]) {
                pd->upload = true;
            }
            pd->nullPixel[subport - 1] = nullPix;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown") {
            const bool rev = EncodeDirection(direction);
            if (rev != pd->reverse[subport - 1]) {
                pd->upload = true;
            }
            pd->reverse[subport - 1] = rev;
        }
    }
}

SanDevicesOutputV4* SanDevices::FindPortDataV4(int group) {

    for (const auto& sd : _outputDataV4) {
        if (sd->group == group) {
            return sd;
        }
    }
    assert(false);
    return nullptr;
}

std::string SanDevices::GenerateOutputURLV5(SanDevicesOutput* outputData) {
    // extract reverse
    std::string rev;
    if (outputData->reverse) { // if check add to request, based on my testing firmware will check if present regardless of value
        rev = "&N=1";
    }

    // extract null pixels
    std::string null;
    if (outputData->nullPixel != 0) { // Only Add to Request if it currently exists
        null = std::format("&H={}", outputData->nullPixel);
    }
    // extract chase
    std::string chase;
    if (outputData->chase) { // if check add to request, based on my testing firmware will check if present regardless of value
        chase = "&F=1";
    } else {
        chase = "";
    }
    std::string colorOrder;
    if (!outputData->serial) {
        colorOrder = std::format("&E={:c}", outputData->colorOrder);
    }

    const int controlPort = EncodeControllerPortV5(outputData->group, outputData->output);

    //http://192.168.1.206/K?A=50&E=A&Z=A&G=1&H=0&B=1&I=0&J=0&D=A
    const std::string request = std::format("/%c?A=%d%s&Z=%c&G=%d%s%s&B=%i%s&I=%i&J=%i&D=%c",
        controlPort + 'J',
        outputData->pixels,
        colorOrder,
        outputData->universe,
        outputData->startChannel,
        rev,
        null,
        outputData->groupCount,
        chase,
        outputData->firstZig,
        outputData->thenEvery,
        outputData->brightness);
    return request;
}

std::string SanDevices::GenerateProtocolURLV5(SanDevicesProtocol* protocolData) {

    //K?E=A&K=A
    //http://192.168.1.206/K?A=50&E=A&Z=A&G=1&H=0&B=1&I=0&J=0&D=A
    const std::string request = std::format("/%c?E=%c&K=%c",
        protocolData->getGroup() + 'J',
        protocolData->getProtocol(),
        protocolData->getTiming());
    return request;
}

std::string SanDevices::GenerateOutputURLV4(SanDevicesOutputV4* outputData) {

    std::string output;
    if (outputData->outputSize != 0) {
        output = std::format("A={}&", outputData->outputSize);
    }

    // extract reverse
    std::string rev;
    if (outputData->reverse[0]) {
        rev += "&H=1";
    }
    if (IsE682()) {
        if (outputData->reverse[1]) { rev += "&I=1"; }
        if (outputData->reverse[2]) { rev += "&J=1"; }
        if (outputData->reverse[3]) { rev += "&K=1"; }
    }

    // extract null pixels
    std::string null;
    if (outputData->nullPixel[0]) {
        rev += "&M=1";
    }
    else {
        rev += "&M=0";
    }
    if (IsE682()) {
        if (outputData->nullPixel[1]) {
            rev += "&N=1";
        }
        else {
            rev += "&N=0";
        }
        if (outputData->nullPixel[2]) {
            rev += "&O=1";
        }
        else {
            rev += "&O=0";
        }
        if (outputData->nullPixel[3]) {
            rev += "&P=1";
        }
        else {
            rev += "&P=0";
        }
    }

    //e682 v4
    //http://192.168.1.206/4?A=2&B=B&C=100&D=1&E=0&F=A&G=1&L=0&M=0&N=0&O=0&P=0
    const std::string request = std::format("/%d?%sB=%c&C=%d&D=%i&E=%i&F=%c&G=%d%s&L=%i%s",
        outputData->group + 3,
        output,
        outputData->protocol,
        outputData->pixels,
        outputData->groupCount,
        outputData->colorOrder,
        outputData->universe,
        outputData->startChannel,
        null,
        outputData->zigzag,
        rev);
    return request;
}
#pragma endregion

#pragma region Encode and Decode
char SanDevices::EncodeStringPortProtocolV4(const std::string& protocol) const {

    std::string const p = Lower(protocol);
    if (p == "ws2811")  { return 'D'; }
    if (p == "tm18xx")  { return 'D'; }
    if (p == "ws2801")  { return 'B'; }
    if (p == "tls3001") { return 'I'; }
    if (p == "lpd6803") { return 'A'; }
    if (p == "gece")    { return 'C'; }
    if (p == "dmx")     { return 'K'; }
    if (p == "renard")  { return 'L'; }
    assert(false);
    return 'D';
}

char SanDevices::EncodeStringPortProtocolV5(const std::string& protocol) const {

    std::string const p = Lower(protocol);
    if (p == "ws2811")  { return 'A'; }
    if (p == "tm18xx")  { return 'D'; }
    if (p == "ws2801")  { return 'C'; }
    if (p == "lpd6803") { return 'B'; }
    if (p == "apa102")  { return 'G'; }
    if (p == "lpd8806") { return 'E'; }
    if (p == "gece")    { return 'O'; }
    if (p == "dmx")     { return 'J'; }
    if (p == "renard")  { return 'M'; }
    assert(false);
    return 'A';
}

char SanDevices::EncodeSerialPortProtocolV5(const std::string& protocol) const {

    std::string const p = Lower(protocol);

    if (p == "dmx")    { return 'J'; }
    if (p == "renard") { return 'M'; }
    assert(false);
    return 'J';
}

char SanDevices::EncodeUniverseSize(int universesize) const {

    if (universesize == 510) { return 'A'; }
    if (universesize == 512) { return 'B'; }
    assert(false);
    
    spdlog::debug("SanDevices DecodeUniverseSize Upload: Invalid Universe Size {}", universesize);
    return 'A';
}

int SanDevices::EncodeColorOrderV4(const std::string& colorOrder) const {

    std::string const c = Lower(colorOrder);

    if (c == "rgb") { return 0; }
    if (c == "rbg") { return 1; }
    if (c == "grb") { return 2; }
    if (c == "gbr") { return 3; }
    if (c == "brg") { return 4; }
    if (c == "bgr") { return 5; }
    assert(false);
    return 0;
}

char SanDevices::EncodeColorOrderV5(const std::string& colorOrder) const {

    std::string const c = Lower(colorOrder);

    if (c == "rgb") { return 'A'; }
    if (c == "rbg") { return 'B'; }
    if (c == "grb") { return 'C'; }
    if (c == "gbr") { return 'D'; }
    if (c == "brg") { return 'E'; }
    if (c == "bgr") { return 'F'; }
    assert(false);
    return 'A';
}

char SanDevices::EncodeBrightness(int brightness) const {

    if (brightness < 19) { return 'D'; }
    if (brightness < 38) { return 'C'; }
    if (brightness < 75) { return 'B'; }
    return 'A';
}

char SanDevices::EncoderRenardSpeed(int baud) const {
    if (baud == 57600) {
        return 'A';
    }
    if (baud == 115200) {
        return 'B';
    }
    if (baud == 230400) {
        return 'C';
    }
    return 'A';
}

inline bool SanDevices::EncodeDirection(const std::string& direction) const {

    return direction == "Reverse";
}

char SanDevices::EncodeUniverse(int universe) const {

    char res = 'A';

    for (const auto& uni : _universes) {
        if (uni == universe) {
            return res;
        }
        ++res;
    }
    return res;
}

SanDevices::SanDeviceModel SanDevices::DecodeControllerType(const std::string& modelName) const {
    if (modelName == "E680") {
        return SanDeviceModel::E680;
    }
    if (modelName == "E681") {
        return SanDeviceModel::E681;
    }
    if (modelName == "E682") {
        return SanDeviceModel::E682;
    }
    if (modelName == "E6804") {
        return SanDeviceModel::E6804;
    }
    assert(false);
    return SanDeviceModel::Unknown;
}

inline std::pair<int, int > SanDevices::DecodeOutputPort(const int output) const {

    return { ((output - 1) / 4) + 1, ((output - 1) % 4) + 1 };
}

inline int SanDevices::EncodeXlightsOutput(const int group, const int subport) const {

    return ((group - 1) * GetOutputsPerGroup()) + subport;
}

inline int SanDevices::EncodeControllerPortV5(const int group, const int subport) const {

    return ((group - 1) * 4) + subport;
}
#pragma endregion

#pragma region Constructors and Destructors
SanDevices::SanDevices(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {
    _firmware = FirmwareVersion::Unknown;

    //Loop For Version 5, we may have to switch web pages first and then scrap data
    for (int i = 0; i < 2; i++) {
        _page = SDGetURL("/");
        if (!_page.empty()) {
            _connected = true;
            static std::regex modelregex("(Controller Model )(E\\d+)");
            std::smatch modm;
            if (std::regex_search(_page, modm, modelregex)) {
                _sdmodel = DecodeControllerType(modm[2].str());
                spdlog::info("Connected to SanDevices controller model {}.", GetModel());
            }
            static std::regex versionregex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>\\<\\/td\\>\\<td\\>)([0-9]+\\.[0-9]+)\\<\\/td\\>");
            std::smatch verm;
            if (std::regex_search(_page, verm, versionregex)) {
                _firmware = FirmwareVersion::Four;
                _version = verm[2].str();
                spdlog::info("                                 firmware {}.", static_cast<int>(_firmware));
                spdlog::info("                                 version {}.", _version);
                break;
            }
            // Firmware Version:</th></td><td>5.038</td>
            // Firmware Version:</th></td><td> 5.051-W5200</td>
            static std::regex version5regex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>)\\s?([0-9]+\\.[0-9]+)(-W5200)?\\<\\/td\\>");
            std::smatch ver5m;
            if (std::regex_search(_page, ver5m, version5regex)) {
                _firmware = FirmwareVersion::Five;
                _version = ver5m[2].str();
                if (ver5m[3].matched) {
                    _version += ver5m[3].str();
                }
                spdlog::info("                                 firmware {}.", static_cast<int>(_firmware));
                spdlog::info("                                 version {}.", _version);
                break;
            }

            //Switch Pages from Version 5 Firmware
            SDGetURL("/H?");
        }
        else {
            _connected = false;
            spdlog::error("Error connecting to SanDevices controller on {}.", _ip);
            break;
        }
    }

    if (_connected) {
        _model = std::format("E{}", static_cast<int>(_sdmodel));
    }
}

SanDevices::~SanDevices() {
    for (const auto& it : _outputData) {
        delete it;
    }
    _outputData.clear();

    for (const auto& it : _protocolData) {
        delete it;
    }
    _protocolData.clear();

    for (const auto& it : _outputDataV4) {
        delete it;
    }
    _outputDataV4.clear();
}
#pragma endregion

#pragma region Getters and Setters
bool SanDevices::SetInputUniverses(Controller* controller, UICallbacks* ui) {
    
    const std::string page = _page;

    if (page.empty()) {
        return false;
    }

    // Check Correct Page from Version 5 Firmware
    if (IsFirmware5()) {
        if (page.find("<H3>System Information: </H3>") == std::string::npos) {
            spdlog::error("SanDevices Outputs Upload: SanDevices wouldn't switch web pages.");
            return false;
        }
    }

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    if (outputs.size() > 12) {
        ui->ShowMessage(std::format("Attempt to upload {} universes to SanDevices controller but only 12 are supported.", outputs.size()), "Error");
        return false;
    }

    // dont make http requests if setting are already right
    bool upload {true};

    int t { -1 };

    auto const o = outputs.front();

    if (o->GetType() == OUTPUT_E131) {
        if (o->GetIP() == "MULTICAST") {
            t = 0;
        }
        else {
            t = 1;
        }
    }
    else if (o->GetType() == OUTPUT_ARTNET) {
        t = 2;
    }

    if ((t == 2 || t == 0) && outputs.size() > 7) {
        ui->ShowMessage(std::format("Attempt to upload {} universes to SanDevices controller but only 7 are supported in Multicast/Artnet Mode.", outputs.size()), "Error");
        return false;
    }

    // set the right input type
    std::string request = "/";
    if (IsFirmware5()) {
        request += "B?";
    }
    else {
        request += "1?";
    }

    request += "A=" + ExtractFromPage(page, "A", "input") +
        "&B=" + ExtractFromPage(page, "B", "input") +
        "&C=" + ExtractFromPage(page, "C", "input") +
        "&D=" + ExtractFromPage(page, "D", "input");

    if (IsFirmware5()) {
        request += "&N=" + ExtractFromPage(page, "N", "input") +
            "&O=" + ExtractFromPage(page, "O", "input") +
            "&P=" + ExtractFromPage(page, "P", "input") +
            "&Q=" + ExtractFromPage(page, "Q", "input") +
            "&I=++++++++++++++++";
        //"&I=" + ExtractFromPage(page, "I", "inputText");
        //I=+++++++++++++++++
        t += 65; //convert int to char
        const std::string currentReceiveMode = ExtractFromPage(page, "E", "select");
        const std::string newReceiveMode = std::format("{:c}", t);
        if (currentReceiveMode == newReceiveMode) {
            upload = false;
        }
        request += std::format("&E={:c}", t);
    }
    else {
        request += std::format("&E={}", t);
    }
    request += "&F=" + ExtractFromPage(page, "F", "input");

    if (IsFirmware5()) {
        request += "&H=" + ExtractFromPage(page, "H", "select");// += "&H=A"; //impossible to extract correctly because the are two fields called "H"
    }
    else {
        request += "&H=" + ExtractFromPage(page, "H", "input");
    }
    request += "&G=" + ExtractFromPage(page, "G", "input");

    if (IsFirmware5()) {
        request += "&J=" + ExtractFromPage(page, "J", "input") +
            "&K=" + ExtractFromPage(page, "K", "input");
    }

    if (upload) {
        SDGetURL(request);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    upload = true;

    request = "";
    std::string requestUnvSize = "/I?";
    int output { 65 };

    for (const auto& it : outputs)
    {
        if (!request.empty()) {
            request += "&";
            requestUnvSize += "&";
        }
        else {
            if (IsFirmware5()) {
                request = "/D?";
            }
            else {
                request = "/3?";
            }
        }
        if (IsFirmware5()) {
            if (it->GetChannels() != 510 && it->GetChannels() != 512) {
                ui->ShowMessage(std::format("Attempt to upload a universe of size {} to SanDevices controller, but only a size of 510/512 is supported", it->GetChannels()), "Error");
                return false;
            }
            requestUnvSize += std::format("%c=%c", output, EncodeUniverseSize(it->GetChannels()));
        }
        else {
            if (it->GetChannels() != 510) {
                ui->ShowMessage(std::format("Attempt to upload a universe of size {} to SanDevices controller, but only a size of 510 is supported in Firmware 4.", it->GetChannels()), "Error");
                return false;
            }
        }
        request += std::format("%c=%i", output++, it->GetUniverse());
    }

    if (0 == t) { //multicast
        if (output > 72) {
            spdlog::error("SanDevices Inputs Upload: More Than 7 Universes are assigned to One Controller, 7 is the MAX in Multicast Mode.");
            return false;
        }
    }
    else { //unicast or artnet
        if (output > 77) {
            spdlog::error("SanDevices Inputs Upload: More Than 12 Universes are assigned to One Controller, 12 is the MAX for Sandevices.");
            return false;
        }
    }

    if (IsFirmware5()) {
        SDGetURL(requestUnvSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    bool const passed = not SDGetURL(request).empty();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return passed;
}

bool SanDevices::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {

    if (IsFirmware5()) {
        return SetOutputsV5(allmodels, outputManager, controller, ui);
    }

    if (IsFirmware4()) {
        return SetOutputsV4(allmodels, outputManager, controller, ui);
    }
    assert(false);
    return false;
}
#pragma endregion
