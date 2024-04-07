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
        
        if (_connected) {
            logger_base.debug("Connected to ILightThat controller model %s.", (const char*)GetFullName().c_str());
        }
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

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    if (success) {
        bool changed = false;
        wxJSONValue outputConfig = new wxJSONValue();
        int first_channel = cud.GetFirstOutput()->GetStartChannel();
        outputConfig["start_address"] = first_channel;
        //GetOutputConfig(outputConfig);
        for (int x = 0; x < cud.GetMaxPixelPort(); x++) {
            UDControllerPort* port = cud.GetControllerPixelPort(x + 1);
            std::string const proto = port->GetProtocol();
            if (port->Pixels() > 750) {
                DisplayError("Max 750 pixels per port for Baldrick Board", parent);
                return false;
            }
            outputConfig["ports"][x]["num_pixels"] = port->Pixels();

            int i = 0;
            for (auto model : port->GetModels()) {
                outputConfig["ports"][x]["models"][i]["name"] = model->GetName();
                int brightness = model->GetBrightness(-1);
                if (brightness == -1)
                {
                    brightness = 100;
                }
                
                outputConfig["ports"][x]["models"][i]["brightness"] = brightness;
                outputConfig["ports"][x]["models"][i]["start"] = (model->GetStartChannel() - port->GetFirstModel()->GetStartChannel()) / 3;
                outputConfig["ports"][x]["models"][i]["num_pixels"] = ((model->GetEndChannel() - model->GetStartChannel()) + 1) / 3;

                i++;
            }
        }
        for (int i = cud.GetMaxPixelPort(); i < controller->GetControllerCaps()->GetMaxPixelPort() + 1; i++)
        {
            outputConfig["ports"][i]["num_pixels"] = 0;
        }

        std::string url = "http://" + _ip + "/settings";
        wxJSONWriter writer;
        wxString Data;
        writer.Write(outputConfig, Data);
        logger_base.debug(std::string("SetHttpConfig: Data: '") + Data + "'");

        std::string contentType = "application/json";
        int ReturnCode = -1;
        std::vector<unsigned char> value(Data.begin(), Data.end());
        CurlManager::INSTANCE.doPost(url, contentType, value, ReturnCode);

        // logger_base.debug(std::string("SetHttpConfig: ReturnCode: '") + std::to_string(ReturnCode) + "'");
        return (200 == ReturnCode);
    }
    return false;
}
#pragma endregion
