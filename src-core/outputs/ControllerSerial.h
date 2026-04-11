#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <spdlog/fmt/fmt.h>
#include <list>
#include <string>

#include "Controller.h"

class Output;
class SerialOutput;

#ifdef SetPort
#undef SetPort  // Windows winspool.h defines SetPort as SetPortW
#endif

// An serial controller sends data to a unique com port
class ControllerSerial : public Controller
{
protected:

#pragma region Member Variables
    std::string _port;
    std::string _type = "DMX";
    int _speed = 250000;
    std::string _saveablePrefix;
    std::string _saveablePostfix;
    std::vector<uint8_t> _prefix;
    std::vector<uint8_t> _postfix;
    std::string _fppProxy;
    SerialOutput* _serialOutput = nullptr;
#pragma endregion Member Variables

public:

#pragma region Private functions
    std::vector<uint8_t> Encode(const std::string& s);
#pragma endregion

#pragma region Constructors and Destructors
    ControllerSerial(OutputManager* om, pugi::xml_node node, const std::string& showDir);
    ControllerSerial(OutputManager* om);
    ControllerSerial(OutputManager* om, const ControllerSerial& from);
    virtual ~ControllerSerial()
    {}
    virtual pugi::xml_node Save(pugi::xml_node parent) override;
    virtual bool UpdateFrom(Controller* from) override;
    virtual Controller* Copy(OutputManager* om) override;
#pragma endregion

#pragma region Getters and Setters
    void SetPort(const std::string& port);
    std::string GetPort() const { return _port; }

    void SetSpeed(int speed);
    int GetSpeed() const { return _speed; }

    void SetPostfix(const std::string& postfix);
    std::vector<uint8_t> GetPostFix() const { return _postfix; }
    std::string GetSaveablePostFix() const { return _saveablePostfix; }

    void SetPrefix(const std::string& prefix);
    std::vector<uint8_t> GetPreFix() const { return _prefix; }
    std::string GetSaveablePreFix() const { return _saveablePrefix; }

    void SetChannels(int channels);
    void SetProtocol(const std::string& type);

    void SetFPPProxy(const std::string& proxy);
    std::string GetControllerFPPProxy() const { return _fppProxy; }

    SerialOutput* GetSerialOutput() const { return _serialOutput; }

#pragma endregion

#pragma region Virtual Functions
    virtual void SetId(int id) override;

    virtual void VMVChanged() override;

    virtual bool IsManaged() const override { return false; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetShortDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_SERIAL; }

    void Convert(pugi::xml_node node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return false; }

    virtual bool IsLookedUpByControllerName() const override { return false; }

    virtual bool SupportsAutoSize() const override {
        return _type != OUTPUT_LOR_OPT && _type != OUTPUT_xxxSERIAL;
    }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return std::to_string(_id); }

    virtual std::string GetColumn1Label() const override {
        if (_model == "FPP") return _type;
        if (_outputs.size() > 0) return _outputs.front()->GetType(); else return Controller::GetColumn1Label();
    }
    virtual std::string GetResolvedIP(bool forceResolve) const override {
        if (_model == "FPP") return GetFirstOutput()->GetResolvedIP();
        return "";
    }
    virtual std::string GetIP() const override {
        if (_model == "FPP" && _port.find(":") != std::string::npos) {
            return _port.substr(0, _port.find(":"));
        }
        return "";
    }
    virtual std::string GetFPPProxy() const override;

    virtual std::string GetColumn2Label() const override { return fmt::format("{}:{}", _port, _speed); }
    virtual std::string GetProtocol() const override { return _type; }

    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override { _lastPingResult = Ping(); }

    virtual std::string GetExport() const override;
#pragma endregion 

    // UI property grid methods moved to ui/controllerproperties/ControllerSerialPropertyAdapter
};
