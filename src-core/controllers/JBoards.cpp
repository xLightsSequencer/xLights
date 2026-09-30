/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "JBoards.h"
#include "ControllerCaps.h"
#include "FPP.h"
#include "../discovery/Discovery.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../render/UICallbacks.h"
#include "../utils/CurlManager.h"
#include "../utils/ExternalHooks.h"
#include "../utils/string_utils.h"
#include "UtilFunctions.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>

#include <log.h>

namespace {
constexpr int kMaxStringsPerPort = 8;
constexpr int kChainLanes = 4;

std::string JsonString(const nlohmann::json& j, const char* key) {
    if (!j.is_object()) return std::string();
    auto it = j.find(key);
    return (it != j.end() && it->is_string()) ? it->get<std::string>() : std::string();
}

int JsonInt(const nlohmann::json& j, const char* key, int def) {
    if (!j.is_object()) return def;
    auto it = j.find(key);
    return (it != j.end() && it->is_number_integer()) ? it->get<int>() : def;
}

nlohmann::json ParseObject(const std::string& body) {
    nlohmann::json j = nlohmann::json::parse(body, nullptr, false);
    return (j.is_discarded() || !j.is_object()) ? nlohmann::json::object() : j;
}

std::string Utf8Prefix(const std::string& s, size_t maxBytes) {
    if (s.size() <= maxBytes) return s;
    size_t n = maxBytes;
    while (n > 0 && (static_cast<unsigned char>(s[n]) & 0xC0) == 0x80) --n;
    return s.substr(0, n);
}

std::string SmartRemoteLetter(int sr) {
    return (sr >= 1 && sr <= 26) ? std::string(1, char('A' + sr - 1)) : std::to_string(sr);
}

std::string ReasonText(const std::string& reason) {
    static const std::map<std::string, std::string> text = {
        {"tooManyStrings", "more than 8 strings on the port"},
        {"overPixelBudget", "more pixels (including null pixels) than the port holds"},
        {"badColorOrder", "a color order the controller does not support"},
        {"badString", "a string the controller could not read"},
        {"emptyStrings", "no strings were given for the port"},
        {"badProtocol", "a protocol the port does not support"},
        {"smartReceiver", "the port belongs to a smart-receiver chain"},
        {"multipleSegments", "the port is split into segments on the controller"},
        {"dmxInUse", "DMX is in use by effects, direct or mixed control; turn on Full xLights Control to take it over"},
        {"noSuchPort", "the controller has no such port"},
        {"pastChannelBridge", "the start channel is past the end of the controller's channels"},
        {"badStartChannel", "an invalid start channel"},
        {"badChannelCount", "an invalid channel count"},
        {"rejected", "the controller rejected the settings"},
    };
    auto it = text.find(reason);
    return it != text.end() ? it->second : reason;
}

std::string ErrorText(const nlohmann::json& response) {
    std::string text = JsonString(response, "error");
    if (text.empty()) text = JsonString(response, "message");
    return text.empty() ? std::string() : " (" + text + ")";
}

struct DiscoveredBoardInfo {
    bool audio = false;
    std::string arch;
};
std::map<std::string, DiscoveredBoardInfo>& DiscoveredBoards() {
    static std::map<std::string, DiscoveredBoardInfo> boards;
    return boards;
}
std::mutex& DiscoveredBoardsLock() {
    static std::mutex lock;
    return lock;
}

DiscoveredData* ReadoptJBoardsBoard(Discovery& discovery, const std::string& ip) {
    DiscoveredData* cd = discovery.FindByIp(ip);
    if (cd != nullptr && cd->controller != nullptr && (cd->typeId == 0xC5 || Lower(cd->vendor) == "jboards")) {
        return cd;
    }
    return nullptr;
}

constexpr auto kPreparedUploadReuse = std::chrono::seconds(60);

struct PreparedUpload {
    std::chrono::steady_clock::time_point when;
    bool answered = false;
    bool audio = false;
    int32_t startChannel = 0;
    std::string ranges;
};
std::map<std::string, PreparedUpload>& PreparedUploads() {
    static std::map<std::string, PreparedUpload> boards;
    return boards;
}
std::mutex& PreparedUploadsLock() {
    static std::mutex lock;
    return lock;
}

std::string BoardUrl(const std::string& ip, const std::string& proxy) {
    return proxy.empty() ? "http://" + ip : "http://" + proxy + "/proxy/" + ip;
}
}

#pragma region Discovery
void JBoards::ProcessDiscoveryInfo(Discovery& discovery, const std::string& ip, const std::string& body) {
    {
        std::lock_guard<std::mutex> lock(DiscoveredBoardsLock());
        DiscoveredBoards().erase(ip);
    }
    nlohmann::json info = nlohmann::json::parse(body, nullptr, false);
    if (info.is_discarded() || !info.is_object()) return;
    const std::string vendor = JsonString(info, "vendor");
    const std::string model = JsonString(info, "model");
    if (Lower(vendor) != "jboards" || model.empty()) {
        spdlog::debug("JBoards discovery: ignoring {} (vendor '{}', model '{}')", ip, vendor, model);
        return;
    }
    DiscoveredData* cd = discovery.FindByIp(ip);
    ControllerEthernet* ce = cd ? cd->controller : nullptr;
    if (!ce) {
        ce = new ControllerEthernet(discovery.GetOutputManager(), false);
        ce->SetProtocol(OUTPUT_DDP);
        ce->SetIP(ip);
        if (cd) {
            cd->controller = ce;
        } else {
            cd = discovery.AddController(ce);
        }
    }
    cd->hostname = JsonString(info, "name");
    const std::string description = JsonString(info, "description");
    cd->extraData["description"] = description.empty() ? cd->hostname : description;
    cd->SetVendor("JBoards");
    cd->SetModel(model);
    cd->SetVariant("");
    cd->version = JsonString(info, "version");
    cd->platform = "JBoards";
    {
        std::lock_guard<std::mutex> lock(DiscoveredBoardsLock());
        DiscoveredBoardInfo& board = DiscoveredBoards()[ip];
        if (info.contains("audio") && info["audio"].is_boolean()) {
            board.audio = info["audio"].get<bool>();
        }
        const std::string arch = JsonString(info, "arch");
        if (!arch.empty()) {
            board.arch = arch;
        }
    }
    if ((cd->mode.empty() || cd->mode == "bridge") && info.contains("mode") && info["mode"].is_string()) {
        cd->mode = info["mode"].get<std::string>();
    }
    JBoards::AdoptDiscoveredBoard(discovery, cd);
}

void JBoards::AdoptDiscoveredBoard(Discovery& discovery, DiscoveredData* cd) {
    if (cd == nullptr || cd->controller == nullptr) return;
    ControllerEthernet* ce = cd->controller;
    if (ce->GetProtocol() != OUTPUT_DDP) {
        ce->SetProtocol(OUTPUT_DDP);
    }
    ce->SetAutoLayout(true);
    ce->SetAutoSize(true, nullptr);
    ce->SetFullxLightsControl(true);
    if (cd->extraData.contains("description") && cd->extraData["description"].is_string()) {
        const std::string description = cd->extraData["description"].get<std::string>();
        const std::string current = ce->GetDescription();
        if (!description.empty() && current != description &&
            (current.empty() || Lower(current).rfind("jboards-", 0) == 0)) {
            cd->SetDescription(description);
        }
    }
}

void JBoards::PrepareDiscovery(Discovery& discovery) {
    discovery.AddBonjour("_jboards._tcp", [&discovery](const std::string& ip) {
        discovery.AddCurl(ip, "/xlights/info", [&discovery, ip](int rc, const std::string& buffer, const std::string&) {
            if (rc == 200) ProcessDiscoveryInfo(discovery, ip, buffer);
            return true;
        });
    });
    discovery.AddBroadcast(DDP_PORT, [&discovery](uint8_t* buffer, int len, const std::string& fromIP) {
        if (len >= 4 && (buffer[0] & DDP_FLAGS1_QUERY) == 0 && (buffer[3] == DDP_ID_CONFIG || buffer[3] == DDP_ID_STATUS)) {
            AdoptDiscoveredBoard(discovery, ReadoptJBoardsBoard(discovery, fromIP));
        }
    });
}

void JBoards::ProcessFPPDiscovery(Discovery& discovery, DiscoveredData* cd) {
    if (cd->vendor.empty()) {
        cd->SetVendor("JBoards");
    }
    AdoptDiscoveredBoard(discovery, cd);
    const std::string ip = cd->ip;
    discovery.AddCurl(ip, "/xlights/info", [&discovery, ip](int rc, const std::string& body, const std::string&) {
        if (rc == 200) {
            ProcessDiscoveryInfo(discovery, ip, body);
        }
        return true;
    });
}

bool JBoards::PlaysAudio(const std::string& ip) {
    std::lock_guard<std::mutex> lock(DiscoveredBoardsLock());
    auto it = DiscoveredBoards().find(ip);
    return it != DiscoveredBoards().end() && it->second.audio;
}

bool JBoards::PrefersUncompressedFSEQ(const std::string& ip) {
    std::lock_guard<std::mutex> lock(DiscoveredBoardsLock());
    auto it = DiscoveredBoards().find(ip);
    return it != DiscoveredBoards().end() && Lower(it->second.arch).rfind("esp32", 0) == 0;
}
#pragma endregion

#pragma region Constructors and Destructors
JBoards::JBoards(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {
    const std::string body = GetURL("/xlights/info");
    const nlohmann::json info = ParseObject(body);
    if (Lower(JsonString(info, "vendor")) != "jboards") {
        spdlog::error("JBoards: no JBoards controller answered at {}.", _ip);
        return;
    }
    _model = JsonString(info, "model");
    _version = JsonString(info, "version");
    if (info.contains("smartReceiverBuses") && info["smartReceiverBuses"].is_array()) {
        for (const auto& b : info["smartReceiverBuses"]) {
            if (b.is_number_integer()) _chainBases.push_back(b.get<int>());
        }
    }
    _chainSlots = JsonInt(info, "smartReceiverSlots", _chainSlots);
    _serialPorts = JsonInt(info, "maxSerialPorts", _serialPorts);
    _pixelPorts = JsonInt(info, "maxPixelPorts", _pixelPorts);
    if (info.contains("audio") && info["audio"].is_boolean()) {
        _audio = info["audio"].get<bool>();
    }
    if (info.contains("sequenceStartChannel") && info["sequenceStartChannel"].is_number_integer()) {
        _sequenceStartChannel = info["sequenceStartChannel"].get<int64_t>();
    }
    _connected = true;
    spdlog::debug("Connected to JBoards controller {}.", GetFullName());
}
#pragma endregion

#pragma region Private Functions
bool JBoards::PostJSON(const std::string& url, const nlohmann::json& body, nlohmann::json& response) {
    response = ParseObject(PutURL(url, body.dump(), "", "", "application/json"));
    return response.value("success", false);
}

bool JBoards::PutJSON(const std::string& url, const nlohmann::json& body, nlohmann::json& response) {
    const std::string baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    int rc = 0;
    CurlManager::SyncRequestOptions options;
    options.method = "PUT";
    options.body = body.dump();
    options.contentType = "application/json";
    options.timeoutSeconds = 15;
    options.responseCode = &rc;
    const std::string res = CurlManager::INSTANCE.doRequest("http://" + baseIP + _baseUrl + url, options);
    response = ParseObject(res);
    return rc == 200 && response.value("success", false);
}

std::string JBoards::SkippedText(const nlohmann::json& response) {
    std::string text;
    if (!response.contains("skipped") || !response["skipped"].is_array()) return text;
    for (const auto& s : response["skipped"]) {
        const std::string reason = JsonString(s, "reason");
        text += "\n  Port " + std::to_string(JsonInt(s, "port", 0)) + ": " +
                ReasonText(reason.empty() ? std::string("rejected") : reason);
    }
    return text;
}

bool JBoards::InChain(int port) const {
    return std::any_of(_chainBases.begin(), _chainBases.end(),
                       [port](int base) { return port >= base && port < base + kChainLanes; });
}

#ifndef DISCOVERYONLY
nlohmann::json JBoards::StringJson(const UDVirtualString& vs, bool fullControl,
                                   int defaultBrightness) {
    nlohmann::json s;
    const int cpp = std::max(1, vs._channelsPerPixel);
    int pixels = vs.Channels() / cpp;
    if (vs._groupCountSet && vs._groupCount > 1) {
        pixels *= vs._groupCount;
        s["grouping"] = vs._groupCount;
    }
    s["startChannel"] = vs._startChannel;
    s["pixels"] = pixels;
    s["description"] = Utf8Prefix(vs._description, 31);
    if (vs._colourOrderSet) {
        s["colorOrder"] = vs._colourOrder;
    } else if (cpp == 4) {
        s["colorOrder"] = "RGBW";
    }
    if (vs._brightnessSet) {
        s["brightness"] = vs._brightness;
    } else if (fullControl) {
        s["brightness"] = defaultBrightness;
    }
    if (vs._gammaSet) s["gamma"] = vs._gamma;
    if (vs._startNullPixelsSet) s["nullPixels"] = vs._startNullPixels;
    if (vs._endNullPixelsSet) s["endNullPixels"] = vs._endNullPixels;
    if (vs._zigZagSet) s["zigzag"] = vs._zigZag;
    if (vs._reverseSet) s["reverse"] = (vs._reverse == "Reverse");
    return s;
}

bool JBoards::UploadChains(UDController& cud, Controller* controller, bool fullControl,
                           std::set<int>& chainPorts, std::string& error) {
    const int defaultBrightness = controller->GetDefaultBrightnessUnderFullControl();

    for (const int base : _chainBases) {
        std::vector<std::array<nlohmann::json, kChainLanes>> cells(std::max(0, _chainSlots));
        for (auto& row : cells) for (auto& c : row) c = nlohmann::json::array();
        int receivers = 0;
        bool plainModels = false;

        for (int lane = 0; lane < kChainLanes; ++lane) {
            const int port = base + lane;
            if (!cud.HasPixelPort(port)) continue;
            for (const auto* vs : cud.GetControllerPixelPort(port)->GetVirtualStrings()) {
                if (vs->_isDummy) continue;
                const int sr = vs->_smartRemote;
                if (sr == 0) { plainModels = true; continue; }
                if (sr < 0 || sr > _chainSlots) {
                    error = "Port " + std::to_string(port) + " uses Smart Remote " + SmartRemoteLetter(sr) +
                            "; this controller has " + std::to_string(_chainSlots) + " receivers per chain.";
                    return false;
                }
                auto& cell = cells[sr - 1][lane];
                if (static_cast<int>(cell.size()) >= kMaxStringsPerPort) {
                    error = "Port " + std::to_string(port) + " Smart Remote " + SmartRemoteLetter(sr) +
                            " has more than " + std::to_string(kMaxStringsPerPort) + " strings.";
                    return false;
                }
                const nlohmann::json s = StringJson(*vs, fullControl, defaultBrightness);
                nlohmann::json seg;
                seg["pixelCount"] = s["pixels"];
                seg["streamingStartChannel"] = s["startChannel"];
                if (s.contains("colorOrder")) seg["colorOrder"] = s["colorOrder"];
                if (s.contains("brightness")) seg["brightness"] = s["brightness"];
                if (s.contains("gamma")) seg["gamma"] = s["gamma"];
                if (s.contains("nullPixels")) seg["startNullPixels"] = s["nullPixels"];
                if (s.contains("endNullPixels")) seg["endNullPixels"] = s["endNullPixels"];
                if (s.contains("grouping")) seg["groupCount"] = s["grouping"];
                if (s.contains("zigzag")) seg["zigZag"] = s["zigzag"];
                if (s.contains("reverse")) seg["reverse"] = s["reverse"];
                cell.push_back(seg);
                receivers = std::max(receivers, sr);
            }
        }

        nlohmann::json response;
        if (receivers == 0) {
            if (fullControl || plainModels) {
                nlohmann::json rm;
                rm["remove"] = true;
                if (!PutJSON("/api/pixels/jbsr-group/" + std::to_string(base), rm, response)) {
                    const std::string reason = JsonString(response, "error");
                    error = "The controller could not remove the smart-receiver chain on ports " +
                            std::to_string(base) + "-" + std::to_string(base + kChainLanes - 1) +
                            (reason.empty() ? std::string(".") : ": " + reason + ".");
                    return false;
                }
            }
            continue;
        }
        if (plainModels) {
            error = "Ports " + std::to_string(base) + "-" + std::to_string(base + kChainLanes - 1) +
                    " mix smart-remote models with plain models; a chain is all smart remotes or none.";
            return false;
        }

        nlohmann::json body;
        body["receiverCount"] = receivers;
        body["protocol"] = "v2";
        nlohmann::json ports = nlohmann::json::array();
        for (int sr = 0; sr < receivers; ++sr) {
            nlohmann::json row = nlohmann::json::array();
            for (int lane = 0; lane < kChainLanes; ++lane) row.push_back(cells[sr][lane]);
            ports.push_back(row);
        }
        body["ports"] = ports;
        spdlog::debug("JBoards chain upload: base {} receivers {}", base, receivers);
        if (!PutJSON("/api/pixels/jbsr-group/" + std::to_string(base), body, response)) {
            const std::string reason = JsonString(response, "error");
            error = "The controller refused the smart-receiver chain on ports " + std::to_string(base) +
                    "-" + std::to_string(base + kChainLanes - 1) + ": " +
                    (reason.empty() ? std::string("a lane may exceed its pixel budget") : reason) + ".";
            return false;
        }
        for (int lane = 0; lane < kChainLanes; ++lane) chainPorts.insert(base + lane);
    }
    return true;
}

bool JBoards::UploadPixelPorts(UDController& cud, ControllerCaps* caps, Controller* controller,
                               bool fullControl, const std::set<int>& chainPorts,
                               nlohmann::json& sent, std::string& warning, std::string& error) {
    const int defaultBrightness = controller->GetDefaultBrightnessUnderFullControl();
    nlohmann::json outputs = nlohmann::json::array();

    for (int port = 1; port <= caps->GetMaxPixelPort(); ++port) {
        if (chainPorts.count(port)) continue;
        nlohmann::json o;
        o["port"] = port;
        nlohmann::json strings = nlohmann::json::array();
        if (cud.HasPixelPort(port)) {
            for (const auto* vs : cud.GetControllerPixelPort(port)->GetVirtualStrings()) {
                if (vs->_isDummy) continue;
                if (vs->_smartRemote != 0) {
                    error = "Port " + std::to_string(port) + " has a smart-remote model (" + vs->_description +
                            ") but is not part of an uploaded smart-receiver chain.";
                    return false;
                }
                strings.push_back(StringJson(*vs, fullControl, defaultBrightness));
            }
            if (static_cast<int>(strings.size()) > kMaxStringsPerPort) {
                error = "Port " + std::to_string(port) + " needs " + std::to_string(strings.size()) +
                        " strings; this controller takes " + std::to_string(kMaxStringsPerPort) +
                        " per port. Give props on this port matching settings, or move some to another port.";
                return false;
            }
        } else if (!fullControl) {
            continue;
        }
        o["enabled"] = !strings.empty();
        o["strings"] = strings;
        outputs.push_back(o);
    }

    nlohmann::json body;
    body["outputs"] = outputs;
    sent = body;
    if (outputs.empty()) return true;
    nlohmann::json response;
    if (!PostJSON("/xlights/outputs", body, response)) {
        error = "The controller did not accept the output configuration" + ErrorText(response) + "." + SkippedText(response);
        return false;
    }
    const std::string skipped = SkippedText(response);
    if (!skipped.empty()) {
        error = "Some ports were not applied:" + skipped;
        return false;
    }
    if (response.contains("driverReinit") && response["driverReinit"].is_boolean() && !response["driverReinit"].get<bool>()) {
        warning = "\n  The controller saved the outputs but could not apply them; they take effect when it restarts.";
    }
    return true;
}

bool JBoards::UploadSerialPorts(UDController& cud, bool fullControl, std::string& error) {
    if (_serialPorts < 1) return true;
    nlohmann::json ports = nlohmann::json::array();
    for (int sp = 1; sp <= _serialPorts; ++sp) {
        nlohmann::json p;
        p["port"] = sp;
        p["protocol"] = "dmx";
        if (cud.HasSerialPort(sp)) {
            UDControllerPort* port = cud.GetControllerSerialPort(sp);
            p["enabled"] = true;
            p["startChannel"] = port->GetStartChannel();
            p["channels"] = port->Channels();
        } else if (fullControl) {
            p["enabled"] = false;
        } else {
            continue;
        }
        ports.push_back(p);
    }
    if (ports.empty()) return true;
    nlohmann::json body;
    body["fullControl"] = fullControl;
    body["ports"] = ports;
    nlohmann::json response;
    if (!PostJSON("/xlights/serial", body, response)) {
        error = "The controller did not accept the DMX configuration" + ErrorText(response) + "." + SkippedText(response);
        return false;
    }
    return true;
}

std::string JBoards::ReadBackDifferences(const nlohmann::json& sent) {
    const nlohmann::json now = ParseObject(GetURL("/xlights/outputs"));
    if (!now.contains("outputs") || !now["outputs"].is_array()) return "\n  Could not read the configuration back.";
    std::map<int, nlohmann::json> byPort;
    for (const auto& o : now["outputs"]) {
        if (o.is_object()) byPort[JsonInt(o, "port", 0)] = o;
    }
    std::string diffs;
    for (const auto& o : sent["outputs"]) {
        const int port = JsonInt(o, "port", 0);
        const auto& want = o["strings"];
        const auto it = byPort.find(port);
        const nlohmann::json got = (it != byPort.end() && it->second.contains("strings") && it->second["strings"].is_array())
                                       ? it->second["strings"] : nlohmann::json::array();
        bool same = want.size() == got.size();
        for (size_t i = 0; same && i < want.size(); ++i) {
            same = JsonInt(want[i], "startChannel", 0) == JsonInt(got[i], "startChannel", -1) &&
                   JsonInt(want[i], "pixels", 0) == JsonInt(got[i], "pixels", -1);
        }
        if (!same) diffs += "\n  Port " + std::to_string(port) + " reads back differently.";
    }
    return diffs;
}

bool JBoards::CheckBoard(Controller* controller, ControllerCaps*& caps, std::string& error) const {
    if (!_connected) {
        error = "Unable to connect to the controller at " + _ip + ".";
        return false;
    }
    caps = ControllerCaps::GetControllerConfig(controller);
    if (caps == nullptr) {
        error = "xLights has no definition for this controller model.";
        return false;
    }
    if (_model.empty()) {
        error = "The controller did not report its model; check its firmware version.";
        return false;
    }
    if (caps->GetModel() != _model) {
        error = "This controller reports itself as " + _model + ", but it is set up in xLights as " +
                caps->GetModel() + ". Pick the matching model in the controller's settings.";
        return false;
    }
    if (_pixelPorts > 0 && caps->GetMaxPixelPort() > _pixelPorts) {
        error = "This controller reports " + std::to_string(_pixelPorts) + " pixel ports; the " +
                caps->GetModel() + " definition has " + std::to_string(caps->GetMaxPixelPort()) + ".";
        return false;
    }
    const auto& outputs = controller->GetOutputs();
    for (Output* o : outputs) {
        if (!caps->SupportsMultipleSimultaneousInputProtocols() && o->GetType() != outputs.front()->GetType()) {
            error = "This controller takes one input protocol at a time; " + outputs.front()->GetType() +
                    " and " + o->GetType() + " are both set up.";
            return false;
        }
        if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET) {
            if (o->GetChannels() > caps->GetMaxInputUniverseChannels() || o->GetChannels() < caps->GetMinInputUniverseChannels()) {
                error = "Universe " + std::to_string(o->GetUniverse()) + " has " + std::to_string(o->GetChannels()) +
                        " channels; this controller takes " + std::to_string(caps->GetMinInputUniverseChannels()) + " to " +
                        std::to_string(caps->GetMaxInputUniverseChannels()) + " per universe.";
                return false;
            }
        }
    }
    return true;
}

bool JBoards::InputConfig(Controller* c, nlohmann::json& cfg, std::string& error) {
    auto* controller = dynamic_cast<ControllerEthernet*>(c);
    if (controller == nullptr) {
        error = c->GetName() + " is not an Ethernet controller.";
        return false;
    }
    Output* o = controller->GetFirstOutput();
    cfg = nlohmann::json::object();
    cfg["e131"]["enabled"] = false;
    cfg["artnet"]["enabled"] = false;
    cfg["ddp"]["enabled"] = false;
    if (o->GetType() == OUTPUT_DDP) {
        auto* ddp = dynamic_cast<DDPOutput*>(o);
        cfg["ddp"]["enabled"] = true;
        cfg["ddp"]["mappingMode"] =
            (ddp != nullptr && ddp->IsKeepChannelNumbers()) ? "showChannels" : "controllerChannels";
    } else if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET) {
        if (!controller->AllSameSize()) {
            error = "All universes on the controller must be the same size.";
            return false;
        }
        const char* key = (o->GetType() == OUTPUT_E131) ? "e131" : "artnet";
        if (o->GetType() == OUTPUT_ARTNET && o->GetUniverse() < 1) {
            error = "Art-Net universe 0 is not supported. Set this controller's start universe to 1 or higher.";
            return false;
        }
        cfg[key]["enabled"] = true;
        cfg[key]["startUniverse"] = o->GetUniverse();
        cfg[key]["universeCount"] = controller->GetOutputCount();
        cfg[key]["channelsPerUniverse"] = o->GetChannels();
    } else {
        error = "Use DDP, E1.31 or Art-Net for this controller.";
        return false;
    }
    cfg["sequenceStartChannel"] = controller->GetStartChannel();
    return true;
}

bool JBoards::PostInputs(const nlohmann::json& cfg, std::string& error) {
    nlohmann::json response;
    if (!PostJSON("/xlights/inputs", cfg, response)) {
        error = "The controller did not accept the input configuration" + ErrorText(response) + ".";
        return false;
    }
    return true;
}
#endif
#pragma endregion

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
bool JBoards::SetInputUniverses(Controller* controller, UICallbacks* ui) {
    ControllerCaps* caps = nullptr;
    nlohmann::json cfg;
    std::string error;
    if (!CheckBoard(controller, caps, error) || !InputConfig(controller, cfg, error) || !PostInputs(cfg, error)) {
        ui->ShowMessage("JBoards Upload Error:\n" + error, "Error");
        return false;
    }
    return true;
}

bool JBoards::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {
    ControllerCaps* caps = nullptr;
    std::string error;
    if (!CheckBoard(controller, caps, error)) {
        ui->ShowMessage("JBoards Upload Error:\n" + error, "Error");
        return false;
    }
    auto progress = ui->BeginProgress("Uploading ...", 100);
    auto fail = [&](const std::string& msg) {
        ui->ShowMessage("JBoards Upload Error:\n" + msg, "Error");
        ui->UpdateProgress(progress, 100, "Aborting.");
        ui->EndProgress(progress);
        return false;
    };

    ui->UpdateProgress(progress, 0, "Scanning models");
    UDController cud(controller, outputManager, allmodels, false);
    std::string check;
    if (!cud.Check(caps, check)) return fail(check);
    const bool fullControl = caps->SupportsFullxLightsControl() && controller->IsFullxLightsControl();

    nlohmann::json inputs;
    if (!InputConfig(controller, inputs, error)) return fail(error);
    if (cud.GetMaxSerialPort() > _serialPorts) {
        return fail("A DMX model is on serial port " + std::to_string(cud.GetMaxSerialPort()) +
                    "; this controller reports " + std::to_string(_serialPorts) + " DMX port" +
                    (_serialPorts == 1 ? "" : "s") + ".");
    }

    for (int port = 1; port <= caps->GetMaxPixelPort(); ++port) {
        if (!cud.HasPixelPort(port)) continue;
        UDControllerPort* pp = cud.GetControllerPixelPort(port);
        for (const auto* vs : pp->GetVirtualStrings()) {
            std::set<int> cpps;
            for (const auto* m : vs->_models) cpps.insert(m->GetChannelsPerPixel());
            if (cpps.size() > 1) {
                pp->CreateVirtualStrings(false, false);
                break;
            }
        }
        std::map<int, int> stringsPerRemote;
        for (const auto* vs : pp->GetVirtualStrings()) {
            if (vs->_isDummy) continue;
            for (const auto* m : vs->_models) {
                if (m->GetChannelsPerPixel() == 1) {
                    return fail("Port " + std::to_string(port) + " has a single-channel model (" + m->GetName() +
                                "); this controller drives RGB and RGBW pixels only.");
                }
            }
            if (vs->_smartRemote != 0 && !InChain(port)) {
                return fail("Port " + std::to_string(port) + " has a model on Smart Remote " +
                            SmartRemoteLetter(vs->_smartRemote) + " (" + vs->_description +
                            "), but that port is not on a smart-receiver chain. Set the model's Smart Remote to None, " +
                            "or move it to a port on a chain.");
            }
            if (vs->_smartRemote < 0 || vs->_smartRemote > _chainSlots) {
                return fail("Port " + std::to_string(port) + " uses Smart Remote " + SmartRemoteLetter(vs->_smartRemote) +
                            "; this controller has " + std::to_string(_chainSlots) + " receivers per chain.");
            }
            if (++stringsPerRemote[vs->_smartRemote] > kMaxStringsPerPort) {
                return fail("Port " + std::to_string(port) +
                            (vs->_smartRemote != 0 ? " Smart Remote " + SmartRemoteLetter(vs->_smartRemote) : std::string()) +
                            " needs more than " + std::to_string(kMaxStringsPerPort) + " strings; this controller takes " +
                            std::to_string(kMaxStringsPerPort) + " per port. Give props on this port matching settings, " +
                            "or move some to another port.");
            }
        }
    }
    for (const int base : _chainBases) {
        bool smart = false;
        bool plain = false;
        for (int port = base; port < base + kChainLanes; ++port) {
            if (!cud.HasPixelPort(port)) continue;
            for (const auto* vs : cud.GetControllerPixelPort(port)->GetVirtualStrings()) {
                if (vs->_isDummy) continue;
                (vs->_smartRemote != 0 ? smart : plain) = true;
            }
        }
        if (smart && plain) {
            return fail("Ports " + std::to_string(base) + "-" + std::to_string(base + kChainLanes - 1) +
                        " mix smart-remote models with plain models; a chain is all smart remotes or none.");
        }
    }

    std::set<int> chainPorts;
    ui->UpdateProgress(progress, 20, "Uploading smart receiver chains.");
    if (!UploadChains(cud, controller, fullControl, chainPorts, error)) return fail(error);

    ui->UpdateProgress(progress, 40, "Uploading pixel ports.");
    nlohmann::json sent;
    std::string warning;
    if (!UploadPixelPorts(cud, caps, controller, fullControl, chainPorts, sent, warning, error)) return fail(error);

    ui->UpdateProgress(progress, 60, "Uploading DMX.");
    if (!UploadSerialPorts(cud, fullControl, error)) return fail(error);

    ui->UpdateProgress(progress, 75, "Uploading inputs.");
    if (!PostInputs(inputs, error)) return fail(error);

    ui->UpdateProgress(progress, 90, "Checking.");
    warning += ReadBackDifferences(sent);
    if (!warning.empty()) ui->ShowMessage("JBoards Upload Warning:" + warning, "Warning");

    ui->UpdateProgress(progress, 100, "Done.");
    ui->EndProgress(progress);
    return true;
}

bool JBoards::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui) {
    return SetOutputs(allmodels, outputManager, controller, ui);
}
#endif
#pragma endregion

#pragma region Sequence Upload
#ifndef DISCOVERYONLY
namespace {
constexpr uint64_t kUploadBlockSize = 16 * 1024 * 1024;

struct ChunkProgress {
    const std::function<bool(int, std::string)>* progress = nullptr;
    std::string label;
    uint64_t offset = 0;
    uint64_t length = 0;
    int last = -1;
    bool cancelled = false;

    bool Report(uint64_t sent) {
        if (progress == nullptr || !*progress || length == 0) return true;
        const int pct = static_cast<int>(std::min<uint64_t>(sent, length) * 1000 / length);
        if (pct != last) {
            last = pct;
            cancelled |= !(*progress)(pct, label);
        }
        return !cancelled;
    }
};

int ChunkProgressCallback(void* clientp, curl_off_t, curl_off_t, curl_off_t, curl_off_t ulnow) {
    auto* p = static_cast<ChunkProgress*>(clientp);
    return p->Report(p->offset + static_cast<uint64_t>(ulnow)) ? 0 : 1;
}

bool IsVideoFile(const std::string& path) {
    static const std::set<std::string> video = { ".mp4", ".avi", ".mov", ".mkv", ".mpg", ".mpeg" };
    return video.count(Lower(std::filesystem::path(path).extension().string())) > 0;
}
}

bool JBoards::UploadFile(const std::string& ip, const std::string& proxy, const std::string& path,
                         const std::string& name, const std::string& dir,
                         const std::function<bool(int, std::string)>& progress, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        error = "ERROR Uploading file: " + name + "    Could not open source file: " + path;
        return false;
    }
    in.seekg(0, std::ios::end);
    const std::streamoff end = in.tellg();
    in.seekg(0);
    if (end <= 0) {
        error = "ERROR Uploading file: " + name + "     Source file is empty: " + path;
        return false;
    }
    const uint64_t length = static_cast<uint64_t>(end);

    const std::string url = BoardUrl(ip, proxy) + "/api/file/" + dir;
    const std::string lengthHeader = "Upload-Length: " + std::to_string(length);
    const std::string nameHeader = "Upload-Name: " + name;

    ChunkProgress cp;
    cp.progress = &progress;
    cp.label = "Transferring " + name + " to " + ip;
    cp.length = length;
    if (!cp.Report(0)) return false;

    int errorCount = 0;
    do {
        const uint64_t chunk = std::min(kUploadBlockSize, length - cp.offset);
        CurlManager::CurlPrivateData* cpd = nullptr;
        CURL* curl = CurlManager::INSTANCE.createCurl(url, &cpd, true);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L * 3 * 60);

        cpd->req->resize(chunk);
        in.read(reinterpret_cast<char*>(cpd->req->data()), static_cast<std::streamsize>(chunk));
        if (static_cast<uint64_t>(in.gcount()) != chunk) {
            curl_easy_cleanup(curl);
            delete cpd;
            error = "ERROR Uploading file: " + name + "     Could not read source file.";
            return false;
        }

        struct curl_slist* headers = nullptr;
        const std::string offsetHeader = "Upload-Offset: " + std::to_string(cp.offset);
        const std::string contentLength = "Content-Length: " + std::to_string(chunk);
        headers = curl_slist_append(headers, "Content-Type: application/offset+octet-stream");
        headers = curl_slist_append(headers, "X-Requested-With: FPPConnect");
        headers = curl_slist_append(headers, "Expect:");
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, offsetHeader.c_str());
        headers = curl_slist_append(headers, lengthHeader.c_str());
        headers = curl_slist_append(headers, nameHeader.c_str());
        headers = curl_slist_append(headers, contentLength.c_str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ChunkProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(chunk));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cpd->req->data());

        spdlog::info("JBoards upload - URL: {}    Method: PATCH    Start: {}   Length: {}   Total: {}", url, cp.offset, chunk, length);
        curl_easy_perform(curl);
        long rc = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        delete cpd;

        if (cp.cancelled) {
            return false;
        }
        if (rc != 200) {
            if ((rc == 0 || rc >= 500) && errorCount < 3) {
                ++errorCount;
                cp.offset = 0;
                in.clear();
                in.seekg(0);
                continue;
            }
            error = "ERROR Uploading file: " + name + ". Response code: " + std::to_string(rc);
            return false;
        }
        cp.offset += chunk;
        if (!cp.Report(cp.offset)) return false;
    } while (cp.offset < length);

    spdlog::debug("{} upload complete to {}. Bytes sent: {}.", name, ip, length);
    return true;
}

bool JBoards::SendSequence(const std::string& ip, const std::string& proxy, const std::string& seq,
                           const std::string& file, const std::string& media,
                           const std::function<bool(int, std::string)>& progress, std::string& error) {
    bool known = false;
    bool answered = false;
    bool audio = false;
    {
        std::lock_guard<std::mutex> lock(PreparedUploadsLock());
        auto it = PreparedUploads().find(ip);
        if (it != PreparedUploads().end() && std::chrono::steady_clock::now() - it->second.when < kPreparedUploadReuse) {
            known = true;
            answered = it->second.answered;
            audio = it->second.audio;
        }
    }
    if (!known) {
        int rc = 0;
        const nlohmann::json info = ParseObject(CurlManager::INSTANCE.doGet(BoardUrl(ip, proxy) + "/xlights/info", rc));
        answered = rc == 200 && Lower(JsonString(info, "vendor")) == "jboards";
        audio = info.contains("audio") && info["audio"].is_boolean() && info["audio"].get<bool>();
    }
    if (!answered) {
        error = "Unable to connect to the controller at " + ip + ".";
        return false;
    }
    std::string errors;
    if (!media.empty() && audio && !IsVideoFile(media)) {
        const std::string mediaName = std::filesystem::path(media).filename().string();
        std::string mediaError;
        if (!FileExists(media)) {
            mediaError = "ERROR Uploading media: " + mediaName + "     Source file not found: " + media;
        } else if (!UploadFile(ip, proxy, media, mediaName, "music", progress, mediaError) && mediaError.empty()) {
            return false;
        }
        errors = mediaError;
    }
    if (!seq.empty()) {
        const std::string name = std::filesystem::path(file).filename().string();
        const std::string dir = EndsWith(name, ".eseq") ? "effects" : "sequences";
        std::string seqError;
        if (!UploadFile(ip, proxy, seq, name, dir, progress, seqError) && seqError.empty()) {
            return false;
        }
        if (!seqError.empty()) {
            errors += (errors.empty() ? "" : "\n") + seqError;
        }
    }
    if (!errors.empty()) {
        spdlog::warn("JBoards sequence upload to {}: {}", ip, errors);
        error = errors;
        return false;
    }
    return true;
}

bool JBoards::UploadSequence(const std::string& ip, const std::string& proxy, const std::string& seq,
                             const std::string& file, const std::string& media,
                             std::function<bool(int, std::string)> progress, std::string& error) {
    const bool uploaded = SendSequence(ip, proxy, seq, file, media, progress, error);
    std::error_code ec;
    if (!seq.empty() && !std::filesystem::equivalent(seq, file, ec)) {
        std::filesystem::remove(seq, ec);
    }
    return uploaded;
}

void JBoards::PrepareSequenceUpload(FPP* inst, OutputManager* outputManager) {
    Controller* controller = nullptr;
    if (outputManager != nullptr) {
        auto controllers = outputManager->GetControllers(inst->ipAddress);
        if (controllers.size() == 1) {
            controller = controllers.front();
        }
    }
    const int32_t startChannel = controller != nullptr ? controller->GetStartChannel() : 0;
    PreparedUpload prepared;
    bool reuse = false;
    {
        std::lock_guard<std::mutex> lock(PreparedUploadsLock());
        auto it = PreparedUploads().find(inst->ipAddress);
        if (it != PreparedUploads().end() && std::chrono::steady_clock::now() - it->second.when < kPreparedUploadReuse &&
            (!it->second.answered || it->second.startChannel == startChannel)) {
            prepared = it->second;
            reuse = true;
        }
    }
    if (!reuse) {
        JBoards board(inst->ipAddress, inst->proxy());
        prepared.answered = board.IsConnected();
        prepared.audio = board._audio;
        prepared.startChannel = startChannel;
        if (board.IsConnected()) {
            if (startChannel >= 1 && board._sequenceStartChannel != startChannel) {
                nlohmann::json body;
                body["sequenceStartChannel"] = startChannel;
                nlohmann::json response;
                if (!board.PostJSON("/xlights/inputs", body, response)) {
                    inst->messages.push_back("Could not set the controller's sequence start channel to " + std::to_string(startChannel) + ".");
                }
            }
            prepared.ranges = JsonString(ParseObject(board.GetURL("/api/system/info")), "channelRanges");
        }
        prepared.when = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(PreparedUploadsLock());
        PreparedUploads()[inst->ipAddress] = prepared;
    }
    std::string ranges = prepared.ranges;
    if (ranges.empty() && controller != nullptr) {
        const uint32_t sc = controller->GetStartChannel() - 1;
        ranges = std::to_string(sc) + "-" + std::to_string(sc + controller->GetChannels() - 1);
    }
    if (!ranges.empty()) {
        inst->ranges = ranges;
    }
}
#endif
#pragma endregion
