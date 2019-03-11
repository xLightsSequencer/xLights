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
#define ZCPP_MULTICAST_TO "224.0.30.5"
#define ZCPP_EXTRACONFIG_PACKET_SIZE ZCPP_PACKET_LEN
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
    bool _sendConfiguration;
    bool _supportsVirtualStrings;
    std::list<wxByte*> _extraConfig;
    std::list<std::string> _protocols;
    #pragma endregion Member Variables

    void ExtractUsedChannelsFromModelData();
    void DeserialiseProtocols(const std::string& protocols);
    std::string SerialiseProtocols();

public:

    #pragma region Constructors and Destructors
    ZCPPOutput(wxXmlNode* node, std::string showdir);
    ZCPPOutput();
    virtual ~ZCPPOutput() override;
    #pragma endregion Constructors and Destructors
    
    #pragma region Static Functions
    static void SendSync(int syncUniverse);
    static std::list<Output*> Discover(OutputManager* outputManager);
    static void InitialiseExtraConfigPacket(wxByte* buffer, int seq, std::string userControllerId);
    static std::string DecodeProtocol(int protocol);
    static int EncodeProtocol(const std::string& protocol);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool NeedsControllerConfig() const override { return true; }
    virtual std::string GetType() const override { return OUTPUT_ZCPP; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(long ch) const override;
    virtual int GetMaxChannels() const override { return ZCPP_MAXCHANNELS; }
    virtual bool IsValidChannelCount(long channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
    virtual long GetEndChannel() const override;
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    void SetSendConfiguration(bool send) { if (_sendConfiguration != send) { _sendConfiguration = send; _dirty = true; } }
    bool IsSendConfiguration() const { return _sendConfiguration; }
    void SetSupportsVirtualStrings(bool supportsVirtualStrings) { if (_supportsVirtualStrings != supportsVirtualStrings) { _supportsVirtualStrings = supportsVirtualStrings; _dirty = true; } }
    bool IsSupportsVirtualStrings() const { return _supportsVirtualStrings; }
    void SetVendor(int vendor) { _vendor = vendor; _dirty = true; }
    void SetModel(int model) { _model = model; _dirty = true; }
    int GetVendor() const { return _vendor; }
    int GetModel() const { return _model; }
    void AddProtocol(const std::string& protocol)
    {
        if (!SupportsProtocol(protocol))
        {
            _protocols.push_back(wxString(protocol).Lower().ToStdString());
        }
    }
    bool SupportsProtocol(const std::string& protocol)
    {
        return std::find(_protocols.begin(), _protocols.end(), wxString(protocol).Lower().ToStdString()) != _protocols.end();
    }
    bool SetModelData(unsigned char* buffer, size_t bufsize, std::list<wxByte*> extraConfig, std::string showDir);
    virtual bool IsLookedUpByControllerName() const override { return true; }
    virtual std::string GetUniverseString() const override { return ""; }
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