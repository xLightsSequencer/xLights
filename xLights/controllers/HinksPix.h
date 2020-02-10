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

#include <wx/protocol/http.h>

#include <list>
#include <string>
#include <memory>

#include "ControllerUploadData.h"
#include "UtilClasses.h"
#include "BaseController.h"

class HinksPix;

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

class HinksPix : public BaseController
{
    #pragma region Member Variables
    wxHTTP _http;
    int _inputMode;//e131=0, ddp=1, artnet=2
    std::string _firmware;
    int _outputTypes[3];
    int _Flex;
    int _numberOfOutputs;

    std::vector<HinksPixOutput> _pixelOutputs;
    std::unique_ptr <HinksPixSerial> _serialOutput;
    #pragma endregion 

    #pragma region Encode and Decode
    int EncodeColorOrder(const std::string& colorOrder) const;
    int EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;
    int EncodeBrightness(int brightness) const;
    int EncodeGamma(int gamma) const;
    #pragma endregion

    #pragma region Private Functions
    bool InitControllerOutputData();
    void InitExpansionBoardData(int expansion, int startport, int length);
    std::unique_ptr < HinksPixSerial> InitSerialData();
    void UpdatePortData(HinksPixOutput& pd, UDControllerPort* stringData, std::map<int, int> const& uniChan) const;
    void UpdateSerialData(HinksPixSerial& pd, UDControllerPort* serialData, int mode) const;
    void UploadPixelOutputs(bool& worked);
    void UploadExpansionBoardData(int expansion, int startport, int length, bool& worked);
    wxString GetControllerData(int rowIndex, std::string const& data = std::string());
    wxString GetControllerE131Data(int rowIndex);
    wxString GetControllerRowData(int rowIndex, std::string const& url, std::string const& data);
    int CalcControllerChannel(int universe, int startChan, std::map<int, int> const& uniChan) const;
    std::map<int, int> MapE131Addresses();
    std::map<wxString,wxString> StringToMap(wxString const& text) const;
    static const std::string GetInfoURL() { return"/GetInfo.cgi"; };
    static const std::string GetE131URL() { return"/GetE131Data.cgi"; };
    const int GetNumberOfOutputs() { return _numberOfOutputs; }
    const int GetNumberOfSerial() { return 1; }

    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }
    #pragma endregion

public:
    #pragma region Constructors and Destructors
    HinksPix(const std::string& ip, const std::string &fppProxy);
    virtual ~HinksPix();
    #pragma endregion

    #pragma region Getters and Setters
    bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) override;
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion

};
