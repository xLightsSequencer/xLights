#ifndef CONTROLLERUPLOADDATA_H
#define CONTROLLERUPLOADDATA_H

// These classes are used to build the necessary data for uploading controller configuration

#include <string>
#include <list>
#include <map>

class UDControllerPort;
class UDController;
class Model;
class ModelManager;
class OutputManager;
class Output;
class wxWindow;

class ControllerRules
{
public:
    ControllerRules() {}
    virtual ~ControllerRules() {}
    virtual int GetMaxPixelPortChannels() const = 0;
    virtual int GetMaxPixelPort() const = 0;
    virtual int GetMaxSerialPortChannels() const = 0;
    virtual int GetMaxSerialPort() const = 0;
    virtual bool IsValidPixelProtocol(const std::string protocol) const = 0;
    virtual bool IsValidSerialProtocol(const std::string protocol) const = 0;
    virtual bool SupportsMultipleProtocols() const = 0;
    virtual bool AllUniversesSameSize() const = 0;
    virtual std::list<std::string> GetSupportedInputProtocols() const = 0;
    virtual bool UniversesMustBeSequential() const = 0;
};

class UDControllerPortModel
{
private:
    long _startChannel;
    long _endChannel;
    int _string;
    Model* _model;
    int _universe;
    long _universeStartChannel;
    std::string _protocol;

    bool ChannelsOnOutputs(std::list<Output*>& outputs) const;

public:
    UDControllerPortModel(Model* m, OutputManager* om, int string);
    virtual ~UDControllerPortModel() {};
    bool operator<(const UDControllerPortModel& cpm) const
    {
        return _startChannel < cpm._startChannel;
    }
    long Channels() const { return _endChannel - _startChannel + 1; }
    long GetStartChannel() const { return _startChannel; }
    long GetEndChannel() const { return _endChannel; }
    Model* GetModel() const { return _model; }
    std::string GetName() const;
    void Dump() const;
    int GetUniverse() const { return _universe; }
    int GetUniverseStartChannel() const { return _universeStartChannel; }
    std::string GetProtocol() const { return _protocol; }
    bool Check(const UDControllerPort* port, bool pixel, ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const;
    int GetBrightness(int currentBrightness);
    int GetNullPixels(int currentNullPixels);
    float GetGamma(int currentGamma);
    std::string GetColourOrder(const std::string& currentColourOrder);
    std::string GetDirection(const std::string& currentDirection);
    int GetGroupCount(int currentGroupCount);
};

class UDControllerPort
{
	private:
		int _port;
        std::string _protocol;
		std::list<UDControllerPortModel*> _models;
        bool _valid;
        std::string _invalidReason;

	public:
        UDControllerPort(int port, std::string protocol = "") { _protocol = protocol; _port = port; _valid = true; _invalidReason = ""; }
		~UDControllerPort();
        UDControllerPortModel* GetFirstModel() const;
        UDControllerPortModel* GetLastModel() const;
		void AddModel(Model* m, OutputManager* om, int string = 0);
		long GetStartChannel() const;
		long GetEndChannel() const;
		long Channels() const;
        std::string GetProtocol() const { return _protocol; }
		std::string GetPortName() const;
		int GetPort() const { return _port; }
        bool ContainsModel(Model* m) const;
        bool IsValid() const { return _valid; }
        void SetInvalid() { _valid = false; }
        std::string GetInvalidReason() const { return _invalidReason; }
        void Dump() const;
        bool Check(const UDController* controller, bool pixel, ControllerRules* rules, std::list<Output*>& outputs, std::string& res) const;
        int GetUniverse() const;
        int GetUniverseStartChannel() const;
};

class UDController
{ 
	private:
		std::string _ipAddress;
		std::list<Output*> _outputs;
		std::map<int, UDControllerPort*> _pixelPorts;
		std::map<int, UDControllerPort*> _serialPorts;

        bool ModelProcessed(Model* m);

	public:
		UDController(std::string ip, ModelManager* mm, OutputManager* om, std::list<int>* outputs, std::string& check);
		~UDController();
		UDControllerPort* GetControllerPixelPort(int port);
		UDControllerPort* GetControllerSerialPort(int port);
		bool IsValid(ControllerRules* rules) const;
        int GetMaxSerialPort() const;
        int GetMaxPixelPort() const;
        void Dump() const;
        bool HasPixelPort(int port) const;
        bool HasSerialPort(int port) const;
        bool Check(ControllerRules* rules, std::string& res);
        Output* GetFirstOutput() const;
};

#endif