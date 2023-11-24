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

#include "ControllerUploadData.h"
#include "../UtilClasses.h"
#include "BaseController.h"

class SanDevices;

class SanDevicesOutput
{
public:
    SanDevicesOutput(int group_, int output_, int stringport_) : group(group_), output(output_), stringport(stringport_) { }
    const int group;
    const int output;
    const int stringport;
    char universe{ 'A' };
    int startChannel{ 1 };
    int pixels{ 0 };
    int groupCount{ 0 };
    int nullPixel{ 0 };
    char colorOrder{ 'A' };
    bool reverse{ false };
    char brightness{ 'A' };
    int firstZig{ 0 };
    int thenEvery{ 0 };
    bool chase{ false };
    bool upload{ false };
    void Dump() const;
};

class SanDevicesOutputV4
{
public:
    SanDevicesOutputV4(int group_) : group(group_) { }
    const int group;
    char protocol{ 'D' };
    int outputSize{ 0 };
    char universe{ 'A' };
    int startChannel{ 1 };
    int pixels{ 0 };
    int groupCount{ 0 };
    int nullPixel[4]{0, 0, 0, 0};
    int colorOrder = 'A';
    bool reverse[4]{ false,false,false,false };
    int zigzag{ 0 };
    bool upload{ false };
    void Dump() const;
};

class SanDevicesProtocol
{
private:
    const int _group;
    char _protocol;
    char _timing;
    bool _upload{ false };
public:
    
    SanDevicesProtocol(int group, char protocol, char timing) : _group(group), _protocol(protocol), _timing(timing) { }
    SanDevicesProtocol(int group, char protocol) : SanDevicesProtocol(group, protocol, 'A') {};

    void Dump() const;

    int getGroup() const { return _group; };
    char getProtocol() const { return _protocol; };
    char getTiming() const { return _timing; };
    bool shouldUpload() const { return _upload; };
    void setProtocol(char protocol) {
       if( _protocol != protocol) {
           _upload = true;
       }
       _protocol = protocol;
    }
    void setTiming(char timing) {
        if (_timing != timing) {
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

class SanDevices : public BaseController
{
    #pragma region Member Variables
    enum class FirmwareVersion { Unknown = -1, Four = 4, Five = 5 }; // enum class
    enum class SanDeviceModel { Unknown = -1, E6804 = 6804, E682 = 682 }; // enum class

    SimpleHTTP _http;
    std::string _page;
    FirmwareVersion _firmware;
    SanDeviceModel _sdmodel = SanDeviceModel::Unknown;
    std::vector<SanDevicesOutput*> _outputData;
    std::vector<SanDevicesProtocol*> _protocolData;
    std::vector<int> _universes;
    std::vector < SanDevicesOutputV4*> _outputDataV4;
    #pragma endregion 

    #pragma region Encode and Decode
    char EncodeStringPortProtocolV4(const std::string& protocol) const;
    char EncodeStringPortProtocolV5(const std::string& protocol) const;
    char EncodeSerialPortProtocolV5(const std::string& protocol) const;
    char EncodeUniverseSize(int universesize) const;
    char EncodeColorOrderV5(const std::string& colorOrder) const;
    int EncodeColorOrderV4(const std::string& colorOrder) const;
    char EncodeBrightness(int brightness) const;
    bool EncodeDirection(const std::string& direction) const;
    char EncodeUniverse(int universe) const;
    SanDeviceModel DecodeControllerType(const std::string& modelName) const;
    std::pair<int, int > DecodeOutputPort(const int output) const;
    int EncodeXlightsOutput(const int group, const int subport) const;
    int EncodeControllerPortV5(const int group, const int subport) const;
    #pragma endregion

    #pragma region Private Functions
    bool SetOutputsV4(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);
    bool SetOutputsV5(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent);
    bool ParseV4Webpage(const std::string& page);
    bool ParseV5MainWebpage(const std::string& page);
    bool ParseV5OutputWebpage(const std::string& page);

    std::string SDGetURL(const std::string& url, bool logresult = false);

    void ResetStringOutputs();
    static int GetNumberOfOutputGroups() { return 4; };
    int GetOutputsPerGroup() const;
    int GetMaxStringOutputs() const { return (GetNumberOfOutputGroups() * GetOutputsPerGroup()); };

    std::string ExtractFromPage(const std::string& page, const std::string& parameter, const std::string& type, int start = 0);
    int ExtractIntFromPage(const std::string& page, const std::string& parameter, const std::string& type, int defaultValue = 0, int start = 0);
    char ExtractCharFromPage(const std::string& page, const std::string& parameter, const std::string& type, char defaultValue = 'A', int start = 0);
    bool ExtractBoolFromPage(const std::string& page, const std::string& parameter, const std::string& type, bool defaultValue = false, int start = 0);


    bool IsFirmware4() const { return _firmware == FirmwareVersion::Four; }
    bool IsFirmware5() const { return _firmware == FirmwareVersion::Five; }
    bool IsE682() const { return _sdmodel == SanDeviceModel::E682; }
    bool IsE6804() const { return _sdmodel == SanDeviceModel::E6804; }

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
    #pragma endregion

public:
    #pragma region Constructors and Destructors
    SanDevices(const std::string& ip, const std::string &fppProxy);
    virtual ~SanDevices();
    #pragma endregion

    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(Controller* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    virtual bool needsHTTP_0_9() const override { return true; }
    #pragma endregion
};
