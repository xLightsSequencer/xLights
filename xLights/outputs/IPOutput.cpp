#include "IPOutput.h"

#include <wx/socket.h>
#include <wx/xml/xml.h>
#include <wx/regex.h>
#include <wx/protocol/http.h>

// This must be below the wx includes
#ifdef __WXMSW__
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#endif

#include <log4cpp/Category.hh>
#include "../UtilFunctions.h"
std::string IPOutput::__localIP = "";

static std::map<std::string, std::string> resolvedIPMap;
static const std::string &resolveIp(const std::string &ip) {
    if (IsIPValid(ip) || (ip == "MULTICAST")) {
        return ip;
    }
    const std::string &resolvedIp = resolvedIPMap[ip];
    if (resolvedIp == "") {
        wxIPV4address add;
        add.Hostname(ip);
        std::string r = add.IPAddress();
        if (r == "0.0.0.0") {
            r = ip;
        }
        resolvedIPMap[ip] = r;
        return resolvedIPMap[ip];
    }
    return resolvedIp;
}

#pragma region Constructors and Destructors
IPOutput::IPOutput(wxXmlNode* node) : Output(node)
{
    _ip = node->GetAttribute("ComPort", "").ToStdString();
    _resolvedIp = resolveIp(_ip);
    _universe = wxAtoi(node->GetAttribute("BaudRate", "1"));
}

IPOutput::IPOutput() : Output()
{
    _universe = 0;
    _ip = "";
    _resolvedIp = "";
}
#pragma endregion Constructors and Destructors

void IPOutput::SetIP(const std::string& ip) {
    Output::SetIP(ip);
    _resolvedIp = resolveIp(_ip);
}



#pragma region Static Functions
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

PINGSTATE IPOutput::Ping() const
{
    return IPOutput::Ping(GetIP());
}

PINGSTATE IPOutput::Ping(const std::string ip)
{
#ifdef __WXMSW__
    unsigned long ipaddr = inet_addr(ip.c_str());
    //unsigned long ipaddr = 0;
    //inet_pton(AF_INET, ip.c_str(), &ipaddr);
    if (ipaddr == INADDR_NONE) {
        return PINGSTATE::PING_ALLFAILED;
    }

    HANDLE hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        return PINGSTATE::PING_ALLFAILED;
    }

    char SendData[32] = "Data Buffer";
    uint32_t ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    void* ReplyBuffer = malloc(ReplySize);
    if (ReplyBuffer == nullptr) {
        IcmpCloseHandle(hIcmpFile);
        return PINGSTATE::PING_ALLFAILED;
    }

    uint32_t dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), nullptr, ReplyBuffer, ReplySize, 1000);
    if (dwRetVal != 0) {
        IcmpCloseHandle(hIcmpFile);
        free(ReplyBuffer);
        return PINGSTATE::PING_OK;
    }
    else
    {
        IcmpCloseHandle(hIcmpFile);
        free(ReplyBuffer);
        return PINGSTATE::PING_ALLFAILED;
    }
#else

    wxHTTP http;
    //http.SetMethod("GET");
    http.SetTimeout(2);
    bool connected = false;
    connected = http.Connect(ip, false);

    if (connected)
    {
        wxInputStream *httpStream = http.GetInputStream("/");
        if (http.GetError() == wxPROTO_NOERR)
        {
            return PINGSTATE::PING_WEBOK;
        }
        wxDELETE(httpStream);
        http.Close();
    }

    return PINGSTATE::PING_UNAVAILABLE;
#endif
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

std::string IPOutput::GetPingDescription() const
{
    return GetIP() + " " + GetDescription();
}

#pragma region Operators
bool IPOutput::operator==(const IPOutput& output) const
{
    if (GetType() != output.GetType()) return false;

    return _universe == output.GetUniverse() && (_ip == output.GetIP() || _ip == output.GetResolvedIP() || _resolvedIp == output.GetIP() || _resolvedIp == output.GetResolvedIP());
}
#pragma endregion Operators

