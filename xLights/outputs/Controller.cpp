
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Controller.h"
#include "UtilFunctions.h"
#include "Output.h"
#include "OutputManager.h"
#include "../OutputModelManager.h"
#include "../controllers/ControllerCaps.h"
#include "ControllerEthernet.h"
#include "ControllerNull.h"
#include "ControllerSerial.h"
#include "../models/Model.h"

#include <numeric>

#include <log.h>

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
Controller::Controller(OutputManager* om, pugi::xml_node node, const std::string& showDir) : _outputManager(om)
{
    std::string activeState = node.attribute("ActiveState").as_string("");
    if (activeState.empty()) {
        if (std::string_view(node.attribute("Active").as_string("1")) == "0") {
            SetActive("Inactive");
        } else {
            SetActive("Active");
        }
    } else {
        SetActive(activeState);
    }

    for (pugi::xml_node n = node.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == "network") {
            _outputs.push_back(Output::Create(this, n, showDir));
            if (_outputs.back() == nullptr) {
                // this shouldnt happen unless we are loading a future file with an output type we dont recognise
                _outputs.pop_back();
            }
        } else if (std::string_view(n.name()) == "ExtraProperty") {
            _extraProperties[n.attribute("name").as_string("")] = n.attribute("value").as_string("");
        }
    }

    _id = node.attribute("Id").as_int(64001);
    std::string defaultName = om->UniqueName(std::string(node.name()) + "_");
    _name = Trim(node.attribute("Name").as_string(defaultName.c_str()));
    _description = Trim(node.attribute("Description").as_string(""));
    _fromBase = std::string_view(node.attribute("FromBase").as_string("0")) == "1";
    _autoSize = std::string_view(node.attribute("AutoSize").as_string("0")) == "1";
    SetAutoLayout(std::string_view(node.attribute("AutoLayout").as_string("1")) == "1");
    _fullxLightsControl = std::string_view(node.attribute("FullxLightsControl").as_string("FALSE")) == "TRUE";
    _defaultBrightnessUnderFullControl = node.attribute("DefaultBrightnessUnderFullControl").as_int(100);
    _defaultGammaUnderFullControl = node.attribute("DefaultGammaUnderFullControl").as_float(1.0);
    SetAutoUpload(std::string_view(node.attribute("AutoUpload").as_string("0")) == "1");
    if (!_autoLayout) _autoSize = false;
    _vendor = node.attribute("Vendor").as_string("");
    _model = node.attribute("Model").as_string("");
    _variant = node.attribute("Variant").as_string("");

    SetSuppressDuplicateFrames(std::string_view(node.attribute("SuppressDuplicates").as_string("0")) == "1");
    SetMonitoring(std::string_view(node.attribute("Monitor").as_string("1")) == "1");

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
    _extraProperties = from._extraProperties;
}

Controller::~Controller() {
    DeleteAllOutputs();
}

pugi::xml_node Controller::Save(pugi::xml_node parent) {

    _dirty = false;

    pugi::xml_node node = parent.append_child("Controller");
    node.append_attribute("Id") = _id;
    node.append_attribute("Name") = _name;
    node.append_attribute("Description") = _description;
    node.append_attribute("Type") = GetType();
    node.append_attribute("Vendor") = GetVendor();
    node.append_attribute("Model") = GetModel();
    node.append_attribute("Variant") = GetVariant();
    node.append_attribute("AutoSize") = _autoSize ? "1" : "0";
    node.append_attribute("FromBase") = _fromBase ? "1" : "0";
    if (_fullxLightsControl)
        node.append_attribute("FullxLightsControl") = "TRUE";
    node.append_attribute("DefaultBrightnessUnderFullControl") = _defaultBrightnessUnderFullControl;
    node.append_attribute("DefaultGammaUnderFullControl") = _defaultGammaUnderFullControl;
    node.append_attribute("ActiveState") = DecodeActiveState(_active);
    node.append_attribute("AutoLayout") = _autoLayout ? "1" : "0";
    node.append_attribute("AutoUpload") = (_autoUpload && SupportsAutoUpload()) ? "1" : "0";
    node.append_attribute("SuppressDuplicates") = _suppressDuplicateFrames ? "1" : "0";
    node.append_attribute("Monitor") = _monitor ? "1" : "0";
    for (const auto& it : _outputs) {
        it->Save(node);
    }
    if (!_extraProperties.empty()) {
        for (auto &p : _extraProperties) {
            pugi::xml_node ep = node.append_child("ExtraProperty");
            ep.append_attribute("name") = p.first;
            ep.append_attribute("value") = p.second;
        }
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

    if (changed) {
        _dirty = true;
    }

    return changed;
}
#pragma endregion

#pragma region Static Functions
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

Controller* Controller::Create(OutputManager* om, pugi::xml_node node, std::string showDir) {

    

    std::string type = node.attribute("Type").as_string("");

    if (type == CONTROLLER_NULL) {
        return new ControllerNull(om, node, showDir);
    }
    else if (type == CONTROLLER_ETHERNET) {
        return new ControllerEthernet(om, node, showDir);
    }
    else if (type == CONTROLLER_SERIAL) {
        return new ControllerSerial(om, node, showDir);
    }

    spdlog::warn("Unknown controller type {} ignored.", type);
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

    ControllerCaps* newcap2 = ControllerCaps::GetControllerConfigByAlternateName(vendor, model, variant);
    if (newcap2 && model != newcap2->GetModel()) {
        _vendor = newcap2->GetVendor();
        _model = newcap2->GetModel();
        _variant = newcap2->GetVariantName();
        _dirty = true;
        VMVChanged();
    }
}
#pragma endregion

#pragma region Getters and Setters
Output* Controller::GetOutput(int outputNumber) const {

    if (outputNumber < 0 || (size_t)outputNumber > _outputs.size()) return nullptr;

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
    return end >= (uint32_t)GetStartChannel() && start < (uint32_t)GetEndChannel();
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

void Controller::Convert(pugi::xml_node node, std::string showDir) {

    _dirty = true;

    if (_outputs.size() == 1) {
        _suppressDuplicateFrames = _outputs.front()->IsSuppressDuplicateFrames();
        _autoSize = _outputs.front()->IsAutoSize_CONVERT();
    }

    std::string c = node.attribute("Controller").as_string("");
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

// UI property grid methods moved to ui/controllerproperties/ControllerPropertyAdapter
#pragma endregion

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
