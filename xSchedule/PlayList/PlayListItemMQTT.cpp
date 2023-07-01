/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/socket.h>

#include "PlayListItemMQTT.h"
#include "PlayListItemMQTTPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

class MQTTThread : public wxThread
{
    std::string _brokerIP;
    int _port;
    std::string _username;
    std::string _password;
    std::string _clientId;
    std::string _topic;
    std::string _data;

public:
    MQTTThread(const std::string& brokerIP, int port, const std::string& clientId, const std::string& username, const std::string& password, const std::string& topic, const std::string& data) :
        _brokerIP(brokerIP), _port(port), _clientId(clientId), _username(username), _password(password), _topic(topic), _data(data) { }

    virtual void* Entry() override
    {
        log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("PlayListMQTT in thread.");

        wxSocketClient client(wxSOCKET_BLOCK);
        wxIPV4address addr;
        addr.Hostname(_brokerIP);
        addr.Service(_port);

        logger_base.debug("PlayListMQTT Connecting to broker %s:%d.", (const char*)_brokerIP.c_str(), _port);
        if (client.Connect(addr, false) || client.WaitOnConnect(0, 500))
        {
            logger_base.debug("    PlayListMQTT Connected to MQTT Broker.");
            uint8_t buffer[1444];
            memset(buffer, 0x00, sizeof(buffer));
            int index = 0;
            buffer[index++] = 0x10; // connect to publish
            // packet size
            index += PlayListItemMQTT::EncodeInt(&buffer[index], 0);
            index += PlayListItemMQTT::EncodeString(&buffer[index], "MQTT");
            buffer[index++] = 0x04; // protocol version
            uint8_t flags = 0x00;
            // I only think I need to handle where user and password are present
            if (_username != "" && _password != "")
            {
                flags |= 0x80;
                flags |= 0x40;
            }
            buffer[index++] = flags;
            buffer[index++] = 30; // keep alive
            buffer[index++] = 0x00;
            index += PlayListItemMQTT::EncodeString(&buffer[index], _clientId);
            if (_username != "" && _password != "")
            {
                index += PlayListItemMQTT::EncodeString(&buffer[index], _username);
                index += PlayListItemMQTT::EncodeString(&buffer[index], _password);
            }
            PlayListItemMQTT::EncodeInt(&buffer[1], index - 2); // set the packet length

            logger_base.debug("    PlayListMQTT Sending connect packet.");
            client.Write(buffer, index);
            wxASSERT(client.LastWriteCount() == index);
            memset(buffer, 0x00, sizeof(buffer));
            client.WaitForRead(0, 500);
            client.Read(buffer, std::min((int)client.LastCount(), (int)sizeof(buffer)));
            if (client.GetLastIOReadSize() > 0)
            {
                logger_base.debug("    PlayListMQTT Response received.");
                if ((buffer[0] & 0xF0) >> 4 == 2)
                {
                    logger_base.debug("    PlayListMQTT Connected ok.");
                    int used = 0;
                    unsigned char* pdata = PlayListItemMQTT::PrepareData(_data, used);

                    memset(buffer, 0x00, sizeof(buffer));

                    int index = 0;
                    buffer[index++] = 0x30; // publish
                    if (2 + _topic.size() + used > sizeof(buffer))
                    {
                        used = 1500 - 2 - _topic.size();
                    }
                    index += PlayListItemMQTT::EncodeInt(&buffer[index], 2 + _topic.size() + used);
                    index += PlayListItemMQTT::EncodeString(&buffer[index], _topic);
                    memcpy(&buffer[index], pdata, used);
                    index += used;
                    client.Write(buffer, index);
                    wxASSERT(client.LastWriteCount() == index);
                    logger_base.info("PlayListMQTT MQTT Sent.");
                    client.WaitForRead(0, 500);
                    client.Read(buffer, std::min((int)client.LastCount(), (int)sizeof(buffer)));
                }
                else
                {
                    logger_base.error("PlayListMQTT Illegal response from MQTT broker for connect.");
                }
            }
            else
            {
                logger_base.error("PlayListMQTT No response from MQTT broker for connect.");
            }
            logger_base.debug("    PlayListMQTT Disconnecting from MQTT Broker.");
            client.Close();
        }
        else
        {
            logger_base.error("PlayListMQTT Unable to connect to MQTT broker.");
        }

        logger_base.debug("PlayListMQTT thread done");

        return nullptr;
    }
};

PlayListItemMQTT::PlayListItemMQTT(wxXmlNode* node) : PlayListItem(node)
{
    PlayListItemMQTT::Load(node);
}

void PlayListItemMQTT::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _brokerIP = node->GetAttribute("IP", "");
    _topic = node->GetAttribute("Topic", "");
    _port = wxAtoi(node->GetAttribute("Port", ""));
    _data = UnXmlSafe(node->GetAttribute("Data", ""));
    _username = node->GetAttribute("Username", "");
    _password = node->GetAttribute("Password", "");
    _clientId = node->GetAttribute("ClientId", "");
    if (_clientId == "") SetDefaultClientId();
}

PlayListItemMQTT::PlayListItemMQTT() : PlayListItem()
{
    _type = "PLIMQTT";
}

PlayListItem* PlayListItemMQTT::Copy(const bool isClone) const
{
    PlayListItemMQTT* res = new PlayListItemMQTT();

	res->_brokerIP = _brokerIP;
    res->_topic = _topic;
    res->_data = _data;
    res->_port = _port;
    res->_username = _username;
    res->_password = _password;
    res->_clientId = _clientId;
    res->_started = false;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemMQTT::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Topic", _topic);
    node->AddAttribute("IP", _brokerIP);
    node->AddAttribute("Data", XmlSafe(_data));
    node->AddAttribute("Username", _username);
    node->AddAttribute("Password", _password);
    node->AddAttribute("ClientId", _clientId);
    node->AddAttribute("Port", wxString::Format("%d", _port));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemMQTT::GetTitle() const
{
    return "MQTT";
}

void PlayListItemMQTT::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemMQTTPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemMQTT::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "MQTT";
}

std::string PlayListItemMQTT::GetTooltip()
{
    return "Use \\xAA to enter binary values where AA is a hexadecimal value.\n\n" + GetTagHint();
}

unsigned char* PlayListItemMQTT::PrepareData(const std::string& s, int& used)
{
    wxString working = s;

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
                    ++i; // skip the second '\\'
                }
                if (working[i + 1] == 'x' || working[i + 1] == 'X')
                {
                    // up to next 2 characters if 0-F will be treated as a hex code
                    ++i;
                    ++i;
                    if (i + 1 < working.size() && isHexChar(working[i]) && isHexChar(working[i + 1]))
                    {
                        buffer[used++] = (char)HexToChar(working[i], working[i + 1]);
                        ++i;
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
                        --i;
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

int PlayListItemMQTT::DecodeInt(uint8_t* pb, int& oldindex)
{
    int index = 0;
    int multiplier = 1;
    int value = 0;
    uint8_t enc = 0;

    do
    {
        enc = pb[index++];
        value += (enc & 127) * multiplier;
        multiplier *= 128;
        if (multiplier > 128 * 128 * 128)
        {
            wxASSERT(false);
        }
    } while ((enc & 128) != 0);

    oldindex += index;

    return value;
}

std::string PlayListItemMQTT::DecodeString(uint8_t* pb, int& oldindex)
{
    std::string value = "";
    uint8_t enc = 0;

    int stringlen = ((int)pb[0] << 8) + pb[1];
    int index = 2;

    for (int i = 0; i < stringlen; i++)
    {
        value += pb[index++];
    }

    oldindex += index;
    return value;
}

int PlayListItemMQTT::EncodeInt(uint8_t* pb, int value)
{
    int index = 0;
    do {
        uint8_t enc = value % 128;
        value = value / 128;
        if (value > 0)
        {
            enc = enc | 128;
        }
        pb[index++] = enc;
    } while (value > 0);
    return index;
}

int PlayListItemMQTT::EncodeString(uint8_t* pb, const std::string& str)
{
    int index = 0;
    pb[index++] = (str.size() & 0xFF00) >> 8;
    pb[index++] = str.size() & 0xFF;
    for (auto it : str)
    {
        pb[index++] = it;
    }
    return index;
}

void PlayListItemMQTT::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (ms >= _delay && !_started)
    {
        _started = true;

        std::string topic = ReplaceTags(_topic);
        wxString working = ReplaceTags(_data);

        logger_base.info("Sending MQTT Event to %s:%d %s", (const char*)_brokerIP.c_str(), _port, (const char*)_topic.c_str());

        MQTTThread* thread = new MQTTThread(_brokerIP, _port, _clientId, _username, _password, topic, working);
        thread->Run();
        wxMicroSleep(1); // encourage the thread to run
    }
}

void PlayListItemMQTT::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
    _started = false;
}

void PlayListItemMQTT::SetClientId(const std::string& clientId)
{
    if (clientId == "") {
        ++_changeCount;
        SetDefaultClientId();
    }
    else
    {
        if (_clientId != clientId) { 
            _clientId = clientId; 
            ++_changeCount; 
        } 
    }
}