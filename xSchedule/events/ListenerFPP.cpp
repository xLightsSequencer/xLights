/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerFPP.h"
#include "ListenerManager.h"
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../../xLights/xLightsVersion.h"
#include "../Control.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>

#include <sys/types.h>
#ifdef __WXMSW__
#include <iphlpapi.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#endif

bool ListenerFPP::IsValidHeader(uint8_t* buffer) {
    return buffer[0] == 'F' &&
           buffer[1] == 'P' &&
           buffer[2] == 'P' &&
           buffer[3] == 'D';
}

ListenerFPP::ListenerFPP(ListenerManager* listenerManager, const std::string& localIP) :
    ListenerBase(listenerManager, localIP) {
    _frameMS = 50;
    _socket = nullptr;
}

void ListenerFPP::Start() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP listener starting.");
    _thread = new ListenerThread(this, _localIP);
}

void ListenerFPP::Stop() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop) {
        logger_base.debug("FPP listener stopping.");
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        if (_thread != nullptr) {
            _stop = true;
            _thread->Stop();
            _thread->Delete();
            delete _thread;
            _thread = nullptr;
        }
    }
}

void ListenerFPP::StartProcess(const std::string& localIP) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address localaddr;
    // if (IPOutput::GetLocalIP() == "")
    {
        localaddr.AnyAddress();
    }
    // else
    {
        //    localaddr.Hostname(IPOutput::GetLocalIP());
    }
    localaddr.Service(FPP_CTRL_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr) {
        logger_base.error("Error opening datagram for FPP reception. %s", (const char*)localaddr.IPAddress().c_str());
    } else if (!_socket->IsOk()) {
        logger_base.error("Error opening datagram for FPP reception. %s OK : FALSE", (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else if (_socket->Error()) {
        logger_base.error("Error opening datagram for FPP reception. %d : %s %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else {
#ifdef __WXMSW__
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
        if (pAdapterInfo == nullptr) {
            logger_base.error("Error getting adapter info.");
            delete _socket;
            _socket = nullptr;
            return;
        }

        if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            if (pAdapterInfo == nullptr) {
                logger_base.error("Error getting adapter info.");
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
        mreq.imr_multiaddr.s_addr = inet_addr(MULTISYNC_MULTICAST_ADDRESS);
        // inet_pton(AF_INET, MULTISYNC_MULTICAST_ADDRESS, &mreq.imr_multiaddr.s_addr);

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

                        logger_base.debug("FPP Remote Subscribing on adapter %s.", (const char*)ip->IpAddress.String);

                        if (setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) < 0) {
                            logger_base.warn("   Could not setup Multicast Group for interface %s\n", (const char*)pAdapter->IpAddressList.IpAddress.String);
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
        struct ifaddrs *interfaces, *tmp;
        getifaddrs(&interfaces);
        memset(&mreq, 0, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(MULTISYNC_MULTICAST_ADDRESS);
        tmp = interfaces;
        int receiveSock = _socket->GetSocket();
        // loop through all the interfaces and subscribe to the group
        while (tmp) {
            // struct sockaddr_in *sin = (struct sockaddr_in *)tmp->ifa_addr;
            // strcpy(address, inet_ntoa(sin->sin_addr));
            if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* address = (struct sockaddr_in*)tmp->ifa_addr;
                mreq.imr_interface.s_addr = address->sin_addr.s_addr;
                if (setsockopt(receiveSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                    logger_base.warn("   Could not setup Multicast Group for interface %s\n", tmp->ifa_name);
                }
            } else if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6) {
                // FIXME for ipv6 multicast
                // LogDebug(VB_SYNC, "   Inet6 interface %s\n", tmp->ifa_name);
            }
            tmp = tmp->ifa_next;
        }
        freeifaddrs(interfaces);
#endif
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("FPP reception datagram opened successfully.");
        _isOk = true;
    }
}

void ListenerFPP::StopProcess() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("FPP Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerFPP::Poll() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr) {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        // wxStopWatch sw;
        // logger_base.debug("Trying to read FPP packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop)
            return;
        // logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0) {
            // logger_base.debug("Waiting for read.");
            _socket->WaitForRead(0, 50);
            // logger_base.debug("Waiting for read done.");
        } else {
            wxIPV4address from;
            // We only process if it did not come from us or we cant determine where it came from
            if (!_socket->GetPeer(from) || !IsValidLocalIP(from)) {
                if (IsValidHeader(buffer)) {
                    ControlPkt* cp = (ControlPkt*)&buffer[0];
                    if (cp->pktType == CTRL_PKT_SYNC) {
                        SyncPkt* sp = (SyncPkt*)(&buffer[0] + sizeof(ControlPkt));

                        if (sp->fileType == SYNC_FILE_SEQ) {
                            uint8_t packetType = sp->pktType;
                            std::string fileName = std::string(sp->filename);
                            uint32_t frameNumber = sp->frameNumber;
                            long ms = frameNumber * _frameMS;

                            logger_base.debug("FPP Sync type %d frame %u ms %ld %s.", (int)packetType, frameNumber, ms, (const char*)fileName.c_str());

                            switch (packetType) {
                            case SYNC_PKT_START: {
                                logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote start %s.", (const char*)fileName.c_str());

                                _listenerManager->Sync(fileName, 0, GetType());
                            } break;
                            case SYNC_PKT_STOP: {
                                logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote stop %s.", (const char*)fileName.c_str());
                                _listenerManager->Sync(fileName, 0xFFFFFFFF, GetType());
                            } break;
                            case SYNC_PKT_SYNC: {
                                _listenerManager->Sync(fileName, ms, GetType());
                            } break;
                            default:
                                break;
                            }
                        }
                    } else if (cp->pktType == CTRL_PKT_EVENT) {
                        _listenerManager->ProcessPacket(GetType(), std::string((char*)(buffer + sizeof(ControlPkt))));
                    } else if (cp->pktType == CTRL_PKT_CMD) {
                        // FIXME - command?
                        std::string str(buffer, buffer + sizeof(buffer));
                        std::vector<std::string> parms;
                        std::string parm;
                        for (int i = 5; i < sizeof(buffer); ++i) {
                            if (0x20 > buffer[i] || 0x7E < buffer[i]) {
                                if (!parm.empty()) {
                                    parms.push_back(parm);
                                    parm.clear();
                                }
                            } else {
                                parm += buffer[i];
                            }
                        }
                        if (parms.size() == 4) {
                            if (parms[2].compare("Trigger Command Preset") == 0) {
                                _listenerManager->ProcessPacket("FPPCommandPreset", parms[3]);
                            }
                        }
                    } else if (cp->pktType == CTRL_PKT_BLANK) {
                        // FIXME - blank data
                    } else if (cp->pktType == CTRL_PKT_PING) {
                        if (buffer[8] == 1 || buffer[15] == 0) { // 1 is discover
                            uint8_t outBuf[512];
                            memset(outBuf, 0, sizeof(outBuf));

                            ControlPkt* cpkt = (ControlPkt*)outBuf;
                            cpkt->fppd[0] = 'F';
                            cpkt->fppd[1] = 'P';
                            cpkt->fppd[2] = 'P';
                            cpkt->fppd[3] = 'D';
                            cpkt->pktType = CTRL_PKT_PING;
                            cpkt->extraDataLen = 294; // v3 ping length

                            uint8_t* ed = (uint8_t*)(outBuf + 7);

                            auto v = wxSplit(xlights_version_string, '.');
                            int majorVersion = wxAtoi(v[0]);
                            int minorVersion = wxAtoi(v[1]);

                            ed[0] = 3;    // ping version 2
                            ed[1] = 0;    // 0 = ping, 1 = discover
                            ed[2] = 0xC1; // xLights type
                            ed[3] = (majorVersion & 0xFF00) >> 8;
                            ed[4] = (majorVersion & 0x00FF);
                            ed[5] = (minorVersion & 0xFF00) >> 8;
                            ed[6] = (minorVersion & 0x00FF);
                            ed[7] = 0x08; // REMOTE mode

                            wxIPV4address localaddr;
                            localaddr.Hostname(wxGetFullHostName());
                            wxString ipAddr = localaddr.IPAddress();

                            wxArrayString ip = wxSplit(ipAddr, '.');
                            ed[8] = wxAtoi(ip[0]);
                            ed[9] = wxAtoi(ip[1]);
                            ed[10] = wxAtoi(ip[2]);
                            ed[11] = wxAtoi(ip[3]);

                            strncpy((char*)(ed + 12), wxGetHostName().c_str(), 65);
                            strncpy((char*)(ed + 77), xlights_version_string.c_str(), 41);
                            strncpy((char*)(ed + 118), "xSchedule", 41);
                            // strncpy((char *)(ed + 159), sysInfo.ranges.c_str(), 41);

                            wxString r = wxString::Format("%d.%d.%d.255", (uint8_t)buffer[15], (uint8_t)buffer[16], (uint8_t)buffer[17]);
                            wxIPV4address remoteAddr;
                            if (buffer[15] == 0) {
                                remoteAddr.Hostname("255.255.255.255");
                            } else {
                                remoteAddr.Hostname(r);
                            }
                            remoteAddr.Service(FPP_CTRL_PORT);
                            _socket->SendTo(remoteAddr, outBuf, 294 + 7);
                        }
                    }
                }
            }
        }
    }
}
