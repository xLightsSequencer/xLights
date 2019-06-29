#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/socket.h>

#include "PlayListItemMQTT.h"
#include "PlayListItemMQTTPanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>

PlayListItemMQTT::PlayListItemMQTT(wxXmlNode* node) : PlayListItem(node)
{
    _brokerIP = "127.0.0.1";
	_topic = "";
	_port = 1883;
    _started = false;
    _data = "";
    PlayListItemMQTT::Load(node);
}

void PlayListItemMQTT::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _brokerIP = node->GetAttribute("IP", "");
    _topic = node->GetAttribute("Topic", "");
    _port = wxAtoi(node->GetAttribute("Port", ""));
    _data = node->GetAttribute("Data", "");
}

PlayListItemMQTT::PlayListItemMQTT() : PlayListItem()
{
    _type = "PLIMQTT";
	_brokerIP = "127.0.0.1";
	_topic = "";
    _data = "";
	_port = 1883;
    _started = false;
}

PlayListItem* PlayListItemMQTT::Copy() const
{
    PlayListItemMQTT* res = new PlayListItemMQTT();

	res->_brokerIP = _brokerIP;
    res->_topic = _topic;
    res->_data = _data;
    res->_port = _port;
    res->_started = false;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemMQTT::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Topic", _topic);
    node->AddAttribute("IP", _brokerIP);
    node->AddAttribute("Data", _data);
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

unsigned char* PlayListItemMQTT::PrepareData(const std::string s, int& used)
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

int DecodeInt(uint8_t* pb)
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

    return value;
}

int EncodeInt(uint8_t* pb, int value)
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

int EncodeString(uint8_t* pb, const std::string str)
{
    int index = 0;
    pb[index++] = 0x00;
    index += EncodeInt(&pb[index], str.size());
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

        logger_base.info("Sending MQTT Event to %s:%d %s", (const char *)_brokerIP.c_str(), _port, (const char *)_topic.c_str());

        wxSocketClient client;
        wxIPV4address addr;
        addr.Hostname(_brokerIP);
        addr.Service(_port);

        if (client.Connect(addr, false) || client.WaitOnConnect(0, 500))
        {
            uint8_t buffer[1444];
            memset(buffer, 0x00, sizeof(buffer));
            int index = 0;
            buffer[index++] = 0x10; // connect to publish
            index += EncodeInt(&buffer[index], 21);
            index += EncodeString(&buffer[index], "MQTT");
            buffer[index++] = 0x04; // protocol version
            uint8_t flags = 0x00;
            if (_username != "") flags |= 0x80;
            if (_password != "") flags |= 0x40;
            buffer[index++] = flags;
            buffer[index++] = 0x00; // keep alive
            buffer[index++] = 0x00;
            index += EncodeString(&buffer[index], "xSchedule");

            client.Write(buffer, index);
            wxASSERT(client.LastWriteCount() == index);
            memset(buffer, 0x00, sizeof(buffer));
            client.WaitForRead(0, 500);

            client.Peek(buffer, sizeof(buffer));

            // wait for up to 100ms for data
            int i = 0;
            while (client.LastCount() == 0 && i < 100)
            {
                wxMilliSleep(1);
                client.Peek(buffer, sizeof(buffer));
                i++;
            }

            client.Read(buffer, std::min((int)client.LastCount(), (int)sizeof(buffer)));
            if (client.GetLastIOReadSize() > 0)
            {
                if ((buffer[0] & 0xF0) >> 4 == 2)
                {
                    std::string topic = ReplaceTags(_topic);
                    int used = 0;
                    unsigned char* pdata = PrepareData(_data, used);

                    memset(buffer, 0x00, sizeof(buffer));

                    int index = 0;
                    buffer[index++] = 0x30; // publish
                    if (2 + topic.size() + used > sizeof(buffer))
                    {
                        used = 1500 - 2 - topic.size();
                    }
                    index += EncodeInt(&buffer[index], 2 + topic.size() + used);
                    index += EncodeString(&buffer[index], topic);
                    memcpy(&buffer[index], pdata, used);
                    index += used;
                    client.Write(buffer, index);
                    wxASSERT(client.LastWriteCount() == index);
                    logger_base.info("MQTT Sent.");
                }
                else
                {
                    logger_base.error("Illegal response from MQTT broker for connect.");
                }
            }
            else
            {
                logger_base.error("No response from MQTT broker for connect.");
            }
            client.Close();
        }
        else
        {
            logger_base.error("Unable to connect to MQTT broker.");
        }
    }
}

void PlayListItemMQTT::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);
    _started = false;
}
