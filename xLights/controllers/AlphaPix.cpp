#include "AlphaPix.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>

#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"

#include <wx/sckstrm.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>

#include <log4cpp/Category.hh>

#include "ControllerRegistry.h"
#include "UtilFunctions.h"

#include <curl/curl.h>

static std::vector<AlphaPixControllerRules> CONTROLLER_TYPE_MAP = {
    AlphaPixControllerRules(4)
};

void AlphaPixOutput::Dump() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("    Output %d Uni %d StartChan %d Pixels %d Rev %d Nulls %d Brightness %d ZigZag %d",
        output,
        universe,
        startChannel,
        pixels,
        reverse,
        nullPixel,
        brightness,
        zigZag
        );
}

void AlphaPixData::Dump() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("    Name %s Protocol %d UseDmx %d DMXUniverse %d ColorOrder %d InputMode %d",
        name.ToStdString().c_str(),
        protocol,
        useDmx,
        dmxUniverse,
        colorOrder,
        inputMode
    );
}

void AlphaPix::RegisterControllers() {
    for (auto &a : CONTROLLER_TYPE_MAP) {
        ControllerRegistry::AddController(&a);
    }
}

AlphaPix::AlphaPix(const std::string& ip, const std::string &proxy) : _ip(ip), _fppProxy(proxy), _baseUrl("")
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_fppProxy.empty()) {
        _baseUrl = "http://"+ _fppProxy + "/proxy/" + _ip;
    } else {
        _baseUrl = "http://" + _ip;
    }

    _page = GetURL("/");
    if (!_page.empty())
    {
        static wxRegEx modelregex("(\\d+) Port Ethernet to SPI Controller", wxRE_ADVANCED | wxRE_NEWLINE);
        if (modelregex.Matches(wxString(_page)))
        {
            _model = wxAtoi(modelregex.GetMatch(wxString(_page), 1).ToStdString());
            logger_base.warn("Connected to AlphaPix controller model %s.", (const char *)EncodeControllerType().c_str());
            _connected = true;
        }
        else
        {
            logger_base.error("Error Determining AlphaPix controller Type.");
            _connected = false;
        }

        //Currently Installed Firmware Version:  2.08
        static wxRegEx firmwareregex("(Currently Installed Firmware Version:  ([0-9]+.[0-9]+))", wxRE_ADVANCED | wxRE_NEWLINE);
        if (firmwareregex.Matches(wxString(_page)))
        {
            _firmware = firmwareregex.GetMatch(wxString(_page), 2).ToStdString();
            logger_base.warn("                                 firmware %s.", (const char*)_firmware.c_str());
        }
    }
    else
    {
        _connected = false;
        logger_base.error("Error connecting to AlphaPix controller on %s.", (const char *)_ip.c_str());
    }
}

AlphaPix::~AlphaPix()
{
    for (auto it = _outputData.begin(); it != _outputData.end(); ++it)
    {
        delete *it;
    }
    _outputData.clear();
}

bool AlphaPix::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("AlphaPix Outputs Upload: Uploading to %s", (const char *)_ip.c_str());

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    auto o = outputs.front();

    progress.Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(_ip, _ip, allmodels, outputManager, &selected, check);

    //first check rules
    AlphaPixControllerRules rules(_model);
    const bool success = cud.Check(&rules, check);

    logger_base.debug(check);

    cud.Dump();
    if (!success)
    {
        DisplayError("AlphaPix Upload Error:\n" + check, parent);
        progress.Update(100, "Aborting.");
        return false;
    }
    //get current config Page
    const wxString page = _page;

    if (page.empty())
    {
        DisplayError("AlphaPix Upload Error:\nWebpage was empty" , parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    bool worked = true;
    AlphaPixData controllerData = AlphaPixData();

    _connected = ParseWebpage(_page, controllerData);
    if (!_connected)
    {
        DisplayError("Unable to Parse Main Webpage.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    logger_base.info("Figuring Out Pixel Output Information.");
    progress.Update(10, "Figuring Out Pixel Output Information.");

    std::string colorOrder;
    std::string pixelType;

    //loop to setup string outputs
    for (int port = 1; port <= GetNumberOfOutputs(); port++)
    {
        AlphaPixOutput* pixOut = FindPortData(port);
        if (cud.HasPixelPort(port))
        {
            UDControllerPort* portData = cud.GetControllerPixelPort(port);
            AlphaPixOutput* pixOut = FindPortData(port);
            UpdatePortData(pixOut, portData);
            const std::string color = portData->GetFirstModel()->GetColourOrder("");
            if (!color.empty() && colorOrder.empty())
                colorOrder = color;
            if (pixelType.empty())
                pixelType = portData->GetFirstModel()->GetProtocol();
        }
    }

    logger_base.debug("Building pixel upload:");
    std::string requestString;
    for (const auto& outputD : _outputData)
    {
        if (requestString != "")
            requestString += "&";
        requestString += BuildStringPortRequest(outputD);
    }

    logger_base.info("Uploading String Output Information.");
    progress.Update(20, "Uploading String Output Information.");

    if (!requestString.empty())
    {
        const wxString res = PutURL(GetOutputURL(), requestString);
        if (res.empty())
            worked = false;
        wxMilliSleep(1000);
    }

    logger_base.info("Figuring Out DMX Output Information.");
    progress.Update(30, "Figuring Out DMX Output Information.");
    std::string serialRequest;

    for (int port = 1; port <= GetNumberOfSerial(); port++)
    {
        if (cud.HasSerialPort(port))
        {
            if (_model == 4)
            {
                if (serialRequest.empty())
                    serialRequest += "Rever5=1&";
                serialRequest += wxString::Format("DMX512=%d", cud.GetControllerSerialPort(port)->GetUniverse());
            }
            else
            {
                logger_base.info("DMX Output Information is Not Enabled for this Controller Type %d.", _model);
            }
        }
    }

    logger_base.info("Uploading DMX Output Information.");
    progress.Update(40, "Uploading DMX Output Information.");
    if (!serialRequest.empty())
    {
        const wxString res = PutURL(GetDMXURL(), serialRequest);
        if (res.empty())
            worked = false;
        wxMilliSleep(1000);
    }

    logger_base.info("Uploading Protocol Type.");
    progress.Update(50, "Uploading Protocol Type.");
    const int newProtocol = EncodeStringPortProtocol(pixelType);
    if (newProtocol != -1 && controllerData.protocol != newProtocol)
    {
        const wxString res = PutURL(GetProtocolURL(), wxString::Format("IC=%d", newProtocol));
        if (res.empty())
            worked = false;
        wxMilliSleep(1000);
    }

    logger_base.info("Uploading Color Order.");
    progress.Update(60, "Uploading Color Order.");
    if (!colorOrder.empty())
    {
        const int newColor = EncodeColorOrder(colorOrder);
        if (newColor != -1 && controllerData.colorOrder != newColor)
        {
            const wxString res = PutURL(GetColorOrderURL(), wxString::Format("RGBORD=0&RGBS=%d", newColor));
                if (res.empty())
                    worked = false;
                wxMilliSleep(1000);
        }
    }

    logger_base.info("Uploading Output Information.");
    progress.Update(70, "Uploading Output Description.");
    const std::string outName = SafeDescription(o->GetDescription());
    if (!outName.empty() && !controllerData.name.IsSameAs(outName))
    {
        const wxString res = PutURL(GetNameURL(), "name=" + outName);
        if (res.empty())
            worked = false;
        wxMilliSleep(1000);
    }

    //upload Input Type
    std::string requestInputString;
    if (o->GetType() == "E131")
    {
        if (controllerData.inputMode != 0)
            requestInputString = "EP=0";
    }
    else if (o->GetType() == "ArtNet")
    {
        if (controllerData.inputMode != 1)
            requestInputString = "EP=1";
    }

    logger_base.info("Uploading Output Type.");
    progress.Update(80, "Updating Output Type.");
    if (!requestInputString.empty())
    {
        const wxString res = PutURL(GetInputTypeURL(), requestInputString);
        if (res.empty())
            worked = false;
        //wait for reboot
        wxMilliSleep(5000);
    }

    return worked;
}

bool AlphaPix::ParseWebpage(const wxString& page, AlphaPixData& data)
{
    _outputData.clear();
    data.name = ExtractName(page);
    data.inputMode = ExtractInputType(page);
    data.useDmx = ExtractDMXEnabled(page);
    data.dmxUniverse = ExtractDMXUniverse(page);
    data.protocol = ExtractProtocol(page);
    data.colorOrder = ExtractColor(page);

    for (int i = 1; i <= GetNumberOfOutputs(); i++)
    {
        AlphaPixOutput* output = ExtractOutputData(page, i);
        output->Dump();
        _outputData.push_back(output);
    }

    return true;
}

AlphaPixOutput* AlphaPix::ExtractOutputData(const wxString& page, int port)
{
    const wxString p(page);
    int start = p.find("SPI Pixel Output Configuration");

    AlphaPixOutput* output = new AlphaPixOutput(port);

    output->universe = ExtractIntFromPage(page, wxString::Format("SU%d", port), "input", 1, start);
    output->startChannel = ExtractIntFromPage(page, wxString::Format("SC%d", port), "input", 1, start);

    output->pixels = ExtractIntFromPage(page, wxString::Format("PC%d", port), "input", 0, start);
    output->nullPixel = ExtractIntFromPage(page, wxString::Format("NP%d", port), "input", 0, start);
    output->zigZag = ExtractIntFromPage(page, wxString::Format("RA%d", port), "input", 0, start);
    output->brightness = ExtractIntFromPage(page, wxString::Format("LM%d", port), "input", 100, start);
    output->reverse = ExtractIntFromPage(page, wxString::Format("RV%d", port), "checkbox", 0, start);

    return output;
}

std::string AlphaPix::ExtractName(const wxString& page)
{
    const wxString p(page);
    int start = p.find("Device name:");

    std::string name = ExtractFromPage(page, "name", "input", start);

    return name;
}

int AlphaPix::ExtractInputType(const wxString& page)
{
    const wxString p(page);
    int start = p.find("Ethernet Protocol:");

    int value = ExtractIntFromPage(page, "EP", "radio", 0, start);

    return value;
}

int AlphaPix::ExtractDMXUniverse(const wxString& page)
{
    const wxString p(page);
    const int start = p.find("DMX Universe:");

    const int value = ExtractIntFromPage(page, "DMX512", "input", 0, start);

    return value;
}

int AlphaPix::ExtractProtocol(const wxString& page)
{
    const wxString p(page);
    const int start = p.find("Pixel IC Protocol:");

    const int protocol = ExtractIntFromPage(page, "IC", "select", 0, start);

    return protocol;
}

int AlphaPix::ExtractColor(const wxString& page)
{
    const wxString p(page);
    const int start = p.find("Pixel Chip Color Output Order:");

    const int colorOrder = ExtractIntFromPage(page, "RGBS", "select", 0, start);

    return colorOrder;
}

bool AlphaPix::ExtractDMXEnabled(const wxString& page)
{
    const wxString p(page);
    const int start = p.find("Enabled:");

    const bool useDMX = ExtractBoolFromPage(page, "Rever5", "checkbox", false, start);

    return useDMX;
}

void AlphaPix::UpdatePortData(AlphaPixOutput* pd, UDControllerPort* stringData) const
{
    if (pd != nullptr)
    {
        const int nullPix = stringData->GetFirstModel()->GetNullPixels(-1);
        if (nullPix != -1)
        {
            pd->nullPixel = nullPix;
        }

        const int brightness = stringData->GetFirstModel()->GetBrightness(-1);
        if (brightness != -1)
        {
            pd->brightness = brightness;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown")
        {
            const bool rev = EncodeDirection(direction);
            pd->reverse = rev;
        }

        const int newUniv = (stringData->GetUniverse());

        pd->universe = newUniv;

        const int startChan = stringData->GetUniverseStartChannel();
        pd->startChannel = startChan;

        const int pixels = stringData->Pixels();
        pd->pixels = pixels;
    }
}

std::string AlphaPix::ExtractFromPage(const wxString& page, const std::string& parameter, const std::string& type, int start)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    const wxString p = wxString(page).Mid(start);

    if (type == "input")
    {
        //<input  style = " width: 80px ;TEXT-ALIGN: center" type="text" value="1" name="DMX512"/>
        //<input\s+style\s=\s\".*\"\stype="text"\s+value=\")([0-9\\.]*?)\"
        //<input\s+style\s=\s\".*\"\stype=\"text\"\s+value=\"(.*)\"\s+name=\"SU1\"
        const wxString regex = "<input\\s+style\\s=\\s\\\".*\\\"\\stype=\\\"text\\\"\\s+value=\\\"(.*)\\\"\\s+name=\\\""+ parameter +"\\\"";
        //logger_base.debug("Regex:%s", (const char*)regex.c_str());

        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            const std::string res = inputregex.GetMatch(wxString(p), 1).ToStdString();
            return res;
        }
    }
    else if (type == "select")
    {
        int startSel = p.find("<select name=\"" + parameter + "\"");
        const wxString pSel = wxString(p).Mid(startSel);
        //<select name="RGBS"
        //<option value="([0-9])\"\sselected=\"selected\"\s>.*<\/option>
        const wxString regex = "<option\\s+value=\"([0-9])\\\"\\sselected=\\\"selected\\\"\\s>.*<\\/option>";
        //logger_base.debug("Regex:%s", (const char*)regex.c_str());
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(pSel)))
        {
            const std::string res = inputregex.GetMatch(wxString(pSel), 1).ToStdString();
            return res;
        }
    }
    else if (type == "checkbox")
    {
        //<input\s+(?:style\s=\s\".*\"\s+)?type=\"checkbox\"\s+name=\"(\w+)\"\s+(checked=\"checked\"\s+)?value=\"[0-9]\"
        const wxString regex = "<input\\s+(?:style\\s=\\s\\\".*\\\"\\s+)?type=\"checkbox\"\\s+name=\\\"" + parameter + "\"\\s+(checked=\\\"checked\\\"\\s+)?value=\\\"[0-9]\\\"";
        //logger_base.debug("Regex:%s", (const char*)regex.c_str());
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            auto count = inputregex.GetMatchCount();
           
            const std::string res = inputregex.GetMatch(wxString(p), 0).ToStdString();
            const std::string res2 = inputregex.GetMatch(wxString(p), 1).ToStdString();
            if (!res2.empty())
                return "1";
            return "0";
            //return res;
        }
    }
    else if (type == "radio")
    {
        // <input\s+type="radio"\s+(?:id="\w+")?\s+name=\"\w+"\s+value=\"([0-9])\"\s+checked="checked"
        const wxString regex = "<input\\s+type=\"radio\"\\s+(?:id=\"\\w+\")?\\s+name=\\\"" + parameter + "\"\\s+value=\\\"([0-9])\\\"\\s+checked=\"checked\"";
        //logger_base.debug("Regex:%s", (const char*)regex.c_str());
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            const std::string res = inputregex.GetMatch(wxString(p), 1).ToStdString();
            return res;
        }
    }
    else
    {
        logger_base.error("AlphaPix::ExtractFromPage   Invalid Regex Type:%s", (const char*)type.c_str());
        wxASSERT(false);
    }

    return "";
}

int AlphaPix::ExtractIntFromPage(const wxString& page, const std::string& parameter, const std::string& type, int defaultValue, int start)
{
    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty())
    {
        return wxAtoi(value);
    }
    return defaultValue;
}

bool AlphaPix::ExtractBoolFromPage(const wxString& page, const std::string& parameter, const std::string& type, bool defaultValue, int start)
{
    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty())
    {
        return value == "1";
    }
    return defaultValue;
}

int AlphaPix::EncodeStringPortProtocol(const std::string& protocol) const
{
    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return 0;
    if (p == "ws2801") return 1;
    if (p == "lpd6803") return 2;
    if (p == "tls3001") return 4;
    if (p == "tm18xx") return 6;
    wxASSERT(false);
    return -1;
}

int AlphaPix::EncodeColorOrder(const std::string& colorOrder) const
{
    wxString c(colorOrder);
    c = c.Lower();

    if (c == "rgb") return 0;
    if (c == "rbg") return 1;
    if (c == "grb") return 2;
    if (c == "gbr") return 3;
    if (c == "brg") return 4;
    if (c == "bgr") return 5;
    wxASSERT(false);
    return -1;
}

bool AlphaPix::EncodeDirection(const std::string& direction) const
{
    return direction == "Reverse";
}

std::string AlphaPix::EncodeControllerType() const
{
    return wxString::Format("%d %s", _model, _firmware).ToStdString();
}

AlphaPixOutput* AlphaPix::FindPortData( int output)
{
    for (AlphaPixOutput* sd : _outputData)
    {
        if (sd->output == output)
        {
            return sd;
        }
    }
    wxASSERT(false);
    return nullptr;
}

wxString AlphaPix::BuildStringPortRequest(AlphaPixOutput* po) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("     Output String %d, Universe %d StartChannel %d Pixels %d",
        po->output, po->universe, po->startChannel, po->pixels);

    std::string reverseAdd;
    if (po->reverse)
    {
        reverseAdd = wxString::Format("&RV%d=1", po->output);
    }

    return wxString::Format("SU%d=%d&SC%d=%d&PC%d=%d&NP%d=%d&RA%d=%d&LM%d=%d%s",
        po->output, po->universe,
        po->output, po->startChannel,
        po->output, po->pixels,
        po->output, po->nullPixel,
        po->output, po->zigZag,
        po->output,po->brightness,
        reverseAdd);
}

wxString AlphaPix::GetURL(const std::string& url, bool logresult)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;

    CURL* curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, std::string(_baseUrl + url).c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

        std::string response_string;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        /* Perform the request, res will get the return code */
        CURLcode r = curl_easy_perform(curl);

        if (r != CURLE_OK)
        {
            logger_base.error("Failure to access %s: %s.", (const char*)url.c_str(), curl_easy_strerror(r));
        }
        else
        {
            res = response_string;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return res;
}

wxString AlphaPix::PutURL(const std::string& url, const std::string& request, bool logresult)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Making request to AlphaPix '%s'.", (const char*)url.c_str());
    logger_base.debug("    With data '%s'.", (const char*)request.c_str());

    CURL* hnd = curl_easy_init();
    if (hnd != nullptr)
    {
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(hnd, CURLOPT_URL, std::string(_baseUrl + url).c_str());

        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, "content-type: application/x-www-form-urlencoded");
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request.c_str());

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 10);
        std::string buffer = "";
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &buffer);

        CURLcode ret = curl_easy_perform(hnd);
        curl_easy_cleanup(hnd);

        if (ret == CURLE_OK)
        {
            return buffer;
        }
        else
        {
            logger_base.error("Failure to access %s: %s.", (const char*)url.c_str(), curl_easy_strerror(ret));
        }
    }

    return "";
}

std::string AlphaPix::SafeDescription(const std::string description) const
{
    wxString desc(description);
    return desc.Left(16).ToStdString();
}
