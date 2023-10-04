
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SanDevices.h"
#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"
#include "../UtilFunctions.h"

#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/sckstrm.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>

#include <log4cpp/Category.hh>

#pragma region Dumps
void SanDevicesOutput::Dump() const {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("    Group %d Output %d Port %d Uni %c StartChan %d Pixels %d GroupCount %d Rev %s ColorOrder %c Nulls %d Brightness %c Chase %s firstZig %d thenEvery %d Upload %s",
        group,
        output,
        stringport,
        universe,
        startChannel,
        pixels,
        groupCount,
        toStr(reverse),
        colorOrder,
        nullPixel,
        brightness,
        toStr(chase),
        firstZig,
        thenEvery,
        toStr(upload)
        );
}

void SanDevicesProtocol::Dump() const {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("    Group %d Protocol %c Timing %c Upload %s",
        getGroup(),
        getProtocol(),
        getTiming(),
        toStr(shouldUpload()));
}

void SanDevicesOutputV4::Dump() const {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("    Group %d outputSize %d Uni %c StartChan %d Pixels %d GroupCount %d Rev %s,%s,%s,%s ColorOrder %d Nulls %d,%d,%d,%d ZigZag %d Upload %s",
        group,
        outputSize,
        universe,
        startChannel,
        pixels,
        groupCount,
        toStr(reverse[0]),
        toStr(reverse[1]),
        toStr(reverse[2]),
        toStr(reverse[3]),
        colorOrder,
        nullPixel[0],
        nullPixel[1],
        nullPixel[2],
        nullPixel[3],
        zigzag,
        toStr(upload)
    );
}
#pragma endregion

#pragma region MyHTTPStream
class MyHTTPStream : public wxSocketInputStream
{
public:
    wxHTTP * m_http = nullptr;
    size_t m_httpsize = 0;
    unsigned long m_read_bytes = 0;

    MyHTTPStream(wxHTTP *http) : wxSocketInputStream(*http) {

        m_http = http;
    }

    size_t GetSize() const wxOVERRIDE { return m_httpsize; }
    virtual ~MyHTTPStream(void) { m_http->Abort(); }

protected:
    size_t OnSysRead(void *buffer, size_t bufsize) wxOVERRIDE {

        if (m_read_bytes >= m_httpsize) {
            m_lasterror = wxSTREAM_EOF;
            return 0;
        }

        size_t ret = wxSocketInputStream::OnSysRead(buffer, bufsize);
        m_read_bytes += ret;

        if (m_httpsize == (size_t)-1 && m_lasterror == wxSTREAM_READ_ERROR) {
            // if m_httpsize is (size_t) -1 this means read until connection closed
            // which is equivalent to getting a READ_ERROR, for clients however this
            // must be translated into EOF, as it is the expected way of signalling
            // end end of the content
            m_lasterror = wxSTREAM_EOF;
        }

        return ret;
    }

    wxDECLARE_NO_COPY_CLASS(MyHTTPStream);
};
#pragma endregion

#pragma region SimpleHTTP
wxInputStream *SimpleHTTP::GetInputStream(const wxString& path, wxString& startResult) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    MyHTTPStream *inp_stream = nullptr;
    wxString new_path;

    m_lastError = wxPROTO_CONNERR;  // all following returns share this type of error
    if (!m_addr) {
        logger_base.error("SimpleHTTP::GetInputStream m_addr was null");
        return nullptr;
    }

    // We set m_connected back to false so wxSocketBase will know what to do.
#ifdef __WXMAC__
    wxSocketClient::Connect(*m_addr, false);
    wxSocketClient::WaitOnConnect(10);

    if (!wxSocketClient::IsConnected())
        return nullptr;
#else
    if (!wxProtocol::Connect(*m_addr)) {
        logger_base.error("SimpleHTTP::GetInputStream wxProtocol::Connect failed to %s.", (const char*)dynamic_cast<wxIPaddress*>(m_addr)->IPAddress().c_str());
        return nullptr;
    }

#endif

    // Use the user-specified method if any or determine the method to use
    // automatically depending on whether we have anything to post or not.
    wxString method = m_method;
    if (method.empty())
        method = m_postBuffer.IsEmpty() ? wxS("GET") : wxS("POST");

    if (!MyBuildRequest(path, method, startResult)) return nullptr;

    inp_stream = new MyHTTPStream(this);

    if (!GetHeader(wxT("Content-Length")).empty())
        inp_stream->m_httpsize = wxAtoi(GetHeader(wxT("Content-Length")));
    else
        inp_stream->m_httpsize = -1;

    inp_stream->m_read_bytes = 0;

    // no error; reset m_lastError
    m_lastError = wxPROTO_NOERR;
    return inp_stream;
}

bool SimpleHTTP::MyBuildRequest(const wxString& path, const wxString& method, wxString& startResult) {

    startResult = "";

    // Use the data in the post buffer, if any.
    if (!m_postBuffer.IsEmpty()) {
        wxString len;
        len << m_postBuffer.GetDataLen();

        // Content length must be correct, so always set, possibly
        // overriding the value set explicitly by a previous call to
        // SetHeader("Content-Length").
        SetHeader(wxS("Content-Length"), len);

        // However if the user had explicitly set the content type, don't
        // override it with the content type passed to SetPostText().
        if (!m_contentType.empty() && GetContentType().empty())
            SetHeader(wxS("Content-Type"), m_contentType);
    }

    m_http_response = 0;

    // If there is no User-Agent defined, define it.
    if (GetHeader(wxT("User-Agent")).empty())
        SetHeader(wxT("User-Agent"), wxVERSION_STRING);

    // Send authentication information
    if (!m_username.empty() || !m_password.empty()) {
        SetHeader(wxT("Authorization"), GenerateAuthString(m_username, m_password));
    }

    wxString buf;
    buf.Printf(wxT("%s %s HTTP/1.0\r\n"), method, path);
    const wxWX2MBbuf pathbuf = buf.mb_str();
    Write(pathbuf, strlen(pathbuf));
    SendHeaders();
    Write("\r\n", 2);

    if (!m_postBuffer.IsEmpty()) {
        Write(m_postBuffer.GetData(), m_postBuffer.GetDataLen());

        m_postBuffer.Clear();
    }

    wxString tmp_str;
    int i = 0;
    while (tmp_str == "" && i++ < 10) {
        m_lastError = ReadLine(this, tmp_str);

        // return only if error and no data
        if (m_lastError != wxPROTO_NOERR && tmp_str == "")
            return false;

        wxMilliSleep(10);
    }
    m_lastError = wxPROTO_NOERR;

    if (!tmp_str.Contains(wxT("HTTP/"))) {
        m_lastError = wxPROTO_NOERR;
        SetHeader(wxT("Content-Length"), wxString::Format("%d", (int)tmp_str.Length()));
        SetHeader(wxT("Content-Type"), wxT("text/html"));
        startResult = tmp_str;

        return true;
    }

    wxStringTokenizer token(tmp_str, wxT(' '));
    wxString tmp_str2;
    bool ret_value;

    token.NextToken();
    tmp_str2 = token.NextToken();

    m_http_response = wxAtoi(tmp_str2);

    switch (tmp_str2[0u].GetValue()) {
    case wxT('1'):
        /* INFORMATION / SUCCESS */
        break;

    case wxT('2'):
        /* SUCCESS */
        break;

    case wxT('3'):
        /* REDIRECTION */
        break;

    default:
        m_lastError = wxPROTO_NOFILE;
        RestoreState();
        return false;
    }

    m_lastError = wxPROTO_NOERR;
    ret_value = ParseHeaders();

    return ret_value;
}
#pragma endregion

#pragma region Private Functions
bool SanDevices::SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SanDevices Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    progress.Update(0, "Scanning models");
    logger_base.info("Scanning models.");

    std::string check;
    UDController cud(controller, outputManager, allmodels, false);

    //first check rules
    auto rules = ControllerCaps::GetControllerConfig(controller);
    bool success = cud.Check(rules, check);

    logger_base.debug(check);

    cud.Dump();

    if (!success) {
        DisplayError("SanDevices Upload Error:\n" + check, parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    //get current config Page
    const std::string page = _page;

    if (page.empty()) return false;

    progress.Update(30, "Reading Protocol Data from Controller.");
    logger_base.info("Reading Protocol Data from Controller.");

    _connected = ParseV4Webpage(_page);
    if (!_connected) {
        DisplayError("Unable to Parse Webpage.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    progress.Update(50, "Figuring Out Protocol and Output Information.");
    logger_base.info("Figuring Out Protocol and Output Information.");
    const int totalOutputGroups = GetNumberOfOutputGroups();
    const int outputPerGroups = GetOutputsPerGroup();

    //loop to setup protocol setting and string outputs
    for (int i = 1; i <= totalOutputGroups; i++) {
        bool first = true;
        SanDevicesOutputV4* newPort = FindPortDataV4(i);
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerPixelPort(outputNumber);
                if (first) {
                    UpdatePortDataV4(newPort, port);
                    first = false;
                }

                const char protocol = EncodeStringPortProtocolV4(port->GetProtocol());

                if (newPort->protocol != protocol) {
                    logger_base.warn("SanDevices Outputs Upload: All The Protocols must be the same across a Output Group. Check Port %d-1 to %d-4", i, i);
                    wxMessageBox(wxString::Format("All The Protocols must be the same across a Output Group. Check Port %d-1 to %d-4", i, i));
                    return false;
                }
                if (newPort->pixels != port->Pixels()) {
                    logger_base.warn("SanDevices Outputs Upload: All The Pixel Lengths must be the same across a Output Group. Check Port %d-1 to %d-4", i, i);
                    wxMessageBox(wxString::Format("All The Pixel Lengths must be the same across a Output Group. Check Port %d-1 to %d-4", i, i));
                    return false;
                }
                UpdateSubPortDataV4(newPort, j, port);
            }
        }
        if (first) {
            newPort->outputSize = 0;
            newPort->upload = true;
        }
    }
    int p = 60;
    logger_base.info("Sending Output Data to Controller.");
    //spam the controller with web requests
    for (const auto& outputD : _outputDataV4) {
        progress.Update(p, "Sending Output Data to Controller.");
        outputD->Dump();
        if (outputD->upload) {
            const std::string url = GenerateOutputURLV4(outputD);
            SDGetURL(url);
            wxMilliSleep(2000);
        }
        p += 10;
    }
    return true;
}

bool SanDevices::SetOutputsV5(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {

    wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    //bool success = true;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SanDevices Outputs Upload: Uploading to %s", (const char*)_ip.c_str());

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

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
        DisplayError("SanDevices Upload Error:\n" + check, parent);
        progress.Update(100, "Aborting.");
        return false;
    }
    //get current config Page
    const std::string page = _page;

    if (page.empty()) return false;

    progress.Update(10, "Reading Protocol Data from Controller.");
    logger_base.info("Reading Protocol Data from Controller.");

    _connected = ParseV5MainWebpage(_page);
    if (!_connected) {
        DisplayError("Unable to Parse Main Webpage.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    const int totalOutputGroups = GetNumberOfOutputGroups();
    const int outputPerGroups = GetOutputsPerGroup();

    progress.Update(20, "Figuring Out Protocol Information.");
    logger_base.info("Figuring Out Protocol Information.");
    //loop to setup protocol setting
    for (int i = 1; i <= totalOutputGroups; i++) {
        SanDevicesProtocol* firstPort = nullptr;
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerPixelPort(outputNumber);
                if (firstPort == nullptr) {
                    firstPort = new SanDevicesProtocol(i, EncodeStringPortProtocolV5(port->GetProtocol()));
                }
                SanDevicesProtocol* newPort = new SanDevicesProtocol(i, EncodeStringPortProtocolV5(port->GetProtocol()));
                if (newPort->getProtocol() != firstPort->getProtocol()) {
                    delete newPort;
                    logger_base.warn("SanDevices Outputs Upload: All The Protocols must be the same across a Output Group. Check Port %d-1 to %d-4", i, i);
                    wxMessageBox(wxString::Format("All The Protocols must be the same across a Output Group. Check Port %d-1 to %d-4", i, i));
                    return false;
                }
                delete newPort;
                if (_protocolData[i - 1]->getProtocol() != firstPort->getProtocol()) {
                    _protocolData[i - 1]->setProtocol(firstPort->getProtocol());
                }
            }
        }
    }

    progress.Update(30, "Sending Protocol Data to Controller.");
    logger_base.info("Sending Protocol Data to Controller.");

    for (const auto& proro : _protocolData) {
        proro->Dump();
        if (proro->shouldUpload()) {
            const std::string url = GenerateProtocolURLV5(proro);
            SDGetURL(url);
            wxMilliSleep(3000);
        }
    }

    progress.Update(50, "Getting Output Data from Controller.");
    logger_base.info("Getting Output Data from Controller.");

    const std::string page2 = SDGetURL("/H?");
    wxMilliSleep(3000);

    if (page2.empty()) {
        logger_base.error("SanDevices Outputs Upload: SanDevices would not return current configuration.");
        wxMessageBox("Error occurred trying to upload to SanDevices.", "Error", wxOK, parent);
        return false;
    }

    _connected = ParseV5OutputWebpage(page2);
    if (!_connected) {
        DisplayError("Unable to Parse Main Webpage.", parent);
        progress.Update(100, "Aborting.");
        return false;
    }

    progress.Update(60, "Figuring Out Output Information.");
    logger_base.info("Figuring Out Output Information.");
    //loop to setup string outputs
    for (int i = 1; i <= totalOutputGroups; i++) {
        for (int j = 1; j <= outputPerGroups; j++) {
            const int outputNumber = EncodeXlightsOutput(i, j);
            if (cud.HasPixelPort(outputNumber)) {
                UDControllerPort* port = cud.GetControllerPixelPort(outputNumber);
                UpdatePortDataV5(i, j, port);
            }
            else {
                SanDevicesOutput* sd = FindPortDataV5(i, j);
                if (sd != nullptr) {
                    sd->pixels = 0;
                    sd->upload = true;
                }
            }
        }
    }
    logger_base.info("Sending Output Data to Controller.");
    int p = 66;
    //spam the controller with web requests
    for (const auto& outputD : _outputData) {
        p += 2;
        progress.Update(p, "Sending Output Data to Controller.");
        outputD->Dump();
        if (outputD->upload) {
            const std::string url = GenerateOutputURLV5(outputD);
            SDGetURL(url);
            wxMilliSleep(3000);
        }
    }
    return true;
}

bool SanDevices::ParseV4Webpage(const std::string& page) {

    //1-1 thru 1-4
    _outputDataV4.clear();
    _universes.clear();

    int fieldStart = 65;//char of 'A'
    const wxString p(page);
    const int start = p.find(" Universe");
    for (int i = 0; i < 12; i++) {
        // extract the universes
        const int univers = ExtractIntFromPage(page, wxString::Format("%c", fieldStart++), "input", 1, start);
        _universes.push_back(univers);
    }

    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        SanDevicesOutputV4* group = ExtractOutputDataV4(page, i);
        group->Dump();
        _outputDataV4.push_back(group);
    }
    if (_outputDataV4.size() == 4 && _universes.size() == 12) return true;
    wxASSERT(false);
    return true;
}

bool SanDevices::ParseV5MainWebpage(const std::string& page) {

    //1-1 thru 1-4
    _protocolData.clear();
    _universes.clear();

    int fieldStart = 65;//char of 'A'
    const wxString p(page);
    const int start = p.find(" Universe");
    for (int i = 0; i < 12; i++) {
        // extract the universes
        const int univers = ExtractIntFromPage(page, wxString::Format("%c", fieldStart++), "input", 1, start);
        _universes.push_back(univers);
    }

    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        SanDevicesProtocol* group = ExtractProtocalDataV5(page, i);
        group->Dump();
        _protocolData.push_back(group);
    }
    if (_protocolData.size() == 4 && _universes.size() == 12) return true;
    wxASSERT(false);
    return true;
}

bool SanDevices::ParseV5OutputWebpage(const std::string& page) {

    _outputData.clear();
    for (int i = 1; i <= GetNumberOfOutputGroups(); i++) {
        for (int j = 1; j <= GetOutputsPerGroup(); j++) {
            SanDevicesOutput* group = ExtractOutputDataV5(page, i, j);
            group->Dump();
            _outputData.push_back(group);
        }
    }
    if (_outputData.size() == GetMaxStringOutputs()) return true;
    wxASSERT(false);
    return true;
}

const std::string DecodeProtocolError(wxProtocolError err)
{
    switch (err) {
    case wxPROTO_NOERR:
        return "Success";
    case wxPROTO_NETERR:
        return "Network Error";
    case wxPROTO_PROTERR:
        return "Protocol Error";
    case wxPROTO_CONNERR:
        return "Connection Error";
    case wxPROTO_INVVAL:
        return "Invalid Value";
    case wxPROTO_NOHNDLR:
        return "No Handler";
    case wxPROTO_NOFILE:
        return "No File";
    case wxPROTO_ABRT:
        return "Abort";
    case wxPROTO_RCNCT:
        return "Reconnection Count";
    case wxPROTO_STREAMING:
        return "Streaming";
    }
    return "Unrecognised error";
}

std::string SanDevices::SDGetURL(const std::string& url, bool logresult) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;

    _http.SetMethod("GET");
    wxString startResult;
    wxInputStream* httpStream = _http.GetInputStream(_baseUrl + url, startResult);
    logger_base.debug("Making request to SanDevices %s '%s' -> %d", (const char*)_ip.c_str(), (const char*)url.c_str(), _http.GetResponse());

    if (_http.GetError() == wxPROTO_NOERR) {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult) {
            logger_base.debug("Response from SanDevices '%s'.", (const char*)res.c_str());
        }
    }
    else {
        logger_base.error("Unable to connect to SanDevices %s '%s' : %d %s.", (const char*)_ip.c_str(), (const char*)url.c_str(), _http.GetError(), (const char*)DecodeProtocolError(_http.GetError()).c_str());
        wxMessageBox(_T("Unable to connect!"));
        res = "";
    }

    return (startResult + res).ToStdString();
}

void SanDevices::ResetStringOutputs() {

    SDGetURL("/4?A=0");
    SDGetURL("/5?A=0");
    SDGetURL("/6?A=0");
    SDGetURL("/7?A=0");
}

inline int SanDevices::GetOutputsPerGroup() const {

    if (IsE682()) return 4;
    return 1;
}

std::string SanDevices::ExtractFromPage(const std::string& page, const std::string& parameter, const std::string& type, int start) {

    const wxString p = wxString(page).Mid(start);
    if (type == "input") {
        const wxString regex = "(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')([0-9\\.]*?)\\'";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p))) {
            const std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            const std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "inputText") {
        const wxString regex = "(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')(\\w+)(\\s+)?\\'";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p))) {
            const std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            const std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "select") {
        const wxString regex = "(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z0-9])\\'selected";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p))) {
            const std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            const std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "selectLetter") {
        const wxString regex = "(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z])\\'selected";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p))) {
            const std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            const std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "checkbox") {
        const wxString regex = "(\\<input type=\\'checkbox\\' name=\\'" + parameter + "\\' value=\\'[0-9]\\')([^\\>]*)\\>";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p))) {
            const std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            const std::string res = inputregex.GetMatch(wxString(p), 2).Trim().Trim(false).ToStdString();
            if (res == "checked") {
                return "1";
            }
            return "0";
        }
    }

    return "";
}

int SanDevices::ExtractIntFromPage(const std::string& page, const std::string& parameter, const std::string& type, int defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return wxAtoi(value);
    }
    return defaultValue;
}

char SanDevices::ExtractCharFromPage(const std::string& page, const std::string& parameter, const std::string& type, char defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return value[0];
    }
    return defaultValue;
}

bool SanDevices::ExtractBoolFromPage(const std::string& page, const std::string& parameter, const std::string& type, bool defaultValue, int start) {

    const std::string value = ExtractFromPage(page, parameter, type, start);
    if (!value.empty()) {
        return value == "1";
    }
    return defaultValue;
}

SanDevicesProtocol* SanDevices::ExtractProtocalDataV5(const std::string& page, int group) {

    const wxString p(page);
    int start = p.find("Output Group Configuration:");

    const std::string tofind = "<td>" + wxString::Format("%i-1 thru %i-4", group, group) + "</td>";

    start = p.find(tofind, start);

    // extract the Protocol
    const char proto = ExtractCharFromPage(page, "E", "select", 'A', start);

    // extract the timing
    const char timing = ExtractCharFromPage(page, "K", "select", 'A', start);

    return new SanDevicesProtocol(group, proto, timing);
}

SanDevicesOutput* SanDevices::ExtractOutputDataV5(const std::string& page, int group, int port) {

    const wxString p(page);
    std::string tofind;

    if (IsE682()) {
        tofind = "<td>" + wxString::Format("%i-%i", group, port) + "</td>";
    }
    else {
        tofind = "<td>" + wxString::Format("%i", group) + "</td>";
    }
    const int start = p.find(tofind);

    SanDevicesOutput* output = new SanDevicesOutput(group, port, EncodeXlightsOutput(group, port));

    // extract the pixel
    output->pixels = ExtractIntFromPage(page, "A", "input", 0, start);

    output->colorOrder = ExtractCharFromPage(page, "E", "select", 'A', start);
    output->universe = ExtractCharFromPage(page, "Z", "select", 'A', start);

    output->startChannel = ExtractIntFromPage(page, "G", "input", 1, start);

    output->reverse = ExtractBoolFromPage(page, "N", "checkbox", false, start);
    output->nullPixel = ExtractIntFromPage(page, "H", "input", 0, start);

    output->groupCount = ExtractIntFromPage(page, "B", "input", 1, start);

    output->chase = ExtractBoolFromPage(page, "F", "checkbox", false, start);
    output->firstZig = ExtractIntFromPage(page, "I", "input", 0, start);
    output->thenEvery = ExtractIntFromPage(page, "J", "input", 0, start);

    output->brightness = ExtractCharFromPage(page, "D", "selectLetter", 'A', start);

    return output;
}

SanDevicesOutputV4* SanDevices::ExtractOutputDataV4(const std::string& page, int group) {

    const wxString p(page);
    int start = p.find("Output Configuration:");

    std::string tofind;

    if (IsE682()) {
        tofind = "<td>" + wxString::Format("%i-1 to %i-4", group, group) + "</td>";
    }
    else {
        tofind = "<td>" + wxString::Format("%i", group) + "</td>";
    }
    start = p.find(tofind, start);

    SanDevicesOutputV4* output = new SanDevicesOutputV4(group);

    if (IsE682()) {
        output->outputSize = ExtractIntFromPage(page, "A", "input", 0, start);
    }
    else {
        output->outputSize = ExtractIntFromPage(page, "A", "checkbox", 0, start);
    }

    output->protocol = ExtractCharFromPage(page, "B", "select", 'A', start);

    // extract the pixel
    output->pixels = ExtractIntFromPage(page, "C", "input", 0, start);
    output->groupCount = ExtractIntFromPage(page, "D", "input", 1, start);

    output->colorOrder = ExtractIntFromPage(page, "E", "select", 0, start);
    output->universe = ExtractCharFromPage(page, "F", "select", 'A', start);

    output->startChannel = ExtractIntFromPage(page, "G", "input", 1, start);
    if (IsE682()) {
        output->reverse[0] = ExtractBoolFromPage(page, "H", "checkbox", false, start);
        output->reverse[1] = ExtractBoolFromPage(page, "I", "checkbox", false, start);
        output->reverse[2] = ExtractBoolFromPage(page, "J", "checkbox", false, start);
        output->reverse[3] = ExtractBoolFromPage(page, "K", "checkbox", false, start);
    }
    else {
        output->reverse[0] = ExtractIntFromPage(page, "H", "checkbox", 0, start);
    }
    output->zigzag = ExtractIntFromPage(page, "L", "input", 0, start);
    if (IsE682()) {
        output->nullPixel[0] = ExtractIntFromPage(page, "M", "input", 0, start);
        output->nullPixel[1] = ExtractIntFromPage(page, "N", "input", 0, start);
        output->nullPixel[2] = ExtractIntFromPage(page, "O", "input", 0, start);
        output->nullPixel[3] = ExtractIntFromPage(page, "P", "input", 0, start);
    }
    else {
        output->nullPixel[0] = ExtractIntFromPage(page, "M", "checkbox", 0, start);
    }

    return output;
}

void SanDevices::UpdatePortDataV5(int group, int output, UDControllerPort* stringData) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    SanDevicesOutput* sd = FindPortDataV5(group, output);
    if (sd != nullptr) {
        if (stringData->GetPort() != sd->stringport) {
            logger_base.warn("SanDevices Outputs Upload: Error %d outputs. Attempt to upload to output %d.", stringData->GetPort(), sd->stringport);
        }
        const char newUniv = EncodeUniverse(stringData->GetUniverse());
        if (newUniv != sd->universe)
            sd->upload = true;
        sd->universe = newUniv;

        const int startChan = stringData->GetUniverseStartChannel();
        if (startChan != sd->startChannel)
            sd->upload = true;
        sd->startChannel = startChan;

        const int pixels = stringData->Pixels();
        if (pixels != sd->pixels)
            sd->upload = true;
        sd->pixels = pixels;

        const int nullPix = stringData->GetFirstModel()->GetStartNullPixels(-1);
        if (nullPix != -1) {
            if (nullPix != sd->nullPixel)
                sd->upload = true;
            sd->nullPixel = nullPix;
        }
        const int groupCount = stringData->GetFirstModel()->GetGroupCount(-1);
        if (groupCount != -1) {
            if (groupCount != sd->groupCount)
                sd->upload = true;
            sd->groupCount = groupCount;
        }
        const int brightness = stringData->GetFirstModel()->GetBrightness(-1);
        if (brightness != -1) {
            const char brightn = EncodeBrightness(brightness);

            sd->upload = true;
            sd->brightness = brightn;
        }

        const std::string colorOrder = stringData->GetFirstModel()->GetColourOrder("unknown");
        if (colorOrder != "unknown") {
            const char co = EncodeColorOrderV5(colorOrder);
            if (co != sd->colorOrder)
                sd->upload = true;
            sd->colorOrder = co;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown") {
            const bool rev = EncodeDirection(direction);
            if (rev != sd->reverse)
                sd->upload = true;
            sd->reverse = rev;
        }
    }
}

SanDevicesOutput* SanDevices::FindPortDataV5(int group, int output) {
    for (const auto& sd : _outputData) {
        if (sd->group == group && sd->output == output) {
            return sd;
        }
    }
    wxASSERT(false);
    return nullptr;
}

void SanDevices::UpdatePortDataV4(SanDevicesOutputV4* pd, UDControllerPort* stringData) const {

    if (pd != nullptr) {
        const char protocol = EncodeStringPortProtocolV4(stringData->GetProtocol());
        if (protocol != pd->protocol)
            pd->upload = true;
        pd->protocol = protocol;

        const char newUniv = EncodeUniverse(stringData->GetUniverse());
        if (newUniv != pd->universe)
            pd->upload = true;
        pd->universe = newUniv;

        const int startChan = stringData->GetUniverseStartChannel();
        if (startChan != pd->startChannel)
            pd->upload = true;
        pd->startChannel = startChan;

        const int pixels = stringData->Pixels();
        if (pixels != pd->pixels)
            pd->upload = true;
        pd->pixels = pixels;

        const int groupCount = stringData->GetFirstModel()->GetGroupCount(-1);
        if (groupCount != -1) {
            if (groupCount != pd->groupCount)
                pd->upload = true;
            pd->groupCount = groupCount;
        }

        const std::string colorOrder = stringData->GetFirstModel()->GetColourOrder("unknown");
        if (colorOrder != "unknown") {
            const char co = EncodeColorOrderV4(colorOrder);
            if (co != pd->colorOrder)
                pd->upload = true;
            pd->colorOrder = co;
        }
    }
}

void SanDevices::UpdateSubPortDataV4(SanDevicesOutputV4* pd, int subport, UDControllerPort* stringData) const {

    if (pd != nullptr) {
        if (subport != pd->outputSize) pd->upload = true;
        pd->outputSize = subport;

        const int nullPix = stringData->GetFirstModel()->GetStartNullPixels(-1);
        if (nullPix != -1) {
            if (nullPix != pd->nullPixel[subport - 1]) pd->upload = true;
            pd->nullPixel[subport - 1] = nullPix;
        }

        const std::string direction = stringData->GetFirstModel()->GetDirection("unknown");
        if (direction != "unknown") {
            const bool rev = EncodeDirection(direction);
            if (rev != pd->reverse[subport - 1])
                pd->upload = true;
            pd->reverse[subport - 1] = rev;
        }
    }
}

SanDevicesOutputV4* SanDevices::FindPortDataV4(int group) {

    for (const auto& sd : _outputDataV4) {
        if (sd->group == group) {
            return sd;
        }
    }
    wxASSERT(false);
    return nullptr;
}

std::string SanDevices::GenerateOutputURLV5(SanDevicesOutput* outputData) {

    // extract reverse
    std::string rev;
    if (outputData->reverse) //if check add to request, based on my testing firmware will check if present regardless of value
        rev = "&N=1";

    // extract null pixels
    std::string null;
    if (outputData->nullPixel != 0)//Only Add to Request if it currently exists
        null = wxString::Format("&H=%i", outputData->nullPixel);

    // extract chase
    std::string chase;
    if (outputData->chase) //if check add to request, based on my testing firmware will check if present regardless of value
        chase = "&F=1";
    else
        chase = "";

    const int controlPort = EncodeControllerPortV5(outputData->group, outputData->output);

    //http://192.168.1.206/K?A=50&E=A&Z=A&G=1&H=0&B=1&I=0&J=0&D=A
    const wxString request = wxString::Format("/%c?A=%d&E=%c&Z=%c&G=%d%s%s&B=%i%s&I=%i&J=%i&D=%c",
        controlPort + 'J',
        outputData->pixels,
        outputData->colorOrder,
        outputData->universe,
        outputData->startChannel,
        rev,
        null,
        outputData->groupCount,
        chase,
        outputData->firstZig,
        outputData->thenEvery,
        outputData->brightness);
    return request;
}

std::string SanDevices::GenerateProtocolURLV5(SanDevicesProtocol* protocolData) {

    //K?E=A&K=A
    //http://192.168.1.206/K?A=50&E=A&Z=A&G=1&H=0&B=1&I=0&J=0&D=A
    const wxString request = wxString::Format("/%c?E=%c&K=%c",
        protocolData->getGroup() + 'J',
        protocolData->getProtocol(),
        protocolData->getTiming());
    return request;
}

std::string SanDevices::GenerateOutputURLV4(SanDevicesOutputV4* outputData) {

    std::string output;
    if (outputData->outputSize != 0) {
        output = wxString::Format("A=%d&", outputData->outputSize);
    }

    // extract reverse
    std::string rev;
    if (outputData->reverse[0]) rev += "&H=1";
    if (IsE682()) {
        if (outputData->reverse[1]) rev += "&I=1";
        if (outputData->reverse[2]) rev += "&J=1";
        if (outputData->reverse[3]) rev += "&K=1";
    }

    // extract null pixels
    std::string null;
    if (outputData->nullPixel[0]) {
        rev += "&M=1";
    }
    else {
        rev += "&M=0";
    }
    if (IsE682()) {
        if (outputData->nullPixel[1]) {
            rev += "&N=1";
        }
        else {
            rev += "&N=0";
        }
        if (outputData->nullPixel[2]) {
            rev += "&O=1";
        }
        else {
            rev += "&O=0";
        }
        if (outputData->nullPixel[3]) {
            rev += "&P=1";
        }
        else {
            rev += "&P=0";
        }
    }

    //e682 v4
    //http://192.168.1.206/4?A=2&B=B&C=100&D=1&E=0&F=A&G=1&L=0&M=0&N=0&O=0&P=0
    const wxString request = wxString::Format("/%d?%sB=%c&C=%d&D=%i&E=%i&F=%c&G=%d%s&L=%i%s",
        outputData->group + 3,
        output,
        outputData->protocol,
        outputData->pixels,
        outputData->groupCount,
        outputData->colorOrder,
        outputData->universe,
        outputData->startChannel,
        null,
        outputData->zigzag,
        rev);
    return request;
}
#pragma endregion

#pragma region Encode and Decode
char SanDevices::EncodeStringPortProtocolV4(const std::string& protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return 'D';
    if (p == "tm18xx") return 'D';
    if (p == "ws2801") return 'B';
    if (p == "tls3001") return 'I';
    if (p == "lpd6803") return 'A';
    if (p == "gece") return 'C';
    if (p == "dmx") return 'K';
    if (p == "renard") return 'L';
    wxASSERT(false);
    return 'D';
}

char SanDevices::EncodeStringPortProtocolV5(const std::string& protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "ws2811") return 'A';
    if (p == "tm18xx") return 'D';
    if (p == "ws2801") return 'C';
    if (p == "lpd6803") return 'B';
    if (p == "apa102") return 'G';
    if (p == "lpd8806") return 'E';
    if (p == "gece") return 'O';
    if (p == "dmx") return 'K';
    if (p == "renard") return 'N';
    wxASSERT(false);
    return 'A';
}

char SanDevices::EncodeSerialPortProtocolV5(const std::string& protocol) const {

    wxString p(protocol);
    p = p.Lower();

    if (p == "dmx") return 'K';
    if (p == "renard") return 'N';
    wxASSERT(false);
    return 'K';
}

char SanDevices::EncodeUniverseSize(int universesize) const {

    if (universesize == 510) return 'A';
    if (universesize == 512) return 'B';
    wxASSERT(false);
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SanDevices DecodeUniverseSize Upload: Invalid Universe Size %i", universesize);
    return 'A';
}

int SanDevices::EncodeColorOrderV4(const std::string& colorOrder) const {

    wxString c(colorOrder);
    c = c.Lower();

    if (c == "rgb") return 0;
    if (c == "rbg") return 1;
    if (c == "grb") return 2;
    if (c == "gbr") return 3;
    if (c == "brg") return 4;
    if (c == "bgr") return 5;
    wxASSERT(false);
    return 0;
}

char SanDevices::EncodeColorOrderV5(const std::string& colorOrder) const {

    wxString c(colorOrder);
    c = c.Lower();

    if (c == "rgb") return 'A';
    if (c == "rbg") return 'B';
    if (c == "grb") return 'C';
    if (c == "gbr") return 'D';
    if (c == "brg") return 'E';
    if (c == "bgr") return 'F';
    wxASSERT(false);
    return 'A';
}

char SanDevices::EncodeBrightness(int brightness) const {

    if (brightness < 19) return 'D';
    if (brightness < 38) return 'C';
    if (brightness < 75) return 'B';
    return 'A';
}

inline bool SanDevices::EncodeDirection(const std::string& direction) const {

    return direction == "Reverse";
}

char SanDevices::EncodeUniverse(int universe) const {

    char res = 'A';

    for (const auto& uni : _universes) {
        if (uni == universe) {
            return res;
        }
        ++res;
    }
    return res;
}

SanDevices::SanDeviceModel SanDevices::DecodeControllerType(const std::string& modelName) const {

    if (modelName == "E682")
        return SanDeviceModel::E682;
    if (modelName == "E6804")
        return SanDeviceModel::E6804;
    wxASSERT(false);
    return SanDeviceModel::Unknown;
}

inline std::pair<int, int > SanDevices::DecodeOutputPort(const int output) const {

    return { ((output - 1) / 4) + 1, ((output - 1) % 4) + 1 };
}

inline int SanDevices::EncodeXlightsOutput(const int group, const int subport) const {

    return ((group - 1) * GetOutputsPerGroup()) + subport;
}

inline int SanDevices::EncodeControllerPortV5(const int group, const int subport) const {

    return ((group - 1) * 4) + subport;
}
#pragma endregion

#pragma region Constructors and Destructors
SanDevices::SanDevices(const std::string& ip, const std::string& proxy) : BaseController(ip, proxy) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _http.SetMethod("GET");
    if (_fppProxy != "") {
        _connected = _http.Connect(_fppProxy);
    }
    else {
        _connected = _http.Connect(_ip);
    }
    _firmware = FirmwareVersion::Unknown;

    if (_connected) {
        //Loop For Version 5, we may have to switch web pages first and then scrap data
        for (int i = 0; i < 2; i++) {
            _page = SDGetURL("/");
            if (!_page.empty()) {
                static wxRegEx modelregex("(Controller Model )(E\\d+)", wxRE_ADVANCED | wxRE_NEWLINE);
                if (modelregex.Matches(wxString(_page))) {
                    _sdmodel = DecodeControllerType(modelregex.GetMatch(wxString(_page), 2).ToStdString());
                    logger_base.info("Connected to SanDevices controller model %s.", (const char*)GetModel().c_str());
                }
                static wxRegEx versionregex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>\\<\\/td\\>\\<td\\>)([0-9]+\\.[0-9]+)\\<\\/td\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (versionregex.Matches(wxString(_page))) {
                    _firmware = FirmwareVersion::Four;
                    _version = versionregex.GetMatch(wxString(_page), 2).ToStdString();
                    logger_base.info("                                 firmware %d.", static_cast<int>(_firmware));
                    logger_base.info("                                 version %s.", (const char*)_version.c_str());
                    break;
                }
                // Firmware Version:</th></td><td>5.038</td>
                // Firmware Version:</th></td><td> 5.051-W5200</td>
                static wxRegEx version5regex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>)\\s?([0-9]+\\.[0-9]+)(-W5200)?\\<\\/td\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (version5regex.Matches(wxString(_page))) {
                    _firmware = FirmwareVersion::Five;
                    _version = version5regex.GetMatch(wxString(_page), 2).ToStdString();
                    if (version5regex.GetMatchCount() > 3) {
                        _version += version5regex.GetMatch(wxString(_page), 3).ToStdString();
                    }
                    logger_base.info("                                 firmware %d.", static_cast<int>(_firmware));
                    logger_base.info("                                 version %s.", (const char*)_version.c_str());
                    break;
                }

                //Switch Pages from Version 5 Firmware
                SDGetURL("/H?");
            }
            else {
                _http.Close();
                _connected = false;
                logger_base.error("Error connecting to SanDevices controller on %s.", (const char*)_ip.c_str());
                break;
            }
        }
    }
    else {
        logger_base.error("Error connecting to SanDevices controller on %s.", (const char*)_ip.c_str());
    }

    if (_connected) {
        _model = wxString::Format("E%d", _sdmodel);
    }
}

SanDevices::~SanDevices() {

    _http.Close();

    for (const auto& it : _outputData) {
        delete it;
    }
    _outputData.clear();

    for (const auto& it : _protocolData) {
        delete it;
    }
    _protocolData.clear();

    for (const auto& it : _outputDataV4) {
        delete it;
    }
    _outputDataV4.clear();
}
#pragma endregion

#pragma region Getters and Setters
bool SanDevices::SetInputUniverses(Controller* controller, wxWindow* parent) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    const std::string page = _page;

    if (page.empty()) return false;

    //Check Correct Page from Version 5 Firmware
    if (IsFirmware5()) {
        if (page.find("<H3>System Information: </H3>") == std::string::npos) {
            logger_base.error("SanDevices Outputs Upload: SanDevices wouldn't switch web pages.");
            return false;
        }
    }

    // Get universes based on IP
    std::list<Output*> outputs = controller->GetOutputs();

    if (outputs.size() > 12) {
        DisplayError(wxString::Format("Attempt to upload %d universes to SanDevices controller but only 12 are supported.", outputs.size()).ToStdString());
        return false;
    }

    //dont make http requests if setting are already right
    bool upload = true;

    int t = -1;

    auto o = outputs.front();

    if (o->GetType() == OUTPUT_E131) {
        if (o->GetIP() == "MULTICAST") {
            t = 0;
        }
        else {
            t = 1;
        }
    }
    else if (o->GetType() == OUTPUT_ARTNET) {
        t = 2;
    }

    if ((t == 2 || t == 0) && outputs.size() > 7) {
        DisplayError(wxString::Format("Attempt to upload %d universes to SanDevices controller but only 7 are supported in Multicast/Artnet Mode.", outputs.size()).ToStdString());
        return false;
    }

    // set the right input type
    wxString request = "/";
    if (IsFirmware5()) {
        request += "B?";
    }
    else {
        request += "1?";
    }

    request += "A=" + ExtractFromPage(page, "A", "input") +
        "&B=" + ExtractFromPage(page, "B", "input") +
        "&C=" + ExtractFromPage(page, "C", "input") +
        "&D=" + ExtractFromPage(page, "D", "input");

    if (IsFirmware5()) {
        request += "&N=" + ExtractFromPage(page, "N", "input") +
            "&O=" + ExtractFromPage(page, "O", "input") +
            "&P=" + ExtractFromPage(page, "P", "input") +
            "&Q=" + ExtractFromPage(page, "Q", "input") +
            "&I=++++++++++++++++";
        //"&I=" + ExtractFromPage(page, "I", "inputText");
        //I=+++++++++++++++++
        t += 65; //convert int to char
        const wxString currentReceiveMode = ExtractFromPage(page, "E", "select");
        const wxString newReceiveMode = wxString::Format("%c", t);
        if (currentReceiveMode == newReceiveMode) {
            upload = false;
        }
        request += std::string(wxString::Format("&E=%c", t).c_str());
    }
    else {
        request += std::string(wxString::Format("&E=%i", t).c_str());
    }
    request += "&F=" + ExtractFromPage(page, "F", "input");

    if (IsFirmware5()) {
        request += "&H=" + ExtractFromPage(page, "H", "select");// += "&H=A"; //impossible to extract correctly because the are two fields called "H"
    }
    else {
        request += "&H=" + ExtractFromPage(page, "H", "input");
    }
    request += "&G=" + ExtractFromPage(page, "G", "input");

    if (IsFirmware5()) {
        request += "&J=" + ExtractFromPage(page, "J", "input") +
            "&K=" + ExtractFromPage(page, "K", "input");
    }

    if (upload) {
        SDGetURL(request.ToStdString());
        wxMilliSleep(1000);
    }
    upload = true;

    request = "";
    wxString requestUnvSize = "/I?";
    int output = 65;

    for (const auto& it : outputs)
    {
        if (!request.empty()) {
            request += "&";
            requestUnvSize += "&";
        }
        else {
            if (IsFirmware5()) {
                request = "/D?";
            }
            else {
                request = "/3?";
            }
        }
        if (IsFirmware5()) {
            if (it->GetChannels() != 510 && it->GetChannels() != 512) {
                DisplayError(wxString::Format("Attempt to upload a universe of size %d to SanDevices controller, but only a size of 510/512 is supported", it->GetChannels()).ToStdString());
                return false;
            }
            requestUnvSize += wxString::Format("%c=%c", output, EncodeUniverseSize(it->GetChannels()));
        }
        else {
            if (it->GetChannels() != 510) {
                DisplayError(wxString::Format("Attempt to upload a universe of size %d to SanDevices controller, but only a size of 510 is supported in Firmware 4.", it->GetChannels()).ToStdString());
                return false;
            }
        }
        request += wxString::Format("%c=%i", output++, it->GetUniverse());
    }

    if (0 == t) { //multicast
        if (output > 72) {
            logger_base.error("SanDevices Inputs Upload: More Than 7 Universes are assigned to One Controller, 7 is the MAX in Multicast Mode.");
            return false;
        }
    }
    else { //unicast or artnet
        if (output > 77) {
            logger_base.error("SanDevices Inputs Upload: More Than 12 Universes are assigned to One Controller, 12 is the MAX for Sandevices.");
            return false;
        }
    }

    if (IsFirmware5()) {
        SDGetURL(requestUnvSize.ToStdString());
        wxMilliSleep(1000);
    }
    bool passed = (SDGetURL(request.ToStdString()) != "");
    wxMilliSleep(1000);
    return passed;
}

bool SanDevices::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) {

    if (IsFirmware5()) {
        return SetOutputsV5(allmodels, outputManager, controller, parent);
    }

    if (IsFirmware4()) {
        return SetOutputsV4(allmodels, outputManager, controller, parent);
    }
    wxASSERT(false);
    return false;
}
#pragma endregion
