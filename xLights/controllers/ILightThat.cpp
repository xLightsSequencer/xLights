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

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <curl/curl.h>

#include <log4cpp/Category.hh>
#include <utils/CurlManager.h>

#pragma region Constructors and Destructors
ILightThat::ILightThat(const std::string& ip, const std::string& proxy) :
    BaseController(ip, proxy)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        wxJSONValue jsonVal;
        wxJSONReader reader;

        reader.Parse(json, &jsonVal);
        _connected = true;
        logger_base.debug("Connected to ILightThat controller model %s.", (const char*)GetFullName().c_str());
    } else {
        _connected = false;
        logger_base.error("Error connecting to ILightThat controller on %s.", (const char*)_ip.c_str());
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("ILightThat Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    std::unordered_map<std::string, int> model_test_cols = {};
    std::string const json = GetURL("/settings");
    if (!json.empty()) {
        wxJSONValue jsonVal;
        wxJSONReader reader;

        reader.Parse(json, &jsonVal);
        if (jsonVal["ports"].IsArray()) {
            for (int i = 0; i < jsonVal["ports"].Size(); i++) {
                for (int j = 0; j < jsonVal["ports"][i]["models"].Size(); j++) {
                    wxJSONValue model = jsonVal["ports"][i]["models"][j];
                    if (model.HasMember("test_colour")) {
                        model_test_cols[model["name"].AsString()] = model["test_colour"].AsInt();
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
        wxJSONValue outputConfig = new wxJSONValue();
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
                i++;
            }
        }

        for (int i = cud.GetMaxPixelPort(); i < controller->GetControllerCaps()->GetMaxPixelPort(); i++) {
            outputConfig["ports"][i]["num_pixels"] = 0;
        }

        wxJSONWriter writer;
        wxString Data;
        writer.Write(outputConfig, Data);
        std::string response = PutURL("/settings", Data, "", "", "application/json");
        return (response == "OK");
    }
    return false;
}
#pragma endregion
