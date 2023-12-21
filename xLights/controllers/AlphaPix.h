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
#include <string>

#include "BaseController.h"
#include "ControllerUploadData.h"
#include "../UtilClasses.h"

class AlphaPixData;
class AlphaPixOutput;
class AlphaPixSerial;

class AlphaPix : public BaseController
{
    #pragma region Member Variables
    wxString _page;
    int _modelnum = -1;
    int _revision = 1;
    std::vector<AlphaPixOutput*> _pixelOutputs;
    std::vector<AlphaPixSerial*> _serialOutputs;
    #pragma endregion

    #pragma region Private Functions
    void UpdatePortData(AlphaPixOutput* pd, UDControllerPort* stringData, bool& changeColor) const;
    void UpdateSerialData(AlphaPixSerial* pd, UDControllerPort* serialData) const;

    wxString BuildStringPortRequest(AlphaPixOutput* po) const;
    wxString BuildStringPortRequestV2(AlphaPixOutput* po) const;

    AlphaPixOutput* FindPortData(int port);
    AlphaPixSerial* FindSerialData(int port);

    bool ParseWebpage(const wxString& page, AlphaPixData &data);

    AlphaPixOutput* ExtractOutputData(const wxString& page, int port);
    AlphaPixOutput* ExtractOutputDataV2(const wxString& page, int port);
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

    int EncodeColorOrder(const std::string& colorOrder) const;
    bool EncodeDirection(const std::string& direction) const;
    int EncodeStringPortProtocol(const std::string& protocol) const;
    std::string SafeDescription(const std::string description) const;

    void UploadPixelOutputs(bool& worked);
    void UploadFlexPixelOutputs(bool& worked);

    const std::string GetFirmware() { return _version; }
    const int GetNumberOfOutputs() { return _modelnum; }
    const int GetNumberOfSerial() {
        if (_modelnum == 4)
            return 1;
        else if (_modelnum == 16)
            return 3;
        else if (_modelnum == 48)
            return 2;
        return 0;
    }

    std::string APGetURL(const std::string& url) const;
    std::string APPutURL(const std::string& url, const std::string& request) const;
    #pragma endregion

    #pragma region Private Static Functions
    static const std::string GetNameURL() { return"/ABOUT"; };
    static const std::string GetInputTypeURL() { return"/InputSignal"; };
    static const std::string GetOutputURL() { return"/SetSPI"; };
    static const std::string GetOutputURL(const int index) { return"/SetSPI_" + std::to_string(index); };
    static const std::string GetColorOrderURL() { return"/PixelColor"; };
    static const std::string GetDMXURL() { return"/DMX512"; };
    static const std::string GetDMXURL(const int index) { return "/DMX512_" + std::to_string(index); };
    static const std::string GetProtocolURL() { return"/PixelIC"; };
    static const std::string GetIndvColorOrderURL() { return"/RGBOrder"; };
    #pragma endregion

public:
    
    #pragma region Constructors and Destructors
    AlphaPix(const std::string& ip, const std::string &fppProxy);
    virtual ~AlphaPix();
    #pragma endregion

#pragma region Static Functions
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data)
    {
        if (data == nullptr)
            return 0;
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }
#pragma endregion
    
    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    virtual bool needsHTTP_0_9() const override { return true; }
    #pragma endregion
};
