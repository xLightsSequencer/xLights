#ifndef ZCPPOUTPUT_H
#define ZCPPOUTPUT_H

#include "IPOutput.h"
#include <wx/socket.h>

// ******************************************************
// * This class represents a single universe for E1.31
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#pragma region ZCPP Constants
#define ZCPP_PACKET_HEADERLEN 6
#define ZCPP_PACKET_LEN 1458
#define ZCPP_SYNCPACKET_LEN 7
#define ZCPP_PORT 30005
#define ZCPP_MAXCHANNELS (16*1024)
#define ZCPP_MODELDATASIZE (40 + 10 * 100)
#pragma endregion ZCPP Constants

class ZCPPOutput : public IPOutput
{
    #pragma region Member Variables
    wxByte* _data;
    wxByte _packet[ZCPP_PACKET_LEN];
    unsigned char _modelData[ZCPP_MODELDATASIZE];
    wxByte _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;
    long _lastSecond;
    int _vendor;
    int _model;
    long _usedChannels;
    #pragma endregion Member Variables

    void ExtractUsedChannelsFromModelData();

public:

    #pragma region Constructors and Destructors
    ZCPPOutput(wxXmlNode* node, std::string showdir);
    ZCPPOutput();
    virtual ~ZCPPOutput() override;
    #pragma endregion Constructors and Destructors
    
    #pragma region Static Functions
    static void SendSync(int syncUniverse);
    static std::list<Output*> Discover(OutputManager* outputManager);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool NeedsControllerConfig() const override { return true; }
    virtual std::string GetType() const override { return OUTPUT_ZCPP; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(long ch) const override;
    virtual int GetMaxChannels() const override { return ZCPP_MAXCHANNELS; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
    virtual long GetEndChannel() const override;
    virtual std::string GetUniverseString() const override;
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    void SetVendor(int vendor) { _vendor = vendor; _dirty = true; }
    void SetModel(int model) { _model = model; _dirty = true; }
    int GetVendor() const { return _vendor; }
    int GetModel() const { return _model; }
    bool SetModelData(unsigned char* buffer, size_t bufsize, std::string showDir);
    virtual bool IsLookedUpByControllerName() const override { return true; }
    #pragma region Getters and Setters

    virtual wxXmlNode* Save() override;

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion Start and Stop
    
    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion Frame Handling
    
    #pragma region Data Setting
    virtual void SetTransientData(int on, long startChannel, int nullnumber) override;
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