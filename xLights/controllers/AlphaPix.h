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
class ControllerEthernet;

class AlphaPixOutput
{
public:
    AlphaPixOutput(int output_) :
        output(output_), universe(1),
        startChannel(1), pixels(0),
        nullPixel(0), reverse(false),
        brightness(100), zigZag(0), 
        colorOrder(0), upload(false)
    { };
    const int output;
    int universe;
    int startChannel;
    int pixels;
    int nullPixel;
    int colorOrder;
    bool reverse;
    int brightness;
    int zigZag;
    bool upload;

    void Dump() const;
};

class AlphaPixSerial
{
public:
    AlphaPixSerial(int output_) :
        output(output_), universe(1),
        enabled(false), upload(false)
    { };
    const int output;
    int universe;
    bool enabled;
    bool upload;
    void Dump() const;
};

class AlphaPixData
{
public:
    AlphaPixData() :
        protocol(0), inputMode(0)
    {
    };
    wxString name;
    int protocol;
    int inputMode;
    void Dump() const;
};

class AlphaPix
{
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    void UpdatePortData(AlphaPixOutput* pd, UDControllerPort* stringData, bool& changeColor) const;

    void UpdateSerialData(AlphaPixSerial* pd, UDControllerPort* serialData) const;

    wxString BuildStringPortRequest(AlphaPixOutput* po) const;
    wxString BuildFlexStringPortRequest(AlphaPixOutput* po) const;

    AlphaPixOutput* FindPortData(int port);
    AlphaPixSerial* FindSerialData(int port);

    bool ParseWebpage(const wxString& page, AlphaPixData &data);

    AlphaPixOutput* ExtractOutputData(const wxString& page, int port);
    AlphaPixOutput* ExtractFlexOutputData(const wxString& page, int port);
    AlphaPixSerial* ExtractSerialData(const wxString& page, int port);
    std::string ExtractName(const wxString& page);
    int ExtractInputType(const wxString& page);
    bool ExtractDMXEnabled(const wxString& page, const wxString& name);
    int ExtractDMXUniverse(const wxString& page, const wxString& name);
    int ExtractProtocol(const wxString& page);
    int ExtractColorType(const wxString& page);
    int ExtractColor(const wxString& page);
    int ExtractSingleColor(const wxString& page, const int output);

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
    static const std::string GetOutputURL(const int index) { return"/SetSPI_" + std::to_string(index); };
    static const std::string GetColorOrderURL() { return"/PixelColor"; };
    static const std::string GetDMXURL() { return"/DMX512"; };
    static const std::string GetDMXURL(const int index) { return "/DMX512_" + std::to_string(index); };
    static const std::string GetProtocolURL() { return"/PixelIC"; };
    static const std::string GetIndvColorOrderURL() { return"/RGBOrder"; };

    std::string _baseUrl;
    const std::string _fppProxy;
    const std::string _ip;
    wxString _page;
    int _model;
    wxString _firmware;

    std::vector<AlphaPixOutput*> _pixelOutputs;
    std::vector<AlphaPixSerial*> _serialOutputs;

    bool _connected;

public:
    AlphaPix(const std::string& ip, const std::string &fppProxy);
    virtual ~AlphaPix();
    bool IsConnected() const { return _connected; };

    bool SetOutputs(ControllerEthernet* controller, ModelManager* allmodels, OutputManager* outputManager, wxWindow* parent);

    void UploadPixelOutputs(bool& worked);
    void UploadFlexPixelOutputs(bool& worked);

    std::string GetModelName() { return EncodeControllerType(); }
    const std::string GetFirmware() { return _firmware; }
    const int GetNumberOfOutputs() { return _model; }
    const int GetNumberOfSerial() 
    { 
        if (_model == 4)
            return 1;
        else if (_model == 16)
            return 3;
        else if (_model == 48)
            return 2;
        return 0;
    }
};

#endif
