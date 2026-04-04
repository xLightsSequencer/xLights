
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "IPOutput.h"

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#endif

#include "utils/CurlManager.h"
#include "utils/ip_utils.h"

#include <log.h>

#pragma region Private Functions
void IPOutput::SaveAttr(pugi::xml_node node) {

    if (_ip != "") {
        node.append_attribute("ComPort") = _ip;
    }
    node.append_attribute("BaudRate") = _universe;

    Output::SaveAttr(node);
}
#pragma endregion

#pragma region Constructors and Destructors
IPOutput::IPOutput(pugi::xml_node node, bool isActive) : Output(node) {
    _ip = node.attribute("ComPort").as_string("");
    if (isActive) {
        SetResolvedIP(_ip);
        ip_utils::ResolveIP(_ip, [this](const std::string &r) {
            SetResolvedIP(r);
        });
    }
    _universe = node.attribute("BaudRate").as_int(1);
}

IPOutput::IPOutput() : Output() {
    _universe = 0;
    _ip = "";
}

IPOutput::IPOutput(const IPOutput& from) :
    Output(from) {
    _ip = from._ip;
    SetResolvedIP(from.GetResolvedIP());
    _universe = from._universe;
}

IPOutput::~IPOutput() {
    ip_utils::waitForAllToResolve();
}

pugi::xml_node IPOutput::Save(pugi::xml_node parent) {
    pugi::xml_node node = parent.append_child("network");
    SaveAttr(node);
    return node;
}
#pragma endregion

#pragma region Static Functions
Output::PINGSTATE IPOutput::Ping(const std::string& ip, const std::string& proxy) {

#ifdef _WIN32
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
            url += (ip_utils::IsIPv6(proxy) ? "[" + proxy + "]" : proxy) + "/proxy/";
        }
        url += ip + "/";
        if (CurlManager::HTTPSGet(url, "", "", 2) != "") {
            return Output::PINGSTATE::PING_WEBOK;
        } else {
            return Output::PINGSTATE::PING_ALLFAILED;
        }
#ifdef _WIN32
    }
#endif
}
#pragma endregion 

#pragma region Getters and Setters
void IPOutput::SetIP(const std::string& ip, bool isActive, bool resolve) {
    Output::SetIP(ip, isActive);
    if (isActive) {
        SetResolvedIP(ip);
        if (resolve) {
            ip_utils::ResolveIP(_ip, [this](const std::string &r) {
                SetResolvedIP(r);
            });
        }
    }
}
#pragma endregion 

#pragma region Operators
bool IPOutput::operator==(const IPOutput& output) const {
    if (GetType() != output.GetType()) return false;
    std::string rip = GetResolvedIP();
    return _universe == output.GetUniverse() && (_ip == output.GetIP() || _ip == output.GetResolvedIP() || rip == output.GetIP() || rip == output.GetResolvedIP());
}
#pragma endregion 
