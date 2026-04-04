
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "KinetOutput.h"
#include <cassert>
#include <cstring>
#include "OutputManager.h"
#include "ControllerEthernet.h"
#include "../utils/ip_utils.h"

#include <log.h>

#pragma region Static Variables
#pragma endregion

#pragma region Private Functions
void KinetOutput::OpenDatagram() {

    

    if (_datagram != nullptr) return;

    _datagram = new sockets::UDPSocket();
    if (_datagram == nullptr) {
        spdlog::error("Error creating Kinet datagram object for {} {}.", _ip, GetUniverse());
        _ok = false;
    }

    const std::string localIp = GetForceLocalIPToUse();
    if (!_datagram->Bind(localIp, 0, false)) {
        spdlog::error("Error initialising Kinet datagram for {} {}. {}", _ip, GetUniverse(), _datagram->LastError());
        delete _datagram;
        _datagram = nullptr;
        _ok = false;
    }
}

void KinetOutput::PopulateHeader()
{
    memset(_data, 0x00, sizeof(_data));

    _data[0] = 0x04;   // magic[4] 0401dc4a
    _data[1] = 0x01;
    _data[2] = 0xdc;
    _data[3] = 0x4a;
	
    _data[4] = 0x01; // ver [2] 0100
    _data[5] = 0x00;
	
	if (_version == 1)
	{
		_data[6] = 0x01;  // type [2] 
	}
	else 
	{
		_data[6] = 0x08; 
	}
	_data[7] = 0x01;
	
	_data[8] = 0x00; // seq [4]
    _data[9] = 0x00;
    _data[10] = 0x00;
    _data[11] = 0x00;

	if (_version == 1)
	{	
		_data[12] = (_universe - 1) & 0xFF; // port
		_data[13] = 0x00; // padding

		_data[14] = 0x00; // flags[2]
		_data[15] = 0x00; 
		
		_data[16] = 0xFF; // time [4]
		_data[17] = 0xFF;
		_data[18] = 0xFF;
		_data[19] = 0xFF;

		_data[20] = 0x00; // uni
	}
	else
	{
		_data[12] = 0xFF; // time [4]
		_data[13] = 0xFF;
		_data[14] = 0xFF;
		_data[15] = 0xFF;
		
		_data[16] = (_universe - 1) & 0xFF; // port
		_data[17] = _sequenceNum; // ?

		_data[18] = 0x00;
		_data[19] = 0x00;
		_data[20] = 0x00;
		_data[21] = 0x02;
		_data[22] = 0x00;
		_data[23] = 0x00;
	}
}
#pragma endregion

#pragma region Constructors and Destructors
KinetOutput::KinetOutput(pugi::xml_node node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > 512) SetChannels(512);
    _sequenceNum = 0;
    _datagram = nullptr;
    _version = node.attribute("Version").as_int(2);
    memset(_data, 0, sizeof(_data));
}

KinetOutput::KinetOutput() : IPOutput() {

    _channels = 512;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

KinetOutput::KinetOutput(const KinetOutput& from) :
    IPOutput(from)
{
    _channels = from._channels;
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

KinetOutput::~KinetOutput() {
    if (_datagram != nullptr) delete _datagram;
}

pugi::xml_node KinetOutput::Save(pugi::xml_node parent)
{
    pugi::xml_node node = parent.append_child("network");
    IPOutput::SaveAttr(node);

    node.append_attribute("Version") = _version;

    return node;
}
#pragma endregion

#pragma region Static Functions
#pragma endregion

#pragma region Getters and Setters
std::string KinetOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "Kinet {" + GetUniverseString() + "} ";
    res += "[1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";

    return res;
}

std::string KinetOutput::GetExport() const {

    return std::format(",{},{},,{},{},,,,{},{}",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}
#pragma endregion

#pragma region Start and Stop
bool KinetOutput::Open() {
    if (!_enabled) return true;
    if (!ip_utils::IsIPValid(GetResolvedIP())) return false;

    _ok = IPOutput::Open();

	PopulateHeader();

    _sequenceNum = 0;

    OpenDatagram();

    _remoteIp = GetResolvedIP();

    return _ok;
}

void KinetOutput::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

#pragma region Frame Handling
void KinetOutput::StartFrame(long msec) {

    

    if (!_enabled) return;

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            spdlog::debug("KinetOutput: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void KinetOutput::EndFrame(int suppressFrames) {

    if (!_enabled || _suspend || _tempDisable|| _datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        _data[8] = _sequenceNum & 0xFF;
        _data[9] = (_sequenceNum >> 8) & 0xFF;
        _data[10] = (_sequenceNum >> 16) & 0xFF;
        _data[11] = (_sequenceNum >> 24) & 0xFF;
        if (_datagram->SendTo(_remoteIp, KINET_PORT, _data, GetHeaderPacketLength() + _channels)) {
            _sequenceNum++;
            FrameOutput();
            _changed = false;
        } else {
            spdlog::error("KinetOutput: SendTo failed for {}:{} - {}", _remoteIp, KINET_PORT, _datagram->LastError());
            SkipFrame();
        }
    }
    else {
        SkipFrame();
    }
}
#pragma endregion

#pragma region Data Setting
void KinetOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled) return;
    assert(channel < _channels);

    if (_data[channel + GetHeaderPacketLength()] != data) {
        _data[channel + GetHeaderPacketLength()] = data;
        _changed = true;
    }
}

void KinetOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (!_enabled) return;
    assert((size_t)channel + size <= (size_t)_channels);

    size_t chs = (std::min)((int32_t)size, _channels - channel);

    if (memcmp(&_data[channel + GetHeaderPacketLength()], data, chs) == 0) {
        // nothing has changed
    }
    else {
        memcpy(&_data[channel + GetHeaderPacketLength()], data, chs);
        _changed = true;
    }
}

void KinetOutput::AllOff() {

    if (!_enabled) return;
    memset(&_data[GetHeaderPacketLength()], 0x00, _channels);
    _changed = true;
}
#pragma endregion





