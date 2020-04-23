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

#include "wxHTTPServer/wxhttpserver.h"

class WebServer : HttpServer
{

public:

        WebServer(int port, bool apionly = false, const wxString& password = "", int mins = 30);
        virtual ~WebServer();
        void SetAPIOnly(bool apiOnly);
        void SetPasswordTimeout(int mins);
        void SetPassword(const wxString& password);
        void SendMessageToAllWebSockets(const wxString& message);
        bool IsSomeoneListening() const;
};
