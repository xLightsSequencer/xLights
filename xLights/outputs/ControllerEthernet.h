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
#include <future>

#include "Controller.h"
#include "IPOutput.h"

class wxXmlNode;
class Output;

// An ethernet controller sends data to a unique IP address
class ControllerEthernet : public Controller
{
protected:

#pragma region Property Choices
    static wxPGChoices __types;
    static void InitialiseTypes(bool forceXXX);
    wxPGChoices GetProtocols() const;
#pragma endregion

#pragma region Member Variables
    std::string _ip;
    std::string _resolvedIp;
    std::string _type;
    bool _forceSizes = false;
    int _priority = 100;
    bool _managed = true;
    std::string _fppProxy;
    std::future<Output::PINGSTATE> _asyncPing;
#pragma endregion

public:

#pragma region Constructors and Destructors
    ControllerEthernet(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerEthernet(OutputManager* om, bool acceptDuplicates = false);
    virtual ~ControllerEthernet();
    virtual wxXmlNode* Save() override;
#pragma endregion

#pragma region Getters and Setters
    void SetIP(const std::string& ip);
    std::string GetIP() const { return _ip; }
    std::string GetResolvedIP() const { return _resolvedIp; }

    std::string GetProtocol() const { return _type; }
    void SetProtocol(const std::string& protocol);

    void SetFPPProxy(const std::string& proxy);
    std::string GetControllerFPPProxy() const { return _fppProxy; }
    std::string GetFPPProxy() const;

    bool IsFPPProxyable() const { return _type == OUTPUT_E131 || _type == OUTPUT_DDP; }

    void SetManaged(bool managed) { if (_managed != managed) { _managed = managed; _dirty = true; } }

    int GetPriority() const { return _priority; }
    void SetPriority(int priority);

    bool AllSameSize() const;
#pragma endregion

#pragma region Virtual Functions
    virtual void SetId(int id) override;
    virtual bool IsNeedsId() const override { return (GetProtocol() != OUTPUT_E131 && GetProtocol() != OUTPUT_ARTNET && GetProtocol() != OUTPUT_xxxETHERNET && GetProtocol() != OUTPUT_OPC); }

    virtual bool IsManaged() const override { return _managed; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_ETHERNET; }

    void Convert(wxXmlNode* node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return GetProtocol() == OUTPUT_ZCPP; }

    virtual bool IsLookedUpByControllerName() const override { return true; }

    virtual bool SupportsAutoSize() const override { return _managed && IsAutoLayout(); }

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return ""; }

    virtual std::string GetColumn1Label() const override { if (_outputs.size() > 0) return _outputs.front()->GetType() + (!_managed ? " (Unmanaged)" : ""); else return Controller::GetColumn1Label(); }
    virtual std::string GetColumn2Label() const override { return _ip; }
    virtual std::string GetColumn3Label() const override;

    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override;
    virtual bool CanPing() const override { return GetIP() != "MULTICAST"; }

    virtual std::string GetExport() const override;

    virtual void SetTransientData(int32_t& startChannel, int& nullnumber) override;

    virtual bool SupportsUpload() const override;

    virtual bool SetChannelSize(int32_t channels) override;
#pragma endregion

#pragma region UI
    #ifndef EXCLUDENETWORKUI
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager) override;
        virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
        virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
    #endif
#pragma endregion UI
};
