#include "IPOutput.h"

#include <wx/xml/xml.h>
#include <wx/regex.h>
#include <wx/socket.h>
#include <log4cpp/Category.hh>

std::string IPOutput::__localIP = "";

#pragma region Constructors and Destructors
IPOutput::IPOutput(wxXmlNode* node) : Output(node)
{
    _ip = node->GetAttribute("ComPort", "").ToStdString();
    _universe = wxAtoi(node->GetAttribute("BaudRate", "1"));
}

IPOutput::IPOutput() : Output()
{
    _universe = 0;
    _ip = "";
}
#pragma endregion Constructors and Destructors

#pragma region Static Functions
bool IPOutput::IsIPValidOrHostname(const std::string &ip, bool iponly) {
    if (IsIPValid(ip)) {
        return true;
    }

    bool hasChar = false;
    bool hasDot = false;
    //hostnames need at least one char in it if fully qualified
    //if not fully qualified (no .), then the hostname only COULD be just numeric
    for (int y = 0; y < ip.length(); y++) {
        char x = ip[y];
        if ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == '-') {
            hasChar = true;
        }
        if (x == '.') {
            hasDot = true;
        }
    }
    if (hasChar || (!hasDot && !hasChar)) {
        if (iponly) return true;
        wxIPV4address addr;
        addr.Hostname(ip);
        wxString ipAddr = addr.IPAddress();
        if (ipAddr != "0.0.0.0") {
            return true;
        }
    }
    return false;
}
bool IPOutput::IsIPValid(const std::string &ip)
{
    wxString ips = wxString(ip).Trim(false).Trim(true);
    if (ips == "")
    {
        return false;
    }
    else
    {
        static wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");

        if (regxIPAddr.Matches(ips))
        {
            return true;
        }
    }

    return false;
}

std::string IPOutput::CleanupIP(const std::string &ip)
{
    bool hasChar = false;
    bool hasDot = false;
    //hostnames need at least one char in it if fully qualified
    //if not fully qualified (no .), then the hostname only COULD be just numeric
    for (int y = 0; y < ip.length(); y++) {
        char x = ip[y];
        if ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == '-') {
            hasChar = true;
        }
        if (x == '.') {
            hasDot = true;
        }
    }
    if (hasChar || !hasDot) {
        //hostname, not ip, don't mangle it
        return ip;
    }
    wxString IpAddr(ip.c_str());
    static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
    if (leadingzero1.Matches(IpAddr))
    {
        wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
        leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
    }
    static wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
    while (leadingzero2.Matches(IpAddr)) // need to do it several times because the results overlap
    {
        wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
        leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
    }
    return IpAddr.ToStdString();
}
#pragma endregion Static Functions

wxXmlNode* IPOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}

void IPOutput::Save(wxXmlNode* node)
{
    if (_ip != "")
    {
        node->AddAttribute("ComPort", _ip);
    }

    node->AddAttribute("BaudRate", wxString::Format("%d", _universe));

    Output::Save(node);
}

std::string IPOutput::DecodeError(wxSocketError err)
{
    switch (err)
    {
    case wxSOCKET_NOERROR:
        return "No Error";
    case wxSOCKET_INVOP:
        return "Invalid Operation";
    case wxSOCKET_IOERR:
        return "IO Error";
    case wxSOCKET_INVADDR:
        return "Invalid Address";
    case wxSOCKET_INVSOCK:
        return "Invalid Socket";
    case wxSOCKET_NOHOST:
        return "No Host";
    case wxSOCKET_INVPORT:
        return "Invalid Port";
    case wxSOCKET_WOULDBLOCK:
        return "Would Block";
    case wxSOCKET_TIMEDOUT:
        return "Timeout";
    case wxSOCKET_MEMERR:
        return "Memory Error";
    case wxSOCKET_OPTERR:
        return "Option Error";
    default:
        return "God knows what happened";
    }
}

#pragma region Operators
bool IPOutput::operator==(const IPOutput& output) const
{
    if (GetType() != output.GetType()) return false;

    return _universe == output.GetUniverse() && _ip == output.GetIP();
}
#pragma endregion Operators

