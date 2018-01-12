#include "PlayListItemFPPEvent.h"
#include "PlayListItemFPPEventPanel.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "../xLights/outputs/IPOutput.h"
#include "../Control.h"

PlayListItemFPPEvent::PlayListItemFPPEvent(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _major = 1;
    _minor = 1;
    PlayListItemFPPEvent::Load(node);
}

void PlayListItemFPPEvent::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _major = wxAtoi(node->GetAttribute("Major", "1"));
    _minor = wxAtoi(node->GetAttribute("Minor", "1"));
}

PlayListItemFPPEvent::PlayListItemFPPEvent() : PlayListItem()
{
    _started = false;
    _major = 1;
    _minor = 1;
}

PlayListItem* PlayListItemFPPEvent::Copy() const
{
    PlayListItemFPPEvent* res = new PlayListItemFPPEvent();
    res->_major = _major;
    res->_minor = _minor;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemFPPEvent::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIFPPEVENT");

    node->AddAttribute("Major", wxString::Format("%d", _major));
    node->AddAttribute("Minor", wxString::Format("%d", _minor));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFPPEvent::GetTitle() const
{
    return "FPP Event";
}

void PlayListItemFPPEvent::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFPPEventPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemFPPEvent::GetNameNoTime() const
{
    if (_name != "") return _name;

    return GetEventString();
}

std::string PlayListItemFPPEvent::GetEventString() const
{
    return wxString::Format("%02d_%02d", _major, _minor).ToStdString();
}

std::string PlayListItemFPPEvent::GetTooltip()
{
    return "";
}

void PlayListItemFPPEvent::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

        // Open the socket
        wxIPV4address localaddr;
        if (IPOutput::GetLocalIP() == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(IPOutput::GetLocalIP());
        }

        wxDatagramSocket* socket = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT | wxSOCKET_BROADCAST);
        if (socket == nullptr)
        {
            logger_base.error("Error opening datagram for FPP Event send.");
        }
        else if (!socket->IsOk())
        {
            logger_base.error("Error opening datagram for FPP Event send. OK : FALSE");
            delete socket;
            socket = nullptr;
        }
        else if (socket->Error())
        {
            logger_base.error("Error opening datagram for FPP Event send. %d : %s", socket->LastError(), (const char*)IPOutput::DecodeError(socket->LastError()).c_str());
            delete socket;
            socket = nullptr;
        }
        else
        {
            logger_base.info("FPP Event send datagram opened successfully.");
        }

        if (socket != nullptr)
        {
            wxIPV4address remoteAddr;
            //remoteAddr.BroadcastAddress();
            remoteAddr.Hostname("255.255.255.255");
            remoteAddr.Service(FPP_CTRL_PORT);

            wxASSERT(sizeof(ControlPkt) == 7); // ensure data is packed correctly

            std::string eventstring = GetEventString();
            int bufsize = sizeof(ControlPkt) + eventstring.size() + 1;
            unsigned char* buffer = (unsigned char*)malloc(bufsize);
            memset(buffer, 0x00, bufsize);

            if (buffer != nullptr)
            {
                ControlPkt* cp = (ControlPkt*)buffer;
                strncpy(cp->fppd, "FPPD", 4);
                cp->pktType = CTRL_PKT_EVENT;
                cp->extraDataLen = bufsize - sizeof(ControlPkt);
                strcpy((char*)(buffer + sizeof(ControlPkt)), eventstring.c_str());

                socket->SendTo(remoteAddr, buffer, bufsize);
                logger_base.info("FPP Event sent %s.", (const char *)eventstring.c_str());

                free(buffer);
            }

            logger_base.info("FPP Event send datagram closed.");
            socket->Close();
            delete socket;
        }
    }
}

void PlayListItemFPPEvent::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
