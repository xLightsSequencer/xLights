#ifndef DDPOUTPUT_H
#define DDPOUTPUT_H

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
#pragma endregion DDP Constants

class DDPOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t _data[DDP_PACKET_LEN];
    uint8_t _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;
    int _channelsPerPacket;
    bool _keepChannelNumbers;
    uint8_t* _fulldata;
    bool _autoStartChannels = false;

    // These are used for DDP sync
    static bool __initialised;
    #pragma  endregion Member Variables

public:

    #pragma region Constructors and Destructors
    DDPOutput(wxXmlNode* node);
    DDPOutput();
    virtual ~DDPOutput() override;
    #pragma endregion  Constructors and Destructors

    #pragma region Static Functions
    static void SendSync();
    #pragma endregion  Static Functions

    virtual wxXmlNode* Save() override;

    #pragma region Getters and Setters
    int GetChannelsPerPacket() const { return _channelsPerPacket; }
    void SetChannelsPerPacket(int cpp) { _channelsPerPacket = cpp; _dirty = true; }
    virtual std::string GetType() const override { return OUTPUT_DDP; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual int GetMaxChannels() const override { return 1000000; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
    virtual bool IsKeepChannelNumbers() const { return _keepChannelNumbers; }
    virtual void KeepChannelNumber(bool b = true) { _keepChannelNumbers = b; _dirty = true; }
    virtual bool IsLookedUpByControllerName() const override;
    virtual bool IsAutoLayoutModels() const override { return _autoStartChannels; }
    virtual void SetAutoStartChannels(bool autoMode) { _autoStartChannels = autoMode; }

    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    #pragma endregion Getters and Setters

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    void OpenDatagram();
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
#endif
    #pragma endregion UI
};

 #endif
