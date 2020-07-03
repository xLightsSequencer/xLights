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
// * This class represents a single universe for DDP
// ******************************************************

#pragma region DDP Constants
#define DDP_PACKET_HEADERLEN 10
#define DDP_PACKET_LEN (DDP_PACKET_HEADERLEN + 1440)
#define DDP_PORT 4048
#define DDP_SYNCPACKET_LEN 10

#define DDP_FLAGS1_VER     0xc0   // version mask
#define DDP_FLAGS1_VER1    0x40   // version=1
#define DDP_FLAGS1_PUSH    0x01
#define DDP_FLAGS1_QUERY   0x02
#define DDP_FLAGS1_REPLY   0x04
#define DDP_FLAGS1_STORAGE 0x08
#define DDP_FLAGS1_TIME    0x10

#define DDP_ID_DISPLAY       1
#define DDP_ID_CONFIG      250
#define DDP_ID_STATUS      251
#pragma endregion 

class DDPOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t _data[DDP_PACKET_LEN];
    uint8_t _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;
    uint8_t* _fulldata;
    int _channelsPerPacket;
    bool _keepChannelNumbers;

    // These are used for DDP sync
    static bool __initialised;
    #pragma  endregion

    #pragma region Private Functions
    void OpenDatagram();
    #pragma  endregion

public:

    #pragma region Constructors and Destructors
    DDPOutput(wxXmlNode* node);
    DDPOutput();
    virtual ~DDPOutput() override;
    virtual wxXmlNode* Save() override;
    #pragma endregion

    #pragma region Static Functions
    static void SendSync();
    #pragma endregion 

    #pragma region Getters and Setters
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }

    int GetChannelsPerPacket() const { return _channelsPerPacket; }
    void SetChannelsPerPacket(int cpp) { _channelsPerPacket = cpp; _dirty = true; }

    virtual bool IsKeepChannelNumbers() const { return _keepChannelNumbers; }
    virtual void SetKeepChannelNumber(bool b = true) { if (_keepChannelNumbers != b) { _keepChannelNumbers = b; _dirty = true; } }

    virtual std::string GetType() const override { return OUTPUT_DDP; }

    virtual std::string GetLongDescription() const override;

    virtual int GetMaxChannels() const override { return 1000000; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
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

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) override;
    #endif
    #pragma endregion UI

};
