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

// These classes are used to build the necessary data for uploading controller configuration

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

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
    #pragma region Member Varaibles
    int32_t _startChannel = -1;
    int32_t _endChannel = -1;
    int _string = -1;
    Model* _model = nullptr;
    int _universe = -1;
    int32_t _universeStartChannel = -1;
    std::string _protocol;
    int _smartRemote = -1;
    #pragma endregion

    #pragma region Private Functions
    bool ChannelsOnOutputs(const std::list<Output*>& outputs) const;
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    UDControllerPortModel(Model* m, Controller* controller, OutputManager* om, int string);
    virtual ~UDControllerPortModel() {};
    #pragma endregion

    #pragma region Operators
    bool operator<(const UDControllerPortModel& cpm) const {

        if (_smartRemote == cpm._smartRemote) {
            return _startChannel < cpm._startChannel;
        }
        return _smartRemote < cpm._smartRemote;
    }
    #pragma endregion

    #pragma region Getters and Setters
    int GetChannelsPerPixel() const;
    int32_t Channels() const { return _endChannel - _startChannel + 1; }
    int32_t GetStartChannel() const { return _startChannel; }
    int32_t GetEndChannel() const { return _endChannel; }
    int GetDMXChannelOffset() const;

    int GetUniverse() const { return _universe; }
    int GetUniverseStartChannel() const { return _universeStartChannel; }

    int GetSmartRemote() const { return _smartRemote; }
    char GetSmartRemoteLetter() const;

    float GetAmps(int defaultBrightness) const;
    int GetSmartTs(int currentTs) const;
    int GetBrightness(int currentBrightness) const;
    int GetStartNullPixels(int currentStartNullPixels) const;
    int GetEndNullPixels(int currentEndNullPixels) const;
    float GetGamma(int currentGamma) const;
    std::string GetColourOrder(const std::string& currentColourOrder) const;
    std::string GetDirection(const std::string& currentDirection) const;
    int GetGroupCount(int currentGroupCount) const;
    std::string GetProtocol() const { return _protocol; }
    bool IsFirstModelString() const { return _string < 1; }
    int GetString() const { return _string; }

    Model* GetModel() const { return _model; }

    std::string GetName() const;

    void Dump() const;

    bool Check(Controller* controller, const UDControllerPort* port, bool pixel, const ControllerCaps* rules, std::string& res) const;
    #pragma endregion
};

struct UDVirtualString
{
    int _index = -1;
    std::string _description;
    int32_t _startChannel = -1;
    int32_t _endChannel = -1;
    int Channels() const {
        if (_models.size() == 0) return 0;
        return _endChannel - _startChannel + 1;
    }
    int _universe = -1;
    int32_t _universeStartChannel = -1;
    std::string _protocol;
    bool _colourOrderSet = false;
    std::string _colourOrder;
    bool _gammaSet = false;
    float _gamma = 1.0;
    bool _brightnessSet = false;
    int _brightness = 100;
    int _smartRemote = false;
    bool _startNullPixelsSet = false;
    int _startNullPixels = 0;
    bool _endNullPixelsSet = false;
    int _endNullPixels = 0;
    bool _groupCountSet = false;
    int _groupCount = 1;
    bool _reverseSet = false;
    bool _tsSet = false;
    int _ts = 0;
    std::string _reverse;
    int _channelsPerPixel = -1;
    std::list<UDControllerPortModel*> _models;
};

class UDControllerPort
{
    #pragma region Member Variables
	int _port = -1;
    std::string _protocol;
	std::list<UDControllerPortModel*> _models;
    bool _valid = false;
    std::string _invalidReason;
    std::list<UDVirtualString*> _virtualStrings;
    std::string _type;
    #pragma endregion

	public:

    #pragma region Constructors and Destructors
        UDControllerPort(const std::string& type, int port, const std::string& protocol = "") { _type = type; _protocol = protocol; _port = port; _valid = true; _invalidReason = ""; }
	~UDControllerPort();
    #pragma endregion

    #pragma region Model Handling
    UDControllerPortModel* GetFirstModel() const;
    UDControllerPortModel* GetLastModel() const;
    Model* GetModelAfter(Model* m) const;
    Model* GetModelBefore(Model* m) const;
    UDControllerPortModel* GetModel(const std::string& modelName, int str) const;
    void AddModel(Model* m, Controller* controller, OutputManager* om, int string = 0, bool eliminateOverlaps = false);
    bool ContainsModel(Model* m, int string) const;
    std::list<UDControllerPortModel*> GetModels() const { return _models; }
    bool SetAllModelsToControllerName(const std::string& controllerName);
    bool SetAllModelsToValidProtocols(const std::vector<std::string>& protocols, const std::string& force);
    bool ClearSmartRemoteOnAllModels();
    bool EnsureAllModelsAreChained();
    #pragma endregion

    #pragma region Virtual String Handling
    void CreateVirtualStrings(bool mergeSequential);
    int GetVirtualStringCount() const { return (int)_virtualStrings.size(); }
    int GetModelCount() const { return (int)_models.size(); }
    std::list<UDVirtualString*> GetVirtualStrings() const { return _virtualStrings; }
    UDVirtualString* GetVirtualString(int index) const {

        int i = 0;
        for (const auto& it : _virtualStrings) {
            if (i == index) return it;
            i++;
        }
        return nullptr;
    }
    #pragma endregion

    #pragma region Getters and Setters
    int32_t GetStartChannel() const;
	int32_t GetEndChannel() const;
	int32_t Channels() const;

    int GetUniverse() const;
    int GetUniverseStartChannel() const;
    std::string GetType() const { return _type; }

    int Pixels() const { return Channels() / 3; }

    std::string GetProtocol() const { return _protocol; }
    bool IsPixelProtocol() const;
	
    float GetAmps(int defaultBrightness) const;

    std::string GetPortName() const;
	int GetPort() const { return _port; }
    
    bool IsValid() const { return _valid; }
    void SetInvalid() { _valid = false; }    
    std::string GetInvalidReason() const { return _invalidReason; }
    bool AtLeastOneModelIsUsingSmartRemote() const;
    bool AtLeastOneModelIsNotUsingSmartRemote() const;

    void Dump() const;
    bool Check(Controller* c, const UDController* controller, bool pixel, const ControllerCaps* rules, std::string& res) const;
    std::string ExportAsCSV(bool withDescription) const;
    #pragma endregion
};

class UDController
{
    #pragma region Member Variables
    Controller* _controller = nullptr;
	std::string _ipAddress;
    std::string _hostName;
	//std::list<Output*> _outputs;
	std::map<int, UDControllerPort*> _pixelPorts;
	std::map<int, UDControllerPort*> _serialPorts;
    std::list<Model*> _noConnectionModels;
    OutputManager* _outputManager = nullptr;
    ModelManager* _modelManager = nullptr;
    #pragma endregion

    #pragma region Private Functions
    bool ModelProcessed(Model* m, int string);
    #pragma endregion
	
    public:

    #pragma region Constructors and Destructors
    UDController(Controller* controller, OutputManager* om, ModelManager* mm, std::string& check, bool eliminateOverlaps);
	~UDController();
    void Rescan(bool eliminateOverlaps);
    #pragma endregion

    #pragma region Port Handling
    UDControllerPort* GetControllerPixelPort(int port);
	UDControllerPort* GetControllerSerialPort(int port);
    UDControllerPort* GetPortContainingModel(Model* model) const;
    UDControllerPortModel* GetControllerPortModel(const std::string& modelName, int str) const;
    #pragma endregion

    #pragma region Getters and Setters
    int GetMaxSerialPort() const;
    int GetMaxPixelPort() const;
    bool HasPixelPort(int port) const;
    bool HasSerialPort(int port) const;
    int GetMaxPixelPortChannels() const;

    Model* GetModelAfter(Model* m) const;
    bool HasModels() const;

    bool SetAllModelsToControllerName(const std::string& controllerName);
    bool SetAllModelsToValidProtocols(const std::vector<std::string>& pixelProtocols, const std::vector<std::string>& serialProtocols, bool allsame);
    bool ClearSmartRemoteOnAllModels();

    bool IsValid(ControllerCaps* rules) const;
    void Dump() const;
    bool Check(const ControllerCaps* rules, std::string& res);
    std::vector<std::string> ExportAsCSV(bool withDescription);
        
    Output* GetFirstOutput() const;
    
    const std::list<Model *> GetNoConnectionModels() const { return _noConnectionModels; }
    #pragma endregion
};
