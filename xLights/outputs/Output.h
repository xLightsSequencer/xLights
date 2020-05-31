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

#include <list>

#include <wx/window.h>
#include <wx/time.h>

class ModelManager;
class OutputManager;
class wxXmlNode;
class OutputModelManager;
class wxPropertyGrid;
class wxPropertyGridEvent;
class ControllerEthernet;
class Controller;

#pragma region Output Constants
// These are used to identify each output type
#define OUTPUT_E131 "E131"
#define OUTPUT_ZCPP "ZCPP"
#define OUTPUT_ARTNET "ArtNet"
#define OUTPUT_DDP "DDP"
#define OUTPUT_DMX "DMX"
#define OUTPUT_NULL "NULL"
#define OUTPUT_PIXELNET "Pixelnet"
#define OUTPUT_OPENPIXELNET "Pixelnet-Open"
#define OUTPUT_LOR "LOR"
#define OUTPUT_LOR_OPT "LOR Optimised"
#define OUTPUT_DLIGHT "D-Light"
#define OUTPUT_RENARD "Renard"
#define OUTPUT_OPENDMX "OpenDMX"
#define OUTPUT_xxxSERIAL "xxx Serial"
#define OUTPUT_xxxETHERNET "xxx Ethernet"
#define OUTPUT_OPC "OPC"
#pragma endregion

class Output
{
protected:

#pragma region Member Variables
    bool _dirty = false;
    std::string _ip;
    std::string _resolvedIp;
    std::string _commPort;
    int32_t _channels = 0;
    int _baudRate = 0;
    int _universe = 0;
    bool _enabled = true;
    bool _suspend = false;
    int _nullNumber = -1; // cached ordinal of null controllers ... may change when reordered or other output are changed
    int32_t _startChannel = -1; // cached start channel of this output ... may change when reordered or other output are changed
    long _timer_msec = 0;
    bool _ok = false;
    bool _suppressDuplicateFrames = false;
    wxLongLong _lastOutputTime = 0;
    int _skippedFrames = 9999;
    bool _changed = false; // set to true when something in the packed has changed
    std::string _fppProxy;
    std::string _globalFPPProxy;
    Output *_fppProxyOutput = nullptr;

    bool _autoSize_CONVERT = false;
    std::string _description_CONVERT;
    #pragma endregion

#pragma region Private Functions
    virtual void Save(wxXmlNode* node);
#pragma endregion

public:

    enum class PINGSTATE
    {
        PING_OK,
        PING_WEBOK,
        PING_OPEN,
        PING_OPENED,
        PING_ALLFAILED,
        PING_UNAVAILABLE,
        PING_UNKNOWN
    };

    #pragma region Constructors and Destructors
    Output(wxXmlNode* node);
    Output(Output* output);
    Output();
    virtual ~Output();
    virtual wxXmlNode* Save();
    #pragma endregion 

    #pragma region Static Functions
    static Output* Create(Controller* c, wxXmlNode* node, std::string showDir);
    static std::list<ControllerEthernet*> Discover(OutputManager* outputManager) { return std::list<ControllerEthernet*>(); } // Discovers controllers supporting this protocol
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    std::string GetCommPort() const { return _commPort; }
    void SetCommPort(const std::string& commPort) { _commPort = commPort; _dirty = true; }

    int GetBaudRate() const;
    void SetBaudRate(int baudRate) { _baudRate = baudRate; _dirty = true; }

    std::string GetIP() const { return _ip; }
    virtual void SetIP(const std::string& ip);

    std::string GetResolvedIP() const { return _resolvedIp; }
    void SetResolvedIP(const std::string& resolvedIP) { if (resolvedIP != _resolvedIp) { _resolvedIp = resolvedIP; _dirty = true; } }

    const std::string GetFPPProxyIP() const { return _fppProxy; }
    void SetFPPProxyIP(const std::string& ip) { _fppProxy = ip; }
    bool IsUsingFPPProxy() const { return _fppProxy != ""; }
    void SetGlobalFPPProxyIP(const std::string& ip) { _globalFPPProxy = ip; }

    int GetUniverse() const { return _universe; }
    void SetUniverse(int universe) { _universe = universe; _dirty = true; }
    virtual std::string GetUniverseString() const { return wxString::Format(wxT("%i"), GetUniverse()).ToStdString(); }

    int32_t GetChannels() const { return _channels; }
    virtual void SetChannels(int32_t channels) { _channels = channels; _dirty = true; }
    virtual int GetMaxChannels() const = 0;
    virtual bool IsValidChannelCount(int32_t channelCount) const = 0;

    int32_t GetStartChannel() const { return _startChannel; }
    int32_t GetEndChannel() const { return _startChannel + _channels - 1; }

    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }

    bool IsEnabled() const { return _enabled; }
    void Enable(bool enable) { _enabled = enable; _dirty = true; }

    void Suspend(bool suspend) { _suspend = suspend; }

    long GetTimer() const { return _timer_msec; }

    virtual std::list<Output*> GetOutputs_CONVERT() const { std::list<Output*> res; return res; }
    std::string GetDescription_CONVERT() const { return _description_CONVERT; }
    bool IsAutoSize_CONVERT() const { return _autoSize_CONVERT; }
    virtual int GetUniverses_CONVERT() const { return 1; }
    virtual Output* GetActualOutput_CONVERT(int32_t startChannel) { return this; }
    virtual bool IsOutputCollection_CONVERT() const { return false; }

    virtual bool IsIpOutput() const = 0;
    virtual bool IsSerialOutput() const = 0;
    virtual bool IsOutputable() const { return true; }

    virtual size_t TxNonEmptyCount() const { return 0; }
    virtual bool TxEmpty() const { return true; }

    bool IsOk() const { return _ok; }

    virtual std::string GetType() const = 0;

    void SetSuppressDuplicateFrames(const bool suppressDuplicateFrames) { _suppressDuplicateFrames = suppressDuplicateFrames; _dirty = true; }
    bool IsSuppressDuplicateFrames() const { return _suppressDuplicateFrames; }

    virtual void SetTransientData(int32_t& startChannel, int nullnumber);

    virtual std::string GetLongDescription() const = 0;
    virtual std::string GetSortName() const = 0;
    virtual std::string GetExport() const { return ""; }
    #pragma endregion 

    #pragma region Operators
    bool operator==(const Output& output) const;
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open();
    virtual void Close();
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void StartFrame(long msec) { _timer_msec = msec; }
    virtual void EndFrame(int suppressFrames) = 0;
    virtual void ResetFrame() {}
    void FrameOutput();
    void SkipFrame() { _skippedFrames++; }
    bool NeedToOutput(int suppressFrames) const { return !IsSuppressDuplicateFrames() || _skippedFrames >= suppressFrames; }
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) = 0;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size);
    virtual void AllOff() = 0;
    #pragma endregion 

    virtual void SendHeartbeat() const {}

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize) {}
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) { return false; }
    #endif
    #pragma endregion UI
};
