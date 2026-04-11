
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OPCOutput.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "OutputManager.h"
#include "../utils/ip_utils.h"

#include <log.h>

#pragma region Private Functions
void OPCOutput::OpenSocket() {

    

    if (_socket != nullptr) return;

    _socket = new sockets::TCPSocket();
    if (_socket == nullptr) {
        spdlog::error("OPCOutput: Error opening socket object to connect to {}.", _remoteIp);
    }
    else
    {
        const std::string localIp = GetForceLocalIPToUse();
        if (!_socket->Connect(_remoteIp, OPC_PORT, localIp, false)) {
            spdlog::error("OPCOutput: {} Error connecting OPC socket: {}", _remoteIp, _socket->LastError());
            delete _socket;
            _socket = nullptr;
        }
        else
        {
            spdlog::debug("OPCOutput: OPC socket connected to {}.", _remoteIp);
            // The pixels sent is timing sensitive, TCP_NODELAY disables
            // Nagle algorithm to delay sending out TCP packets to combine
            // with later data.  This needs to not delay or combine writes.
            if (!_socket->SetNoDelay(true)) {
                spdlog::error("OPCOutput: failed to set TCP_NODELAY");
            }
        }
    }
}
#pragma endregion

#pragma region Constructors and Destructors
OPCOutput::OPCOutput(pugi::xml_node node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > GetMaxChannels()) SetChannels(GetMaxChannels());
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::OPCOutput() : IPOutput() {

    _channels = 510;
    _universe = 1;
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::OPCOutput(const OPCOutput& from) :
    IPOutput(from)
{
    _channels = from._channels;
    _universe = from._universe;
    _socket = nullptr;
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

OPCOutput::~OPCOutput()
{
    if (_socket != nullptr) delete _socket;
    if (_data != nullptr) free(_data);
}

pugi::xml_node OPCOutput::Save(pugi::xml_node parent) {

    pugi::xml_node node = parent.append_child("network");
    IPOutput::SaveAttr(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
std::string OPCOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "OPC {" + std::to_string(_universe) + "} ";
    res += "[1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ")";

    return res;
}

void OPCOutput::SetChannels(int32_t channels)
{
    IPOutput::SetChannels(channels);
    if (_data != nullptr)
    {
        free(_data);
        _data = nullptr;
    }
    _data = (uint8_t*)malloc(_channels + OPC_PACKET_HEADERLEN);
    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);
}

std::string OPCOutput::GetExport() const {

    return fmt::format(",{},{},,{},{},,,,{},{}",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}

void OPCOutput::SetTransientData(int32_t& startChannel, int nullnumber) {

    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetTransientData(startChannel, nullnumber);
    //}

    assert(startChannel != -1);
    //_outputNumber = on++;
    _startChannel = startChannel;
    startChannel += GetChannels();
}
#pragma endregion

#pragma region Start and Stop
bool OPCOutput::Open() {

    if (!_enabled) return true;
    if (_ip == "") return false;
    if (!ip_utils::IsIPValid(GetResolvedIP())) return false;

    _ok = IPOutput::Open();
    //if (_fppProxyOutput) {
    //    return _ok;
    //}

    memset(_data, 0x00, _channels + OPC_PACKET_HEADERLEN);

    _data[0] = (uint8_t)_universe;
    _data[1] = 0x00; // RGB values
    _data[2] = (uint8_t)((_channels & 0xFF00) >> 8);
    _data[3] = (uint8_t)(_channels & 0xFF);

    _remoteIp = GetResolvedIP();

    OpenSocket();

    return _ok && _socket != nullptr;
}

void OPCOutput::Close() {

    if (_socket != nullptr) {
        //while (_socket->WaitForWrite(0, 50));
        delete _socket;
        _socket = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion 

#pragma region Frame Handling
void OPCOutput::StartFrame(long msec) {

    

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    return _fppProxyOutput->StartFrame(msec);
    //}

    if (_socket == nullptr && OutputManager::IsRetryOpen()) {
        OpenSocket();
        if (_ok) {
            spdlog::debug("OPCOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void OPCOutput::EndFrame(int suppressFrames) {

    // on really low end controllers I have seen this get so backed up it re-enters
    static bool reentry = false;

    if (!_enabled || _suspend || _tempDisable) return;

    //if (_fppProxyOutput) {
    //    _fppProxyOutput->EndFrame(suppressFrames);
    //    return;
    //}

    if (_socket == nullptr) return;

    if (reentry) return;
    reentry = true;

    if (_changed || NeedToOutput(suppressFrames)) {
        if (_socket->Write(_data, _channels + OPC_PACKET_HEADERLEN)) {
            FrameOutput();
        }
        else {
            spdlog::error("OPCOutput: Write failed for {}: {}", _remoteIp, _socket->LastError());
            delete _socket;
            _socket = nullptr;
            SkipFrame();
        }
    }
    else {
        SkipFrame();
    }
    reentry = false;
}

void OPCOutput::ResetFrame() {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->ResetFrame();
    //    return;
    //}
}
#pragma endregion

#pragma region Data Setting
void OPCOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetOneChannel(channel, data);
    //    return;
    //}

    if (_data[channel + OPC_PACKET_HEADERLEN] != data) {
        _data[channel + OPC_PACKET_HEADERLEN] = data;
        _changed = true;
    }
}

void OPCOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->SetManyChannels(channel, data, size);
    //} 
    //else {
        size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));

        if (memcmp(&_data[channel + OPC_PACKET_HEADERLEN], data, chs) == 0) {
            // nothing changed
        } else {
            memcpy(&_data[channel + OPC_PACKET_HEADERLEN], data, chs);
            _changed = true;
        }
    //}
}

void OPCOutput::AllOff() {

    if (!_enabled) return;
    //if (_fppProxyOutput) {
    //    _fppProxyOutput->AllOff();
    //} 
    //else {
        memset(&_data[OPC_PACKET_HEADERLEN], 0x00, _channels);
        _changed = true;
    //}
}
#pragma endregion



