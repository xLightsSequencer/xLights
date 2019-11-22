#ifndef AlphaPix_H
#define AlphaPix_H

#include <list>
#include <string>
#include "ControllerUploadData.h"
#include "UtilClasses.h"

class ModelManager;
class Output;
class OutputManager;
class AlphaPix;

class AlphaPixControllerRules : public ControllerRules
{
    int _model;

public:
    AlphaPixControllerRules(int model) : ControllerRules()
    {
        _model = model;
    }
    virtual ~AlphaPixControllerRules() {}

    virtual const std::string GetControllerId() const override
    {
        return wxString::Format("AlphaPix %d", _model).ToStdString();
    }
    virtual const std::string GetControllerManufacturer() const override { return "AlphaPix"; };

    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }

    virtual int GetMaxPixelPortChannels() const override
    {
        if (_model == 4)
        {
            return 680 * 3;
        }
        else if (_model == 16)
        {
            return 340 * 3;
        }
        return 0;

    }
    virtual int GetMaxPixelPort() const override
    {
        return _model;
    }
    virtual int GetMaxSerialPortChannels() const override
    {
        return 512;
    }
    virtual int GetMaxSerialPort() const override
    {
        if (_model == 4)
        {
            return 1;
        }
        else if (_model == 16)
        {
            return 4;
        }
        return 0;
    }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "ws2811") return true;
        if (p == "tm18xx") return true;
        if (p == "ws2801") return true;
        if (p == "tls3001") return true;
        if (p == "lpd6803") return true;

        return false;
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        if (p == "dmx") return true;

        return false;
    }
    virtual bool SupportsMultipleProtocols() const override { return false; }
    virtual bool SupportsSmartRemotes() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return true; }
    virtual bool UniversesMustBeSequential() const override { return false; }
    virtual std::set<std::string> GetSupportedInputProtocols() const override
    {
        std::set<std::string> res = { "E131", "ARTNET" };
        return res;
    }
    
    virtual bool SingleUpload() const override { return true; }
};

class AlphaPixOutput
{
public:
    AlphaPixOutput(int output_) :
        output(output_), universe(1),
        startChannel(1), pixels(0),
        nullPixel(0), reverse(false),
        brightness(100), zigZag(0)
    { };
    const int output;
    int universe;
    int startChannel;
    int pixels;
    int nullPixel;
    bool reverse;
    int brightness;
    int zigZag;

    void Dump() const;
};

class AlphaPixData
{
public:
    AlphaPixData() :
        protocol(0), useDmx(false), dmxUniverse(1),
        colorOrder(0), inputMode(0)
    {
    };
    wxString name;
    int protocol;
    bool useDmx;
    int dmxUniverse;
    int colorOrder;
    int inputMode;
    void Dump() const;
};

class AlphaPix
{
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    void UpdatePortData(AlphaPixOutput* pd, UDControllerPort* stringData) const;

    wxString BuildStringPortRequest(AlphaPixOutput* po) const;

    AlphaPixOutput* FindPortData(int port);

    bool ParseWebpage(const wxString& page, AlphaPixData &data);

    AlphaPixOutput* ExtractOutputData(const wxString& page, int port);
    std::string ExtractName(const wxString& page);
    int ExtractInputType(const wxString& page);
    bool ExtractDMXEnabled(const wxString& page);
    int ExtractDMXUniverse(const wxString& page);
    int ExtractProtocol(const wxString& page);
    int ExtractColor(const wxString& page);

    std::string ExtractFromPage(const wxString& page, const std::string& parameter, const std::string& type, int start = 0);
    int ExtractIntFromPage(const wxString& page, const std::string& parameter, const std::string& type, int defaultValue = 0, int start = 0);
    bool ExtractBoolFromPage(const wxString& page, const std::string& parameter, const std::string& type, bool defaultValue = false, int start = 0);

    wxString GetURL(const std::string& url, bool logresult = true);
    wxString PutURL(const std::string& url, const std::string& request, bool logresult = true);

    int EncodeColorOrder(const std::string& colorOrder) const;
    bool EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;
    std::string EncodeControllerType() const;
    std::string SafeDescription(const std::string description) const;

    static const std::string GetNameURL() { return"/ABOUT"; };
    static const std::string GetInputTypeURL() { return"/InputSignal"; };
    static const std::string GetOutputURL() { return"/SetSPI"; };
    static const std::string GetColorOrderURL() { return"/PixelColor"; };
    static const std::string GetDMXURL() { return"/DMX512"; };
    static const std::string GetProtocolURL() { return"/PixelIC"; };

    std::string _baseUrl;
    const std::string _fppProxy;
    const std::string _ip;
    wxString _page;
    int _model;
    wxString _firmware;

    std::vector<AlphaPixOutput*> _outputData;

    bool _connected;

public:
    AlphaPix(const std::string& ip, const std::string &fppProxy);
    virtual ~AlphaPix();
    bool IsConnected() const { return _connected; };

    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);

    std::string GetModelName() { return EncodeControllerType(); }
    const std::string GetFirmware() { return _firmware; }
    const int GetNumberOfOutputs() { return _model; }
    const int GetNumberOfSerial() { return _model / 4; }

    static void RegisterControllers();
};

#endif
