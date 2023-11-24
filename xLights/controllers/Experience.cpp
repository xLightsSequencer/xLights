/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Experience.h"
#include "ControllerCaps.h"
#include "../UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../outputs/DDPOutput.h"

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <curl/curl.h>

#include "../utils/Curl.h"

#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/sstream.h>

#include <log4cpp/Category.hh>

#pragma region Private Functions
bool Experience::GetJSONData(const std::string& url, wxJSONValue& val) const
{
    std::string const sval = GetURL(url);
    if (!sval.empty()) {
        wxJSONReader reader;
        reader.Parse(sval, &val);
        return true;
    }
    return false;
}

std::string Experience::PostJSONToURL(const std::string& url, const wxJSONValue& val) const
{
    wxString str;
    wxJSONWriter writer(wxJSONWRITER_STYLED, 0, 3);
    writer.Write(val, str);

    return PutURL(url, str, "", "", "application/json");
}
#pragma endregion

bool Experience::UploadSequence(const std::string& seq, const std::string& file,std::function<bool(int, std::string)> progress)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    std::string url = "http://" + baseIP + _baseUrl + "/upload";
    logger_base.debug("Uploading to URL: %s", (const char*)url.c_str());

    wxFileName fn(file);
    return Curl::HTTPUploadFile(url, seq, fn.GetFullName().ToStdString(), progress);
}

#pragma region Encode and Decode
int Experience::EncodeBrightness(int brightness) const
{
    //above 30 round to 10s, below 30 round to 5s
    if (brightness < 30) {
        int i = brightness + 2;
        i -= i % 5;
        return i;
    }
    int ii = brightness + 5;
    ii -= ii % 10;
    return ii;
}

int Experience::EncodeGamma(double gamma) const
{
    if (gamma > 3.0) {
        return 3.0;
    }
    return gamma * 10;
}

wxString Experience::EncodeColorOrder(std::string const& colorOrder) const
{
    //no convertion needed yet
    return Lower(colorOrder);
}
#pragma endregion

#pragma region Constructors and Destructors
Experience::Experience(const std::string& ip, const std::string& proxy) :
    BaseController(ip, proxy) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxJSONValue data;

    // Get Controller Info
    if (!GetJSONData(GetStateURL(), data)) {
        logger_base.error("Error connecting to Genius controller on %s.", (const char*)_ip.c_str());
        return;
    }

    if (data.Size() > 0) {
        //decode controller type
        _model = data["system"]["controller_model_name"].AsString();
        _version = data["system"]["firmware_version"].AsString();
        _controllerType = data["system"]["controller_line"].AsString();
        _numberOfPixelOutputs = data["system"]["number_of_local_outputs"].AsInt();
        _numberOfRemoteOutputs = data["system"]["number_of_long_range_pixel_ports"].AsInt();
        _numberOfSerialOutputs = data["system"]["number_of_long_range_dmx_ports"].AsInt();
        _connected = true;
        logger_base.debug("Connected to Genius controller model %s.", (const char*)GetFullName().c_str());
    } else {
        _connected = false;
        logger_base.error("Error connecting to Genius controller on %s.", (const char*)_ip.c_str());
        DisplayError(wxString::Format("Error connecting to Genius controller on %s.", _ip).ToStdString());
    }
}

#pragma endregion

#pragma region Getters and Setters

int32_t Experience::SetInputUniverses(wxJSONValue& data, Controller* controller)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Experience Inputs Upload: Uploading to %s", (const char*)_ip.c_str());
    int32_t startChannel{ -1 };
    auto eth = dynamic_cast<ControllerEthernet*>(controller);
    if (eth == nullptr) {
        return startChannel;
    }
    bool const allSameSize = eth->AllSameSize();

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    data["system"]["friendly_name"] = wxString(eth->GetName());
    data["system"]["auto_chain_all_outputs"] = false; //disable for xLights to handle

    auto out = outputs.front();
    startChannel = out->GetStartChannel();

    if (out->GetType() == OUTPUT_E131 || out->GetType() == OUTPUT_ARTNET) {
        wxJSONValue universes;
        if (allSameSize) {
            // all the same size, make one entry
            wxJSONValue universe;
            universe["start_universe"] = out->GetUniverse();
            universe["number_of_universes"] = eth->GetOutputCount();
            universe["channels_per_universe"] = out->GetChannels();
            universe["start_channel"] = 1;
            universes.Append(universe);
        } else {
            //not the same size, loop through them all indivually
            for (auto const& it : outputs) {
                wxJSONValue universe;
                universe["start_universe"] = it->GetUniverse();
                universe["number_of_universes"] = 1;
                universe["channels_per_universe"] = it->GetChannels();
                universe["start_channel"] = it->GetStartChannel() - out->GetStartChannel() + 1;
                universes.Append(universe);
            }
        }
        data["inputs"] = universes;
        data["system"]["operating_mode"] = out->GetType() == OUTPUT_E131 ? wxString("e1.31") : wxString("artnet");
    } else if (out->GetType() == OUTPUT_DDP) {
        data["system"]["operating_mode"] = wxString("ddp");
        DDPOutput* ddp = (DDPOutput*)out;
        data["system"]["start_channel"] = ddp->IsKeepChannelNumbers() ? ddp->GetStartChannel() : 1;
        if (ddp->IsKeepChannelNumbers()) {
            startChannel = 1;
        }
    } else  {
        //should never hit this
        DisplayError(wxString::Format(
                         "Invalud Input Type For Experience Controller %s.",
                         out->GetType())
                         .ToStdString());
        return startChannel;
    }

    return startChannel;
}

bool Experience::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* c, wxWindow* parent) {
    ControllerEthernet* controller = dynamic_cast<ControllerEthernet*>(c);
    if (controller == nullptr) {
        DisplayError(wxString::Format("%s is not a Experience controller.", c->GetName().c_str()));
        return false;
    }

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Experience Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    progress.Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    //first check rules
    auto rules = ControllerCaps::GetControllerConfig(controller);
    const bool success = cud.Check(rules, check);

    logger_base.debug(check);

    cud.Dump();
    if (!success) {
        DisplayError("Experience Upload Error:\n" + check, parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    bool const fullControl = rules->SupportsFullxLightsControl() && controller->IsFullxLightsControl();
    int const defaultBrightness = EncodeBrightness(controller->GetDefaultBrightnessUnderFullControl());

    logger_base.info("Initializing Pixel Output Information.");
    progress.Update(10, "Initializing Pixel Output Information.");

    wxJSONValue stringData;

    // get controller data from API
    if (!GetJSONData(GetConfigURL(), stringData)) {
        logger_base.error("Error connecting to Genius controller on %s.", (const char*)_ip.c_str());
        return false;
    }

    logger_base.info("Initializing Universe Input Information.");
    progress.Update(20, "Initializing Universe Input Information.");
    int32_t startChannel = SetInputUniverses(stringData, controller);

    if (-1 == startChannel) {
        logger_base.error("Error Calculating Universe Input Information.");
        return false;
    }

    logger_base.info("Figuring Out Pixel Output Information.");
    progress.Update(30, "Figuring Out Pixel Output Information.");
    //loop to setup string outputs
    for (int p = 1; p <= GetNumberOfPixelOutputs(); p++) {
        wxJSONValue port;
        port["long_range_port_index"].SetType(wxJSONTYPE_NULL);
        if (cud.HasPixelPort(p)) {
            UDControllerPort* portData = cud.GetControllerPixelPort(p);
            portData->CreateVirtualStrings(false);
            for (const auto& pvs : portData->GetVirtualStrings()) {
                wxJSONValue vs;
                vs["n"] = pvs->_description;
                vs["sc"] = pvs->_startChannel - startChannel + 1;
                vs["ec"] = pvs->Channels() / pvs->_channelsPerPixel;
                if (pvs->_reverseSet && pvs->_reverse == "Reverse") {
                    vs["r"] = true;
                }
                if (pvs->_gammaSet) {
                    vs["g"] = EncodeGamma(pvs->_gamma);
                }
                if (pvs->_brightnessSet) {
                    vs["b"] = EncodeBrightness(pvs->_brightness);
                } else if (fullControl) {
                    vs["b"] = defaultBrightness;
                }
                if (pvs->_startNullPixelsSet) {
                    vs["sn"] = pvs->_startNullPixels;
                }
                if (pvs->_endNullPixelsSet) {
                    vs["en"] = pvs->_endNullPixels;
                }
                if (pvs->_colourOrderSet) {
                    vs["st"] = EncodeColorOrder(pvs->_colourOrder);
                }
                port["virtual_strings"].Append(vs);
            }
            port["disabled"] = false;
            stringData["outputs"][p - 1] = port;
        } else if (fullControl) {
            wxJSONValue vs;
            vs["sc"] = 1;
            vs["ec"] = 1;
            port["virtual_strings"].Append(vs);
            stringData["outputs"][p - 1] = port;
        }
    }

    for (int lrIdx = 0; lrIdx < GetNumberOfRemoteOutputs(); ++lrIdx) {
        std::unordered_set<int> remoteIds;
        for (int subID = 0; subID < 4; ++subID) {
            wxJSONValue port;
            port["long_range_port_index"] = lrIdx;
            int portID = GetNumberOfPixelOutputs() + (lrIdx * 4) + subID + 1;
            //one based
            if (cud.HasPixelPort(portID)) {
                UDControllerPort* portData = cud.GetControllerPixelPort(portID);
                portData->CreateVirtualStrings(false);

                for (const auto& pvs : portData->GetVirtualStrings()) {
                    wxJSONValue vs;
                    vs["n"] = pvs->_description;
                    vs["sc"] = pvs->_startChannel - startChannel + 1;
                    vs["ec"] = pvs->Channels() / pvs->_channelsPerPixel;
                    if (pvs->_smartRemote > 0) {
                        vs["ri"] = pvs->_smartRemote - 1;
                    }
                    remoteIds.insert(pvs->_smartRemote);
                    if (pvs->_reverseSet && pvs->_reverse == "Reverse") {
                        vs["r"] = true;
                    }
                    if (pvs->_gammaSet) {
                        vs["g"] = EncodeGamma(pvs->_gamma);
                    }
                    if (pvs->_brightnessSet) {
                        vs["b"] = EncodeBrightness(pvs->_brightness);
                    } else if (fullControl) {
                        vs["b"] = defaultBrightness;
                    }
                    if (pvs->_startNullPixelsSet) {
                        vs["sn"] = pvs->_startNullPixels;
                    }
                    if (pvs->_endNullPixelsSet) {
                        vs["en"] = pvs->_endNullPixels;
                    }
                    if (pvs->_colourOrderSet) {
                        vs["st"] =  EncodeColorOrder(pvs->_colourOrder);
                    }
                    port["virtual_strings"].Append(vs);
                }
                port["disabled"] = false;
                stringData["outputs"][portID - 1] = port;
            } else if (fullControl) {
                wxJSONValue vs;
                vs["sc"] = 1;
                vs["ec"] = 1;
                port["virtual_strings"].Append(vs);
                stringData["outputs"][portID - 1] = port;
            }
        }
        if (remoteIds.size() != 0) {
            stringData["long_range_ports"][lrIdx]["number_of_receivers"] = remoteIds.size();
            stringData["long_range_ports"][lrIdx]["type"] = wxString("pixel");
        } else if (fullControl) {
            stringData["long_range_ports"][lrIdx]["number_of_receivers"] = 1;
            stringData["long_range_ports"][lrIdx]["type"] = wxString("pixel");
        }
    }

    logger_base.info("Figuring Out DMX Output Information.");
    progress.Update(50, "Figuring Out DMX Output Information.");

    for (int sp = 1; sp <= GetNumberOfSerial(); sp++) {
        wxJSONValue sport;
        sport["long_range_port_index"] = GetNumberOfRemoteOutputs() + sp - 1;
        int portID = GetNumberOfPixelOutputs() + (GetNumberOfRemoteOutputs() * 4) + sp;
        int lrIdx = GetNumberOfRemoteOutputs() + sp - 1;
        if (cud.HasSerialPort(sp)) {
            UDControllerPort* portData = cud.GetControllerSerialPort(sp);
            wxJSONValue vs;
            
            vs["sc"] = portData->GetStartChannel() - startChannel + 1;
            vs["ec"] = portData->Channels();
            
            sport["virtual_strings"].Append(vs);
            sport["disabled"] = false;
            stringData["outputs"][portID - 1] = sport;

            stringData["long_range_ports"][lrIdx]["number_of_receivers"] = 1;
            stringData["long_range_ports"][lrIdx]["type"] = wxString("dmx");
        } else if (fullControl) {
            wxJSONValue vs;
            vs["sc"] = 1;
            vs["ec"] = 1;
            sport["virtual_strings"].Append(vs);
            stringData["outputs"][portID - 1] = sport;
            stringData["long_range_ports"][lrIdx]["number_of_receivers"] = 1;
            stringData["long_range_ports"][lrIdx]["type"] = wxString("dmx");
        }
    }

    logger_base.info("Uploading Output Information.");
    progress.Update(70, "Uploading String Output Information.");

    PostJSONToURL(GetConfigURL(), stringData);

    progress.Update(100, "Done.");
    return true;
}
#pragma endregion


