#pragma once

#include <wx/wx.h>

#include <list>
#include <map>
#include <string>

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "Output.h"

class wxXmlNode;
class OutputManager;
class OutputModelManager;

#pragma region Controller Constants
// These are used to identify each output type
#define CONTROLLER_NULL "Null"
#define CONTROLLER_ETHERNET "Ethernet"
#define CONTROLLER_SERIAL "Serial"
#pragma endregion 

class Controller
{
protected:

#pragma region Member Variables
    OutputManager* _outputManager = nullptr;
    bool _dirty = false;         // dirty means it needs saving
    int _id = 64001;             // the controller id ... a unique number
    std::string _name;           // a unique name for the controller
    std::string _description;    // a description for the controller
    int _controllerNumber = -1;  // cached ordinal of this controller ... may change when reordered or other output are changed
    bool _ok = false;            // controller initiated ok
    bool _autoSize = false;      // controller flexes the number of outputs to meet the needs of xLights
    bool _autoStartChannels = false;
    std::list<Output*> _outputs; // the outputs on the controller
    bool _active = true;         // output to controller is active
    std::string _vendor;
    std::string _model;
    std::string _firmwareVersion;
    bool _suppressDuplicateFrames = false;
    Output::PINGSTATE _lastPingResult = Output::PINGSTATE::PING_UNKNOWN;
    #pragma endregion Member Variables

    static int EncodeChoices(const wxPGChoices& choices, const std::string& choice);
    static std::string DecodeChoices(const wxPGChoices& choices, int choice);

public:

    #pragma region Constructors and Destructors
    Controller(OutputManager* om, wxXmlNode* node, const std::string& showDir);
    Controller(OutputManager* om);
    virtual ~Controller();
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static Controller* Create(OutputManager* om, wxXmlNode* node, std::string showDir);
    static std::list<Controller*> Discover(OutputManager* outputManager) { return std::list<Controller*>(); } // Discovers controllers supporting this connection type
    static void ConvertOldTypeToVendorModel(const std::string& old, std::string& vendor, std::string& model);
    #pragma endregion Static Functions

    #pragma region Getters and Setters

    Output* GetOutput(int outputNumber) const; // output number is zero based
    Output* GetOutput(int32_t absoluteChannel, int32_t& startChannel) const;
    std::list<Output*> GetOutputs() const { return _outputs; }
    void DeleteAllOutputs();
    int32_t GetStartChannel() const;
    int32_t GetEndChannel() const;
    uint32_t GetChannels() const;

    bool IsDirty() const;
    void ClearDirty();

    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { if (_name != name) { _name = name; _dirty = true; } }
    int GetId() const { return _id; }
    virtual void SetId(int id) = 0;
    void SetTheId(int id) { if (_id != id) { _id = id; _dirty = true; } }
    void EnsureUniqueId();
    std::string GetDescription() const { return _description; }
    void SetDescription(const std::string& description) { if (_description != description) { _description = description; _dirty = true; } }
    void SetAutoSize(bool autosize) { if (_autoSize != autosize) { _autoSize = autosize; _dirty = true; } }
    bool IsAutoSize() const { return _autoSize; }
    void SetAutoStartChannel(bool autosc) { if (_autoStartChannels != autosc) { _autoStartChannels = autosc; _dirty = true; } }
    bool IsAutoStartChannel() const { return _autoStartChannels; }
    bool IsEnabled() const { return std::any_of(begin(_outputs), end(_outputs), [](Output* o) { return o->IsEnabled(); }); }
    void Enable(bool enable) { for (auto& it : _outputs) { it->Enable(enable); } }
    int GetControllerNumber() const { return _controllerNumber; }
    virtual void SetTransientData(int& cn, int& on, int32_t& startChannel, int& nullnumber);
    bool IsOk() const { return _ok; }
    int GetOutputCount() const { return _outputs.size(); }
    bool IsActive() const { return _active; }
    void SetActive(bool active) { if (_active != active) { _active = active;  _dirty = true; } }
    std::string GetVendor() const { return _vendor; }
    void SetVendor(const std::string& vendor) { if (_vendor != vendor) { _vendor = vendor; _dirty = true; } }
    std::string GetModel() const { return _model; }
    void SetModel(const std::string& model) { if (_model != model) { _model = model; _dirty = true; } }
    std::string GetFirmwareVersion() const { return _firmwareVersion; }
    void SetFirmwareVersion(const std::string& firmwareVersion) { if (_firmwareVersion != firmwareVersion) { _firmwareVersion = firmwareVersion; _dirty = true; } }
    bool IsSuppressDuplicateFrames() const { return _suppressDuplicateFrames; }
    void SetSuppressDuplicateFrames(bool suppress);
    Output* GetFirstOutput() const { wxASSERT(_outputs.size() > 0); return _outputs.front(); }
    std::string GetVMF() const;

    virtual bool SupportsAutoStartChannels() const { return false; }
    virtual bool SupportsSuppressDuplicateFrames() const { return true; }
    virtual bool SupportsUpload() const {
        return false;
    }

    // Used on test dialog
    virtual std::string GetLongDescription() const { return GetName() + "\n" + GetDescription(); }

    // Used in xSchedule
    virtual std::string GetPingDescription() const { return GetName(); }

    // return the controller type
    virtual std::string GetType() const = 0;

    virtual void Convert(wxXmlNode* node, std::string showDir);

    // true if ... XXXXXXXXXXX
    virtual bool NeedsControllerConfig() const { return false; }
    
    // Maximum number of outputs this controller supports ... some only support the one
    virtual int GetMaxOutputs() const { return 1; }

    // True if we can use the controller name method and model chaining on this controller type
    virtual bool IsLookedUpByControllerName() const { return false; }

    // True if this controller type can support autosize
    virtual bool SupportsAutoSize() const { return false; }

    // True if we can switch models around at will
    virtual bool IsAutoLayoutModels() const { return false; }

    // Used in tooltip on model dialog
    virtual std::string GetChannelMapping(int32_t ch) const = 0;
    virtual std::string GetUniverseString() const = 0;

    virtual std::string GetColumn1Label() const { return GetType(); }
    virtual std::string GetColumn2Label() const { return ""; }
    virtual std::string GetColumn3Label() const { return GetUniverseString(); }
    std::string GetColumn4Label() const { return wxString::Format("%ld [%ld-%ld]", (long)GetChannels(), (long)GetStartChannel(), (long)GetEndChannel()); }
    virtual std::string GetColumn5Label() const { return GetDescription(); }
    virtual std::string GetColumn6Label() const { return wxString::Format("%d", GetId()); }

    Output::PINGSTATE GetLastPingState() const { return _lastPingResult; }
    virtual Output::PINGSTATE Ping() { _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE; return GetLastPingState(); }
    virtual void AsyncPing() { _lastPingResult = Output::PINGSTATE::PING_UNKNOWN; }
    virtual bool CanPing() const { return false; }

    virtual std::string GetSortName() const { return GetName(); }
    virtual std::string GetExport() const = 0;
    #pragma endregion Getters and Setters

    #pragma region Operators
    bool operator==(const Controller& controller) const { return _id == controller._id; }
    #pragma endregion Operators

    virtual wxXmlNode* Save();

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual void AddProperties(wxPropertyGrid* propertyGrid);
	virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager);
    virtual void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const;
#endif
    #pragma endregion UI
};
