#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <wx/socket.h>

// A very simplified web socket class that can only send text web socket messages

class WebSocketClient
{
    wxSocketClient _socket;
    bool _connected;

    std::string ReadSocket();
    void ClearIncomingData();

public:
    WebSocketClient();
    bool Connect(std::string ip, std::string url);
    bool IsConnected() const { return _connected; }
    bool Send(std::string message);
    std::string Receive();
    virtual ~WebSocketClient();
};

#endif
