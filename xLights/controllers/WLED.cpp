
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
#include "../models/ModelManager.h"
#include "ControllerCaps.h"
#include "../outputs/ControllerEthernet.h"
#include "../UtilFunctions.h"

#include "../xSchedule/wxJSON/jsonreader.h"

#include <log4cpp/Category.hh>

#include <regex>

#pragma region Output Classes
class WLEDOutput
{
public:
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

    wxString BuildRequest() const {
        /*
          "L0" //ascii 0-9 //strip data pin
          "LC" //strip length
          "CO" //strip color order
          "LT" //strip type
          "LS" //strip start LED
          "CV" //strip reverse
        */

        std::string reverseAdd;
        if (reverse) {
            reverseAdd = wxString::Format("&CV%d=1", output - 1);
        }

        return wxString::Format("L0%d=%d&LC%d=%d&CO%d=%d&LT%d=%d&LS%d=%d%s",
            output - 1, pin,
            output - 1, pixels,
            output - 1, colorOrder,
            output - 1, protocol,
            output - 1, startCount,
            reverseAdd);
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
            _model = jsonVal["arch"].AsString();
            _connected = true;

            auto fwv = wxSplit(_version, '.');
            int majorVer = 0;
            int minorVer = 0;
            int patchVer = 0;

            if (fwv.size() > 0) {
                majorVer = wxAtoi(fwv[0]);
                if (fwv.size() > 1) {
                    minorVer = wxAtoi(fwv[1]);
                    if (fwv.size() > 2) {
                        patchVer = wxAtoi(fwv[2]);
                    }
                }
            }

            if(_model.find("32") != std::string::npos) {
                _controllerType = WLEDType::ESP32;
            }

            if (minorVer < 12 && majorVer < 1) {
                logger_base.error("Only Version 0.12.0 of WLED Is Supported.");
                _connected = false;
            }
        }
        else {
            logger_base.error("Error Determining WLED controller Type.");
            _connected = false;
        }

        if (_connected) {
            logger_base.debug("Connected to WLED controller model %s.", (const char*)GetFullName().c_str());
        }
    }
    else {
        _connected = false;
        logger_base.error("Error connecting to WLED controller on %s.", (const char *)_ip.c_str());
    }
}

WLED::~WLED() {

    for (const auto& it : _pixelOutputs)
    {
        delete it;
    }
    _pixelOutputs.clear();

}
#pragma endregion

#pragma region Private Functions
bool WLED::ParseOutputWebpage(const std::string& page) {

    _pixelOutputs.clear();

    for (int i = 1; i <= GetNumberOfOutputs(); i++) {
        WLEDOutput* output = ExtractOutputData(page, i);
        output->Dump();
        _pixelOutputs.push_back(output);
    }

    return true;
}

WLEDOutput* WLED::ExtractOutputData(const std::string& page, int port) {
    //     pin              length            type              color order      start LED        reverse
    //d.Sf.L00.value=2;d.Sf.LC0.value=30;d.Sf.LT0.value=22;d.Sf.CO0.value=0;d.Sf.LS0.value=0;d.Sf.CV0.checked=0;
    WLEDOutput* output = new WLEDOutput(port);
    output->pin = ExtractIntFromPage(page, wxString::Format("L0%d", port - 1), 255);
    output->pixels = ExtractIntFromPage(page, wxString::Format("LC%d", port - 1), 0);
    output->startCount = ExtractIntFromPage(page, wxString::Format("LS%d", port - 1), 0);
    output->protocol = ExtractIntFromPage(page, wxString::Format("LT%d", port - 1), 1);
    output->colorOrder = ExtractIntFromPage(page, wxString::Format("CO%d", port - 1), 0);
    output->reverse = ExtractBoolFromPage(page, wxString::Format("CV%d", port - 1), false);

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

int WLED::ExtractIntFromPage(const std::string& page, const std::string& parameter, int defaultValue) {

    //;d.Sf.U2.value=65506;
    std::regex regex("d\\.Sf\\." + parameter + "\\.value=(\\d+);");

    std::smatch m;
    if (std::regex_search(page, m, regex)) {
        if (m.size() > 1) {
            if (!m[1].str().empty()) {
                return wxAtoi(m[1].str());
            }
        }
    }

    return defaultValue;
}

bool WLED::ExtractBoolFromPage(const std::string& page, const std::string& parameter, bool defaultValue) {

    //;d.Sf.RB.checked=1;
    std::regex regex( "d\\.Sf\\." + parameter + "\\.checked=(\\d+);" );

    std::smatch m;
    if (std::regex_search(page, m, regex)) {
        if (m.size() > 1) {
            if (!m[1].str().empty()) {
                return wxAtoi(m[1].str());
            }
        }
    }

    return defaultValue;
}

std::string WLED::ExtractStringFromPage(const std::string& page, const std::string& parameter, std::string defaultValue) {

    std::regex regex("d\\.Sf\\." + parameter + "\\.value=\"(\\w+)\";");

    std::smatch m;
    if (std::regex_search(page, m, regex)) {
        if (m.size() > 1) {
            if (!m[1].str().empty()) {
                return m[1].str();
            }
        }
    }
    return defaultValue;
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
    }
    else if (_controllerType == WLEDType::ESP32) {
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
    }
    else if (_controllerType == WLEDType::QuinLEDDigQuadESP8266) {
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
    }
    else if (_controllerType == WLEDType::QuinLEDDigQuadESP32) {
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

bool WLED::SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) {

    //http://192.168.5.180/settings/sync
    const std::string page = GetURL(GetSyncPageURL());

    //get prevous RGB Mode
    int rgbMode = ExtractIntFromPage(page, "DM", 4);

    if (controller->GetOutputCount() > 9) {
        DisplayError(wxString::Format("Attempt to upload %d universes to WLED controller but only 9 are supported.", controller->GetOutputCount()).ToStdString());
        return false;
    }

    if(!controller->AllSameSize()) {
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
    }

    std::string request = "EP=" + std::to_string(port);

    if (o->GetIP() == "MULTICAST") {
        request += "&EM=1";
    }

    if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET) {
        request += "&EU=" + std::to_string(o->GetUniverse()); //universe
        request += "&DA=" + std::to_string(1); //universe start channel
    }
    else if (o->GetType() == OUTPUT_DDP) {
        request += "&DA=" + std::to_string(o->GetStartChannel()); //DDP start channel
    }

    //Turn On E131/DDP Multiple RGB Mode "DM=4", TODO: Support RGBW mode "DM=6"
    if (rgbMode < 4) {
        rgbMode = 4;
    }
    request += "&DM=" + std::to_string(rgbMode);

    const std::string donePage = PutURL(GetSyncPageURL(), request);

    return true;
}

bool WLED::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) {

    //http://192.168.5.180/settings/leds
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

    //get current config Page
    const std::string page = GetURL(GetLEDPageURL());

    if (page.empty()) {
        DisplayError("WLED Upload Error:\nWebpage was empty", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    bool worked = true;
    _connected = ParseOutputWebpage(page);
    if (!_connected) {
        DisplayError("Unable to Parse Main Webpage.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    logger_base.info("Figuring Out Pixel Output Information.");
    progress.Update(30, "Figuring Out Pixel Output Information.");

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

    logger_base.info("Uploading String Output Information.");
    progress.Update(60, "Uploading String Output Information.");

    UploadPixelOutputs(worked, totalCount);

    if (!worked) {
        logger_base.error("Error Uploading to WLED controller, Page HTML:%s.", (const char*)page.c_str());
    }

    logger_base.info("WLED Outputs Upload Done.");
    progress.Update(100, "Done.");
    return worked;
}

void WLED::UploadPixelOutputs(bool& worked, int totalPixelCount) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Building pixel upload:");
    //total Pixel Count
    std::string requestString = "LC=" + std::to_string(totalPixelCount);
    
    //Port Pixel Count
    for (const auto& pixelPort : _pixelOutputs) {
        if (pixelPort->pixels != 0) {
            requestString += "&";
            requestString += pixelPort->BuildRequest();
        }
    }

    logger_base.info("Post String Output Information.");

    if (!requestString.empty()) {
        const std::string res = PutURL(GetLEDPageURL(), requestString);
        if (res.empty()) {
            worked = false;
        }
        wxMilliSleep(2000);
    }
}
#pragma endregion
