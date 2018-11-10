#ifndef WEBSERVER_H
#define WEBSERVER_H

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

#endif
