
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

#include "ControllerEthernet.h"
#include "OutputManager.h"
#include "Output.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../OutputModelManager.h"
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
#include "../utils/ip_utils.h"

#ifndef EXCLUDENETWORKUI
#include "../xLightsMain.h"
#endif

#pragma region Property Choices
wxPGChoices ControllerEthernet::__types;

wxPGChoices ControllerEthernet::GetProtocols() const
{
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(this);

    if (caps == nullptr) return __types;

    wxPGChoices types;
    for (const auto& it : caps->GetInputProtocols()) {
        if (it == "e131") types.Add(OUTPUT_E131);
        else if (it == "zcpp") types.Add(OUTPUT_ZCPP);
        else if (it == "artnet") types.Add(OUTPUT_ARTNET);
        else if (it == "kinet") types.Add(OUTPUT_KINET);
        else if (it == "ddp") types.Add(OUTPUT_DDP);
        else if (it == "opc") types.Add(OUTPUT_OPC);
        else if (it == "player only")
            types.Add(OUTPUT_PLAYER_ONLY);
        else if (it == "twinkly")
            types.Add(OUTPUT_TWINKLY);
        else if (it == "xxx ethernet") {
            if (SpecialOptions::GetOption("xxx") == "true" || GetProtocol() == OUTPUT_xxxETHERNET) {
                types.Add(OUTPUT_xxxETHERNET);
            }
        }
    }
    return types;
}

void ControllerEthernet::InitialiseTypes(bool forceXXX) {

    if (__types.GetCount() == 0) {
        __types.Add(OUTPUT_E131);
        __types.Add(OUTPUT_ZCPP);
        __types.Add(OUTPUT_ARTNET);
        __types.Add(OUTPUT_DDP);
        __types.Add(OUTPUT_OPC);
        __types.Add(OUTPUT_KINET);
        if (SpecialOptions::GetOption("xxx") == "true" || forceXXX) {
            __types.Add(OUTPUT_xxxETHERNET);
        }
        __types.Add(OUTPUT_TWINKLY);
        __types.Add(OUTPUT_PLAYER_ONLY);
    }
    else if (forceXXX) {
        bool found = false;
        for (size_t i = 0; i < __types.GetCount(); i++) {
            if (__types.GetLabel(i) == OUTPUT_xxxETHERNET) {
                found = true;
                break;
            }
        }
        if (!found) {
            __types.Add(OUTPUT_xxxETHERNET);
        }
    }
}
#pragma endregion

#pragma region Constructors and Destructors
ControllerEthernet::ControllerEthernet(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir) {

    _type = node->GetAttribute("Protocol");
    InitialiseTypes(_type == OUTPUT_xxxETHERNET);
    SetIP(node->GetAttribute("IP"));
    SetFPPProxy(node->GetAttribute("FPPProxy"));
    SetPriority(wxAtoi(node->GetAttribute("Priority", "100")));
    SetVersion(wxAtoi(node->GetAttribute("Version", "1")));
    _expanded = node->GetAttribute("Expanded", "FALSE") == "TRUE";
    _universePerString = node->GetAttribute("UPS", "FALSE") == "TRUE";
    _forceLocalIP = node->GetAttribute("ForceLocalIP", "");
    _dirty = false;
}

ControllerEthernet::ControllerEthernet(OutputManager* om, bool acceptDuplicates) : Controller(om) {

    _managed = !acceptDuplicates;
    InitialiseTypes(false);
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

    // wait for an active ping to finish
    if (_asyncPing.valid()) {
        _asyncPing.wait_for(std::chrono::seconds(2));
    }
}

wxXmlNode* ControllerEthernet::Save() {

    wxXmlNode* um = Controller::Save();

    um->AddAttribute("IP", _ip);
    um->AddAttribute("Protocol", _type);
    um->AddAttribute("FPPProxy", _fppProxy);
    um->AddAttribute("Priority", wxString::Format("%d", _priority));
    um->AddAttribute("Version", wxString::Format("%d", _version));
    um->AddAttribute("Expanded", _expanded ? _("TRUE") : _("FALSE"));
    um->AddAttribute("UPS", _universePerString ? _("TRUE") : _("FALSE"));
    um->AddAttribute("ForceLocalIP", _forceLocalIP);

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
        if (IsActive()) _resolvedIp = ip_utils::ResolveIP(_ip);
        _dirty = true;
        if (_outputManager != nullptr) _outputManager->UpdateUnmanaged();

        for (auto& it : GetOutputs()) {
            it->SetIP(_ip, IsActive());
            it->SetResolvedIP(_resolvedIp);
        }
    }
}

// because we dont resolve IPs on creation for inactive controllers then if the controller is set active we need to resolve it then
void ControllerEthernet::PostSetActive()
{
    if (IsActive() && _ip != "" && _resolvedIp == "")
    {
        _resolvedIp = ip_utils::ResolveIP(_ip);
        for (auto& it : GetOutputs()) {
            it->SetResolvedIP(_resolvedIp);
        }
    }
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

    if (_fppProxy != "" && _fppProxy != _ip) {
        return _fppProxy;
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
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
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

void ControllerEthernet::Convert(wxXmlNode* node, std::string showDir) {

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
            InitialiseTypes(true);
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

    wxString res = wxString::Format("Channel %d maps to ...\nType: %s\nName: %s\nIP: %s\n", ch, GetProtocol(), GetName(), GetIP());

    int32_t sc;
    auto o = GetOutput(ch, sc);

    if (o == nullptr) {
        res += wxString::Format("Channel: INVALID %d\n", ch);
    }
    else {
        if (o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_xxxETHERNET) {
            res += wxString::Format("Universe: %s\nChannel: %d\n", o->GetUniverseString(), sc);
        }
        else if (o->GetType() == OUTPUT_KINET) {
            res += wxString::Format("Port: %s\nChannel: %d\n", o->GetUniverseString(), sc);
        }
        else if (o->GetType() == OUTPUT_OPC) {
            res += wxString::Format("Channel: %s\nMessage Offset: %d\n", o->GetUniverseString(), sc);
        }
        else {
            res += wxString::Format("Channel: %d\n", sc);
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
    return wxString::Format("%d", GetId());
}

void ControllerEthernet::VMVChanged(wxPropertyGrid *grid)
{
    SetUniversePerString(false);
    auto c = ControllerCaps::GetControllerConfig(_vendor, _model, _variant);
    if (c != nullptr) {
        auto const& prefer = c->GetPreferredInputProtocol();
        bool autoLayout = IsAutoLayout();
        if (!prefer.empty() && autoLayout) {
            #ifndef EXCLUDENETWORKUI
            if (grid && GetProtocol() != prefer) {
                if (_outputs.size() > 0) {
                    _outputs.front()->RemoveProperties(grid);
                }
                SetProtocol(prefer);
                if (_outputs.size() > 0) {
                    std::list<wxPGProperty *> expandProperties;
                    auto before = grid->GetProperty("Managed");
                    _outputs.front()->AddProperties(grid, before, this, AllSameSize(), expandProperties);
                }
            } else 
            #endif
            {
                SetProtocol(prefer);
            }
        }
    }
}

Output::PINGSTATE ControllerEthernet::Ping() {

    if (GetResolvedIP() == "MULTICAST") {
        _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE;
    } else if (_outputs.size() > 0) {
        std::string ip = GetResolvedIP();
        if (ip.empty()) {
            ip = GetIP();
        }
        _lastPingResult = dynamic_cast<IPOutput*>(_outputs.front())->Ping(ip, GetFPPProxy());
    }
    else {
        E131Output ipo;
        ipo.SetIP(_ip, IsActive());
        _lastPingResult = ipo.Ping(GetResolvedIP(), GetFPPProxy());
    }
    return GetLastPingState();
}

void ControllerEthernet::AsyncPing() {

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

    return wxString::Format("%s,%d,%d,%s,%s,%s,,,\"%s\",%s,%d,%s,%s,%s,%s,%s,%s,%s",
                            GetName(),
                            GetStartChannel(),
                            GetEndChannel(),
                            GetVMV(),
                            GetProtocol(),
                            GetIP(),
                            GetDescription(),
                            GetColumn3Label(),
                            GetChannels(),
                            (IsActive() ? _("") : _("DISABLED")),
                            (IsSuppressDuplicateFrames() ? _("SuppressDuplicates") : _("")),
                            (IsAutoSize() ? _("AutoSize") : _("")),
                            (IsAutoLayout() ? _("AutoLayout") : _("")),
                            (IsAutoUpload() ? _("AutoUpload") : _("")),
                            (IsFullxLightsControl() ? _("FullxLightsControl") : _("")),
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
                    for (size_t s = 0; s < m->GetNumPhysicalStrings(); s++) {
                        size_t chs = m->NodesPerString(s) * m->GetChanCountPerNode();
                        if (chs > 0) {
                            universes += ((chs - 1) / channels_per_universe) + 1;
                        }
                    }
                }
            }
        }
        else {
            if (_outputs.size() != 0) channels_per_universe = _outputs.front()->GetChannels();

            //calculate required universes
            universes = (channels + channels_per_universe - 1) / channels_per_universe;
        }

        _forceSizes = false;

        //require a minimum of one universe
        universes = std::max(1, universes);

        auto const oldIP = _outputs.front()->GetIP();

        //if required universes is less than num of outputs, remove unneeded universes
        while (universes < _outputs.size()) {
            delete _outputs.back();
            _outputs.pop_back();
        }

        //if required universes is greater than  num of outputs, add needed universes
        int diff = universes - _outputs.size();
        for (int i = 0; i < diff; i++) {
            auto const lastUsedUniverse = _outputs.back()->GetUniverse();
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
                    for (size_t s = 0; s < m->GetNumPhysicalStrings(); s++) {
                        size_t chs = m->NodesPerString(s) * m->GetChanCountPerNode();

                        if (m->GetNumPhysicalStrings() == 1) {
                            chs = m->GetChanCount();
                        }

                        while (chs > 0) {
                            size_t uch = std::min(chs, (size_t)channels_per_universe);
                            wxASSERT(o != end(_outputs));
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

void ControllerEthernet::UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    Controller::UpdateProperties(propertyGrid, modelManager, expandProperties);
    wxPGProperty *p = propertyGrid->GetProperty("Protocol");
    if (p) {
        wxPGChoices protocols = GetProtocols();
        p->SetChoices(protocols);
        if (EncodeChoices(protocols, _type) == -1) {
            p->SetChoiceSelection(0);
            SetProtocol(Controller::DecodeChoices(protocols, 0));
        }
        else
        {
            p->SetChoiceSelection(EncodeChoices(protocols, _type));
        }
    }
    p = propertyGrid->GetProperty("Multicast");
    if (p) {
        if (_type == OUTPUT_E131) {
            p->SetValue(_ip == "MULTICAST");
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IP");
    if (_ip != "MULTICAST") {
        p->Hide(false);
        p->SetValue(_ip);
        if (GetProtocol() == OUTPUT_ZCPP || GetProtocol() == OUTPUT_DDP) {
            p->SetHelpString("This must be unique across all controllers.");
        } else {
            p->SetHelpString("This should ideally be unique across all controllers.");
        }
    } else {
        p->Hide(true);
    }
    p = propertyGrid->GetProperty("Priority");
    if (p) {
        if (_type == OUTPUT_E131 || _type == OUTPUT_ZCPP) {
            p->Hide(false);
            p->SetValue(_priority);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("Managed");
    if (p) {
        if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_OPC || _type == OUTPUT_KINET) {
            p->Hide(false);
            p->SetValue(_managed);
            if (!_managed) {
                p->SetHelpString("This controller cannot be made managed until all other controllers with the same IP address are removed.");
            } else {
                p->SetHelpString("");
            }
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("FPPProxy");
    if (p) {
        if (IsFPPProxyable()) {
            p->Hide(false);
            p->SetValue(GetControllerFPPProxy());
            if (!GetControllerFPPProxy().empty() && (GetControllerFPPProxy() == _ip || !ip_utils::IsIPValidOrHostname(GetControllerFPPProxy()))) {
                p->SetBackgroundColour(*wxRED);
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("ForceLocalIP");
    if (p) {
        if (_type != OUTPUT_PLAYER_ONLY) {
            p->Hide(false);
                        
            auto ips = GetLocalIPs();
            int val = 0;
            int idx = 1;
            for (const auto& it : ips) {
                if (it == _forceLocalIP)
                    val = idx;
                ++idx;
            }
            p->SetValue(val);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("Models");
    if (p) {
        if (_type == OUTPUT_PLAYER_ONLY) {
            p->Hide(true);
        } else {
            p->Hide(false);
            p->SetValue(modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1));
            p->SetHelpString(modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), 4));
        }
    }
    
    if (_outputs.size() >= 1) {
        _outputs.front()->UpdateProperties(propertyGrid, this, modelManager, expandProperties);
    }
}

void ControllerEthernet::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    Controller::AddProperties(propertyGrid, modelManager, expandProperties);

    wxPGProperty* p = nullptr;
    p = propertyGrid->Append(new wxBoolProperty("Multicast", "Multicast", _ip == "MULTICAST"));
    p->SetEditor("CheckBox");

    p = propertyGrid->Append(new wxStringProperty("IP Address", "IP", _ip));

    p = propertyGrid->Append(new wxStringProperty("FPP Proxy IP/Hostname", "FPPProxy", GetControllerFPPProxy()));
    p->SetHelpString("This is typically the WIFI IP of a FPP instance that bridges two networks.");

    
    auto protocols = GetProtocols();
    p = propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, EncodeChoices(protocols, _type)));


    auto ips = GetLocalIPs();
    wxPGChoices choices;
    choices.Add("");
    for (const auto& it : ips) {
        choices.Add(it);
    }
    propertyGrid->Append(new wxEnumProperty("Force Local IP", "ForceLocalIP", choices, 0));

    p = propertyGrid->Append(new wxUIntProperty("Priority", "Priority", _priority));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Some controllers can receive data from more than one source and will ignore one of the sources where this priority is lower.");
    
    p = propertyGrid->Append(new wxStringProperty("Models", "Models", ""));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    auto p2 = propertyGrid->Append(new wxBoolProperty("Managed", "Managed", _managed));
    p2->SetEditor("CheckBox");
    p2->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p2->ChangeFlag(wxPG_PROP_READONLY, true);

    bool allSameSize = AllSameSize();
    if (_outputs.size() >= 1) {
        //FIXME
        _outputs.front()->AddProperties(propertyGrid, p2, this, allSameSize, expandProperties);
    }   
}

bool ControllerEthernet::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "IP") {
        SetIP(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::IP");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::IP", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::IP", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::IP", nullptr);
        return true;
    } else if (name == "Multicast") {
        if (event.GetValue().GetBool()) {
            SetIP("MULTICAST");
        } else {
            SetIP("");
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Multicast");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Multicast", nullptr);
        return true;
    } else if (name == "Priority") {
        SetPriority(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Priority");
        return true;
    } else if (name == "Version") {
        SetVersion(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Version");
        return true;
    } else if (name == "FPPProxy") {
        SetFPPProxy(event.GetValue().GetString().Trim(true).Trim(false));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::FPPProxy");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::FPPProxy", nullptr);
        return true;
    } else if (name == "ForceLocalIP") {
        auto ips = GetLocalIPs();

        if (event.GetValue().GetLong() == 0) {
            SetForceLocalIP("");
        } else {
            if (event.GetValue().GetLong() >= ips.size() + 1) { // need to add one as dropdown has blank first entry
                // likely the number of IPs changed after the list was loaded so ignore
            } else {
                auto it = begin(ips);
                std::advance(it, event.GetValue().GetLong() - 1);
                SetForceLocalIP(*it);
            }
        }

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::ForceLocalIP");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::ForceLocalIP", nullptr);
        return true;
    } else if (name == "Managed") {
        SetManaged(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Managed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Managed", nullptr);
        return true;
    } else if (name == "Protocol") {
        auto protocols = GetProtocols();
        
        if (_outputs.size() > 0) {
            wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
            _outputs.front()->RemoveProperties(grid);
        }
        SetProtocol(Controller::DecodeChoices(protocols, event.GetValue().GetLong()));
        
        if (_outputs.size() > 0) {
            wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
            std::list<wxPGProperty *> expandProperties;
            auto before = grid->GetProperty("Managed");
            _outputs.front()->AddProperties(grid, before, this, AllSameSize(), expandProperties);
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Protocol");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        return true;
    } else if (name == "Universe") {
        int univ = event.GetValue().GetLong();
        for (auto& it : _outputs) {
            it->SetUniverse(univ++);
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Universe");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        return true;
    } else if (name == "Universes") {
        // add universes
        while (_outputs.size() < event.GetValue().GetLong()) {
            AddOutput();
        }

        // drop universes
        while (_outputs.size() > event.GetValue().GetLong()) {
            delete _outputs.back();
            _outputs.pop_back();
        }

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Universes");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Universes", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Universes", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Universes", nullptr);
        return true;
    } else if (name == "UniversePerString") {

        SetUniversePerString(event.GetValue().GetBool());

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::UniversePerString");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::UniversePerString", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::UniversePerString", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::UniversePerString", nullptr);
        return true;
    } else if (name == "IndivSizes") {
        _forceSizes = event.GetValue().GetBool();

        // Let user stop this if they didnt understand the implications
        if (!_forceSizes && !AllSameSize()) {
            if (wxMessageBox(wxString::Format("Are you sure you want to set all universes to %ld channels?", (long)_outputs.front()->GetChannels()), "Confirm resize?", wxICON_QUESTION | wxYES_NO) != wxYES) {
                event.GetProperty()->SetValue(wxVariant(true));
                return true;
            }
        }
        SetAllSameSize(!_forceSizes, outputModelManager);
        return true;
    } else if (name == "Channels") {
        for (auto& it : _outputs) {
            it->SetChannels(event.GetValue().GetLong());
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Channels");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
        }
        return true;
    } else if (StartsWith(name, "Channels/")) {
        // the property label is the universe number so we look up the output based on that
        wxString n = event.GetProperty()->GetLabel();
        if (n.Contains("or")) n = n.AfterLast(' ');
        int univ = wxAtoi(n);
        for (auto& it : _outputs) {
            if (it->GetUniverse() == univ) {
                it->SetChannels(event.GetValue().GetLong());
                outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Channels/");
                outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Channels/", nullptr);
                outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Channels/", nullptr);
                outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Channels/", nullptr);
                return true;
            }
        }
    }

    if (_outputs.size() == 1) {
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager, this)) return true;
    } else if (_outputs.size() > 1) {
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager, this)) {
            auto it = _outputs.begin();
            ++it;
            while (it != _outputs.end()) {
                (*it)->HandlePropertyEvent(event, outputModelManager, this);
                ++it;
            }
            return true;
        }
    }
    return false;
}

void ControllerEthernet::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    Controller::ValidateProperties(om, propGrid);

    auto p = propGrid->GetPropertyByName("Protocol");
    auto caps = ControllerCaps::GetControllerConfig(this);
    if (caps != nullptr && p != nullptr) {
        // controller must support the protocol
        if (!caps->IsValidInputProtocol(Lower(_type))) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (GetIP() != "MULTICAST") {
        p = propGrid->GetPropertyByName("IP");
        if (GetIP() == "") {
            p->SetBackgroundColour(*wxRED);
        }
        // ZCPP and DDP cannot share IP with any other output
        else if (GetProtocol() == OUTPUT_ZCPP || GetProtocol() == OUTPUT_DDP) {
            p = propGrid->GetPropertyByName("IP");
            bool err = false;
            for (const auto& it : _outputManager->GetControllers(GetIP())) {
                if (it != this) {
                    err = true;
                }
            }

            if (!ip_utils::IsIPValidOrHostname(GetIP())) {
                err = true;
            }

            if (err) {
                p->SetBackgroundColour(*wxRED);
            }
            else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
        else {
            if (!ip_utils::IsIPValidOrHostname(GetIP())) {
                p->SetBackgroundColour(*wxRED);
            }
            else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    p = propGrid->GetPropertyByName("Universe");
    if (p != nullptr) {
        long u = p->GetValue().GetLong();
        bool valid = true;

        if (_type == OUTPUT_ARTNET) {
            if (u < 0 || u > 32767) {
                valid = false;
            }
        }
        else if (_type == OUTPUT_E131) {
            if (u < 1 || u > 64000) {
                valid = false;
            }
        }
        else if (_type == OUTPUT_OPC || _type == OUTPUT_KINET) {
            if (u < 1 || u > 255) {
                valid = false;
            }
        }

        if (valid) {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
        else {
            p->SetBackgroundColour(*wxRED);
        }
    }

    p = propGrid->GetPropertyByName("Universes");
    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_KINET)) {
        if (_outputs.size() > caps->GetMaxInputE131Universes()) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_KINET)) {
        if (_outputs.front()->GetChannels() > caps->GetMaxInputUniverseChannels()) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_KINET)) {
        for (const auto& it : _outputs) {
            p = propGrid->GetPropertyByName("Channels/" + it->GetUniverseString());
            if (p != nullptr) {
                if (it->GetChannels() > caps->GetMaxInputUniverseChannels()) {
                    p->SetBackgroundColour(*wxRED);
                }
                else {
                    p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                }
            }
        }
    }

    if (IsFPPProxyable()) {
        p = propGrid->GetPropertyByName("FPPProxy");
        if (!GetControllerFPPProxy().empty() && (GetControllerFPPProxy() == _ip || !ip_utils::IsIPValidOrHostname(GetControllerFPPProxy()))) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}
#endif

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
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::SetAllSameSize");
            omm->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::SetAllSameSize", nullptr);
            omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::SetAllSameSize", nullptr);
            omm->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_PROPERTIES, "ControllerEthernet::SetAllSameSize", nullptr);
            omm->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::SetAllSameSize", nullptr);
        }
#endif
    }
}

#pragma endregion
