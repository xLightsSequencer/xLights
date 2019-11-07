#ifndef ESPIXELSTICK_H
#define ESPIXELSTICK_H

#include <list>
#include <string>
#include "WebSocketClient.h"
#include "ControllerUploadData.h"

class ModelManager;
class Output;
class OutputManager;

class ESPixelStickControllerRules : public ControllerRules
{
public:
    ESPixelStickControllerRules() : ControllerRules() {}
    virtual ~ESPixelStickControllerRules() {}
    virtual const std::string GetControllerId() const override {
        return std::string("ESPixelStick");
    }
    virtual const std::string GetControllerDescription() const override {
        return std::string("ESPixelStick");
    }
    virtual const std::string GetControllerManufacturer() const override {
        return "ESPixelStick";
    }
    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }
    virtual int GetMaxPixelPortChannels() const override { return 1360 * 3; }
    virtual int GetMaxPixelPort() const override { return 1; }
    virtual int GetMaxSerialPortChannels() const override { return 0; } // not implemented yet
    virtual int GetMaxSerialPort() const override { return 0; } // not implemented yet
    virtual bool IsValidPixelProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "ws2811" || p == "gece");
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override
    {
        wxString p(protocol);
        p = p.Lower();
        return (p == "renard" || p == "dmx");
    }
    virtual bool SupportsMultipleProtocols() const override { return false; }
    virtual bool SupportsSmartRemotes() const override { return false; }
    virtual bool SupportsMultipleInputProtocols() const override { return false; }
    virtual bool AllUniversesSameSize() const override { return true; }
    virtual std::set<std::string> GetSupportedInputProtocols() const override {
        std::set<std::string> res = {"E131", "DDP", "ZCPP"};
        return res;
    };
    virtual bool UniversesMustBeSequential() const override { return true; }

    virtual bool SingleUpload() const override { return true; }
};

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
    
    static void RegisterControllers();
};

#endif
