#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "wxHTTPServer/wxhttpserver.h"

class WebServer : HttpServer
{

public:

        WebServer(int port);
        virtual ~WebServer();
};

#endif
