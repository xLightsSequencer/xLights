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

#include "ListenerBase.h"
#include <string>
#include <mutex>

#include <wx/wx.h>
#include <wx/sckipc.h>

class wxDatagramSocket;

class ListenerMQTT : public ListenerBase
{
    std::string _ip = "127.0.0.1";
    std::string _username = "";
    std::string _password = "";
    std::string _clientId = "xSchedule";
    int _port = 1883;
    wxSocketClient _client;
    std::list<std::string> _toSubscribe;
    std::mutex _topicLock;

public:
    ListenerMQTT(ListenerManager* _listenerManager, const std::string& ip, int port, const std::string& username = "", const std::string& password = "", const std::string& clientId = "");
    virtual ~ListenerMQTT() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "MQTT"; }
    virtual void StartProcess() override;
    virtual void StopProcess() override;
    virtual void Poll() override;
    std::string GetBrokerIP() const { return _ip; }
    int GetBrokerPort() const { return _port; }
    bool Subscribe(const std::string& topic);
};

