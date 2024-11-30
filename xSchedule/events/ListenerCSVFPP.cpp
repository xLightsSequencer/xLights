/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerCSVFPP.h"
#include "ListenerManager.h"
#include "../../xLights/UtilFunctions.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../Control.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>

bool ListenerCSVFPP::IsValidHeader(uint8_t* buffer) {
    return buffer[0] == 'F' &&
           buffer[1] == 'P' &&
           buffer[2] == 'P' &&
           buffer[3] == ',' &&
           buffer[4] == '1' &&
           buffer[5] == ',' &&
           buffer[6] == '0';
}

ListenerCSVFPP::ListenerCSVFPP(ListenerManager* listenerManager, const std::string& localIP) :
    ListenerBase(listenerManager, localIP) {
    _socket = nullptr;
}

void ListenerCSVFPP::Start() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP CSV listener starting.");
    _thread = new ListenerThread(this, _localIP);
}

void ListenerCSVFPP::Stop() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop) {
        logger_base.debug("FPP CSV listener stopping.");
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

void ListenerCSVFPP::StartProcess(const std::string& localIP) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address localaddr;
    if (localIP == "") {
        localaddr.AnyAddress();
    } else {
        localaddr.Hostname(localIP);
    }
    localaddr.Service(FPP_CTRL_CSV_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_NONE);
    if (_socket == nullptr) {
        logger_base.error("Error opening datagram for FPP CSV reception. %s", (const char*)localaddr.IPAddress().c_str());
    } else if (!_socket->IsOk()) {
        logger_base.error("Error opening datagram for FPP CSV reception. %s OK : FALSE", (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else if (_socket->Error()) {
        logger_base.error("Error opening datagram for FPP CSV reception. %d : %s %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    } else {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("FPP CSV reception datagram opened successfully.");
        _isOk = true;
    }
}

void ListenerCSVFPP::StopProcess() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("FPP CSV Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerCSVFPP::Poll() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr) {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        // wxStopWatch sw;
        // logger_base.debug("Trying to read FPP unicast packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop)
            return;
        // logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0) {
            _socket->WaitForRead(0, 50);
        } else {
            if (IsValidHeader(buffer)) {
                static wxString lastMessage = "";

                char* cr = strchr((char*)buffer, '\n');
                if ((size_t)cr - (size_t)&buffer[0] < sizeof(buffer)) {
                    *cr = 0x00;
                }

                wxString msg = wxString(buffer);

                if (msg == lastMessage) {
                    // we dont want to double process
                } else {
                    logger_base.debug("Pkt %s.", (const char*)msg.c_str());
                    lastMessage = msg;
                    wxArrayString components = wxSplit(msg, ',');

                    if (components.size() >= 5) {
                        // uint8_t packetType = wxAtoi(components[3]);
                        std::string fileName = components[4].ToStdString();
                        int action = wxAtoi(components[3]);
                        if (components.size() >= 7 && action == SYNC_PKT_SYNC) {
                            int secondsElapsed = wxAtoi(components[5]) * 1000 + wxAtoi(components[6]);
                            _listenerManager->Sync(fileName, secondsElapsed, GetType());
                        } else if (action == SYNC_PKT_STOP) {
                            logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote stop %s.", (const char*)fileName.c_str());
                            _listenerManager->Sync(fileName, 0xFFFFFFFF, GetType());
                        } else if (action == SYNC_PKT_START) {
                            logger_base.debug("!!!!!!!!!!!!!!!!!!!!!!!!!!! Remote start %s.", (const char*)fileName.c_str());
                            _listenerManager->Sync(fileName, 0, GetType());
                        }
                    }
                    logger_base.debug("Pkt dispatched.");
                }
            }
        }
    }
}
