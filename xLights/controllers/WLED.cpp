
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/progdlg.h>

#include "WLED.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/DDPOutput.h"
#include "../models/ModelManager.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"
#include "../UtilFunctions.h"

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <curl/curl.h>

#include <log4cpp/Category.hh>

#pragma region Output Classes
struct WLEDOutput {
    const int output;
    int startCount = 0;
    int pixels = 0;
    int colorOrder = 0;
    int protocol = 1;
    bool reverse = false;
    uint8_t pin = 255;

    WLEDOutput(int output_) : output(output_) { }
    void Dump() const {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("    Output %d Start %d Pixels %d Rev %d ColorOrder %d Protocol %d Pin %d",
            output,
            startCount,
            pixels,
            reverse,
            colorOrder,
            protocol,
            pin
        );
    }

    void UpdateJSON(wxJSONValue& json) {
        wxJSONValue portJson;
        portJson["len"] = pixels;
        portJson["start"] = startCount;
        wxJSONValue pinJson;
        pinJson.Append(pin);
        portJson["pin"] = pinJson;
        portJson["type"] = protocol;
        portJson["order"] = colorOrder;
        portJson["rev"] = reverse;
        portJson["skip"] = 0;

        json.Append(portJson);
    }
};
#pragma endregion

#pragma region Constructors and Destructors
WLED::WLED(const std::string& ip, const std::string &proxy) : BaseController(ip, proxy) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const json = GetURL(GetInfoURL());
    if (!json.empty()) {
        wxJSONValue jsonVal;
        wxJSONReader reader;

        reader.Parse(json, &jsonVal);
        if (jsonVal.HasMember("ver") && jsonVal.HasMember("arch") && jsonVal.HasMember("name")) {
            _version = jsonVal["ver"].AsString();
            int vid = jsonVal["vid"].AsInt();
            _model = jsonVal["arch"].AsString();
            _connected = true;

            if(_model.find("32") != std::string::npos) {
                _controllerType = WLEDType::ESP32;
            }

            if (vid < 2105110) {
                logger_base.error("Build 2105110 of WLED Is Required, '%d' is Installed .", vid);
                _connected = false;
            }
        } else {
            logger_base.error("Error Determining WLED controller Type.");
            _connected = false;
        }

        if (_connected) {
            logger_base.debug("Connected to WLED controller model %s.", (const char*)GetFullName().c_str());
        }
    } else {
        _connected = false;
        logger_base.error("Error connecting to WLED controller on %s.", (const char *)_ip.c_str());
    }
}

WLED::~WLED() {

    for (const auto& it : _pixelOutputs) {
        delete it;
    }
    _pixelOutputs.clear();

}
#pragma endregion

#pragma region Private Functions

bool WLED::ParseOutputJSON(wxJSONValue & jsonVal) {

    _pixelOutputs.clear();

    for (int i = 1; i <= GetNumberOfOutputs(); i++) {
        WLEDOutput* output = ExtractOutputJSON(jsonVal, i);
        output->Dump();
        _pixelOutputs.push_back(output);
    }

    return true;
}

WLEDOutput* WLED::ExtractOutputJSON(wxJSONValue& jsonVal, int port) {

    WLEDOutput* output = new WLEDOutput(port);

    auto json = jsonVal["hw"]["led"]["ins"][port - 1];

    if (json.IsValid()) {
        if (json["len"].IsInt()) {
            output->pixels = json["len"].AsInt();
        }
        if (json["start"].IsInt()) {
            output->startCount = json["start"].AsInt();
        }
        if (json["pin"].IsArray()) {
            if (json["pin"][0].IsValid()) {
                output->pin = json["pin"][0].AsInt();
            }
        }
        if (json["type"].IsInt()) {
            output->protocol = json["type"].AsInt();
        }
        if (json["order"].IsInt()) {
            output->colorOrder = json["order"].AsInt();
        }
        if (json["rev"].IsBool()) {
            output->reverse = json["rev"].AsBool();
        }

    }
    //work around for un-setup pins
    if (output->pin == 255) {
        output->pin = GetOutputPin(port);
    }

    return output;
}

void WLED::UpdatePortData(WLEDOutput* pd, UDControllerPort* stringData, int startNumber) const {

    if (pd != nullptr) {
        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown" && pd->reverse != EncodeDirection(direction)) {
            pd->reverse = EncodeDirection(direction);
        }

        const std::string color = stringData->GetFirstModel()->GetColourOrder("");
        if (!color.empty()) {
            int newcolor = EncodeColorOrder(color);
            if (pd->colorOrder != newcolor) {
                pd->colorOrder = newcolor;
            }
        }

        int const protocol = EncodeStringPortProtocol(stringData->GetFirstModel()->GetProtocol());
        if (protocol != -1) {
            pd->protocol = protocol;
        }

        if (pd->startCount != startNumber) {
            pd->startCount = startNumber;
        }

        if (pd->startCount != startNumber) {
            pd->startCount = startNumber;
        }

        if (pd->pixels != stringData->Pixels()) {
            pd->pixels = stringData->Pixels();
        }
    }
}

void WLED::UpdatePixelOutputs(bool& worked, int totalPixelCount, wxJSONValue& jsonVal) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Building pixel upload:");
    //total Pixel Count
    jsonVal["hw"]["led"]["total"] = totalPixelCount;

    //Port Pixel Count
    wxJSONValue newLEDS;
    for (const auto& pixelPort : _pixelOutputs) {
        if (pixelPort->pixels != 0) {
            pixelPort->UpdateJSON(newLEDS);
        }
    }

    jsonVal["hw"]["led"]["ins"] = newLEDS;
}

bool WLED::PostJSON(wxJSONValue const& jsonVal) {
    wxString str;
    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 3);
    writer.Write(jsonVal, str);

    const std::string url = GetCfgURL();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    logger_base.debug("Making request to Controller '%s'.", (const char*)url.c_str());
    logger_base.debug("    With data '%s'.", (const char*)str.c_str());

    CURL* hnd = curl_easy_init();

    if (hnd) {
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");

        curl_easy_setopt(hnd, CURLOPT_URL, std::string("http://" + baseIP + _baseUrl + url).c_str());
        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, "cache-control: no-cache");
        headers = curl_slist_append(headers, "content-type: application/json");
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, (long)str.size());
        curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, (const char*)str.c_str());

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeFunction);

        std::string buffer = "";
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &buffer);

        CURLcode ret = curl_easy_perform(hnd);
        if (ret == CURLE_OK) {
            if (buffer.find("error") != std::string::npos) {
                logger_base.error("Error From WLED %s", (const char*)buffer.c_str());
                return false;
            }
            return true;
        } else {
            logger_base.error("Failure to access %s: %s.", (const char*)url.c_str(), curl_easy_strerror(ret));
        }
    }
    return false;
}

bool WLED::SetupInput(ControllerEthernet* controller, wxJSONValue& jsonVal) {

    //get prevous RGB Mode
    int rgbMode = jsonVal["if"]["live"]["dmx"]["mode"].AsInt();

    if (controller->GetOutputCount() > 9) {
        DisplayError(wxString::Format("Attempt to upload %d universes to WLED controller but only 9 are supported.", controller->GetOutputCount()).ToStdString());
        return false;
    }

    if (!controller->AllSameSize()) {
        DisplayError("Attempting to upload universes to the WLED controller that are not the same size.");
        return false;
    }

    int port = 0;
    auto o = controller->GetFirstOutput();

    if (o->GetChannels() > 510) {
        DisplayError(wxString::Format("Attempt to upload a universe of size %d to the WLED controller, but only a size of 510 or smaller is supported", o->GetChannels()).ToStdString());
        return false;
    }

    if (o->GetType() == OUTPUT_E131) {
        port = 5568;
    }
    else if (o->GetType() == OUTPUT_ARTNET) {
        port = 6454;
    }
    else if (o->GetType() == OUTPUT_DDP) {
        port = 4048;
        DDPOutput* ddp = dynamic_cast<DDPOutput*>(o);
        if (ddp) {
            if (ddp->IsKeepChannelNumbers()) {
                DisplayError("The DDP 'Keep Channel Numbers' option is not support with WLED, Please Disable");
                return false;
            }
        }
    }

    jsonVal["if"]["live"]["en"] = true;
    jsonVal["if"]["live"]["port"] = port;

    if (o->GetIP() == "MULTICAST") {
        jsonVal["if"]["live"]["mc"] = true;
    }

    if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET) {
        jsonVal["if"]["live"]["dmx"]["uni"] = o->GetUniverse();
        jsonVal["if"]["live"]["dmx"]["addr"] = 1;
    }
    else if (o->GetType() == OUTPUT_DDP) {
        jsonVal["if"]["live"]["dmx"]["addr"] = 1;// o->GetStartChannel();
    }

    //Turn On E131/DDP Multiple RGB Mode "DM=4", TODO: Support RGBW mode "DM=6"
    if (rgbMode < 4) {
        rgbMode = 4;
    }
    jsonVal["if"]["live"]["dmx"]["mode"] = rgbMode;
    return true;
}

int WLED::EncodeStringPortProtocol(const std::string& protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return 22;
    if (p == "tm18xx") return 31;
    if (p == "ws2801") return 50;
    if (p == "lpd8806") return 52;
    if (p == "apa102") return 51;

    wxASSERT(false);
    return -1;
}

int WLED::EncodeColorOrder(const std::string& colorOrder) const {

    wxString c(colorOrder);
    c = c.Lower();

    if (c == "grb") return 0;
    if (c == "rgb") return 1;
    if (c == "brg") return 2;
    if (c == "rbg") return 3;
    if (c == "bgr") return 4;
    if (c == "gbr") return 5;

    wxASSERT(false);
    return -1;
}

bool WLED::EncodeDirection(const std::string& direction) const {

    return direction == "Reverse";
}

WLEDOutput* WLED::FindPortData(int port) {

    for (const auto& sd : _pixelOutputs) {
        if (sd->output == port) {
            return sd;
        }
    }
    wxASSERT(false);
    return nullptr;
}

const int WLED::GetNumberOfOutputs() {

    if (_controllerType == WLEDType::QuinLEDDigQuadESP8266 ) {
        return 3; 
    }
    if (_controllerType == WLEDType::QuinLEDDigQuadESP32) {
        return 4;
    }
    if (_controllerType == WLEDType::ESP32) {
        return 8;
    }
    if (_controllerType == WLEDType::ESP8266) {
        return 2;
    }
    return 2;
}

const uint8_t WLED::GetOutputPin(int port) {
    //Config upload needs GPIO output Pin, This Makes Generic List
    if (_controllerType == WLEDType::ESP8266) {
        switch (port) {
        case 1:
            return 2;
        case 2:
            return 1;
        case 3:
            return 3;
        default:
            return 2;
        }
    } else if (_controllerType == WLEDType::ESP32) {
        //esp32dev board
        switch (port) {
        case 1:
            return 2;
        case 2:
            return 13;
        case 3:
            return 12;
        case 4:
            return 14;
        case 5:
            return 27;
        case 6:
            return 26;
        case 7:
            return 25;
        case 8:
            return 33;
        default:
            return 2;
        }
    } else if (_controllerType == WLEDType::QuinLEDDigQuadESP8266) {
        //v2 pinout
        switch (port) {
        case 1:
            return 2;
        case 2:
            return 3;
        case 3:
            return 1;
        default:
            return 2;
        }
    } else if (_controllerType == WLEDType::QuinLEDDigQuadESP32) {
        //v2 pinout
        switch (port) {
        case 1:
            return 16;
        case 2:
            return 3;
        case 3:
            return 1;
        case 4:
            return 4;
        default:
            return 16;
        }
    }
    return 2;
}

#pragma endregion

#pragma region Getters and Setters
bool WLED::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("WLED Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    progress.Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, check, false);

    //first check rules
    auto caps = ControllerCaps::GetControllerConfig(controller);
    const bool success = cud.Check(caps, check);

    logger_base.debug(check);

    cud.Dump();

    if (!success) {
        DisplayError("WLED Upload Error:\n" + check, parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    //QuinLED-Dig-Quad Override
    if (controller->GetModel().find("QuinLED") != std::string::npos &&
        controller->GetModel().find("Quad") != std::string::npos) {
        if (_controllerType == WLEDType::ESP8266) _controllerType = WLEDType::QuinLEDDigQuadESP8266;
        if (_controllerType == WLEDType::ESP32) _controllerType = WLEDType::QuinLEDDigQuadESP32;
    }

    //get current config JSON
    const std::string page = GetURL(GetCfgURL());

    wxJSONValue val;
    wxJSONReader reader;
    int parseResult = reader.Parse(page, &val);

    if (parseResult != 0) {
        DisplayError("WLED Upload Error:\n JSON Parse Error", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    bool worked = SetupInput(controller, val);
    if (!worked) {
        progress.Update(100, "Aborting.");
        return false;
    }

    worked = ParseOutputJSON(val);
    if (!worked) {
        DisplayError("Unable to Parse JSON.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    logger_base.info("Figuring Out Pixel Output Information.");
    progress.Update(20, "Figuring Out Pixel Output Information.");

    //loop to setup string outputs
    int totalCount = 0;
    for (int port = 1; port <= GetNumberOfOutputs(); port++) {
        WLEDOutput* pixOut = FindPortData(port);
        if(pixOut == nullptr) {
            continue;
        }
        if (cud.HasPixelPort(port)) {
            UDControllerPort* portData = cud.GetControllerPixelPort(port);
            UpdatePortData(pixOut, portData, totalCount); 
        }
        totalCount += pixOut->pixels;
    }

    logger_base.info("Updating String Output Information.");
    progress.Update(40, "Updating String Output Information.");

    UpdatePixelOutputs(worked, totalCount, val);

    if (!worked) {
        logger_base.error("Error Updating to WLED controller, JSON:%s.", (const char*)page.c_str());
    }

    logger_base.info("Uploading JSON to WLED.");
    progress.Update(70, "Uploading JSON to WLED.");

    //reboot
    val["rb"] = true;

    bool uploadWorked = PostJSON(val);

    if (!uploadWorked) {
        logger_base.error("Error Uploading to WLED controller, JSON:%s.", (const char*)page.c_str());
        worked = false;
    }

    logger_base.info("WLED Outputs Upload Done.");
    progress.Update(100, "Done.");
    return worked;
}
#pragma endregion
