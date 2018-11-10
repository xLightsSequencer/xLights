#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "wxHTTPServer/wxhttpserver.h"

class WebServer : HttpServer
{

public:

        WebServer(int port, bool apionly = false, const std::string& password = "", int mins = 30);
        virtual ~WebServer();
        void SetAPIOnly(bool apiOnly);
        void SetPasswordTimeout(int mins);
        void SetPassword(const std::string& password);
        void SendMessageToAllWebSockets(const std::string& message);
        bool IsSomeoneListening() const;
};

#endif
