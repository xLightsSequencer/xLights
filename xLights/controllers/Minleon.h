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

#include <wx/xml/xml.h>
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <list>

#include "BaseController.h"
#include "ControllerUploadData.h"

class MinleonString;

class Minleon : public BaseController
{
    #pragma region Member Variables
    std::vector<MinleonString*> _stringPorts;
    std::string _version;
    std::string _protocol;
    int _startUniverse = -1;
    int _ports = 0;
    int _grouping = 0;
    std::string _nm;
    std::string _gw;
    int _t0h = 0;
    int _t1h = 0;
    int _tbit = 0;
    int _tres = 0;
    std::string _chip = "ws2811";
    std::string _conv;
    bool _ndbPro = false;
    #pragma endregion

    #pragma region Private Functions

    #pragma region MinleonString Handling
    void ParseStringPorts(std::vector<MinleonString*>& stringPorts, wxJSONValue& val) const;
    void InitialiseStrings(std::vector<MinleonString*>& stringsData, int max) const;
    std::string BuildStringPort(MinleonString* string) const;
    MinleonString* FindPort(const std::vector<MinleonString*>& stringData, int port) const;
    int GetPixelCount(const std::vector<MinleonString*>& stringData, int port) const;
    int GetMaxPixelPort(const std::vector<MinleonString*>& stringData) const;
    void DumpStringData(std::vector<MinleonString*> stringData, int startUniverse) const;
    void SetTimingsFromProtocol();
    void PostURL(const std::string& url, const std::string& data) const;
    int GetMax16PortPixels(const std::string& chip) const;
    int GetMax8PortPixels(const std::string& chip) const;
    #pragma endregion

    #pragma region Port Handling
    void Upload(bool reboot);
    void UploadNDBPro(bool reboot);
    #pragma endregion

    #pragma region Encode and Decode
    int EncodeStringPortProtocol(const std::string& protocol) const;
    std::string DecodeStringPortProtocol(int protocol) const;
    int EncodeInputProtocol(const std::string& protocol) const;
    std::string DecodeInputProtocol(int protocol) const;
    #pragma endregion

    #pragma endregion

public:
    #pragma region Constructors and Destructors
    Minleon(const std::string& ip, const std::string &proxy, const std::string& forceLocalIP);
    virtual ~Minleon();
    #pragma endregion

    #pragma region Static Functions
    #pragma endregion

    #pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
    #pragma endregion
};

