#ifndef ZCPPOUTPUT_H
#define ZCPPOUTPUT_H

#include "IPOutput.h"
#include <wx/socket.h>
#include "ZCPP.h"

class ControllerEthernet;

#define ZCPP_MAXCHANNELS (16 * 1024)

class ZCPPOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t* _data;
    ZCPP_packet_t _packet;
    uint8_t _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;
    long _lastSecond;
    int _vendor;
    int _model;
    int _priority = 100;
    long _usedChannels;
    bool _supportsVirtualStrings = false;
    bool _supportsSmartRemotes = false;
    bool _multicast = false;
    bool _dontConfigure = false;
    std::list<ZCPP_packet_t*> _extraConfig;
    std::list<ZCPP_packet_t*> _modelData;
    std::list<std::string> _protocols;
    #pragma endregion Member Variables

    void ExtractUsedChannelsFromModelData();
    void DeserialiseProtocols(const std::string& protocols);
    std::string SerialiseProtocols();

public:

    #pragma region Constructors and Destructors
    ZCPPOutput(wxXmlNode* node, std::string showdir);
    ZCPPOutput();
    ZCPPOutput(ZCPPOutput* output);
    virtual ~ZCPPOutput() override;
    #pragma endregion Constructors and Destructors
    
    #pragma region Static Functions
    static void SendSync();
    static std::list<ControllerEthernet*> Discover(OutputManager* outputManager);
    static void InitialiseExtraConfigPacket(ZCPP_packet_t* packet, int seq, uint8_t priority);
    static void InitialiseModelDataPacket(ZCPP_packet_t* packet, int seq, uint8_t priority, const std::string& description);
    static std::string DecodeProtocol(int protocol);
    static int EncodeProtocol(const std::string& protocol);
    static int EncodeColourOrder(const std::string& colourOrder);
    static wxArrayString GetVendors();
    static int EncodeVendor(const std::string& vendor);
    static std::string DecodeVendor(int vendor);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool NeedsControllerConfig() const override { return true; }
    virtual std::string GetType() const override { return OUTPUT_ZCPP; }
    virtual std::string GetLongDescription() const override;
    virtual int GetMaxChannels() const override { return ZCPP_MAXCHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
    virtual int32_t GetEndChannel() const override;
    virtual void SetChannels(int32_t channels) override;
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    void SetSupportsVirtualStrings(bool supportsVirtualStrings) { if (_supportsVirtualStrings != supportsVirtualStrings) { _supportsVirtualStrings = supportsVirtualStrings; _dirty = true; } }
    bool IsSupportsVirtualStrings() const { return _supportsVirtualStrings; }
    void SetSupportsSmartRemotes(bool supportsSmartRemotes) { if (_supportsSmartRemotes != supportsSmartRemotes) { _supportsSmartRemotes = supportsSmartRemotes; _dirty = true; } }
    bool IsSupportsSmartRemotes() const { return _supportsSmartRemotes; }
    void SetDontConfigure(bool dontConfigure) { if (_dontConfigure != dontConfigure) { _dontConfigure = dontConfigure; _dirty = true; } }
    bool IsDontConfigure() const { return _dontConfigure; }
    void SetMulticast(bool multicast) { if (_multicast != multicast) { _multicast = multicast; _dirty = true; } }
    bool IsMulticast() const { return _multicast; }
    void SetVendor(int vendor) { _vendor = vendor; _dirty = true; }
    void SetModel(int model) { _model = model; _dirty = true; }
    void SetPriority(int priority) { _priority = priority; _dirty = true; }
    int GetVendor() const { return _vendor; }
    int GetModel() const { return _model; }
    int GetPriority() const { return _priority; }
    void AllOn();
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
    bool SetModelData(std::list<ZCPP_packet_t*> modelData, std::list<ZCPP_packet_t*> extraConfig, std::string showDir);
    virtual bool IsLookedUpByControllerName() const override { return true; }
    virtual bool IsAutoLayoutModels() const override { return true; }
    virtual std::string GetUniverseString() const override { return ""; }
    virtual std::string GetExport() const override;
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
    virtual void SetTransientData(int& on, int32_t& startChannel, int nullnumber) override;
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting
    
//    #pragma region UI
//#ifndef EXCLUDENETWORKUI
//    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
//#endif
//    #pragma endregion UI
};
#endif
