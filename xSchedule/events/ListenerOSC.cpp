/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerOSC.h"
#include "ListenerManager.h"
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../OSCPacket.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"

#include <wx/socket.h>

#include <log4cpp/Category.hh>

ListenerOSC::ListenerOSC(ListenerManager* listenerManager, const std::string& localIP) :
    ListenerBase(listenerManager, localIP) {
    _frameMS = 50;
    _socket = nullptr;
}

void ListenerOSC::Start() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("OSC listener starting.");
    _thread = new ListenerThread(this, _localIP);
}

void ListenerOSC::Stop() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop) {
        logger_base.debug("OSC listener stopping.");
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

void ListenerOSC::StartProcess(const std::string& localIP) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address localaddr;
    if (localIP == "") {
        localaddr.AnyAddress();
    } else {
        localaddr.Hostname(localIP);
    }
    int port = _listenerManager->GetScheduleManager()->GetOptions()->GetOSCOptions()->GetClientPort();
    localaddr.Service(port);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
    if (_socket == nullptr) {
        logger_base.error("Error opening datagram for OSC reception on port %d. %s", port, (const char*)localaddr.IPAddress().c_str());
    } else if (!_socket->IsOk()) {
        logger_base.error("Error opening datagram for OSC reception on port %d. %s OK : FALSE", port, (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else if (_socket->Error()) {
        logger_base.error("Error opening datagram for OSC reception on port %d. %d : %s %s", port, _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("OSC reception datagram opened successfully on port %d.", port);
        _isOk = true;
    }
}

void ListenerOSC::StopProcess() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("OSC Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerOSC::Poll() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr) {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        // wxStopWatch sw;
        // logger_base.debug("Trying to read OSC packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        auto read = _socket->GetLastIOReadSize();

        if (_stop)
            return;
        // logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0) {
            _socket->WaitForRead(0, 50);
        } else {
            OSCPacket packet(buffer, sizeof(buffer), _listenerManager->GetScheduleManager()->GetOptions()->GetOSCOptions(), _frameMS);

            if (packet.IsSync()) {
                std::string stepname = packet.GetStepName();
                std::string timingname = packet.GetTimingName();
                long ms = packet.GetMS(_frameMS);

                if (stepname != "") {
                    _frameMS = _listenerManager->Sync(stepname, ms, GetType());
                } else if (timingname != "") {
                    _frameMS = _listenerManager->Sync(timingname, ms, GetType());
                }
            } else if (packet.IsOk()) {
                logger_base.debug("OSC Path: %s.", (const char*)packet.GetPath().c_str());
                _listenerManager->ProcessPacket(GetType(), packet.GetPath(), packet.GetP1(), packet.GetP2(), packet.GetP3());
            } else {
                logger_base.debug("Invalid OSC Packet.");
                DumpBinary(buffer, read);
            }
        }
    }
}
