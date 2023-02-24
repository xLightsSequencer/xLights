
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Minleon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include "../utils/Curl.h"

#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../outputs/DDPOutput.h"
#include "../models/ModelManager.h"
#include "ControllerUploadData.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

#pragma region MinleonString Handling
class MinleonString
{
public:
	int _port = -1;
	int _tees = 0;
	bool _reverse = false;
	int _nodes = 0;
	int _startChannel = 0;
    void Dump(int startUniverse) const {

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (startUniverse == -1) {
            logger_base.debug("    Port %02d Tees %d %s Nodes %d Start %d",
                _port + 1,
                _tees,
                (const char*)(_reverse ? _("REVERSE") : _("")).c_str(),
                _nodes,
                _startChannel
            );
        }
        else {
            logger_base.debug("    Port %02d Tees %d %s Nodes %d Start %d (Universe %d, Start Channel %d)",
                _port + 1,
                _tees,
                (const char*)(_reverse ? _("REVERSE") : _("")).c_str(),
                _nodes,
                _startChannel,
                startUniverse + (_startChannel - 1) / 510,
                (_startChannel - 1) % 510 + 1
            );
        }
    }
    MinleonString(wxJSONValue& val)
    {
        if (val["port"].IsInt()) {
            // this is the web request response
            if (val.HasMember("port")) {
                _port = val["port"].AsInt();
            } else {
                _port = val["p"].AsInt();
            }
            _tees = val["ts"].AsInt();
            _reverse = (val["rev"].AsInt() == 1);
            _nodes = val["l"].AsInt();
            _startChannel = val["ss"].AsInt();
        }
        else {
            // This is the DDP config response
            if (val.HasMember("port")) {
                _port = wxAtoi(val["port"].AsString());
            } else {
                _port = val["p"].AsInt();
            }
            _tees = wxAtoi(val["ts"].AsString());
            _reverse = false;
            _nodes = wxAtoi(val["l"].AsString());
            _startChannel = wxAtoi(val["ss"].AsString());
        }
    }
    MinleonString(int port, int ts, bool reverse, int nodes, int startChannel)
    {
        _port = port;
        _tees = ts;
        _reverse = reverse;
        _nodes = nodes;
        _startChannel = startChannel;
    }
    MinleonString() {}
};

void Minleon::ParseStringPorts(std::vector<MinleonString*>& stringPorts, wxJSONValue& val) const
{
    for (auto& it : stringPorts)
    {
        delete it;
    }
    stringPorts.clear();

    for (int i = 0; i < val.Size(); i++)
    {
        stringPorts.push_back(new MinleonString(val[i]));
    }
}

void Minleon::InitialiseStrings(std::vector<MinleonString*>& stringsData, int max) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Filling in missing strings.");

    std::vector<MinleonString*> newStringsData;

    for (int i = 0; i < max; i++) {
        bool added = false;
        for (const auto& sd : stringsData) {
            if (sd->_port == i) {
                newStringsData.push_back(sd);
                added = true;
            }
        }
        if (!added) {
            MinleonString* string = new MinleonString();
            string->_port = i;
            newStringsData.push_back(string);
            logger_base.debug("    Added default string to port %d.", i + 1);
        }
    }
    stringsData = newStringsData;
}

std::string Minleon::BuildStringPort(MinleonString* string) const {

    int start = 25 + string->_port * 3;

    return wxString::Format("&L%03d=%d&I%03d=%d%sI%03d=%d",
        start, string->_tees,
        (string->_reverse ? wxString::Format("H%03d=0", start) : _("")),
        start + 1, string->_nodes,
        start + 2, string->_startChannel
    ).ToStdString();
}

MinleonString* Minleon::FindPort(const std::vector<MinleonString*>& stringData, int port) const {

    for (const auto& it : stringData) {
        if (it->_port == port)
        {
            return it;
        }
    }
    wxASSERT(false);
    return nullptr;
}

int Minleon::GetPixelCount(const std::vector<MinleonString*>& stringData, int port) const {

    int count = 0;
    for (const auto& sd : stringData) {
        if (sd->_port == port) {
            count += sd->_nodes;
        }
    }
    return count;
}

int Minleon::GetMaxPixelPort(const std::vector<MinleonString*>& stringData) const {

    int max = 0;
    for (const auto& sd : stringData) {
        if (sd->_port + 1 > max) {
            max = sd->_port + 1;
        }
    }
    return max;
}

void Minleon::DumpStringData(std::vector<MinleonString*> stringData, int startUniverse) const {

    for (const auto& sd : stringData) {
        sd->Dump(startUniverse);
    }
}

void Minleon::SetTimingsFromProtocol()
{
    if (_protocol == "ws2811") {
        _t0h = 350;
        _t1h = 650;
        _tbit = 1250;
        _tres = 280;
    } else if (_protocol == "ws2812b") {
        _t0h = 400;
        _t1h = 800;
        _tbit = 1250;
        _tres = 50;
    } else if (_protocol == "ws2818") {
        _t0h = 400;
        _t1h = 800;
        _tbit = 1250;
        _tres = 300;
    } else if (_protocol == "sk6812") {
        _t0h = 300;
        _t1h = 600;
        _tbit = 1250;
        _tres = 80;
    } else if (_protocol == "gs820x") {
        _t0h = 310;
        _t1h = 930;
        _tbit = 1250;
        _tres = 300;
    } else if (_protocol == "gs8206/8") {
        _t0h = 310;
        _t1h = 930;
        _tbit = 1250;
        _tres = 300;
    } else if (_protocol == "ucs2903") {
        _t0h = 250;
        _t1h = 1000;
        _tbit = 1250;
        _tres = 24;
    } else if (_protocol == "tm18xx") {
        _t0h = 500;
        _t1h = 1000;
        _tbit = 1500;
        _tres = 10;
    } else if (_protocol == "tm1804") {
        _t0h = 500;
        _t1h = 1000;
        _tbit = 1500;
        _tres = 10;
    } else if (_protocol == "sm16703") {
        _t0h = 300;
        _t1h = 900;
        _tbit = 1250;
        _tres = 80;
    } else if (_protocol == "apa104") {
        _t0h = 400;
        _t1h = 800;
        _tbit = 1250;
        _tres = 50;
    } else if (_protocol == "ucs8903") {
        _t0h = 400;
        _t1h = 850;
        _tbit = 1260;
        _tres = 100;
    } else if (_protocol == "rgb+") {
        _t0h = 400;
        _t1h = 850;
        _tbit = 1260;
        _tres = 1000;
    } else if (_protocol == "rgbw+") {
        _t0h = 400;
        _t1h = 850;
        _tbit = 1260;
        _tres = 1000;
    } else if (_protocol == "rgb+2") {
        _t0h = 350;
        _t1h = 700;
        _tbit = 1250;
        _tres = 250;
    } else if (_protocol == "rm2021") {
        _t0h = 300;
        _t1h = 900;
        _tbit = 1250;
        _tres = 200;
    } else {
        _t0h = 400;
        _t1h = 850;
        _tbit = 1260;
        _tres = 100;
    }
}

void Minleon::PostURL(const std::string& url, const std::string& data) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto response = Curl::HTTPSPost("http://" + _ip + url, data);
    logger_base.debug("%s", (const char*)response.c_str());
}

int Minleon::GetMax8PortPixels(const std::string& chip) const
{
    if (chip == "rgb+")
        return 460;
    if (chip == "rgbw+")
        return 345;
    if (chip == "rgb+2")
        return 613;
    return 920;
}

int Minleon::GetMax16PortPixels(const std::string& chip) const
{
    if (chip == "rgb+") return 230;
    if (chip == "rgbw+") return 172;
    if (chip == "rgb+2") return 306;
    return 460;
}
#pragma endregion

#pragma region Port Handling
void Minleon::UploadNDBPro(bool reboot)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int universe = 0;
    if (_startUniverse != -1)
        universe = _startUniverse;

    std::string data = wxString::Format("{\"chip\":\"%d\",\"conv\":\"%s\",\"t0h\":\"%d\",\"t1h\":\"%d\",\"tbit\":\"%d\",\"trst\":\"%d\",\"proto\":\"%d\",\"universe\":\"%d\",\"ports\":[", 
        EncodeStringPortProtocol(_chip),
        _conv,
        _t0h, _t1h, _tbit, _tres, 
        EncodeInputProtocol(_protocol),
        universe);
    bool first = true;
    for (const auto& it : _stringPorts) {
        if (!first) {
            data += ",";
        } else {
            first = false;
        }
        data += wxString::Format("{\"p\":%d,\"ts\":\"%d\",\"l\":\"%d\",\"rev\":\"%d\",\"ss\":\"%d\"}",
                                 it->_port,
                                 it->_tees,
                                 it->_nodes,
                                 it->_reverse ? 1 : 0,
            it->_startChannel
            );
    }
    data += "]}";

    logger_base.debug("%s", (const char*)data.c_str());

    PostURL("/api/config", data);
}

void Minleon::UploadNDB(bool reboot)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int universe = 0;
    if (_startUniverse != -1)
        universe = _startUniverse;

    std::map<std::string, std::string> parms;

    auto ips = wxSplit(_ip, '.');
    parms["I000"] = ips.size() > 0 ? ips[0] : "0";
    parms["I001"] = ips.size() > 1 ? ips[1] : "0";
    parms["I002"] = ips.size() > 2 ? ips[2] : "0";
    parms["I003"] = ips.size() > 3 ? ips[3] : "0";

    auto nms = wxSplit(_nm, '.');
    parms["I004"] = nms.size() > 0 ? nms[0] : "0";
    parms["I005"] = nms.size() > 1 ? nms[1] : "0";
    parms["I006"] = nms.size() > 2 ? nms[2] : "0";
    parms["I007"] = nms.size() > 3 ? nms[3] : "0";

    auto gws = wxSplit(_gw, '.');
    parms["I008"] = gws.size() > 0 ? gws[0] : "0";
    parms["I009"] = gws.size() > 1 ? gws[1] : "0";
    parms["I010"] = gws.size() > 2 ? gws[2] : "0";
    parms["I011"] = gws.size() > 3 ? gws[3] : "0";

    parms["I012"] = wxString::Format("%d", EncodeInputProtocol(_protocol));

    parms["I013"] = wxString::Format("%d", universe);

    int i = 14;
    for (const auto& it : _stringPorts) {
        parms[wxString::Format("I%03d", i++)] = wxString::Format("%d", it->_tees);
        parms[wxString::Format("I%03d", i++)] = wxString::Format("%d", it->_nodes);
        parms[wxString::Format("I%03d", i++)] = wxString::Format("%d", it->_startChannel);
    }
    parms["op"] = "Save";

    std::string send;
    for (const auto& it : parms) {
        if (send != "")
            send += "&";
        send += it.first + "=" + it.second;
    }

    logger_base.debug((const char*)send.c_str());

    PostURL("/00.html", send);

    if (reboot) {
        parms.clear();
        parms["op"] = "Reboot";
        send = "";
        for (const auto& it : parms) {
            if (send != "")
                send += "&";
            send += it.first + "=" + it.second;
        }

        PostURL("/00.html", send);
    }
}

void Minleon::UploadNDPPlus(bool reboot)
{

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    SetTimingsFromProtocol();

    if (_ndbPro) {
        return UploadNDBPro(reboot);
    } else if (_ndbOrig) {
        return UploadNDB(reboot);
    }

    int universe = 0;
    if (_startUniverse != -1) universe = _startUniverse;

    std::map<std::string, std::string> parms;

    auto ips = wxSplit(_ip, '.');
    parms["I000"] = ips.size() > 0 ? ips[0]: "0";
    parms["I001"] = ips.size() > 1 ? ips[1]: "0";
    parms["I002"] = ips.size() > 2 ? ips[2] : "0";
    parms["I003"] = ips.size() > 3 ? ips[3] : "0";

    auto nms = wxSplit(_nm, '.');
    parms["I004"] = nms.size() > 0 ? nms[0] : "0";
    parms["I005"] = nms.size() > 1 ? nms[1] : "0";
    parms["I006"] = nms.size() > 2 ? nms[2] : "0";
    parms["I007"] = nms.size() > 3 ? nms[3] : "0";

    auto gws = wxSplit(_gw, '.');
    parms["I008"] = gws.size() > 0 ? gws[0] : "0";
    parms["I009"] = gws.size() > 1 ? gws[1] : "0";
    parms["I010"] = gws.size() > 2 ? gws[2] : "0";
    parms["I011"] = gws.size() > 3 ? gws[3] : "0";

    parms["I012"] = wxString::Format("%d", EncodeInputProtocol(_protocol));

    parms["I013"] = wxString::Format("%d", universe);

    parms["I015"] = wxString::Format("%d", _t0h);
    parms["I016"] = wxString::Format("%d", _t1h);
    parms["I017"] = wxString::Format("%d", _tbit);
    parms["I018"] = wxString::Format("%d", _tres);
    parms["I019"] = _conv;

    parms["I020"] = wxString::Format("%d", EncodeStringPortProtocol(_chip));

    parms["I021"] = wxString::Format("%d", (int)_stringPorts.size());

    parms["I022"] = wxString::Format("%d", _grouping);

    int i = 25;
    for (const auto& it : _stringPorts) {
        if (it->_reverse) {
            parms[wxString::Format("H%03d", i)] = "0";
        }
        parms[wxString::Format("L%03d", i++)] = wxString::Format("%d", it->_tees);
        parms[wxString::Format("I%03d", i++)] = wxString::Format("%d", it->_nodes);
        parms[wxString::Format("I%03d", i++)] = wxString::Format("%d", it->_startChannel);
    }
    parms["op"] = "Save";

    std::string send;
    for (const auto& it : parms) {
        if (send != "") send += "&";
        send += it.first + "=" + it.second;
    }

    logger_base.debug((const char*)send.c_str());

    PostURL("/00.html", send);

    if (reboot) {
        parms.clear();
        parms["op"] = "Reboot";
        send = "";
        for (const auto& it : parms) {
            if (send != "") send += "&";
            send += it.first + "=" + it.second;
        }

        PostURL("/00.html", send);
    }
}
#pragma endregion

#pragma region Encode and Decode
static std::map<int, std::string> NDBProProtocols = {
    { 1, "ucs8903" },
    { 2, "rgbw+" },
    { 3, "sk6812" },
    { 4, "ws2811" },
    { 5, "ws2811" },
    { 6, "ws2811" },
    { 7, "gs820x" },
    { 8, "ucs2903" },
    { 9, "tm18xx" },
    { 10, "sm16703" },
    { 11, "apa104" },
    { 12, "rgb+2" },
    { 13, "ws2811" },
    { 14, "rm2021" }
};

std::string Minleon::DecodeStringPortProtocol(int protocol) const
{
    if (NDBProProtocols.find(protocol) != NDBProProtocols.end())
        return NDBProProtocols[protocol];
    return "ws2811";
}

int Minleon::EncodeStringPortProtocol(const std::string& protocol) const
{
    for (const auto& it : NDBProProtocols) {
        if (it.second == protocol)
            return it.first;
    }
    return 6;
}

int Minleon::EncodeInputProtocol(const std::string& protocol) const {

    if (protocol == OUTPUT_E131) return 2;
    if (protocol == OUTPUT_ARTNET) return 1;
    if (protocol == OUTPUT_DDP) return 0;

    return -1;
}

std::string Minleon::DecodeInputProtocol(int protocol) const {

    switch (protocol)
    {
    case 0:
        return OUTPUT_DDP;
    case 1:
        return OUTPUT_ARTNET;
    case 2:
        return OUTPUT_E131;
    }

    return "";
}
#pragma endregion

#pragma region Private Functions
#pragma endregion

#pragma region Constructors and Destructors
Minleon::Minleon(const std::string& ip, const std::string& proxy, const std::string& forceLocalIP) :
    BaseController(ip, proxy)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _version = "";
    _protocol = "";
    _startUniverse = -1;
    _ports = 0;

    logger_base.debug("Connecting to Minleon on %s.", (const char*)_ip.c_str());

    logger_base.debug("Getting minleon status.");
    auto status = DDPOutput::Query(_ip, DDP_ID_STATUS, forceLocalIP);

#ifdef USEDDP
    if (!status.IsNull()) {
        _protocol = "DDP";
        _version = status["status"]["ver"].AsString();
        logger_base.debug("   Version: %s", (const char*)_version.c_str());
        logger_base.debug("   Manufacturer: %s", (const char*)status["status"]["man"].AsString().c_str());
        logger_base.debug("   Model: %s", (const char*)status["status"]["mod"].AsString().c_str());
        logger_base.debug("   Push: %s", (const char*)status["status"]["push"].AsString().c_str());
        logger_base.debug("   MAC: %s", (const char*)status["status"]["mac"].AsString().c_str());

        logger_base.debug("Getting minleon status.");
        auto config = DDPOutput::Query(_ip, DDP_ID_CONFIG);
        _ports = config["config"]["ports"].AsArray()->Count();
        ParseStringPorts(_stringPorts, config["config"]["ports"]);
        logger_base.debug("Downloaded string data.");
        DumpStringData(_stringPorts, -1);
        _connected = true;
    } else {
#endif
        _connected = true;
        std::string html = GetURL("/");
        if (html == "") {
            html = GetURL("/psys.html");
        }
        logger_base.debug("/:\n%s", (const char*)html.c_str());
        //</script>NDB+ v2.2
        //<p><form>
        wxRegEx extractVersion("\\/script>([^<\r\n]*)(\r|\n|<)", wxRE_ADVANCED | wxRE_NEWLINE);
        if (extractVersion.Matches(wxString(html))) {
            _version = extractVersion.GetMatch(wxString(html), 1).ToStdString();
            logger_base.debug("Firmware version : %s", (const char*)_version.c_str());
        } else {
            logger_base.debug("Firmware version : Unable to determine.");
        }

        std::string configJSON = GetURL("/01.html");

        if (configJSON == "" || configJSON == "This URI does not exist") {
            logger_base.warn("    Error retrieving 01.html from Minleon controller.");

            configJSON = GetURL("/api/config");

            if (configJSON == "" || configJSON == "This URI does not exist") {

                // it may be an original NDB
                if (ParseNDBHTML(html, 4) != "") {
                    logger_base.info("    Original NDB.");
                    _nm = ParseNDBHTML(html, 4) + "." + ParseNDBHTML(html, 5) + "." + ParseNDBHTML(html, 6) + "." + ParseNDBHTML(html, 7);
                    _gw = ParseNDBHTML(html, 8) + "." + ParseNDBHTML(html, 9) + "." + ParseNDBHTML(html, 10) + "." + ParseNDBHTML(html, 11);
                    if (ParseNDBHTML(html, 12) == "1") {
                        _protocol = OUTPUT_DDP;
                    } else if (ParseNDBHTML(html, 13) == "1") {
                        _protocol = OUTPUT_ARTNET;
                    }
                    _startUniverse = wxAtoi(ParseNDBHTML(html, 13));
                    uint8_t index = 14;
                    for (uint8_t i = 0; i < 16; ++i) {
                        // Ts
                        auto ts = wxAtoi(ParseNDBHTML(html, index++));
                        // Lights
                        auto lights = wxAtoi(ParseNDBHTML(html, index++));
                        // Slot
                        auto slot = wxAtoi(ParseNDBHTML(html, index++));
                        _stringPorts.push_back(new MinleonString(i, ts, false, lights, slot));
                    }
                    _ndbOrig = true;
                } else {
                    logger_base.warn("    Error retrieving api/config from Minleon controller.");
                    _connected = false;
                    return;
                }
            }

            _ndbPro = true;

            logger_base.debug("api/config:\n%s", (const char*)configJSON.c_str());

        } else {
            logger_base.debug("01.html:\n%s", (const char*)configJSON.c_str());
        }

        if (!_ndbOrig) {
            wxJSONValue val;
            wxJSONReader reader;
            reader.Parse(configJSON, &val);

            _nm = val["config"]["nm"].AsString();
            _gw = val["config"]["gw"].AsString();
            _t0h = wxAtoi(val["config"]["t0h"].AsString());
            _t1h = wxAtoi(val["config"]["t1h"].AsString());
            _tbit = wxAtoi(val["config"]["tbit"].AsString());
            _tres = wxAtoi(val["config"]["tres"].AsString());
            _conv = val["config"]["conv"].AsString();

            std::string p;
            if (val["config"].HasMember("protocol")) {
                p = val["config"]["protocol"].AsString();
            } else {
                html = GetURL("/pout.html");
                wxRegEx extractProtocol("type=\"radio\"[^>]*value=\"([^\"]*)[^>]* checked>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (extractProtocol.Matches(wxString(html))) {
                    p = extractProtocol.GetMatch(wxString(html), 1).ToStdString();
                }
            }
            if (p == "E1.31" || p == "2") {
                _protocol = OUTPUT_E131;
            } else if (p == "Art-Net" || p == "1") {
                _protocol = OUTPUT_ARTNET;
            } else if (p == "DDP" || p == "0") {
                _protocol = OUTPUT_DDP;
            } else {
                _protocol = p;
            }

            if (_protocol != OUTPUT_DDP) {
                if (val["config"].HasMember("universe")) {
                    _startUniverse = wxAtoi(val["config"]["universe"].AsString());
                } else {
                    _startUniverse = wxAtoi(val["config"]["univ"].AsString());
                }
            }

            _chip = DecodeStringPortProtocol(wxAtoi(val["config"]["chip"].AsString()));
            if (val["config"].HasMember("nports")) {
                _ports = wxAtoi(val["config"]["nports"].AsString());
            } else {
                _ports = val["config"]["ports"].AsArray()->Count();
            }
            if (val["config"].HasMember("rpt")) {
                _grouping = wxAtoi(val["config"]["rpt"].AsString());
            } else {
                _grouping = 1;
            }
            ParseStringPorts(_stringPorts, val["config"]["ports"]);
        }

        logger_base.debug("Downloaded string data.");
        DumpStringData(_stringPorts, _startUniverse);

        logger_base.debug("Connected to Minleon %s", (const char*)_version.c_str());
#ifdef USEDDP
    }
#endif
}

Minleon::~Minleon() {
    for (auto& it : _stringPorts)
    {
        delete it;
    }
    _stringPorts.clear();
}
#pragma endregion

std::string Minleon::ParseNDBHTML(const std::string& html, uint8_t index)
{
    auto tag = wxString::Format("I%03d", index).ToStdString();
    wxRegEx extractTagValue("name=\"I" + tag + "\"value=\"([^\"]*)\"",
                            wxRE_ADVANCED | wxRE_NEWLINE);
    if (extractTagValue.Matches(wxString(html))) {
        return extractTagValue.GetMatch(wxString(html), 1).ToStdString();
    }
    return "";
}

#pragma region Static Functions
#pragma endregion

#pragma region Getters and Setters
bool Minleon::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {

    //ResetStringOutputs(); // this shouldnt be used normally

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Minleon Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    progress.Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    auto caps = ControllerCaps::GetControllerConfig(controller->GetVendor(), controller->GetModel(), controller->GetVariant());
    bool success = true;

    if (caps != nullptr) {
        success = cud.Check(caps, check);
    }

    if (controller->IsFullxLightsControl()) {
        for (auto& it : _stringPorts) {
            delete it;
        }
        _stringPorts.clear();
    }

    logger_base.debug(check);
    cud.Dump();

    bool reboot = false;
    if (success) {
        int neededports = cud.GetMaxPixelPort();
        if (neededports <= 8) {
            neededports = 8;
        }
        else {
            neededports = 16;
        }

        if (_protocol != controller->GetProtocol()) {
            reboot = true;
            _protocol = controller->GetProtocol();
        }

        _startUniverse = -1;
        if (controller->GetProtocol() != OUTPUT_DDP) {
            _startUniverse = controller->GetFirstOutput()->GetUniverse();
        }

        bool chipSet = false;
        bool groupingSet = false;
        bool rgbSet = false;
        _conv = "4";

        int maxpixelsfor16ports = GetMax16PortPixels(_chip);

        int maxChannels = cud.GetMaxPixelPortChannels();
        int maxPorts = 16;
        if (maxChannels > 460 * 3) {
            maxPorts = 8;
        }
        if (neededports > maxPorts) {
            check += wxString::Format("Needed ports %d but maximum possible ports is only %d.\n", neededports, maxPorts);
            success = false;
        }

        if (success) {

            // get the string ports to the right size
            while (_stringPorts.size() < maxPorts) {
                _stringPorts.push_back(new MinleonString(_stringPorts.size(), 0, false, 0, 1));
            }
            while (_stringPorts.size() > maxPorts) {
                delete _stringPorts.back();
                _stringPorts.pop_back();
            }

            // now make them correct
            bool usingSmartTs = false;
            bool allsameorzero = true;
            int allsame = -1;
            for (const auto& it : _stringPorts) {
                auto p = cud.GetControllerPixelPort(it->_port + 1);
                if (p != nullptr) {
                    p->CreateVirtualStrings(true);
                    auto vs = p->GetVirtualString(0);
                    if (vs != nullptr) {
                        if (!chipSet) {
                            chipSet = true;
                            _chip = vs->_protocol;
                        }
                        if (!_ndbPro) {
                            if (!groupingSet && vs->_groupCountSet) {
                                groupingSet = true;
                                if (vs->_groupCount <= 1) {
                                    _grouping = 0;
                                } else {
                                    _grouping = (maxChannels / 3) / vs->_groupCount;
                                }
                            }
                        }
                        if (!rgbSet && vs->_colourOrderSet) {
                            rgbSet = true;
                            if (vs->_colourOrder == "RGB") _conv = "4";
                            else if (vs->_colourOrder == "GRB") _conv = "5";
                            else if (vs->_colourOrder == "BGR") _conv = "6";
                        }
                        it->_tees = vs->_ts;
                        it->_reverse = vs->_reverse == "Reverse";
                        if (it->_tees > 0) {
                            it->_nodes = (vs->Channels() / 3) / it->_tees;
                        }
                        else {
                            it->_nodes = vs->Channels() / 3;
                        }
                        it->_startChannel = vs->_startChannel - controller->GetStartChannel() + 1;

                        if (vs->Channels() / 3 > maxpixelsfor16ports * (maxPorts == 16 ? 1 : 2)) {
                            check += wxString::Format("Port %d has %d pixels but can only support %d.\n", it->_port + 1, vs->Channels() / 3, maxpixelsfor16ports * (maxPorts == 16 ? 1 : 2));
                            success = false;
                        }
                    }
                }
                usingSmartTs |= it->_tees > 0;
                if (allsame == -1 && it->_nodes != 0) {
                    allsame = it->_nodes;
                }
                else {
                    if (it->_nodes != 0 && it->_nodes != allsame) {
                        allsameorzero = false;
                    }
                }
            }

            if (usingSmartTs && !allsameorzero) {
                check += "When using smart Ts all ports must be zero or the same size.\n";
                success = false;
            }

            logger_base.debug("Minleon port data prepared.");
            DumpStringData(_stringPorts, _startUniverse);

            progress.Update(10, "Port data prepared.");
        }
    }

    if (success && cud.GetMaxPixelPort() > 0) {
        progress.Update(60, "Uploading string ports.");

        if (check != "") {
            DisplayWarning("Upload warnings:\n" + check);
            check = ""; // to suppress double display
        }

        logger_base.info("Uploading string ports.");
        UploadNDPPlus(reboot);
    }
    else {
        if (cud.GetMaxPixelPort() > 0 && (caps == nullptr || caps->GetMaxPixelPort() > 0) && check != "") {
            DisplayError("Not uploaded due to errors.\n" + check);
            check = "";
        }
    }

    progress.Update(100, "Done.");
    logger_base.info("Minleon upload done.");

    return success;
}
#pragma endregion
