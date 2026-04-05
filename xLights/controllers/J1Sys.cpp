
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "J1Sys.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "../utils/string_utils.h"
#include "ControllerUploadData.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"

#include <regex>

#include "../render/UICallbacks.h"

#include <cassert>
#include <cstdlib>
#include <format>
#include <log.h>

// This code has been tested with
// ECG-P12S App Version 3.3
// ECG-P2 App Version 2.9b

// P12R - 1 universe per output
//      - 0 serial ports
//      - can receive 12 universes  
// P12S/P12D - 1 universe per output on 3.3
//           - 2 universes per output on 3.4
//           - 2 serial ports
//           - can receive 26 universes

#pragma region Port Structures
struct J1SysPixelOutput
{
    char port;
    bool active = false;
    char protocol;
    int speed = -1;
    int universe = -1;
    int startChannel = -1;
    int pixels = -1;
};

struct J1SysSerialOutput
{
    char port;
    bool active = false;
    char protocol;
    int speed = -1;
    int universe = -1;
};
#pragma endregion

//static std::string J1SYS_P12S = "J1Sys-P12S";
//static std::string J1SYS_P12R = "J1Sys-P12R";
//static std::string J1SYS_P12D = "J1Sys-P12D";
//static std::string J1SYS_P2 = "J1Sys P2";
//
static std::string J1SYS_MODEL_P12S = "ECG-P12S";
static std::string J1SYS_MODEL_P12R = "ECG-P12R";
static std::string J1SYS_MODEL_P12D = "ECG-P12D";

#pragma region Encode and Decode
char J1Sys::EncodeStringPortProtocol(std::string protocol) const {
    const std::string p = Lower(protocol);

    if (p == "ws2811") return '4';
    if (p == "tm180x") return '2';
    if (p == "tm18xx") return '2';
    if (p == "ws2801") return '1';
    if (p == "tls3001") return '3';
    if (p == "lpd6803") return '0';

    return -1;
}

char J1Sys::EncodeSerialPortProtocol(std::string protocol) const {
    const std::string p = Lower(protocol);

    if (p == "dmx") return 'D';
    if (p == "renard") return 'R';

    return -1;
}

int J1Sys::DecodeProtocolSpeed(std::string protocol) const {
    const std::string p = Lower(protocol);

    if (p == "ws2811") return 3200;
    if (p == "ws2801") return 750;
    if (p == "dmx") return 5;
    if (p == "renard") return 3;
    return 0;
}
#pragma endregion

#pragma region String Port Handling
std::string J1Sys::BuildStringPort(bool active, int string, char protocol, int speed, int startChannel, int universe, int pixels) const {

    

    int out = 65 + string;

    spdlog::debug("     Output String {}, Protocol {} Universe {} StartChannel {} Pixels {}",
        string, protocol, universe, startChannel, pixels);

    return std::format("sA{:c}={}&sT{:c}={:c}&sB{:c}={}&sU{:c}={}&sS{:c}={}&sC{:c}={}",
        out, active ? 1 : 0,
        out, protocol,
        out, speed,
        out, universe,
        out, startChannel,
        out, pixels);
}

void J1Sys::ResetStringOutputs() {

    PutURL("/protect/stringConfig.htm", "", _username, _password);
}

void J1Sys::ReadCurrentConfig(std::vector<J1SysPixelOutput>& j) {

    std::string config = GetURL("/protect/stringConfig.htm");

    if (!config.empty()) {
        for (auto i = 0; i < (int)j.size(); i++) {
            j[i].port = i;
            std::regex ar(std::format("sA{:c}[^>]*checked", (char)(i + 65)));
            j[i].active = std::regex_search(config, ar);

            if (i % GetBankSize() == 0) {
                std::regex pr(std::format("sT{:c}>[^#]*selected>([^<]*)", (char)(i + 65)));
                std::smatch pm;
                if (std::regex_search(config, pm, pr)) {
                    j[i].protocol = EncodeStringPortProtocol(pm[1].str());
                }
                std::regex sr(std::format("sB{:c}[^>]*value=\"([^\"]*)\"", (char)(i + 65)));
                std::smatch sm;
                if (std::regex_search(config, sm, sr)) {
                    j[i].speed = (int)std::strtol(sm[1].str().c_str(), nullptr, 10);
                }
            }
            else {
                j[i].protocol = j[i / GetBankSize() * GetBankSize()].protocol;
                j[i].speed = j[i / GetBankSize() * GetBankSize()].speed;
            }

            std::regex ur(std::format("sU{:c}[^>]*value=\"([0-9]*)", (char)(i + 65)));
            std::smatch um;
            if (std::regex_search(config, um, ur)) {
                j[i].universe = (int)std::strtol(um[1].str().c_str(), nullptr, 10);
            }
            std::regex scr(std::format("sS{:c}[^>]*value=\"([0-9]*)", (char)(i + 65)));
            std::smatch scm;
            if (std::regex_search(config, scm, scr)) {
                j[i].startChannel = (int)std::strtol(scm[1].str().c_str(), nullptr, 10);
            }
            std::regex pxr(std::format("sC{:c}[^>]*value=\"([0-9]*)", (char)(i + 65)));
            std::smatch pxm;
            if (std::regex_search(config, pxm, pxr)) {
                j[i].pixels = (int)std::strtol(pxm[1].str().c_str(), nullptr, 10);
            }
        }
    }
}

void J1Sys::DumpConfig(const std::vector<J1SysPixelOutput>& outputs) const {

    
    for (const auto& j : outputs) {
        spdlog::debug("   Port {}, Active: {}, Protocol: {}, Speed {}, Universe {}, StartChannel {}, Pixels {}", j.port + 65, j.active, j.protocol, j.speed, j.universe, j.startChannel, j.pixels);
    }
}

int J1Sys::GetBankSize() const {

    if (_outputs == 2) return 4;
    return 1;
}
#pragma endregion

#pragma region Serial Port Handling
std::string J1Sys::BuildSerialPort(bool active, int port, char protocol, int speed, int universe) const {

    

    spdlog::debug("     Output Serial {}, Protocol {} Universe {}",
        port, protocol, universe);

    return std::format("pA{}={}&pP{}={:c}&pB{}={}&pU{}={}",
        port, active ? 1 : 0,
        port, protocol,
        port, speed,
        port, universe);
}

void J1Sys::ResetSerialOutputs() {

    if (_outputs == 12) {
        PutURL("/protect/portConfig.htm", "", _username, _password);
    }
}

void J1Sys::DumpConfig(const std::vector<J1SysSerialOutput>& outputs) const {

    
    for (const auto& j : outputs) {
        spdlog::debug("   Port {}, Active: {}, Protocol: {}, Speed {}, Universe {}", j.port + 65, j.active, j.protocol, j.speed, j.universe);
    }
}

void J1Sys::ReadCurrentSerialConfig(std::vector<J1SysSerialOutput>& j) {

    std::string config = GetURL("/protect/portConfig.htm");

    if (!config.empty()) {
        for (auto i = 0; i < (int)j.size(); i++) {
            j[i].port = i;
            std::regex ar(std::format("pA{}[^>]*checked", i + 1));
            j[i].active = std::regex_search(config, ar);

            std::regex pr(std::format("pP{}.+?value=\"(.)\" selected", i + 1));
            std::smatch pm;
            if (std::regex_search(config, pm, pr)) {
                j[i].protocol = pm[1].str()[0];
            }
            std::regex sr(std::format("pB{}.+?value=\"([0-9]+)\" selected", i + 1));
            std::smatch sm;
            if (std::regex_search(config, sm, sr)) {
                j[i].speed = (int)std::strtol(sm[1].str().c_str(), nullptr, 10);
            }

            std::regex ur(std::format("pU{}[^>]*value=\"([0-9]*)", i + 1));
            std::smatch um;
            if (std::regex_search(config, um, ur)) {
                j[i].universe = (int)std::strtol(um[1].str().c_str(), nullptr, 10);
            }
        }
    }
}
#pragma endregion

#pragma region Private Functions
void J1Sys::Reboot() {

    GetURL("/protect/reboot.htm?");
}
#pragma endregion

#pragma region Constructors and Destructors
J1Sys::J1Sys(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {

    
    _outputs = 0;

    spdlog::debug("J1Sys upload tested to work with:");
    spdlog::debug("    ECG-P2 App Version 2.9b");
    spdlog::debug("    ECG-P12S App Version 3.3");

    _connected = true;
    std::string page = GetURL("/sysinfo.htm");
    if (page != "") {
        static std::regex versionregex("(App Version:\\<\\/b\\>\\<\\/td\\>\\<td\\>.nbsp;\\<\\/td\\>\\<td\\>)([^\\<]*)\\<");
        std::smatch verm;
        if (std::regex_search(page, verm, versionregex)) {
            _version = verm[2].str();
            spdlog::debug("Connected to J1Sys controller version {}.", (const char*)_version.c_str());
        }
        static std::regex modelregex("(document\\.getElementById\\(.titleRight.\\)\\.innerHTML = .)([^\"]*)\"");
        std::smatch modm;
        if (std::regex_search(page, modm, modelregex)) {
            _model = modm[2].str();
            spdlog::debug("     model {}.", (const char*)_model.c_str());
            static std::regex outputsregex("([0-9]+)");
            std::smatch outm;
            if (std::regex_search(_model, outm, outputsregex)) {
                _outputs = (int)std::strtol(outm[1].str().c_str(), nullptr, 10);
                spdlog::debug("     outputs {}.", _outputs);
            }
        }
    }
    else {
        _connected = false;
        spdlog::error("Error connecting to J1Sys controller on {}.", (const char*)_ip.c_str());
    }
}
#pragma endregion

#pragma region Getters and Setters
bool J1Sys::SetInputUniverses(Controller* controller, OutputManager* outputManager) {

    assert(_outputs != 0);

    bool e131 = false;
    bool artnet = false;

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    for (const auto& it : outputs)
    {
        if (it->GetType() == OUTPUT_E131) {
            e131 = true;
        }
        else if (it->GetType() == OUTPUT_ARTNET) {
            artnet = true;
        }
    }

    if (_outputs == 2) {
        if (outputs.size() > 8) {
            DisplayError(std::format("Attempt to upload {} universes to j1Sys P2 controller but only 8 are supported.", (int)outputs.size()));
            return false;
        }
    }
    else if (_outputs == 12) {
        int maxUniverses = 12;
        if (_model != J1SYS_MODEL_P12R && std::strtod(_version.c_str(), nullptr) >= 3.4)
        {
            maxUniverses = 26;
        }
        if ((int)outputs.size() > maxUniverses)
        {
            DisplayError(std::format("Attempt to upload {} universes to j1Sys P12 controller but only {} are supported.", (int)outputs.size(), maxUniverses));
            return false;
        }
    }

    for (auto o : outputs)
    {
        if (o->GetChannels() > 510)
        {
            DisplayError(std::format("Attempt to upload universe {} to j1Sys controller of size {} but maximum is 510.", o->GetUniverse(), o->GetChannels()));
            return false;
        }
    }

    std::string request = std::format("an=0&e1en={}&anen={}", (e131) ? 1 : 0, (artnet) ? 1 : 0);
    std::string res = PutURL("/protect/ipConfig.htm", request, _username, _password);
    if (res != "" && !StartsWith(res, "401 "))
    {
        return true;
    }

    return false;
}

bool J1Sys::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui)
{
    if (!SetInputUniverses(controller, outputManager)) {
        return false;
    }

    //ResetStringOutputs(); // this shouldnt be used normally
    //ResetSerialOutputs(); // this shouldnt be used normally

    
    spdlog::debug("J1Sys Outputs Upload: Uploading to {}", (const char*)_ip.c_str());

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    auto caps = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(caps, check);

    cud.Dump();

    spdlog::debug(check);

    if (success && cud.GetMaxPixelPort() > 0) {
        // one per config row
        std::vector<J1SysPixelOutput> j1SysOutputs(_outputs * GetBankSize());

        ReadCurrentConfig(j1SysOutputs);
        spdlog::debug("Existing config:");
        DumpConfig(j1SysOutputs);

        for (int pp = 1; pp <= _outputs; pp++) {
            if (cud.HasPixelPort(pp)) {
                UDControllerPort* port = cud.GetControllerPixelPort(pp);
                long lastEnd = -1;
                int output = (pp - 1) * GetBankSize();
                int bankStart = (pp - 1) * GetBankSize();

                for (const auto& m : port->GetModels()) {
                    if (lastEnd != -1 && m->GetStartChannel() != lastEnd + 1) {
                        output++;
                        lastEnd = -1;
                    }

                    j1SysOutputs[output].protocol = EncodeStringPortProtocol(m->GetProtocol());
                    j1SysOutputs[output].speed = DecodeProtocolSpeed(m->GetProtocol());

                    if (lastEnd == -1) {
                        int channels = m->Channels();
                        while (channels > 0) {
                            if (output >= bankStart + GetBankSize()) {
                                ui->ShowMessage("Controller " + _ip + " too many outputs required for port " + std::to_string(pp) + ".", "Error");
                                spdlog::debug("Erroneous config:");
                                DumpConfig(j1SysOutputs);
                                return false;
                            }

                            j1SysOutputs[output].active = true;
                            for (int i = output % GetBankSize() + 1; i < GetBankSize(); i++) {
                                j1SysOutputs[bankStart + i].active = false;
                                j1SysOutputs[bankStart + i].universe = 0;
                                j1SysOutputs[bankStart + i].startChannel = 0;
                                j1SysOutputs[bankStart + i].pixels = 0;
                            }
                            int32_t sc;
                            auto o = outputManager->GetOutput(m->GetStartChannel() + m->Channels() - channels, sc);
                            j1SysOutputs[output].universe = o->GetUniverse();
                            j1SysOutputs[output].startChannel = sc;
                            if (channels < o->GetChannels() - sc + 1) {
                                j1SysOutputs[output].pixels = channels / 3;
                                channels = 0;
                            }
                            else {
                                j1SysOutputs[output].pixels = (o->GetChannels() - sc + 1) / 3;
                                channels -= o->GetChannels() - sc + 1;
                                output++;
                            }
                        }
                        lastEnd = m->GetEndChannel();
                    }
                    else {
                        int channels = m->Channels();
                        while (channels > 0) {
                            if (output >= bankStart + GetBankSize()) {
                                ui->ShowMessage("Controller " + _ip + " too many outputs required for port " + std::to_string(pp) + ".", "Error");
                                spdlog::debug("Erroneous config:");
                                DumpConfig(j1SysOutputs);
                                return false;
                            }

                            int32_t sc;
                            auto o = outputManager->GetOutput(m->GetStartChannel() + m->Channels() - channels, sc);
                            if (j1SysOutputs[output].universe == 0) {
                                j1SysOutputs[output].universe = o->GetUniverse();
                                j1SysOutputs[output].startChannel = sc;
                                j1SysOutputs[output].active = true;
                                for (int i = output % GetBankSize() + 1; i < GetBankSize(); i++) {
                                    j1SysOutputs[bankStart + i].active = false;
                                    j1SysOutputs[bankStart + i].universe = 0;
                                    j1SysOutputs[bankStart + i].startChannel = 0;
                                    j1SysOutputs[bankStart + i].pixels = 0;
                                }
                                j1SysOutputs[output].protocol = j1SysOutputs[(pp - 1) * GetBankSize()].protocol;
                                j1SysOutputs[output].speed = j1SysOutputs[(pp - 1) * GetBankSize()].speed;
                            }
                            if (channels < o->GetChannels() - j1SysOutputs[output].startChannel + 1 - j1SysOutputs[output].pixels * 3) {
                                j1SysOutputs[output].pixels += channels / 3;
                                channels = 0;
                            }
                            else {
                                j1SysOutputs[output].pixels = (o->GetChannels() - j1SysOutputs[output].startChannel + 1) / 3;
                                channels -= o->GetChannels() - j1SysOutputs[output].startChannel + 1 - j1SysOutputs[output].pixels * 3;
                                output++;
                            }
                        }
                        lastEnd = m->GetEndChannel();
                    }
                }

                // make sure every row has a copy of the bank protocol and speed
                for (auto i = 1; i < GetBankSize(); i++) {
                    j1SysOutputs[bankStart + i].protocol = j1SysOutputs[bankStart].protocol;
                    j1SysOutputs[bankStart + i].speed = j1SysOutputs[bankStart].speed;
                }
            }
            else {
                // unused port
            }
        }

        int port = 0;
        for (auto& j : j1SysOutputs) {
            j.port = port;
            port++;
        }

        spdlog::debug("Uploading pixel config:");
        DumpConfig(j1SysOutputs);

        spdlog::debug("Building pixel upload:");
        std::string requestString;
        for (const auto& j : j1SysOutputs) {
            if (requestString != "") requestString += "&";
            requestString += BuildStringPort(j.active, j.port, j.protocol, j.speed, j.startChannel, j.universe, j.pixels);
        }

        if (requestString != "") {
            std::string res = PutURL("/protect/stringConfig.htm", requestString, _username, _password);
            if (res == "") {
                success = false;
            }
        }
    }

    if (success && cud.GetMaxSerialPort() > 0) {
        std::vector<J1SysSerialOutput> j1SysOutputs(caps->GetMaxSerialPort());

        ReadCurrentSerialConfig(j1SysOutputs);
        spdlog::debug("Existing config:");
        DumpConfig(j1SysOutputs);

        for (int sp = 1; sp <= cud.GetMaxSerialPort(); sp++) {
            if (cud.HasSerialPort(sp)) {
                UDControllerPort* port = cud.GetControllerSerialPort(sp);
                j1SysOutputs[sp - 1].active = true;
                j1SysOutputs[sp - 1].protocol = EncodeSerialPortProtocol(port->GetProtocol());
                if (j1SysOutputs[sp - 1].protocol == 'D' && j1SysOutputs[sp - 1].speed < 5) {
                    j1SysOutputs[sp - 1].speed = 5;
                }
                j1SysOutputs[sp - 1].universe = port->GetUniverse();

                int32_t sc;
                auto o = outputManager->GetOutput(port->GetStartChannel(), sc);
                int32_t sc2;
                auto o2 = outputManager->GetOutput(port->GetEndChannel(), sc2);

                if (o != o2) {
                    ui->ShowMessage("Controller " + _ip + " serial port " + std::to_string(sp) + " requires more than 1 universe.", "Error");
                    spdlog::debug("Erroneous config:");
                    DumpConfig(j1SysOutputs);
                    return false;
                }

                if (sc != 1) {
                    ui->ShowMessage("Controller " + _ip + " serial port " + std::to_string(sp) + " does not start on channel 1 of universe " +
                        std::to_string(port->GetUniverse()) + ". It starts at " +
                        std::to_string(port->GetStartChannel()) + ".", "Error");
                    spdlog::debug("Erroneous config:");
                    DumpConfig(j1SysOutputs);
                    return false;
                }
            }
        }

        spdlog::debug("Uploading serial config:");
        DumpConfig(j1SysOutputs);

        spdlog::debug("Building serial upload:");
        std::string requestString;
        for (const auto& j : j1SysOutputs) {
            if (requestString != "")
                requestString += "&";
            requestString += BuildSerialPort(j.active, j.port + 1, j.protocol, j.speed, j.universe);
        }

        if (requestString != "") {
            std::string res = PutURL("/protect/portConfig.htm", requestString, _username, _password);
            if (res == "") {
                success = false;
            }
        }
    }

    if (success) Reboot();

    return success;
}
#pragma endregion
