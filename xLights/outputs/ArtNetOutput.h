#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
#pragma endregion 

class ControllerEthernet;

class ArtNetOutput : public IPOutput
{
#pragma region Member Variables
    uint8_t _data[ARTNET_PACKET_LEN];
    uint8_t _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket* _datagram;

    // These are used for artnet sync
    static int __ip1;
    static int __ip2;
    static int __ip3;
    static bool __initialised;
#pragma endregion

#pragma region Private Functions
    void OpenDatagram();
#pragma endregion

public:

#pragma region Constructors and Destructors
    ArtNetOutput(wxXmlNode* node);
    ArtNetOutput();
    virtual ~ArtNetOutput() override;
#pragma endregion

#pragma region Static Functions
    static int GetArtNetNet(int u) { return (u & 0x7F00) >> 8; }
    static int GetArtNetSubnet(int u) { return (u & 0x00F0) >> 4; }
    static int GetArtNetUniverse(int u) { return u & 0x000F; }
    static int GetArtNetCombinedUniverse(int net, int subnet, int universe) { return ((net & 0x007F) << 8) + ((subnet & 0x000F) << 4) + (universe & 0x000F); }
    static void SendSync();
    static std::list<ControllerEthernet*> Discover(OutputManager* outputManager); // Discovers controllers supporting this protocol
#pragma endregion  

#pragma region Getters and Setters
    int GetArtNetUniverse() const { return GetArtNetUniverse(_universe); }
    int GetArtNetSubnet() const { return GetArtNetSubnet(_universe); }
    int GetArtNetNet() const { return GetArtNetNet(_universe); }
    void SetArtNetUniverse(int net, int subnet, int universe) { _universe = GetArtNetCombinedUniverse(net, subnet, universe); }

    virtual std::string GetType() const override { return OUTPUT_ARTNET; }

    virtual std::string GetLongDescription() const override;

    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= 512; }

    virtual std::string GetUniverseString() const override;

    virtual std::string GetExport() const override;
#pragma endregion

#pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
#pragma endregion

#pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
#pragma endregion

#pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
#pragma endregion
};