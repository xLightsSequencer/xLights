#ifndef HinksPix_H
#define HinksPix_H

#include <wx/protocol/http.h>
#include <list>
#include <string>
#include <memory>
#include "ControllerUploadData.h"
#include "UtilClasses.h"

class ModelManager;
class Output;
class OutputManager;
class HinksPix;

class HinksPixControllerRules : public ControllerRules
{
    int _outputs;
    bool _flex;

public:
    HinksPixControllerRules(bool flex) : ControllerRules(), _flex(flex)
    {
        if (_flex) 
            _outputs = 16;
        else
            _outputs = 48;
    }
    virtual ~HinksPixControllerRules() {}

    virtual const std::string GetControllerId() const override
    {
        if(_flex)
            return "HinksPix PRO";
        else
            return "EasyLights Pix16";
    }
    virtual const std::string GetControllerManufacturer() const override { return "HinksPix"; };

    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }

    virtual int GetMaxPixelPortChannels() const override
    {
            return 680 * 3;

    }
    virtual int GetMaxPixelPort() const override
    {
        return _outputs;
    }
    virtual int GetMaxSerialPortChannels() const override
    {
        return 512;
    }
    virtual int GetMaxSerialPort() const override
    {
        return 1;
    }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "ws2811") return true;
        if (p == "ws2801") return true;
        if (p == "tls3001") return true;
        if (p == "apa102") return true;

        return false;
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "dmx") return true;

        return false;
    }
    virtual bool SupportsMultipleProtocols() const override { return true; }
    virtual bool SupportsSmartRemotes() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return false; }
    virtual bool UniversesMustBeSequential() const override { return true; }
    virtual std::set<std::string> GetSupportedInputProtocols() const override
    {
        std::set<std::string> res = { "E131", "ARTNET", "DDP" };
        return res;
    }
    
    virtual bool SingleUpload() const override { return false; }
};

class HinksPixOutput
{
public:
    HinksPixOutput(int output_) :
        output(output_), universe(1),
        startChannel(1), pixels(50),
        direction(0), protocol(0),
        nullPixel(0), colorOrder(0),
        brightness(100), gamma(1),
        controlerStartChannel(1),
        controlerEndChannel(150)
    { };
    const int output;
    int universe;
    int startChannel;
    int pixels;
    int direction;
    int protocol;
    int nullPixel;
    int colorOrder;
    int brightness;
    int gamma;

    int getControllerStartChannel() const { return controlerStartChannel;}
    int getControllerEndChannel() const { return controlerEndChannel; }
    void setControllerChannels(const int startChan);

    void Dump() const;
    void SetConfig(wxString const& data);
    wxString BuildCommand() const;

private:
    int controlerStartChannel;
    int controlerEndChannel;
};

class HinksPixSerial
{
public:
    HinksPixSerial() :
        mode(2), e131Universe(1),
        e131StartChannel(1), e131NumOfChan(512),
        e131Enabled(false), ddpStartChannel(1),
        ddpNumOfChan(512), ddpEnabled(false),
        upload(false)
    { };
    int mode;
    int e131Universe;
    int e131StartChannel;
    int e131NumOfChan;
    bool e131Enabled;
    int ddpStartChannel;
    int ddpNumOfChan;
    bool ddpEnabled;
    bool upload;
    void Dump() const;
    void SetConfig(std::map<wxString, wxString> const& data);

    wxString BuildCommand() const;
};

class HinksPix
{
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    bool InitControllerOutputData();

    void UpdatePortData(HinksPixOutput& pd, UDControllerPort* stringData, std::map<int, int> const& uniChan) const;

    void UpdateSerialData(HinksPixSerial& pd, UDControllerPort* serialData, int mode) const;

    void UploadPixelOutputs(bool& worked);

    std::map<wxString,wxString> StringToMap(wxString const& text) const;

    void InitExpansionBoardData(int expansion, int startport, int length);

    void UploadExpansionBoardData(int expansion, int startport, int length, bool& worked);

    std::unique_ptr < HinksPixSerial> InitSerialData();

    wxString GetControllerData(int rowIndex, std::string const& data = std::string());

    wxString GetControllerE131Data(int rowIndex);

    wxString GetControllerRowData(int rowIndex, std::string const& url, std::string const& data);

    std::map<int, int> MapE131Addresses();

    int CalcControllerChannel(int universe, int startChan, std::map<int, int> const& uniChan) const;

    int EncodeColorOrder(const std::string& colorOrder) const;
    int EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;
    int EncodeBrightness(int brightness) const;
    int EncodeGamma(int gamma) const;
    std::string EncodeControllerType() const;

    static const std::string GetInfoURL() { return"/GetInfo.cgi"; };

    static const std::string GetE131URL() { return"/GetE131Data.cgi"; };

    wxHTTP _http;

    std::string _baseUrl;
    const std::string _fppProxy;
    const std::string _ip;

    int _inputMode;//e131=0, ddp=1, artnet=2

    //int _model;
    std::string _firmware;
    int _outputTypes[3];
    int _Flex;
    int _numberOfOutputs;

    std::vector<HinksPixOutput> _pixelOutputs;
    std::unique_ptr <HinksPixSerial> _serialOutput;

    bool _connected;

public:
    HinksPix(const std::string& ip, const std::string &fppProxy);
    virtual ~HinksPix();
    bool IsConnected() const { return _connected; };

    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);

    static void RegisterControllers();

    std::string GetModelName() { return EncodeControllerType(); }
    const std::string GetFirmware() const { return  _firmware; } ;
    const int GetNumberOfOutputs() { return _numberOfOutputs; }
    const int GetNumberOfSerial() { return 1; }
};
#endif