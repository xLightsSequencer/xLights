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
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    ControllerSerial(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerSerial(OutputManager* om);
    virtual ~ControllerSerial() {}
    virtual wxXmlNode* Save() override;
#pragma endregion

#pragma region Getters and Setters
    void SetPort(const std::string& port);
    std::string GetPort() const { return _port; }

    void SetSpeed(int speed);
    int GetSpeed() const { return _speed; }

    void SetChannels(int channels);
    void SetProtocol(const std::string& type);

    std::string GetProtocol() const { return _type; }
#pragma endregion

#pragma region Virtual Functions
    virtual void SetId(int id) override;

    virtual bool IsManaged() const override { return false; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_SERIAL; }

    void Convert(wxXmlNode* node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return false; }

    virtual bool IsLookedUpByControllerName() const override { return false; }

    virtual bool SupportsAutoSize() const override { return _type != OUTPUT_LOR_OPT; }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return wxString::Format("%d", _id); }

    virtual std::string GetColumn1Label() const override { if (_outputs.size() > 0) return _outputs.front()->GetType(); else return Controller::GetColumn1Label(); }
    virtual std::string GetColumn2Label() const override { return wxString::Format("%s:%d", _port, _speed); }

    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override { _lastPingResult = Ping(); }

    virtual std::string GetExport() const override;
#pragma endregion 

#pragma region UI
    #ifndef EXCLUDENETWORKUI
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager) override;
        virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
        virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
    #endif
#pragma endregion
};
