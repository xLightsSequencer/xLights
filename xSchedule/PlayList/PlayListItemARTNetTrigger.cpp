/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemARTNetTrigger.h"
#include "PlayListItemARTNetTriggerPanel.h"
#include "PlayList.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "../xLights/outputs/IPOutput.h"
#include "../xLights/outputs/ArtNetOutput.h"
#include "../Control.h"
#include <wx/protocol/http.h>
#include "../../xLights/UtilFunctions.h"

PlayListItemARTNetTrigger::PlayListItemARTNetTrigger(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _oem = 0xFFFF;
    _key = 1;
    _subkey = 1;
    _data = "";
    _ip = "";
    PlayListItemARTNetTrigger::Load(node);
}

void PlayListItemARTNetTrigger::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _oem = wxAtoi(node->GetAttribute("OEM", "65535"));
    _key = wxAtoi(node->GetAttribute("Key", "1"));
    _subkey = wxAtoi(node->GetAttribute("SubKey", "1"));
    _data = node->GetAttribute("Data", "");
    _ip = node->GetAttribute("IP", "");
}

PlayListItemARTNetTrigger::PlayListItemARTNetTrigger() : PlayListItem()
{
    _type = "PLIARTNetTrigger";
    _started = false;
    _oem = 0xFFFF;
    _key = 1;
    _subkey = 1;
    _data = "";
    _ip = "";
}

PlayListItem* PlayListItemARTNetTrigger::Copy() const
{
    PlayListItemARTNetTrigger* res = new PlayListItemARTNetTrigger();
    res->_oem = _oem;
    res->_key = _key;
    res->_subkey = _subkey;
    res->_ip = _ip;
    res->_data = _data;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemARTNetTrigger::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("OEM", wxString::Format("%d", _oem));
    node->AddAttribute("Key", wxString::Format("%d", _key));
    node->AddAttribute("SubKey", wxString::Format("%d", _subkey));
    node->AddAttribute("IP", _ip);
    node->AddAttribute("Data", _data);

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemARTNetTrigger::GetTitle() const
{
    return "ARTNet Trigger";
}

void PlayListItemARTNetTrigger::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemARTNetTriggerPanel(notebook, this), GetTitle(), true);
}

unsigned char* PlayListItemARTNetTrigger::PrepareData(const std::string s, int& used)
{
    wxString working = ReplaceTags(s);

    unsigned char* buffer = (unsigned char*)malloc(working.size());
    used = 0;

    for (int i = 0; i < working.size(); i++)
    {
        if (working[i] == '\\')
        {
            if (i + 1 < working.size())
            {
                if (working[i + 1] == '\\')
                {
                    buffer[used++] = working[i];
                    i++; // skip the second '\\'
                }
                if (working[i + 1] == 'x' || working[i + 1] == 'X')
                {
                    // up to next 2 characters if 0-F will be treated as a hex code
                    i++;
                    i++;
                    if (i + 1 < working.size() && isHexChar(working[i]) && isHexChar(working[i + 1]))
                    {
                        buffer[used++] = (char)HexToChar(working[i], working[i + 1]);
                        i++;
                    }
                    else if (i < working.size() && isHexChar(working[i]))
                    {
                        buffer[used++] = (char)HexToChar(working[i]);
                    }
                    else
                    {
                        // \x was not followed by a hex digit so put in \x
                        buffer[used++] = '\\';
                        buffer[used++] = 'x';
                        i--;
                    }
                }
            }
            else
            {
                buffer[used++] = working[i];
            }
        }
        else
        {
            buffer[used++] = working[i];
        }
    }

    unsigned char* res = (unsigned char*)malloc(used);
    memcpy(res, buffer, used);
    free(buffer);
    return res;
}

std::string PlayListItemARTNetTrigger::GetNameNoTime() const
{
    if (_name != "") return _name;

    return wxString::Format("%d:%d:%d", _oem, _key, _subkey);
}

std::string PlayListItemARTNetTrigger::GetTooltip()
{
    return "Use \\xAA to enter binary values where AA is a hexadecimal value.\n\n" + GetTagHint();
}

void PlayListItemARTNetTrigger::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

        int dbuffsize;
        unsigned char* dbuffer = PrepareData(_data, dbuffsize);

        if (dbuffer != nullptr)
        {
            uint8_t pkt[18 + 512];
            memset(pkt, 0x00, sizeof(pkt));

            pkt[0] = 'A';
            pkt[1] = 'r';
            pkt[2] = 't';
            pkt[3] = '-';
            pkt[4] = 'N';
            pkt[5] = 'e';
            pkt[6] = 't';

            pkt[8] = 0x00;
            pkt[9] = 0x99;

            pkt[10] = 0x00;
            pkt[11] = 14;

            pkt[14] = (_oem >> 8) & 0xFF;
            pkt[15] = _oem & 0xFF;

            pkt[16] = _key & 0xFF;
            pkt[17] = _subkey & 0xFF;
            memcpy(&pkt[18], dbuffer, std::min(512, dbuffsize));
            delete dbuffer;

            bool ok = true;

            wxIPV4address localaddr;
            if (IPOutput::__localIP == "")
            {
                localaddr.AnyAddress();
            }
            else
            {
                localaddr.Hostname(IPOutput::__localIP);
            }

            wxDatagramSocket* datagram = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);
            if (datagram == nullptr)
            {
                logger_base.error("Error initialising Artnet trigger datagram for %s. %s", (const char*)_ip.c_str(), (const char*)localaddr.IPAddress().c_str());
                ok = false;
            }
            else if (!datagram->IsOk())
            {
                logger_base.error("Error initialising Artnet trigger datagram for %s. %s OK : FALSE", (const char*)_ip.c_str(), (const char*)localaddr.IPAddress().c_str());
                delete datagram;
                datagram = nullptr;
                ok = false;
            }
            else if (datagram->Error())
            {
                logger_base.error("Error creating Artnet trigger datagram => %d : %s. %s", datagram->LastError(), (const char*)DecodeIPError(datagram->LastError()).c_str(), (const char*)localaddr.IPAddress().c_str());
                delete datagram;
                datagram = nullptr;
                ok = false;
            }

            if (ok)
            {
                wxIPV4address remoteAddr;
                remoteAddr.Hostname(_ip.c_str());
                remoteAddr.Service(ARTNET_PORT);

                datagram->SendTo(remoteAddr, pkt, sizeof(pkt));

                datagram->Close();
                delete datagram;
            }
        }
    }
}

void PlayListItemARTNetTrigger::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}