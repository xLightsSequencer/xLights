/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "WebSocketClient.h"
#include <log4cpp/Category.hh>

WebSocketClient::WebSocketClient()
{
    _connected = false;
}

bool WebSocketClient::Connect(std::string ip, std::string url)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Connecting to websocket %s %s.", (const char *)ip.c_str(), (const char *)url.c_str());

    wxIPV4address addr;
    addr.Hostname(ip);
    addr.Service(80);
    _socket.Connect(addr, false);
    _socket.WaitOnConnect(10);

    if (!_socket.IsConnected())
    {
        logger_base.error("    Failed to connect.");
        return false;
    }

    _connected = true;

    logger_base.debug("    Connected.");

    if (url == "") url = "/";
    wxString line = wxString::Format("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\nSec-WebSocket-Version: 13\r\nOrigin:http://%s/\r\n\r\n", url, ip, ip);
    _socket.Write(line.c_str(), line.Length());
    wxMilliSleep(500);
    wxString answer = ReadSocket();

    if (answer.StartsWith("HTTP/1.1 ") && wxAtoi(answer.substr(9)) == 101)
    {
        logger_base.debug("    Converted to websocket.");

        ClearIncomingData();

        return true;
    }

    logger_base.error("     Failed to convert to web socket %d.", wxAtoi(answer.substr(9)));

    _socket.Close();
    _connected = false;

    return false;
}

bool WebSocketClient::Send(std::string message)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("WebSocket Sent: %s", (const char *)message.c_str());

    bool useMask = false;
    const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
    std::vector<uint8_t> header;
    header.assign(2 + (message.size() >= 126 ? 2 : 0) + (message.size() >= 65536 ? 6 : 0) + (useMask ? 4 : 0), 0);
    header[0] = 0x80 | 0x01;
    if (false) {}
    else if (message.size() < 126) {
        header[1] = (message.size() & 0xff) | (useMask ? 0x80 : 0);
        if (useMask) {
            header[2] = masking_key[0];
            header[3] = masking_key[1];
            header[4] = masking_key[2];
            header[5] = masking_key[3];
        }
    }
    else if (message.size() < 65536) {
        header[1] = 126 | (useMask ? 0x80 : 0);
        header[2] = (message.size() >> 8) & 0xff;
        header[3] = (message.size() >> 0) & 0xff;
        if (useMask) {
            header[4] = masking_key[0];
            header[5] = masking_key[1];
            header[6] = masking_key[2];
            header[7] = masking_key[3];
        }
    }
    else { // TODO: run coverage testing here
        header[1] = 127 | (useMask ? 0x80 : 0);
        header[2] = 0; // (message.size() >> 56) & 0xff;
        header[3] = 0; // (message.size() >> 48) & 0xff;
        header[4] = 0; // (message.size() >> 40) & 0xff;
        header[5] = 0; // (message.size() >> 32) & 0xff;
        header[6] = (message.size() >> 24) & 0xff;
        header[7] = (message.size() >> 16) & 0xff;
        header[8] = (message.size() >> 8) & 0xff;
        header[9] = (message.size() >> 0) & 0xff;
        if (useMask) {
            header[10] = masking_key[0];
            header[11] = masking_key[1];
            header[12] = masking_key[2];
            header[13] = masking_key[3];
        }
    }
    for (size_t i = 0; i < message.size(); i++)
    {
        header.push_back(message[i]);
    }
    _socket.Write(&header[0], header.size());
    return true;
}

std::string WebSocketClient::Receive()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    unsigned char buffer[4096];
    _socket.Peek(buffer, sizeof(buffer));
    auto read = _socket.LastCount();
    if (read >= 2)
    {
        bool fin = (buffer[0] & 0x80) == 0x80;
        char opcode = (buffer[0] & 0x0f);
        char mask = (buffer[1] & 0x80) == 0x80;
        char N0 = (buffer[1] & 0x7f);
        int header_size = 2 + (N0 == 126 ? 2 : 0) + (N0 == 127 ? 8 : 0) + (mask ? 4 : 0);

        if (read >= header_size)
        {
            char masking_key[4];
            int i = 0;
            unsigned long N = 0;
            if (N0 < 126) {
                N = N0;
                i = 2;
            }
            else if (N0 == 126) {
                N = 0;
                N += ((uint64_t)buffer[2]) << 8;
                N += ((uint64_t)buffer[3]);
                i = 4;
            }
            else if (N0 == 127) {
                N = 0;
                N += ((uint64_t)buffer[2]) << 56;
                N += ((uint64_t)buffer[3]) << 48;
                N += ((uint64_t)buffer[4]) << 40;
                N += ((uint64_t)buffer[5]) << 32;
                N += ((uint64_t)buffer[6]) << 24;
                N += ((uint64_t)buffer[7]) << 16;
                N += ((uint64_t)buffer[8]) << 8;
                N += ((uint64_t)buffer[9]);
                i = 10;
            }
            if (mask) {
                masking_key[0] = ((uint8_t)buffer[i + 0]) << 0;
                masking_key[1] = ((uint8_t)buffer[i + 1]) << 0;
                masking_key[2] = ((uint8_t)buffer[i + 2]) << 0;
                masking_key[3] = ((uint8_t)buffer[i + 3]) << 0;
            }
            else {
                masking_key[0] = 0;
                masking_key[1] = 0;
                masking_key[2] = 0;
                masking_key[3] = 0;
            }
            if (read < header_size + N || opcode != 0x01)
            {
                return "";
            }

            // now really read it
            _socket.Read(buffer, header_size + N);

            if (mask)
            {
                for (size_t j = 0; j != N; ++j)
                {
                    buffer[j + header_size] ^= masking_key[j & 0x3];
                }
            }

            buffer[N + header_size] = 0x00;

            wxASSERT(fin != 0);

        std::string res = std::string((char*)&buffer[header_size]);
        logger_base.debug("WebSocket Received: %s", (const char *)res.c_str());
        return res;
        }
    }

    return "";
}

std::string WebSocketClient::ReadSocket()
{
    unsigned char buffer[4096];
    memset(buffer, 0x00, sizeof(buffer));
    for (int i = 0; i < 2 || (i < 255 && buffer[i - 2] != '\r' && buffer[i - 1] != '\n'); ++i)
    {
        _socket.Read(&buffer[i], 1);
        if (_socket.LastReadCount() == 0) return "";
        buffer[i+1] = 0;
    }

    return std::string((char*)buffer);
}

void WebSocketClient::ClearIncomingData()
{
    int to = _socket.GetTimeout();
    _socket.SetTimeout(1);
    unsigned char buffer[4096];
    int read = 999;
    while (read != 0)
    {
        _socket.Read(buffer, sizeof(buffer));
        read = _socket.LastReadCount();
    }
    _socket.SetTimeout(to);
}

WebSocketClient::~WebSocketClient()
{
    _socket.Close();
    _connected = false;
}
