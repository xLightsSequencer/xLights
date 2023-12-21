/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Falcon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"

#ifndef DISCOVERONLY
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../utils/Curl.h"
#include "ControllerUploadData.h"
#endif

#include "../outputs/ControllerEthernet.h"
#include "../outputs/DDPOutput.h"
#include "ControllerCaps.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>
#include <thread>

#pragma region V4

std::string Falcon::SendToFalconV4(std::string msg)
{
    return PutURL("/api", msg, "", "", "application/json");
}

std::vector<std::string> Falcon::V4_GetMediaFiles()
{
    // {"T":"Q","M":"WV","B":0,"E":0,"I":0,"P":{}}
    // {"R":200,"T":"Q","F":1,"B":0,"M":"IN","RB":0,"P":{"F":["xxx.wav"]},"W":" ","L":""}

    std::vector<std::string> res;

    bool success = true;
    bool done = false;
    int batch = 0;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    while (!done) {
        bool finalCall;
        int outBatch;
        bool reboot;
        wxJSONValue outParams;
        if (CallFalconV4API("Q", "WV", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {

            const wxJSONInternalArray* parr = outParams["F"].AsArray();

            for (size_t i = 0; i < parr->Count(); i++) {
                res.push_back(parr->Item(i)["f"].AsString());
            }

            batch++;
            if (finalCall) done = true;
        }
        else {
            done = true;
            res.clear();
            success = false;
        }
    }

    return res;
}

bool Falcon::V4_IsFileUploading()
{
    int batch = 0;
    bool finalCall;
    int outBatch;
    bool reboot;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    wxJSONValue outParams;

    bool uploading = true;
    if (CallFalconV4API("Q", "WD", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {
        int d = outParams["D"].AsInt();

        if (d == 1) uploading = false;
    }
    else {
        uploading = false;
    }

    return uploading;
}

int Falcon::V4_GetConversionProgress()
{
    int batch = 0;
    bool finalCall;
    int outBatch;
    bool reboot;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    wxJSONValue outParams;
    if (CallFalconV4API("Q", "WD", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {
        int d = outParams["D"].AsInt();

        if (d == 1) return 100;

        int mp3 = outParams["MP3"].AsInt();

        if (mp3 == 100) return 99;

        return mp3;
    }
    else {
        return 100;
    }
}

int Falcon::CallFalconV4API(const std::string& type, const std::string& method, int inbatch, int expected, int index, const wxJSONValue& params, bool& finalCall, int& outbatch, bool& reboot, wxJSONValue& result)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // {"T":"S","M":"IN","B":0,"E":1,"I":0,"P":{"A":[{"u":1,"c":512,"uc":16,"p":"e"}]}}
    // {"R":200,"T":"S","M":"IN","F":1,"B":0,"RB":0,"P":{},"W":"","L":""}

    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 0);
    wxString p;
    writer.Write(params, p);

    std::string send = wxString::Format("{\"T\":\"%s\",\"M\":\"%s\",\"B\":%d,\"E\":%d,\"I\":%d,\"P\":%s}", type, method, inbatch, expected, index, p).ToStdString();

    std::string res = SendToFalconV4(send);

    logger_base.debug(res);

    if (res == "") return 506;

    wxJSONValue resJson;
    wxJSONReader reader;
    reader.Parse(res, &resJson);

    int resInt = resJson["R"].AsInt();
    finalCall = resJson["F"].AsInt() == 1;
    outbatch = resJson["B"].AsInt();
    reboot = resJson["RB"].AsInt() == 1;
    result = resJson["P"];

    return resInt;
}

bool Falcon::V4_GetInputs(std::vector<FALCON_V4_INPUTS>& res)
{
    // {"T":"Q","M":"IN","B":0,"E":0,"I":0,"P":{}}
    // {"R":200,"T":"Q","F":1,"B":0,"M":"IN","RB":0,"P":{"A":[{"p":"e","u":1,"c":512,"uc":16}]},"W":" ","L":""}
    res.clear();
    bool success = true;
    bool done = false;
    int batch = 0;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    while (!done) {
        bool finalCall;
        int outBatch;
        bool reboot;
        wxJSONValue outParams;
        if (CallFalconV4API("Q", "IN", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {

            const wxJSONInternalArray* parr = outParams["A"].AsArray();

            for (size_t i = 0; i < parr->Count(); i++) {
                FALCON_V4_INPUTS input;
                input.universe = parr->Item(i)["u"].AsInt();
                input.channels = parr->Item(i)["c"].AsInt();
                input.universeCount = parr->Item(i)["uc"].AsInt();
                input.protocol = parr->Item(i)["p"].AsString() == "e" ? 0 : 1;
                res.push_back(input);
            }

            batch++;
            if (finalCall) done = true;
        }
        else {
            done = true;
            res.clear();
            success = false;
        }
    }

    return success;
}

#define FALCON_V4_SEND_INPUT_BATCH_SIZE 10
bool Falcon::V4_SendInputs(std::vector<FALCON_V4_INPUTS>& res, bool& reboot)
{
    // {"T":"S","M":"IN","B":0,"E":1,"I":0,"P":{"A":[{"u":1,"c":512,"uc":16,"p":"e"}]}}
    // {"R":200,"T":"S","M":"IN","F":1,"B":0,"RB":0,"P":{},"W":"","L":""}

    size_t batches = res.size() / FALCON_V4_SEND_INPUT_BATCH_SIZE + 1;
    if (res.size() % FALCON_V4_SEND_INPUT_BATCH_SIZE == 0 && res.size() != 0) --batches;

    size_t left = res.size();

    bool success = true;
    int batch = 0;
    while (success && left > 0) {

        std::string params = "{\"A\":[";

        for (size_t i = batch * FALCON_V4_SEND_INPUT_BATCH_SIZE; i < (batch + 1) * FALCON_V4_SEND_INPUT_BATCH_SIZE && i < res.size(); ++i) {
            if (batch != 0) params += ",";
            params += wxString::Format("{\"u\":%d,\"c\":%d,\"uc\":%d,\"p\":\"%c\"}", res[i].universe, res[i].channels, res[i].universeCount, (res[i].protocol == 0 ? 'e' : 'a')).ToStdString();
            --left;
        }

        params += "]}";

        wxJSONValue p;
        wxJSONReader reader;
        reader.Parse(params, &p);

        bool finalCall;
        int outBatch;
        wxJSONValue outParams;
        if (CallFalconV4API("S", "IN", batch, res.size(), batch * FALCON_V4_SEND_INPUT_BATCH_SIZE, p, finalCall, outBatch, reboot, outParams) == 200) {
            ++batch;
        }
        else             {
            success = false;
        }
    }

    return success;
}

// start channel is one based
bool Falcon::V4_SendBoardMode(int boardMode, int controllerMode, unsigned long startChannel, bool& reboot)
{
    // {"T":"S","M":"BM","B":0,"E":0,"I":0,"P":{"B":0,"O":2,"ps":0}}
    // {"R":200,"T":"S","M":"BM","F":1,"B":0,"RB":1,"P":{},"W":" ","L":""}

    bool success = true;

    std::string params = wxString::Format("{\"B\":%d,\"O\":%d,\"ps\":%lu}", boardMode, controllerMode, startChannel - 1).ToStdString();
    wxJSONValue p;
    wxJSONReader reader;
    reader.Parse(params, &p);

    bool finalCall;
    int outBatch;
    wxJSONValue outParams;
    if (CallFalconV4API("S", "BM", 0, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {
    }
    else {
        success = false;
    }

    return success;
}

bool Falcon::V4_GetStatus(wxJSONValue& res)
{
    // {"T":"Q","M":"ST","B":0,"E":0,"I":0,"P":{}}
    // {"R":200,"T":"Q","RB":0,"F":0,"B":0,"M":"ST","P":{"SD":"D","I":"192.168.0.124","K":"255.255.255.0","G":"192.168.0.1","D":"198.153.192.50","U":284,"N":"Falcon_F16V4","O":0,"BR":16,"V":"F16V4 v1.00","WI":"10.124.0.1","WK":"255.255.255.0","WG":"0.0.0.0","WD":"0.0.0.0","WSD":"D","WS":"","WP":"","CP":""},"W":" ","L":""}
    // {"T":"Q","M":"ST","B":1,"E":0,"I":0,"P":{}}
    // {"R":200,"T":"Q","RB":0,"F":1,"B":1,"M":"ST","P":{"sm":0,"su":0,"ssc":0,"sr":250000,"T1":341,"T2":327,"V1":115,"V2":111,"PT":380,"FN":2550,"UC":16,"C":"02-fe-00-18-00-3b","P":16,"S":0,"A":0,"B":0,"UG":{"s":"00000001","v":"1.1","d":"v11.zip"},"FW":2,"TG":"255.255.255.255","DP":10000,"DM":3,"DL":4,"SY":"N","PR":"N","Z":65535,"BT":6,"ps":0,"NT":"time.nist.gov","TZ":0,"DST":"N","IN":"Y","FS":32,"BS":50,"DT":"2021-04-17","TM":"04:12:48","CC":16777215},"W":" ","L":""}

    bool success = true;
    for (const auto& n : res.GetMemberNames()) {
        res.Remove(n);
    }

    bool done = false;
    int batch = 0;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    while (!done) {
        bool finalCall;
        int outBatch;
        bool reboot;
        wxJSONValue outParams;
        if (CallFalconV4API("Q", "ST", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {

            for (const auto& n : outParams.GetMemberNames()) {
                res[n] = wxJSONValue(outParams[n]);
            }

            ++batch;
            if (finalCall) done = true;
        }
        else {
            done = true;
            for (const auto& n : res.GetMemberNames()) {
                res.Remove(n);
            }
            success = false;
        }
    }

    return success;
}

bool Falcon::V4_SetSerialConfig(int protocol, int universe, int startChannel, int rate)
{
    // {"T":"S","M":"SE","B":0,"E":0,"I":0,"P":{"sm":0,"su":0,"ssc":0,"sr":250000}}
    // {"R":200,"T":"S","M":"SE","F":1,"B":0,"RB":0,"P":{},"W":" ","L":""}

    bool success = true;

    std::string params = wxString::Format("{\"sm\":%d,\"su\":%d,\"ssc\":%d,\"sr\":%d}", protocol, universe, startChannel, rate).ToStdString();
    wxJSONValue p;
    wxJSONReader reader;
    reader.Parse(params, &p);

    bool finalCall;
    int outBatch;
    bool reboot;
    wxJSONValue outParams;
    if (CallFalconV4API("S", "SE", 0, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {
    }
    else {
        success = false;
    }

    return success;
}

void Falcon::V4_GetStartChannel(int modelUniverse, int modelUniverseStartChannel, unsigned long modelStartChannel, int& universe, unsigned long& startChannel, bool oneBased, uint32_t controllerFirstChannel)
{
    if (_v4status["A"].AsInt() == 0) {
        universe = 0;
        startChannel = modelStartChannel - 1;
        if (oneBased)
            startChannel -= (controllerFirstChannel - 1);
    }
    else {
        universe = modelUniverse;
        startChannel = modelUniverseStartChannel - 1;
    }
}

bool Falcon::V4_IsValidStartChannel(Controller* controller, int universe, long startChannel)
{
    if (_v4status["A"].AsInt() == 0) {
        if (universe != 0) return false;
        if (startChannel < 0 || startChannel >= controller->GetChannels()) return false;
    }
    else {
        int firstUniverse = controller->GetFirstOutput()->GetUniverse();
        int lastUniverse = firstUniverse + controller->GetOutputCount() - 1;
        if (universe < firstUniverse || universe > lastUniverse) return false;
        if (startChannel < 0 || startChannel >= controller->GetFirstOutput()->GetChannels()) return false;
    }
    return true;
}

bool Falcon::V4_GetStrings(std::vector<FALCON_V4_STRING>& res)
{
    // {"T":"Q","M":"SP","B":4,"E":0,"I":0,"P":{}}
    // {"R":200,"T":"Q","F":0,"B":3,"M":"SP","RB":0,"P":{"A":[{"p":9,"s":0,"r":0,"v":0,"u":10,"sc":0,"n":100,"z":0,"ns":0,"ne":0,"g":10,"o":0,"b":40,"gp":1,"nm":"Port 10","bl":0,"l":13},{"p":10,"t":"P","s":0,"r":0,"v":0,"u":11,"sc":0,"n":100,"z":0,"ns":0,"ne":0,"g":10,"o":0,"b":40,"gp":1,"nm":"Port 11","bl":0,"l":13},{"p":11,"t":"P","s":0,"r":0,"v":0,"u":12,"sc":0,"n":100,"z":0,"ns":0,"ne":0,"g":10,"o":0,"b":40,"gp":1,"nm":"Port 12","bl":0,"l":13}]},"W":" ","L":""}
    res.clear();
    bool success = true;
    bool done = false;
    int batch = 0;
    wxJSONValue p(wxJSONTYPE_OBJECT);
    while (!done) {
        bool finalCall;
        int outBatch;
        bool reboot;
        wxJSONValue outParams;
        if (CallFalconV4API("Q", "SP", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {

            const wxJSONInternalArray* parr = outParams["A"].AsArray();

            for (size_t i = 0; i < parr->Count(); i++) {

                FALCON_V4_STRING str;
                str.port = parr->Item(i)["p"].AsInt(); // p
                str.string = parr->Item(i)["s"].AsInt(); // s
                str.smartRemote = parr->Item(i)["r"].AsInt(); // r
                str.universe = parr->Item(i)["u"].AsInt(); // u
                str.name = parr->Item(i)["nm"].AsString(); // nm
                str.blank = parr->Item(i)["bl"].AsInt(); // bl
                str.gamma = parr->Item(i)["g"].AsInt(); // g
                str.brightness = parr->Item(i)["b"].AsInt(); // b
                str.zigcount = parr->Item(i)["z"].AsInt(); // z
                str.endNulls = parr->Item(i)["ne"].AsInt(); // ne
                str.startNulls = parr->Item(i)["ns"].AsInt(); // ns
                str.colourOrder = parr->Item(i)["o"].AsInt(); // o
                str.direction = parr->Item(i)["v"].AsInt(); // v
                str.group = parr->Item(i)["gp"].AsInt(); // gp
                str.pixels = parr->Item(i)["n"].AsInt(); // n
                str.protocol = parr->Item(i)["l"].AsInt();
                str.startChannel = parr->Item(i)["sc"].AsInt();
                res.push_back(str);
            }

            ++batch;
            if (finalCall) done = true;
        }
        else {
            done = true;
            res.clear();
            success = false;
        }
    }

    return success;
}


void Falcon::V4_DumpStrings(const std::vector<FALCON_V4_STRING>& str)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int lastPort = -1;
    int lastRemote = -1;

    for (const auto& it : str) {
        if (it.port != lastPort) {
            logger_base.debug("  Port %d %s", it.port + 1, (const char*)(it.port % 16 == 0 ? V4_DecodePixelProtocol(it.protocol).c_str() : ""));
            lastRemote = 0;
            lastPort = it.port;
        }
        if (it.smartRemote != lastRemote) {
            logger_base.debug("    Remote %c", char(it.smartRemote + 64));
        }
        logger_base.debug("      String %d '%s' Universe %d StartChannel %d Pixels %d", it.string + 1, (const char *)it.name.c_str(), it.universe, it.startChannel + 1, it.pixels);
    }
}

#define FALCON_V4_SEND_STRING_BATCH_SIZE 1
bool Falcon::V4_SendOutputs(std::vector<FALCON_V4_STRING>& res, int addressingMode, unsigned long startChannel, bool& reboot)
{
    // {"T":"S","M":"SP","B":14,"E":16,"I":14,"P":{"AD":1,"B":0,"ps":0,"A":[{"t":"P","l":13,"p":14,"r":0,"s":0,"v":0,"u":15,"sc":0,"n":100,"z":0,"ns":0,"ne":0,"g":10,"o":0,"b":40,"gp":1,"nm":"Port 15","bl":0}]}}
    // {"R":200,"T":"S","M":"SP","F":0,"B":0,"RB":0,"P":{},"W":" ","L":""}

    // strings must be in port order. Within port they must be in smart remote order. Within smart remote they must be in string order.

    size_t batches = res.size() / FALCON_V4_SEND_STRING_BATCH_SIZE + 1;
    if (res.size() % FALCON_V4_SEND_STRING_BATCH_SIZE == 0 && res.size() != 0) --batches;

    size_t left = res.size();

    bool success = true;
    int batch = 0;
    while (success && left > 0) {

        // a board mode of 255 means dont change anything
        std::string params = wxString::Format("{\"AD\":%d,\"B\":%d,\"ps\":-10,\"A\":[", addressingMode, 255, startChannel).ToStdString();

        for (size_t i = batch * FALCON_V4_SEND_STRING_BATCH_SIZE; i < (batch + 1) * FALCON_V4_SEND_STRING_BATCH_SIZE && i < res.size(); i++) {
            if (batch != 0 || i != 0) params += ",";
            params += wxString::Format("{\"l\":%d,\"p\":%d,\"r\":%d,\"s\":%d,\"v\":%d,\"u\":%d,\"sc\":%d,\"n\":%d,\"z\":%d,\"ns\":%d,\"ne\":%d,\"g\":%d,\"o\":%d,\"b\":%d,\"gp\":%d,\"nm\":\"%s\",\"bl\":%d}",
                res[i].protocol,
                res[i].port,
                res[i].smartRemote,
                res[i].string,
                res[i].direction,
                res[i].universe,
                res[i].startChannel,
                res[i].pixels,
                res[i].zigcount,
                res[i].startNulls,
                res[i].endNulls,
                res[i].gamma,
                res[i].colourOrder,
                res[i].brightness,
                res[i].group,
                res[i].name,
                res[i].blank
                ).ToStdString();
            --left;
        }

        params += "]}";

        wxJSONValue p;
        wxJSONReader reader;
        reader.Parse(params, &p);

        bool finalCall;
        int outBatch;
        wxJSONValue outParams;
        if (CallFalconV4API("S", "SP", batch, res.size(), batch * FALCON_V4_SEND_STRING_BATCH_SIZE, p, finalCall, outBatch, reboot, outParams) == 200) {
            ++batch;
            wxMilliSleep(50);
        }
        else {
            success = false;
        }
    }

    return success;
}

#define V4_CONTROLLERMODE_E131_ARTNET 0
#define V4_CONTROLLERMODE_ZCPP 1
#define V4_CONTROLLERMODE_DDP 2
#define V4_CONTROLLERMODE_FPPREMOTE 3
#define V4_CONTROLLERMODE_FPPMASTER 4
#define V4_CONTROLLERMODE_FPPPLAYER 5

#define V4_PIXEL_PROTOCOL_APA102 0
#define V4_PIXEL_PROTOCOL_APA109 1
#define V4_PIXEL_PROTOCOL_DMX512P 2
#define V4_PIXEL_PROTOCOL_LPD6803 3
#define V4_PIXEL_PROTOCOL_LPD8806 4
#define V4_PIXEL_PROTOCOL_SM16716 5
#define V4_PIXEL_PROTOCOL_TLS3001 6
#define V4_PIXEL_PROTOCOL_TM1814 7
#define V4_PIXEL_PROTOCOL_TM1829 8
#define V4_PIXEL_PROTOCOL_UCS8903 9
#define V4_PIXEL_PROTOCOL_UCS8903_16 10
#define V4_PIXEL_PROTOCOL_UCS8904 11
#define V4_PIXEL_PROTOCOL_UCS8904_16 12
#define V4_PIXEL_PROTOCOL_WS2801 13
#define V4_PIXEL_PROTOCOL_WS2811 14
#define V4_PIXEL_PROTOCOL_WS2811_SLOW 15
#define V4_PIXEL_PROTOCOL_SJ1221 16
#define V4_PIXEL_PROTOCOL_DMX512P_4 17

std::string Falcon::V4_DecodeMode(int mode) const
{
    switch (mode) {
    case V4_CONTROLLERMODE_E131_ARTNET: return "E1.31/ArtNET";
    case V4_CONTROLLERMODE_ZCPP: return "ZCPP";
    case V4_CONTROLLERMODE_DDP: return "DDP";
    case V4_CONTROLLERMODE_FPPREMOTE: return "Remote";
    case V4_CONTROLLERMODE_FPPMASTER: return "Master";
    case V4_CONTROLLERMODE_FPPPLAYER: return "Player";
    default: break;
    }
    return "Unknown";
}

std::string Falcon::V4_DecodePixelProtocol(int protocol)
{
    switch (protocol) {
    case V4_PIXEL_PROTOCOL_APA102: return "apa102";
    case V4_PIXEL_PROTOCOL_APA109: return "apa109";
    case V4_PIXEL_PROTOCOL_DMX512P: return "dmx512p";
    case V4_PIXEL_PROTOCOL_DMX512P_4:
        return "dmx512p-4";
    case V4_PIXEL_PROTOCOL_LPD6803:
        return "lpd6803";
    case V4_PIXEL_PROTOCOL_LPD8806: return "lpd8806";
    case V4_PIXEL_PROTOCOL_SM16716: return "sm16716";
    case V4_PIXEL_PROTOCOL_TLS3001: return "tls3001";
    case V4_PIXEL_PROTOCOL_TM1814: return "tm1814";
    case V4_PIXEL_PROTOCOL_TM1829: return "tm1829";
    case V4_PIXEL_PROTOCOL_UCS8903: return "ucs8903";
    case V4_PIXEL_PROTOCOL_UCS8903_16: return "ucs8903 16 bit";
    case V4_PIXEL_PROTOCOL_UCS8904: return "ucs8904";
    case V4_PIXEL_PROTOCOL_UCS8904_16: return "ucs8904 16 bit";
    case V4_PIXEL_PROTOCOL_WS2801: return "ws2801";
    case V4_PIXEL_PROTOCOL_WS2811: return "ws2811";
    case V4_PIXEL_PROTOCOL_WS2811_SLOW:
        return "ws2811 slow";
    case V4_PIXEL_PROTOCOL_SJ1221:
        return "sj1221";
    }
    return "";
}

int Falcon::V4_EncodePixelProtocol(const std::string& protocol)
{
    if (protocol == "apa102") return V4_PIXEL_PROTOCOL_APA102;
    if (protocol == "apa109") return V4_PIXEL_PROTOCOL_APA109;
    if (protocol == "dmx512p") return V4_PIXEL_PROTOCOL_DMX512P;
    if (protocol == "dmx512p-4")
        return V4_PIXEL_PROTOCOL_DMX512P_4;
    if (protocol == "lpd6803")
        return V4_PIXEL_PROTOCOL_LPD6803;
    if (protocol == "lpd8806") return V4_PIXEL_PROTOCOL_LPD8806;
    if (protocol == "sm16716") return V4_PIXEL_PROTOCOL_SM16716;
    if (protocol == "tls3001") return V4_PIXEL_PROTOCOL_TLS3001;
    if (protocol == "tm1814") return V4_PIXEL_PROTOCOL_TM1814;
    if (protocol == "tm1829") return V4_PIXEL_PROTOCOL_TM1829;
    if (protocol == "ucs8903") return V4_PIXEL_PROTOCOL_UCS8903;
    if (protocol == "ucs8903 16 bit") return V4_PIXEL_PROTOCOL_UCS8903_16;
    if (protocol == "ucs8904") return V4_PIXEL_PROTOCOL_UCS8904;
    if (protocol == "ucs8904 16 bit") return V4_PIXEL_PROTOCOL_UCS8904_16;
    if (protocol == "ws2801") return V4_PIXEL_PROTOCOL_WS2801;
    if (protocol == "ws2811") return V4_PIXEL_PROTOCOL_WS2811;
    if (protocol == "ws2811 slow") return V4_PIXEL_PROTOCOL_WS2811_SLOW;

    if (protocol == "tm18xx") return V4_PIXEL_PROTOCOL_WS2811;
    if (protocol == "lx1203") return V4_PIXEL_PROTOCOL_WS2811;
    if (protocol == "sj1221")
        return V4_PIXEL_PROTOCOL_SJ1221;
    return -1;
}

#define V4_BOARDCONFIG_16 0
#define V4_BOARDCONFIG_16_2SR 1
#define V4_BOARDCONFIG_16_16 2
#define V4_BOARDCONFIG_16_16_2SR 3
#define V4_BOARDCONFIG_16_4SR 4
#define V4_BOARDCONFIG_16_4SR_2SR 5
#define V4_BOARDCONFIG_16_16_16 6
#define V4_BOARDCONFIG_16_16_4SR 7
#define V4_BOARDCONFIG_16_4SR_16 8
#define V4_BOARDCONFIG_16_4SR_4SR 9
#define V4_BOARDCONFIG_4SR_4SR_4SR 10
#define V4_BOARDCONFIG_4SR_4SR 11

std::string Falcon::V4_DecodeBoardConfiguration(int config) const
{
    switch (config) {
    case V4_BOARDCONFIG_16: return "16 Local Ports";
    case V4_BOARDCONFIG_16_2SR: return "16 Local Ports + 2 Smart Receiver Chains on Receiver Ports";
    case V4_BOARDCONFIG_16_16: return "16 + 16 Local Ports";
    case V4_BOARDCONFIG_16_16_2SR: return "16 + 16 Local Ports + 2 Smart Receiver Chains on Receiver Ports";
    case V4_BOARDCONFIG_16_4SR: return "16 Local Ports + 4 Smart Receiver Chains";
    case V4_BOARDCONFIG_16_4SR_2SR: return "16 Local Ports + 4 Smart Receiver Chains + 2 Smart Receiver Chains on Receiver Ports";
    case V4_BOARDCONFIG_16_16_16: return "16 + 16 + 16 Local Ports (Ports 33-40 mirrored on Receiver Ports)";
    case V4_BOARDCONFIG_16_16_4SR: return "16 + 16 Local Ports + 4 Smart Receiver Chains";
    case V4_BOARDCONFIG_16_4SR_16: return "16 Local Ports + 4 Smart Receiver Chains + 16 Local Ports (Ports 33-40 mirrored on Receiver Ports)";
    case V4_BOARDCONFIG_16_4SR_4SR: return "16 Local Ports + 4 Smart Receiver Chains + 4 Smart Receiver Chains";
    case V4_BOARDCONFIG_4SR_4SR_4SR: return "12 Smart Receiver Chains";
    case V4_BOARDCONFIG_4SR_4SR: return "8 Smart Receiver Chains";
    default:
        break;
    }
    return "Unknown";
}

bool Falcon::V4_IsPortSmartRemoteEnabled(int boardMode, int port)
{
    switch (boardMode) {
    case V4_BOARDCONFIG_16:
        return false;
    case V4_BOARDCONFIG_16_2SR:
    case V4_BOARDCONFIG_16_16:
        return port >= 16 && port < 24;
    case V4_BOARDCONFIG_16_16_2SR:
    case V4_BOARDCONFIG_16_16_16:
        return port >= 32 && port < 40;
    case V4_BOARDCONFIG_16_4SR:
        return port >= 16 && port < 32;
    case V4_BOARDCONFIG_16_4SR_2SR:
    case V4_BOARDCONFIG_16_4SR_16:
        return port >= 16 && port < 40;
    case V4_BOARDCONFIG_16_16_4SR:
        return port >= 32 && port < 48;
    case V4_BOARDCONFIG_16_4SR_4SR:
        return port >= 16 && port < 48;
    case V4_BOARDCONFIG_4SR_4SR_4SR:
    case V4_BOARDCONFIG_4SR_4SR:
        return true;
    }
    return false;
}

int Falcon::V4_GetBoardPorts(int boardMode)
{
    switch (boardMode) {
    case V4_BOARDCONFIG_16:
        return 16;
    case V4_BOARDCONFIG_16_2SR:
        return 24;
    case V4_BOARDCONFIG_16_16:
    case V4_BOARDCONFIG_16_4SR:
    case V4_BOARDCONFIG_4SR_4SR:
        return 32;
    case V4_BOARDCONFIG_16_16_2SR:
    case V4_BOARDCONFIG_16_4SR_2SR:
        return 40;
    case V4_BOARDCONFIG_4SR_4SR_4SR:
    case V4_BOARDCONFIG_16_16_16:
    case V4_BOARDCONFIG_16_16_4SR:
    case V4_BOARDCONFIG_16_4SR_16:
    case V4_BOARDCONFIG_16_4SR_4SR:
        return 48;
    }
    return 0;
}

int Falcon::V4_GetMaxPortPixels(int boardMode, int protocol)
{
    if (V4_GetBoardPorts(boardMode) > 32) {
        switch (protocol) {
        case V4_PIXEL_PROTOCOL_APA102:
        case V4_PIXEL_PROTOCOL_TM1814:
            return 526;
        case V4_PIXEL_PROTOCOL_APA109: return 528;
        case V4_PIXEL_PROTOCOL_LPD8806:
        case V4_PIXEL_PROTOCOL_TM1829:
            return 703;
        case V4_PIXEL_PROTOCOL_SM16716: return 647;
        case V4_PIXEL_PROTOCOL_UCS8903:
        case V4_PIXEL_PROTOCOL_UCS8903_16:
        case V4_PIXEL_PROTOCOL_WS2811_SLOW:
            return 352;
        case V4_PIXEL_PROTOCOL_UCS8904:
        case V4_PIXEL_PROTOCOL_UCS8904_16:
            return 264;
        case V4_PIXEL_PROTOCOL_LPD6803:
        case V4_PIXEL_PROTOCOL_WS2801:
        case V4_PIXEL_PROTOCOL_WS2811:
            return 704;
        case V4_PIXEL_PROTOCOL_TLS3001:
            return 192;
        case V4_PIXEL_PROTOCOL_DMX512P:
            return 510;
        case V4_PIXEL_PROTOCOL_DMX512P_4:
            return 382;
        case V4_PIXEL_PROTOCOL_SJ1221:
            return 232;
        }
    }
    else {
        switch (protocol) {
        case V4_PIXEL_PROTOCOL_APA102:
        case V4_PIXEL_PROTOCOL_TM1814:
            return 766;
        case V4_PIXEL_PROTOCOL_APA109: return 768;
        case V4_PIXEL_PROTOCOL_LPD8806:
        case V4_PIXEL_PROTOCOL_TM1829:
            return 1023;
        case V4_PIXEL_PROTOCOL_SM16716: return 943;
        case V4_PIXEL_PROTOCOL_UCS8903:
        case V4_PIXEL_PROTOCOL_UCS8903_16:
        case V4_PIXEL_PROTOCOL_WS2811_SLOW:
            return 512;
        case V4_PIXEL_PROTOCOL_UCS8904:
        case V4_PIXEL_PROTOCOL_UCS8904_16:
            return 384;
        case V4_PIXEL_PROTOCOL_LPD6803:
        case V4_PIXEL_PROTOCOL_WS2801:
        case V4_PIXEL_PROTOCOL_WS2811:
            return 1024;
        case V4_PIXEL_PROTOCOL_TLS3001:
            return 288;
        case V4_PIXEL_PROTOCOL_DMX512P:
            return 743;
        case V4_PIXEL_PROTOCOL_DMX512P_4:
            return 557;
        case V4_PIXEL_PROTOCOL_SJ1221:
            return 339;
        }
    }

    return 0;
}

int Falcon::V4_EncodeInputProtocol(const std::string& protocol)
{
    if (protocol == OUTPUT_ARTNET) return 1;
    if (protocol == OUTPUT_E131) return 0;

    return 0;
}

int Falcon::V4_GetRebootSecs()
{
    if (_ip == _v4status["I"].AsString()) {
        return 8;
    }
    else         {
        return 20;
    }
}

void Falcon::V4_WaitForReboot(const std::string& name, wxWindow* parent)
{
    std::unique_ptr<wxProgressDialog> progress;
    progress.reset(new wxProgressDialog(wxString::Format("Rebooting controller '%s' ...", name), "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
    progress->Show();

    for (int i = 0; i < 100; i++) {
        progress->Update(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(V4_GetRebootSecs() * 1000 / 100));
    }
}

bool Falcon::V4_SetInputMode(Controller* controller, wxWindow* parent)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPMASTER ||
        _v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPPLAYER ||
        _v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPREMOTE) {

        auto sc = controller->GetStartChannel();
        if (_v4status["ps"].AsInt() + 1 != sc) {
            bool reboot = false;
            logger_base.debug("Controller in Player/Master/Remote mode. Setting controller start channel: %lu", sc);
            if (Falcon::V4_SendBoardMode(_v4status["B"].AsInt(), _v4status["O"].AsInt(), sc, reboot)) {
                if (reboot) {
                    V4_WaitForReboot(controller->GetName(), parent);
                }
                return true;
            }
            else {
                logger_base.error("Failed to set board mode.");
                return false;
            }
        }
        return true;
    }

    // Get universes based on IP
    auto protocol = controller->GetProtocol();

    if (protocol == OUTPUT_DDP) {

        wxASSERT(controller->GetOutput(0) != nullptr);
        DDPOutput* ddp = dynamic_cast<DDPOutput*>(controller->GetOutput(0));

        size_t ddpStart = ddp->IsKeepChannelNumbers() ? ddp->GetStartChannel() : 1;
        if (_v4status["O"].AsInt() != V4_CONTROLLERMODE_DDP || _v4status["ps"].AsInt() + 1 != ddpStart) {
            logger_base.debug("Setting controller to DDP. Start channel: %lu", ddpStart);
            bool reboot = false;
            if (Falcon::V4_SendBoardMode(_v4status["B"].AsInt(), V4_CONTROLLERMODE_DDP, ddpStart, reboot)) {
                if (reboot) {
                    V4_WaitForReboot(controller->GetName(), parent);
                }
                return true;
            }
            return false;
        }
    }
    else if (protocol == OUTPUT_E131 || protocol == OUTPUT_ARTNET) {
        auto sc = controller->GetStartChannel();
        if (_v4status["O"].AsInt() != V4_CONTROLLERMODE_E131_ARTNET || _v4status["ps"].AsInt() + 1 != sc) {

            logger_base.debug("Setting controller to E131/ArtNET. Start channel: %lu", sc);
            bool reboot = false;
            if (Falcon::V4_SendBoardMode(_v4status["B"].AsInt(), V4_CONTROLLERMODE_E131_ARTNET, sc, reboot)) {
                if (reboot) {
                    V4_WaitForReboot(controller->GetName(), parent);
                }
                if (!V4_GetStatus(_v4status)) {
                    logger_base.error("Failed to retrieve status after setting board mode.");
                    return false;
                }
            }
            else {
                logger_base.error("Failed to set board mode.");
                return false;
            }
        }
    }

    return true;
}

bool Falcon::V4_SetInputUniverses(Controller* controller, wxWindow* parent)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!V4_SetInputMode(controller, parent)) {
        return false;
    }

    auto protocol = controller->GetProtocol();
    if ((protocol == OUTPUT_E131 || protocol == OUTPUT_ARTNET) && 
        (_v4status["O"].AsInt() != V4_CONTROLLERMODE_FPPMASTER &&
        _v4status["O"].AsInt() != V4_CONTROLLERMODE_FPPPLAYER &&
        _v4status["O"].AsInt() != V4_CONTROLLERMODE_FPPREMOTE))
    {
        auto outputs = controller->GetOutputs();

        if (outputs.size() > 192) {
            DisplayError(wxString::Format("Attempt to upload %d universes to falcon v4 controller but only 192 are supported.", outputs.size()).ToStdString());
            return false;
        }

        if (outputs.size() > 0) {
            std::vector<FALCON_V4_INPUTS> inputs;

            FALCON_V4_INPUTS input;
            input.universe = outputs.front()->GetUniverse();
            input.channels = outputs.front()->GetChannels();
            input.protocol = V4_EncodeInputProtocol(outputs.front()->GetType());
            input.universeCount = outputs.size();
            inputs.push_back(input);

            bool reboot = false; // but this never will cause a reboot
            if (Falcon::V4_SendInputs(inputs, reboot)) {
                return true;
            }
            logger_base.error("Failed to set inputs.");
            return false;
        }
        return true;
    }

    // for zcpp we dont send anything
    return true;
}

// use a sr of -1 to return the first regardless of sr
int Falcon::V4_GetStringFirstIndex(const std::vector<FALCON_V4_STRING>& falconStrings, const int p, const int sr)
{
    int index = 0;
    for (const auto& it : falconStrings) {
        if (it.port == p && (sr == -1 || it.smartRemote == sr)) return index;
        ++index;
    }
    return -1;
}

int Falcon::V4_EncodeColourOrder(const std::string co) const
{
    if (co == "RGB") return 0;
    if (co == "RBG") return 1;
    if (co == "GRB") return 2;
    if (co == "GBR") return 3;
    if (co == "BRG") return 4;
    if (co == "BGR") return 5;
    return 0;
}

#ifndef DISCOVERYONLY

// force brightness value to a value the falcon supports
int Falcon::V4_ValidBrightness(int b) const
{
    if (b > 95) return 100;
    if (b > 85) return 90;
    if (b > 75) return 80;
    if (b > 65) return 70;
    if (b > 55) return 60;
    if (b > 45) return 50;
    if (b > 35) return 40;
    if (b > 25) return 30;
    if (b > 18) return 20;
    if (b > 12) return 15;
    if (b > 7) return 10;
    return 5;
}

int Falcon::V4_ValidGamma(int g) const
{
    if (g < 15) return 10;
    if (g < 22) return 20;
    if (g < 25) return 23;
    if (g < 27) return 25;
    if (g < 30) return 28;
    return 30;
}

void Falcon::V4_MakeStringsValid(Controller* controller, UDController& cud, std::vector<FALCON_V4_STRING>& str, int addressingMode)
{
    for (auto& it : str) {

        if (!V4_IsValidStartChannel(controller, it.universe, it.startChannel)) {
            V4_GetStartChannel(cud.GetFirstOutput()->GetUniverse(), 1, cud.GetFirstOutput()->GetStartChannel(), it.universe, it.startChannel, false, 0);
        }
    }
}

int Falcon::V4_GetBrightness(int port, int sr, int defaultBrightness, const std::vector<FALCON_V4_STRING>& falconStrings)
{
    for (const auto& it : falconStrings)
    {
        if (it.port == port && it.smartRemote == sr)
            return it.brightness;
    }
    return defaultBrightness;
}

bool Falcon::V4_PopulateStrings(std::vector<FALCON_V4_STRING>& uploadStrings, const std::vector<FALCON_V4_STRING>& falconStrings, UDController& cud, ControllerCaps* caps, int defaultBrightness, std::string& error, bool oneBased, uint32_t controllerFirstChannel, bool fullcontrol)
{
    bool success = true;

    // work out the number of smart remotes on each port
    std::vector<int> smartRemotes;
    smartRemotes.resize(caps->GetMaxPixelPort());

    for (int p = 0; p < caps->GetMaxPixelPort(); p++) {
        smartRemotes[p] = -1;
    }

    int banks = (caps->GetMaxPixelPort() + 15) / 16;
    std::vector<int> protocols;
    protocols.resize(banks);

    for (int b = 0; b < banks; b++) {
        protocols[b] = -1;
    }

    // lets start with what we downloaded
    for (const auto& it : falconStrings) {
        if (it.port < caps->GetMaxPixelPort()) {
            if (it.smartRemote > smartRemotes[it.port]) {
                if ((it.smartRemote > 0 && smartRemotes[it.port] <= 0) || (it.smartRemote == 0 && smartRemotes[it.port] > 0)) {
                    error = wxString::Format("Port %d has an invalid smart remote configuration.", it.port + 1);
                    return false;
                }
                else                 {
                    smartRemotes[it.port] = it.smartRemote;
                }
            }

            int b = it.port / 16;
            if (protocols[b] == -1 || protocols[b] == it.protocol) {
                protocols[b] = it.protocol;
            }
            else             {
                error = wxString::Format("Port %d has an invalid protocol configuration.", it.port + 1);
                return false;
            }
        }
    }

    if (!success) return success;

    // now lets check what we plan to upload
    for (int p = 0; success && p < cud.GetMaxPixelPort(); p++) {
        UDControllerPort* pp = cud.GetControllerPixelPort(p + 1);
        if (pp != nullptr) {
            if (pp->AtLeastOneModelIsUsingSmartRemote()) {
                for (const auto& it : pp->GetVirtualStrings()) {
                    if (it->_smartRemote > smartRemotes[p]) {
                        if ((it->_smartRemote > 0 && smartRemotes[p] == 0) || (it->_smartRemote == 0 && smartRemotes[p] > 0)) {
                            error = wxString::Format("Port %d has an invalid smart remote configuration.", p + 1);
                            return false;
                        }
                        else {
                            smartRemotes[p] = it->_smartRemote;
                        }
                    }
                }
            }

            int b = p / 16;
            int protocol = V4_EncodePixelProtocol(pp->GetProtocol());
            if (protocol != -1) {
                {
                    if (protocols[b] == -1 || protocols[b] == protocol) {
                        protocols[b] = protocol;
                    }
                    else {
                        error = wxString::Format("Port %d has an invalid protocol configuration.", p + 1);
                        return false;
                    }
                }
            }
        }
    }

    for (int b = 0; b < banks; b++) {
        if (protocols[b] == -1) protocols[b] = V4_PIXEL_PROTOCOL_WS2811;
    }

    if (!success) return success;

    // set the number of smart remotes to the same for each block of 4 ports
    for (int p = 0; p < caps->GetMaxPixelPort(); p += 4) {
        int srmax = 0;
        for (int pp = 0; pp < 4; pp++) {
            if (smartRemotes[p + pp] > srmax) srmax = smartRemotes[p + pp];
        }
        for (int pp = 0; pp < 4; pp++) {
            smartRemotes[p + pp] = srmax;
        }
    }

    for (int p = 0; p < caps->GetMaxPixelPort(); p++) {
        if (cud.HasPixelPort(p + 1) && cud.GetControllerPixelPort(p + 1)->GetModelCount() > 0) {
            // take data from cud
            auto pp = cud.GetControllerPixelPort(p + 1);

            int bank = p / 16;
            int maxPixels = V4_GetMaxPortPixels(_v4status["B"].AsInt(), protocols[bank]);
            if (pp->Pixels() > maxPixels) {
                error = wxString::Format("Port %d has too many pixels on it for the nominated board configuration/pixel type.", p + 1);
                return false;
            }

            pp->CreateVirtualStrings(true);
            for (int sr = smartRemotes[p] == 0 ? 0 : 1; sr < smartRemotes[p] + 1; sr++) {
                int gamma = 10;
                int brightness = defaultBrightness;
                int startNulls = 0;
                int endNulls = 0;
                int colourOrder = 0;
                int direction = 0;
                int group = 1;
                int s = 0;
                bool done = false;
                for (const auto& it : pp->GetVirtualStrings()) {
                    if (it->_smartRemote == sr) {
                        done = true;
                        FALCON_V4_STRING str;
                        str.port = p;
                        str.string = s++;
                        str.smartRemote = sr;
                        if (sr != 0 && !V4_IsPortSmartRemoteEnabled(_v4status["B"].AsInt(), p)) {
                            error = wxString::Format("Port %d does not support smart remotes.", p + 1);
                            return false;
                        }
                        str.name = SafeDescription(it->_description);
                        str.blank = false;
                        str.gamma = V4_ValidGamma(it->_gammaSet ? it->_gamma * 10 : gamma);
                        str.brightness = V4_ValidBrightness(it->_brightnessSet ? it->_brightness : (fullcontrol ? defaultBrightness : V4_GetBrightness(p, sr, defaultBrightness, falconStrings)));
                        str.zigcount = 0;
                        str.endNulls = it->_endNullPixelsSet ? it->_endNullPixels : 0;
                        str.startNulls = it->_startNullPixelsSet ? it->_startNullPixels : 0;
                        str.colourOrder = it->_colourOrderSet ? V4_EncodeColourOrder(it->_colourOrder) : colourOrder;
                        str.direction = it->_reverseSet ? (it->_reverse == "Forward" ? 0 : 1) : direction;
                        str.group = it->_groupCountSet ? it->_groupCount : group;
                        str.zigcount = it->_zigZagSet ? it->_zigZag : 0; // dont carry between props
                        str.pixels = INTROUNDUPDIV(it->Channels(), GetChannelsPerPixel(it->_protocol)) * str.group;
                        str.protocol = protocols[p / 16];
                        V4_GetStartChannel(it->_universe, it->_universeStartChannel, it->_startChannel, str.universe, str.startChannel, oneBased, controllerFirstChannel);

                        uploadStrings.push_back(str);

                        gamma = str.gamma;
                        brightness = defaultBrightness;
                        startNulls = str.startNulls;
                        endNulls = str.endNulls;
                        colourOrder = str.colourOrder;
                        direction = str.direction;
                        group = str.group;
                    }
                }
                if (!done) {
                    // create a default string
                    FALCON_V4_STRING str;
                    str.port = p;
                    str.string = 0;
                    str.smartRemote = sr;
                    str.name = wxString::Format("Port %d", p + 1);
                    str.blank = false;
                    str.gamma = 10;
                    str.brightness = fullcontrol ? defaultBrightness : V4_GetBrightness(p, 0, defaultBrightness, falconStrings);
                    str.zigcount = 0;
                    str.endNulls = 0;
                    str.startNulls = 0;
                    str.colourOrder = 0;
                    str.direction = 0;
                    str.group = 1;
                    str.pixels = 0;
                    str.protocol = protocols[p / 16];
                    V4_GetStartChannel(cud.GetFirstOutput()->GetUniverse(), 1, cud.GetFirstOutput()->GetStartChannel(), str.universe, str.startChannel, oneBased, controllerFirstChannel);
                    uploadStrings.push_back(str);
                }
            }
        }
        else if (V4_GetStringFirstIndex(falconStrings, p, -1) != -1) {
            // take it from strings
            for (int sr = smartRemotes[p] == 0 ? 0 : 1; sr < smartRemotes[p] + 1; sr++) {
                if (V4_GetStringFirstIndex(falconStrings, p, sr) != -1) {
                    int i = V4_GetStringFirstIndex(falconStrings, p, sr);
                    while (i < falconStrings.size() && falconStrings[i].port == p && falconStrings[i].smartRemote == sr) {
                        uploadStrings.push_back(falconStrings[i]);
                        ++i;
                    }
                }
                else                     {
                    // create a default string
                    FALCON_V4_STRING str;
                    str.port = p;
                    str.string = 0;
                    str.smartRemote = sr;
                    str.name = wxString::Format("Port %d", p + 1);
                    str.blank = false;
                    str.gamma = 10;
                    str.brightness = fullcontrol ? defaultBrightness : V4_GetBrightness(p, sr, defaultBrightness, falconStrings);
                    str.zigcount = 0;
                    str.endNulls = 0;
                    str.startNulls = 0;
                    str.colourOrder = 0;
                    str.direction = 0;
                    str.group = 1;
                    str.pixels = 0;
                    str.protocol = protocols[p / 16];
                    V4_GetStartChannel(cud.GetFirstOutput()->GetUniverse(), 1, cud.GetFirstOutput()->GetStartChannel(), str.universe, str.startChannel, oneBased, controllerFirstChannel);
                    uploadStrings.push_back(str);
                }
            }
        }
        else {
            for (int sr = smartRemotes[p] == 0 ? 0 : 1; sr < smartRemotes[p] + 1; sr++) {
                // create a default string
                FALCON_V4_STRING str;
                str.port = p;
                str.string = 0;
                str.smartRemote = sr;
                str.name = wxString::Format("Port %d", p + 1);
                str.blank = false;
                str.gamma = 10;
                str.brightness = fullcontrol ? defaultBrightness : V4_GetBrightness(p, sr, defaultBrightness, falconStrings);
                str.zigcount = 0;
                str.endNulls = 0;
                str.startNulls = 0;
                str.colourOrder = 0;
                str.direction = 0;
                str.group = 1;
                str.pixels = 0;
                str.protocol = protocols[p / 16];
                V4_GetStartChannel(cud.GetFirstOutput()->GetUniverse(), 1, cud.GetFirstOutput()->GetStartChannel(), str.universe, str.startChannel, oneBased, controllerFirstChannel);
                uploadStrings.push_back(str);
            }
        }
    }

    return success;
}

bool Falcon::V4_SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent, bool doProgress)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Falcon Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    bool success = true;

    std::unique_ptr<wxProgressDialog> progress;
    if (doProgress) {
        progress.reset(new wxProgressDialog("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
        progress->Show();
    }

    int defaultBrightness = V4_ValidBrightness(controller->GetDefaultBrightnessUnderFullControl());

    if (doProgress) progress->Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    auto caps = ControllerCaps::GetControllerConfig(controller);

    if (caps == nullptr) return false;

    success = cud.Check(caps, check);

    logger_base.debug(check);

    cud.Dump();

    if (cud.GetMaxPixelPort() > 0 && caps->GetMaxPixelPort() > 0 && UDController::IsError(check)) {
        DisplayError("Not uploaded due to errors.\n" + check);
        check = "";
        if (doProgress) progress->Update(100, "Aborting.");
        return false;
    }

    if (!V4_SetInputMode(controller, parent)) {
        if (doProgress) progress->Update(100, "Aborting.");
        return false;
    }

    bool fullcontrol = caps->SupportsFullxLightsControl() && controller->IsFullxLightsControl();

    std::vector<FALCON_V4_STRING> falconStrings;
    if (!fullcontrol) {

        if (_v4status["B"].AsInt() != wxAtoi(caps->GetCustomPropertyByPath("v4BoardMode", "99"))) {
            logger_base.debug("Current board mode: %d. desired board mode: %d", _v4status["B"].AsInt(), wxAtoi(caps->GetCustomPropertyByPath("v4BoardMode", "0")));
            DisplayError("Falcon Outputs Upload: Board is currently set to the wrong mode. Please correct it.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        if (doProgress) progress->Update(40, "Rerieving strings.");

        if (!V4_GetStrings(falconStrings)) {
            DisplayError("Falcon Outputs Upload: Failed to retrieve current string configuration.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        // we need to make sure all the returned strings are valid against the input channels
        V4_MakeStringsValid(controller, cud, falconStrings, _v4status["A"].AsInt());

        logger_base.info("Retrieved falcon string configuration.");
        V4_DumpStrings(falconStrings);
    }
    else {

        if (_v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPMASTER ||
            _v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPPLAYER ||
            _v4status["O"].AsInt() == V4_CONTROLLERMODE_FPPREMOTE) {
            // we dont validate inputs
        }
        else {
            // validate the inputs look correct
            std::vector<FALCON_V4_INPUTS> inputs;
            if (V4_GetInputs(inputs)) {
                if (controller->GetProtocol() == OUTPUT_ARTNET || controller->GetProtocol() == OUTPUT_E131) {
                    if (inputs.size() != 1) {
                        logger_base.debug("Board has %lu inputs where it should just have 1.", inputs.size());
                        DisplayError("Falcon inputs not as expected. Upload inputs to correct.", parent);
                        if (doProgress) progress->Update(100, "Aborting.");
                        return false;
                    }

                    if (inputs.front().universe != controller->GetOutput(0)->GetUniverse() ||
                        inputs.front().universeCount != controller->GetOutputCount() ||
                        inputs.front().channels != controller->GetOutput(0)->GetChannels() ||
                        (inputs.front().protocol == 1 && controller->GetProtocol() != OUTPUT_ARTNET) ||
                        (inputs.front().protocol == 0 && controller->GetProtocol() != OUTPUT_E131)) {
                        logger_base.debug("Board has inputs %d:%d:%d:%s while xlights has %d:%d:%d:%s. These need to match.",
                            inputs[0].universe, inputs[0].universeCount, inputs[0].channels, inputs[0].protocol == 1 ? OUTPUT_ARTNET : OUTPUT_E131,
                            controller->GetOutput(0)->GetUniverse(), controller->GetOutputCount(), controller->GetOutput(0)->GetChannels(), (const char*)controller->GetProtocol().c_str()
                        );
                        DisplayError("Falcon inputs not as expected. Upload inputs to correct.", parent);
                        if (doProgress) progress->Update(100, "Aborting.");
                        return false;
                    }
                }
            }
        }

        if (doProgress) progress->Update(40, "Ensuring board configuration is correct.");

        if (_v4status["B"].AsInt() != wxAtoi(caps->GetCustomPropertyByPath("v4BoardMode", "0"))) {
            // we need to change the board mode - controller mode and start channel should be already set
            bool reboot = false;
            if (!V4_SendBoardMode(wxAtoi(caps->GetCustomPropertyByPath("v4BoardMode", "0")), _v4status["O"].AsInt(), _v4status["ps"].AsInt() + 1, reboot)) {
                DisplayError("Falcon Outputs Upload: Failed to set board mode.", parent);
                if (doProgress) progress->Update(100, "Aborting.");
                return false;
            }
            if (reboot) {
                V4_WaitForReboot(controller->GetName(), parent);
            }

            // just give the falcon a second to gather its thoughts
            wxSleep(1);

            // this just makes sure our status is up to date
            if (!V4_GetStatus(_v4status)) {
                DisplayError("Falcon Outputs Upload: Failed to retrieve current configuration.", parent);
                if (doProgress) progress->Update(100, "Aborting.");
                return false;
            }
        }
    }

    if (doProgress) progress->Update(50, "Reworking pixel ports.");

    bool oneBased = false;
    if (controller->GetProtocol() == OUTPUT_DDP) {
        wxASSERT(controller->GetOutput(0) != nullptr);
        DDPOutput* ddp = dynamic_cast<DDPOutput*>(controller->GetOutput(0));
        oneBased = !ddp->IsKeepChannelNumbers();
    }

    std::vector<FALCON_V4_STRING> uploadStrings;
    std::string error;
    if (!V4_PopulateStrings(uploadStrings, falconStrings, cud, caps, defaultBrightness, error, oneBased, controller->GetStartChannel(), fullcontrol)) {
        DisplayError("Falcon Outputs Upload: Problem constructing strings for upload:\n" + error, parent);
        if (doProgress) progress->Update(100, "Aborting.");
        return false;
    }

    logger_base.info("Proposed falcon string configuration.");
    V4_DumpStrings(uploadStrings);

    if (doProgress) progress->Update(70, "Uploading pixel ports.");

    bool reboot = false;
    if (!V4_SendOutputs(uploadStrings, _v4status["A"].AsInt(), cud.GetFirstOutput()->GetStartChannel(), reboot)) {
        DisplayError("Falcon Outputs Upload: Problem uploading string configuration.", parent);
        if (doProgress) progress->Update(100, "Aborting.");
        return false;
    }

    if (cud.HasSerialPort(1)) {
        if (doProgress) progress->Update(80, "Uploading serial ports.");

        // just give the falcon a second to gather its thoughts
        wxSleep(1);

        auto sp = cud.GetControllerSerialPort(1);

        int rate = 250000;
        if (Lower(sp->GetProtocol()) != "dmx") rate = _v4status["sr"].AsInt();

        int universe = 0;
        unsigned long startChannel = 0;
        V4_GetStartChannel(sp->GetUniverse(), sp->GetUniverseStartChannel(), sp->GetStartChannel(), universe, startChannel, oneBased, controller->GetStartChannel());

        if (!V4_SetSerialConfig(Lower(sp->GetProtocol()) == "dmx" ? 0 : 1, universe, startChannel, rate)) {
            DisplayError("Falcon Outputs Upload: Problem uploading serial port configuration.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }
    }

    if (check != "") {
        DisplayWarning("Upload warnings:\n" + check);
    }

    if (doProgress) progress->Update(100, "Done.");
    logger_base.info("Falcon upload done.");

    return success;
}
#endif
#pragma endregion

#pragma region FalconString Handling
class FalconString
{
public:
    int virtualStringIndex = -1;
    int protocol = -1;
    int universe = -1;
    int startChannel = -1;
    int pixels = -1;
    std::string description;
    int index = -1;
    int port = -1;
    float gamma = 1.0;
    int groupCount = 1;
    int zig = 0;
    int smartRemote = 0;
    int nullPixels = 0;
    std::string colourOrder;
    std::string direction;
    int brightness = 100;

    FalconString(int defaultBrightness) : brightness(defaultBrightness) {}

    void Dump() const {

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("    Index %02d Port %02d SmartRemote %d VirtualString %d Prot %d Desc '%s' Uni %d StartChan %d Pixels %d Group %d Direction %s ColorOrder %s Nulls %d Brightness %d Gamma %.1f ZigZag %d",
            index,
            port + 1,
            smartRemote,
            virtualStringIndex,
            protocol,
            (const char*)description.c_str(),
            universe,
            startChannel,
            pixels,
            groupCount,
            (const char*)direction.c_str(),
            (const char*)colourOrder.c_str(),
            nullPixels,
            brightness,
            gamma, zig);
    }
    const bool operator>(const FalconString& other) const
    {
        if (port / 4 == other.port / 4) {
            if (smartRemote > other.smartRemote) return false;
            if (smartRemote < other.smartRemote) return true;
        }

        if (port > other.port) return false;
        if (port < other.port) return true;

        if (virtualStringIndex > other.virtualStringIndex) return false;
        if (virtualStringIndex < other.virtualStringIndex) return true;

        return true;
    }
};

#define MINIMUMPIXELS 1
void Falcon::InitialiseStrings(std::vector<FalconString*>& stringsData, int max, int minuniverse, int defaultBrightness, int32_t firstchannel) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Filling in missing strings.");

    std::vector<FalconString*> newStringsData;

    int index = 0;
    for (int i = 0; i < max; i++) {
        bool added = false;
        for (const auto& sd : stringsData) {
            if (sd->port == i) {
                sd->index = index++;
                newStringsData.push_back(sd);
                added = true;
            }
        }
        if (!added) {
            FalconString* string = new FalconString(defaultBrightness);
            if (_usingAbsolute) {
                string->universe = 0;
                string->startChannel = firstchannel;
            } else {
                string->universe = minuniverse;
                string->startChannel = 1;
            }
            string->virtualStringIndex = 0;
            string->pixels = MINIMUMPIXELS;
            string->protocol = 0;
            string->description = "";
            string->port = i;
            string->index = index++;
            //string->brightness = 100;//set in initializer list
            string->nullPixels = 0;
            string->gamma = 1.0;
            string->colourOrder = "RGB";
            string->direction = "Forward";
            string->groupCount = 1;
            string->zig = 0;
            string->smartRemote = 0x00;
            newStringsData.push_back(string);
            logger_base.debug("    Added default string to port %d.", i + 1);
        }
    }
    stringsData = newStringsData;
}

std::string Falcon::BuildStringPort(FalconString* string) const {

    return wxString::Format("&p%i=%i&x%i=%i&t%i=%i&u%i=%i&s%i=%i&c%i=%i&y%i=%s&b%i=%i&n%i=%i&G%i=%i&o%i=%i&d%i=%i&g%i=%i&w%i=%d&z%i=%d",
        string->index, string->port,
        string->index, string->virtualStringIndex,
        string->index, string->protocol,
        string->index, string->universe,
        string->index, string->startChannel,
        string->index, string->pixels,
        string->index, string->description,
        string->index, EncodeBrightness(string->brightness),
        string->index, string->nullPixels,
        string->index, EncodeGamma(string->gamma),
        string->index, EncodeColourOrder(string->colourOrder),
        string->index, EncodeDirection(string->direction),
        string->index, string->groupCount,
        string->index, string->smartRemote, 
        string->index, string->zig)
        .ToStdString();
}

FalconString* Falcon::FindPort(const std::vector<FalconString*>& stringData, int port) const {

    for (const auto& it : stringData) {
        if (it->port == port)
        {
            return it;
        }
    }
    wxASSERT(false);
    return nullptr;
}

int Falcon::GetPixelCount(const std::vector<FalconString*>& stringData, int port) const
{

    int count = 0;
    bool smrA = false;
    bool smrB = false;
    bool smrC = false;
    for (const auto& sd : stringData) {
        if (sd->port == port) {
            // have to include any null pixels in the count
            count += sd->pixels + sd->nullPixels;
            if (sd->smartRemote == 1 && sd->pixels > 0) smrA = true;
            if (sd->smartRemote == 2 && sd->pixels > 0) smrB = true;
            if (sd->smartRemote == 3 && sd->pixels > 0) smrC = true;
        }
    }

    if (smrC) {
        if (!smrB) count++;
        if (!smrA) count++;
    }
    else if (smrB) {
        if (!smrA) count++;
    }

    return count;
}

int Falcon::GetMaxPixelPort(const std::vector<FalconString*>& stringData) const {

    int max = 0;
    for (const auto& sd : stringData) {
        if (sd->port + 1 > max) {
            max = sd->port + 1;
        }
    }
    return max;
}

void Falcon::RemoveNonSmartRemote(std::vector<FalconString*>& stringData, int port)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto it = begin(stringData);
    while (it != end(stringData)) {
        if ((*it)->port == port && (*it)->smartRemote == 0) {
            logger_base.debug("Removing non smart remote port %d", port + 1);
            it = stringData.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Falcon::EnsureSmartStringExists(std::vector<FalconString*>& stringData, int port, int smartRemote, int minuniverse, int defaultBrightness, int32_t firstchannel) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool found = false;
    for (const auto& it : stringData) {
        if (it->port == port && it->smartRemote == smartRemote) {
            found = true;
            break;
        }
    }

    if (!found) {
        logger_base.debug("Adding in dummy string for a smart remote Port: %d Smart Remote %d", port + 1, smartRemote + 1);
        FalconString* string = new FalconString(defaultBrightness);
        string->startChannel = firstchannel;
        string->virtualStringIndex = 0;
        string->pixels = MINIMUMPIXELS;
        string->protocol = 0;
        string->universe = minuniverse;
        string->description = "";
        string->port = port;
        string->index = stringData.size();
        string->brightness = defaultBrightness;
        string->nullPixels = 0;
        string->gamma = 1.0;
        string->colourOrder = "RGB";
        string->direction = "Forward";
        string->groupCount = 1;
        string->zig = 0;
        string->smartRemote = smartRemote;
        stringData.push_back(string);
    }
}

void Falcon::DumpStringData(std::vector<FalconString*> stringData) const {

    for (const auto& sd : stringData) {
        sd->Dump();
    }
}
#pragma endregion

#pragma region strings.xml Handling
int Falcon::CountStrings(const wxXmlDocument& stringsDoc) const {

    if (stringsDoc.GetRoot() == nullptr) return 0;

    int count = 0;
    int last = -1;
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        int port = wxAtoi(e->GetAttribute("p"));
        if (port > last) {
            count++;
        }
        last = port;
    }
    return count;
}
int Falcon::NumConfiguredStrings() {
    if (IsV4()) {
        int batch = 0;
        wxJSONValue p(wxJSONTYPE_OBJECT);
        bool finalCall;
        int outBatch;
        bool reboot;
        wxJSONValue outParams;
        if (CallFalconV4API("Q", "SP", batch, 0, 0, p, finalCall, outBatch, reboot, outParams) == 200) {
            return outParams["A"].Size();
        }
        return -1;
    }
    // get the current config before I start
    std::string strings = GetURL("/strings.xml");
    if (strings == "") {
        return 0;
    }
    wxStringInputStream strm(strings);
    wxXmlDocument stringsDoc(strm);

    if (!stringsDoc.IsOk()) {
        return 0;
    }
    return CountStrings(stringsDoc);
}

void Falcon::ReadStringData(const wxXmlDocument& stringsDoc, std::vector<FalconString*>& stringData, int defaultBrightness) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (stringsDoc.GetRoot() == nullptr) return;

    int count = 0;
    for (auto n = stringsDoc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
        count++;
    }

    logger_base.debug("Strings.xml had %d entries.", count);
    if (count == 0) return;

    int oldCount = stringData.size();
    stringData.resize(count);
    for (int i = oldCount; i < count; ++i) {
        stringData[i] = nullptr;
    }

    int i = 0;
    int lastString = -1;
    for (auto e = stringsDoc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
        int port = wxAtoi(e->GetAttribute("p"));

        //<vs y="" p="7" u="2000" us="0" s="0" c="50" g="1" t="0" d="0" o="0" n="0" z="0" b="13" bl="0" ga="0"/>
        // y = description
        // u = universe
        // us = universe start channel - 1
        // s = absolute channel I think
        // c = pixel count
        // g = grouping
        // t = protocol
        // d = direction (index 0 - forward, 1 - reversed)
        // o = pixel order (index 0 - RGB, 1 - RBG, 2 - GRB, 3 - GBR, 4 - BRG, 5 - BGR)
        // n = null pixels
        // z = zig zag count - IGNORED
        // b = brightness (index 0 - 100, 1 - 95, 2 - 90, 3 - 85, 4 - 80, 5 - 75, 6 - 70, 7 - 65, 8 - 60, 9 - 50, 10 - 40, 11 - 30, 12 - 20, 13 - 10)
        // bl = blank (1 if checked) - IGNORED
        // ga = gamma (index 0 - none, 1 - 2.0, 2 - 2.3, 3 - 2.5, 4 - 2.8, 5 - 3.0
        FalconString* string = new FalconString(defaultBrightness);
        string->startChannel = wxAtoi(e->GetAttribute("us")) + 1;
        if (!_usingAbsolute) {
            if (string->startChannel < 1 || string->startChannel > 512) string->startChannel = 1;
        }
        string->pixels = wxAtoi(e->GetAttribute("c"));
        if (string->pixels < 0 || string->pixels > GetMaxPixels()) string->pixels = 0;
        string->protocol = wxAtoi(e->GetAttribute("t", "0"));
        string->universe = wxAtoi(e->GetAttribute("u"));
        if (string->universe <= 1 || string->universe > 64000) string->universe = 1;
        string->description = e->GetAttribute("y", "").ToStdString();
        string->port = wxAtoi(e->GetAttribute("p"));
        string->brightness = DecodeBrightness(wxAtoi(e->GetAttribute("b", "0")));
        string->nullPixels = wxAtoi(e->GetAttribute("n", "0"));
        string->gamma = DecodeGamma(wxAtoi(e->GetAttribute("ga", "0")));
        string->colourOrder = DecodeColourOrder(wxAtoi(e->GetAttribute("o", "0")));
        string->direction = DecodeDirection(wxAtoi(e->GetAttribute("d", "0")));
        string->groupCount = std::max(1, wxAtoi(e->GetAttribute("g", "1")));
        string->zig = wxAtoi(e->GetAttribute("z", "0"));
        int sr = wxAtoi(e->GetAttribute("sr", "-1"));
        if (sr == -1) {
            string->smartRemote = wxAtoi(e->GetAttribute("x", "0"));
        }
        else {
            string->smartRemote = sr;
        }
        string->virtualStringIndex = wxAtoi(e->GetAttribute("si", "0"));
        string->index = i;
        stringData[i] = string;

        lastString = port;
        i++;
    }
}

int Falcon::MaxPixels(const wxXmlDocument& stringsDoc, int board) const {

    if (stringsDoc.GetRoot() == nullptr) return 0;

    switch (board) {
    case 0:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k0"));
    case 1:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k1"));
    case 2:
        return wxAtoi(stringsDoc.GetRoot()->GetAttribute("k2"));
    default:
        return 0;
    }
}
#pragma endregion

#pragma region Port Handling
#ifndef DISCOVERYONLY
void Falcon::ResetStringOutputs() {

    PutURL("/StringPorts.htm", "S=4&p0=0&p1=1&p2=2&p3=3");
}

void Falcon::UploadStringPort(const std::string& request, bool final) {

    std::string r = request;
    if (final) {
        r = "r=1&" + r;
    }
    else {
        r = "r=0&" + r;
    }

    PutURL("/StringPorts.htm", r);
}

void Falcon::UploadStringPorts(std::vector<FalconString*>& stringData, int maxMain, int maxDaughter1, int maxDaughter2, int minuniverse, int defaultBrightness, int32_t firstchannel) {

    int maxPort = 0;
    for (const auto& sd : stringData) {
        maxPort = std::max(maxPort, sd->port);
    }

    int S = stringData.size();
    int m = 0;

    if (maxPort + 1 > GetDaughter2Threshold()) {
        m = 2;
    }
    else if (maxPort + 1 > GetDaughter1Threshold()) {
        m = 1;
    }

    std::string base = wxString::Format("m=%i&S=%i", m, S).ToStdString();

    if (SupportsVariableExpansions()) {
        base += wxString::Format("&k0=%i&k1=%i&k2=%i", maxMain, maxDaughter1, maxDaughter2).ToStdString();
    }

#define PACKETSIZE 40
    int packets = (stringData.size() + PACKETSIZE - 1) / PACKETSIZE;
    for (int p = 0; p < packets; p++) {
        std::string message = base + "&q=" + wxString::Format("%d", p).ToStdString();

        for (int i = p * PACKETSIZE; i < stringData.size() && i < (p + 1) * PACKETSIZE; ++i) {
            message += BuildStringPort(stringData[i]);
        }

        UploadStringPort(message, p == packets - 1);
    }
}

std::string Falcon::GetSerialOutputURI(ControllerCaps* caps, int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent) {

    if (output > caps->GetMaxSerialPort()) {
        DisplayError("Falcon " + GetModel() + " only supports " + wxString::Format("%d", caps->GetMaxSerialPort()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", parent);
        return "";
    }

    if (_usingAbsolute) {
        return wxString::Format("t%d=%d&s%d=%d",
            output - 1, protocol,
            output - 1, portstart).ToStdString();
    }
    else {
        int32_t sc;
        auto o = outputManager->GetOutput(portstart, sc);
        if (o != nullptr) {
            return wxString::Format("t%d=%d&u%d=%d&s%d=%d",
                output - 1, protocol,
                output - 1, o->GetUniverse(),
                output - 1, (int)sc).ToStdString();
        }
        else {
            DisplayError("Error uploading serial output to falcon. " + wxString::Format("%i", portstart) + " does not map to a universe.");
        }
    }

    return "";
}
#endif
#pragma endregion

#pragma region Encode and Decode
int Falcon::DecodeSerialOutputProtocol(std::string protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return 0;
    if (p == "pixelnet") return 1;
    if (p == "renard") return 2;
    return 0;
}

int Falcon::DecodeStringPortProtocol(std::string protocol) const {

    wxString p(protocol);
    p = p.Lower();
    if (p == "ws2811") return 0;
    if (p == "tm18xx") return 1;
    if (p == "lx1203") return 2;
    if (p == "ws2801") return 3;
    if (p == "tls3001") return 4;
    if (p == "lpd6803") return 5;
    if (p == "gece") return 6;
    if (p == "lpd8806") return 7;
    if (p == "apa102") return 8;
    if (p == "ucs1903") return 9;
    if (p == "dm412") return 10;
    if (p == "p9813") return 11;
    if (p == "ucs2903") return 12;
    if (p == "tm1814") return 14;

    return 0;
}

int Falcon::DecodeBrightness(int brightnessCode) const {

    switch (brightnessCode) {
    case 0: return 100;
    case 1: return 95;
    case 2: return 90;
    case 3: return 85;
    case 4: return 80;
    case 5: return 75;
    case 6: return 70;
    case 7: return 65;
    case 8: return 60;
    case 9: return 50;
    case 10: return 40;
    case 11: return 30;
    case 12: return 20;
    case 13: return 10;
    default: break;
    }
    return 100;
}

int Falcon::EncodeBrightness(int brightness) const {

    if (brightness < 15) return 13;
    if (brightness < 25) return 12;
    if (brightness < 35) return 11;
    if (brightness < 45) return 10;
    if (brightness < 55) return 9;
    if (brightness < 62.5) return 8;
    if (brightness < 67.5) return 7;
    if (brightness < 72.5) return 6;
    if (brightness < 77.5) return 5;
    if (brightness < 82.5) return 4;
    if (brightness < 87.5) return 3;
    if (brightness < 92.5) return 2;
    if (brightness < 97.5) return 1;
    return 0;
}

float Falcon::DecodeGamma(int gammaCode) const {

    switch (gammaCode) {
    case 0: return 1.0f;
    case 1: return 2.0f;
    case 2: return 2.3f;
    case 3: return 2.5f;
    case 4: return 2.8f;
    case 5: return 3.0f;
    default: break;
    }
    return 1.0f;
}

int Falcon::EncodeGamma(float gamma) const {

    if (gamma < 1.5) return 0;
    if (gamma < 2.15) return 1;
    if (gamma < 2.4) return 2;
    if (gamma < 2.65) return 3;
    if (gamma < 2.9) return 4;
    return 5;
}

std::string Falcon::DecodeColourOrder(int colourOrderCode) const {

    switch (colourOrderCode) {
    case 0: return "RGB";
    case 1: return "RBG";
    case 2: return "GRB";
    case 3: return "GBR";
    case 4: return "BRG";
    case 5: return "BGR";
    default: break;
    }
    return "RGB";
}

int Falcon::EncodeColourOrder(const std::string& colourOrder) const {

    if (colourOrder == "RGB") return 0;
    if (colourOrder == "RBG") return 1;
    if (colourOrder == "GRB") return 2;
    if (colourOrder == "GBR") return 3;
    if (colourOrder == "BRG") return 4;
    if (colourOrder == "BGR") return 5;
    return 0;
}

std::string Falcon::DecodeDirection(int directionCode) const {

    switch (directionCode) {
    case 0: return "Forward";
    case 1: return "Reverse";
    default: break;
    }
    return "Forward";
}

int Falcon::EncodeDirection(const std::string& direction) const {

    if (direction == "Forward") return 0;
    if (direction == "Reverse") return 1;
    return 0;
}
#pragma endregion

#pragma region Private Functions
std::string Falcon::SafeDescription(const std::string description) const {

    wxString desc(description);
    int replaced = desc.Replace("  ", " ");
    while (replaced != 0) {
        replaced = desc.Replace("  ", " ");
    }
    return desc.Left(25).ToStdString();
}

bool Falcon::IsEnhancedV2Firmware() const {

    if (_firmwareVersion == "") return false;

    auto fwv = wxSplit(_firmwareVersion, '.');
    int majorfw = 0;
    int minorfw = 0;

    if (fwv.size() > 0) {
        majorfw = wxAtoi(fwv[0]);
        if (fwv.size() > 1) {
            minorfw = wxAtoi(fwv[1]);
        }
    }

    if (majorfw < 2 || (majorfw == 2 && minorfw < 1)) {
        return false;
    }

    return true;
}

int Falcon::GetMaxPixels() const {

    if (IsV2() && !IsEnhancedV2Firmware()) {
        return 680;
    }
    else {
        return 1024;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
Falcon::Falcon(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _v4status = wxJSONValue(wxJSONTYPE_OBJECT);
    _firmwareVersion = "";
    _model = "";
    _versionnum = 0;
    _modelnum = 0;
    _majorFirmwareVersion = 0;
    _minorFirmwareVersion = 0;
    _usingAbsolute = false;

    logger_base.debug("Connecting to Falcon on %s.", (const char*)_ip.c_str());

    _connected = true;
    int p = 0;
    std::string versionxml = GetURL("/status.xml");
    if (versionxml == "") {
        logger_base.error("    Error retrieving status.xml from falcon controller.");
        _connected = false;
        return;
    }
    logger_base.debug("status.xml:\n%s", (const char*)versionxml.c_str());

    wxStringInputStream stream(versionxml);
    wxXmlDocument xml(stream);
    if (!xml.IsOk() || xml.GetRoot() == nullptr) {
        logger_base.error("     Status XML parses as invalid.");
        _firmwareVersion = "UNKNOWN";
        _usingAbsolute = false;
        _name = "UNKNOWN";
        _model = "UNKNOWN";
        _connected = false;
    }
    else {
        wxXmlNode* node = xml.GetRoot()->GetChildren();
        while (node) {
            if (node->GetName() == "v" || node->GetName() == "fv") {
                _firmwareVersion = node->GetNodeContent();
            }
            else if (node->GetName() == "a") {
                _usingAbsolute = node->GetNodeContent() == "0";
            }
            else if (node->GetName() == "n") {
                _name = node->GetNodeContent().Trim();
            }
            else if (node->GetName() == "p") {
                p = wxAtoi(node->GetNodeContent());
                DecodeModelVersion(p, _modelnum, _versionnum);
                _model = wxString::Format("F%dv%d", _modelnum, _versionnum).ToStdString();
            }
            _status[node->GetName()] = node->GetNodeContent();
            node = node->GetNext();
        }

        if (_versionnum == 4 || _versionnum == 5) {
            // this is going to need special handling
            if (V4_GetStatus(_v4status)) {
                _modelnum = _v4status["BR"].AsInt();
                _model = wxString::Format("F%dv%d", _modelnum, _versionnum).ToStdString();
            }
        }
        else {
            if (_versionnum == 0 || _modelnum == 0 || _firmwareVersion == "") {
                std::string version = GetURL("/index.htm");
                if (version == "") {
                    logger_base.error("    Error retrieving index.htm from falcon controller.");
                    _connected = false;
                    return;
                }

                if (_firmwareVersion == "") {
                    //<title>F4V2            - v1.10</title>
                    static wxRegEx firmwareversionregex("(title.*?v)([0-9]+\\.[0-9]+)\\<\\/title\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                    if (firmwareversionregex.Matches(wxString(version))) {
                        _firmwareVersion = firmwareversionregex.GetMatch(wxString(version), 2).ToStdString();
                    }
                }
            }

            if (_firmwareVersion != "") {
                static wxRegEx majminregex("(\\d+)\\.(\\d+)(.*)", wxRE_ADVANCED);
                if (majminregex.Matches(wxString(_firmwareVersion))) {
                    _majorFirmwareVersion = wxAtoi(majminregex.GetMatch(wxString(_firmwareVersion), 1));
                    _minorFirmwareVersion = wxAtoi(majminregex.GetMatch(wxString(_firmwareVersion), 2));
                    logger_base.error("    Parsed firmware version %d.%d.", _majorFirmwareVersion, _minorFirmwareVersion);
                }
            }
        }

        logger_base.debug("Connected to falcon - p=%d Model: '%s' Firmware Version '%s'. F%d:V%d", p, (const char*)GetModel().c_str(), (const char*)_firmwareVersion.c_str(), _modelnum, _versionnum);
    }

    if (_versionnum == 0 || _modelnum == 0) {
        _connected = false;
        logger_base.error("Error connecting to falcon controller on %s. Unable to determine model/version.", (const char*)_ip.c_str());
    }
}
#pragma endregion

#pragma region Static Functions
void Falcon::DecodeModelVersion(int p, int& model, int& version) {

    switch (p) {
    case 1:
    case 2:
    case 3:
        model = 16;
        version = 2;
        break;
    case 4:
        model = 4;
        version = 2;
        break;
    case 5:
        model = 16;
        version = 3;
        break;
    case 6:
        model = 4;
        version = 3;
        break;
    case 7:
        model = 48;
        version = 3;
        break;
    case 128:
        model = 16;
        version = 4;
        break;
    case 129:
        model = 48;
        version = 4;
        break;
    case 130:
        model = 16;
        version = 5;
        break;
    case 131:
        model = 48;
        version = 5;
        break;
    case 132:
        model = 32;
        version = 5;
        break;
    default:
        model = 16;
        version = 3;
        break;
    }
}
std::string Falcon::DecodeMode(int mode)
{
    switch (mode) {
    case 0: return "E131/ArtNET";
    case 2: return "Player";
    case 4: return "Remote";
    case 8: return "Master";
    case 16: return "ZCPP";
    case 64: return "DDP";
    }
    return std::string();
}
#pragma endregion

#pragma region Getters and Setters
std::string Falcon::GetMode() {
    if (_versionnum == 4 || _versionnum == 5) {
        return V4_DecodeMode(_v4status["O"].AsInt());
    }
    return DecodeMode(wxAtoi(_status["m"].AsString()));
}



#ifndef DISCOVERYONLY
bool Falcon::UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    SetInputUniverses(controller, parent);
    SetOutputs(allmodels, outputManager, controller, parent, false);
    return true;
}

bool Falcon::V4_ValidateWAV(const std::string& media)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxFile f;
    if (f.Open(media))         {
        
        uint8_t buffer[36];
        if (f.Read(&buffer, sizeof(buffer)) == sizeof(buffer))             {

            // is it a WAV file
            if (buffer[0] != 82 || buffer[1] != 73 || buffer[2] != 70 || buffer[3] != 70) {
                logger_base.error("WAV file token missing.");
                return false;
            }

            // is it PCM
            if (buffer[20] != 1 || buffer[21] != 0) {
                logger_base.error("Not a PCM audio format.");
                return false;
            }

            // is sample rate 44100
            if (!(
                (buffer[24] == 0x44 && buffer[25] == 0xac) || // 44100
                (buffer[24] == 0x80 && buffer[25] == 0xbb) || // 48000
                (buffer[24] == 0x00 && buffer[25] == 0x7d) || // 32000
                (buffer[24] == 0x22 && buffer[25] == 0x56) || // 22050
                (buffer[24] == 0x80 && buffer[25] == 0x3e) || // 16000
                (buffer[24] == 0x11 && buffer[25] == 0x2b) || // 11025
                (buffer[24] == 0x40 && buffer[25] == 0x1f)    // 8000
                    )) {
                int br = (((int)buffer[25]) << 8) + (int)buffer[24];
                logger_base.error("Not valid bit rate: %d", br);
                return false;
            }

            // 2 channels
            if (buffer[22] != 2 || buffer[23] != 0) {
                logger_base.error("Not a stereo file.");
                return false;
            }

            // is it block align 4
            if (buffer[32] != 4 || buffer[33] != 0) {
                logger_base.error("WAV file block alignment is not 4.");
                return false;
            }

            // is it 16 bit
            if (buffer[34] != 16 || buffer[35] != 0) {
                logger_base.error("Not 16 bits per sample.");
                return false;
            }
            return true;
        }
    }
    return false;
}

bool Falcon::UploadSequence(const std::string& seq, const std::string& file, const std::string& media, std::function<bool(int, std::string)> progress)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = true;

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    std::string url = "http://" + baseIP + _baseUrl + "/upload.cgi";
    logger_base.debug("Uploading to URL: %s", (const char*)url.c_str());

    if (media != "") {
        wxFileName fn(media);
        std::string origfile = fn.GetFullName()/*.Lower()*/.ToStdString();
        bool ismp3 = fn.GetExt().Lower() == "mp3";
        fn.SetExt("wav");
        std::string wavfile = fn.GetFullName() /*.Lower()*/.ToStdString();
        auto lwavfile = Lower(wavfile);

        // check to see if controller has the media file
        auto wavs = V4_GetMediaFiles();
        bool found = false;

        for (const auto& it : wavs) {
            if (Lower(it) == lwavfile) {
                found = true;
                break;
            }
        }

        // if not then upload it
        if (!found) {

            bool skip = false;

            if (!ismp3) {
                if (!V4_ValidateWAV(media)) {
                    skip = true;
                    wxMessageBox("WAV file not valid: " + media + " : Skipping.");
                }
            }

            if (!skip && res) {
                res = res && Curl::HTTPUploadFile(url, media, origfile, progress);

                if (res) {
                    if (ismp3) {
                        if (progress != nullptr) progress(0, "Converting to WAV file.");
                        wxSleep(1);
                        int p = 0;
                        while (p != 100) {
                            p = V4_GetConversionProgress();
                            if (progress != nullptr) progress(p * 10, "Converting to WAV file.");
                            if (p != 100) wxSleep(5);
                        }
                    }
                    else {
                        while (V4_IsFileUploading()) {
                            wxSleep(1);
                        }
                    }
                }
            }
        }
    }

    // upload the fseq
    {
        wxFileName fn(file);
        res = res && Curl::HTTPUploadFile(url, seq, fn.GetFullName()/*.Lower()*/.ToStdString(), progress);

        if (res) {
            while (V4_IsFileUploading()) {
                wxSleep(1);
            }
        }
    }
    return res;
}

bool Falcon::SetInputUniverses(Controller* controller, wxWindow* parent) {

    if (!ValidateBoard(controller)) {
        DisplayError("Falcon Inputs Upload: Board version does not match your controller settings.", parent);
        return false;
    }

    if (_versionnum == 4 || _versionnum == 5) {
        return V4_SetInputUniverses(controller, parent);
    }

    wxString request;
    int output = 0;

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    int cm = -1;
    std::string strings = GetURL("/strings.xml");
    wxStringInputStream stream(strings);
    wxXmlDocument xml(stream);
    if (!xml.IsOk() || xml.GetRoot() == nullptr) {
    }
    else {
        wxXmlNode* node = xml.GetRoot();
        if (node != nullptr) {
            cm = wxAtoi(node->GetAttribute("m", "-1"));
        }

        // the m parameter in strings.xml is not reliable ... so get the home page and search for "<input type="hidden" name="m" id="m"  value="64" />"
        std::string status = GetURL("/");
        if (status != "") {
            static wxRegEx mregex("(id=\"m\" +value=\")([0-9]+)\"", wxRE_ADVANCED);
            if (mregex.Matches(wxString(status))) {
                cm = wxAtoi(mregex.GetMatch(wxString(status), 2).ToStdString());
            }
        }
    }

    if (outputs.size() > 0 && outputs.front()->GetType() == OUTPUT_DDP) {

        if (!IsFirmwareEqualOrGreaterThan(2, 58)) {
            DisplayError("Attempt to set controller to DDP mode but firmware installed does not support DDP. Install 2.58 or higher.");
            return false;
        }

        DDPOutput* ddp = (DDPOutput*)outputs.front();

        _usingAbsolute = true;

        request = wxString::Format("c=64&d=%d", ddp->IsKeepChannelNumbers() ? ddp->GetStartChannel() : 1);
        std::string response = PutURL("/index.htm", request.ToStdString());

        if ((cm & 0xFE) != 64) {
            std::unique_ptr<wxProgressDialog> progress;
            progress.reset(new wxProgressDialog(wxString::Format("Rebooting controller '%s' ...", controller->GetName()), "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
            progress->Show();

            for (int i = 0; i < 100; i++) {
                progress->Update(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(20000 / 100));
            }
        }

        return (response != "");
    }
    else {
        if (outputs.size() > 96) {
            DisplayError(wxString::Format("Attempt to upload %d universes to falcon controller but only 96 are supported.", outputs.size()).ToStdString());
            return false;
        }

        if (IsFirmwareEqualOrGreaterThan(2, 58) && (cm & 0xFE) != 0) {
            // need to switch to e131 mode
            request = "c=0";
            std::string response = PutURL("/index.htm", request.ToStdString());

            std::unique_ptr<wxProgressDialog> progress;
            progress.reset(new wxProgressDialog(wxString::Format("Rebooting controller '%s' ...", controller->GetName()), "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
            progress->Show();

            for (int i = 0; i < 100; i++) {
                progress->Update(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(20000 / 100));
            }
        }

        for (const auto& it : outputs) {
            int t = -1;
            if (it->GetType() == OUTPUT_E131) {
                t = 0;
            }
            else if (it->GetType() == OUTPUT_ARTNET) {
                t = 1;
            }
            request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
                output, it->GetUniverse(),
                output, (int)it->GetChannels(),
                output, (int)it->GetStartChannel(),
                output, t);
            output++;
        }

        request = wxString::Format("z=%d&a=%d", output, (_usingAbsolute ? 0 : 1)) + request;
        std::string response = PutURL("/E131.htm", request.ToStdString());
        return (response != "");
    }
}

bool Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {
    return SetOutputs(allmodels, outputManager, controller, parent, true);
}

bool Falcon::ValidateBoard(Controller* controller)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool valid = true;
    if ((controller->GetModel() == "F16V4" || controller->GetModel() == "F48V4") && _versionnum != 4) {
        // we say it is a V4 but it isnt    
        logger_base.error("Controller model is %s but response from controller says it is a V%d", (const char*)controller->GetModel().c_str(), _versionnum);
        valid = false;
    } else if ((controller->GetModel() == "F16V5" || controller->GetModel() == "F48V5" || controller->GetModel() == "F32V5") && _versionnum != 5) {
        // we say it is a V4 but it isnt
        logger_base.error("Controller model is %s but response from controller says it is a V%d", (const char*)controller->GetModel().c_str(), _versionnum);
        valid = false;
    } else if ((controller->GetModel() == "F16V2" || controller->GetModel() == "F16V2R" || controller->GetModel() == "F4V2") && _versionnum != 2) {
        // we say it is a V4 but it isnt    
        logger_base.error("Controller model is %s but response from controller says it is a V%d", (const char*)controller->GetModel().c_str(), _versionnum);
        valid = false;
    }
    else if ((controller->GetModel() == "F16V3" || controller->GetModel() == "F48" || controller->GetModel() == "F4V3") && _versionnum != 3) {
        // we say it is a V4 but it isnt    
        logger_base.error("Controller model is %s but response from controller says it is a V%d", (const char*)controller->GetModel().c_str(), _versionnum);
        valid = false;
    }

    return valid;
}

bool Falcon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent, bool doProgress) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //ResetStringOutputs(); // this shouldnt be used normally

    if (!ValidateBoard(controller)) {
        DisplayError("Falcon Outputs Upload: Board version does not match your controller settings.", parent);
        return false;
    }

    if (_versionnum == 4 || _versionnum == 5)
        return V4_SetOutputs(allmodels, outputManager, controller, parent, doProgress);

    std::unique_ptr<wxProgressDialog> progress;
    if (doProgress) {
        progress.reset(new wxProgressDialog("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE));
        progress->Show();
    }

    int defaultBrightness = controller->GetDefaultBrightnessUnderFullControl();

    logger_base.debug("Falcon Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    if (doProgress) progress->Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    auto caps = ControllerCaps::GetControllerConfig(controller);
    bool success = true;

    success = cud.Check(caps, check);

    logger_base.debug(check);

    cud.Dump();

    bool fullcontrol = false;
    int currentStrings = 0;
    int mainPixels = 680;
    int daughter1Pixels = 0;
    int daughter2Pixels = 0;
    int minuniverse = controller->GetFirstOutput()->GetUniverse();
    bool absoluteonebased = (controller->GetFirstOutput()->GetType() == OUTPUT_DDP && !dynamic_cast<DDPOutput*>(controller->GetFirstOutput())->IsKeepChannelNumbers());
    int32_t firstchannel = 1;
    if (absoluteonebased) firstchannel = controller->GetFirstOutput()->GetStartChannel();
    int32_t firstchanneloncontroller = firstchannel;
    if (controller->GetFirstOutput()->GetType() == OUTPUT_DDP && dynamic_cast<DDPOutput*>(controller->GetFirstOutput())->IsKeepChannelNumbers()) {
        firstchanneloncontroller = controller->GetFirstOutput()->GetStartChannel();
    } else {
        firstchanneloncontroller = controller->GetFirstOutput()->GetStartChannel();
    }

    if (caps != nullptr) {
        fullcontrol = caps->SupportsFullxLightsControl() && controller->IsFullxLightsControl();
        mainPixels = caps->GetMaxPixelPortChannels() / 3;
    }

    std::vector<FalconString*> stringData;

    if (!fullcontrol) {
        if (doProgress) progress->Update(10, "Retrieving string configuration from Falcon.");
        logger_base.info("Retrieving string configuration from Falcon.");

        // get the current config before I start
        std::string strings = GetURL("/strings.xml");
        if (strings == "") {
            DisplayError("Error occured trying to upload to Falcon. strings.xml could not be retrieved.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        wxStringInputStream strm(wxString(strings.c_str()));
        wxXmlDocument stringsDoc(strm);

        if (!stringsDoc.IsOk()) {
            DisplayError("Falcon Outputs Upload: Could not parse Falcon strings.xml.", parent);
            if (doProgress) progress->Update(100, "Aborting.");
            return false;
        }

        if (doProgress) progress->Update(40, "Processing current configuration data.");
        logger_base.info("Processing current configuration data.");

        currentStrings = CountStrings(stringsDoc);
        mainPixels = GetMaxPixels();
        daughter1Pixels = 0;
        daughter2Pixels = 0;
        if (SupportsVariableExpansions()) {
            mainPixels = MaxPixels(stringsDoc, 0);
            daughter1Pixels = MaxPixels(stringsDoc, 1);
            daughter2Pixels = MaxPixels(stringsDoc, 2);
        }
        else {
            if (currentStrings > GetBank1Threshold()) {
                mainPixels = mainPixels / 2;
                daughter1Pixels = mainPixels;
            }
        }

        logger_base.info("Current Falcon configuration split: Main = %d, Expansion1 = %d, Expansion2 = %d, Strings = %d", mainPixels, daughter1Pixels, daughter2Pixels, currentStrings);
        logger_base.info("Maximum string port configured in xLights: %d", cud.GetMaxPixelPort());

        ReadStringData(stringsDoc, stringData, defaultBrightness);

        logger_base.debug("Downloaded string data.");
        DumpStringData(stringData);
    }

    int maxPixels = GetMaxPixels();
    int totalPixelPorts = GetDaughter1Threshold();
    if (IsF48() || cud.GetMaxPixelPort() > GetDaughter2Threshold() ||
        currentStrings > GetDaughter2Threshold()) {
        logger_base.info("String port count needs to be %d.", caps->GetMaxPixelPort());
        totalPixelPorts = caps->GetMaxPixelPort();
    }
    else if (cud.GetMaxPixelPort() > GetDaughter1Threshold() ||
        currentStrings > GetDaughter1Threshold()) {
        logger_base.info("String port count needs to be %d.", GetDaughter2Threshold());
        totalPixelPorts = GetDaughter2Threshold();
    }
    else {
        logger_base.info("String port count needs to be %d.", GetDaughter1Threshold());
    }
    InitialiseStrings(stringData, totalPixelPorts, minuniverse, defaultBrightness, firstchanneloncontroller);

    //logger_base.debug("Missing strings added.");
    //DumpStringData(stringData);

    logger_base.info("Falcon pixel split: Main = %d, Expansion1 = %d, Expansion2 = %d", mainPixels, daughter1Pixels, daughter2Pixels);

    if (doProgress) progress->Update(50, "Configuring string ports.");
    logger_base.info("Configuring string ports.");

    bool portdone[100];
    memset(&portdone, 0x00, sizeof(portdone)); // all false

    // break it up into virtual strings
    std::vector<FalconString*> newStringData;
    std::vector<FalconString*> toDelete;
    int index = 0;
    for (int pp = 1; pp <= totalPixelPorts; pp++) {
        if (cud.HasPixelPort(pp)) {
            UDControllerPort* port = cud.GetControllerPixelPort(pp);
            logger_base.info("Pixel Port %d Protocol %s.", pp, (const char*)port->GetProtocol().c_str());

            port->CreateVirtualStrings(true);

            FalconString* firstString = nullptr;
            for (const auto& sd : stringData) {
                if (sd->port == pp - 1) {
                    if (firstString == nullptr) {
                        firstString = sd;
                    }
                    else {
                        toDelete.push_back(sd);
                    }
                }
            }
            wxASSERT(firstString != nullptr);

            // need to add virtual strings
            bool first = true;
            for (const auto& vs : port->GetVirtualStrings()) {
                FalconString* fs;
                if (first) {
                    fs = firstString;
                    first = false;
                }
                else {
                    fs = new FalconString(defaultBrightness);
                }

                // if we have switched smart remotes all settings should reset
                if (firstString->smartRemote != vs->_smartRemote) {
                    firstString = fs;
                }

                // ignore index ... we will fix them up when done
                fs->port = pp - 1;
                fs->index = index++;
                fs->virtualStringIndex = vs->_index;
                fs->protocol = DecodeStringPortProtocol(vs->_protocol);
                fs->universe = vs->_universe;
                if (_usingAbsolute) {
                    fs->startChannel = vs->_startChannel;
                }
                else {
                    fs->startChannel = vs->_universeStartChannel;
                }
                fs->pixels = INTROUNDUPDIV(vs->Channels(), GetChannelsPerPixel(vs->_protocol));
                fs->description = SafeDescription(vs->_description);
                fs->smartRemote = vs->_smartRemote;
                if (vs->_brightnessSet) {
                    fs->brightness = vs->_brightness;
                }
                else {
                    fs->brightness = firstString->brightness;
                }
                if (vs->_startNullPixelsSet) {
                    fs->nullPixels = vs->_startNullPixels;
                }
                else {
                    fs->nullPixels = 0;
                }
                if (vs->_gammaSet) {
                    fs->gamma = vs->_gamma;
                }
                else {
                    fs->gamma = firstString->gamma;
                }
                if (vs->_colourOrderSet) {
                    fs->colourOrder = vs->_colourOrder;
                }
                else {
                    fs->colourOrder = firstString->colourOrder;
                }
                if (vs->_reverseSet) {
                    fs->direction = vs->_reverse;
                }
                else {
                    fs->direction = firstString->direction;
                }
                if (vs->_groupCountSet) {
                    fs->groupCount = std::max(1, vs->_groupCount);
                    fs->pixels *= std::max(1, vs->_groupCount);
                }
                else {
                    fs->groupCount = 1;
                }
                if (vs->_zigZagSet) {
                    fs->zig = std::max(0, vs->_zigZag);
                } else {
                    fs->zig = 0;
                }
                newStringData.push_back(fs);
            }
        }
        else {
            int priorSmartRemote = 0;
            for (const auto& sd : stringData) {
                if (sd->port == pp - 1) {
                    sd->index = index++;
                    if (sd->port % 4 != 0 && priorSmartRemote != 0 && sd->smartRemote == 0) sd->smartRemote = 1;
                    newStringData.push_back(sd);
                }
                priorSmartRemote = sd->smartRemote;
            }
        }
    }
    stringData = newStringData;

    // delete any read strings we didnt keep
    for (const auto& d : toDelete) {
        delete d;
    }

    // fill in missing smart ports
    {
        int mp = 0;
        for (const auto& sd : stringData) {
            mp = std::max(mp, sd->port);
        }

        int quads = (mp + 3) / 4;
        for (int i = 0; i < quads; i++) {
            int maxRemote = 0;
            for (int j = 0; j < 4; j++) {
                for (const auto& it : stringData) {
                    if (it->port == i * 4 + j) {
                        if (it->smartRemote != 0) {
                            maxRemote = std::max(maxRemote, it->smartRemote);
                        }
                    }
                }
            }

            if (maxRemote > 0) {
                for (int k = 0; k < 4; k++) {
                    RemoveNonSmartRemote(stringData, i * 4 + k);
                    for (int j = 0; j < maxRemote; j++) {
                        EnsureSmartStringExists(stringData, i * 4 + k, j + 1, minuniverse, defaultBrightness, firstchanneloncontroller);
                    }
                }
            }
        }

        // Sort strings in the right order
        std::sort(begin(stringData), end(stringData), [](FalconString* a, FalconString* b) {
            return *a > *b;
        });

        // reindex the string data
        int i = 0;
        for (auto& it : stringData) {
            it->index = i++;
        }
    }

    logger_base.debug("Virtual strings created.");
    DumpStringData(stringData);

    logger_base.info("Working out required pixel splits.");
    int maxMain = 0;
    int maxDaughter1 = 0;
    int maxDaughter2 = 0;
    int largestMainPort = -1;
    int largestDaughter1Port = -1;
    int largestDaughter2Port = -1;

    for (auto pp = 0; pp < totalPixelPorts; ++pp) {
        int pixels = GetPixelCount(stringData, pp);
        if (pp < GetBank1Threshold()) {
            if (pixels > maxMain) {
                maxMain = pixels;
                largestMainPort = pp + 1;
            }
        }
        else if (pp < GetDaughter2Threshold()) {
            if (pixels > maxDaughter1) {
                maxDaughter1 = pixels;
                largestDaughter1Port = pp + 1;
            }
        }
        else {
            if (pixels > maxDaughter2) {
                maxDaughter2 = pixels;
                largestDaughter2Port = pp + 1;
            }
        }
    }

    int maxPort = GetMaxPixelPort(stringData);

    if (!SupportsVariableExpansions()) {
        // minimum of main is 1
        if (maxMain == 0) {
            maxMain = 1;
        }

        if (maxDaughter1 > 0) {
            if (maxMain > maxPixels / 2 || maxDaughter1 > maxPixels / 2) {
                DisplayError(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports 340/340 pixel split with expansion board. (%d/%d)",
                    _ip, maxMain, maxDaughter1).ToStdString());
                success = false;
            }

            maxMain = maxPixels / 2;
            maxDaughter1 = maxPixels / 2;

            if (maxDaughter2 > 0) {
                DisplayError(wxString::Format("Falcon Outputs Upload: %s V2 Controller only supports one expansion board.",
                    _ip).ToStdString());
                success = false;
                maxDaughter2 = 0;
            }
        }

        logger_base.info("Falcon pixel fixed split: Main = %d, Expansion1 = %d", maxMain, maxDaughter1);
    }
    else {
        if (maxMain == 0) {
            maxMain = 1;
        }

        if (maxPort > GetDaughter2Threshold() && maxDaughter2 == 0) {
            maxDaughter2 = 1;
        }

        if (maxPort > GetBank1Threshold() && maxDaughter1 == 0) {
            maxDaughter1 = 1;
        }

        if (IsF4() && IsV3()) {
            // v3 supports 1024 on all outputs
            maxMain = maxPixels;
            maxDaughter1 = 0;
            maxDaughter2 = 0;
        }

        logger_base.info("Falcon pixel required split: Main = %d, Expansion1 = %d, Expansion2 = %d", maxMain, maxDaughter1, maxDaughter2);

        if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
            success = false;
            check += "ERROR: Total pixels exceeded maximum allowed on a pixel port: " + wxString::Format("%d", maxPixels).ToStdString() + "\n";
            if (largestDaughter2Port >= 0) {
                check += wxString::Format("       Bank 1 Port %d=%d, Bank 2 Port %d=%d, Bank 3 Port %d=%d\n", largestMainPort, maxMain, largestDaughter1Port, maxDaughter1, largestDaughter2Port, maxDaughter2);
            }
            else if (largestDaughter1Port >= 0) {
                check += wxString::Format("       Bank 1 Port %d=%d, Bank 2 Port %d=%d\n", largestMainPort, maxMain, largestDaughter1Port, maxDaughter1);
            }
            else                 {
                check += wxString::Format("       Bank 1 Port %d=%d\n", largestMainPort, maxMain);
            }

            logger_base.warn("ERROR: Total pixels exceeded maximum allowed on a pixel port: %d", maxPixels);

            if (_modelnum == 48) {
                check += "Trying to disable unused banks on the F48.\n";
                logger_base.debug("Trying to disable unused banks on the F48.");
                // if it looks like we arent using the last 16 ports and everything is still set to the default
                if (cud.GetMaxPixelPort() <= 16 && maxDaughter1 == 50 && maxDaughter2 == 50) {
                    int left = maxPixels - maxMain;
                    maxDaughter1 = left / 2;
                    maxDaughter1 = std::max(1, maxDaughter1);
                    maxDaughter2 = left - maxDaughter1;
                    maxDaughter2 = std::max(1, maxDaughter2);
                    success = true;
                    if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
                        success = false;
                        check += "ERROR: It looked like you were only using the first 16 outputs but even accounting for that there are too many pixels on this port.\n";
                        logger_base.error("It looked like you were only using the first 16 outputs but even accounting for that there are too many pixels on this port.");
                    }
                }
                else if (cud.GetMaxPixelPort() <= 32 && maxDaughter2 == 50) {
                    maxDaughter2 = maxPixels - maxMain - maxDaughter1;
                    maxDaughter2 = std::max(1, maxDaughter2);
                    success = true;
                    if (maxMain + maxDaughter1 + maxDaughter2 > maxPixels) {
                        success = false;
                        check += "ERROR: It looked like you were only using the first 32 outputs but even accounting for that there are too many pixels on this port.\n";
                        logger_base.error("It looked like you were only using the first 32 outputs but even accounting for that there are too many pixels on this port.");
                    }
                }
                else {
                    check += "ERROR: Unable to adjust banks. Resetting your controller to its defaults may help.\n";
                    logger_base.error("    Unable to adjust banks. Resetting your controller to its defaults may help.");
                }
            }
        }

        if (maxMain + maxDaughter1 + maxDaughter2 < maxPixels) {
            if (maxDaughter2 != 0) {
                maxDaughter2 = maxPixels - maxMain - maxDaughter1;
            }
            else if (maxDaughter1 != 0) {
                maxDaughter1 = maxPixels - maxMain;
            }
            else {
                maxMain = maxPixels;
            }
            logger_base.info("Falcon pixel split adjusted to add up to %d: Main = %d, Expansion1 = %d, Expansion2 = %d", maxPixels, maxMain, maxDaughter1, maxDaughter2);
        }
    }

    if (success && cud.GetMaxPixelPort() > 0) {
        for (int i = 1; i <= cud.GetMaxPixelPort(); i += 4) {
            bool smartRemoteFound = false;
            bool nonSmartRemoteFound = false;
            for (int j = 0; j < 4; j++) {
                if (cud.GetControllerPixelPort(i + j)->AtLeastOneModelIsUsingSmartRemote()) {
                    smartRemoteFound = true;
                }
                if (cud.GetControllerPixelPort(i + j)->AtLeastOneModelIsNotUsingSmartRemote()) {
                    nonSmartRemoteFound = true;
                }
            }

            if (smartRemoteFound && nonSmartRemoteFound) {
                success = false;
                check += wxString::Format("ERROR: Ports %d-%d have a mix of models using smart and non-smart remotes. This is not supported.\n", i, i + 3);
                logger_base.error("Ports %d-%d have a mix of models using smart and non-smart remotes. This is not supported.", i, i + 3);
            }
        }
    }

    if (success && stringData.size() > 0) {
        if (doProgress) progress->Update(60, "Uploading string ports.");

        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
            check = ""; // to suppress double display
        }

        if (absoluteonebased) {
            logger_base.info("Applying one based adjustment, Subtracting = %d", firstchannel - 1);
            for (auto& it : stringData) {
                if (it->startChannel >= firstchannel - 1) {
                    it->startChannel -= (firstchannel - 1);
                }
            }
        }

        logger_base.info("Uploading string ports.");
        UploadStringPorts(stringData, maxMain, maxDaughter1, maxDaughter2, minuniverse, defaultBrightness, firstchanneloncontroller);
    }
    else {
        if (stringData.size() > 0 && caps->GetMaxPixelPort() > 0 && UDController::IsError(check)) {
            DisplayError("Not uploaded due to errors.\n" + check);
            check = "";
        }
    }

    // delete all our string data
    while (stringData.size() > 0) {
        delete stringData.back();
        stringData.pop_back();
    }

    if (success && cud.GetMaxSerialPort() > 0) {
        if (doProgress) progress->Update(90, "Uploading serial ports.");

        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
        }

        bool sendSerial = false;

        std::string uri = "a=";
        if (_usingAbsolute) {
            uri += "0";
        }
        else {
            uri += "1";
        }
        uri += "&btnSave=Save";

        for (int sp = 1; sp <= cud.GetMaxSerialPort(); sp++) {
            if (cud.HasSerialPort(sp)) {
                sendSerial = true;
                UDControllerPort* port = cud.GetControllerSerialPort(sp);
                int sc = port->GetStartChannel();
                logger_base.info("Serial Port %d Protocol %s Start Channel %d.", sp, (const char*)port->GetProtocol().c_str(), sc);

                uri += "&";
                uri += GetSerialOutputURI(caps, port->GetPort(), outputManager, DecodeSerialOutputProtocol(port->GetProtocol()), sc, parent);
            }
        }

        if (sendSerial)
        {
            PutURL("/SerialOutputs.htm", uri);
        }
    }
    else {
        if (caps->GetMaxSerialPort() > 0 && UDController::IsError(check)) {
            DisplayError("Not uploaded due to errors.\n" + check);
            check = "";
        }
    }

    if (!success && check != "") {
        DisplayError("Not uploaded due to errors.\n" + check);
        check = "";
    }

    if (doProgress) progress->Update(100, "Done.");
    logger_base.info("Falcon upload done.");

    return success;
}
#endif
#pragma endregion
