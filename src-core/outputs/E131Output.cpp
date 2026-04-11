
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "E131Output.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif
#include "OutputManager.h"
#include "UtilFunctions.h"
#include "../utils/ip_utils.h"
#include "ControllerEthernet.h"

#include <spdlog/fmt/fmt.h>

#include <log.h>

// Parse a UUID string (e.g. "c0de0080-c69b-...") into 16 raw bytes at dest[0..15].
static void ParseUUIDBytes(const char* uuid, uint8_t* dest) {
    std::string id(uuid);
    std::erase(id, '-');
    for (auto& c : id) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    assert(id.size() == 32);
    for (int i = 0, j = 0; i < 32; i += 2) {
        char msb = id[i];
        char lsb = id[i + 1];
        msb -= std::isdigit(static_cast<unsigned char>(msb)) ? 0x30 : 0x57;
        lsb -= std::isdigit(static_cast<unsigned char>(lsb)) ? 0x30 : 0x57;
        dest[j++] = (uint8_t)((msb << 4) | lsb);
    }
}

#pragma region Private Functions
void E131Output::CreateMultiUniverses_CONVERT(int num) {

    _numUniverses_CONVERT = num;

    for (auto i : _outputs_CONVERT) {
        delete i;
    }
    _outputs_CONVERT.clear();

    if (_numUniverses_CONVERT < 2) return;

    for (int i = 0; i < _numUniverses_CONVERT; i++) {
        E131Output* e = new E131Output();
        e->SetIP(_ip, true);
        e->SetUniverse(_universe + i);
        e->SetChannels(_channels);
        e->_description_CONVERT = _description_CONVERT;
        e->SetSuppressDuplicateFrames(_suppressDuplicateFrames);
        e->SetPriority(_priority);
        _outputs_CONVERT.push_back(e);
    }
}

void E131Output::OpenDatagram() {

    

    if (_datagram != nullptr) return;

    _datagram = new sockets::UDPSocket();
    if (_datagram == nullptr) {
        spdlog::error("E131Output: Error creating datagram object.");
    }
    else if (!_datagram->Bind(GetForceLocalIPToUse(), 0, false)) {
        spdlog::error("E131Output: Error opening datagram. {}", _datagram->LastError());
        delete _datagram;
        _datagram = nullptr;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
E131Output::E131Output(pugi::xml_node node, bool isActive) : IPOutput(node, isActive) {

    if (_channels > 512) SetChannels(512);
    if (_autoSize_CONVERT) _autoSize_CONVERT = false;
    _numUniverses_CONVERT = node.attribute("NumUniverses").as_int(1);
    _priority = node.attribute("Priority").as_int(100);
    if (_numUniverses_CONVERT > 1) {
        CreateMultiUniverses_CONVERT(_numUniverses_CONVERT);
    }
    _sequenceNum = 0;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

E131Output::E131Output() : IPOutput() {

    _channels = 510;
    _universe = 1;
    _sequenceNum = 0;
    _numUniverses_CONVERT = 1;
    _priority = E131_DEFAULT_PRIORITY;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

E131Output::E131Output(const E131Output& from) :
    IPOutput(from)
{
    _numUniverses_CONVERT = from._numUniverses_CONVERT;
    if (_numUniverses_CONVERT > 1) {
        CreateMultiUniverses_CONVERT(_numUniverses_CONVERT);
    }
    _priority = from._priority;
    _channels = from._channels;
    _universe = from._universe;
    _sequenceNum = 0;
    _priority = from._priority;
    _datagram = nullptr;
    memset(_data, 0, sizeof(_data));
}

E131Output::~E131Output()
{
    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
    while (_outputs_CONVERT.size() > 0) {
        delete _outputs_CONVERT.back();
        _outputs_CONVERT.pop_back();
    }
}

pugi::xml_node E131Output::Save(pugi::xml_node parent) {

    pugi::xml_node node = parent.append_child("network");
    IPOutput::SaveAttr(node);

    if (_priority != E131_DEFAULT_PRIORITY) {
        node.append_attribute("Priority") = _priority;
    }

    return node;
}
#pragma endregion

#pragma region Static Functions
void E131Output::SendSync(int syncUniverse, const std::string& localIP) {

    
    static uint8_t syncdata[E131_SYNCPACKET_LEN];
    static uint8_t syncSequenceNum = 0;
    static bool initialised = false;
    static uint16_t _lastsyncuniverse = 0;
    static std::string syncremoteIp;
    static sockets::UDPSocket* syncdatagram = nullptr;

    if (!initialised) {

        spdlog::debug("Initialising e131 Sync.");
        initialised = true;

        memset(syncdata, 0x00, sizeof(syncdata));

        syncdata[1] = 0x10;   // RLP preamble size (low)
        syncdata[4] = 0x41;   // ACN Packet Identifier (12 bytes)
        syncdata[5] = 0x53;
        syncdata[6] = 0x43;
        syncdata[7] = 0x2d;
        syncdata[8] = 0x45;
        syncdata[9] = 0x31;
        syncdata[10] = 0x2e;
        syncdata[11] = 0x31;
        syncdata[12] = 0x37;
        syncdata[16] = 0x70;  // RLP Protocol flags and length (high)
        syncdata[17] = 0x21;  // 0x021 = 49 - 16
        syncdata[21] = 0x08;

        ParseUUIDBytes(XLIGHTS_UUID, &syncdata[22]);

        syncdata[38] = 0x70;  // Framing Protocol flags and length (high)
        syncdata[39] = 0x0b;  // 0x00B = 49 - 38
        syncdata[43] = 0x01;
    }

    if (syncUniverse > 0) {
        if (syncUniverse != _lastsyncuniverse) {
            _lastsyncuniverse = syncUniverse;
            syncSequenceNum = 0;   // sequence number
            syncdata[45] = syncUniverse >> 8;
            syncdata[46] = syncUniverse & 0xff;

            if (syncdatagram != nullptr) {
                delete syncdatagram;
            }

            syncdatagram = new sockets::UDPSocket();

            if (syncdatagram == nullptr) {
                spdlog::error("Error creating E131 sync datagram object.");
            }
            else if (!syncdatagram->Bind(localIP, 0, false)) {
                spdlog::error("Error initialising E131 sync datagram: {}", syncdatagram->LastError());
                delete syncdatagram;
                syncdatagram = nullptr;
            }

            // multicast - universe number must be in lower 2 bytes
            syncremoteIp = fmt::format("239.255.{}.{}", syncdata[45], syncdata[46]);
            spdlog::debug("e131 Sync sync universe changed to {} => {}:{}.", syncUniverse, syncremoteIp, E131_PORT);
        }

        syncdata[44] = syncSequenceNum++;   // sequence number
        syncdata[45] = syncUniverse >> 8;
        syncdata[46] = syncUniverse & 0xff;

        // bail if we dont have a datagram to use
        if (syncdatagram != nullptr) {
            syncdatagram->SendTo(syncremoteIp, E131_PORT, syncdata, E131_SYNCPACKET_LEN);
        }
    }
}

std::string E131Output::GetTag() {
    // creates a unique tag per running instance of xLights on this machine
    return "xLights " + std::to_string(getpid());
}
#pragma endregion

#pragma region Getters and Setters
std::string E131Output::GetLongDescription() const {

    assert(!IsOutputCollection_CONVERT());

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "E1.31 {" + std::to_string(_universe) + "} ";
    res += "[1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ")";

    return res;
}

std::string E131Output::GetExport() const {

    return fmt::format(",{},{},,{},{},,,,{},{}",
        GetStartChannel(),
        GetEndChannel(),
        GetType(),
        GetIP(),
        GetUniverse(),
        GetChannels());
}

void E131Output::SetTransientData(int32_t& startChannel, int nullnumber) {

    assert(!IsOutputCollection_CONVERT());

    if (_fppProxyOutput) {
        _fppProxyOutput->SetTransientData(startChannel, nullnumber);
    }

    assert(startChannel != -1);
    _startChannel = startChannel;
    startChannel += GetChannels();
}
#pragma endregion

#pragma region Start and Stop
bool E131Output::Open() {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return true;
    if (_ip == "") return false;
    if (_ip != "MULTICAST" && !ip_utils::IsIPValid(GetResolvedIP())) return false;

    _ok = IPOutput::Open();
    if (_fppProxyOutput) {
        return _ok;
    }

    memset(_data, 0x00, sizeof(_data));
    _sequenceNum = 0;
    uint8_t UnivHi = _universe >> 8;   // Universe Number (high)
    uint8_t UnivLo = _universe & 0xff; // Universe Number (low)

    _data[1] = 0x10;   // RLP preamble size (low)
    _data[4] = 0x41;   // ACN Packet Identifier (12 bytes)
    _data[5] = 0x53;
    _data[6] = 0x43;
    _data[7] = 0x2d;
    _data[8] = 0x45;
    _data[9] = 0x31;
    _data[10] = 0x2e;
    _data[11] = 0x31;
    _data[12] = 0x37;
    _data[16] = 0x72;  // RLP Protocol flags and length (high)
    _data[17] = 0x6e;  // 0x26e = 638 - 16
    _data[21] = 0x04;

    // CID/UUID

    ParseUUIDBytes(XLIGHTS_UUID, &_data[22]);

    _data[38] = 0x72;  // Framing Protocol flags and length (high)
    _data[39] = 0x58;  // 0x258 = 638 - 38
    _data[43] = 0x02;
    // Source Name (64 bytes)
    strcpy((char*)&_data[44], GetTag().c_str());
    _data[108] = _priority;  // Priority (Default is 100)
    _data[113] = UnivHi;  // Universe Number (high)
    _data[114] = UnivLo;  // Universe Number (low)
    _data[115] = 0x72;  // DMP Protocol flags and length (high)
    _data[116] = 0x0b;  // 0x20b = 638 - 115
    _data[117] = 0x02;  // DMP Vector (Identifies DMP Set Property Message PDU)
    _data[118] = 0xa1;  // DMP Address Type & Data Type
    _data[122] = 0x01;  // Address Increment (low)
    _data[123] = 0x02;  // Property value count (high)
    _data[124] = 0x01;  // Property value count (low)

    OpenDatagram();

    if (GetResolvedIP().rfind("239.255.", 0) == 0 || _ip == "MULTICAST") {
        // multicast - universe number must be in lower 2 bytes
        _remoteIp = fmt::format("239.255.{}.{}", (int)UnivHi, (int)UnivLo);
    }
    else {
        _remoteIp = GetResolvedIP();
    }

    uint8_t NumHi = (_channels + 1) >> 8;   // Channels (high)
    uint8_t NumLo = (_channels + 1) & 0xff; // Channels (low)

    _data[123] = NumHi;  // Property value count (high)
    _data[124] = NumLo;  // Property value count (low)

    int i = E131_PACKET_LEN - 16 - (512 - _channels);
    uint8_t hi = i >> 8;   // (high)
    uint8_t lo = i & 0xff; // (low)

    _data[16] = hi + 0x70;  // RLP Protocol flags and length (high)
    _data[17] = lo;  // 0x26e = E131_PACKET_LEN - 16

    i = E131_PACKET_LEN - 38 - (512 - _channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[38] = hi + 0x70;  // Framing Protocol flags and length (high)
    _data[39] = lo;  // 0x258 = E131_PACKET_LEN - 38

    i = E131_PACKET_LEN - 115 - (512 - _channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[115] = hi + 0x70;  // DMP Protocol flags and length (high)
    _data[116] = lo;  // 0x20b = E131_PACKET_LEN - 115

    return _ok && _datagram != nullptr;
}

void E131Output::Close() {

    if (_datagram != nullptr) {
        delete _datagram;
        _datagram = nullptr;
    }
    IPOutput::Close();
}
#pragma endregion 

#pragma region Frame Handling
void E131Output::StartFrame(long msec) {

    

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return;
    if (_fppProxyOutput) {
        return _fppProxyOutput->StartFrame(msec);
    }

    if (_datagram == nullptr && OutputManager::IsRetryOpen()) {
        OpenDatagram();
        if (_ok) {
            spdlog::debug("E131Output: Open retry successful");
        }
    }

    _timer_msec = msec;
}

void E131Output::EndFrame(int suppressFrames) {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled || _suspend || _tempDisable) return;

    if (_fppProxyOutput) {
        _fppProxyOutput->EndFrame(suppressFrames);
        return;
    }

    if (_datagram == nullptr) return;

    if (_changed || NeedToOutput(suppressFrames)) {
        _data[111] = _sequenceNum;
        _datagram->SendTo(_remoteIp, E131_PORT, _data, E131_PACKET_LEN - (512 - _channels));
        _sequenceNum = _sequenceNum == 255 ? 0 : _sequenceNum + 1;
        FrameOutput();
    }
    else {
        SkipFrame();
    }
}

void E131Output::ResetFrame() {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return;
    if (_fppProxyOutput) {
        _fppProxyOutput->ResetFrame();
        return;
    }
}
#pragma endregion

#pragma region Data Setting
void E131Output::SetOneChannel(int32_t channel, unsigned char data) {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return;
    if (_fppProxyOutput) {
        _fppProxyOutput->SetOneChannel(channel, data);
        return;
    }

    if (_data[channel + E131_PACKET_HEADERLEN] != data) {
        _data[channel + E131_PACKET_HEADERLEN] = data;
        _changed = true;
    }
}

void E131Output::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return;

    if (_fppProxyOutput) {
        _fppProxyOutput->SetManyChannels(channel, data, size);
    } 
    else {
        size_t chs = (std::min)(size, (size_t)(GetMaxChannels() - channel));

        if (memcmp(&_data[channel + E131_PACKET_HEADERLEN], data, chs) == 0) {
            // nothing changed
        } else {
            memcpy(&_data[channel + E131_PACKET_HEADERLEN], data, chs);
            _changed = true;
        }
    }
}

void E131Output::AllOff() {

    assert(!IsOutputCollection_CONVERT());

    if (!_enabled) return;

    if (_fppProxyOutput) {
        _fppProxyOutput->AllOff();
    } 
    else {
        memset(&_data[E131_PACKET_HEADERLEN], 0x00, _channels);
        _changed = true;
    }
}
#pragma endregion


