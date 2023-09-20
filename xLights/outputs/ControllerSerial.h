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

#include <list>
#include <string>

#include "Controller.h"

class wxXmlNode;
class Output;
class SerialOutput;

// An serial controller sends data to a unique com port
class ControllerSerial : public Controller
{
protected:

#pragma region Property Choices
    static wxPGChoices __types;
    static wxPGChoices __ports;
    static wxPGChoices __speeds;
    static void InitialiseTypes(bool forceXXX);
    wxPGChoices GetProtocols() const;
#pragma endregion

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
    ControllerSerial(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerSerial(OutputManager* om);
    ControllerSerial(OutputManager* om, const ControllerSerial& from);
    virtual ~ControllerSerial()
    {}
    virtual wxXmlNode* Save() override;
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

#pragma endregion

#pragma region Virtual Functions
    virtual void SetId(int id) override;

    virtual void VMVChanged(wxPropertyGrid *grid = nullptr) override;

    virtual bool IsManaged() const override { return false; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetShortDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_SERIAL; }

    void Convert(wxXmlNode* node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return false; }

    virtual bool IsLookedUpByControllerName() const override { return false; }

    virtual bool SupportsAutoSize() const override { return _type != OUTPUT_LOR_OPT; }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return wxString::Format("%d", _id); }

    virtual std::string GetColumn1Label() const override {
        if (_model == "FPP") return _type;
        if (_outputs.size() > 0) return _outputs.front()->GetType(); else return Controller::GetColumn1Label();
    }
    virtual std::string GetResolvedIP() const override {
        if (_model == "FPP") return GetFirstOutput()->GetResolvedIP();
        return "";
    }
    virtual std::string GetIP() const override {
        if (_model == "FPP" && _port.find(":") != -1) {
            return _port.substr(0, _port.find(":"));
        }
        return "";
    }
    virtual std::string GetFPPProxy() const override;

    virtual std::string GetColumn2Label() const override { return wxString::Format("%s:%d", _port, _speed); }
    virtual std::string GetProtocol() const override { return _type; }

    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override { _lastPingResult = Ping(); }

    virtual std::string GetExport() const override;
#pragma endregion 

#pragma region UI
    #ifndef EXCLUDENETWORKUI
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
        virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
        virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
    #endif
#pragma endregion
};
