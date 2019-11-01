#include "PacketData.h"

#include <wx/socket.h>

PacketData::PacketData()
{
    memset(_data, 0x00, sizeof(_data));
    _tag = wxString::Format("xFade %d", wxGetProcessId()).ToStdString();
}

uint8_t PacketData::GetData(int c)
{
    wxASSERT(c >= 0 && c < 512);
    if (_type == E131PORT)
    {
        if (_length >= c + E131_PACKET_HEADERLEN)
        {
            return _data[c + E131_PACKET_HEADERLEN];
        }
    }
    else if (_type == ARTNETPORT)
    {
        if (_length >= c + ARTNET_PACKET_HEADERLEN)
        {
            return _data[c + ARTNET_PACKET_HEADERLEN];
        }
    }

    return 0;
}

uint8_t* PacketData::GetDataPtr()
{
    if (_type == E131PORT)
    {
        return &_data[E131_PACKET_HEADERLEN];
    }
    else if (_type == ARTNETPORT)
    {
        return &_data[ARTNET_PACKET_HEADERLEN];
    }

    return nullptr;
}

void PacketData::SetData(int c, uint8_t dd)
{
    wxASSERT(c >= 0 && c < 512);
    if (_type == E131PORT)
    {
        if (_length >= c + E131_PACKET_HEADERLEN)
        {
            _data[c + E131_PACKET_HEADERLEN] = dd;
        }
    }
    else if (_type == ARTNETPORT)
    {
        if (_length >= c + ARTNET_PACKET_HEADERLEN)
        {
            _data[c + ARTNET_PACKET_HEADERLEN] = dd;
        }
    }
}

bool PacketData::Update(long type, uint8_t packet[], int len)
{
    if (type == E131PORT)
    {
        // validate the packet
        if (len < E131_PACKET_HEADERLEN) return false;
        if (packet[4] != 0x41) return false;
        if (packet[5] != 0x53) return false;
        if (packet[6] != 0x43) return false;
        if (packet[7] != 0x2d) return false;
        if (packet[8] != 0x45) return false;
        if (packet[9] != 0x31) return false;
        if (packet[10] != 0x2e) return false;
        if (packet[11] != 0x31) return false;
        if (packet[12] != 0x37) return false;

        _universe = ((int)_data[113] << 8) + (int)_data[114];
        _type = type;
        _length = len;
        wxASSERT(_length >= E131_PACKET_HEADERLEN && _length <= E131_PACKET_HEADERLEN + 512);
        memcpy(_data, packet, len);
    }
    else if (type == ARTNETPORT)
    {
        // validate the packet
        if (len < ARTNET_PACKET_HEADERLEN) return false;
        if (packet[0] != 'A') return false;
        if (packet[1] != 'r') return false;
        if (packet[2] != 't') return false;
        if (packet[3] != '-') return false;
        if (packet[4] != 'N') return false;
        if (packet[5] != 'e') return false;
        if (packet[6] != 't') return false;
        if (packet[9] != 0x50) return true; // pretend success as otherwise I will log excessively

        _universe = ((int)_data[15] << 8) + (int)_data[14];
        _type = type;
        _length = len;
        wxASSERT(_length >= ARTNET_PACKET_HEADERLEN && _length <= ARTNET_PACKET_HEADERLEN + 512);
        memcpy(_data, packet, len);
    }

    return true;
}

void PacketData::Send(wxDatagramSocket* e131Socket, wxDatagramSocket* artNETSocket, const std::string& ip) const
{
    wxIPV4address remoteaddr;
    if (wxString(ip).StartsWith("239.255.") || ip == "MULTICAST")
    {
        // multicast - universe number must be in lower 2 bytes
        wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", 239, 255, (int)UniverseHigh(), (int)UniverseLow());
        remoteaddr.Hostname(ipaddrWithUniv);
    }
    else
    {
        remoteaddr.Hostname(ip.c_str());
    }

    if (_type == E131PORT)
    {
        remoteaddr.Service(E131PORT);
        if (e131Socket != nullptr) e131Socket->SendTo(remoteaddr, _data, _length);
    }
    else
    {
        remoteaddr.Service(ARTNETPORT);
        if (artNETSocket != nullptr) artNETSocket->SendTo(remoteaddr, _data, _length);
    }
}

int PacketData::GetDataLength() const
{
    if (_type == E131PORT)
    {
        if (_length < E131_PACKET_HEADERLEN) return 0;
        wxASSERT(_length - E131_PACKET_HEADERLEN <= 512);
        return _length - E131_PACKET_HEADERLEN;
    }
    else if (_type == ARTNETPORT)
    {
        if (_length < ARTNET_PACKET_HEADERLEN) return 0;
        wxASSERT(_length - ARTNET_PACKET_HEADERLEN <= 512);
        return _length - ARTNET_PACKET_HEADERLEN;
    }
    return 0;
}

int PacketData::GetNextSequenceNum(int u)
{
    _sequenceNum[u] = _sequenceNum[u] >= 255 ? 0 : _sequenceNum[u] + 1;
    return _sequenceNum[u];
}

void PacketData::InitialiseArtNETHeader()
{
    memset(_data, 0x00, sizeof(_data));
    int channels = GetDataLength();

    _data[0] = 'A';   // ID[8]
    _data[1] = 'r';
    _data[2] = 't';
    _data[3] = '-';
    _data[4] = 'N';
    _data[5] = 'e';
    _data[6] = 't';
    _data[9] = 0x50;
    _data[11] = 0x0E; // Protocol version Low
    _data[14] = (_universe & 0xFF);
    _data[15] = ((_universe & 0xFF00) >> 8);
    _data[16] = 0x02; // we are going to send all 512 bytes
    _data[16] = (uint8_t)(channels >> 8);  // Property value count (high)
    _data[17] = (uint8_t)(channels & 0xff);  // Property value count (low)}
}

#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"

void PacketData::InitialiseE131Header()
{
    memset(_data, 0x00, sizeof(_data));
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

    wxString id = XLIGHTS_UUID;
    id.Replace("-", "");
    id.MakeLower();
    if (id.Len() != 32) throw "invalid CID";
    for (int i = 0, j = 22; i < 32; i += 2)
    {
        wxChar msb = id.GetChar(i);
        wxChar lsb = id.GetChar(i + 1);
        msb -= isdigit(msb) ? 0x30 : 0x57;
        lsb -= isdigit(lsb) ? 0x30 : 0x57;
        _data[j++] = (uint8_t)((msb << 4) | lsb);
    }

    _data[38] = 0x72;  // Framing Protocol flags and length (high)
    _data[39] = 0x58;  // 0x258 = 638 - 38
    _data[43] = 0x02;
    // Source Name (64 bytes)
    memset(&_data[44], 0x00, 64);
    strcpy((char*)&_data[44], _tag.c_str());
    _data[108] = 100;  // Priority
    _data[113] = _universe >> 8;  // Universe Number (high)
    _data[114] = _universe & 0xff;  // Universe Number (low)
    _data[115] = 0x72;  // DMP Protocol flags and length (high)
    _data[116] = 0x0b;  // 0x20b = 638 - 115
    _data[117] = 0x02;  // DMP Vector (Identifies DMP Set Property Message PDU)
    _data[118] = 0xa1;  // DMP Address Type & Data Type
    _data[122] = 0x01;  // Address Increment (low)
    _data[123] = 0x02;  // Property value count (high)
    _data[124] = 0x01;  // Property value count (low)

    int channels = GetDataLength();
    int i = channels;
    uint8_t NumHi = (channels + 1) >> 8;   // Channels (high)
    uint8_t NumLo = (channels + 1) & 0xff; // Channels (low)

    _data[123] = NumHi;  // Property value count (high)
    _data[124] = NumLo;  // Property value count (low)

    i = E131_PACKET_LEN - 16 - (512 - channels);
    uint8_t hi = i >> 8;   // (high)
    uint8_t lo = i & 0xff; // (low)

    _data[16] = hi + 0x70;  // RLP Protocol flags and length (high)
    _data[17] = lo;  // 0x26e = E131_PACKET_LEN - 16

    i = E131_PACKET_LEN - 38 - (512 - channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[38] = hi + 0x70;  // Framing Protocol flags and length (high)
    _data[39] = lo;  // 0x258 = E131_PACKET_LEN - 38

    i = E131_PACKET_LEN - 115 - (512 - channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[115] = hi + 0x70;  // DMP Protocol flags and length (high)
    _data[116] = lo;  // 0x20b = E131_PACKET_LEN - 115
}

int PacketData::GetSequenceNum() const
{
    if (_type == E131PORT)
    {
        if (_length <= E131_PACKET_HEADERLEN) return -1;
        return (int)_data[111];
    }
    else if (_type == ARTNETPORT)
    {
        if (_length <= ARTNET_PACKET_HEADERLEN) return -1;
        return (int)_data[12];
    }
    return -1;
}

void PacketData::InitialiseLength(long type, int length, int universe)
{
    _type = type;
    _length = length;
    _universe = universe;

    if (_type == E131PORT)
    {
        wxASSERT(_length >= E131_PACKET_HEADERLEN && _length <= E131_PACKET_HEADERLEN + 512);
        InitialiseE131Header();
    }
    else if (_type == ARTNETPORT)
    {
        wxASSERT(_length >= ARTNET_PACKET_HEADERLEN && _length <= ARTNET_PACKET_HEADERLEN + 512);
        InitialiseArtNETHeader();
    }
}

void PacketData::ApplyBrightness(int brightness, const std::list<int>& excludeChannels)
{
    if (brightness == 100) return;

    if (excludeChannels.size() == 0)
    {
        if (brightness == 0)
        {
            memset(GetDataPtr(), 0x00, GetDataLength());
        }
        else
        {
            uint8_t* p = GetDataPtr();
            for (int i = 0; i < GetDataLength(); i++)
            {
                *(p + i) = (uint8_t)((int)*(p + i) * brightness / 100);
            }
        }
    }
    else
    {
        uint8_t* p = GetDataPtr();
        for (int i = 0; i < GetDataLength(); i++)
        {
            if (std::find(excludeChannels.begin(), excludeChannels.end(), i + 1) == excludeChannels.end())
            {
                *(p + i) = (uint8_t)((int)*(p + i) * brightness / 100);
            }
        }
    }
}

void PacketData::CopyFrom(PacketData* source, long targetType)
{
    wxASSERT(source != nullptr);
    _length = 0;
    _type = targetType;
    _universe = source->_universe;

    if (source->_type == targetType)
    {
        memcpy(_data, source->_data, sizeof(_data));
        _length = source->_length;
        wxASSERT(_length >= 0 && _length <= sizeof(_data));

        if (_type == E131PORT)
        {
            wxASSERT(_length >= E131_PACKET_HEADERLEN && _length <= E131_PACKET_HEADERLEN + 512);
            memset(&_data[44], 0x00, 64);
            strncpy((char*)&_data[44], _tag.c_str(), 64);
            _data[111] = GetNextSequenceNum(_universe);
        }
        else if (_type == ARTNETPORT)
        {
            wxASSERT(_length >= ARTNET_PACKET_HEADERLEN && _length <= ARTNET_PACKET_HEADERLEN + 512);
            // nothing to do
            _data[12] = GetNextSequenceNum(_universe);
        }
    }
    else
    {
        if (_type == E131PORT)
        {
            // converting from ARTNET
            _length = E131_PACKET_HEADERLEN + source->GetDataLength();
            InitialiseE131Header();
            memcpy(source->GetDataPtr(), GetDataPtr(), GetDataLength());
            memset(&_data[44], 0x00, 64);
            strncpy((char*)&_data[44], _tag.c_str(), 64);
            _data[111] = GetNextSequenceNum(_universe);
        }
        else if (_type == ARTNETPORT)
        {
            // converting from E131
            _length = ARTNET_PACKET_HEADERLEN + source->GetDataLength();
            InitialiseArtNETHeader();
            memcpy(source->GetDataPtr(), GetDataPtr(), GetDataLength());
            _data[12] = GetNextSequenceNum(_universe);
        }
    }
}