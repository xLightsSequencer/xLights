/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerE131.h"
#include <log.h>
#include <wx/socket.h>
#include "../../xLights/outputs/E131Output.h"
#include "ListenerManager.h"
#include "../../xLights/utils/UtilFunctions.h"
#include "../../xLights/ui/wxUtilities.h"

#include <sys/types.h>
#ifdef __WXMSW__
#include <iphlpapi.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

bool ListenerE131::IsValidHeader(uint8_t* buffer)
{
    return
        buffer[0] == 0x00 &&
        buffer[1] == 0x10 &&
        buffer[4] == 'A' &&
        buffer[5] == 'S' &&
        buffer[6] == 'C' &&
        buffer[7] == '-' &&
        buffer[8] == 'E' &&
        buffer[9] == '1' &&
        buffer[10] == '.' &&
        buffer[11] == '1' &&
        buffer[12] == '7';
}

ListenerE131::ListenerE131(ListenerManager* listenerManager, const std::string& localIP) : ListenerBase(listenerManager, localIP)
{
    _socket = nullptr;
}

void ListenerE131::Start()
{
    spdlog::debug("E131 listener starting.");
    _thread = new ListenerThread(this, _localIP);
}

void ListenerE131::Stop()
{
    if (!_stop)
    {
        spdlog::debug("E131 listener stopping.");
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        if (_thread != nullptr)
        {
            _stop = true;
            _thread->Stop();
            _thread->Delete();
            delete _thread;
            _thread = nullptr;
        }
    }
}

void ListenerE131::StartProcess(const std::string& localIP)
{
    wxIPV4address localaddr;
    if (localIP == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(localIP);
    }
    localaddr.Service(E131_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr)
    {
        spdlog::error("Error opening datagram for E131 reception. {}", localaddr.IPAddress().ToStdString());
    }
    else if (!_socket->IsOk())
    {
        spdlog::error("Error opening datagram for E131 reception. {} OK : FALSE", localaddr.IPAddress().ToStdString());
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        spdlog::error("Error opening datagram for E131 reception. {} : {} {}", (int)_socket->LastError(), DecodeIPError(_socket->LastError()), localaddr.IPAddress().ToStdString());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        spdlog::info("E131 reception datagram opened successfully.");
        _isOk = true;
    }

    for (const auto it : _multicastUniverses)         {
        Subscribe(it);
    }
}

void ListenerE131::Subscribe(uint16_t universe)
{
    if (_socket == nullptr) return;

    wxString ip = wxString::Format("%d.%d.%d.%d", 239, 255, (universe >> 8) & 0xFF, universe & 0xFF);

#ifdef __WXMSW__
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == nullptr) {
        spdlog::error("ListenerE131::Subscribe Error getting adapter info.");
        delete _socket;
        _socket = nullptr;
        return;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == nullptr) {
            spdlog::error("ListenerE131::Subscribe Error getting adapter info.");
            delete _socket;
            _socket = nullptr;
            return;
        }
    }

    int receiveSock = _socket->GetSocket();
    PIP_ADAPTER_INFO pAdapter = nullptr;
    DWORD dwRetVal = 0;
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    //inet_pton(AF_INET, MULTISYNC_MULTICAST_ADDRESS, &mreq.imr_multiaddr.s_addr);

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {

            auto ip = &pAdapter->IpAddressList;
            while (ip != nullptr) {
                auto ipc = wxSplit(ip->IpAddress.String, '.');
                if (ipc.size() == 4 && wxString(ip->IpAddress.String) != "0.0.0.0") {
                    uint8_t* p = (uint8_t*)&mreq.imr_interface.s_addr;
                    for (auto it : ipc) {
                        *p = (uint8_t)wxAtoi(it);
                        p++;
                    }

                    spdlog::debug("ListenerE131::Subscribe Subscribing on adapter {}.", ip->IpAddress.String);

                    if (setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) < 0) {
                        spdlog::warn("   Could not setup Multicast Group for interface {}\n", pAdapter->IpAddressList.IpAddress.String);
                    }
                }
                ip = ip->Next;
            }

            pAdapter = pAdapter->Next;
        }
    }

    free(pAdapterInfo);
#else
    struct ip_mreq mreq;
    struct ifaddrs* interfaces, * tmp;
    getifaddrs(&interfaces);
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    tmp = interfaces;
    int receiveSock = _socket->GetSocket();
    //loop through all the interfaces and subscribe to the group
    while (tmp) {
        //struct sockaddr_in *sin = (struct sockaddr_in *)tmp->ifa_addr;
        //strcpy(address, inet_ntoa(sin->sin_addr));
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* address = (struct sockaddr_in*)tmp->ifa_addr;
            mreq.imr_interface.s_addr = address->sin_addr.s_addr;
            if (setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                spdlog::warn("   ListenerE131::Subscribe Could not setup Multicast Group for interface {}\n", tmp->ifa_name);
            }
        }
        else if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6) {
            //FIXME for ipv6 multicast
            //LogDebug(VB_SYNC, "   Inet6 interface %s\n", tmp->ifa_name);
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(interfaces);
#endif   

}

void ListenerE131::StopProcess()
{
    if (_socket != nullptr) {
        spdlog::info("E131 Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerE131::Poll()
{


    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read E131 packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0)
        {
            _socket->WaitForRead(0, 50);
        }
        else
        {
            if (IsValidHeader(buffer))
            {
                int size = ((buffer[16] << 8) + buffer[17]) & 0x0FFF;
                int universe = (buffer[113] << 8) + buffer[114];
                //logger_base.debug("Processing packet.");
                _listenerManager->ProcessPacket(GetType(), universe, &buffer[126], size - 126);
                //logger_base.debug("Processing packet done.");
            }
        }
    }
}

void ListenerE131::AddMulticast(uint16_t universe)
{
    if (std::find(begin(_multicastUniverses), end(_multicastUniverses), universe) == end(_multicastUniverses)) {
        _multicastUniverses.push_back(universe);
        if (_socket != nullptr)             {
            Subscribe(universe);
        }
    }
}
