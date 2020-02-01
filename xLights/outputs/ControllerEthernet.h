#pragma once

#include <list>
#include <string>
#include <future>

#include "Controller.h"
#include "IPOutput.h"

class wxXmlNode;
class Output;

// An ethernet controller sends data to a unique IP address
class ControllerEthernet : public Controller
{
protected:

    static wxPGChoices __types;
    static void InitialiseTypes(bool forceXXX);
    bool AllSameSize() const;

#pragma region Member Variables
    std::string _ip;
    std::string _resolvedIp;
    std::string _type;
    bool _forceSizes = false;
    int _priority = 100;
    bool _managed = true;
    std::string _fppProxy;
    std::future<Output::PINGSTATE> _asyncPing;
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    ControllerEthernet(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerEthernet(OutputManager* om, bool acceptDuplicates = false);
    virtual ~ControllerEthernet() { if (_asyncPing.valid()) _asyncPing.wait_for(std::chrono::seconds(2)); }
    void Convert(wxXmlNode* node, std::string showDir); // loads a legacy networks node
#pragma endregion Constructors and Destructors

#pragma region Static Functions
#pragma endregion Static Functions

#pragma region Getters and Setters
    void SetIP(const std::string& ip);
    std::string GetIP() const { return _ip; }
    std::string GetResolvedIP() const { return _resolvedIp; }
    std::string GetProtocol() const { return _type; }
    void SetProtocol(const std::string& protocol);
    void SetFPPProxy(const std::string& proxy) { if (_fppProxy != proxy) { _fppProxy = proxy; _dirty = true; } }
    //void SetType(const std::string& type);
    std::string GetControllerFPPProxy() const { return _fppProxy; }
    std::string GetFPPProxy() const;
    virtual bool NeedsControllerConfig() const override { return false; }
    virtual bool IsLookedUpByControllerName() const override { return false; }
    virtual bool IsAutoLayoutModels() const override { return false; }
    virtual std::string GetType() const override { return CONTROLLER_ETHERNET; }
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetColumn1Label() const override { if (_outputs.size() > 0) return _outputs.front()->GetType() + (!_managed ? " (Unmanaged)" : ""); else return Controller::GetColumn1Label(); }
    virtual std::string GetColumn2Label() const override { return _ip; }
    virtual std::string GetColumn3Label() const override;
    virtual void SetTransientData(int& cn, int& on, int32_t& startChannel, int& nullnumber) override;
    virtual bool SupportsAutoStartChannels() const override { return _managed; }
    virtual bool SupportsAutoSize() const override { return _managed && (_type == OUTPUT_ZCPP || _type == OUTPUT_DDP); }
    bool IsFPPProxyable() const { return _type == OUTPUT_E131 || _type == OUTPUT_DDP; }
    void SetManaged(bool managed) { if (_managed != managed) { _managed = managed; _dirty = true; } }
    virtual std::string GetUniverseString() const override { return ""; }
    bool IsManaged() const { return _managed; }
    virtual bool SupportsUpload() const override;
    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override;

    int GetPriority() const { return _priority; }
    void SetPriority(int priority);
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;

#pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
    virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
#endif
#pragma endregion UI
};

