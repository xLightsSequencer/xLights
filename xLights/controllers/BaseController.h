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
#include "../UtilClasses.h"

class ModelManager;
class OutputManager;
class Controller;

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
    virtual bool needsHTTP_0_9() const { return false; }
    std::string GetURL(const std::string& url, const std::string& username = "", const std::string& password = "") const;
    std::string PutURL(const std::string& url, const std::string& request, const std::string& username = "", const std::string& password = "", const std::string& contentType = "x-www-form-urlencoded") const;
    #pragma endregion

public:


    #pragma region Constructors and Destructors
    BaseController() {}
    BaseController(const std::string& ip, const std::string &fppProxy);
    virtual ~BaseController() {}
    
    
#ifndef DISCOVERYONLY
    static BaseController *CreateBaseController(Controller *controller, const std::string &ip = "");
#endif

    #pragma endregion
    
    #pragma region Getters and Setters
    [[nodiscard]] bool IsConnected() const { return _connected; };

    [[nodiscard]] virtual const std::string &GetModel() const { return _model; }
    [[nodiscard]] virtual const std::string &GetVersion() const { return _version; }
    [[nodiscard]] virtual std::string GetFullName() const { return _model + ((_version == "") ? _("") : (_(" ") + _version)); }

#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(Controller* controller, wxWindow* parent) { return false; }
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) = 0;
    
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) { return false; }
    //virtual bool ResetAfterOutput(OutputManager* outputManager, Controller* controller, wxWindow* parent) { return false; }
#endif

    [[nodiscard]] virtual bool UsesHTTP() const = 0;
    
    #pragma endregion
};
