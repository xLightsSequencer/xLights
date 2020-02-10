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

#include "ControllerUploadData.h"
#include "UtilClasses.h"

class ModelManager;
class OutputManager;
class ControllerEthernet;

class BaseController
{
protected:
    #pragma region Member Variables
    std::string _fppProxy;
    std::string _ip;
    std::string _baseUrl;
    std::string _model;
    std::string _version;
    bool _connected = false;
    #pragma endregion

    #pragma region Protected Functions
    std::string GetURL(const std::string& url, bool logresult = true);
    std::string PutURL(const std::string& url, const std::string& request, bool logresult = true);
    #pragma endregion

public:
    
    #pragma region Static Functions
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {

        if (data == nullptr) return 0;
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }
    #pragma endregion

    #pragma region Constructors and Destructors
    BaseController() {}
    BaseController(const std::string& ip, const std::string &fppProxy);
    virtual ~BaseController() {}
    #pragma endregion
    
    #pragma region Getters and Setters
    bool IsConnected() const { return _connected; };

    virtual std::string GetModel() const { return _model; }
    virtual std::string GetVersion() const { return _version; }
    virtual std::string GetFullName() const { return _model + ((_version == "") ? _("") : (_(" ") + _version)); }
    virtual bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) { return false; }
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) = 0;
    virtual bool UsesHTTP() const = 0;
    #pragma endregion
};
