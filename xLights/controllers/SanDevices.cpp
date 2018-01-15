#include "SanDevices.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include "models/Model.h"
#include <log4cpp/Category.hh>
#include "outputs/OutputManager.h"
#include "outputs/Output.h"
#include "models/ModelManager.h"
#include <wx/sckstrm.h>
#include <wx/tokenzr.h>

class MyHTTPStream : public wxSocketInputStream
{
public:
    wxHTTP * m_http;
    size_t m_httpsize;
    unsigned long m_read_bytes;

    MyHTTPStream(wxHTTP *http) : wxSocketInputStream(*http)
    {
        m_http = http;
        m_httpsize = 0;
        m_read_bytes = 0;
    }

    size_t GetSize() const wxOVERRIDE { return m_httpsize; }
    virtual ~MyHTTPStream(void) { m_http->Abort(); }

protected:
    size_t OnSysRead(void *buffer, size_t bufsize) wxOVERRIDE;

    wxDECLARE_NO_COPY_CLASS(MyHTTPStream);
};

size_t MyHTTPStream::OnSysRead(void *buffer, size_t bufsize)
{
    if (m_read_bytes >= m_httpsize)
    {
        m_lasterror = wxSTREAM_EOF;
        return 0;
    }

    size_t ret = wxSocketInputStream::OnSysRead(buffer, bufsize);
    m_read_bytes += ret;

    if (m_httpsize == (size_t)-1 && m_lasterror == wxSTREAM_READ_ERROR)
    {
        // if m_httpsize is (size_t) -1 this means read until connection closed
        // which is equivalent to getting a READ_ERROR, for clients however this
        // must be translated into EOF, as it is the expected way of signalling
        // end end of the content
        m_lasterror = wxSTREAM_EOF;
    }

    return ret;
}

wxInputStream *SimpleHTTP::GetInputStream(const wxString& path, wxString& startResult)
{
    MyHTTPStream *inp_stream;

    wxString new_path;

    m_lastError = wxPROTO_CONNERR;  // all following returns share this type of error
    if (!m_addr)
        return nullptr;

    // We set m_connected back to false so wxSocketBase will know what to do.
#ifdef __WXMAC__
    wxSocketClient::Connect(*m_addr, false);
    wxSocketClient::WaitOnConnect(10);

    if (!wxSocketClient::IsConnected())
        return nullptr;
#else
    if (!wxProtocol::Connect(*m_addr))
        return nullptr;
#endif

    // Use the user-specified method if any or determine the method to use
    // automatically depending on whether we have anything to post or not.
    wxString method = m_method;
    if (method.empty())
        method = m_postBuffer.IsEmpty() ? wxS("GET") : wxS("POST");

    if (!MyBuildRequest(path, method, startResult))
        return nullptr;

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

bool SimpleHTTP::MyBuildRequest(const wxString& path, const wxString& method, wxString& startResult)
{
    startResult = "";

    // Use the data in the post buffer, if any.
    if (!m_postBuffer.IsEmpty())
    {
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
    while (tmp_str == "" && i++ < 10)
    {
        m_lastError = ReadLine(this, tmp_str);

        // return only if error and no data
        if (m_lastError != wxPROTO_NOERR && tmp_str == "")
            return false;

        wxMilliSleep(10);
    }
    m_lastError = wxPROTO_NOERR;

    if (!tmp_str.Contains(wxT("HTTP/"))) {
        m_lastError = wxPROTO_NOERR;
        SetHeader(wxT("Content-Length"), wxString::Format("%i", (long)tmp_str.Length()));
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

    switch (tmp_str2[0u].GetValue())
    {
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

SanDevices::SanDevices(const std::string& ip)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ip = ip;

    _http.SetMethod("GET");
    _connected = _http.Connect(_ip);
    _eVersion = FirmwareVersion::Unknown;

    if (_connected)
    {
        //Loop For Version 5, we may have to switch web pages first and then scrap data
        for (int i = 0; i < 2; i++)
        {
            std::string page = GetURL("/");
            if (page != "")
            {
                static wxRegEx modelregex("(Controller Model )([^ ]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                if (modelregex.Matches(wxString(page)))
                {
                    _model = modelregex.GetMatch(wxString(page), 2).ToStdString();
                    logger_base.error("Connected to SanDevices controller model %s.", (const char *)_model.c_str());
                }
                static wxRegEx versionregex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>\\<\\/td\\>\\<td\\>)([0-9]+\\.[0-9]+)\\<\\/td\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (versionregex.Matches(wxString(page)))
                {
                    _version = versionregex.GetMatch(wxString(page), 2).ToStdString();
                    logger_base.error("                                 version %s.", (const char *)_version.c_str());
                    _eVersion = FirmwareVersion::Four;
                    break;
                }
                static wxRegEx version5regex("(Firmware Version:\\<\\/th\\>\\<\\/td\\>\\<td\\>)([0-9]+\\.[0-9]+)\\<\\/td\\>", wxRE_ADVANCED | wxRE_NEWLINE);
                if (version5regex.Matches(wxString(page)))
                {
                    _version = version5regex.GetMatch(wxString(page), 2).ToStdString();
                    logger_base.error("                                 version %s.", (const char *)_version.c_str());
                    _eVersion = FirmwareVersion::Five;
                    break;
                }

                //Switch Pages from Version 5 Firmware
                GetURL("/H?");
            }
            else
            {
                _http.Close();
                _connected = false;
                logger_base.error("Error connecting to SanDevices controller on %s.", (const char *)_ip.c_str());
                break;
            }
        }
    }
    else
    {
        logger_base.error("Error connecting to SanDevices controller on %s.", (const char *)_ip.c_str());
    }
}

int SanDevices::GetOutputsPerPort() const
{
    if (FirmwareVersion::Four == _eVersion && _model == "E682")
    {
        return 4;
    }

    return 1;
}

int SanDevices::GetMaxStringOutputs() const
{
    if (FirmwareVersion::Five == _eVersion && _model == "E682")
    {
        return 16;
    }
    return 4;
}

int SanDevices::GetMaxSerialOutputs() const
{
    return 4;
}

SanDevices::~SanDevices()
{
    _http.Close();
}

std::string SanDevices::GetURL(const std::string& url, bool logresult)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;

    _http.SetMethod("GET");
    wxString startResult;
    wxInputStream *httpStream = _http.GetInputStream(wxString(url), startResult);
    logger_base.debug("Making request to SanDevices %s '%s' -> %d", (const char *)_ip.c_str(), (const char *)url.c_str(), _http.GetResponse());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        if (logresult)
        {
            logger_base.debug("Response from SanDevices '%s'.", (const char *)res.c_str());
        }
    }
    else
    {
        logger_base.error("Unable to connect to SanDevices %s '%s' : %d.", (const char *)_ip.c_str(), (const char *)url.c_str(), _http.GetError());
        wxMessageBox(_T("Unable to connect!"));
        res = "";
    }

    return (startResult + res).ToStdString();
}

char SanDevices::EncodeUniverse(int universe, OutputManager* outputManager, std::list<int>& selected)
{
    char res = 'A';

    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);
    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->GetUniverse() == universe)
        {
            return res;
        }
        ++res;
    }

    return res;
}

bool SanDevices::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string page = GetURL("/");

    if (page == "") return false;

    //Check Correct Page from Version 5 Firmware
    if (FirmwareVersion::Five == _eVersion)
    {
        if (page.find("<H3>System Information: </H3>") == std::string::npos)
        {
            logger_base.error("SanDevices Outputs Upload: SanDevices wouldn't switch web pages.");
            return false;
        }
    }

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    int t = -1;

    auto o = outputs.front();

    if (o->GetType() == "E131")
    {
        if (o->GetIP() == "MULTICAST")
        {
            t = 0;
        }
        else
        {
            t = 1;
        }
    }
    else if (o->GetType() == "ArtNet")
    {
        t = 2;
    }

    // set the right input type
    wxString request = "/";
    if (FirmwareVersion::Five == _eVersion)
        request += "B?";
    else
        request += "1?";
    request += "A=" + ExtractFromPage(page, "A", "input") +
        "&B=" + ExtractFromPage(page, "B", "input") +
        "&C=" + ExtractFromPage(page, "C", "input") +
        "&D=" + ExtractFromPage(page, "D", "input");
    if (FirmwareVersion::Five == _eVersion)
    {
        request += "&N=" + ExtractFromPage(page, "N", "input") +
            "&O=" + ExtractFromPage(page, "O", "input") +
            "&P=" + ExtractFromPage(page, "P", "input") +
            "&Q=" + ExtractFromPage(page, "Q", "input") +
            "&I=" + ExtractFromPage(page, "I", "inputText");
        t += 65; //convert int to char
        request += std::string(wxString::Format("&E=%c", t).c_str());
    }
    else
        request += std::string(wxString::Format("&E=%i", t).c_str());
    request += "&F=" + ExtractFromPage(page, "F", "input");
    if (FirmwareVersion::Five == _eVersion)
        request += "&H=" + ExtractFromPage(page, "H", "select");// += "&H=A"; //impossible to extract correctly because the are two fields called "H"
    else
        request += "&H=" + ExtractFromPage(page, "H", "input");
    request += "&G=" + ExtractFromPage(page, "G", "input");

    if (FirmwareVersion::Five == _eVersion)
    {
        request += "&J=" + ExtractFromPage(page, "J", "input") +
            "&K=" + ExtractFromPage(page, "K", "input");
    }

    GetURL(request.ToStdString());

    request = "";
    wxString requestUnvSize = "/I?";
    int output = 65;

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if (request != "")
        {
            request += "&";
            requestUnvSize += "&";
        }
        else
        {
            if (FirmwareVersion::Five == _eVersion)
                request = "/D?";
            else
                request = "/3?";
        }
        requestUnvSize += wxString::Format("%c=%c", output, DecodeUniverseSize((*it)->GetChannels()));
        request += wxString::Format("%c=%i", output++, (*it)->GetUniverse());
    }

    if (0 == t)//multicast
    {
        if (output > 71)
        {
            logger_base.error("SanDevices Inputs Upload: More Than 6 Universes are assigned to One Controller, 6 is the MAX in Multicast Mode.");
            return false;
        }
    }
    else//unicast or artnet
    {
        if (output > 77)
        {
            logger_base.error("SanDevices Inputs Upload: More Than 12 Universes are assigned to One Controller, 12 is the MAX for Sandevices.");
            return false;
        }
    }

    if (FirmwareVersion::Five == _eVersion)
        GetURL(requestUnvSize.ToStdString());

    return (GetURL(request.ToStdString()) != "");
}

std::string SanDevices::ExtractFromPage(const std::string page, const std::string parameter, const std::string type, int start)
{
    wxString p = wxString(page).Mid(start);

    if (type == "input")
    {
        wxString regex = "(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')([0-9\\.]*?)\\'";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    if (type == "inputText")
    {
        wxString regex = "(\\<input name=\\'" + parameter + "\\'[^\\>]*value=\\')(\\w+)(\\s+)?\\'";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "select")
    {
        wxString regex = "(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z0-9])\\'selected";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "selectLetter")
    {
        wxString regex = "(\\<select name=\\'" + parameter + "\\'\\>.*?\\')([A-Z])\\'selected";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            std::string res = inputregex.GetMatch(wxString(p), 2).ToStdString();
            return res;
        }
    }
    else if (type == "checkbox")
    {
        wxString regex = "(\\<input type=\\'checkbox\\' name=\\'" + parameter + "\\' value=\\'[0-9]\\')([^\\>]*)\\>";
        wxRegEx inputregex(regex, wxRE_ADVANCED | wxRE_NEWLINE);
        if (inputregex.Matches(wxString(p)))
        {
            std::string a = inputregex.GetMatch(wxString(p), 1).ToStdString();
            std::string res = inputregex.GetMatch(wxString(p), 2).Trim().Trim(false).ToStdString();
            if (res == "checked")
            {
                return "1";
            }
            return "0";
        }
    }

    return "";
}

bool SanDevicescompare_startchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

bool SanDevices::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
    //ResetStringOutputs(); // this shouldnt be used normally

    bool success = true;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SanDevices Outputs Upload: Uploading to %s", (const char *)_ip.c_str());
    // build a list of models on this controller
    std::list<Model*> models;
    std::list<std::string> protocolsused;
    std::list<Model*> warnedmodels;
    int maxport = 0;

    // Get universes based on IP
    std::list<Output*> outputs = outputManager->GetAllOutputs(_ip, selected);

    for (auto ito = outputs.begin(); ito != outputs.end(); ++ito)
    {
        // this universe is sent to the SanDevices

        // find all the models in this range
        for (auto it = allmodels->begin(); it != allmodels->end(); ++it)
        {
            if (it->second->GetDisplayAs() != "ModelGroup")
            {
                int modelstart = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
                int modelend = modelstart + it->second->GetChanCount() - 1;
                if ((modelstart >= (*ito)->GetStartChannel() && modelstart <= (*ito)->GetEndChannel()) ||
                    (modelend >= (*ito)->GetStartChannel() && modelend <= (*ito)->GetEndChannel()))
                {
                    //logger_base.debug("Model %s start %d end %d found on controller %s output %d start %d end %d.",
                    //    (const char *)it->first.c_str(), modelstart, modelend,
                    //    (const char *)_ip.c_str(), node, currentcontrollerstartchannel, currentcontrollerendchannel);
                    if (!it->second->IsControllerConnectionValid())
                    {
                        // only warn if we have not already warned
                        if (std::find(warnedmodels.begin(), warnedmodels.end(), it->second) == warnedmodels.end())
                        {
                            warnedmodels.push_back(it->second);
                            logger_base.warn("SanDevices Outputs Upload: Model %s on controller %s does not have its Controller Connection details completed: '%s'. Model ignored.", (const char *)it->first.c_str(), (const char *)_ip.c_str(), (const char *)it->second->GetControllerConnection().c_str());
                            wxMessageBox("Model " + it->first + " on controller " + _ip + " does not have its Contoller Connection details completed: '" + it->second->GetControllerConnection() + "'. Model ignored.", "Model Ignored");
                        }
                    }
                    else
                    {
                        // model uses channels in this universe

                        // check we dont already have this model in our list
                        if (std::find(models.begin(), models.end(), it->second) == models.end())
                        {
                            logger_base.debug("SanDevices Outputs Upload: Uploading Model %s.", (const char *)it->first.c_str());
                            models.push_back(it->second);
                            if (std::find(protocolsused.begin(), protocolsused.end(), it->second->GetProtocol()) == protocolsused.end())
                            {
                                protocolsused.push_back(it->second->GetProtocol());
                            }
                            if (it->second->GetPort() > maxport)
                            {
                                maxport = it->second->GetPort();
                            }
                        }
                    }
                }
            }
        }
    }


    // sort the models by start channel
    models.sort(SanDevicescompare_startchannel);

    //Switch Pages from Version 5 Firmware
    if (FirmwareVersion::Five == _eVersion)
        GetURL("/H?");

    // get the current config before I start
    std::string page = GetURL("/");
    if (page == "")
    {
        logger_base.error("SanDevices Outputs Upload: SanDevices would not return current configuration.");
        wxMessageBox("Error occured trying to upload to SanDevices.", "Error", wxOK, parent);
        return false;
    }

    //Check Correct Page from Version 5 Firmware
    if (FirmwareVersion::Five == _eVersion)
    {
        if (page.find("<H3>Output Configuration:</H3>") == std::string::npos)
        {
            logger_base.error("SanDevices Outputs Upload: SanDevices wouldn't switch web pages.");
            wxMessageBox("Error occured trying to upload to SanDevices.", "Error", wxOK, parent);
            return false;
        }
    }

    // for each protocol
    for (auto protocol = protocolsused.begin(); protocol != protocolsused.end(); ++protocol)
    {
        //std::string sendmessage;

        bool portdone[100];
        memset(&portdone, 0x00, sizeof(portdone)); // all false

        // for each port ... this is the max of any port type but it should be ok
        for (int i = 1; i <= maxport; i++)
        {
            // find the first and last
            Model* first = nullptr;
            Model* last = nullptr;
            int highestend = 0;
            long loweststart = 999999999;

            for (auto model = models.begin(); model != models.end(); ++model)
            {
                if ((*model)->GetProtocol() == *protocol && (*model)->GetPort() == i)
                {
                    int modelstart = (*model)->GetNumberFromChannelString((*model)->ModelStartChannel);
                    int modelend = modelstart + (*model)->GetChanCount() - 1;
                    if (modelstart < loweststart)
                    {
                        loweststart = modelstart;
                        first = *model;
                    }
                    if (modelend > highestend)
                    {
                        highestend = modelend;
                        last = *model;
                    }
                }
            }

            if (first != nullptr)
            {
                int portstart = first->GetNumberFromChannelString(first->ModelStartChannel);
                int portend = last->GetNumberFromChannelString(last->ModelStartChannel) + last->GetChanCount() - 1;
                int numstrings = first->GetNumPhysicalStrings();
                int outputsPerPort = GetOutputsPerPort();
                int outputsUsed = 1;
                int channelsperstring = first->NodesPerString() * first->GetChanCountPerNode();

                if (numstrings > 1 && outputsPerPort > 1)
                {
                    if (numstrings <= outputsPerPort)
                    {
                        outputsUsed = numstrings;
                        channelsperstring = numstrings * channelsperstring;
                        numstrings = 1;
                        portend = portstart + channelsperstring - 1;
                    }
                    else
                    {
                        int ons = numstrings;
                        numstrings = std::ceil(numstrings / outputsPerPort);
                        outputsUsed = ons / numstrings;
                        channelsperstring = outputsUsed * channelsperstring;
                    }
                }

                // upload it
                if (DecodeStringPortProtocol(*protocol) >= 0)
                {
                    if (first == last && numstrings > 1)
                    {
                        for (int j = 0; j < numstrings; j++)
                        {
                            int outputPort = i + j;
                            if (portdone[outputPort])
                            {
                                logger_base.warn("SanDevices Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), outputPort);
                                wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), outputPort));
                                success = false;
                            }
                            else
                            {
                                portdone[outputPort] = true;
                                //if (sendmessage != "") sendmessage = sendmessage + "&";
                                long startChannel;
                                Output* output = outputManager->GetOutput(portstart + j * channelsperstring, startChannel);
                                if (FirmwareVersion::Five == _eVersion)
                                    success = UploadStringPortFirmwareFive(page, outputPort, outputsUsed, DecodeStringPortProtocolFive(*protocol), startChannel, EncodeUniverse(output->GetUniverse(), outputManager, selected), channelsperstring / 3, first->GetName(), parent) && success;
                                else
                                    success = UploadStringPort(page, outputPort, outputsUsed, DecodeStringPortProtocol(*protocol), startChannel, EncodeUniverse(output->GetUniverse(), outputManager, selected), channelsperstring / 3, first->GetName(), parent) && success;
                            }
                        }
                    }
                    else
                    {
                        if (portdone[i])
                        {
                            logger_base.warn("SanDevices Outputs Upload: Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i);
                            wxMessageBox(wxString::Format("Attempt to upload model %s to string port %d but this string port already has a model on it.", (const char *)first->GetName().c_str(), i));
                            success = false;
                        }
                        else
                        {
                            portdone[i] = true;
                            long startChannel;
                            Output* output = outputManager->GetOutput(portstart, startChannel);
                            wxASSERT(channelsperstring == portend - portstart + 1);
                            if (FirmwareVersion::Five == _eVersion)
                                success = UploadStringPortFirmwareFive(page, i, outputsUsed, DecodeStringPortProtocolFive(*protocol), startChannel, EncodeUniverse(output->GetUniverse(), outputManager, selected), (portend - portstart + 1) / 3, first->GetName(), parent) && success;
                            else
                                success = UploadStringPort(page, i, outputsUsed, DecodeStringPortProtocol(*protocol), startChannel, EncodeUniverse(output->GetUniverse(), outputManager, selected), (portend - portstart + 1) / 3, first->GetName(), parent) && success;
                        }
                    }
                }
                else
                {
                    logger_base.warn("SanDevices Outputs Upload: Controller %s protocol %s not supported by this controller.",
                        (const char *)_ip.c_str(), (const char *)protocol->c_str());
                    wxMessageBox("Controller " + _ip + " protocol " + (*protocol) + " not supported by this controller.", "Protocol Ignored");
                    success = false;
                }
            }
            else
            {
                // nothing on this port ... ignore it
            }
        }

        // disable the other ports
        for (int i = 1; i <= GetMaxStringOutputs(); i++)
        {
            if (!portdone[i])
            {
                if (FirmwareVersion::Five == _eVersion)
                {
                    if (_model == "E682")
                        GetURL(wxString::Format("/%c?A=0", 'J' + i).ToStdString());
                    else
                        GetURL(wxString::Format("/%c?A=0", 'J' + (((i - 1) * 4) + 1)).ToStdString());//skip every four letters for E6804
                }
                else
                    GetURL(wxString::Format("/%i?A=0", i + 3).ToStdString());
            }
        }
    }
    return success;
}

char SanDevices::DecodeStringPortProtocol(std::string protocol)
{
    if (FirmwareVersion::Five == _eVersion)
        return DecodeStringPortProtocolFive(protocol);

    if (protocol == "ws2811") return 'D';
    if (protocol == "tm18xx") return 'D';
    if (protocol == "ws2801") return 'B';
    if (protocol == "tls3001") return 'I';
    if (protocol == "lpd6803") return 'A';
    if (protocol == "gece") return 'C';
    if (protocol == "dmx") return 'K';

    return -1;
}

char SanDevices::DecodeStringPortProtocolFive(std::string protocol)
{
    if (protocol == "ws2811") return 'A';
    if (protocol == "tm18xx") return 'D';
    if (protocol == "ws2801") return 'C';
    if (protocol == "lpd6803") return 'B';
    if (protocol == "gece") return 'O';
    if (protocol == "dmx") return 'K';
    if (protocol == "renard") return 'N';

    return -1;
}

char SanDevices::DecodeSerialPortProtocolFive(std::string protocol)
{
    if (protocol == "dmx") return 'K';
    if (protocol == "renard") return 'N';

    return -1;
}

char SanDevices::DecodeUniverseSize(int universesize)
{
    if (universesize == 510) return 'A';
    if (universesize == 512) return 'B';

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("SanDevices DecodeUniverseSize Upload: Invalid Universe Size %i", universesize);
    return 'A';
}

bool SanDevices::UploadStringPort(const std::string& page, int output, int outputsUsed, char protocol, int portstartchannel, char universe, int pixels, const std::string& description, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (output > GetMaxStringOutputs())
    {
        logger_base.warn("SanDevices Outputs Upload: SanDevices only supports %d outputs. Attempt to upload to output %d.", GetMaxStringOutputs(), output);
        wxMessageBox("SanDevices only supports " + wxString::Format("%d", GetMaxStringOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid String Output", wxOK, parent);
        return false;
    }

    wxString p(page);
    int start = p.find("Output Configuration");

    std::string tofind = "";

    if (_model == "E682")
    {
        tofind = "<td>" + wxString::Format("%i-1 to %i-4", output, output) + "</td>";
    }
    else
    {
        tofind = "<td>" + wxString::Format("%i", output) + "</td>";
    }
    start = p.find(tofind, start);

    // extract the group size
    std::string gs = ExtractFromPage(page, "D", "input", start);

    // extact the colour order
    std::string co = ExtractFromPage(page, "E", "select", start);

    // extract reverse
    std::string rev = ExtractFromPage(page, "H", "checkbox", start);

    // extract zig zag every
    std::string zz = ExtractFromPage(page, "L", "input", start);

    // extract null pixels
    std::string n = ExtractFromPage(page, "M", "input", start);

    std::string a = ExtractFromPage(page, "A", "checkbox", start);

    wxString request = wxString::Format("/%d?A=%d&B=%c&C=%d&D=%s&E=%s&F=%c&G=%d&H=%s&L=%s&M=%s",
        output + 3,
        outputsUsed,
        protocol,
        pixels,
        gs,
        co,
        universe,
        portstartchannel,
        rev,
        zz,
        n);
    return (GetURL(request.ToStdString()) != "");
}

bool SanDevices::UploadStringPortFirmwareFive(const std::string& page, int output, int outputsUsed, char protocol, int portstartchannel, char universe, int pixels, const std::string& description, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (output > GetMaxStringOutputs())
    {
        logger_base.warn("SanDevices Outputs Upload: SanDevices only supports %d outputs. Attempt to upload to output %d.", GetMaxStringOutputs(), output);
        wxMessageBox("SanDevices only supports " + wxString::Format("%d", GetMaxStringOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", "Invalid String Output", wxOK, parent);
        return false;
    }

    wxString p(page);
    int start = p.find("Output Configuration");

    std::string tofind = "";

    if (_model == "E682")
    {
        std::pair<int, int> ports = DecodeOutputPort(output);
        tofind = "<td>" + wxString::Format("%i-%i", ports.first, ports.second) + "</td>";
    }
    else
    {
        tofind = "<td>" + wxString::Format("%i", output) + "</td>";
        output = ((output - 1) * 4) + 1; //Skip every four letters for E6804
    }
    start = p.find(tofind, start);

    // extact the colour order
    std::string co = ExtractFromPage(page, "E", "select", start);
    if(co != "") //Only Add to Request if it currently exists
        co = wxString::Format("&E=%s",co);

    // extract reverse
    std::string rev = ExtractFromPage(page, "N", "checkbox", start);
    if (rev == "1") //if check add to request, based on my testing firmware will check if present reguardless of value
        rev = "&N=1";
    else
        rev = "";

    // extract null pixels
    std::string n = ExtractFromPage(page, "H", "input", start);
    if (n != "")//Only Add to Request if it currently exists
        n = wxString::Format("&H=%s", n);

    // extract the group size
    std::string gs = ExtractFromPage(page, "B", "input", start);

    // extract chase
    std::string chase = ExtractFromPage(page, "F", "checkbox", start);
    if (chase == "1") //if check add to request, based on my testing firmware will check if present reguardless of value
        chase = "&F=1";
    else
        chase = "";

    // extract first zig zag 
    std::string zz = ExtractFromPage(page, "I", "input", start);

    // extract then zig zag every
    std::string tzz = ExtractFromPage(page, "J", "input", start);

    // extact the intensatiy
    std::string intens = ExtractFromPage(page, "D", "select", start);

    //http://192.168.1.206/K?A=50&E=A&Z=A&G=1&H=0&B=1&I=0&J=0&D=A
    wxString request = wxString::Format("/%c?A=%d%s&Z=%c&G=%d%s%s&B=%s%s&I=%s&J=%s&D=%s",
        output + 'J',
        pixels,
        co,
        universe,
        portstartchannel,
        rev,
        n,
        gs,
        chase,
        zz,
        tzz,
        intens);
    return (GetURL(request.ToStdString()) != "");
}

void SanDevices::ResetStringOutputs()
{
    GetURL("/4?A=0");
    GetURL("/5?A=0");
    GetURL("/6?A=0");
    GetURL("/7?A=0");
}

std::pair<int, int > SanDevices::DecodeOutputPort(const int output)
{
    int port = ((output - 1) / 4) + 1;
    int subport = ((output - 1) % 4) + 1;

    return std::pair<int, int>(port, subport);
}