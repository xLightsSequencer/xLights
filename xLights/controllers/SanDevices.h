#ifndef SanDevices_H
#define SanDevices_H

#include <wx/protocol/http.h>
#include <list>
#include <string>
#include "ControllerUploadData.h"
#include "UtilClasses.h"

class ModelManager;
class Output;
class OutputManager;
class SanDevices;

class SanDevicesControllerRules : public ControllerRules
{
    int _type;
    int _firmware;

public:
    SanDevicesControllerRules(int type, int firmware) : ControllerRules()
    {
        _type = type;
        _firmware = firmware;
    }
    virtual ~SanDevicesControllerRules() {}

    virtual const std::string GetControllerId() const override
    {
        return wxString::Format("E%d Firmware %d", _type, _firmware).ToStdString();
    }
    virtual const std::string GetControllerManufacturer() const override { return "SanDevices"; };

    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }

    virtual int GetMaxPixelPortChannels() const override
    {
        return 999 * 3;

    }
    virtual int GetMaxPixelPort() const override
    {
        if (_type == 6804)
        {
            return 4;
        }

        if (_type == 682)
        {
            return 16;
        }

        return -1;
    }
    virtual int GetMaxSerialPortChannels() const override
    {
        return 512;
    }
    virtual int GetMaxSerialPort() const override
    {
       return 4;
    }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "ws2811") return true;
        if (p == "tm18xx") return true;
        if (p == "ws2801") return true;
        if (p == "tls3001" && _firmware == 4) return true;
        if (p == "apa102" && _firmware == 5) return true;
        if (p == "lpd8806" && _firmware == 5) return true;
        if (p == "lpd6803") return true;
        if (p == "gece") return true;

        return false;
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "dmx") return true;
        if (p == "renard") return true;

        return false;
    }
    virtual bool SupportsMultipleProtocols() const override { return true; }
    virtual bool SupportsSmartRemotes() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return false; }
    virtual bool UniversesMustBeSequential() const override { return false; }
    virtual std::set<std::string> GetSupportedInputProtocols() const override
    {
        std::set<std::string> res = { "E131", "ARTNET" };
        return res;
    }
    
};

class SanDevicesOutput
{
public:
    SanDevicesOutput(int group_, int output_, int stringport_) :
        group(group_), output(output_), stringport(stringport_),
        universe('A'), startChannel(1), pixels(0), groupCount(0),
        nullPixel(0), colorOrder('A'), reverse(false),
        brightness('A'), firstZig(0), thenEvery(0),
        chase(false),upload(false)
    { };
    const int group;
    const int output;
    const int stringport;
    char universe;
    int startChannel;
    int pixels;
    int groupCount;
    int nullPixel;
    char colorOrder;
    bool reverse;
    char brightness;
    int firstZig;
    int thenEvery;
    bool chase;
    bool upload;
    void Dump() const;
};

class SanDevicesOutputV4
{
public:
    SanDevicesOutputV4(int group_) :
        group(group_), protocol('D'), outputSize(0),
        universe('A'), startChannel(1), pixels(0), groupCount(0), 
        colorOrder('A'), zigzag(0), upload(false)
    {
    };
    const int group;
    char protocol;
    int outputSize;
    char universe;
    int startChannel;
    int pixels;
    int groupCount;
    int nullPixel[4]{0, 0, 0, 0};
    int colorOrder;
    bool reverse[4]{ false,false,false,false };
    int zigzag;
    bool upload;
    void Dump() const;
};

class SanDevicesProtocol
{
private:
    const int _group;
    char _protocol;
    char _timing;
    bool _upload;
public:
    
    SanDevicesProtocol(int group, char protocol, char timing) :
        _group(group), _protocol(protocol), _timing(timing), _upload(false)
    { };
    SanDevicesProtocol(int group, char protocol) : SanDevicesProtocol(group, protocol, 'A') {};
    void Dump() const;
    int getGroup() const { return _group; };
    char getProtocol() const { return _protocol; };
    char getTiming() const { return _timing; };
    bool shouldUpload() const { return _upload; };
    void setProtocol(char protocol)
    {
       if( _protocol != protocol)
       {
           _upload = true;
       }
       _protocol = protocol;
    }
    void setTiming(char timing)
    {
        if (_timing != timing)
        {
            _upload = true;
        }
        _timing = timing;
    }
};

// I had to write this http class as wxHTTP is unable to handle the SanDevices
class SimpleHTTP : public wxHTTP
{
    bool MyBuildRequest(const wxString& path, const wxString& method, wxString& startResult);

public:
    SimpleHTTP() : wxHTTP() { }
    virtual ~SimpleHTTP() { }
    wxInputStream *GetInputStream(const wxString& path, wxString& startResult);
};

class SanDevices
{
    enum class FirmwareVersion { Unknown = -1, Four = 4, Five = 5 }; // enum class
    enum class SanDeviceModel { Unknown = -1, E6804 = 6804, E682 = 682 }; // enum class

    SimpleHTTP _http;
    std::string _baseUrl;
    std::string _fppProxy;
    std::string _ip;
    std::string _page;
    FirmwareVersion _firmware;
    SanDeviceModel _model;
    std::string _version;
    std::vector<SanDevicesOutput*> _outputData;
    std::vector<SanDevicesProtocol*> _protocolData;
    std::vector<int> _universes;

    std::vector < SanDevicesOutputV4*> _outputDataV4;

    bool _connected;
    bool SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
    bool SetOutputsV5(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
    bool ParseV4Webpage(const std::string& page);
    bool ParseV5MainWebpage(const std::string& page);
    bool ParseV5OutputWebpage(const std::string& page);

    std::string GetURL(const std::string& url, bool logresult = false);
    char EncodeStringPortProtocolV4(const std::string& protocol) const;
    char EncodeStringPortProtocolV5(const std::string& protocol) const;
    char EncodeSerialPortProtocolV5(const std::string& protocol) const;
    char EncodeUniverseSize(int universesize) const;
    char EncodeColorOrderV5(const std::string& colorOrder) const;
    int EncodeColorOrderV4(const std::string& colorOrder) const;
    char EncodeBrightness(int brightness) const;
    bool EncodeDirection(const std::string& direction) const;

    void ResetStringOutputs();
    static int GetNumberOfOutputGroups() { return 4; };
    static int GetMaxSerialOutputs() { return 4; };
    int GetOutputsPerGroup() const;
    int GetMaxStringOutputs() const { return (GetNumberOfOutputGroups() * GetOutputsPerGroup()); };

    std::string ExtractFromPage(const std::string& page, const std::string& parameter, const std::string& type, int start = 0);
    int ExtractIntFromPage(const std::string& page, const std::string& parameter, const std::string& type, int defaultValue = 0, int start = 0);
    char ExtractCharFromPage(const std::string& page, const std::string& parameter, const std::string& type, char defaultValue = 'A', int start = 0);
    bool ExtractBoolFromPage(const std::string& page, const std::string& parameter, const std::string& type, bool defaultValue = false, int start = 0);

    char EncodeUniverse(int universe) const;

    SanDeviceModel DecodeControllerType(const std::string& modelName) const;

    std::pair<int, int > DecodeOutputPort(const int output) const;

    int EncodeXlightsOutput(const int group, const int subport) const;
    int EncodeControllerPortV5(const int group, const int subport) const;

    bool IsFirmware4() const { return _firmware == FirmwareVersion::Four; }
    bool IsFirmware5() const { return _firmware == FirmwareVersion::Five; }
    bool IsE682() const { return _model == SanDeviceModel::E682; }
    bool IsE6804() const { return _model == SanDeviceModel::E6804; }
    std::string EncodeControllerType() const;

    SanDevicesProtocol* ExtractProtocalDataV5(const std::string& page, int group);
    SanDevicesOutput* ExtractOutputDataV5(const std::string& page, int group, int port);
    SanDevicesOutputV4* ExtractOutputDataV4(const std::string& page, int group);

    void UpdatePortDataV5(int group, int output, UDControllerPort* stringData);
    SanDevicesOutput* FindPortDataV5(int group, int output);

    void UpdatePortDataV4(SanDevicesOutputV4* pd, UDControllerPort* stringData) const;
    void UpdateSubPortDataV4(SanDevicesOutputV4* pd, int subport, UDControllerPort* stringData) const;
    SanDevicesOutputV4* FindPortDataV4(int group);

    std::string GenerateOutputURLV5(SanDevicesOutput* outputData);
    std::string GenerateProtocolURLV5(SanDevicesProtocol* protocolData);

    std::string GenerateOutputURLV4(SanDevicesOutputV4* outputData);

public:
    SanDevices(const std::string& ip, const std::string &fppProxy);
    bool IsConnected() const { return _connected; };
    virtual ~SanDevices();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);

    static void RegisterControllers();
    
    std::string GetModelName() { return EncodeControllerType(); }
    const std::string &GetVersion() { return _version; }
    std::string GetPixelControllerTypeString() { return wxString::Format("E%d Firmware %d", _model, _firmware).ToStdString(); }
};

#endif
