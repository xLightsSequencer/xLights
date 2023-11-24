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
#include "ZCPP.h"

#include <wx/socket.h>

class ControllerEthernet;
class Discovery;

#define ZCPP_MAXCHANNELS (16 * 1024)

class ZCPPOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t* _data = nullptr;
    ZCPP_packet_t _packet;
    uint8_t _sequenceNum = 0;
    wxIPV4address _remoteAddr;
    wxDatagramSocket*_datagram = nullptr;
    long _lastSecond = -1;
    int _vendor = -1;
    int _model = -1;
    int _priority = 100;
    long _usedChannels = 0;
    bool _supportsVirtualStrings = false;
    bool _supportsSmartRemotes = false;
    bool _multicast = false;
    bool _dontConfigure = false;
    std::list<ZCPP_packet_t*> _extraConfig;
    std::list<ZCPP_packet_t*> _modelData;
    std::list<std::string> _protocols;
    #pragma endregion 

    #pragma region Private Functions
    void ExtractUsedChannelsFromModelData(Controller* c);
 
    void DeserialiseProtocols(const std::string& protocols);
    std::string SerialiseProtocols();
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    ZCPPOutput(Controller* c, wxXmlNode* node, std::string showdir);
    ZCPPOutput();
    ZCPPOutput(const ZCPPOutput& from);
    virtual ~ZCPPOutput() override;
    virtual wxXmlNode* Save() override;
    virtual Output* Copy() override
    {
        return new ZCPPOutput(*this);
    }
#pragma endregion 
    
    #pragma region Static Functions
    static void SendSync(const std::string& localIP);

    static void InitialiseExtraConfigPacket(ZCPP_packet_t* packet, int seq, uint8_t priority);
    static void InitialiseModelDataPacket(ZCPP_packet_t* packet, int seq, uint8_t priority, const std::string& description);

    static std::string DecodeProtocol(int protocol);
    static int EncodeProtocol(const std::string& protocol);

    static int EncodeColourOrder(const std::string& colourOrder);

    #ifndef EXCLUDEDISCOVERY
    static void PrepareDiscovery(Discovery &discovery);
    #endif

    static wxArrayString GetVendors();
    static int EncodeVendor(const std::string& vendor);
    static std::string DecodeVendor(int vendor);
    #pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_ZCPP; }
    
    virtual std::string GetLongDescription() const override;
    
    virtual int GetMaxChannels() const override { return ZCPP_MAXCHANNELS; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }
    virtual void SetChannels(int32_t channels) override;
    
    int GetId() const { return _universe; }
    void SetId(int id) { if (_universe != id) { _universe = id; _dirty = true; } }
    
    void SetSupportsVirtualStrings(bool supportsVirtualStrings) { if (_supportsVirtualStrings != supportsVirtualStrings) { _supportsVirtualStrings = supportsVirtualStrings; _dirty = true; } }
    bool IsSupportsVirtualStrings() const { return _supportsVirtualStrings; }
    
    void SetSupportsSmartRemotes(bool supportsSmartRemotes) { if (_supportsSmartRemotes != supportsSmartRemotes) { _supportsSmartRemotes = supportsSmartRemotes; _dirty = true; } }
    bool IsSupportsSmartRemotes() const { return _supportsSmartRemotes; }
    
    void SetDontConfigure(bool dontConfigure) { if (_dontConfigure != dontConfigure) { _dontConfigure = dontConfigure; _dirty = true; } }
    bool IsDontConfigure() const { return _dontConfigure; }
    
    void SetMulticast(bool multicast) { if (_multicast != multicast) { _multicast = multicast; _dirty = true; } }
    bool IsMulticast() const { return _multicast; }
    
    void SetVendor(int vendor) { if (_vendor != vendor) { _vendor = vendor; _dirty = true; } }
    int GetVendor() const { return _vendor; }

    void SetModel(int model) { if (_model != model) { _model = model; _dirty = true; } }
    int GetModel() const { return _model; }

    void SetPriority(int priority) { if (_priority != priority) { _priority = priority; _dirty = true; } }
    int GetPriority() const { return _priority; }

    void AllOn();

    void AddProtocol(const std::string& protocol) {
        if (!SupportsProtocol(protocol)) {
            _protocols.push_back(wxString(protocol).Lower().ToStdString());
        }
    }
    bool SupportsProtocol(const std::string& protocol) {
        return std::find(_protocols.begin(), _protocols.end(), wxString(protocol).Lower().ToStdString()) != _protocols.end();
    }

    bool SetModelData(Controller* c, std::list<ZCPP_packet_t*> modelData, std::list<ZCPP_packet_t*> extraConfig, std::string showDir);

    virtual std::string GetUniverseString() const override { return ""; }
    #pragma region 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 
    
    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion 
    
    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion 
    
    #pragma region UI
    #ifndef EXCLUDENETWORKUI
    virtual void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    virtual void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before, Controller *c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller *c) override;
    virtual void RemoveProperties(wxPropertyGrid* propertyGrid) override;
    #endif
    #pragma endregion 
};
