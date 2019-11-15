#include "ListenerMQTT.h"
#include "ListenerManager.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../../xLights/UtilFunctions.h"
#include "../PlayList/PlayListItemMQTT.h"

#include <wx/socket.h>

#include <log4cpp/Category.hh>

ListenerMQTT::ListenerMQTT(ListenerManager* listenerManager, const std::string& ip, int port, const std::string& username, const std::string& password, const std::string& clientId) : ListenerBase(listenerManager)
{
    _ip = ip;
    _port = port;
    _username = username;
    _password = password;
    _clientId = clientId;
}

void ListenerMQTT::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("MQTT listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerMQTT::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop)
    {
        logger_base.debug("MQTT listener stopping.");
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

void ListenerMQTT::StartProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _isOk = false;
    wxIPV4address addr;
    addr.Hostname(_ip);
    addr.Service(_port);

    if (_client.Connect(addr, false) || _client.WaitOnConnect(0, 500))
    {
        uint8_t buffer[1444];
        memset(buffer, 0x00, sizeof(buffer));
        int index = 0;
        buffer[index++] = 0x10; // connect to publish
        index += PlayListItemMQTT::EncodeInt(&buffer[index], 0); // packer length
        index += PlayListItemMQTT::EncodeString(&buffer[index], "MQTT");
        buffer[index++] = 0x04; // protocol version
        uint8_t flags = 0x00;
        if (_username != "" && _password != "")
        {
            flags |= 0x80;
            flags |= 0x40;
        }
        buffer[index++] = flags;
        buffer[index++] = 0x00; // keep alive
        buffer[index++] = 0x00;
        index += PlayListItemMQTT::EncodeString(&buffer[index], _clientId);
        if (_username != "" && _password != "")
        {
            index += PlayListItemMQTT::EncodeString(&buffer[index], _username);
            index += PlayListItemMQTT::EncodeString(&buffer[index], _password);
        }
        PlayListItemMQTT::EncodeInt(&buffer[1], index - 2); // set the packet length

        _client.Write(buffer, index);
        wxASSERT(_client.LastWriteCount() == index);
        memset(buffer, 0x00, sizeof(buffer));
        _client.WaitForRead(0, 500);

        _client.Peek(buffer, sizeof(buffer));

        // wait for up to 100ms for data
        int i = 0;
        while (_client.LastCount() == 0 && i < 100)
        {
            wxMilliSleep(1);
            _client.Peek(buffer, sizeof(buffer));
            i++;
        }

        _client.Read(buffer, std::min((int)_client.LastCount(), (int)sizeof(buffer)));
        if (_client.GetLastIOReadSize() > 0)
        {
            if ((buffer[0] & 0xF0) >> 4 == 2)
            {
                _isOk = true;
            }
        }
    }
}

bool ListenerMQTT::Subscribe(const std::string& topic)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static int __msgid = 1;

    if (!_isOk)
    {
        std::unique_lock<std::mutex> lock(_topicLock);

        if (std::find(begin(_toSubscribe), end(_toSubscribe), topic) == end(_toSubscribe))
        {
            _toSubscribe.push_back(topic);
        }
        return false;
    }

    uint8_t buffer[1444];
    memset(buffer, 0x00, sizeof(buffer));
    int index = 0;

    // now i need to subscribe
    memset(buffer, 0x00, sizeof(buffer));
    buffer[index++] = 0x82; // subscribe
    index += PlayListItemMQTT::EncodeInt(&buffer[index], 5 + topic.length());
    buffer[index++] = (__msgid & 0xFF00) >> 8;
    buffer[index++] = (__msgid & 0xFF);
    __msgid++;
    index += PlayListItemMQTT::EncodeString(&buffer[index], topic);
    buffer[index++] = 0x00;

    logger_base.info("MQTT subscribing to topic %s.", (const char*)topic.c_str());
    _client.Write(buffer, index);
    wxASSERT(_client.LastWriteCount() == index);

    return true;
}

void ListenerMQTT::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_client.IsConnected()) {
        logger_base.info("MQTT Listener closed.");
        _client.Close();
    }
    _isOk = false;
}

void ListenerMQTT::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_isOk)
    {
        unsigned char buffer[1500];
        memset(buffer, 0x00, sizeof(buffer));

        _client.WaitForRead(0, 500);
        if (_stop) return;
        _client.Peek(buffer, sizeof(buffer));

        if (_client.LastCount() > 0)
        {
            //wxStopWatch sw;
            //logger_base.debug("Trying to read MQTT packet.");
            _client.Read(buffer, std::min((int)_client.LastCount(), (int)sizeof(buffer)));
            //logger_base.debug(" Read done. %ldms", sw.Time());

            if (_stop) return;

            if (_client.GetLastIOReadSize() != 0)
            {
                if ((buffer[0] & 0xF0) >> 4 == 3)
                {
                    int index = 1;
                    int pktsize = PlayListItemMQTT::DecodeInt(&buffer[index], index);
                    std::string topic = PlayListItemMQTT::DecodeString(&buffer[index], index);

                    logger_base.debug("MQTT Topic: %s.", (const char*)topic.c_str());

                    std::string data = "";
                    for (int i = 0; i < pktsize - topic.length() - 2; i++)
                    {
                        data += buffer[index++];
                    }
                    _listenerManager->ProcessPacket(GetType(), topic, data);
                }
                else
                {
                    logger_base.error("Unexpected MQTT Packet: %d.", (buffer[0] & 0xF0) >> 4);
                }
            }
            else
            {
                logger_base.error("Invalid MQTT Packet.");
            }
        }

        {
            std::unique_lock<std::mutex> lock(_topicLock);

            int lastSize = 0;
            while (_toSubscribe.size() != 0 && _toSubscribe.size() != lastSize)
            {
                lastSize = _toSubscribe.size();
                if (Subscribe(_toSubscribe.front()))
                {
                    _toSubscribe.pop_front();
                }
            }
        }
    }
}
