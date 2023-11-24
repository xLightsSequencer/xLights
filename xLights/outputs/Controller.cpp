
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>

#include "Controller.h"
#include "../UtilFunctions.h"
#include "Output.h"
#include "OutputManager.h"
#include "../OutputModelManager.h"
#include "../controllers/ControllerCaps.h"
#include "ControllerEthernet.h"
#include "ControllerNull.h"
#include "ControllerSerial.h"
#include "../models/Model.h"

#include <numeric>

#include <log4cpp/Category.hh>

static wxArrayString ACTIVETYPENAMES;

// This class is used to convert old controller names to the new structure
class ControllerNameVendorMap
{
public:
    #pragma region Member Variables
    std::string _oldVendor;
    std::string _oldName;
    std::string _vendor;
    std::string _model;
    std::string _variant;
    #pragma endregion

    #pragma region Constructors and Destructors
    ControllerNameVendorMap(const std::string& oldVendor, const std::string& oldName, const std::string& vendor, const std::string& model, const std::string& variant = "") :
        _oldVendor(oldVendor), _oldName(oldName), _vendor(vendor), _model(model), _variant(variant) {}
    #pragma endregion
};

const std::vector<ControllerNameVendorMap> __controllerNameMap =
{
    ControllerNameVendorMap("AlphaPix", "AlphaPix 16", "Holiday Coro", "AlphaPix 16"),
    ControllerNameVendorMap("AlphaPix", "AlphaPix 4", "Holiday Coro", "AlphaPix 4"),
    ControllerNameVendorMap("AlphaPix", "AlphaPix Flex", "Holiday Coro", "AlphaPix Flex"),
    ControllerNameVendorMap("", "ESPixelStick", "ESPixelStick", ""),

    ControllerNameVendorMap("Falcon", "F16v2", "Falcon", "F16V2R", "Two Expansion Boards"),
    ControllerNameVendorMap("Falcon", "F16v3", "Falcon", "F16V3", "Two Expansion Boards"),
    ControllerNameVendorMap("Falcon", "F48v3", "Falcon", "F48"),
    ControllerNameVendorMap("Falcon", "F4v2", "Falcon", "F4V2", "One Expansion Board"),
    ControllerNameVendorMap("Falcon", "F4v3", "Falcon", "F4V3", "One Expansion Board"),

    ControllerNameVendorMap("HinksPix", "EasyLights Pix16", "HinksPix", "EasyLights Pix16"),
    ControllerNameVendorMap("HinksPix", "HinksPix PRO", "HinksPix", "PRO"),
    ControllerNameVendorMap("J1Sys", "J1Sys P2", "J1Sys", "P2"),
    ControllerNameVendorMap("J1Sys", "J1Sys-P12D", "J1Sys", "P12D"),
    ControllerNameVendorMap("J1Sys", "J1Sys-P12R", "J1Sys", "P12R"),
    ControllerNameVendorMap("J1Sys", "J1Sys-P12S", "J1Sys", "P12S"),
    ControllerNameVendorMap("PixLite", "16", "Advatek", "PixLite 16"),
    ControllerNameVendorMap("PixLite", "16 MkII", "Advatek", "PixLite 16 MkII"),
    ControllerNameVendorMap("PixLite", "4", "Advatek", "PixLite 4"),
    ControllerNameVendorMap("PixLite", "4 MkII", "Advatek", "PixLite 4 MkII"),
    ControllerNameVendorMap("SanDevices", "E6804 Firmware 4", "SanDevices", "E6804", "4.x Firmware"),
    ControllerNameVendorMap("SanDevices", "E6804 Firmware 5", "SanDevices", "E6804", "5.x Firmware"),
    ControllerNameVendorMap("SanDevices", "E682 Firmware 4", "SanDevices", "E682", "4.x Firmware"),
    ControllerNameVendorMap("SanDevices", "E682 Firmware 5", "SanDevices", "E682", "5.x Firmware"),

    ControllerNameVendorMap("Experience Lights", "Genius Pixel 16 Controller", "Experience Lights", "Genius Pixel 16"),
    ControllerNameVendorMap("Experience Lights", "Genius Pixel 8 Controller", "Experience Lights", "Genius Pixel 8"),
    ControllerNameVendorMap("Experience Lights", "Genius Long Range Controller", "Experience Lights", "Genius Long Range"),
    ControllerNameVendorMap("Experience Lights", "Genius PRO: Pixel 16 Controller", "Experience Lights", "Genius Pro 16 Port"),
    ControllerNameVendorMap("Experience Lights", "Genius PRO: Pixel 32 Controller", "Experience Lights", "Genius Pro 32 Port")
};

#pragma region Constructors and Destructors
Controller::Controller(OutputManager* om, wxXmlNode* node, const std::string& showDir) : _outputManager(om)
{
    if (node->GetAttribute("ActiveState", "") == "") {
        if (node->GetAttribute("Active", "1") == "0") {
            SetActive("Inactive");
        } else {
            SetActive("Active");
        }
    } else {
        SetActive(node->GetAttribute("ActiveState", "Active"));
    }

    for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "network") {
            _outputs.push_back(Output::Create(this, n, showDir));
            if (_outputs.back() == nullptr) {
                // this shouldnt happen unless we are loading a future file with an output type we dont recognise
                _outputs.pop_back();
            }
        }
    }

    _id = wxAtoi(node->GetAttribute("Id", "64001"));
    _name = node->GetAttribute("Name", om->UniqueName(node->GetName() + "_")).Trim(true).Trim(false);
    _description = node->GetAttribute("Description", "").Trim(true).Trim(false);
    _fromBase = node->GetAttribute("FromBase", "0") == "1";
    _autoSize = node->GetAttribute("AutoSize", "0") == "1";
    SetAutoLayout(node->GetAttribute("AutoLayout", "1") == "1");
    _fullxLightsControl = node->GetAttribute("FullxLightsControl", "FALSE") == "TRUE";
    _defaultBrightnessUnderFullControl = wxAtoi(node->GetAttribute("DefaultBrightnessUnderFullControl", "100"));
    _defaultGammaUnderFullControl = wxAtof(node->GetAttribute("DefaultGammaUnderFullControl", "1.0"));
    SetAutoUpload(node->GetAttribute("AutoUpload", "0") == "1");
    if (!_autoLayout) _autoSize = false;
    _vendor = node->GetAttribute("Vendor");
    _model = node->GetAttribute("Model");
    _variant = node->GetAttribute("Variant");

    SetSuppressDuplicateFrames(node->GetAttribute("SuppressDuplicates", "0") == "1");
    SetMonitoring(node->GetAttribute("Monitor", "1") == "1");

    _dirty = false;
    SearchForNewVendor(_vendor, _model, _variant);
}

Controller::Controller(OutputManager* om) : _outputManager(om) {
    // everything else is initialised in the header
    if (om != nullptr) _id = om->UniqueId();
}

Controller::Controller(OutputManager* om, const Controller& from) :
    _outputManager(om)
{
    _dirty = true;
    _id = from._id;
    _name = from._name;
    _description = from._description;
    SetVendor(from.GetVendor());
    SetModel(from.GetModel());
    SetVariant(from.GetVariant());
    _autoSize = from._autoSize;
    _fullxLightsControl = from._fullxLightsControl;
    _defaultBrightnessUnderFullControl = from._defaultBrightnessUnderFullControl;
    _defaultGammaUnderFullControl = from._defaultGammaUnderFullControl;
    _active = from._active;
    _autoLayout = from._autoLayout;
    _autoUpload = from._autoUpload;
    _suppressDuplicateFrames = from._suppressDuplicateFrames;
    _monitor = from._monitor;
    for (const auto& it : from._outputs) {
        _outputs.push_back(it->Copy());
    }
}

Controller::~Controller() {
    DeleteAllOutputs();
}

wxXmlNode* Controller::Save() {

    _dirty = false;

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "Controller");
    node->AddAttribute("Id", wxString::Format("%d", _id));
    node->AddAttribute("Name", _name);
    node->AddAttribute("Description", _description);
    node->AddAttribute("Type", GetType());
    node->AddAttribute("Vendor", GetVendor());
    node->AddAttribute("Model", GetModel());
    node->AddAttribute("Variant", GetVariant());
    node->AddAttribute("AutoSize", _autoSize ? "1" : "0");
    node->AddAttribute("FromBase", _fromBase ? "1" : "0");
    if (_fullxLightsControl)
        node->AddAttribute("FullxLightsControl", "TRUE");
    node->AddAttribute("DefaultBrightnessUnderFullControl", wxString::Format("%d", _defaultBrightnessUnderFullControl));
    node->AddAttribute("DefaultGammaUnderFullControl", wxString::Format("%g", _defaultGammaUnderFullControl));
    node->AddAttribute("ActiveState", DecodeActiveState(_active));
    node->AddAttribute("AutoLayout", _autoLayout ? "1" : "0");
    node->AddAttribute("AutoUpload", _autoUpload && SupportsAutoUpload() ? "1" : "0");
    node->AddAttribute("SuppressDuplicates", _suppressDuplicateFrames ? "1" : "0");
    node->AddAttribute("Monitor", _monitor ? "1" : "0");
    for (const auto& it : _outputs) {
        node->AddChild(it->Save());
    }

    return node;
}
bool Controller::UpdateFrom(Controller* from)
{
    bool changed = false;

    if (_id != from->_id) {
        changed = true;
        _id = from->_id;
    }

    if (_name != from->_name) {
        changed = true;
        _name = from->_name;
    }

    if (_description != from->_description) {
        changed = true;
        _description = from->_description;
    }

    if (GetVendor() != from->GetVendor()) {
        changed = true;
        SetVendor(from->GetVendor());
    }

    if (GetModel() != from->GetModel()) {
        changed = true;
        SetModel(from->GetModel());
    }

    if (GetVariant() != from->GetVariant()) {
        changed = true;
        SetVariant(from->GetVariant());
    }

    if (_autoSize != from->_autoSize) {
        changed = true;
        _autoSize = from->_autoSize;
    }

    if (_fullxLightsControl != from->_fullxLightsControl) {
        changed = true;
        _fullxLightsControl = from->_fullxLightsControl;
    }

    if (_defaultBrightnessUnderFullControl != from->_defaultBrightnessUnderFullControl) {
        changed = true;
        _defaultBrightnessUnderFullControl = from->_defaultBrightnessUnderFullControl;
    }

    if (_defaultGammaUnderFullControl != from->_defaultGammaUnderFullControl) {
        changed = true;
        _defaultGammaUnderFullControl = from->_defaultGammaUnderFullControl;
    }

    if (_active != from->_active) {
        changed = true;
        _active = from->_active;
    }

    if (_autoLayout != from->_autoLayout) {
        changed = true;
        _autoLayout = from->_autoLayout;
    }

    if (_autoUpload != from->_autoUpload) {
        changed = true;
        _autoUpload = from->_autoUpload;
    }

    if (_suppressDuplicateFrames != from->_suppressDuplicateFrames) {
        changed = true;
        _suppressDuplicateFrames = from->_suppressDuplicateFrames;
    }
    if (_monitor != from->_monitor) {
        changed = true;
        _monitor = from->_monitor;
    }

    bool outputsSame = _outputs.size() == from->_outputs.size();
    if (outputsSame) {
        auto itto = _outputs.begin();
        auto itfrom = from->_outputs.begin();

        while (itto != _outputs.end() && outputsSame)
        {
            // this is probably not a great shortcut but it should catch most changes ... we can enhance it later
            if ((*itto)->GetLongDescription() != (*itfrom)->GetLongDescription())
            {
                outputsSame = false;
            }

            ++itto;
            ++itfrom;
        }
    }
    if (!outputsSame) {
        changed = true;
        DeleteAllOutputs();

        for (const auto& it : from->_outputs) {
            _outputs.push_back(it->Copy());
        }
    }

    if (changed)
    {
        _dirty = true;
    }

    return changed;
}
#pragma endregion

#pragma region Static Functions
int Controller::EncodeChoices(const wxPGChoices& choices, const std::string& choice) {
    wxString c(choice);
    c.MakeLower();

    for (size_t i = 0; i < choices.GetCount(); i++) {
        if (choices[i].GetText().Lower() == c) return i;
    }
    return -1;
}

std::string Controller::DecodeChoices(const wxPGChoices& choices, int choice)
{
    if (choice < 0 || choice >= (int)choices.GetCount()) {
        wxASSERT(false);
        return "";
    }
    return choices[choice].GetText();
}

Controller::ACTIVESTATE Controller::EncodeActiveState(const std::string& state)
{
    if (state == "Active") return ACTIVESTATE::ACTIVE;
    if (state == "Inactive") return ACTIVESTATE::INACTIVE;
    if (state == "xLights Only") return ACTIVESTATE::ACTIVEINXLIGHTSONLY;
	return ACTIVESTATE::ACTIVE;
}

std::string Controller::DecodeActiveState(Controller::ACTIVESTATE state)
{
    switch (state) {
    case ACTIVESTATE::ACTIVE:
        return "Active";
    case ACTIVESTATE::ACTIVEINXLIGHTSONLY:
        return "xLights Only";
    case ACTIVESTATE::INACTIVE:
        return "Inactive";
    }
    return "Active";
}

Controller* Controller::Create(OutputManager* om, wxXmlNode* node, std::string showDir) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string type = node->GetAttribute("Type", "").ToStdString();

    if (type == CONTROLLER_NULL) {
        return new ControllerNull(om, node, showDir);
    }
    else if (type == CONTROLLER_ETHERNET) {
        return new ControllerEthernet(om, node, showDir);
    }
    else if (type == CONTROLLER_SERIAL) {
        return new ControllerSerial(om, node, showDir);
    }

    logger_base.warn("Unknown controller type %s ignored.", (const char*)type.c_str());
    wxASSERT(false);
    return nullptr;
}

void Controller::ConvertOldTypeToVendorModel(const std::string& old, std::string& vendor, std::string& model, std::string &variant) {
    vendor = "";
    model = "";
    variant = "";
    for (const auto& it : __controllerNameMap) {
        if (it._oldName == old) {
            vendor = it._vendor;
            model = it._model;
            variant = it._variant;
            return;
        }
    }
    ControllerCaps *caps = ControllerCaps::GetControllerConfigByID(old);
    if (caps) {
        vendor = caps->GetVendor();
        model = caps->GetModel();
        variant = caps->GetVariantName();
    }
}

void Controller::SearchForNewVendor( std::string const& vendor, std::string const& model, std::string const& variant)
{
    //look if current vendor is good
    ControllerCaps* cap = ControllerCaps::GetControllerConfig(vendor, model, variant);
    if ( cap ) {
        return;
    }
    // look for controller in other "vendors" if branding changes
    ControllerCaps* newcap = ControllerCaps::GetControllerConfigByModel( model, variant);
    if (newcap && vendor != newcap->GetVendor())
    {
        _vendor = newcap->GetVendor();
        _model = newcap->GetModel();
        _variant = newcap->GetVariantName();
        _dirty = true;
        VMVChanged();
    }
}
#pragma endregion

#pragma region Getters and Setters
Output* Controller::GetOutput(int outputNumber) const {

    if (outputNumber < 0 || outputNumber > _outputs.size()) return nullptr;

    auto it = _outputs.begin();
    std::advance(it, outputNumber);
    return *it;
}

Output* Controller::GetOutput(int32_t absoluteChannel, int32_t& startChannel) const {

    for (const auto& it : GetOutputs())
    {
        if (absoluteChannel >= it->GetStartChannel() && absoluteChannel <= it->GetEndChannel())
        {
            startChannel = absoluteChannel - it->GetStartChannel() + 1;
            return it;
        }
    }
    return nullptr;
}

void Controller::DeleteAllOutputs() {

    while (_outputs.size() > 0) {
        delete _outputs.front();
        _outputs.pop_front();
    }
}

// Gets the start channel of the first output on this controller
int32_t Controller::GetStartChannel() const {

    if (_outputs.size() == 0) return 0;

    return _outputs.front()->GetStartChannel();
}

int32_t Controller::GetEndChannel() const {

    if (_outputs.size() == 0) return 0;

    return _outputs.back()->GetEndChannel();
}

int32_t Controller::GetChannels() const {
    return std::accumulate(begin(_outputs), end(_outputs), 0, [](uint32_t accumulator, Output* const o) { return accumulator + o->GetChannels(); });
}

bool Controller::ContainsChannels(uint32_t start, uint32_t end) const {
    return end >= GetStartChannel() && start < GetEndChannel();
}

bool Controller::IsDirty() const {
    if (_dirty)
        return _dirty;
    for (const auto& it : _outputs) {
        if (it->IsDirty())
            return true;
    }
    return false;
}

void Controller::ClearDirty() {
    _dirty = false;
    for (auto& it : _outputs) {
        it->ClearDirty();
    }
}

void Controller::EnsureUniqueId() {
    if (_outputManager != nullptr) _id = _outputManager->UniqueId();
}

void Controller::EnsureUniqueName() {
    if (_outputManager != nullptr) SetName(_outputManager->UniqueName(GetName()));
}

void Controller::SetAutoLayout(bool autoLayout) {
    if (_autoLayout != autoLayout) {
        _autoLayout = autoLayout;
        _dirty = true;
    }
}

void Controller::SetAutoUpload(bool autoUpload) {
    if (_autoUpload != autoUpload) {
        _autoUpload = autoUpload;
        _dirty = true;
    }
}

bool Controller::IsActive() const
{
    return _active == ACTIVESTATE::ACTIVE || (_active == ACTIVESTATE::ACTIVEINXLIGHTSONLY && IsxLights());
}

void Controller::SetActive(const std::string& active) {
    auto a = EncodeActiveState(active);
    if (_active != a) {
        _active = a;
        _dirty = true;
    }

    // always cascade it ... just in case as I have seen some cases where this gets out of sync
    for (auto& it : _outputs) {
        it->Enable(IsActive());
    }

    PostSetActive();
}

bool Controller::CanVisualise() const
{
    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        return caps->GetMaxPixelPort() > 0 || caps->GetMaxSerialPort() > 0 || caps->SupportsVirtualMatrix() || caps->SupportsLEDPanelMatrix();
    }
    return false;
}

std::string Controller::GetVMV() const {

    std::string res = GetVendor();
    if (GetModel() != "") res += " " + GetModel();
    if (GetVariant() != "") res += " - " + GetVariant();
    return res;
}

ControllerCaps* Controller::GetControllerCaps() const
{
    return ControllerCaps::GetControllerConfig(this);
}

void Controller::SetSuppressDuplicateFrames(bool suppress) {

    if (_suppressDuplicateFrames != suppress) {
        _suppressDuplicateFrames = suppress;
        _dirty = true;
        std::for_each(begin(_outputs), end(_outputs), [suppress](Output* o) { o->SetSuppressDuplicateFrames(suppress); });
    }
}
void Controller::SetMonitoring(bool monitor) {

    if (_monitor != monitor) {
        _monitor = monitor;
        _dirty = true;
    }
}

void Controller::SetGlobalFPPProxy(const std::string& globalFPPProxy)
{
    for (const auto& it : _outputs) {
        it->SetGlobalFPPProxyIP(globalFPPProxy);
    }
}
#pragma endregion

#pragma region Virtual Functions
void Controller::SetTransientData(int32_t& startChannel, int& nullnumber)
{
    for (auto& it : _outputs) {

        // make sure data which is now kept on the controller is duplicated to the outputs so they behave as expected
        it->SetSuppressDuplicateFrames(_suppressDuplicateFrames);
        it->Enable(IsActive());

        it->SetTransientData(startChannel, nullnumber);
    }
}

bool Controller::SupportsAutoLayout() const
{
    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        return caps->SupportsAutoLayout();
    }
    return false;
}

bool Controller::SupportsAutoUpload() const {
    if (GetProtocol() == OUTPUT_ZCPP) return false;

    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        return caps->SupportsAutoUpload();
    }
    return false;
}

void Controller::Convert(wxXmlNode* node, std::string showDir) {

    _dirty = true;

    if (_outputs.size() == 1) {
        _suppressDuplicateFrames = _outputs.front()->IsSuppressDuplicateFrames();
        _autoSize = _outputs.front()->IsAutoSize_CONVERT();
    }

    auto const c = node->GetAttribute("Controller");
    if (c != "") {

        for (const auto& it : __controllerNameMap) {
            if (it._oldName == c) {
                SetVendor(it._vendor);
                SetModel(it._model);
                SetVariant(it._variant);
                break;
            }
        }
        ControllerCaps *caps = ControllerCaps::GetControllerConfigByID(c);
        if (caps) {
            SetVendor(caps->GetVendor());
            SetModel(caps->GetModel());
            SetVariant(caps->GetVariantName());
        }

        // vendor didnt convert ... strange
        if (GetVendor() == "") {
            wxASSERT(false);
        }
    }
}

std::string Controller::GetJSONData() const
{
    std::string json = "{\"name\":\"" + JSONSafe(GetName()) + "\"" +
            ",\"desc\":\"" + JSONSafe(GetDescription()) + "\"" +
            ",\"type\":\"" + JSONSafe(GetType()) + "\"" +
            ",\"vendor\":\"" + JSONSafe(GetVendor()) + "\"" +
            ",\"model\":\"" + JSONSafe(GetModel()) + "\"" +
            ",\"variant\":\"" + JSONSafe(GetVariant()) + "\"" +
            ",\"protocol\":\"" + GetProtocol() + "\"" +
            ",\"id\":" + std::to_string(GetId()) +
            ",\"startchannel\":" + std::to_string(GetStartChannel()) +
            ",\"channels\":" + std::to_string(GetChannels()) +
            ",\"managed\":" + toStr(IsManaged()) +
            ",\"autolayout\":" + toStr(IsAutoLayout()) +
            ",\"canvisualise\":" + toStr(CanVisualise()) +
            ",\"active\":" + toStr(IsActive()) +
            ",\"ip\":\"" + JSONSafe(GetIP()) + "\"" +
            ",\"controllercap\":" + GetCapJSONData() + "}";
    
    return json;
}

std::string Controller::GetCapJSONData() const
{
    
    auto caps = GetControllerCaps();
    if (caps == nullptr) {
        return "{}";
    }
    std::string json = "{\"pixelports\":" + std::to_string(caps->GetMaxPixelPort()) +
            ",\"serialports\":" + std::to_string(caps->GetMaxSerialPort()) +
            ",\"supportspanels\":" + toStr(caps->SupportsLEDPanelMatrix()) +
            ",\"supportsvirtualmatrix\":" + toStr(caps->SupportsVirtualMatrix()) +
            ",\"smartremotecount\":" + std::to_string(caps->GetSmartRemoteCount()) +
            (caps->GetSmartRemoteCount() ?
            ",\"smartremotetypes\":[\"" + Join(caps->GetSmartRemoteTypes(), std::string("\",\"")) + "\"]" :
             ",\"smartremotetypes\":[]") +
            (!caps->GetPixelProtocols().empty()?
            ",\"pixelprotocols\":[\"" + Join(caps->GetPixelProtocols(), std::string("\",\"")) + "\"]" :
             ",\"pixelprotocols\":[]") +
            (!caps->GetSerialProtocols().empty()?
            ",\"serialprotocols\":[\"" + Join(caps->GetSerialProtocols(), std::string("\",\"")) + "\"]}" :
             ",\"serialprotocols\":[]}");

    return json;
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void Controller::AddModels(wxPGProperty* p, wxPGProperty* vp) {
    int mIdx = -1;
    wxPGChoices models;

    if (_vendor != "") {
        for (const auto& it : ControllerCaps::GetModels(GetType(), _vendor)) {
            models.Add(it);
            if (it == _model) {
                mIdx = models.GetCount() - 1;
            }
        }
    }
    p->SetChoices(models);
    if (models.GetCount() > 0) {
        p->Hide(false);
        if (mIdx >= 0) {
            p->SetChoiceSelection(mIdx);
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        } else {
            p->SetBackgroundColour(*wxRED);
            vp->Hide(true);
        }
    } else {
        p->Hide(true);
    }
}
void Controller::AddVariants(wxPGProperty* p) {
    int variantIdx = -1;
    wxPGChoices variants;
    
    if (_vendor != "" && _model != "") {
        p->Hide(false);
        std::list<std::string> vars = ControllerCaps::GetVariants(GetType(), _vendor, _model);
        for (const auto& it : vars) {
            variants.Add(it);
            if (it == _variant) {
                variantIdx = variants.GetCount() - 1;
            }
        }
        if (variants.GetCount() > 1) {
            if (variantIdx == -1) {
                variantIdx = 0;
                _variant = vars.front();
            }
        } else {
            _variant = "";
        }
        p->SetChoices(variants);
        if (variants.GetCount() > 1) {
            p->Hide(false);
            if (variantIdx >= 0) {
                p->SetChoiceSelection(variantIdx);
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            } else {
                p->SetBackgroundColour(*wxRED);
            }
        } else {
            p->Hide(true);
        }
    } else {
        p->Hide(true);
    }
}


void Controller::UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    
    wxPGProperty* p = propertyGrid->GetProperty("ControllerName");
    if (p) p->SetValue(GetName());

    p = propertyGrid->GetProperty("ControllerDescription");
    if (p) p->SetValue(GetDescription());
    
    p = propertyGrid->GetProperty("ControllerId");
    if (p) {
        p->Hide(!IsNeedsId());
        p->SetValue(GetId());
    }
    
    p  = propertyGrid->GetProperty("AutoLayout");
    if (p) {
        p->SetValue(IsAutoLayout());
        p->Hide(!SupportsAutoLayout());
    }
    
    p  = propertyGrid->GetProperty("AutoUpload");
    if (p) {
        p->SetValue(IsAutoUpload());
        p->Hide(!SupportsAutoUpload());
    }
    
    p  = propertyGrid->GetProperty("AutoSize");
    if (p) {
        p->SetValue(IsAutoSize());
        p->Hide(!SupportsAutoSize());
    }
    
    p  = propertyGrid->GetProperty("SuppressDuplicates");
    if (p) {
        p->SetValue(IsSuppressDuplicateFrames());
        p->Hide(!SupportsSuppressDuplicateFrames());
    }
    p  = propertyGrid->GetProperty("Monitor");
    if (p) {
        p->SetValue(IsMonitoring());
    }

    p  = propertyGrid->GetProperty("FullxLightsControl");
    if (p) {
        p->SetValue(IsFullxLightsControl());
        p->Hide(!SupportsFullxLightsControl());
    }
    if (SupportsFullxLightsControl() && IsFullxLightsControl()) {
        p  = propertyGrid->GetProperty("DefaultBrightnessUnderFullxLightsControl");
        if (p) {
            p->SetValue(GetDefaultBrightnessUnderFullControl());
            p->Hide(!SupportsDefaultBrightness());
        }

        p  = propertyGrid->GetProperty("DefaultGammaUnderFullxLightsControl");
        if (p) {
            p->SetValue(GetDefaultGammaUnderFullControl());
            p->Hide(!SupportsDefaultGamma());
        }
    } else {
        p  = propertyGrid->GetProperty("DefaultBrightnessUnderFullxLightsControl");
        if (p) p->Hide(true);

        p  = propertyGrid->GetProperty("DefaultGammaUnderFullxLightsControl");
        if (p) p->Hide(true);
    }
    p  = propertyGrid->GetProperty("Active");
    if (p) {
        p->SetChoiceSelection((int)_active);
    }
}

void Controller::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    wxPGProperty* p = propertyGrid->Append(new wxStringProperty("Name", "ControllerName", GetName()));
    p->SetHelpString("This must be unique.");

    propertyGrid->Append(new wxStringProperty("Description", "ControllerDescription", GetDescription()));
    
    
    int v = 0;
    wxPGChoices vendors;
    for (const auto& it : ControllerCaps::GetVendors(GetType())) {
        vendors.Add(it);
        if (it == _vendor) {
            v = vendors.GetCount() - 1;
        }
    }
    if (vendors.GetCount() > 0) {
        propertyGrid->Append(new wxEnumProperty("Vendor", "Vendor", vendors, v));
        wxPGProperty *mp = propertyGrid->Append(new wxEnumProperty("Model", "Model"));
        wxPGProperty *vp = propertyGrid->Append(new wxEnumProperty("Variant", "Variant"));
        AddVariants(vp);
        AddModels(mp, vp);
    }

    p = propertyGrid->Append(new wxUIntProperty("Id", "ControllerId", GetId()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 65335);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("For controllers that don't support universes the Id can be used with some start channel addressing modes.");

    p = propertyGrid->Append(new wxBoolProperty("Auto Layout Models", "AutoLayout", IsAutoLayout()));
    p->SetEditor("CheckBox");
    p->SetHelpString("Auto layout models causes xLights to move models around in the controllers channel range to optimise them. It also needs to be set to allow you to move modes in the visualiser between controllers.");
    
    p = propertyGrid->Append(new wxBoolProperty("Auto Upload Configuration", "AutoUpload", IsAutoUpload()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will send your controller configuration to the controller when you turn on output to lights. This is known to cause delays in turning on output to lights when controllers are not reachable.");
    
    p = propertyGrid->Append(new wxBoolProperty("Auto Size", "AutoSize", IsAutoSize()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will cause xLights to dynamically resize the number of channels on the controller to ensure there are exactly enough for the models on the controller.");
    
    p = propertyGrid->Append(new wxBoolProperty("Full xLights Control", "FullxLightsControl", IsFullxLightsControl()));
    p->SetEditor("CheckBox");
    p->SetHelpString("This option will when uploading erase the configuration of all unused ports on the controller per the configuration in xLights.");

    p = propertyGrid->Append(new wxUIntProperty("Default Port Brightness", "DefaultBrightnessUnderFullxLightsControl", GetDefaultBrightnessUnderFullControl()));
    p->SetAttribute("Min", 5);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This option will set the brightness of all ports to this value unless specifically overriden by a model.");

    p = propertyGrid->Append(new wxFloatProperty("Default Port Gamma", "DefaultGammaUnderFullxLightsControl", GetDefaultGammaUnderFullControl()));
    p->SetAttribute("Min", 0.1F);
    p->SetAttribute("Max", 5.0F);
    p->SetEditor("SpinCtrlDouble");
    p->SetHelpString("This option will set the Gamma of all ports to this value unless specifically overriden by a model.");

    if (ACTIVETYPENAMES.IsEmpty()) {
        ACTIVETYPENAMES.push_back("Active");
        ACTIVETYPENAMES.push_back("Inactive");
        ACTIVETYPENAMES.push_back("xLights Only");
    }
    p = propertyGrid->Append(new wxEnumProperty("Active", "Active", ACTIVETYPENAMES, wxArrayInt(), (int)_active));
    p->SetHelpString("When inactive no data is output and any upload to FPP or xSchedule will also not output to the lights. When xLights only it will output when played in xLights but again FPP and xSchedule will not output to the lights.");

    p = propertyGrid->Append(new wxBoolProperty("Monitor", "Monitor", IsMonitoring()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When selected, the show player will monitor the connectivity to the controller via ping packets or other network traffic to make sure the controller is up and running.  Disabling can reduce network traffic as well as reduce load on the controller.");
    
    p = propertyGrid->Append(new wxBoolProperty("Suppress Duplicate Frames", "SuppressDuplicates", IsSuppressDuplicateFrames()));
    p->SetEditor("CheckBox");
    p->SetHelpString("When selected if a data packet is the same in a subsequent frame then xLights will not send the duplicate frame trusting that the controller will just reuse the previously sent data. This can reduce unnecessary network traffic.");
}

bool Controller::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager)
{
    wxString const name = event.GetPropertyName();
    wxPropertyGrid *propertyGrid  = (wxPropertyGrid*)event.GetEventObject();

    if (name == "ControllerName") {
        auto cn = event.GetValue().GetString().Trim(true).Trim(false);
        if (_outputManager->GetController(cn) != nullptr || cn == "" || cn == NO_CONTROLLER) {
            DisplayError("Controller name '" + cn + "' blank or already used. Controller names must be unique and non blank.");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::ControllerName");
            return false;
        } else {
            SetName(cn);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::ControllerName");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "Controller::HandlePropertyEvent::ControllerName");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::ControllerName");
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Controller::HandlePropertyEvent::ControllerName");
            return true;
        }
    } else if (name == "ControllerDescription") {
        SetDescription(event.GetValue().GetString().Trim(true).Trim(false));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Controllerdescription");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::ControllerName");
        return true;
    } else if (name == "ControllerId") {
        SetId(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::ControllerId");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::ControllerName");
        return true;
    } else if (name == "Active") {
        SetActive(ACTIVETYPENAMES[event.GetValue().GetLong()]);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Active");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::Active");
        return true;
    } else if (name == "AutoLayout") {
        SetAutoLayout(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::AutoLayout");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::AutoLayout");
        return true;
    } else if (name == "AutoUpload") {
        SetAutoUpload(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::AutoUpload");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::AutoUpload");
        return true;
    } else if (name == "SuppressDuplicates") {
        SetSuppressDuplicateFrames(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::SuppressDuplicates");
        return true;
    } else if (name == "Monitor") {
        SetMonitoring(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Monitor");
        return true;
    } else if (name == "AutoSize") {
        SetAutoSize(event.GetValue().GetBool(), outputModelManager);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::AutoSize");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "Controller::HandlePropertyEvent::AutoSize");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Controller::HandlePropertyEvent::AutoSize");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::AutoSize");
        return true;
    } else if (name == "FullxLightsControl") {
        SetFullxLightsControl(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::FullxLightsControl");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::FullxLightsControl");
        return true;
    } else if (name == "DefaultBrightnessUnderFullxLightsControl") {
        SetDefaultBrightnessUnderFullControl(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::DefaultBrightnessUnderFullxLightsControl");
        return true;
    } else if (name == "DefaultGammaUnderFullxLightsControl") {
        SetDefaultGammaUnderFullControl(event.GetValue().GetDouble());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::DefaultGammaUnderFullxLightsControl");
        return true;
    } else if (name == "Vendor") {
        int idx = event.GetValue().GetLong();
        auto const vendors = ControllerCaps::GetVendors(GetType());
        auto it = begin(vendors);
        std::advance(it, idx);
        if (event.GetValue().GetLong() >= 0 && it != end(vendors)) {
            SetVendor(*it, propertyGrid);

            auto models = ControllerCaps::GetModels(GetType(), *it);
            if (models.size() == 2) {
                SetModel(models.back());
                auto variants = ControllerCaps::GetVariants(GetType(), *it, models.front());
                if (variants.size() == 2) {
                    SetVariant(variants.back(), propertyGrid);
                } else {
                    SetVariant("", propertyGrid);
                }
            } else {
                SetModel("", propertyGrid);
                SetVariant("", propertyGrid);
            }
        } else {
            SetVendor("", propertyGrid);
            SetModel("", propertyGrid);
            SetVariant("", propertyGrid);
        }
        
        wxPGProperty *mp = propertyGrid->GetProperty("Model");
        wxPGProperty *vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::Vendor");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Vendor");
        return true;
    } else if (name == "Model") {
        auto const models = ControllerCaps::GetModels(GetType(), _vendor);
        auto it = begin(models);
        std::advance(it, event.GetValue().GetLong());
        SetModel(*it, propertyGrid);

        std::list<std::string> variants = ControllerCaps::GetVariants(GetType(), _vendor, *it);
        SetVariant(variants.front(), propertyGrid);
        
        wxPGProperty *mp = propertyGrid->GetProperty("Model");
        wxPGProperty *vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Model");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::Model");
        return true;
    } else if (name == "Variant") {
        auto const versions = ControllerCaps::GetVariants(GetType(), _vendor, _model);
        auto it = begin(versions);
        std::advance(it, event.GetValue().GetLong());
        SetVariant(*it, propertyGrid);
        
        wxPGProperty *mp = propertyGrid->GetProperty("Model");
        wxPGProperty *vp = propertyGrid->GetProperty("Variant");
        AddVariants(vp);
        AddModels(mp, vp);

        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Controller::HandlePropertyEvent::Variant");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Controller::HandlePropertyEvent::Variant");
        return true;
    }
    return false;
}

void Controller::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    auto p = propGrid->GetPropertyByName("ControllerId");
    auto const& name = GetName();

    if (p != nullptr) {
        // Id should be unique
        int id = GetId();
        for (const auto& it : om->GetControllers()) {
            if (it->GetName() != name && it->GetId() == id && it->GetId() != -1) {
                p->SetBackgroundColour(*wxRED);
                break;
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    p = propGrid->GetPropertyByName("ControllerName");
    if (p != nullptr) {
        if (!_outputManager->IsControllerNameUnique(name)) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}

void Controller::HandleExpanded(wxPropertyGridEvent& event, bool expanded)
{
    if (_outputs.size() > 0) {
        _outputs.front()->HandleExpanded(event, expanded);
    }
}

#pragma endregion
#endif

#ifndef EXCLUDEDISCOVERY
void Controller::SetAutoSize(bool autosize, OutputModelManager* omm)
{
    if (_autoSize != autosize) {
        _autoSize = autosize;
        _dirty = true;
        ControllerEthernet* ce = dynamic_cast<ControllerEthernet*>(this);
        if (ce != nullptr) {
            if (_autoSize) {
                ce->SetAllSameSize(true, omm);
            } else {
                ce->SetUniversePerString(false);
            }
        }
    }
}

bool Controller::SetChannelSize(int32_t channels, std::list<Model*> models, uint32_t universeSize)
{
    if (_outputs.size() == 0) return false;

    for (auto& it2 : GetOutputs()) {
        it2->AllOff();
        it2->EndFrame(0);
    }
    GetFirstOutput()->SetChannels(channels);

    return true;
}
#endif
