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
    std::string _forceLocalIP;
    bool _forceSizes = false;
    int _priority = 100;
    int _version = 1;
    bool _managed = true;
    std::string _fppProxy;
    bool _expanded = false;
    bool _universePerString = false;
    std::future<Output::PINGSTATE> _asyncPing;
#pragma endregion

public:

#pragma region Constructors and Destructors
    ControllerEthernet(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    ControllerEthernet(OutputManager* om, bool acceptDuplicates = false);
    ControllerEthernet(OutputManager* om, const ControllerEthernet& from);
    virtual ~ControllerEthernet();
    virtual wxXmlNode* Save() override;
    virtual bool UpdateFrom(Controller* from) override;
    virtual Controller* Copy(OutputManager* om) override;
#pragma endregion

#pragma region Getters and Setters
    void SetIP(const std::string& ip);
    virtual std::string GetIP() const override { return _ip; }
    virtual std::string GetResolvedIP() const override { return _resolvedIp; }
    virtual void PostSetActive() override;

    virtual std::string GetProtocol() const override { return _type; }
    void SetProtocol(const std::string& protocol);

    std::string GetForceLocalIP() const;
    std::string GetControllerForceLocalIP() const;
    void SetForceLocalIP(const std::string& localIP);
    void SetGlobalForceLocalIP(const std::string& localIP);

    void SetFPPProxy(const std::string& proxy);
    std::string GetControllerFPPProxy() const { return _fppProxy; }
    virtual std::string GetFPPProxy() const override;

    bool IsFPPProxyable() const
    {
        // player only is proxyable because the web UI is
        return _type == OUTPUT_E131 || _type == OUTPUT_DDP || _type == OUTPUT_TWINKLY || _type == OUTPUT_PLAYER_ONLY;
    }

    bool IsForcingSizes() const { return _forceSizes; }
    void SetManaged(bool managed) { if (_managed != managed) { _managed = managed; _dirty = true; } }

    bool IsUniversePerString() const { return _universePerString; }
    void SetUniversePerString(bool ups);

    int GetPriority() const { return _priority; }
    void SetPriority(int priority);

    void AddOutput();

    int GetVersion() const { return _version; }
    void SetVersion(int version);

    bool AllSameSize() const;
#pragma endregion

#pragma region Virtual Functions
    virtual void SetId(int id) override;
    virtual int GetId() const override;
    virtual bool IsNeedsId() const override { return (GetProtocol() != OUTPUT_E131 && GetProtocol() != OUTPUT_ARTNET && GetProtocol() != OUTPUT_xxxETHERNET && GetProtocol() != OUTPUT_OPC && GetProtocol() != OUTPUT_KINET); }

    virtual bool IsManaged() const override { return _managed; }

    virtual std::string GetLongDescription() const override;

    virtual std::string GetShortDescription() const override;

    virtual std::string GetType() const override { return CONTROLLER_ETHERNET; }

    void Convert(wxXmlNode* node, std::string showDir) override; // loads a legacy networks node

    virtual bool NeedsControllerConfig() const override { return GetProtocol() == OUTPUT_ZCPP; }

    virtual bool IsLookedUpByControllerName() const override { return true; }

    virtual bool CanTempDisable() const override { return true; }

    virtual bool SupportsAutoSize() const override
    {
        return _managed && IsAutoLayout() && GetProtocol() != OUTPUT_PLAYER_ONLY;
    }
    virtual bool SupportsSuppressDuplicateFrames() const override
    {
        return GetProtocol() != OUTPUT_PLAYER_ONLY;
    }
    virtual bool SupportsFullxLightsControl() const override;
    virtual bool SupportsDefaultBrightness() const override;
    virtual bool SupportsDefaultGamma() const override;

    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetUniverseString() const override { return ""; }

    virtual std::string GetColumn1Label() const override { if (_outputs.size() > 0) return _outputs.front()->GetType() + (!_managed ? " (Unmanaged)" : ""); else return Controller::GetColumn1Label(); }
    virtual std::string GetColumn2Label() const override { return _ip; }
    virtual std::string GetColumn3Label() const override;

    virtual void VMVChanged(wxPropertyGrid *grid = nullptr) override;

    virtual Output::PINGSTATE Ping() override;
    virtual void AsyncPing() override;
    virtual bool CanPing() const override { return GetIP() != "MULTICAST"; }

    bool IsExpanded() const { return _expanded; }
    void SetExpanded(bool expanded);

    virtual std::string GetExport() const override;

    virtual void SetTransientData(int32_t& startChannel, int& nullnumber) override;

    virtual bool SupportsUpload() const override;

#pragma endregion

#pragma region UI
    #ifndef EXCLUDENETWORKUI
        bool SupportsUniversePerString() const;
    
        virtual void UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
        virtual bool HandlePropertyEvent(wxPropertyGridEvent & event, OutputModelManager * outputModelManager) override;
        virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
        virtual void HandleExpanded(wxPropertyGridEvent& event, bool expanded) override { SetExpanded(expanded); }
        virtual bool SetChannelSize(int32_t channels, std::list<Model*> = {}, uint32_t universeSize = 510) override;
#endif

    #ifndef EXCLUDEDISCOVERY
        void SetAllSameSize(bool allSame, OutputModelManager* omm);
    #endif

#pragma endregion UI
};
