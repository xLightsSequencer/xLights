
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "ControllerEthernet.h"
#include "OutputManager.h"
#include "Output.h"
#include "UtilFunctions.h"
#include "../utils/SpecialOptions.h"
#include "../models/OutputModelManager.h"
#include "IPOutput.h"
#include "E131Output.h"
#include "ArtNetOutput.h"
#include "ZCPPOutput.h"
#include "DDPOutput.h"
#include "KinetOutput.h"
#include "xxxEthernetOutput.h"
#include "TwinklyOutput.h"
#include "OPCOutput.h"
#include "../controllers/ControllerCaps.h"
#include "../models/ModelManager.h"
#ifndef EXCLUDENETWORKUI
#include "../models/Model.h"
#endif
#include "../utils/ip_utils.h"

#include "../xLightsMain.h"

#include <format>

#include <log.h>

// Property choices moved to ui/controllerproperties/ControllerEthernetPropertyAdapter

#pragma region Constructors and Destructors
ControllerEthernet::ControllerEthernet(OutputManager* om, pugi::xml_node node, const std::string& showDir) : Controller(om, node, showDir) {

    _type = node.attribute("Protocol").as_string("");
    // Property choice initialization moved to ControllerEthernetPropertyAdapter
    SetIP(node.attribute("IP").as_string(""));
    SetFPPProxy(node.attribute("FPPProxy").as_string(""));
    SetPriority(node.attribute("Priority").as_int(100));
    SetVersion(node.attribute("Version").as_int(1));
    _expanded = std::string_view(node.attribute("Expanded").as_string("FALSE")) == "TRUE";
    _universePerString = std::string_view(node.attribute("UPS").as_string("FALSE")) == "TRUE";
    _forceLocalIP = node.attribute("ForceLocalIP").as_string("");
    _dirty = false;
}

ControllerEthernet::ControllerEthernet(OutputManager* om, bool acceptDuplicates) : Controller(om) {

    _managed = !acceptDuplicates;
    // Property choice initialization moved to ControllerEthernetPropertyAdapter
    _name = (om == nullptr) ? "Dummy" : om->UniqueName("Ethernet_");
    _type = OUTPUT_E131;
    _expanded = false;
    E131Output* o = new E131Output();
    _outputs.push_back(o);
}

ControllerEthernet::ControllerEthernet(OutputManager* om, const ControllerEthernet& from) :
    Controller(om, from)
{
    _type = from._type;
    SetIP(from._ip);
    SetFPPProxy(from._fppProxy);
    SetPriority(from._priority);
    SetVersion(from._version);
    _expanded = from._expanded;
    _universePerString = from._universePerString;
    _forceLocalIP = from._forceLocalIP;
}

ControllerEthernet::~ControllerEthernet() {
    ip_utils::waitForAllToResolve();
    // wait for an active ping to finish
    if (_asyncPing.valid()) {
        _asyncPing.wait_for(std::chrono::seconds(2));
    }
}

pugi::xml_node ControllerEthernet::Save(pugi::xml_node parent) {

    pugi::xml_node um = Controller::Save(parent);

    um.append_attribute("IP") = _ip;
    um.append_attribute("Protocol") = _type;
    um.append_attribute("FPPProxy") = _fppProxy;
    um.append_attribute("Priority") = _priority;
    um.append_attribute("Version") = _version;
    um.append_attribute("Expanded") = _expanded ? "TRUE" : "FALSE";
    um.append_attribute("UPS") = _universePerString ? "TRUE" : "FALSE";
    um.append_attribute("ForceLocalIP") = _forceLocalIP;

    return um;
}
bool ControllerEthernet::UpdateFrom(Controller* from)
{
    bool changed = Controller::UpdateFrom(from);

    ControllerEthernet* fromEth = static_cast<ControllerEthernet*>(from);

    if (_ip != fromEth->_ip) {
        changed = true;
        _ip = fromEth->_ip;
    }
    if (_type != fromEth->_type) {
        changed = true;
        _type = fromEth->_type;
    }
    if (_fppProxy != fromEth->_fppProxy) {
        changed = true;
        _fppProxy = fromEth->_fppProxy;
    }
    if (_priority != fromEth->_priority) {
        changed = true;
        _priority = fromEth->_priority;
    }
    if (_version != fromEth->_version) {
        changed = true;
        _version = fromEth->_version;
    }
    if (_expanded != fromEth->_expanded) {
        changed = true;
        _expanded = fromEth->_expanded;
    }
    if (_universePerString != fromEth->_universePerString) {
        changed = true;
        _universePerString = fromEth->_universePerString;
    }
    if (_forceLocalIP != fromEth->_forceLocalIP) {
        changed = true;
        _forceLocalIP = fromEth->_forceLocalIP;
    }

    return changed;
}
Controller* ControllerEthernet::Copy(OutputManager* om)
{
    return new ControllerEthernet(om, *this);
}
#pragma endregion

#pragma region Getters and Setters
void ControllerEthernet::SetIP(const std::string& ip) {

    auto const& iip = ip_utils::CleanupIP(ip);
    if (_ip != iip) {
        _ip = iip;
        if (IsActive()) {
            std::unique_lock<std::shared_mutex> lock(_resolveMutex);
            _resolvedIp = _ip;
            lock.unlock();
            ip_utils::ResolveIP(_ip, [this](const std::string &r) {
                std::unique_lock<std::shared_mutex> lock(_resolveMutex);
                _resolvedIp = r;
                lock.unlock();
                std::shared_lock<std::shared_mutex> lock2(_resolveMutex);
                for (auto& it : GetOutputs()) {
                    it->SetResolvedIP(_resolvedIp);
                }
            });
        }
        _dirty = true;
        if (_outputManager != nullptr) _outputManager->UpdateUnmanaged();

        std::shared_lock<std::shared_mutex> lock(_resolveMutex);
        for (auto& it : GetOutputs()) {
            it->SetIP(_ip, IsActive(), false); // don't resolve as we'll set it above in the callback
            it->SetResolvedIP(_resolvedIp);
        }
    }
}

// because we dont resolve IPs on creation for inactive controllers then if the controller is set active we need to resolve it then
void ControllerEthernet::PostSetActive()
{
    std::unique_lock<std::shared_mutex> lock(_resolveMutex);
    if (IsActive() && !_ip.empty() && _resolvedIp.empty()) {
        _resolvedIp = ip_utils::ResolveIP(_ip);
        lock.unlock();
        std::shared_lock<std::shared_mutex> lock(_resolveMutex);
        for (auto& it : GetOutputs()) {
            it->SetResolvedIP(_resolvedIp);
        }
    }
}
std::string ControllerEthernet::GetResolvedIP(bool forceResolve) const {
    std::shared_lock<std::shared_mutex> lock(_resolveMutex);
    if (_resolvedIp.empty() && !_ip.empty() && forceResolve) {
        return ip_utils::ResolveIP(_ip);
    }
    return _resolvedIp;
}

void ControllerEthernet::SetProtocol(const std::string& protocol) {

    int totchannels = GetChannels();
    auto const oldtype = _type;
    auto oldoutputs = _outputs;
    _outputs.clear(); // empties the list but doesnt delete anything yet

    _type = protocol;

    if (_type == OUTPUT_PLAYER_ONLY) {
    
    }
    else if (_type == OUTPUT_ZCPP || _type == OUTPUT_DDP || _type == OUTPUT_TWINKLY) {
        if (_type == OUTPUT_ZCPP) {
            auto zo = new ZCPPOutput();
            _outputs.push_back(zo);
            if (oldoutputs.size() != 0) {
                zo->SetId(oldoutputs.front()->GetUniverse());
            }
            SetId(zo->GetId());
        }
        else if (_type == OUTPUT_DDP) {
            auto ddpo = new DDPOutput();
            _outputs.push_back(ddpo);

            if (oldoutputs.size() != 0) {
                if (_outputManager != nullptr && _outputManager->IsIDUsed(oldoutputs.front()->GetUniverse())) {
                    ddpo->SetId(_outputManager->UniqueId());
                } else {
                    ddpo->SetId(oldoutputs.front()->GetUniverse());
                }
            } else if (_outputManager != nullptr) {
                ddpo->SetId(_outputManager->UniqueId());
            }
            SetId(ddpo->GetId());
            
            auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
            if( c && c->DDPStartsAtOne() )
                ddpo->SetKeepChannelNumber(false);
            
        } else if (_type == OUTPUT_TWINKLY) {
            auto to = new TwinklyOutput();
            _outputs.push_back(to);
            if (oldoutputs.size() != 0) {
                if (_outputManager != nullptr && _outputManager->IsIDUsed(oldoutputs.front()->GetUniverse())) {
                    to->SetId(_outputManager->UniqueId());
                } else {
                    to->SetId(oldoutputs.front()->GetUniverse());
                }
            } else if (_outputManager != nullptr) {
                to->SetId(_outputManager->UniqueId());
            }
            SetId(to->GetId());
        }
        if (_outputs.size() > 0 && oldoutputs.size() != 0) {
            _outputs.front()->SetChannels(totchannels);
            _outputs.front()->SetFPPProxyIP(oldoutputs.front()->GetFPPProxyIP());
            _outputs.front()->SetIP(oldoutputs.front()->GetIP(), IsActive());
            _outputs.front()->SetSuppressDuplicateFrames(oldoutputs.front()->IsSuppressDuplicateFrames());
        }
    }
    else {
        if (oldtype == OUTPUT_E131 || oldtype == OUTPUT_ARTNET || oldtype == OUTPUT_xxxETHERNET || oldtype == OUTPUT_OPC || oldtype == OUTPUT_KINET) {
            for (const auto& it : oldoutputs) {
                if (_type == OUTPUT_E131) {
                    _outputs.push_back(new E131Output());
                }
                else if (_type == OUTPUT_ARTNET) {
                    _outputs.push_back(new ArtNetOutput());
                }
                else if (_type == OUTPUT_KINET) {
                    _outputs.push_back(new KinetOutput());
                }
                else if (_type == OUTPUT_xxxETHERNET) {
                    _outputs.push_back(new xxxEthernetOutput());
                }
                else if (_type == OUTPUT_OPC) {
                    _outputs.push_back(new OPCOutput());
                }
                if (_outputs.size() > 0) {
                    _outputs.back()->SetIP(oldoutputs.front()->GetIP(), IsActive());
                    _outputs.back()->SetUniverse(it->GetUniverse());
                    _outputs.back()->SetChannels(it->GetChannels());
                    _outputs.back()->Enable(IsActive());
                }
            }
        }
        else {
            #define CONVERT_CHANNELS_PER_UNIVERSE 510
            int universes = (totchannels + CONVERT_CHANNELS_PER_UNIVERSE - 1) / CONVERT_CHANNELS_PER_UNIVERSE;
            int left = universes * CONVERT_CHANNELS_PER_UNIVERSE;

            int u = 0;

            if (oldoutputs.size() != 0) {
                if (_outputManager != nullptr && _outputManager->IsIDUsed(oldoutputs.front()->GetUniverse())) {
                    u = _outputManager->UniqueId() - 1;
                } else {
                    u = oldoutputs.front()->GetUniverse() - 1;
                }
            }

            for (int i = 0; i < universes; i++) {
                if (_type == OUTPUT_E131) {
                    _outputs.push_back(new E131Output());
                }
                else if (_type == OUTPUT_ARTNET) {
                    _outputs.push_back(new ArtNetOutput());
                }
                else if (_type == OUTPUT_KINET) {
                    _outputs.push_back(new KinetOutput());
                }
                else if (_type == OUTPUT_xxxETHERNET) {
                    _outputs.push_back(new xxxEthernetOutput());
                }
                else if (_type == OUTPUT_OPC) {
                    _outputs.push_back(new OPCOutput());
                }
                if (_outputs.size() > 0 && oldoutputs.size() != 0) {
                    _outputs.back()->SetChannels(left > CONVERT_CHANNELS_PER_UNIVERSE ? CONVERT_CHANNELS_PER_UNIVERSE : left);
                    left -= _outputs.back()->GetChannels();
                    _outputs.back()->SetIP(oldoutputs.front()->GetIP(), IsActive());
                    _outputs.back()->SetUniverse(u + i + 1);
                    _outputs.back()->Enable(IsActive());
                }
            }
        }
    }

    if (_ip == "MULTICAST" && _type != OUTPUT_E131 && _type != OUTPUT_ZCPP) {
        SetIP("");
    }

    SetIP(GetIP()); // this ensures IP cascades as appropriate
    SetId(GetId()); // this ensure ids cascade as appropriate

    while (oldoutputs.size() > 0) {
        delete oldoutputs.front();
        oldoutputs.pop_front();
    }

    if (_outputManager != nullptr)
        _outputManager->UpdateUnmanaged();
}

std::string ControllerEthernet::GetForceLocalIP() const
{
    if (_forceLocalIP != "") {
        return _forceLocalIP;
    }

    // a controller should not proxy itself
    if (_outputManager != nullptr) return _outputManager->GetGlobalForceLocalIP();

    return "";
}

std::string ControllerEthernet::GetControllerForceLocalIP() const
{
    return _forceLocalIP;
}

void ControllerEthernet::SetForceLocalIP(const std::string& localIP)
{
    if (_forceLocalIP != localIP) {
        _forceLocalIP = localIP;
        _dirty = true;
        for (auto& it : _outputs) {
            it->SetForceLocalIP(localIP);
        }
    }
}

void ControllerEthernet::SetGlobalForceLocalIP(const std::string& localIP)
{
    for (const auto& it : _outputs) {
        it->SetGlobalForceLocalIP(localIP);
    }
}

void ControllerEthernet::SetFPPProxy(const std::string& proxy) {

    if (_fppProxy != proxy && proxy != _ip) {
        _fppProxy = proxy;
        _dirty = true;
        for (auto& it : _outputs) {
            it->SetFPPProxyIP(proxy);
        }
    }
}

std::string ControllerEthernet::GetFPPProxy() const {

    if (_fppProxy != "") {
        std::string _resolvedfppProxy = ip_utils::ResolveIP(_fppProxy);
        if (_resolvedfppProxy != _ip) {
            return _resolvedfppProxy;
        }
    }

    // a controller should not proxy itself
    if (_outputManager != nullptr && _ip != _outputManager->GetGlobalFPPProxy()) {
        return _outputManager->GetGlobalFPPProxy();
    }

    return "";
}

void ControllerEthernet::SetUniversePerString(bool ups)
{
    if (_universePerString != ups) {
        _universePerString = ups;
        _dirty = true;
    }
}

void ControllerEthernet::SetPriority(int priority) {

    if (_priority != priority) {
        _priority = priority;
        _dirty = true;

        for (auto& it : _outputs) {
            if (dynamic_cast<E131Output*>(it) != nullptr) {
                dynamic_cast<E131Output*>(it)->SetPriority(priority);
            }
            else if (dynamic_cast<ZCPPOutput*>(it) != nullptr) {
                dynamic_cast<ZCPPOutput*>(it)->SetPriority(priority);
            }
        }
    }
}

void ControllerEthernet::SetVersion(int version)
{
    if (_version != version) {
        _version = version;
        _dirty = true;

        for (auto& it : _outputs) {
            if (dynamic_cast<KinetOutput*>(it) != nullptr) {
                dynamic_cast<KinetOutput*>(it)->SetVersion(version);
            }
        }
    }
}

bool ControllerEthernet::AllSameSize() const {

    int32_t size = -1;
    for (const auto& it : _outputs) {
        if (size < 0) {
            size = it->GetChannels();
        }
        else if (it->GetChannels() != size) {
            return false;
        }
    }
    return true;
}
#pragma endregion

#pragma region Virtual Functions
void ControllerEthernet::SetId(int id) {

    Controller::SetId(id);
    if (GetProtocol() == OUTPUT_DDP) {
        dynamic_cast<DDPOutput*>(GetFirstOutput())->SetId(id);
    }
    else if (GetProtocol() == OUTPUT_ZCPP) {
        dynamic_cast<ZCPPOutput*>(GetFirstOutput())->SetId(id);
    }
}

int ControllerEthernet::GetId() const
{
    if (GetProtocol() == OUTPUT_E131 || GetProtocol() == OUTPUT_ARTNET || GetProtocol() == OUTPUT_KINET) {
        return -1;
    }
    else         {
        return Controller::GetId();
    }
}

std::string ControllerEthernet::GetLongDescription() const {

    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + " " + GetProtocol() + " " + GetIP() + " ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";
    res += _description;

    return res;
}

std::string ControllerEthernet::GetShortDescription() const {

	std::string res = "";

	if (!IsActive()) res += "INACTIVE ";
	res += GetName() + " " + GetProtocol() + " " + GetIP();
    if (!_description.empty()) {
        res += " ";
        res += _description;
    }

	return res;
}

void ControllerEthernet::Convert(pugi::xml_node node, std::string showDir) {

    _outputs.push_back(Output::Create(this, node, showDir));
    if (_outputs.back() == nullptr) {
        // this shouldnt happen unless we are loading a future file with an output type we dont recognise
        _outputs.pop_back();
        return;
    }

    Controller::Convert(node, showDir);

    if (_outputs.size() == 1) {
        if (_name == "" || StartsWith(_name, "Ethernet_")) {
            if (_outputs.back()->GetDescription_CONVERT() != "") {
                _name = _outputManager->UniqueName(_outputs.back()->GetDescription_CONVERT());
            }
            else {
                _name = _outputManager->UniqueName("Unnamed");
            }
        }
        SetIP(_outputs.front()->GetIP());
        _type = _outputs.front()->GetType();
        _fppProxy = _outputs.front()->GetFPPProxyIP();
        _id = _outputs.front()->GetUniverse();
        if (_id < 0) _id = _outputManager->UniqueId();
        if (_type == OUTPUT_E131) {
            _priority = dynamic_cast<E131Output*>(_outputs.front())->GetPriority();
        }
        else if (_type == OUTPUT_ZCPP) {
            _priority = dynamic_cast<ZCPPOutput*>(_outputs.front())->GetPriority();
        }
        else if (_type == OUTPUT_xxxETHERNET) {
            // xxx ethernet type - adapter handles protocol choices initialization
        }
    }

    if (_outputs.back()->IsOutputCollection_CONVERT()) {
        auto o = _outputs.back();
        _outputs.pop_back();

        for (auto& it : o->GetOutputs_CONVERT()) {
            if (it->GetType() == OUTPUT_E131) {
                _outputs.push_back(new E131Output(*dynamic_cast<E131Output*>(it)));
                _outputs.back()->Enable(IsActive());
            }
            else {
                wxASSERT(false);
            }
        }
        delete o;
    }
}

bool ControllerEthernet::SupportsFullxLightsControl() const
{
    if (_type == OUTPUT_ZCPP) return false;

    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        return c->SupportsFullxLightsControl();
    }
    return false;
}

bool ControllerEthernet::SupportsDefaultBrightness() const
{
    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        return c->SupportsDefaultBrightness();
    }
    return false;
}

bool ControllerEthernet::SupportsDefaultGamma() const
{
    if (_type == OUTPUT_ZCPP)
        return false;

    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        return c->SupportsDefaultGamma();
    }
    return false;
}

std::string ControllerEthernet::GetChannelMapping(int32_t ch) const
{

    std::string res = std::format("Channel {} maps to ...\nType: {}\nName: {}\nIP: {}\n", ch, GetProtocol(), GetName(), GetIP());

    int32_t sc;
    auto o = GetOutput(ch, sc);

    if (o == nullptr) {
        res += std::format("Channel: INVALID {}\n", ch);
    }
    else {
        if (o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_xxxETHERNET) {
            res += std::format("Universe: {}\nChannel: {}\n", o->GetUniverseString(), sc);
        }
        else if (o->GetType() == OUTPUT_KINET) {
            res += std::format("Port: {}\nChannel: {}\n", o->GetUniverseString(), sc);
        }
        else if (o->GetType() == OUTPUT_OPC) {
            res += std::format("Channel: {}\nMessage Offset: {}\n", o->GetUniverseString(), sc);
        }
        else {
            res += std::format("Channel: {}\n", sc);
        }
    }

    if (!IsActive()) {
        res += " INACTIVE\n";
    }

    return res;
}

std::string ControllerEthernet::GetColumn3Label() const {

    if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_OPC || _type == OUTPUT_KINET) {
        if (_outputs.size() == 1) {
            return _outputs.front()->GetUniverseString();
        }
        else if (_outputs.size() > 1) {
            return _outputs.front()->GetUniverseString() + "-" + _outputs.back()->GetUniverseString();
        }
    }
    return std::to_string(GetId());
}

void ControllerEthernet::VMVChanged()
{
    SetUniversePerString(false);
    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        auto const& prefer = c->GetPreferredInputProtocol();
        bool const autoLayout = IsAutoLayout();
        auto const& disable_monitor = c->DisableMonitoring();
        if (disable_monitor) {
            SetMonitoring(false);
        }
        if (autoLayout) {
            if (!prefer.empty()) {
                SetProtocol(prefer);
            }
            auto const& state = c->GetPreferredState();
            if (!state.empty()) {
                SetActive(state);
            }
        }
    }
}

Output::PINGSTATE ControllerEthernet::Ping() {

    if (hasAlpha(_resolvedIp)) {
        for (auto& it : GetOutputs()) { // Get the actual IPOutput object
            IPOutput* ipOutput = dynamic_cast<IPOutput*>(it);
            if (ipOutput) {
                ipOutput->SetIP(this->GetResolvedIP(), true, true); // Re-resolve IP
                ip_utils::waitForAllToResolve();
                _resolvedIp = ipOutput->GetResolvedIP();
            }
        }
    }
    if (GetResolvedIP(false) == "MULTICAST") {
        _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE;
    } else if (_outputs.size() > 0) {
        std::string ip = GetResolvedIP(true);
        if (ip.empty()) {
            ip = GetIP();
        }
        _lastPingResult = dynamic_cast<IPOutput*>(_outputs.front())->Ping(ip, GetFPPProxy());
    } else {
        _lastPingResult = IPOutput::Ping( ip_utils::ResolveIP(_ip), GetFPPProxy());
    }
    return GetLastPingState();
}

void ControllerEthernet::AsyncPing() {
    if (!IsActive()) {
        // don't ping in-active controllers
        return;
    }

    // if one is already running dont start another
    if (_asyncPing.valid() && _asyncPing.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) return;

    _asyncPing = std::async(std::launch::async, &ControllerEthernet::Ping, this);
}

void ControllerEthernet::SetExpanded(bool expanded)
{
    if (_expanded != expanded) {
        _expanded = expanded;
        _dirty = true;
    }
}

std::string ControllerEthernet::GetExport() const {

    return std::format("{},{},{},{},{},{},,,\"{}\",{},{},{},{},{},{},{},{},{}",
                            GetName(),
                            GetStartChannel(),
                            GetEndChannel(),
                            GetVMV(),
                            GetProtocol(),
                            GetIP(),
                            GetDescription(),
                            GetColumn3Label(),
                            GetChannels(),
                            (IsActive() ? "" : "DISABLED"),
                            (IsSuppressDuplicateFrames() ? "SuppressDuplicates" : ""),
                            (IsAutoSize() ? "AutoSize" : ""),
                            (IsAutoLayout() ? "AutoLayout" : ""),
                            (IsAutoUpload() ? "AutoUpload" : ""),
                            (IsFullxLightsControl() ? "FullxLightsControl" : ""),
                            GetFPPProxy());
}

void ControllerEthernet::SetTransientData(int32_t& startChannel, int& nullnumber) {

    // copy down properties that should be on every output
    for (auto& it : _outputs) {
        if (it->GetType() == OUTPUT_E131) {
            dynamic_cast<E131Output*>(it)->SetPriority(_priority);
        }
        else if (it->GetType() == OUTPUT_ZCPP) {
            dynamic_cast<ZCPPOutput*>(it)->SetPriority(_priority);
        }
        else if (it->GetType() == OUTPUT_KINET) {
            dynamic_cast<KinetOutput*>(it)->SetVersion(_version);
        }
    }

    Controller::SetTransientData(startChannel, nullnumber);
}

bool ControllerEthernet::SupportsUpload() const {

    if (_type == OUTPUT_ZCPP) return false;

    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        return c->SupportsUpload();
    }
    return false;
}

#pragma endregion

#pragma region UI

#ifndef EXCLUDENETWORKUI
bool ControllerEthernet::SetChannelSize(int32_t channels, std::list<Model*> models, uint32_t universeSize)
{
    
    if (_outputs.size() == 0) return false;

    for (auto& it2 : GetOutputs()) {
        it2->AllOff();
        it2->EndFrame(0);
    }
    
    if (_type == OUTPUT_ZCPP || _type == OUTPUT_DDP || _type == OUTPUT_TWINKLY) {
        _outputs.front()->SetChannels(channels);
        return true;
    }
    else {

        int channels_per_universe = universeSize;
        int universes = 0;
        if (IsUniversePerString() && models.size() > 0) {
            // number of universes should equal sum(((stringsize -1) / 510) + 1)
            int lastSerialPort = -1;
            for (const auto& m : models) {
                if (m->IsSerialProtocol() && m->GetControllerPort() == lastSerialPort) {
                    // skip this one
                } else if (m->IsSerialProtocol()) {
                    universes++;
                    lastSerialPort = m->GetControllerPort();
                }
                else {
                    for (int s = 0; s < m->GetNumPhysicalStrings(); s++) {
                        size_t chs = m->NodesPerString(s) * m->GetChanCountPerNode();
                        if (chs > 0) {
                            universes += ((chs - 1) / channels_per_universe) + 1;
                        }
                    }
                }
            }
        }
        else 
        {
            if(SupportsUniversePerString())
            {
                if (_outputs.size() != 0) channels_per_universe = universeSize;
            } 
            else {

            if (_outputs.size() != 0) channels_per_universe = _outputs.front()->GetChannels();
            }

            //calculate required universes
            universes = (channels + channels_per_universe - 1) / channels_per_universe;
        }

        _forceSizes = false;

        //require a minimum of one universe
        universes = std::max(1, universes);

        auto const oldIP = _outputs.front()->GetIP();

        if (!IsUniversePerString() && SupportsUniversePerString())
        {
            while (_outputs.size()) {
            delete _outputs.back();
            _outputs.pop_back();
            }
         }

        //if required universes is less than num of outputs, remove unneeded universes
        while (universes < (int)_outputs.size()) {
            delete _outputs.back();
            _outputs.pop_back();
        }

        //if required universes is greater than  num of outputs, add needed universes
        int diff = universes - (int)_outputs.size();
        for (int i = 0; i < diff; i++) {
            auto lastUsedUniverse = 0;

            if(_outputs.size())
                lastUsedUniverse = _outputs.back()->GetUniverse();

            if (_type == OUTPUT_E131) {
                _outputs.push_back(new E131Output());
                if (dynamic_cast<E131Output*>(_outputs.back()) != nullptr) {
                    dynamic_cast<E131Output*>(_outputs.back())->SetPriority(_priority);
                }
            }
            else if (_type == OUTPUT_ARTNET) {
                _outputs.push_back(new ArtNetOutput());
            }
            else if (_type == OUTPUT_KINET) {
                _outputs.push_back(new KinetOutput());
                if (dynamic_cast<KinetOutput*>(_outputs.back()) != nullptr) {
                    dynamic_cast<KinetOutput*>(_outputs.back())->SetVersion(_version);
                }
            }
            else if (_type == OUTPUT_xxxETHERNET) {
                _outputs.push_back(new xxxEthernetOutput());
            }
            else if (_type == OUTPUT_OPC) {
                _outputs.push_back(new OPCOutput());
            }
            _outputs.back()->SetChannels(channels_per_universe);
            _outputs.back()->SetIP(oldIP, IsActive());
            _outputs.back()->SetUniverse(lastUsedUniverse + 1);
            _outputs.back()->SetFPPProxyIP(_fppProxy);
            _outputs.back()->SetForceLocalIP(_forceLocalIP);
            _outputs.back()->SetSuppressDuplicateFrames(_suppressDuplicateFrames);
            _outputs.back()->Enable(IsActive());
        }

        if (IsUniversePerString() && models.size() > 0) {
            // now we have the right number of outputs ... we just need to set their sizes
            auto o = begin(_outputs);
            int lastSerialPort = -1;
            for (const auto& m : models) {
                if (m->IsSerialProtocol() && m->GetControllerPort() == lastSerialPort) {
                    // do nothing
                } else if (m->IsSerialProtocol()) {
                    // This wass a leap before you look bug... triggered with missing .xcontroller file
                    if (GetControllerCaps() == nullptr || GetControllerCaps()->NeedsFullUniverseForDMX()) {
                        (*o)->SetChannels(GetControllerCaps() == nullptr ? 510 : GetControllerCaps()->GetMaxSerialPortChannels()); // serial universes are always their max or 510 if we dont know the max
                        ++o;
                    } else {
                        // this is tricky ... we need to work out how many channels we need for this port
                        uint32_t chs = 0;
                        for (const auto& mm : models) {
                            if (mm->GetControllerProtocol() == m->GetControllerProtocol() && mm->GetControllerPort() == m->GetControllerPort()) {
                                chs += mm->GetChanCount();
                            }
                        }
                        if (chs == 0)
                            chs = 1;
                        (*o)->SetChannels(chs);
                        ++o;
                    }
                    lastSerialPort = m->GetControllerPort();
                } else {
                    for (int s = 0; s < m->GetNumPhysicalStrings(); s++) {
                        size_t chs = m->NodesPerString(s) * m->GetChanCountPerNode();

                        if (m->GetNumPhysicalStrings() == 1) {
                            chs = m->GetChanCount();
                        }

                        while (chs > 0) {
                            size_t uch = std::min(chs, (size_t)channels_per_universe);
                            wxASSERT(o != end(_outputs));
                            if (o == end(_outputs)) {
                                spdlog::debug("Unexpected error. Not enough outputs. Channels remaining: {}, Outputs size: {}", chs, _outputs.size());
                                return false;
                            }
                            (*o)->SetChannels(uch);
                            chs -= uch;
                            ++o;
                        }
                    }
                }
            }
        }
    }
    return true;
}
#endif // EXCLUDENETWORKUI

bool ControllerEthernet::SupportsUniversePerString() const
{
    auto eth = dynamic_cast<const ControllerEthernet*>(this);

    if (eth == nullptr) return false;

    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        return caps->SupportsUniversePerString();
    }
    return false;
}

// UI property grid methods (UpdateProperties, AddProperties, HandlePropertyEvent,
// ValidateProperties, SetProtocolAndRebuildProperties) moved to
// ui/controllerproperties/ControllerEthernetPropertyAdapter

void ControllerEthernet::AddOutput()
{
	if (_type == OUTPUT_E131) {
		_outputs.push_back(new E131Output());
	}
	else if (_type == OUTPUT_ARTNET) {
		_outputs.push_back(new ArtNetOutput());
	}
	else if (_type == OUTPUT_KINET) {
		_outputs.push_back(new KinetOutput());
	}
	else if (_type == OUTPUT_xxxETHERNET) {
		_outputs.push_back(new xxxEthernetOutput());
	}
	else if (_type == OUTPUT_OPC) {
		_outputs.push_back(new OPCOutput());
    } else if (_type == OUTPUT_PLAYER_ONLY) {
    }
    else
    {
		wxASSERT(false);
	}
    if (_outputs.size() > 0) {
        _outputs.back()->SetIP(_outputs.front()->GetIP(), IsActive());
        _outputs.back()->SetChannels(_outputs.front()->GetChannels());
        _outputs.back()->SetFPPProxyIP(_outputs.front()->GetFPPProxyIP());
        _outputs.back()->SetSuppressDuplicateFrames(_outputs.front()->IsSuppressDuplicateFrames());
        _outputs.back()->SetUniverse(_outputs.front()->GetUniverse() + _outputs.size() - 1);
        _outputs.back()->Enable(IsActive());
    }
}

void ControllerEthernet::SetAllSameSize(bool allSame, OutputModelManager* omm)
{
    if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_KINET) {
        _forceSizes = !allSame;

        if (allSame) {
            for (auto& it : _outputs) {
                it->SetChannels(_outputs.front()->GetChannels());
            }
        }
        #ifndef EXCLUDENETWORKUI
        if (omm != nullptr) {
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE |
                             OutputModelManager::WORK_UPDATE_NETWORK_PROPERTIES, "ControllerEthernet::SetAllSameSize");
            omm->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::SetAllSameSize", nullptr);
        }
        #endif
    }
}

#pragma endregion
