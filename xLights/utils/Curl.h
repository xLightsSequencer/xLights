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

#include "../../include/curl/curl.h"

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/url.h>

#include <string>
#include <algorithm>

#include <log4cpp/Category.hh>

class Curl {
public:
    struct Var {
        std::string key;
        std::string value;
        Var(const std::string& k, const std::string& v)
        {
            key = k;
            value = v;
        }
    };
    typedef struct Var Var;
    
    static std::string HTTPSPost(const std::string& url, const wxString& body, const std::string& user = "", const std::string& password = "", const std::string& contentType = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int* responseCode = nullptr);
    
    static std::string HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {});

    static std::string HTTPSGet(const std::string& s, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int* responseCode = nullptr);

    static bool HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user = "", const std::string& password = "", int timeout = 10, wxProgressDialog* prog = nullptr, bool keepProgress = false);
    
    static bool HTTPUploadFile(const std::string& url, const std::string& filename, const std::string& file, std::function<bool(int, std::string)> dlg, const std::string& username = "", const std::string& password = "");

    static int CurlDebug(CURL* handle, curl_infotype type, char* data, size_t size, void* userp);
};
