
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
#include "xxxEthernetOutput.h"
#include "OPCOutput.h"
#include "../controllers/ControllerCaps.h"
#include "../models/ModelManager.h"

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
        else if (it == "ddp") types.Add(OUTPUT_DDP);
        else if (it == "opc") types.Add(OUTPUT_OPC);
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
        if (SpecialOptions::GetOption("xxx") == "true" || forceXXX) {
            __types.Add(OUTPUT_xxxETHERNET);
        }
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
    _dirty = false;
}

ControllerEthernet::ControllerEthernet(OutputManager* om, bool acceptDuplicates) : Controller(om) {

    _managed = !acceptDuplicates;
    InitialiseTypes(false);
    _name = om->UniqueName("Ethernet_");
    _type = OUTPUT_E131;
    E131Output* o = new E131Output();
    _outputs.push_back(o);
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

    return um;
}
#pragma endregion

#pragma region Getters and Setters
void ControllerEthernet::SetIP(const std::string& ip) {

    auto const& iip = CleanupIP(ip);
    if (_ip != iip) {
        _ip = iip;
        _resolvedIp = ResolveIP(_ip);
        _dirty = true;
        _outputManager->UpdateUnmanaged();

        for (auto& it : GetOutputs()) {
            it->SetIP(_ip);
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

    if (_type == OUTPUT_ZCPP || _type == OUTPUT_DDP) {
        if (_type == OUTPUT_ZCPP) {
            _outputs.push_back(new ZCPPOutput());
        }
        else if (_type == OUTPUT_DDP) {
            _outputs.push_back(new DDPOutput());
        }
        _outputs.front()->SetChannels(totchannels);
        _outputs.front()->SetFPPProxyIP(oldoutputs.front()->GetFPPProxyIP());
        _outputs.front()->SetIP(oldoutputs.front()->GetIP());
        _outputs.front()->SetSuppressDuplicateFrames(oldoutputs.front()->IsSuppressDuplicateFrames());
    }
    else {
        if (oldtype == OUTPUT_E131 || oldtype == OUTPUT_ARTNET || oldtype == OUTPUT_xxxETHERNET || oldtype == OUTPUT_OPC) {
            for (const auto& it : oldoutputs) {
                if (_type == OUTPUT_E131) {
                    _outputs.push_back(new E131Output());
                }
                else if (_type == OUTPUT_ARTNET) {
                    _outputs.push_back(new ArtNetOutput());
                }
                else if (_type == OUTPUT_xxxETHERNET) {
                    _outputs.push_back(new xxxEthernetOutput());
                }
                else if (_type == OUTPUT_OPC) {
                    _outputs.push_back(new OPCOutput());
                }
                _outputs.back()->SetIP(oldoutputs.front()->GetIP());
                _outputs.back()->SetUniverse(it->GetUniverse());
                _outputs.back()->SetChannels(it->GetChannels());
            }
        }
        else {
            #define CONVERT_CHANNELS_PER_UNIVERSE 510
            int universes = (totchannels + CONVERT_CHANNELS_PER_UNIVERSE - 1) / CONVERT_CHANNELS_PER_UNIVERSE;
            int left = totchannels;
            for (int i = 0; i < universes; i++) {
                if (_type == OUTPUT_E131) {
                    _outputs.push_back(new E131Output());
                }
                else if (_type == OUTPUT_ARTNET) {
                    _outputs.push_back(new ArtNetOutput());
                }
                else if (_type == OUTPUT_xxxETHERNET) {
                    _outputs.push_back(new xxxEthernetOutput());
                }
                else if (_type == OUTPUT_OPC) {
                    _outputs.push_back(new OPCOutput());
                }
                _outputs.back()->SetChannels(left > CONVERT_CHANNELS_PER_UNIVERSE ? CONVERT_CHANNELS_PER_UNIVERSE : left);
                left -= _outputs.back()->GetChannels();
                _outputs.back()->SetIP(oldoutputs.front()->GetIP());
                _outputs.back()->SetUniverse(i + 1);
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

void ControllerEthernet::SetFPPProxy(const std::string& proxy) { 

    if (_fppProxy != proxy) { 
        _fppProxy = proxy; 
        _dirty = true; 
        for (auto& it : _outputs) {
            it->SetFPPProxyIP(proxy);
        }
    } 
}

std::string ControllerEthernet::GetFPPProxy() const {

    if (_fppProxy != "") {
        return _fppProxy;
    }
    return _outputManager->GetGlobalFPPProxy();
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

std::string ControllerEthernet::GetLongDescription() const {

    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + " " + GetProtocol() + " " + GetIP() + " ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

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

std::string ControllerEthernet::GetChannelMapping(int32_t ch) const {

    wxString res = wxString::Format("Channel %d maps to ...\nType: %s\nName: %s\nIP: %s\n", ch, GetProtocol(), GetName(), GetIP());

    int32_t sc;
    auto o = GetOutput(ch, sc);

    if (o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_xxxETHERNET) {
        res += wxString::Format("Universe: %s\nChannel: %d\n", o->GetUniverseString(), sc);
    }
    else if (o->GetType() == OUTPUT_OPC)
    {
        res += wxString::Format("Channel: %s\nMessage Offset: %d\n", o->GetUniverseString(), sc);
    }
    else {
        res += wxString::Format("Channel: %d\n", sc);
    }

    if (!IsActive()) {
        res += " INACTIVE\n";
    }

    return res;
}

std::string ControllerEthernet::GetColumn3Label() const {

    if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_OPC) {
        if (_outputs.size() == 1) {
            return _outputs.front()->GetUniverseString();
        }
        else if (_outputs.size() > 1) {
            return _outputs.front()->GetUniverseString() + "-" + _outputs.back()->GetUniverseString();
        }
    }
    return wxString::Format("%d", GetId());
}

Output::PINGSTATE ControllerEthernet::Ping() {

    if (GetResolvedIP() == "MULTICAST") {
        _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE;
    }
    else {
        _lastPingResult = dynamic_cast<IPOutput*>(_outputs.front())->Ping(GetResolvedIP(), GetFPPProxy());
    }
    return GetLastPingState();
}

void ControllerEthernet::AsyncPing() {

    // if one is already running dont start another
    if (_asyncPing.valid() && _asyncPing.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) return;

    _asyncPing = std::async(std::launch::async, &ControllerEthernet::Ping, this);
}

std::string ControllerEthernet::GetExport() const {

    return wxString::Format("%s,%d,%d,%s,%s,%s,,,\"%s\",%s,%d,%s,%s,%s,%s",
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
        GetFPPProxy()
    );
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

bool ControllerEthernet::SetChannelSize(int32_t channels) {
    if (_outputs.size() == 0) return false;

    for (auto& it2 : GetOutputs()) {
        it2->AllOff();
        it2->EndFrame(0);
    }

    if (_type == OUTPUT_ZCPP || _type == OUTPUT_DDP) {
        _outputs.front()->SetChannels(channels);
        return true;
    }
    else {
        int channels_per_universe = 510;
        if(_outputs.size() != 0)
            channels_per_universe = _outputs.front()->GetChannels();

        //calculate required universes
        int universes = (channels + channels_per_universe - 1) / channels_per_universe;
        _forceSizes = false;
        //require a minimum of one universe
        universes = std::max(1, universes);

        //if required universes equals  num of outputs, there is enough channels, return true
        if (universes == _outputs.size()) {
            return true;
        }

        auto const oldIP = _outputs.front()->GetIP();
        _forceSizes = false;

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
            else if (_type == OUTPUT_xxxETHERNET) {
                _outputs.push_back(new xxxEthernetOutput());
            }
            else if (_type == OUTPUT_OPC) {
                _outputs.push_back(new OPCOutput());
            }
            _outputs.back()->SetChannels(channels_per_universe);
            _outputs.back()->SetIP(oldIP);
            _outputs.back()->SetUniverse(lastUsedUniverse + 1);
            _outputs.back()->SetFPPProxyIP(_fppProxy);
            _outputs.back()->SetSuppressDuplicateFrames(_suppressDuplicateFrames);
        }
    }
    return true;
}

#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ControllerEthernet::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager) {

    Controller::AddProperties(propertyGrid, modelManager);

    wxPGProperty* p = nullptr;
    if (_type == OUTPUT_E131) {
        p = propertyGrid->Append(new wxBoolProperty("Multicast", "Multicast", _ip == "MULTICAST"));
        p->SetEditor("CheckBox");
    }

    if (_ip != "MULTICAST") {
        p = propertyGrid->Append(new wxStringProperty("IP Address", "IP", _ip));
        if (GetProtocol() == OUTPUT_ZCPP || GetProtocol() == OUTPUT_DDP) {
            p->SetHelpString("This must be unique across all controllers.");
        }
        else {
            p->SetHelpString("This should ideally be unique across all controllers.");
        }
    }

    auto protocols = GetProtocols();
    p = propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, EncodeChoices(protocols, _type)));

    bool allSameSize = AllSameSize();
    if (_outputs.size() == 1) {
        _outputs.front()->AddProperties(propertyGrid, allSameSize);
    }

    if (_type == OUTPUT_E131 || _type == OUTPUT_ZCPP) {
        p = propertyGrid->Append(new wxUIntProperty("Priority", "Priority", _priority));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");
    }

    if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_OPC) {
        p = propertyGrid->Append(new wxBoolProperty("Managed", "Managed", _managed));
        p->SetEditor("CheckBox");
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPG_PROP_READONLY, true);
        if (!_managed) {
            p->SetHelpString("This controller cannot be made managed until all other controllers with the same IP address are removed.");
        }
    }

    if (IsFPPProxyable()) {
        p = propertyGrid->Append(new wxStringProperty("FPP Proxy IP/Hostname", "FPPProxy", GetControllerFPPProxy()));
        p->SetHelpString("This is typically the WIFI IP of a FPP instance that bridges two networks.");
    }

    if (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET || _type == OUTPUT_xxxETHERNET || _type == OUTPUT_OPC) {
        auto u = "Start Universe";
        auto uc = "Universe Count";
        auto ud = "Universes";
        if (_type == OUTPUT_xxxETHERNET) {
            u = "Start Port";
            uc = "Port Count";
            ud = "Ports";
        }
        else if (_type == OUTPUT_OPC)
        {
            u = "Start OPC Channel";
            uc = "OPC Channel Count";
            ud = "OPC Channels";
        }
        p = propertyGrid->Append(new wxUIntProperty(u, "Universe", _outputs.front()->GetUniverse()));
        if (_type == OUTPUT_ARTNET)
            p->SetAttribute("Min", 0);
        else
            p->SetAttribute("Min", 1);
        if (_type == OUTPUT_OPC) {
            p->SetAttribute("Max", 255);
        }
        else {
            p->SetAttribute("Max", 64000);
        }
        p->SetEditor("SpinCtrl");

        if (_type != OUTPUT_OPC)
        {
            p = propertyGrid->Append(new wxUIntProperty(uc, "Universes", _outputs.size()));
            p->SetAttribute("Min", 1);
            p->SetAttribute("Max", 1000);

            if (IsAutoSize()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Universes Count cannot be changed when an output is set to Auto Size.");
            }
            else {
                p->SetEditor("SpinCtrl");
            }

            if (_outputs.size() > 1) {
                p = propertyGrid->Append(new wxStringProperty(ud, "UniversesDisplay", _outputs.front()->GetUniverseString() + "- " + _outputs.back()->GetUniverseString()));
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            }

            p = propertyGrid->Append(new wxBoolProperty("Individual Sizes", "IndivSizes", !allSameSize || _forceSizes));
            p->SetEditor("CheckBox");

            if (IsAutoSize()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Individual Sizes cannot be changed when an output is set to Auto Size.");
            }
        }

        if (!allSameSize || _forceSizes) {
            wxPGProperty* p2 = propertyGrid->Append(new wxPropertyCategory("Sizes", "Sizes"));
            for (const auto& it : _outputs) {
                p = propertyGrid->AppendIn(p2, new wxUIntProperty(it->GetUniverseString(), "Channels/" + it->GetUniverseString(), it->GetChannels()));
                p->SetAttribute("Min", 1);
                p->SetAttribute("Max", it->GetMaxChannels());
                p->SetEditor("SpinCtrl");
                auto modelsOnUniverse = modelManager->GetModelsOnChannels(it->GetStartChannel(), it->GetEndChannel(), 4);
                p->SetHelpString(wxString::Format("[%d-%d]\n", it->GetStartChannel(), it->GetEndChannel()) + modelsOnUniverse);
                if (modelsOnUniverse != "") {
                    if (wxSystemSettings::GetAppearance().IsDark()) {
                        p->SetBackgroundColour(wxColour(104, 128, 79));
                    } else {
                        p->SetBackgroundColour(wxColour(208, 255, 158));
                    }
                }
            }
        }
        else {
            std::string chlabel = "Channels";
            if (GetProtocol() == OUTPUT_E131 || GetProtocol() == OUTPUT_ARTNET)
            {
                chlabel = "Channels per Universe";
            }
            else if (GetProtocol() == OUTPUT_xxxETHERNET)
            {
                chlabel = "Channels per Port";
            }
            else if (GetProtocol() == OUTPUT_OPC)
            {
                chlabel = "Message Data Size";
            }
            p = propertyGrid->Append(new wxUIntProperty(chlabel, "Channels", _outputs.front()->GetChannels()));
            p->SetAttribute("Min", 1);
            p->SetAttribute("Max", _outputs.front()->GetMaxChannels());
            if (IsAutoSize()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
            }
            else {
                p->SetEditor("SpinCtrl");
            }

            p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1)));
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString(modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), 4));
        }
    }
    else {
        p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _outputs.front()->GetChannels()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", _outputs.front()->GetMaxChannels());

        if (IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        }
        else {
            p->SetEditor("SpinCtrl");
        }

        p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1)));
        p->ChangeFlag(wxPG_PROP_READONLY, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->SetHelpString(modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), 4));
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
    }
    else if (name == "Multicast") {
        if (event.GetValue().GetBool()) {
            SetIP("MULTICAST");
        }
        else {
            SetIP("");
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Multicast");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Multicast", nullptr);
        return true;
    }
    else if (name == "Priority") {
        SetPriority(_priority = event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Priority");
        return true;
    }
    else if (name == "FPPProxy") {
        SetFPPProxy(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::FPPProxy");
        return true;
    }
    else if (name == "Managed") {
        SetManaged(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Managed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Managed", nullptr);
        return true;
    }
    else if (name == "Protocol") {
        auto protocols = GetProtocols();
        SetProtocol(Controller::DecodeChoices(protocols, event.GetValue().GetLong()));

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Protocol");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
        return true;
    }
    else if (name == "Universe") {
        int univ = event.GetValue().GetLong();
        for (auto& it : _outputs) {
            it->SetUniverse(univ++);
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Universe");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        return true;
    }
    else if (name == "Universes") {
        // add universes
        while (_outputs.size() < event.GetValue().GetLong()) {
            if (_type == OUTPUT_E131) {
                _outputs.push_back(new E131Output());
            }
            else if (_type == OUTPUT_ARTNET) {
                _outputs.push_back(new ArtNetOutput());
            }
            else if (_type == OUTPUT_xxxETHERNET) {
                _outputs.push_back(new xxxEthernetOutput());
            }
            else if (_type == OUTPUT_OPC) {
                _outputs.push_back(new OPCOutput());
            }
            else {
                wxASSERT(false);
            }
            _outputs.back()->SetIP(_outputs.front()->GetIP());
            _outputs.back()->SetChannels(_outputs.front()->GetChannels());
            _outputs.back()->SetFPPProxyIP(_outputs.front()->GetFPPProxyIP());
            _outputs.back()->SetSuppressDuplicateFrames(_outputs.front()->IsSuppressDuplicateFrames());
            _outputs.back()->SetUniverse(_outputs.front()->GetUniverse() + _outputs.size() - 1);
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
    }
    else if (name == "IndivSizes") {

        _forceSizes = event.GetValue().GetBool();

        // Let user stop this if they didnt understand the implications
        if (!_forceSizes && !AllSameSize())
        {
            if (wxMessageBox(wxString::Format("Are you sure you want to set all universes to %ld channels?", (long)_outputs.front()->GetChannels()), "Confirm resize?", wxICON_QUESTION | wxYES_NO) != wxYES)
            {
                event.GetProperty()->SetValue(wxVariant(true));
                return true;
            }
        }

        if (!_forceSizes) {
            for (auto& it : _outputs) {
                it->SetChannels(_outputs.front()->GetChannels());
            }
        }

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::IndivSizes");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::IndivSizes", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::IndivSizes", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::IndivSizes", nullptr);
        return true;
    }
    else if (name == "Channels") {
        for (auto& it : _outputs) {
            it->SetChannels(event.GetValue().GetLong());
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Channels");
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
            outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
        }
        return true;
    }
    else if (StartsWith(name, "Channels/")) {
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
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager)) return true;
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
            if (err) {
                p->SetBackgroundColour(*wxRED);
            }
            else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    p = propGrid->GetPropertyByName("Universes");
    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET)) {
        if (_outputs.size() > caps->GetMaxInputE131Universes()) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET)) {
        if (_outputs.front()->GetChannels() > caps->GetMaxInputUniverseChannels()) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (caps != nullptr && p != nullptr && (_type == OUTPUT_E131 || _type == OUTPUT_ARTNET)) {
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
}
#endif
#pragma endregion
