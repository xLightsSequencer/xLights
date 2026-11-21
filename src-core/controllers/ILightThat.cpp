/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerCaps.h"
#include "ILightThat.h"

#include "UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include <curl/curl.h>

#include <nlohmann/json.hpp>

#include <log.h>
#include <utils/CurlManager.h>

#pragma region Constructors and Destructors
ILightThat::ILightThat(const std::string& ip, const std::string& proxy) :
    BaseController(ip, proxy)
{
    
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        _connected = true;
        spdlog::debug("Connected to ILightThat controller model {}.", (const char*)GetFullName().c_str());
    } else {
        _connected = false;
        spdlog::error("Error connecting to ILightThat controller on {}.", (const char*)_ip.c_str());
    }
}

ILightThat::~ILightThat()
{
    /*
    for (const auto& it : _pixelOutputs) {
        delete it;
    }
    _pixelOutputs.clear();*/
}

#pragma endregion

#pragma region Getters and Setters
bool ILightThat::GetBoardCapabilities(BoardCapabilities& caps, std::string& error) const
{
    caps = BoardCapabilities();

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    std::string const url = "http://" + baseIP + _baseUrl + "/api/board/info";

    // The board answers 503 while it's busy or short of memory, and asks for a retry
    int rc = 0;
    std::string res;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        res = CurlManager::INSTANCE.doGet(url, rc);
        if (rc != 503) {
            break;
        }
    }

    if (rc == 404) {
        // Firmware from before the endpoint, which applies none of these settings
        spdlog::debug("ILightThat Outputs Upload: {} has no /api/board/info.", (const char*)_ip.c_str());
        return true;
    }
    if (rc != 200) {
        error = rc == 0 ? "Could not read the board's capabilities: no response." :
                          "Could not read the board's capabilities: HTTP " + std::to_string(rc) + ".";
        return false;
    }

    nlohmann::json info = nlohmann::json::parse(res, nullptr, false);
    if (!info.is_object()) {
        error = "Could not read the board's capabilities: the reply was not understood.";
        spdlog::warn("ILightThat Outputs Upload: /api/board/info replied {}", (const char*)res.c_str());
        return false;
    }
    if (!info.contains("capabilities") || !info["capabilities"].is_array()) {
        return true;
    }
    for (auto const& c : info["capabilities"]) {
        if (!c.is_string()) {
            continue;
        }
        std::string const name = c.get<std::string>();
        if (name == "grouping") {
            caps.grouping = true;
        } else if (name == "null") {
            caps.nulls = true;
        } else if (name == "zigzag") {
            caps.zigzag = true;
        }
    }
    return true;
}

bool ILightThat::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, UICallbacks* ui)
{
    
    spdlog::debug("ILightThat Outputs Upload: Uploading to {}", (const char*)_ip.c_str());

    std::unordered_map<std::string, int> model_test_cols = {};
    nlohmann::json jsonVal;
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        try {
            jsonVal = nlohmann::json::parse(json);
            if (jsonVal["ports"].is_array()) {
                for (int i = 0; i < (int)jsonVal["ports"].size(); i++) {
                    if (jsonVal["ports"][i].contains("models")) {
                        for (int j = 0; j < (int)jsonVal["ports"][i]["models"].size(); j++) {
                            auto model = jsonVal["ports"][i]["models"][j];
                            if (model.contains("test_colour")) {
                                model_test_cols[model["name"].get<std::string>()] = model["test_colour"].get<int>();
                            }
                        }
                    }
                }
            }
        } catch (nlohmann::json::parse_error& ex) {
            spdlog::warn("ILightThat Outputs Upload: Failed to parse JSON: {}", ex.what());
            spdlog::warn((const char*)json.c_str());
        } catch (std::exception& e) {
            spdlog::warn("ILightThat Outputs Upload: Failed to parse JSON: {}", e.what());
            spdlog::warn((const char*)json.c_str());
        }
    }

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool const success = cud.Check(rules, check);

    if (success) {
        BoardCapabilities boardCaps;
        std::string capsError;
        if (!GetBoardCapabilities(boardCaps, capsError)) {
            spdlog::error("ILightThat Outputs Upload: {}", (const char*)capsError.c_str());
            ui->ShowMessage("ILightThat Upload Error:\n" + capsError, "Error");
            return false;
        }

        std::string unsupported;
        int const maxPortChannels = rules ? rules->GetMaxPixelPortChannels() : 0;

        nlohmann::json outputConfig;
        int const first_channel = cud.GetFirstOutput()->GetStartChannel();
        outputConfig["start_address"] = first_channel;
        outputConfig["start_universe"] = cud.GetFirstOutput()->GetUniverse();
        if (cud.GetFirstOutput()->GetType() == OUTPUT_E131 ||
            cud.GetFirstOutput()->GetType() == OUTPUT_ARTNET) {
            // We know number of channels will all be the same, as the xcontroller file specifies
            // AllInputUniversesMustBeSameSize
            outputConfig["channels_per_universe"] = cud.GetFirstOutput()->GetChannels();
        } else {
            // DDP or something else. Give a nice default value...
            outputConfig["channels_per_universe"] = 510;
        }
        //GetOutputConfig(outputConfig);

        // We're not actually using these as serial port - temp stand-in for relays
        for (int x = 0; x < cud.GetMaxSerialPort(); x++) {
            UDControllerPort* port = cud.GetControllerSerialPort(x + 1);
            std::string const proto = port->GetProtocol();

            outputConfig["ports"][x]["num_pixels"] = port->Pixels();

            int i = 0;
            for (auto model : port->GetModels()) {
                outputConfig["ports"][x]["models"][i]["name"] = model->GetName();
                outputConfig["ports"][x]["models"][i]["brightness"] = 100;
                outputConfig["ports"][x]["models"][i]["start_channel"] = model->GetStartChannel() - port->GetFirstModel()->GetStartChannel();
                outputConfig["ports"][x]["models"][i]["num_channels"] = (model->GetEndChannel() - model->GetStartChannel()) + 1;
                outputConfig["ports"][x]["models"][i]["colour_order"] = std::string("RGB");
                i++;
            }
        }

        int relay_ports = controller->GetControllerCaps()->GetMaxSerialPort();
        for (int x = 0 + relay_ports; x < cud.GetMaxPixelPort() + relay_ports; x++) {
            UDControllerPort* port = cud.GetControllerPixelPort((x - relay_ports) + 1);
            std::string const proto = port->GetProtocol();
            outputConfig["ports"][x]["num_pixels"] = port->Pixels();

            int i = 0;
            int portWireChannels = 0;
            size_t const unsupportedBeforePort = unsupported.size();
            for (auto model : port->GetModels()) {
                outputConfig["ports"][x]["models"][i]["name"] = model->GetName();
                int brightness = model->GetBrightness(-1);
                if (brightness == -1) {
                    if (controller->IsFullxLightsControl()) {
                        brightness = controller->GetDefaultBrightnessUnderFullControl();
                    } else {
                        brightness = 100;
                    }
                }

                float gamma = model->GetGamma(-1);
                if (gamma == -1) {
                    if (controller->IsFullxLightsControl()) {
                        gamma = controller->GetDefaultGammaUnderFullControl();
                    } else {
                        gamma = 1.0;
                    }
                }

                if (model_test_cols.find(model->GetName()) == model_test_cols.end()) {
                    outputConfig["ports"][x]["models"][i]["test_colour"] = _model_test_default_colours [_model_test_default_col_idx];
                    _model_test_default_col_idx = (_model_test_default_col_idx + 1) % _model_test_default_colours.size();
                } else {
                    outputConfig["ports"][x]["models"][i]["test_colour"] = model_test_cols.find(model->GetName())->second;
                }

                outputConfig["ports"][x]["models"][i]["brightness"] = brightness;
                outputConfig["ports"][x]["models"][i]["gamma"] = gamma;
                outputConfig["ports"][x]["models"][i]["start_channel"] = model->GetStartChannel() - port->GetFirstModel()->GetStartChannel();

                std::string colorOrder = model->GetColourOrder("unknown");
                if (colorOrder == "unknown") {
                    colorOrder = "RGB";
                }
                outputConfig["ports"][x]["models"][i]["colour_order"] = colorOrder;

                // The board counts a model in channels on the wire, where xLights counts the
                // model's own, so a grouped model is sent as its length times the group count
                // and the board divides it back down. Nulls are counted in channels as well,
                // at the width of a pixel in the model's colour order. The keys are sent only
                // to firmware that lists the setting in /api/board/info; a model that needs a
                // setting the board can't apply is refused, as its output would be wrong.
                int const groupCount = std::max(1, model->GetGroupCount(1));
                int const channelsPerPixel = (int)colorOrder.size();
                int const startNulls = std::max(0, model->GetStartNullPixels(0));
                int const endNulls = std::max(0, model->GetEndNullPixels(0));
                int const zigZag = std::max(0, model->GetZigZag(0));
                int const wireChannels = ((model->GetEndChannel() - model->GetStartChannel()) + 1) * groupCount;
                outputConfig["ports"][x]["models"][i]["num_channels"] = wireChannels;
                portWireChannels += wireChannels + (startNulls + endNulls) * channelsPerPixel;

                if (boardCaps.grouping) {
                    outputConfig["ports"][x]["models"][i]["grouping"] = groupCount;
                } else if (groupCount > 1) {
                    unsupported += "Model " + model->GetName() + " uses grouping, which the board's firmware does not support.\n";
                }
                if (boardCaps.nulls) {
                    outputConfig["ports"][x]["models"][i]["start_null_channels"] = startNulls * channelsPerPixel;
                    outputConfig["ports"][x]["models"][i]["end_null_channels"] = endNulls * channelsPerPixel;
                } else if (startNulls > 0 || endNulls > 0) {
                    unsupported += "Model " + model->GetName() + " uses null pixels, which the board's firmware does not support.\n";
                }
                if (boardCaps.zigzag) {
                    outputConfig["ports"][x]["models"][i]["zigzag"] = zigZag;
                } else if (zigZag > 1) {
                    // The board treats 0 and 1 alike, as off
                    unsupported += "Model " + model->GetName() + " uses zig zag, which the board's firmware does not support.\n";
                }
                i++;
            }

            // xLights checked the port against its data channels; grouping and nulls make it
            // longer on the wire, and the board cuts a port off at its limit. A port with a
            // setting the board can't apply has that to fix first, not its length.
            if (unsupported.size() == unsupportedBeforePort && maxPortChannels > 0 && portWireChannels > maxPortChannels) {
                unsupported += "Port " + std::to_string((x - relay_ports) + 1) + " needs " + std::to_string(portWireChannels) +
                               " channels with grouping and null pixels. Maximum " + std::to_string(maxPortChannels) + ".\n";
            }
        }

        if (!unsupported.empty()) {
            spdlog::warn("ILightThat Outputs Upload: {}", (const char*)unsupported.c_str());
            ui->ShowMessage("ILightThat Upload Error:\n" + unsupported, "Error");
            return false;
        }

        for (int i = 0; i < jsonVal["ports"].size(); i++) {
            outputConfig["ports"][i]["port_type"] = jsonVal["ports"][i]["port_type"];
        }

        std::string const response = PutURL("/settings", outputConfig.dump(), "", "", "application/json");
        return (response == "OK");
    }
    spdlog::warn("ILightThat Outputs Upload: {}", (const char*)check.c_str());
    ui->ShowMessage("ILightThat Upload Error:\n" + check, "Error");
    return false;
}
#pragma endregion
