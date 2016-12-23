#ifndef ARTNETOUTPUT_H
#define ARTNETOUTPUT_H

#include "IPOutput.h"
#include <wx/sckaddr.h>
#include <wx/socket.h>

// ******************************************************
// * This class represents a single universe for ArtNET
// ******************************************************

#define ARTNET_PACKET_HEADERLEN 18
#define ARTNET_PACKET_LEN (ARTNET_PACKET_HEADERLEN + 512)
#define ARTNET_PORT 0x1936
#define ARTNET_MAXCHANNEL 32768
#define ARTNET_SYNCPACKET_LEN 14

class ArtNetOutput : public IPOutput
{
    wxByte _data[ARTNET_PACKET_LEN];
    wxByte _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;
    static int __ip1;
    static int __ip2;
    static int __ip3;
    static bool __initialised;

    int GetArtNetUniverse() const { return _universe & 0x000F; }
    int GetArtNetSubnet() const { return (_universe & 0x00F0) >> 4; }
    int GetArtNetNet() const { return (_universe & 0x7F00) >> 8; }
    void SetArtNetUniverse(int net, int subnet, int universe) { _universe = ((net & 0x007F) << 8) + ((subnet & 0x000F) << 4) + (universe & 0x000F); }

public:
    ArtNetOutput(wxXmlNode* node);
    ArtNetOutput();
    virtual ~ArtNetOutput() override;

    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void Close() override;
    virtual void StartFrame(long msec) override;
    virtual void EndFrame() override;
    virtual void ResetFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void SetManyChannels(int channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_ARTNET; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(int ch) const override;
    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= 512; }
    static void SendSync();
	
#ifndef EXCLUDENETWORKUI
    virtual int Configure(wxWindow* parent, OutputManager& outputManager) override;
#endif
};

 #endif