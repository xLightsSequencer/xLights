#ifndef ESPIXELSTICK_H
#define ESPIXELSTICK_H

#include <list>
#include <string>
#include "WebSocketClient.h"

class ModelManager;
class Output;
class OutputManager;

class ESPixelStick
{
    WebSocketClient _wsClient;
	std::string _ip;
    std::string _version;
    bool _connected;
    std::string DecodeStringPortProtocol(std::string protocol);
    std::string DecodeSerialPortProtocol(std::string protocol);
    std::string DecodeSerialSpeed(std::string protocol);
    int GetMaxStringOutputs() const;
    int GetMaxSerialOutputs() const;
    std::string GetFromJSON(std::string section, std::string key, std::string json);

public:
    ESPixelStick(const std::string& ip);
    bool IsConnected() const { return _connected; };
    virtual ~ESPixelStick();
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
};

#endif
