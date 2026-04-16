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
#include "PowerDMX.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/DDPOutput.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../render/UICallbacks.h"

#include "../utils/CurlManager.h"

#include <log.h>
#include <spdlog/fmt/fmt.h>

#pragma region Private Functions
bool PowerDMX::GetJSONData(std::string const& url, nlohmann::json& val) const {
    std::string const sval = GetURL(url);
    if (!sval.empty()) {
        try {
            val = nlohmann::json::parse(sval);
            return true;
        } catch (nlohmann::json::parse_error& e) {
            spdlog::warn("PowerDMX Outputs Upload: Failed to parse JSON from {}: {}", url, e.what());
        }
    }
    return false;
}

std::string PowerDMX::PostJSONToURL(std::string const& url, nlohmann::json const& val) const {
    return PutURL(url, val.dump(3), "", "", "application/json");
}
#pragma endregion

bool PowerDMX::UploadSequence(std::string const& seq, std::string const& file, std::function<bool(int, std::string)> progress) {
    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    std::string const url = "http://" + baseIP + _baseUrl + GetUploadURL();
    spdlog::debug("Uploading to URL: {}", url);
    std::filesystem::path fn(file);
    return CurlManager::HTTPUploadFile(url, seq, fn.filename().string(), progress);
}

#pragma region Encode and Decode
int PowerDMX::EncodeBrightness(int brightness) const {
    // above 30 round to 10s, below 30 round to 5s
    if (brightness < 30) {
        int i = brightness + 2;
        i -= i % 5;
        return i;
    }
    int ii = brightness + 5;
    ii -= ii % 10;
    return ii;
}

int PowerDMX::EncodeGamma(double gamma) const {
    if (1.5 > gamma) {
        return 10;
    }
    if (2.15 > gamma) {
        return 20;
    }
    if (2.4 > gamma) {
        return 23;
    }
    if (2.65 > gamma) {
        return 25;
    }
    if (2.9 > gamma) {
        return 28;
    }
    return 30;
}

std::string PowerDMX::EncodeColorOrder(std::string const& colorOrder) const {
    // no convertion needed yet
    return Lower(colorOrder);
}
#pragma endregion

#pragma region Constructors and Destructors
PowerDMX::PowerDMX(std::string const& ip, std::string const& proxy) :
    BaseController(ip, proxy) {
    nlohmann::json status;

    // Get Controller Info
    if (!GetJSONData(GetStatusURL(), status)) {
        spdlog::error("Error connecting to PowerDMX controller on {}.", _ip);
        return;
    }

    //nlohmann::json config;
    /*
    
    {
     "system": {
     "vendor": "PowerDMX",
     "product": "PowerDMX",
     "variant": "PowerDMX-49",
     "typeId": 208,
     "version": "1.0.0",
     "mac": "7C:2C:67:60:01:90"
     },
     "capabilities": {
     "max_ports": 49,
     "max_pixels_per_port": 4096,
     "supported_inputs": ["ddp", "e131"],
     "fseq": {
     "supported": true,
     "max_version": 2,
     "compression": ["none", "zlib"]
     }
     },
     "config": {
     "active_input": "ddp",
     "ports": [
     {
     "port": 1,
     "pixels": 200,
     "universe": 1,
     "protocol": "ws2811",
     "description": "Local Port 1"
     },
     {
     "port": 2,
     "pixels": 100,
     "universe": 3,
     "protocol": "ws2811",
     "description": "Remote Node 1-1"
     }
     ]
     }
    }
    */

    if (status.size() > 0) {
        // decode controller type
        _model = status["system"]["variant"].get<std::string>();
        _version = status["system"]["version"].get<std::string>();
        _numberOfPixelOutputs = status["capabilities"]["max_ports"].get<int>();
        _connected = true;
        spdlog::debug("Connected to PowerDMX controller model {} v{}.", _model, _version);
    } else {
        _connected = false;
        spdlog::error("Error connecting to PowerDMX controller on {}.", _ip);
        DisplayError(fmt::format("Error connecting to PowerDMX controller on {}.", _ip));
    }
}

#pragma endregion

#pragma region Getters and Setters

int32_t PowerDMX::SetInputUniverses(nlohmann::json& data, Controller* controller) {
    spdlog::debug("PowerDMX Inputs Upload: Uploading to {}", _ip);
    int32_t startChannel{ -1 };
    auto eth = dynamic_cast<ControllerEthernet*>(controller);
    if (eth == nullptr) {
        return startChannel;
    }

    /*
    {
     "inputs": "ddp",
     "ports": [
     { "port": 1, "pixels": 200, "universe": 1 },
     { "port": 2, "pixels": 100, "universe": 3 }
     ]
    }
    */

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    auto out = outputs.front();
    startChannel = out->GetStartChannel();

    //bool const allSameSize = eth->AllSameSize();
    //if (out->GetType() == OUTPUT_E131 || out->GetType() == OUTPUT_ARTNET) {
    //    nlohmann::json universes;
    //    if (allSameSize) {
    //        // all the same size, make one entry
    //        nlohmann::json universe;
    //        universe["start_universe"] = out->GetUniverse();
    //        universe["number_of_universes"] = eth->GetOutputCount();
    //        universe["channels_per_universe"] = out->GetChannels();
    //        universe["start_channel"] = 1;
    //        universes.push_back(universe);
    //    } else {
    //        // not the same size, loop through them all individually
    //        for (auto const& it : outputs) {
    //            nlohmann::json universe;
    //            universe["start_universe"] = it->GetUniverse();
    //            universe["number_of_universes"] = 1;
    //            universe["channels_per_universe"] = it->GetChannels();
    //            universe["start_channel"] = it->GetStartChannel() - out->GetStartChannel() + 1;
    //            universes.push_back(universe);
    //        }
    //    }
    //    data["inputs"] = universes;
    //} else if (out->GetType() == OUTPUT_DDP) {
    //    DDPOutput* ddp = (DDPOutput*)out;
    //    if (ddp->IsKeepChannelNumbers()) {
    //        data["system"]["start_channel"] = startChannel;
    //        startChannel = 1; // offset for pixel page
    //    } else {
    //        data["system"]["start_channel"] = 1;
    //    }
    //} else {
    //    // should never hit this
    //    DisplayError(fmt::format(
    //        "Invalid Input Type For Experience Controller {}.",
    //        out->GetType()));
    //    return startChannel;
    //}

    

    if (out->GetType() == OUTPUT_DDP) {
        data["inputs"] = "ddp";
    } else {
        data["inputs"] = out->GetType() == OUTPUT_E131 ? "e1.31" : "artnet";
    }

    return startChannel;
}

bool PowerDMX::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* c, UICallbacks* ui) {
    ControllerEthernet* controller = dynamic_cast<ControllerEthernet*>(c);
    if (controller == nullptr) {
        DisplayError(fmt::format("{} is not a PowerDMX controller.", c->GetName()));
        return false;
    }

    auto progressTk = ui->BeginProgress("Uploading ...", 100);

    spdlog::debug("PowerDMX Outputs Upload: Uploading to {}", _ip);

    ui->UpdateProgress(progressTk, 0, "Scanning models");
    spdlog::info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    // first check rules
    auto rules = ControllerCaps::GetControllerConfig(controller);
    const bool success = cud.Check(rules, check);

    spdlog::debug(check);

    cud.Dump();
    if (!success) {
        ui->ShowMessage("PowerDMX Upload Error:\n" + check, "Error");
        ui->UpdateProgress(progressTk, 100, "Aborting.");
        ui->EndProgress(progressTk);
        return false;
    }

    spdlog::info("Initializing Pixel Output Information.");
    ui->UpdateProgress(progressTk, 10, "Initializing Pixel Output Information.");

    nlohmann::json stringData;

    spdlog::info("Initializing Universe Input Information.");
    ui->UpdateProgress(progressTk, 20, "Initializing Universe Input Information.");
    int32_t const startChannel = SetInputUniverses(stringData, controller);

    if (-1 == startChannel) {
        spdlog::error("Error Calculating Universe Input Information.");
        return false;
    }

    spdlog::info("Figuring Out Pixel Output Information.");
    ui->UpdateProgress(progressTk, 30, "Figuring Out Pixel Output Information.");
    // loop to setup string outputs
    for (int p = 1; p <= GetNumberOfPixelOutputs(); p++) {
        //{ "port": 1, "pixels": 200, "universe": 1 },
        nlohmann::json port;
        port["port"] = p;
        if (cud.HasPixelPort(p)) {
            UDControllerPort* portData = cud.GetControllerPixelPort(p);
            port["pixels"] = portData->Pixels();
            port["universe"] = portData->GetUniverse();
        } else {
            port["pixels"] = 0;
            port["universe"] = 0;
        }
        stringData["ports"].push_back(port);
    }
    spdlog::info("Uploading Output Information.");
    ui->UpdateProgress(progressTk, 70, "Uploading String Output Information.");

    PostJSONToURL(GetConfigURL(), stringData);
    ui->UpdateProgress(progressTk, 100, "Done.");
    ui->EndProgress(progressTk);
    return true;
}
#pragma endregion
