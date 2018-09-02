#ifndef ARTNETOUTPUT_H
#define ARTNETOUTPUT_H

#include "IPOutput.h"
#include <wx/sckaddr.h>
#include <wx/socket.h>

// ******************************************************
// * This class represents a single universe for ArtNET
// ******************************************************

#pragma region ArtNet Constants
#define ARTNET_PACKET_HEADERLEN 18
#define ARTNET_PACKET_LEN (ARTNET_PACKET_HEADERLEN + 512)
#define ARTNET_PORT 0x1936
#define ARTNET_MAXCHANNEL 32768
#define ARTNET_SYNCPACKET_LEN 14
#pragma endregion ArtNet Constants

class ArtNetOutput : public IPOutput
{
    #pragma region Member Variables
    wxByte _data[ARTNET_PACKET_LEN];
    wxByte _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;

    // These are used for artnet sync
    static int __ip1;
    static int __ip2;
    static int __ip3;
    static bool __initialised;
    #pragma  endregion Member Variables

public:

    #pragma region Constructors and Destructors
    ArtNetOutput(wxXmlNode* node);
    ArtNetOutput();
    virtual ~ArtNetOutput() override;
    #pragma endregion  Constructors and Destructors

    #pragma region Static Functions
    static int GetArtNetNet(int u);
    static int GetArtNetSubnet(int u);
    static int GetArtNetUniverse(int u);
    static int GetArtNetCombinedUniverse(int net, int subnet, int u);
    static void SendSync();
    static std::list<Output*> Discover(OutputManager* outputManager); // Discovers controllers supporting this protocol
    #pragma endregion  Static Functions

    #pragma region Getters and Setters
    int GetArtNetUniverse() const { return GetArtNetUniverse(_universe); }
    int GetArtNetSubnet() const { return GetArtNetSubnet(_universe); }
    int GetArtNetNet() const { return GetArtNetNet(_universe); }
    void SetArtNetUniverse(int net, int subnet, int universe) { _universe = GetArtNetCombinedUniverse(net, subnet, universe); }
    virtual std::string GetType() const override { return OUTPUT_ARTNET; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(long ch) const override;
    virtual int GetMaxChannels() const override { return 512; }
    virtual std::string GetUniverseString() const override;
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= 512; }
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override {};
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(long channel, unsigned char data) override;
    virtual void SetManyChannels(long channel, unsigned char* data, long size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager) override;
#endif
    #pragma endregion UI
};

 #endif