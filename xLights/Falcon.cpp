#include "Falcon.h"
#include <wx/msgdlg.h>
#include <wx/sstream.h>
#include <log4cpp/Category.hh>
#include <wx/regex.h>
#include <wx/xml/xml.h>

Falcon::Falcon(const std::string& ip)
{
	_ip = ip;
	
    _http.SetMethod("GET");
	_connected = _http.Connect(_ip);

    if (_connected)
    {
        std::string versionxml = GetURL("/status.xml");
        if (versionxml != "")
        {
            static wxRegEx versionregex("(\\<v\\>)([0-9]+\\.[0-9]+)\\<\\/v\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(versionxml)))
            {
                _version = versionregex.GetMatch(wxString(versionxml), 2).ToStdString();
            }
        }
        else
        {
            std::string version = GetURL("/index.htm");

            //<title>F4V2            - v1.10</title>
            static wxRegEx versionregex("(title.*?v)([0-9]+\\.[0-9]+)\\<\\/title\\>", wxRE_ADVANCED | wxRE_NEWLINE);
            if (versionregex.Matches(wxString(version)))
            {
                _version = versionregex.GetMatch(wxString(version), 2).ToStdString();
            }
        }
    }
    else
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        wxProtocolLog* log = _http.GetLog();
        logger_base.error("Error connecting to falcon controller on %s.", _ip);
    }
}

Falcon::~Falcon()
{
    _http.Close();
}

std::string Falcon::GetURL(const std::string& url)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;
    _http.SetMethod("GET");
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to falcon '%s'.", (const char *)url.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        logger_base.debug("Response from falcon '%s'.", (const char *)res.c_str());
    }
    else
    {
        logger_base.error("Unable to connect to falcon '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
    }

    wxDELETE(httpStream);
    return res.ToStdString();
}

std::string Falcon::PutURL(const std::string& url, const std::string& request)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString res;
    _http.SetMethod("POST");
    _http.SetPostText("application/x-www-form-urlencoded", request);
    wxInputStream *httpStream = _http.GetInputStream(wxString(url));
    logger_base.debug("Making request to falcon '%s'.", (const char *)url.c_str());
    logger_base.debug("    With data '%s'.", (const char *)request.c_str());

    if (_http.GetError() == wxPROTO_NOERR)
    {
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

        logger_base.debug("Response from falcon '%s'.", (const char *)res.c_str());
    }
    else
    {
        logger_base.error("Unable to connect to falcon '%s'.", (const char *)url.c_str());
        wxMessageBox(_T("Unable to connect!"));
    }
    _http.SetPostText("", "");

    wxDELETE(httpStream);
    return res.ToStdString();
}

void Falcon::SetInputUniverses(const wxXmlNode* root)
{
    wxString request;
    long currentcontrollerstartchannel = 0;
    long currentcontrollerendchannel = 0;
    int nullcount = 1;
    int output = 0;

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "network")
        {
            currentcontrollerstartchannel = currentcontrollerendchannel + 1;
            wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
            long MaxChannels;
            MaxChannelsStr.ToLong(&MaxChannels);
            int universes = wxAtoi(e->GetAttribute("NumUniverses", "1"));
            currentcontrollerendchannel = currentcontrollerstartchannel + (MaxChannels * universes) - 1;

            std::string type = std::string(e->GetAttribute("NetworkType", ""));
            std::string ip = std::string(e->GetAttribute("ComPort", ""));
            int u = wxAtoi(e->GetAttribute("BaudRate", ""));
            int t = -1;
            if (type == "E131")
            {
                t = 0;
            }
            else if (type == "ArtNet")
            {
                t = 1;
            }
            if ((type == "E131" || type == "ArtNet") && ip == _ip)
            {
                for (int i = 0; i < universes; i++)
                {
                    request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
                        output, u + i,
                        output, MaxChannels,
                        output, currentcontrollerstartchannel,
                        output, t);
                    output++;
                }
            }
        }
    }

    request = wxString::Format("z=%d", output) + request;
    std::string response = PutURL("/E131.htm", request.ToStdString());
}

void Falcon::SetInputUniverses(const wxXmlNode* root, std::list<int>& selected)
{
    wxString request;
    long currentcontrollerstartchannel = 0;
    long currentcontrollerendchannel = 0;
    int nullcount = 1;
    int output = 0;
    int node = 0;

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "network")
        {
            currentcontrollerstartchannel = currentcontrollerendchannel + 1;
            wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
            long MaxChannels;
            MaxChannelsStr.ToLong(&MaxChannels);
            int universes = wxAtoi(e->GetAttribute("NumUniverses", "1"));
            currentcontrollerendchannel = currentcontrollerstartchannel + (MaxChannels * universes) - 1;

            std::string type = std::string(e->GetAttribute("NetworkType", ""));
            std::string ip = std::string(e->GetAttribute("ComPort", ""));
            int u = wxAtoi(e->GetAttribute("BaudRate", ""));
            int t = -1;
            if (type == "E131")
            {
                t = 0;
            }
            else if (type == "ArtNet")
            {
                t = 1;
            }
            if ((type == "E131" || type == "ArtNet") && (ip == _ip || std::find(selected.begin(), selected.end(), node) != selected.end()))
            {
                for (int i = 0; i < universes; i++)
                {
                    request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d",
                        output, u + i,
                        output, MaxChannels,
                        output, currentcontrollerstartchannel,
                        output, t);
                    output++;
                }
            }
            node++;
        }
    }

    request = wxString::Format("z=%d", output) + request;
    std::string response = PutURL("/E131.htm", request.ToStdString());
}

void Falcon::SetInputUniverses(const std::list<wxXmlNode>& inputs)
{
    wxString request;

    request = wxString::Format("z=%d", inputs.size());

    int output = 0;
    int ch = 1;

    for (auto it = inputs.begin(); it != inputs.end(); ++it)
    {
        std::string type = std::string(it->GetAttribute("NetworkType", ""));
        int t = -1;

        if (type == "E131")
        {
            t = 0;
        }
        else if (type == "ArtNet")
        {
            t = 1;
        }

        int u = wxAtoi(it->GetAttribute("BaudRate", ""));
        wxString MaxChannelsStr = it->GetAttribute("MaxChannels", "0");
        long c;
        MaxChannelsStr.ToLong(&c);
        int ucount = wxAtoi(it->GetAttribute("NumUniverses", "1"));

        for (int i = 0; i < ucount; i++)
        {
            request += wxString::Format("&u%d=%d&s%d=%d&c%d=%d&t%d=%d", 
                output, u + i,
                output, c,
                output, ch,
                output, t);
            ch += c;
            output++;
        }
    }

    std::string response = PutURL("/E131.htm", request.ToStdString());
}

