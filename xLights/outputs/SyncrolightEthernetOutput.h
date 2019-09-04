#pragma once

#include "IPOutput.h"

#include <wx/socket.h>

class SyncrolightEthernetOutput : public IPOutput
{
protected:
    #pragma region Syncrolight Constants
    static const unsigned int SYNCROLIGHTETHERNET_PACKET_HEADERLEN = 7;
    static const unsigned int SYNCROLIGHTETHERNET_PACKET_FOOTERLEN = 1;
    static const unsigned int SYNCROLIGHTETHERNET_MAX_CHANNELS = 4096*3;
    static const unsigned int SYNCROLIGHT_PORT = 11000;
    static const unsigned int SYNCROLIGHT_HEARTBEATINTERVAL = 25;
    #pragma endregion Syncrolight Constants

    #pragma region Member Variables
    uint8_t _packet[908];
    uint8_t* _data = nullptr;
    wxIPV4address _remoteAddr;
    wxDatagramSocket* _datagram = nullptr;
    int _port = 0;
    #pragma endregion Member Variables

    static void Heartbeat(int mode);

public:

    #pragma region Constructors and Destructors
    SyncrolightEthernetOutput(wxXmlNode* node);
    SyncrolightEthernetOutput(SyncrolightEthernetOutput* output);
    SyncrolightEthernetOutput();
    virtual ~SyncrolightEthernetOutput() override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_SYNCROLIGHTETHERNET; }
    virtual int32_t GetMaxChannels() const override { return SYNCROLIGHTETHERNET_MAX_CHANNELS; }
    static int GetMaxSyncrolightChannels() { return SYNCROLIGHTETHERNET_MAX_CHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= SYNCROLIGHTETHERNET_MAX_CHANNELS; }
    virtual bool IsLookedUpByControllerName() const override;
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    virtual std::string GetLongDescription() const override;
    int GetPort() const { return _port; }
    void SetPort(int port) { _port = port; _dirty = true; }
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetExport() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;

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
    virtual void SetManyChannels(int32_t channel, unsigned char data[], size_t size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
        virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
    #endif
    #pragma endregion UI
};
