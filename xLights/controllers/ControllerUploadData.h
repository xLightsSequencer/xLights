#pragma once

// These classes are used to build the necessary data for uploading controller configuration

#include <string>
#include <list>
#include <map>
#include <set>

class UDControllerPort;
class UDController;
class Model;
class ModelManager;
class OutputManager;
class Output;
class wxWindow;
class Controller;
class ControllerCaps;

class UDControllerPortModel
{
private:
    int32_t _startChannel;
    int32_t _endChannel;
    int _string;
    Model* _model;
    int _universe;
    int32_t _universeStartChannel;
    std::string _protocol;
    int _smartRemote;

    bool ChannelsOnOutputs(std::list<Output*>& outputs) const;

public:
    UDControllerPortModel(Model* m, Controller* controller, OutputManager* om, int string);
    virtual ~UDControllerPortModel() {};
    bool operator<(const UDControllerPortModel& cpm) const
    {
        if (_smartRemote == cpm._smartRemote)
        {
            return _startChannel < cpm._startChannel;
        }
        return _smartRemote < cpm._smartRemote;
    }
    int GetChannelsPerPixel();
    int32_t Channels() const { return _endChannel - _startChannel + 1; }
    int32_t GetStartChannel() const { return _startChannel; }
    int32_t GetEndChannel() const { return _endChannel; }
    int GetSmartRemote() const { return _smartRemote; }
    Model* GetModel() const { return _model; }
    std::string GetName() const;
    void Dump() const;
    int GetUniverse() const { return _universe; }
    int GetUniverseStartChannel() const { return _universeStartChannel; }
    std::string GetProtocol() const { return _protocol; }
    bool Check(Controller* controller, const UDControllerPort* port, bool pixel, const ControllerCaps* rules, std::string& res) const;
    int GetBrightness(int currentBrightness);
    int GetNullPixels(int currentNullPixels);
    float GetGamma(int currentGamma);
    std::string GetColourOrder(const std::string& currentColourOrder);
    std::string GetDirection(const std::string& currentDirection);
    int GetGroupCount(int currentGroupCount);
    int GetDMXChannelOffset();
};

struct UDVirtualString
{
    int _index = -1;
    std::string _description;
    int32_t _startChannel;
    int32_t _endChannel;
    int Channels() const { 
        if (_models.size() == 0) return 0; 
        return _endChannel - _startChannel + 1; 
    }
    int _universe;
    int32_t _universeStartChannel;
    std::string _protocol;
    bool _colourOrderSet;
    std::string _colourOrder;
    bool _gammaSet;
    float _gamma;
    bool _brightnessSet;
    int _brightness;
    int _smartRemote;
    bool _nullPixelsSet;
    int _nullPixels;
    bool _groupCountSet;
    int _groupCount;
    bool _reverseSet;
    std::string _reverse;
    int _channelsPerPixel;
    std::list<UDControllerPortModel*> _models;
};

class UDControllerPort
{
	private:
		int _port;
        std::string _protocol;
		std::list<UDControllerPortModel*> _models;
        bool _valid;
        std::string _invalidReason;
        std::list<UDVirtualString*> _virtualStrings;

	public:
        UDControllerPort(int port, const std::string & protocol = "") { _protocol = protocol; _port = port; _valid = true; _invalidReason = ""; }
		~UDControllerPort();
        UDControllerPortModel* GetFirstModel() const;
        UDControllerPortModel* GetLastModel() const;
		void AddModel(Model* m, Controller* controller, OutputManager* om, int string = 0);
		int32_t GetStartChannel() const;
		int32_t GetEndChannel() const;
		int32_t Channels() const;
        int Pixels() const { return Channels() / 3; }
        std::string GetProtocol() const { return _protocol; }
        bool IsPixelProtocol() const;
		std::string GetPortName() const;
		int GetPort() const { return _port; }
        bool ContainsModel(Model* m) const;
        bool IsValid() const { return _valid; }
        void SetInvalid() { _valid = false; }
        std::string GetInvalidReason() const { return _invalidReason; }
        void Dump() const;
        bool Check(Controller* c, const UDController* controller, bool pixel, const ControllerCaps* rules, std::string& res) const;
        int GetUniverse() const;
        int GetUniverseStartChannel() const;
        void CreateVirtualStrings(bool mergeSequential);
        int GetVirtualStringCount() const { return (int)_virtualStrings.size(); }
        std::list<UDVirtualString*> GetVirtualStrings() const { return _virtualStrings; }
        UDVirtualString* GetVirtualString(int index) const
        {
            int i = 0;
            for (const auto& it : _virtualStrings)
            {
                if (i == index) return it;
                i++;
            }
            return nullptr;
        }
        std::list<UDControllerPortModel*> GetModels() const { return _models; }
};

class UDController
{
	private:
        Controller* _controller = nullptr;
		std::string _ipAddress;
        std::string _hostName;
		std::list<Output*> _outputs;
		std::map<int, UDControllerPort*> _pixelPorts;
		std::map<int, UDControllerPort*> _serialPorts;

        bool ModelProcessed(Model* m);
        std::list<Model*> _noConnectionModels;

	public:
        UDController(Controller* controller, OutputManager* om, ModelManager* mm, std::string& check);
		~UDController();
		UDControllerPort* GetControllerPixelPort(int port);
		UDControllerPort* GetControllerSerialPort(int port);
		bool IsValid(ControllerCaps* rules) const;
        int GetMaxSerialPort() const;
        int GetMaxPixelPort() const;
        void Dump() const;
        bool HasPixelPort(int port) const;
        bool HasSerialPort(int port) const;
        bool Check(const ControllerCaps* rules, std::string& res);
        Output* GetFirstOutput() const;
    
        const std::list<Model *> GetNoConnectionModels() const { return _noConnectionModels; }
};
