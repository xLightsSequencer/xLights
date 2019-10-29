#ifndef PACKETDATA_H
#define PACKETDATA_H

#include <wx/wx.h>
#include <map>

#define ARTNET_PACKET_HEADERLEN 18
#define ARTNET_PACKET_LEN (ARTNET_PACKET_HEADERLEN + 512)
#define E131_PACKET_HEADERLEN 126
#define E131_PACKET_LEN (E131_PACKET_HEADERLEN + 512)

class wxDatagramSocket;

class PacketData
{
public:
    uint8_t _data[1533];
    long _type = 0;
    int _universe = 0;
    int _length = 0;
    std::map<int, int> _sequenceNum;
    std::string _tag = "";

    virtual ~PacketData() { }
    PacketData();
    uint8_t GetData(int c);
    uint8_t* GetDataPtr();
    void SetData(int c, uint8_t dd);
    bool Update(long type, uint8_t* packet, int len);
    void Send(wxDatagramSocket* e131Socket, wxDatagramSocket* artNETSocket, const std::string& ip) const;
    int GetDataLength() const;
    uint8_t UniverseHigh() const { return (_universe >> 8) & 0xFF; }
    uint8_t UniverseLow() const { return _universe & 0xFF; }
    void CopyFrom(PacketData* source, long targetType);
    int GetNextSequenceNum(int u);
    void InitialiseArtNETHeader();
    void InitialiseE131Header();
    int GetSequenceNum() const;
    void InitialiseLength(long type, int length, int universe);
    void ApplyBrightness(int brightness, const std::list<int>& excludeChannels);
};

#endif 
