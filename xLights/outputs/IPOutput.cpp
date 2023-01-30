
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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

#include "utils/Curl.h"
#include "utils/ip_utils.h"

#include <log4cpp/Category.hh>

#pragma region Private Functions
void IPOutput::Save(wxXmlNode* node) {

    if (_ip != "") {
        node->AddAttribute("ComPort", _ip);
    }
    node->AddAttribute("BaudRate", wxString::Format("%d", _universe));

    Output::Save(node);
}
#pragma endregion

#pragma region Constructors and Destructors
IPOutput::IPOutput(wxXmlNode* node) : Output(node) {

    _ip = node->GetAttribute("ComPort", "").ToStdString();
    _resolvedIp = ip_utils::ResolveIP(_ip);
    _universe = wxAtoi(node->GetAttribute("BaudRate", "1"));
}

IPOutput::IPOutput() : Output() {
    _universe = 0;
    _ip = "";
    _resolvedIp = "";
}

wxXmlNode* IPOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}
#pragma endregion 

#pragma region Static Functions
Output::PINGSTATE IPOutput::Ping(const std::string& ip, const std::string& proxy) {

#ifdef __WXMSW__
    if (proxy == "") {
        unsigned long ipaddr = inet_addr(ip.c_str());
        //unsigned long ipaddr = 0;
        //inet_pton(AF_INET, ip.c_str(), &ipaddr);
        if (ipaddr == INADDR_NONE) {
            return Output::PINGSTATE::PING_ALLFAILED;
        }

        HANDLE hIcmpFile = IcmpCreateFile();
        if (hIcmpFile == INVALID_HANDLE_VALUE) {
            return Output::PINGSTATE::PING_ALLFAILED;
        }

        char SendData[32] = "Data Buffer";
        uint32_t ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
        ICMP_ECHO_REPLY* ReplyBuffer = (ICMP_ECHO_REPLY*)malloc(ReplySize);
        if (ReplyBuffer == nullptr) {
            IcmpCloseHandle(hIcmpFile);
            return Output::PINGSTATE::PING_ALLFAILED;
        }

        uint32_t dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), nullptr, ReplyBuffer, ReplySize, 1000);
        if (dwRetVal != 0 && ReplyBuffer->Status == 0) {
            IcmpCloseHandle(hIcmpFile);
            free(ReplyBuffer);
            return Output::PINGSTATE::PING_OK;
        }
        else {
            IcmpCloseHandle(hIcmpFile);
            free(ReplyBuffer);
            return Output::PINGSTATE::PING_ALLFAILED;
        }
    }
    else {
#endif
        std::string url = "http://";
        if (proxy != "") {
            url += proxy + "/proxy/";
        }
        url += ip + "/";
        if (Curl::HTTPSGet(url, "", "", 2) != "") {
            return Output::PINGSTATE::PING_WEBOK;
        }
        else {
            return Output::PINGSTATE::PING_ALLFAILED;
        }
#ifdef __WXMSW__
    }
#endif
}
#pragma endregion 

#pragma region Getters and Setters
void IPOutput::SetIP(const std::string& ip) {

    Output::SetIP(ip);
    _resolvedIp = ip_utils::ResolveIP(_ip);
}
#pragma endregion 

#pragma region Operators
bool IPOutput::operator==(const IPOutput& output) const {

    if (GetType() != output.GetType()) return false;

    return _universe == output.GetUniverse() && (_ip == output.GetIP() || _ip == output.GetResolvedIP() || _resolvedIp == output.GetIP() || _resolvedIp == output.GetResolvedIP());
}
#pragma endregion 
