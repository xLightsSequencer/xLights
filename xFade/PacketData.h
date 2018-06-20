#ifndef PACKETDATA_H
#define PACKETDATA_H

#include <wx/wx.h>
#include <map>

#define ARTNET_PACKET_HEADERLEN 18
#define ARTNET_PACKET_LEN (ARTNET_PACKET_HEADERLEN + 512)
#define E131_PACKET_HEADERLEN 126
#define E131_PACKET_LEN (E131_PACKET_HEADERLEN + 512)

class PacketData
{
public:
    wxByte _data[1533];
    long _type;
    int _universe;
    int _length;
    std::string _source;
    std::map<int, int> _sequenceNum;
    std::string _tag;
    std::string _localIP;

    virtual ~PacketData() { }
    PacketData();
    wxByte GetData(int c);
    wxByte* GetDataPtr();
    void SetData(int c, wxByte dd);
    bool Update(long type, wxByte* packet, int len);
    void Send(std::string ip) const;
    int GetDataLength() const;
    wxByte UniverseHigh() const { return (_universe >> 8) & 0xFF; }
    wxByte UniverseLow() const { return _universe & 0xFF; }
    void CopyFrom(PacketData* source, long targetType);
    int GetNextSequenceNum(int u);
    void InitialiseArtNETHeader();
    void InitialiseE131Header();
    void SetLocalIP(std::string localIP) { _localIP = localIP; }
    int GetSequenceNum() const;
    void InitialiseLength(long type, int length, int universe);
    void ApplyBrightness(int brightness);
};

#endif 
