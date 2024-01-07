#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wxHTTPServer/wxhttpserver.h"

class WebServer : HttpServer
{

public:

        WebServer(int port, bool apionly = false, const wxString& password = "", int mins = 30, bool allowUnauthPages = false, const std::string& defaultPage = "index.html");
        virtual ~WebServer();
        void SetAPIOnly(bool apiOnly);
        void SetPasswordTimeout(int mins);
        void SetPassword(const wxString& password);
        void GeneratePass();
        void SendMessageToAllWebSockets(const wxString& message);
        bool IsSomeoneListening() const;
        void SetAllowUnauthenticatedPagesToBypassLogin(bool allowUnauthPages);
        void SetDefaultPage(const std::string& defaultPage);
};
