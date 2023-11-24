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

#include <wx/wx.h>

#include <list>
#include <map>
#include <string>
#include <functional>

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include "../UtilFunctions.h"
#include "Output.h"

class wxXmlNode;
class OutputManager;
class OutputModelManager;
class ModelManager;
class ControllerCaps;
class BaseController;
class OutputModelManager;
class Model;

#pragma region Controller Constants
// These are used to identify each output type
#define CONTROLLER_NULL "Null"
#define CONTROLLER_ETHERNET "Ethernet"
#define CONTROLLER_SERIAL "Serial"
#pragma endregion

class Controller
{
public:
    enum class ACTIVESTATE {ACTIVE, INACTIVE, ACTIVEINXLIGHTSONLY};

protected:

#pragma region Member Variables
    OutputManager* _outputManager = nullptr;   // we need this from time to time
    bool _dirty = false;                       // dirty means it needs saving
    int _id = 64001;                           // the controller id ... a unique number
    std::string _name;                         // a unique name for the controller
    std::string _description;                  // a description for the controller
    bool _ok = false;                          // controller initiated ok
    bool _autoSize = true;                    // controller flexes the number of outputs to meet the needs of xLights
    bool _fullxLightsControl = true;          // when true on upload xLights wipes all other config
    int _defaultBrightnessUnderFullControl = 100; // brightness to use when controllers dont have anything on a port
    float _defaultGammaUnderFullControl = 1.0F; // Gamma to use when controllers dont have anything on a port
    std::list<Output*> _outputs;               // the outputs on the controller
    ACTIVESTATE _active = ACTIVESTATE::ACTIVE; // output to controller is active

    bool _autoLayout = true;
    bool _autoUpload = false;
    std::string _vendor;                     // the controller vendor
    std::string _model;                      // the model of the controller
    std::string _variant;                    // the variant of the controller
    bool _suppressDuplicateFrames = false;   // should we suppress duplicate fromes
    bool _monitor = true;                    // should FPP/player monitor this output (Ping) for connectivity
    Output::PINGSTATE _lastPingResult = Output::PINGSTATE::PING_UNKNOWN; // last ping result
    bool _tempDisable = false;
    bool _fromBase = false;

    std::map<std::string, std::string> _runtimeProperties;  // place to store various properties/state/etc that may be needed at runtime
#pragma endregion

public:

    #pragma region Constructors and Destructors
    Controller(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    Controller(OutputManager* om);
    Controller(OutputManager* om, const Controller& from);
    virtual ~Controller();
    virtual wxXmlNode* Save();
    virtual Controller* Copy(OutputManager* om) = 0;
    virtual bool UpdateFrom(Controller* from);
    #pragma endregion

    #pragma region Static Functions
    // encodes/decodes string lists to indices
    static int EncodeChoices(const wxPGChoices& choices, const std::string& choice);
    static std::string DecodeChoices(const wxPGChoices& choices, int choice);
    static Controller::ACTIVESTATE EncodeActiveState(const std::string& state);
    static std::string DecodeActiveState(Controller::ACTIVESTATE state);

    static Controller* Create(OutputManager* om, wxXmlNode* node, std::string showDir);
    static void ConvertOldTypeToVendorModel(const std::string& old, std::string& vendor, std::string& model, std::string &variant);

    #pragma endregion Static Functions

    #pragma region Getters and Setters
    Output* GetOutput(int outputNumber) const; // output number is zero based
    Output* GetOutput(int32_t absoluteChannel, int32_t& startChannel) const;
    std::list<Output*> GetOutputs() const { return _outputs; }
    int GetOutputCount() const { return _outputs.size(); }
    Output* GetFirstOutput() const { wxASSERT(_outputs.size() > 0); return _outputs.front(); }

    void DeleteAllOutputs();

    int32_t GetStartChannel() const;
    int32_t GetEndChannel() const;
    int32_t GetChannels() const;
    bool ContainsChannels(uint32_t start, uint32_t end) const;

    bool IsDirty() const;
    void ClearDirty();

    void SetFromBase(bool base)
    {
        if (_fromBase != base)
        {
            _dirty = true;
            _fromBase = base;
        }
    }
    bool IsFromBase() const
    {
        return _fromBase;
    }

    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { if (_name != name) { _name = name; _dirty = true; } }

    virtual int GetId() const { return _id; }
    void EnsureUniqueId();
    void EnsureUniqueName();

    const std::string &GetDescription() const { return _description; }
    void SetDescription(const std::string& description) { if (_description != description) { _description = description; _dirty = true; } }

    bool IsAutoSize() const
    {
        return IsAutoLayout() && _autoSize && GetProtocol() != OUTPUT_PLAYER_ONLY;
    }

    void SetFullxLightsControl(bool fullxLightsControl) { if (_fullxLightsControl != fullxLightsControl) { _fullxLightsControl = fullxLightsControl; _dirty = true; } }
    bool IsFullxLightsControl() const { return _fullxLightsControl; }

    void SetDefaultBrightnessUnderFullControl(int brightness) { if (_defaultBrightnessUnderFullControl != brightness) { _defaultBrightnessUnderFullControl = brightness; _dirty = true; } }
    int GetDefaultBrightnessUnderFullControl() const { return _defaultBrightnessUnderFullControl; }

    void SetDefaultGammaUnderFullControl(float Gamma) { if (_defaultGammaUnderFullControl != Gamma) { _defaultGammaUnderFullControl = Gamma; _dirty = true; } }
    float GetDefaultGammaUnderFullControl() const { return _defaultGammaUnderFullControl; }

    bool IsEnabled() const { return std::any_of(begin(_outputs), end(_outputs), [](Output* o) { return o->IsEnabled(); }); }
    void Enable(bool enable) { for (auto& it : _outputs) { it->Enable(enable); } }

    bool IsAutoLayout() const { return _autoLayout; }
    void SetAutoLayout(bool autoLayout);

    bool IsAutoUpload() const { return _autoUpload; }
    void SetAutoUpload(bool autoUpload);

    Controller::ACTIVESTATE GetActive() const { return _active; }
    virtual bool IsActive() const;
    void SetActive(const std::string& active);
    virtual void PostSetActive()
    {}

    virtual bool CanVisualise() const;

    bool IsOk() const { return _ok; }

    const std::string &GetVendor() const { return _vendor; }
    void SetVendor(const std::string& vendor, wxPropertyGrid *grid = nullptr) { if (_vendor != vendor) { _vendor = vendor; _dirty = true; VMVChanged(grid); } }
    const std::string &GetModel() const { return _model; }
    void SetModel(const std::string& model, wxPropertyGrid *grid = nullptr) { if (_model != model) { _model = model; _dirty = true; VMVChanged(grid); } }
    const std::string &GetVariant() const { return _variant; }
    void SetVariant(const std::string& variant, wxPropertyGrid *grid = nullptr) { if (_variant != variant) { _variant = variant; _dirty = true;  VMVChanged(grid); } }
    std::string GetVMV() const;
    ControllerCaps* GetControllerCaps() const;
    void SearchForNewVendor( std::string const& vendor, std::string const& model, std::string const& variant);

    bool IsSuppressDuplicateFrames() const { return _suppressDuplicateFrames; }
    void SetSuppressDuplicateFrames(bool suppress);

    bool IsMonitoring() const { return _monitor; }
    void SetMonitoring(bool monitor);

    void SetGlobalFPPProxy(const std::string& globalFPPProxy);

    Output::PINGSTATE GetLastPingState() const { return _lastPingResult; }

    const std::string &GetRuntimeProperty(const std::string &p, const std::string &def = "") const {
        const auto &a = _runtimeProperties.find(p);
        if (a != _runtimeProperties.end()) {
            return a->second;
        }
        return def;
    }
    void SetRuntimeProperty(const std::string &p, const std::string &v) { _runtimeProperties[p] = v;}
    #pragma endregion

    #pragma region Virtual Functions
    virtual void SetId(int id) { if (_id != id) { _id = id; _dirty = true; } }

    virtual void SetTransientData(int32_t& startChannel, int& nullnumber);

    virtual bool SupportsSuppressDuplicateFrames() const { return true; }
    virtual bool SupportsUpload() const { return false; }
    virtual bool SupportsAutoUpload() const;
    virtual bool SupportsAutoLayout() const;
    virtual bool IsManaged() const = 0;
    virtual bool CanSendData() const { return true; }
    virtual void VMVChanged(wxPropertyGrid *grid = nullptr) {}

    virtual bool CanTempDisable() const { return false; }
    void TempDisable(bool disable)
    {
        if (CanTempDisable()) {
            _tempDisable = disable;
            for (const auto& it : _outputs) {
                it->TempDisable(disable);
            }
        }
    }
    bool IsTempDisable() const { return _tempDisable; }

    // true if this controller needs the user to be able to edit id
    virtual bool IsNeedsId() const { return true; }

    // Used on test dialog
    virtual std::string GetLongDescription() const { return GetName() + "\n" + GetDescription(); }

    // Long Description without Channels
    virtual std::string GetShortDescription() const { return GetLongDescription(); }

    // Used in xSchedule
    virtual std::string GetPingDescription() const { return GetName() + (IsActive() ? "" : " (Inactive)") + (IsTempDisable() ? _(" (Down)") : _("") ); }

    // return the controller type
    virtual std::string GetType() const = 0;

    // convert an output onto this controller
    virtual void Convert(wxXmlNode* node, std::string showDir);

    // true if config needs to be rebuilt
    virtual bool NeedsControllerConfig() const { return false; }

    // Maximum number of outputs this controller supports ... some only support the one
    virtual int GetMaxOutputs() const { return 1; }

    // True if we can use the controller name method and model chaining on this controller type
    virtual bool IsLookedUpByControllerName() const { return false; }

    // True if this controller type can support autosize
    virtual bool SupportsAutoSize() const { return false; }

    virtual bool SupportsFullxLightsControl() const { return false; }
    virtual bool SupportsDefaultBrightness() const { return false; }
    virtual bool SupportsDefaultGamma() const { return false; }

    virtual std::string GetIP() const { return GetResolvedIP(); }
    virtual std::string GetResolvedIP(bool forceResolve = false) const { return ""; }
    virtual std::string GetFPPProxy() const { return ""; }
    virtual std::string GetProtocol() const { return ""; }

    // Used in tooltip on model dialog
    virtual std::string GetChannelMapping(int32_t ch) const = 0;
    virtual std::string GetUniverseString() const = 0;

    virtual std::string GetColumn1Label() const { return GetType(); }
    virtual std::string GetColumn2Label() const { return ""; }
    virtual std::string GetColumn3Label() const { return GetUniverseString(); }
    virtual std::string GetColumn4Label() const { return wxString::Format("%ld [%ld-%ld]", (long)GetChannels(), (long)GetStartChannel(), (long)GetEndChannel()); }
    virtual std::string GetColumn5Label() const { return GetDescription(); }
    virtual std::string GetColumn6Label() const { return wxString::Format("%d", GetId()); }

    virtual Output::PINGSTATE Ping() { _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE; return GetLastPingState(); }
    virtual void AsyncPing() { _lastPingResult = Output::PINGSTATE::PING_UNKNOWN; }
    virtual bool CanPing() const { return false; }

    virtual std::string GetSortName() const { return GetName(); }
    virtual std::string GetExport() const = 0;
    
    virtual std::string GetJSONData() const;
    virtual std::string GetCapJSONData() const;
    
    #pragma endregion

    #pragma region Operators
    bool operator==(const Controller& controller) const { return _id == controller._id; }
    #pragma endregion

    #pragma region UI
    #ifndef EXCLUDENETWORKUI
        void AddModels(wxPGProperty* property, wxPGProperty* vp);
        void AddVariants(wxPGProperty* property);

        virtual void UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties);
        virtual void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties);
	    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager);
        virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const;
        virtual void HandleExpanded(wxPropertyGridEvent& event, bool expanded);

    #endif

    #ifndef EXCLUDEDISCOVERY
        virtual bool SetChannelSize(int32_t channels, std::list<Model*> models = {}, uint32_t universeSize = 510);
        void SetAutoSize(bool autosize, OutputModelManager* omm);
    #endif

    #pragma endregion
};
