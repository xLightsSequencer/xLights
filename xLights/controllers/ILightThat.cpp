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

#include "../UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"

#include <nlohmann/json.hpp>

#include <curl/curl.h>

#include "./utils/spdlog_macros.h"
#include <utils/CurlManager.h>

#pragma region Constructors and Destructors
ILightThat::ILightThat(const std::string& ip, const std::string& proxy) :
    BaseController(ip, proxy)
{
    
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        //nlohmann::json val = nlohmann::json::parse(json);
        _connected = true;
        LOG_DEBUG("Connected to ILightThat controller model %s.", (const char*)GetFullName().c_str());
    } else {
        _connected = false;
        LOG_ERROR("Error connecting to ILightThat controller on %s.", (const char*)_ip.c_str());
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
bool ILightThat::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent)
{
    
    LOG_DEBUG("ILightThat Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    std::unordered_map<std::string, int> model_test_cols = {};
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        nlohmann::json jsonVal = nlohmann::json::parse(json);

        if (jsonVal["ports"].is_array()) {
            for (int i = 0; i < jsonVal["ports"].size(); i++) {
                for (int j = 0; j < jsonVal["ports"][i]["models"].size(); j++) {
                    auto model = jsonVal["ports"][i]["models"][j];
                    if (model.contains("test_colour")) {
                        model_test_cols[model["name"].get<std::string>()] = model["test_colour"].get<int>();
                    }
                }
            }
        }
    }

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    if (success) {
        nlohmann::json outputConfig;
        int first_channel = cud.GetFirstOutput()->GetStartChannel();
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
        for (int x = 0; x < cud.GetMaxPixelPort(); x++) {
            UDControllerPort* port = cud.GetControllerPixelPort(x + 1);
            std::string const proto = port->GetProtocol();
            outputConfig["ports"][x]["num_pixels"] = port->Pixels();

            int i = 0;
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

                if (model_test_cols.find(model->GetName()) == model_test_cols.end()) {
                    outputConfig["ports"][x]["models"][i]["test_colour"] = _model_test_default_colours [_model_test_default_col_idx];
                    _model_test_default_col_idx = (_model_test_default_col_idx + 1) % _model_test_default_colours.size();
                } else {
                    outputConfig["ports"][x]["models"][i]["test_colour"] = model_test_cols.find(model->GetName())->second;
                }

                outputConfig["ports"][x]["models"][i]["brightness"] = brightness;
                outputConfig["ports"][x]["models"][i]["start"] = (model->GetStartChannel() - port->GetFirstModel()->GetStartChannel()) / 3;
                outputConfig["ports"][x]["models"][i]["num_pixels"] = ((model->GetEndChannel() - model->GetStartChannel()) + 1) / 3;

                const std::string colorOrder = model->GetColourOrder("unknown");
                if (colorOrder != "unknown") {
                    outputConfig["ports"][x]["models"][i]["colour_order"] = colorOrder;
                }
                else
                {
                    outputConfig["ports"][x]["models"][i]["colour_order"] = std::string("RGB");
                }
                i++;
            }
        }

        for (int i = cud.GetMaxPixelPort(); i < controller->GetControllerCaps()->GetMaxPixelPort(); i++) {
            outputConfig["ports"][i]["num_pixels"] = 0;
        }
        std::string response = PutURL("/settings", outputConfig.dump(), "", "", "application/json");
        return (response == "OK");
    }
    return false;
}
#pragma endregion
