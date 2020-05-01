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
