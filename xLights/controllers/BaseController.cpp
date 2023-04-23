
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
    std::string res;
    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;

    CURL* curl = curl_easy_init();
    if (curl) {
        auto u = std::string("http://" + baseIP + _baseUrl + url);
        logger_base.debug("Curl GET: %s", (const char*)u.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
        curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        //curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

        std::string response_string;

        if (!username.empty())
        {
            curl_easy_setopt(curl, CURLOPT_USERNAME, (const char*)username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, (const char*)password.c_str());
        }

#ifdef __WXMSW__
         curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, Curl::CurlDebug);
         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        /* Perform the request, res will get the return code */
        CURLcode r = curl_easy_perform(curl);

        if (r != CURLE_OK) {
            logger_base.error("Failure to access %s: %s.", (const char*)url.c_str(), curl_easy_strerror(r));
        }
        else {
            res = response_string;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return res;
}

std::string BaseController::PutURL(const std::string& url, const std::string& request, const std::string& username, const std::string& password, const std::string& contentType) const
{

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const baseIP = _fppProxy.empty() ? _ip : _fppProxy;
    logger_base.debug("Making request to Controller '%s'.", (const char*)url.c_str());
    logger_base.debug("    With data '%s'.", (const char*)request.c_str());

    CURL* curl = curl_easy_init();
    if (curl != nullptr) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        auto u = std::string("http://" + baseIP + _baseUrl + url);
        logger_base.debug("Curl POST: %s", (const char*)u.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

#ifdef __WXMSW__
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, Curl::CurlDebug);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers, _("content-type: application/" + contentType).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)request.c_str());
        //curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

        if (username != "")
        {
            curl_easy_setopt(curl, CURLOPT_USERNAME, (const char*)username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, (const char*)password.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_HTTP09_ALLOWED, 1L);
        std::string buffer = "";
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode  ret = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (ret == CURLE_OK) {
            return buffer;
        }
        logger_base.error("Failure to access %s: %s.", (const char*)url.c_str(), curl_easy_strerror(ret));
    }

    return "";
}
#pragma endregion
