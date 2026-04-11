
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xxxEthernetOutput.h"
#include <cstring>

#include <cassert>

#include "OutputManager.h"
#include "UtilFunctions.h"
#include "../utils/ip_utils.h"

#include <spdlog/fmt/fmt.h>

#include <log.h>

static const int32_t xxxCHANNELSPERPACKET = 1200;

#pragma region Private Functions
void xxxEthernetOutput::Heartbeat(int mode, const std::string& localIP) {

    

    static int64_t __lastTime = 0;
    static std::string __remoteIp;
    static uint8_t __pkt[] = { 0x80, 0x01, 0x00, 0x81 };
    static sockets::UDPSocket* __datagram = nullptr;

    if (mode == 1) {
        // output
        if (__datagram == nullptr) return;

        int64_t now = GetCurrentTimeMillis();
        if (__lastTime + xxx_HEARTBEATINTERVAL < now) {
            __datagram->SendTo(__remoteIp, xxx_PORT, __pkt, sizeof(__pkt));
            __lastTime = now;
        }
    }
    else if (mode == 0) {
        // initialise
        if (__datagram != nullptr) return;

        __remoteIp = "224.0.0.0";

        __datagram = new sockets::UDPSocket();

        if (__datagram != nullptr) {
            if (!__datagram->Bind(localIP, 0, false)) {
                spdlog::error("xxxEthernetOutput: Error creating xxxEthernet heartbeat datagram. {}",
                    __datagram->LastError());
                delete __datagram;
                __datagram = nullptr;
            }
        }
        else {
            spdlog::error("xxxEthernetOutput: Error creating xxxEthernet heartbeat datagram.");
        }
    }
    else if (mode == 9) {
        // close
        if (__datagram == nullptr) return;
        __datagram->Close();
        delete __datagram;
        __datagram = nullptr;
    }
}

void xxxEthernetOutput::OpenDatagram() {

    

    if (_datagram != nullptr) return;

    _datagram = new sockets::UDPSocket();
    if (_datagram == nullptr) {
        spdlog::error("xxxEthernetOutput: Error opening datagram.");
    }
    else if (!_datagram->Bind(GetForceLocalIPToUse(), 0, false)) {
        spdlog::error("xxxEthernetOutput: Error opening datagram. {}", _datagram->LastError());
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
xxxEthernetOutput::xxxEthernetOutput(pugi::xml_node node, bool isActive) : IPOutput(node, isActive) {

    SetId(node.attribute("Id").as_int(0));
    if (node.attribute("Port").as_int(-1) != -1)
    {
        _universe = node.attribute("Port").as_int(0);
    }
    _data = (uint8_t*)malloc(_channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
    memset(_data, 0, _channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
}

xxxEthernetOutput::xxxEthernetOutput() : IPOutput() {

    _universe = 1;
    _data = (uint8_t*)malloc(_channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
    memset(_data, 0, _channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
}

xxxEthernetOutput::xxxEthernetOutput(const xxxEthernetOutput& from) : IPOutput(from) {

    _channels = from._channels;
    _universe = from._universe;
    _data = (uint8_t*)malloc(_channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
    memset(_data, 0, _channels + xxxETHERNET_PACKET_HEADERLEN + xxxETHERNET_PACKET_FOOTERLEN);
}

xxxEthernetOutput::~xxxEthernetOutput() {
    if (_data != nullptr) {
        free(_data);
        _data = nullptr;
    }
}

pugi::xml_node xxxEthernetOutput::Save(pugi::xml_node parent) {

    pugi::xml_node node = parent.append_child("network");

    node.append_attribute("Id") = GetId();

    IPOutput::SaveAttr(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
std::string xxxEthernetOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "xxxEthernet {" + std::to_string(_universe) + "} ";
    res += "[1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";

    return res;
}

std::string xxxEthernetOutput::GetExport() const {

    return fmt::format(",{},{},,{},{},,,,{},{}",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}
#pragma endregion

#pragma region Start and Stop
bool xxxEthernetOutput::Open() {

    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(GetResolvedIP())) return false;

    _ok = IPOutput::Open();

    memset(_packet, 0, sizeof(_packet));

    OpenDatagram();
    _remoteIp = GetResolvedIP();

    Heartbeat(0, GetForceLocalIPToUse());

    return _ok && _datagram != nullptr;
}

void xxxEthernetOutput::Close() {

    Heartbeat(9, GetForceLocalIPToUse());
    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion

#pragma region Frame Handling
void xxxEthernetOutput::StartFrame(long msec) {

    

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            spdlog::debug("xxxEthernetOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void xxxEthernetOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend) return;

        if (_datagram == nullptr) return;

        if (_changed || NeedToOutput(suppressFrames)) {
            int current = 0;
            for (int i = 0; i < (_channels - 1) / xxxCHANNELSPERPACKET + 1; i++) {
                _packet[0] = 0x80;
                _packet[1] = _universe;
                _packet[2] = 0x1d; // 0x1c
                _packet[3] = (uint8_t)(((current / 3) >> 8) & 0xFF); // high start pixel
                _packet[4] = (uint8_t)((current / 3) & 0xFF); // low start pixel
                int ch = (std::min)(_channels - current, xxxCHANNELSPERPACKET);
                _packet[5] = (uint8_t)((ch >> 8) & 0xFF); // high pixels per packet
                _packet[6] = (uint8_t)(ch & 0xFF); // low pixels per packet
                memcpy(&_packet[xxxETHERNET_PACKET_HEADERLEN], &_data[current], ch);
                _packet[xxxETHERNET_PACKET_HEADERLEN + ch] = 0x81;
                _datagram->SendTo(_remoteIp, xxx_PORT, _packet, xxxETHERNET_PACKET_HEADERLEN + ch + xxxETHERNET_PACKET_FOOTERLEN);
                current += xxxCHANNELSPERPACKET;
            }
            FrameOutput();
            Heartbeat(1, GetForceLocalIPToUse());
        }
        else {
            SkipFrame();
        }
}
#pragma endregion 

#pragma region Data Setting
void xxxEthernetOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;

    if (_data[channel] != data) {
        _data[channel] = data;
        _changed = true;
    }
}

void xxxEthernetOutput::SetManyChannels(int32_t channel, unsigned char data[], size_t size) {

    size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));
    if (memcmp(&_data[channel], data, chs) != 0) {
        memcpy(&_data[channel], data, chs);
        _changed = true;
    }
}

void xxxEthernetOutput::AllOff() {

    memset(_data, 0x00, _channels);
    _changed = true;
}
#pragma endregion 

