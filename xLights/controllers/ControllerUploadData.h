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

#include "../ExportSettings.h"
#include "../UtilFunctions.h"
#include "../Pixels.h"

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
    std::string _smartRemoteType;
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
    int GetLightsPerNode() const;
    int32_t Channels() const { return _endChannel - _startChannel + 1; }
    int32_t GetStartChannel() const { return _startChannel; }
    int32_t GetEndChannel() const { return _endChannel; }
    int GetDMXChannelOffset() const;

    int GetUniverse() const { return _universe; }
    int GetUniverseStartChannel() const { return _universeStartChannel; }

    int GetSmartRemote() const { return _smartRemote; }
    char GetSmartRemoteLetter() const;
    std::string GetSmartRemoteType() { return _smartRemoteType; }

    float GetAmps(int defaultBrightness) const;
    int GetSmartTs(int currentTs) const;
    int GetBrightness(int currentBrightness) const;
    int GetStartNullPixels(int currentStartNullPixels) const;
    int GetEndNullPixels(int currentEndNullPixels) const;
    float GetGamma(int currentGamma) const;
    std::string GetColourOrder(const std::string& currentColourOrder) const;
    std::string GetDirection(const std::string& currentDirection) const;
    int GetGroupCount(int currentGroupCount) const;
    int GetZigZag(int currentZigZag) const;
    std::string GetProtocol() const
    {
        return _protocol;
    }
    bool IsFirstModelString() const { return _string < 1; }
    int GetString() const { return _string; }

    Model* GetModel() const { return _model; }

    std::string GetName() const;

    void Dump() const;

    bool Check(Controller* controller, const ControllerCaps* rules, std::string& res) const;
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
    bool _zigZagSet = false;
    int _zigZag = 0;
    bool _reverseSet = false;
    bool _tsSet = false;
    bool _isDummy = false;
    int _ts = 0;
    std::string _reverse;
    int _channelsPerPixel = -1;
    std::string _smartRemoteType;
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
    bool _separateUniverses = false;
    OutputManager* _om = nullptr;
    bool _isSmartRemotePort = false;
    #pragma endregion

	public:

    #pragma region Constructors and Destructors
    UDControllerPort(const std::string& type, int port, const std::string& protocol = "") { _type = type; _protocol = protocol; _port = port; _valid = true; _invalidReason = ""; }
	~UDControllerPort();
    #pragma endregion

    #pragma region Model Handling
    [[nodiscard]] UDControllerPortModel* GetFirstModel() const;
    [[nodiscard]] UDControllerPortModel* GetFirstModel(int sr) const;
    [[nodiscard]] UDControllerPortModel* GetLastModel() const;
    [[nodiscard]] Model* GetModelAfter(Model* m) const;
    [[nodiscard]] Model* GetModelBefore(Model* m) const;
    [[nodiscard]] UDControllerPortModel* GetModel(const std::string& modelName, int str) const;
    void AddModel(Model* m, Controller* controller, OutputManager* om, int string = 0, bool eliminateOverlaps = false);
    bool ContainsModel(Model* m, int string) const;
    std::list<UDControllerPortModel*> GetModels() const { return _models; }
    int CountEmptySmartRemotesBefore(int sr) const;
    bool SetAllModelsToControllerName(const std::string& controllerName);
    bool SetAllModelsToValidProtocols(const std::vector<std::string>& protocols, const std::string& force);
    bool ClearSmartRemoteOnAllModels();
    bool EnsureAllModelsAreChained();
    void TagSmartRemotePort()
    {
        _isSmartRemotePort = true;
    }
    [[nodiscard]] bool IsSmartRemotePort() const
    {
        return _isSmartRemotePort;
    }
    [[nodiscard]] int GetSmartRemoteCount() const;
    #pragma endregion

    #pragma region Virtual String Handling
    void CreateVirtualStrings(bool mergeSequential);
    [[nodiscard]] int GetVirtualStringCount() const
    {
        return (int)_virtualStrings.size();
    }
    [[nodiscard]] int GetRealVirtualStringCount() const
    {
        int count = 0;
        for (const auto& it : _virtualStrings)
        {
            if (!it->_isDummy)
                ++count;
        }
        return count;
    }
    [[nodiscard]] int GetModelCount() const
    {
        return (int)_models.size();
    }
    [[nodiscard]] int GetModelCount(int sr) const;
    [[nodiscard]] std::list<UDVirtualString*> GetVirtualStrings() const
    {
        return _virtualStrings;
    }
    [[nodiscard]] UDVirtualString* GetVirtualString(int index) const
    {

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

    int Pixels() const
    { 
        return INTROUNDUPDIV(Channels(), GetChannelsPerPixel(GetProtocol())); 
    }

    std::string GetProtocol() const { return _protocol; }
    bool IsPixelProtocol() const;

    float GetAmps(int defaultBrightness) const;
    float GetAmps(int defaultBrightness, int sr) const;

    std::string GetPortName() const;
	int GetPort() const { return _port; }

    bool IsValid() const { return _valid; }
    void SetInvalid() { _valid = false; }
    std::string GetInvalidReason() const { return _invalidReason; }
    bool AtLeastOneModelIsUsingSmartRemote() const;
    bool AtLeastOneModelIsNotUsingSmartRemote() const;
    [[nodiscard]] bool AllSmartRemoteTypesSame() const;
    [[nodiscard]] bool AllSmartRemoteTypesSame(int smartRemote) const;
    [[nodiscard]] std::string GetSmartRemoteType(int smartRemote) const;

    void SetSeparateUniverses(bool sep) { _separateUniverses = sep; }

    void Dump() const;
    bool Check(Controller* c, bool pixel, const ControllerCaps* rules, std::string& res) const;
    [[nodiscard]] std::vector<std::string> ExportAsCSV(ExportSettings::SETTINGS const& settings, float brightness) const;
    [[nodiscard]] std::string ExportAsJSON() const;
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
    std::map<int, UDControllerPort*> _virtualMatrixPorts;
    std::map<int, UDControllerPort*> _ledPanelMatrixPorts;
    std::list<Model*> _noConnectionModels;
    OutputManager* _outputManager = nullptr;
    ModelManager* _modelManager = nullptr;
    #pragma endregion

    #pragma region Private Functions
    bool ModelProcessed(Model* m, int string);
    #pragma endregion

    void ClearPorts();

    public:

    static bool IsError(const std::string& check);

    #pragma region Constructors and Destructors
    UDController(Controller* controller, OutputManager* om, ModelManager* mm, bool eliminateOverlaps);
	~UDController();
    void Rescan(bool eliminateOverlaps);
    #pragma endregion

    #pragma region Port Handling
    [[nodiscard]] UDControllerPort* GetControllerPixelPort(int port);
    [[nodiscard]] UDControllerPort* GetControllerSerialPort(int port);
    [[nodiscard]] UDControllerPort* GetControllerVirtualMatrixPort(int port);
    [[nodiscard]] UDControllerPort* GetControllerLEDPanelMatrixPort(int port);
    [[nodiscard]] UDControllerPort* GetPortContainingModel(Model* model) const;
    [[nodiscard]] UDControllerPortModel* GetControllerPortModel(const std::string& modelName, int str) const;
    #pragma endregion

    #pragma region Getters and Setters
    [[nodiscard]] int GetMaxSerialPort() const;
    [[nodiscard]] int GetMaxPixelPort() const;
    [[nodiscard]] int GetMaxLEDPanelMatrixPort() const;
    [[nodiscard]] int GetMaxVirtualMatrixPort() const;
    [[nodiscard]] bool HasPixelPort(int port) const;
    [[nodiscard]] bool HasSerialPort(int port) const;
    [[nodiscard]] bool HasLEDPanelMatrixPort(int port) const;
    [[nodiscard]] bool HasVirtualMatrixPort(int port) const;
    [[nodiscard]] int GetMaxPixelPortChannels() const;
    [[nodiscard]] int GetSmartRemoteCount(int port);
    void TagSmartRemotePorts();

    [[nodiscard]] Model* GetModelAfter(Model* m) const;
    [[nodiscard]] bool HasModels() const;

    bool SetAllModelsToControllerName(const std::string& controllerName);
    bool SetAllModelsToValidProtocols(const std::vector<std::string>& pixelProtocols, const std::vector<std::string>& serialProtocols, bool allsame);
    bool ClearSmartRemoteOnAllModels();

    [[nodiscard]] bool IsValid() const;
    void Dump() const;
    bool Check(const ControllerCaps* rules, std::string& res);
    [[nodiscard]] std::vector<std::vector<std::string>> ExportAsCSV(ExportSettings::SETTINGS const& settings, float brightness, int& columnSize);
    
    [[nodiscard]] std::string ExportAsJSON();

    Output* GetFirstOutput() const;

    const std::list<Model *> GetNoConnectionModels() const { return _noConnectionModels; }
    #pragma endregion
};
