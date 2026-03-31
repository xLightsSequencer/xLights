/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemFPPEvent.h"
#include "PlayList.h"
#include "PlayListItemFPPEventPanel.h"
#include "../../xLights/utils/UtilFunctions.h"
#include "../../xLights/ui/wxUtilities.h"
#include "../Control.h"
#include "../xLights/outputs/IPOutput.h"
#include "../xLights/utils/ip_utils.h"
#include "utils/CurlManager.h"
#include <log.h>
#include <wx/notebook.h>
#include <wx/protocol/http.h>
#include <wx/socket.h>
#include <wx/xml/xml.h>

class FPPEventThread : public wxThread {
    std::string _ip;
    int _major;
    int _minor;
    int _method = 2;
    std::string _localIP;

public:
    FPPEventThread(const std::string& ip, int method, int major, int minor, const std::string& localIP) :
        _ip(ip), _major(major), _minor(minor), _method(method), _localIP(localIP) {
    }

    std::string GetEventString() const {
        return wxString::Format("%02d_%02d", _major, _minor).ToStdString();
    }

    virtual void* Entry() override {
        

        spdlog::debug("PlayListFPPEvent in thread.");

        if (ip_utils::IsIPValidOrHostname(_ip) && _ip != "255.255.255.255") {
#ifdef EXTREME_FPPEVENT_LOGGING
            spdlog::debug("   Getting URL");
#endif
            if (_method == 0) {
                std::string eventString = GetEventString();
                std::string url = "http://" + _ip + "/fppxml.php?command=triggerEvent&id=" + eventString;
                spdlog::debug("FPP Event sent {}:{}", _ip, url);
                auto res = CurlManager::HTTPSGet(url, "", "", 1);
                spdlog::info("CURL GET: {}", res);
            } else if (_method == 1) {
                std::string url = "http://" + _ip + "/api/command";
                std::string body = wxString::Format("{\"command\":\"Trigger Event\",\"args\":[\"%u\",\"%u\"]}", _major, _minor).ToStdString();
                spdlog::debug("FPP Event sent {}:{}:{}", _ip, url, body);
                auto res = CurlManager::HTTPSPost("http://" + _ip + "/api/command", body);
            } else {
                std::string url = "http://" + _ip + "/api/command";
                std::string body = wxString::Format("{\"command\":\"Trigger Command Preset Slot\",\"args\":[\"%u\"]}", _major).ToStdString();
                spdlog::debug("FPP Event sent {}:{}:{}", _ip, url, body);
                auto res = CurlManager::HTTPSPost("http://" + _ip + "/api/command", body);
            }
        } else {
            // I am pretty sure this no longer works in FPP ... at least I dont seem to be able to make it work

            // Open the socket
            wxIPV4address localaddr;
            if (_localIP == "") {
                localaddr.AnyAddress();
            } else {
                localaddr.Hostname(_localIP);
            }

            wxDatagramSocket* socket = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
            if (socket == nullptr) {
                spdlog::error("Error opening datagram for FPP Event send. {}", localaddr.IPAddress().ToStdString());
            } else if (!socket->IsOk()) {
                spdlog::error("Error opening datagram for FPP Event send. {} OK : FALSE", localaddr.IPAddress().ToStdString());
                delete socket;
                socket = nullptr;
            } else if (socket->Error()) {
                spdlog::error("Error opening datagram for FPP Event send. {} : {} {}", (int)socket->LastError(), DecodeIPError(socket->LastError()), localaddr.IPAddress().ToStdString());
                delete socket;
                socket = nullptr;
            } else {
                spdlog::info("FPP Event send datagram opened successfully.");
            }

            if (socket != nullptr) {
                wxIPV4address remoteAddr;
                // remoteAddr.BroadcastAddress();
                remoteAddr.Hostname("255.255.255.255");
                remoteAddr.Service(FPP_CTRL_PORT);

                wxASSERT(sizeof(ControlPkt) == 7); // ensure data is packed correctly

                std::string eventString = GetEventString();
                int dbufsize = sizeof(ControlPkt) + eventString.size() + 1;
                unsigned char* dbuffer = (unsigned char*)malloc(dbufsize);
                memset(dbuffer, 0x00, dbufsize);

                if (dbuffer != nullptr) {
                    ControlPkt* cp = (ControlPkt*)dbuffer;
                    memcpy(cp->fppd, "FPPD", 4);
                    cp->pktType = CTRL_PKT_EVENT;
                    cp->extraDataLen = dbufsize - sizeof(ControlPkt) - 1;
                    strcpy((char*)(dbuffer + sizeof(ControlPkt)), eventString.c_str());

                    socket->SendTo(remoteAddr, dbuffer, dbufsize - 1);
                    spdlog::info("FPP Event broadcast {}.", eventString);

                    free(dbuffer);
                }

                spdlog::info("FPP Event send datagram closed.");
                socket->Close();
                delete socket;
            }
        }

        spdlog::debug("PlayListFPPEvent thread done.");

        return nullptr;
    }
};

PlayListItemFPPEvent::PlayListItemFPPEvent(wxXmlNode* node) :
    PlayListItem(node) {
    PlayListItemFPPEvent::Load(node);
}

void PlayListItemFPPEvent::Load(wxXmlNode* node) {
    PlayListItem::Load(node);
    _major = wxAtoi(node->GetAttribute("Major", "1"));
    _minor = wxAtoi(node->GetAttribute("Minor", "1"));
    _ip = node->GetAttribute("IP", "");
    _method = wxAtoi(node->GetAttribute("Method", "2"));
}

PlayListItemFPPEvent::PlayListItemFPPEvent() :
    PlayListItem() {
    _type = "PLIFPPEVENT";
}

PlayListItem* PlayListItemFPPEvent::Copy(const bool isClone) const {
    PlayListItemFPPEvent* res = new PlayListItemFPPEvent();
    res->_major = _major;
    res->_minor = _minor;
    res->_ip = _ip;
    res->_started = false;
    res->_method = _method;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemFPPEvent::Save() {
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Major", wxString::Format("%d", _major));
    node->AddAttribute("Minor", wxString::Format("%d", _minor));
    node->AddAttribute("IP", _ip);
    node->AddAttribute("Method", wxString::Format("%d", _method));
    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFPPEvent::GetTitle() const {
    return "FPP Event";
}

void PlayListItemFPPEvent::Configure(wxNotebook* notebook) {
    notebook->AddPage(new PlayListItemFPPEventPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemFPPEvent::GetNameNoTime() const {
    if (_name != "")
        return _name;

    if (_method == 2) {
        return wxString::Format("%02d", _major).ToStdString();
    }

    return wxString::Format("%02d_%02d", _major, _minor).ToStdString();
}

std::string PlayListItemFPPEvent::GetTooltip() {
    return "";
}

// #define EXTREME_FPPEVENT_LOGGING

void PlayListItemFPPEvent::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) {
    if (ms >= _delay && !_started) {
        _started = true;

        FPPEventThread* thread = new FPPEventThread(_ip, _method, _major, _minor, GetLocalIP());
        thread->Run();
        wxMicroSleep(1); // encourage the thread to run        }
    }
}

void PlayListItemFPPEvent::Start(long stepLengthMS) {
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
