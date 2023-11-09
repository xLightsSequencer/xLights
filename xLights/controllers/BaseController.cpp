
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BaseController.h"
#include "ControllerCaps.h"
#include "../outputs/OutputManager.h"
#include "../models/ModelManager.h"
#include "../outputs/ControllerEthernet.h"

#include <curl/curl.h>

#ifdef __WXMSW__
#include "../utils/Curl.h"
#endif

#include <log4cpp/Category.hh>

#include "BaseController.h"
#include "Falcon.h"
#include "FPP.h"
#include "AlphaPix.h"
#include "HinksPix.h"
#include "J1Sys.h"
#include "Pixlite16.h"
#include "ESPixelStick.h"
#include "SanDevices.h"
#include "Minleon.h"
#include "WLED.h"
#include "Experience.h"
#include "utils/CurlManager.h"

#pragma region Constructors and Destructors
BaseController::BaseController(const std::string& ip, const std::string &proxy) : _ip(ip), _fppProxy(proxy), _baseUrl("") {
    if (!_fppProxy.empty()) {
        _baseUrl = "/proxy/" + _ip;
    }
}

#ifndef DISCOVERYONLY
BaseController *BaseController::CreateBaseController(Controller *controller, const std::string &ipOrig) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string ip = ipOrig;
    ControllerCaps *caps = controller->GetControllerCaps();
    if (!caps) {
        logger_base.error("Unable to get controller capabilities.");
        return nullptr;
    }
    std::string vendor = controller->GetVendor();
    BaseController* bc = nullptr;

    if (ipOrig == "") {
        ip = controller->GetResolvedIP();
        if (ip == "MULTICAST") {
            logger_base.error("Unable to upload to a multicast controller.");
            return nullptr;
        }
    }
    auto proxy = controller->GetFPPProxy();
    std::string flip = "";
    if (dynamic_cast<ControllerEthernet*>(controller) != nullptr) {
        flip = dynamic_cast<ControllerEthernet*>(controller)->GetFirstOutput()->GetForceLocalIPToUse();
    }
    
    
    std::string driver = caps->GetConfigDriver();
    if (driver == "Falcon") {
        bc = new Falcon(ip, proxy);
    } else if (driver == "PixLite16") {
        bc = new Pixlite16(ip);
    } else if (driver == "ESPixelStick") {
        bc = new ESPixelStick(ip);
    } else if (driver == "J1Sys") {
        bc = new J1Sys(ip, proxy);
    } else if (driver == "SanDevices") {
        bc = new SanDevices(ip, proxy);
    } else if (driver == "HinksPix") {
        bc = new HinksPix(ip, proxy);
    } else if (driver == "AlphaPix") {
        bc = new AlphaPix(ip, proxy);
    } else if (driver == "FPP") {
        bc = new FPP(ip, proxy, caps->GetModel());
    } else if (driver == "Minleon") {
        bc = new Minleon(ip, proxy, flip);
    } else if (driver == "Experience") {
        bc = new Experience(ip, proxy);
    } else if (driver == "WLED") {
        bc = new WLED(ip, proxy);
    } else {
        logger_base.warn("Vendor not recognized ... assuming it is a FPP based vendor : %s.", (const char*)vendor.c_str());
        bc = new FPP(ip, proxy, caps->GetModel());
    }
    return bc;
}
#endif

#pragma endregion

#pragma region Protected Functions
std::string BaseController::GetURL(const std::string& url, const std::string& username, const std::string& password) const{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    auto furl = std::string("http://" + baseIP + _baseUrl + url);
    if (!username.empty()) {
        CurlManager::INSTANCE.setHostUsernamePassword(baseIP, username, password);
    }
    if (needsHTTP_0_9() && _fppProxy.empty()) {
        CurlManager::INSTANCE.setHostAllowHTTP_0_9(baseIP, true);
    }
    int rc = 0;
    std::string res = CurlManager::INSTANCE.doGet(furl, rc);
    if (rc == 0 && !needsHTTP_0_9()) {
        logger_base.error("Failure to access %s: %s.", (const char*)furl.c_str(), res.c_str());
        return "";
    }
    return res;
}

std::string BaseController::PutURL(const std::string& url, const std::string& request, const std::string& username, const std::string& password, const std::string& contentType) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    logger_base.debug("Making request to Controller '%s'.", (const char*)url.c_str());
    logger_base.debug("    With data '%s'.", (const char*)request.c_str());
    
    auto furl = std::string("http://" + baseIP + _baseUrl + url);
    if (!username.empty()) {
        CurlManager::INSTANCE.setHostUsernamePassword(baseIP, username, password);
    }
    if (needsHTTP_0_9() && _fppProxy.empty()) {
        CurlManager::INSTANCE.setHostAllowHTTP_0_9(baseIP, true);
    }
    int rc = 0;
    std::string res = CurlManager::INSTANCE.doPost(furl, contentType, request, rc);
    if (rc == 0 && !needsHTTP_0_9()) {
        logger_base.error("Failure to post to %s: %s.", (const char*)furl.c_str(), res.c_str());
        return "";
    }
    return res;
}
#pragma endregion
