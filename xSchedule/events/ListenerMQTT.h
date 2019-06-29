#ifndef LISTENERMQTT_H
#define LISTENERMQTT_H

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>
#include <wx/sckipc.h>

class wxDatagramSocket;

class ListenerMQTT : public ListenerBase
{
    std::string _ip = "127.0.0.1";
    std::string _username = "";
    std::string _password = "";
    int _port = 1883;
    wxSocketClient _client;

public:
    ListenerMQTT(ListenerManager* _listenerManager, const std::string& ip, int port, const std::string& username = "", const std::string& password = "");
    virtual ~ListenerMQTT() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "MQTT"; }
    virtual void StartProcess() override;
    virtual void StopProcess() override;
    virtual void Poll() override;
    std::string GetBrokerIP() const { return _ip; }
    int GetBrokerPort() const { return _port; }
    void Subscribe(const std::string& topic);
};
#endif
