
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerEthernetPropertyAdapter.h"
#include "OutputPropertyAdapters.h"

#include <wx/settings.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/msgdlg.h>

#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/OutputManager.h"
#include "outputs/E131Output.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/KinetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/xxxEthernetOutput.h"
#include "outputs/OPCOutput.h"
#include "outputs/TwinklyOutput.h"
#include "OutputModelManager.h"
#include "controllers/ControllerCaps.h"
#include "UtilFunctions.h"
#include "utils/ip_utils.h"
#include "models/ModelManager.h"
#include "models/Model.h"

#include "SpecialOptions.h"

#include <log.h>

static wxPGChoices __ethernetTypes;

static void InitialiseEthernetTypes(bool forceXXX) {
    if (__ethernetTypes.GetCount() == 0) {
        __ethernetTypes.Add(OUTPUT_E131);
        __ethernetTypes.Add(OUTPUT_ZCPP);
        __ethernetTypes.Add(OUTPUT_ARTNET);
        __ethernetTypes.Add(OUTPUT_DDP);
        __ethernetTypes.Add(OUTPUT_OPC);
        __ethernetTypes.Add(OUTPUT_KINET);
        if (SpecialOptions::GetOption("xxx") == "true" || forceXXX) {
            __ethernetTypes.Add(OUTPUT_xxxETHERNET);
        }
        __ethernetTypes.Add(OUTPUT_TWINKLY);
        __ethernetTypes.Add(OUTPUT_PLAYER_ONLY);
    } else if (forceXXX) {
        bool found = false;
        for (size_t i = 0; i < __ethernetTypes.GetCount(); i++) {
            if (__ethernetTypes.GetLabel(i) == OUTPUT_xxxETHERNET) { found = true; break; }
        }
        if (!found) __ethernetTypes.Add(OUTPUT_xxxETHERNET);
    }
}

static wxPGChoices GetEthernetProtocols(const ControllerEthernet& ethernet) {
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(&ethernet);
    if (caps == nullptr) {
        InitialiseEthernetTypes(ethernet.GetProtocol() == OUTPUT_xxxETHERNET);
        return __ethernetTypes;
    }
    wxPGChoices types;
    for (const auto& it : caps->GetInputProtocols()) {
        if (it == "e131") types.Add(OUTPUT_E131);
        else if (it == "zcpp") types.Add(OUTPUT_ZCPP);
        else if (it == "artnet") types.Add(OUTPUT_ARTNET);
        else if (it == "kinet") types.Add(OUTPUT_KINET);
        else if (it == "ddp") types.Add(OUTPUT_DDP);
        else if (it == "opc") types.Add(OUTPUT_OPC);
        else if (it == "player only") types.Add(OUTPUT_PLAYER_ONLY);
        else if (it == "twinkly") types.Add(OUTPUT_TWINKLY);
        else if (it == "xxx ethernet") {
            if (SpecialOptions::GetOption("xxx") == "true" || ethernet.GetProtocol() == OUTPUT_xxxETHERNET) {
                types.Add(OUTPUT_xxxETHERNET);
            }
        }
    }
    return types;
}

ControllerEthernetPropertyAdapter::ControllerEthernetPropertyAdapter(Controller& controller)
    : ControllerPropertyAdapter(controller)
    , _ethernet(static_cast<ControllerEthernet&>(controller)) {
}

bool ControllerEthernetPropertyAdapter::SupportsUniversePerString() const {
    auto caps = _ethernet.GetControllerCaps();
    if (caps != nullptr) {
        return caps->SupportsUniversePerString();
    }
    return false;
}

bool ControllerEthernetPropertyAdapter::SetChannelSize(int32_t channels, std::list<Model*> models, uint32_t universeSize) {

    auto const& outputs = _ethernet.GetOutputs();
    if (outputs.size() == 0) return false;

    for (auto& it2 : outputs) {
        it2->AllOff();
        it2->EndFrame(0);
    }

    auto protocol = _ethernet.GetProtocol();
    if (protocol == OUTPUT_ZCPP || protocol == OUTPUT_DDP || protocol == OUTPUT_TWINKLY) {
        _ethernet.GetFirstOutput()->SetChannels(channels);
        return true;
    } else {

        int channels_per_universe = universeSize;
        int universes = 0;
        if (_ethernet.IsUniversePerString() && models.size() > 0) {
            // number of universes should equal sum(((stringsize -1) / 510) + 1)
            int lastSerialPort = -1;
            for (const auto& m : models) {
                if (m->IsSerialProtocol() && m->GetControllerPort() == lastSerialPort) {
                    // skip this one
                } else if (m->IsSerialProtocol()) {
                    universes++;
                    lastSerialPort = m->GetControllerPort();
                } else {
                    for (int s = 0; s < m->GetNumPhysicalStrings(); s++) {
                        size_t chs = m->NodesPerString(s) * m->GetChanCountPerNode();
                        if (chs > 0) {
                            universes += ((chs - 1) / channels_per_universe) + 1;
                        }
                    }
                }
            }
        } else {
            if (SupportsUniversePerString()) {
                if (outputs.size() != 0) channels_per_universe = universeSize;
            } else {
                if (outputs.size() != 0) channels_per_universe = outputs.front()->GetChannels();
            }

            //calculate required universes
            universes = (channels + channels_per_universe - 1) / channels_per_universe;
        }

        _ethernet.SetForceSizes(false);

        //require a minimum of one universe
        universes = std::max(1, universes);

        auto const oldIP = outputs.front()->GetIP();

        if (!_ethernet.IsUniversePerString() && SupportsUniversePerString()) {
            _ethernet.ClearOutputs();
        }

        //if required universes is less than num of outputs, remove unneeded universes
        _ethernet.RemoveTrailingOutputs(universes);

        //if required universes is greater than num of outputs, add needed universes
        int diff = universes - (int)_ethernet.GetOutputCount();
        for (int i = 0; i < diff; i++) {
            auto lastUsedUniverse = 0;

            if (_ethernet.GetOutputCount() > 0)
                lastUsedUniverse = _ethernet.GetOutputs().back()->GetUniverse();

            Output* newOutput = nullptr;
            if (protocol == OUTPUT_E131) {
                auto* e131 = new E131Output();
                e131->SetPriority(_ethernet.GetPriority());
                newOutput = e131;
            } else if (protocol == OUTPUT_ARTNET) {
                newOutput = new ArtNetOutput();
            } else if (protocol == OUTPUT_KINET) {
                auto* kinet = new KinetOutput();
                kinet->SetVersion(_ethernet.GetVersion());
                newOutput = kinet;
            } else if (protocol == OUTPUT_xxxETHERNET) {
                newOutput = new xxxEthernetOutput();
            } else if (protocol == OUTPUT_OPC) {
                newOutput = new OPCOutput();
            }

            if (newOutput) {
                newOutput->SetChannels(channels_per_universe);
                newOutput->SetIP(oldIP, _ethernet.IsActive());
                newOutput->SetUniverse(lastUsedUniverse + 1);
                newOutput->SetFPPProxyIP(_ethernet.GetControllerFPPProxy());
                newOutput->SetForceLocalIP(_ethernet.GetControllerForceLocalIP());
                newOutput->SetSuppressDuplicateFrames(_ethernet.IsSuppressDuplicateFrames());
                newOutput->Enable(_ethernet.IsActive());
                _ethernet.AppendOutput(newOutput);
            }
        }

        if (_ethernet.IsUniversePerString() && models.size() > 0) {
            // now we have the right number of outputs ... we just need to set their sizes
            auto const& outs = _ethernet.GetOutputs();
            auto o = begin(outs);
            int lastSerialPort = -1;
            for (const auto& m : models) {
                if (m->IsSerialProtocol() && m->GetControllerPort() == lastSerialPort) {
                    // do nothing
                } else if (m->IsSerialProtocol()) {
                    if (_ethernet.GetControllerCaps() == nullptr || _ethernet.GetControllerCaps()->NeedsFullUniverseForDMX()) {
                        (*o)->SetChannels(_ethernet.GetControllerCaps() == nullptr ? 510 : _ethernet.GetControllerCaps()->GetMaxSerialPortChannels());
                        ++o;
                    } else {
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
                            wxASSERT(o != end(outs));
                            if (o == end(outs)) {
                                spdlog::debug("Unexpected error. Not enough outputs. Channels remaining: {}, Outputs size: {}", chs, outs.size());
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

void ControllerEthernetPropertyAdapter::UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties, OutputModelManager* outputModelManager) {
    ControllerPropertyAdapter::UpdateProperties(propertyGrid, modelManager, expandProperties, outputModelManager);

    auto protocol = _ethernet.GetProtocol();
    auto ip = _ethernet.GetIP();

    wxPGProperty* p = propertyGrid->GetProperty("Protocol");
    if (p) {
        wxPGChoices protocols = GetEthernetProtocols(_ethernet);
        p->SetChoices(protocols);
        if (EncodeChoices(protocols, protocol) == -1) {
            p->SetChoiceSelection(0);
            SetProtocolAndRebuildProperties(DecodeChoices(protocols, 0), propertyGrid, outputModelManager);
        } else {
            p->SetChoiceSelection(EncodeChoices(protocols, protocol));
        }
    }
    p = propertyGrid->GetProperty("Multicast");
    if (p) {
        if (protocol == OUTPUT_E131) {
            p->SetValue(ip == "MULTICAST");
            p->Hide(false);
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("IP");
    if (ip != "MULTICAST") {
        p->Hide(false);
        p->SetValue(ip);
        if (protocol == OUTPUT_ZCPP || protocol == OUTPUT_DDP) {
            p->SetHelpString("This must be unique across all controllers.");
        } else {
            p->SetHelpString("This should ideally be unique across all controllers.");
        }
    } else {
        p->Hide(true);
    }
    p = propertyGrid->GetProperty("Priority");
    if (p) {
        if (protocol == OUTPUT_E131 || protocol == OUTPUT_ZCPP) {
            p->Hide(false);
            p->SetValue(_ethernet.GetPriority());
        } else {
            p->Hide(true);
        }
    }
    p = propertyGrid->GetProperty("Managed");
    if (p) {
        p->Hide(false);
        p->SetValue(_ethernet.IsManaged());
        if (!_ethernet.IsManaged()) {
            p->SetHelpString("This controller cannot be made managed until all other controllers with the same IP address are removed.");
        } else {
            p->SetHelpString("");
        }
    }
    p = propertyGrid->GetProperty("FPPProxy");
    if (p) {
        if (_ethernet.IsFPPProxyable()) {
            p->Hide(false);
            p->SetValue(_ethernet.GetControllerFPPProxy());
            if (!_ethernet.GetControllerFPPProxy().empty() && (_ethernet.GetControllerFPPProxy() == ip || !ip_utils::IsIPValidOrHostname(_ethernet.GetControllerFPPProxy()))) {
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
        if (protocol != OUTPUT_PLAYER_ONLY) {
            p->Hide(false);

            auto ips = GetLocalIPs();
            int val = 0;
            int idx = 1;
            auto forceLocalIP = _ethernet.GetControllerForceLocalIP();
            for (const auto& it : ips) {
                if (it == forceLocalIP)
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
        if (protocol == OUTPUT_PLAYER_ONLY) {
            p->Hide(true);
        } else {
            p->Hide(false);
            p->SetValue(modelManager->GetModelsOnChannels(_ethernet.GetStartChannel(), _ethernet.GetEndChannel(), -1));
            p->SetHelpString(modelManager->GetModelsOnChannels(_ethernet.GetStartChannel(), _ethernet.GetEndChannel(), 4));
        }
    }

    auto const& outputs = _ethernet.GetOutputs();
    if (outputs.size() >= 1) {
        auto adapter = OutputPropertyAdapter::Create(*outputs.front());
        adapter->UpdateProperties(propertyGrid, &_ethernet, modelManager, expandProperties);
    }
}

void ControllerEthernetPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    ControllerPropertyAdapter::AddProperties(propertyGrid, modelManager, expandProperties);

    auto ip = _ethernet.GetIP();
    auto protocol = _ethernet.GetProtocol();

    wxPGProperty* p = nullptr;
    p = propertyGrid->Append(new wxBoolProperty("Multicast", "Multicast", ip == "MULTICAST"));
    p->SetEditor("CheckBox");

    p = propertyGrid->Append(new wxStringProperty("IP Address", "IP", ip));

    p = propertyGrid->Append(new wxStringProperty("FPP Proxy IP/Hostname", "FPPProxy", _ethernet.GetControllerFPPProxy()));
    p->SetHelpString("This is typically the WIFI IP of a FPP instance that bridges two networks.");

    auto protocols = GetEthernetProtocols(_ethernet);
    p = propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, EncodeChoices(protocols, protocol)));

    auto ips = GetLocalIPs();
    wxPGChoices choices;
    choices.Add("");
    for (const auto& it : ips) {
        choices.Add(it);
    }
    propertyGrid->Append(new wxEnumProperty("Force Local IP", "ForceLocalIP", choices, 0));

    p = propertyGrid->Append(new wxUIntProperty("Priority", "Priority", _ethernet.GetPriority()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("Some controllers can receive data from more than one source and will ignore one of the sources where this priority is lower.");

    p = propertyGrid->Append(new wxStringProperty("Models", "Models", ""));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    auto p2 = propertyGrid->Append(new wxBoolProperty("Managed", "Managed", _ethernet.IsManaged()));
    p2->SetEditor("CheckBox");
    p2->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p2->ChangeFlag(wxPGFlags::ReadOnly, true);

    bool allSameSize = _ethernet.AllSameSize();
    auto const& outputs = _ethernet.GetOutputs();
    if (outputs.size() >= 1) {
        auto adapter = OutputPropertyAdapter::Create(*outputs.front());
        adapter->AddProperties(propertyGrid, p2, &_ethernet, allSameSize, expandProperties);
    }
}

bool ControllerEthernetPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (ControllerPropertyAdapter::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "IP") {
        _ethernet.SetIP(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::IP");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::IP", nullptr);
        return true;
    } else if (name == "Multicast") {
        if (event.GetValue().GetBool()) {
            _ethernet.SetIP("MULTICAST");
        } else {
            _ethernet.SetIP("");
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerEthernet::HandlePropertyEvent::Multicast");
        return true;
    } else if (name == "Priority") {
        _ethernet.SetPriority(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Priority");
        return true;
    } else if (name == "Version") {
        _ethernet.SetVersion(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerEthernet::HandlePropertyEvent::Version");
        return true;
    } else if (name == "FPPProxy") {
        _ethernet.SetFPPProxy(event.GetValue().GetString().Trim(true).Trim(false));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerEthernet::HandlePropertyEvent::FPPProxy");
        return true;
    } else if (name == "ForceLocalIP") {
        auto ips = GetLocalIPs();

        if (event.GetValue().GetLong() == 0) {
            _ethernet.SetForceLocalIP("");
        } else {
            if (event.GetValue().GetLong() >= (long)ips.size() + 1) { // need to add one as dropdown has blank first entry
                // likely the number of IPs changed after the list was loaded so ignore
            } else {
                auto it = begin(ips);
                std::advance(it, event.GetValue().GetLong() - 1);
                _ethernet.SetForceLocalIP(*it);
            }
        }

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerEthernet::HandlePropertyEvent::ForceLocalIP");
        return true;
    } else if (name == "Managed") {
        _ethernet.SetManaged(event.GetValue().GetBool());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerEthernet::HandlePropertyEvent::Managed");
        return true;
    } else if (name == "Protocol") {
        auto protocols = GetEthernetProtocols(_ethernet);
        wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
        SetProtocolAndRebuildProperties(DecodeChoices(protocols, event.GetValue().GetLong()), grid, outputModelManager);
        return true;
    } else if (name == "Universe") {
        int univ = event.GetValue().GetLong();
        for (auto& it : _ethernet.GetOutputs()) {
            it->SetUniverse(univ++);
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::Universe");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Universe", nullptr);
        return true;
    } else if (name == "Universes") {
        // add universes
        while ((long)_ethernet.GetOutputCount() < event.GetValue().GetLong()) {
            _ethernet.AddOutput();
        }

        // drop universes
        _ethernet.RemoveTrailingOutputs(event.GetValue().GetLong());

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::Universes");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Universes", nullptr);
        return true;
    } else if (name == "UniversePerString") {
        _ethernet.SetUniversePerString(event.GetValue().GetBool());

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::UniversePerString");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::UniversePerString", nullptr);
        return true;
    } else if (name == "IndivSizes") {
        bool forceSizes = event.GetValue().GetBool();

        // Let user stop this if they didnt understand the implications
        if (!forceSizes && !_ethernet.AllSameSize()) {
            if (wxMessageBox(wxString::Format("Are you sure you want to set all universes to %ld channels?", (long)_ethernet.GetFirstOutput()->GetChannels()), "Confirm resize?", wxICON_QUESTION | wxYES_NO) != wxYES) {
                event.GetProperty()->SetValue(wxVariant(true));
                return true;
            }
        }
        _ethernet.SetAllSameSize(!forceSizes, outputModelManager);
        return true;
    } else if (name == "Channels") {
        for (auto& it : _ethernet.GetOutputs()) {
            it->SetChannels(event.GetValue().GetLong());
            outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::Channels");
            outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Channels", nullptr);
        }
        return true;
    } else if (StartsWith(name, "Channels/")) {
        // the property label is the universe number so we look up the output based on that
        wxString n = event.GetProperty()->GetLabel();
        if (n.Contains("or")) n = n.AfterLast(' ');
        int univ = wxAtoi(n);
        for (auto& it : _ethernet.GetOutputs()) {
            if (it->GetUniverse() == univ) {
                it->SetChannels(event.GetValue().GetLong());
                outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::Channels/");
                outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Channels/", nullptr);
                return true;
            }
        }
    }

    auto const& outputs = _ethernet.GetOutputs();
    if (outputs.size() == 1) {
        auto adapter = OutputPropertyAdapter::Create(*outputs.front());
        if (adapter->HandlePropertyEvent(event, outputModelManager, &_ethernet)) return true;
    } else if (outputs.size() > 1) {
        auto it = outputs.begin();
        auto adapter = OutputPropertyAdapter::Create(**it);
        if (adapter->HandlePropertyEvent(event, outputModelManager, &_ethernet)) {
            ++it;
            while (it != outputs.end()) {
                auto a = OutputPropertyAdapter::Create(**it);
                a->HandlePropertyEvent(event, outputModelManager, &_ethernet);
                ++it;
            }
            return true;
        }
    }
    return false;
}

void ControllerEthernetPropertyAdapter::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    ControllerPropertyAdapter::ValidateProperties(om, propGrid);

    auto protocol = _ethernet.GetProtocol();
    auto ip = _ethernet.GetIP();
    auto const& outputs = _ethernet.GetOutputs();

    auto p = propGrid->GetPropertyByName("Protocol");
    auto caps = ControllerCaps::GetControllerConfig(&_ethernet);
    if (caps != nullptr && p != nullptr) {
        // controller must support the protocol
        if (!caps->IsValidInputProtocol(Lower(protocol))) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (ip != "MULTICAST") {
        p = propGrid->GetPropertyByName("IP");
        if (ip == "") {
            p->SetBackgroundColour(*wxRED);
        }
        // ZCPP and DDP cannot share IP with any other output
        else if (protocol == OUTPUT_ZCPP || protocol == OUTPUT_DDP) {
            p = propGrid->GetPropertyByName("IP");
            bool err = false;
            for (const auto& it : om->GetControllers(ip)) {
                if (it != &_ethernet) {
                    err = true;
                }
            }

            if (!ip_utils::IsIPValidOrHostname(ip)) {
                err = true;
            }

            if (err) {
                p->SetBackgroundColour(*wxRED);
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        } else {
            if (!ip_utils::IsIPValidOrHostname(ip)) {
                p->SetBackgroundColour(*wxRED);
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    p = propGrid->GetPropertyByName("Universe");
    if (p != nullptr) {
        long u = p->GetValue().GetLong();
        bool valid = true;

        if (protocol == OUTPUT_ARTNET) {
            if (u < 0 || u > 32767) {
                valid = false;
            }
        } else if (protocol == OUTPUT_E131) {
            if (u < 1 || u > 64000) {
                valid = false;
            }
        } else if (protocol == OUTPUT_OPC || protocol == OUTPUT_KINET) {
            if (u < 1 || u > 255) {
                valid = false;
            }
        }

        if (valid) {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        } else {
            p->SetBackgroundColour(*wxRED);
        }
    }

    p = propGrid->GetPropertyByName("Universes");
    if (caps != nullptr && p != nullptr && (protocol == OUTPUT_E131 || protocol == OUTPUT_ARTNET || protocol == OUTPUT_KINET)) {
        if ((int)outputs.size() > caps->GetMaxInputE131Universes()) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    if (caps != nullptr && p != nullptr && (protocol == OUTPUT_E131 || protocol == OUTPUT_ARTNET || protocol == OUTPUT_KINET)) {
        if (outputs.front()->GetChannels() > caps->GetMaxInputUniverseChannels()) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (caps != nullptr && (protocol == OUTPUT_E131 || protocol == OUTPUT_ARTNET || protocol == OUTPUT_KINET)) {
        for (const auto& it : outputs) {
            p = propGrid->GetPropertyByName("Channels/" + it->GetUniverseString());
            if (p != nullptr) {
                if (it->GetChannels() > caps->GetMaxInputUniverseChannels()) {
                    p->SetBackgroundColour(*wxRED);
                } else {
                    p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                }
            }
        }
    }

    if (_ethernet.IsFPPProxyable()) {
        p = propGrid->GetPropertyByName("FPPProxy");
        if (!_ethernet.GetControllerFPPProxy().empty() && (_ethernet.GetControllerFPPProxy() == ip || !ip_utils::IsIPValidOrHostname(_ethernet.GetControllerFPPProxy()))) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}

void ControllerEthernetPropertyAdapter::SetProtocolAndRebuildProperties(const std::string& protocol, wxPropertyGrid* grid, OutputModelManager* outputModelManager) {
    auto const& outputs = _ethernet.GetOutputs();
    if (outputs.size() > 0) {
        auto adapter = OutputPropertyAdapter::Create(*outputs.front());
        adapter->RemoveProperties(grid);
    }
    _ethernet.SetProtocol(protocol);

    if (_ethernet.GetOutputCount() > 0) {
        std::list<wxPGProperty*> expandProperties;
        auto before = grid->GetProperty("Managed");
        auto adapter = OutputPropertyAdapter::Create(*_ethernet.GetFirstOutput());
        adapter->AddProperties(grid, before, &_ethernet, _ethernet.AllSameSize(), expandProperties);
    }
    outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerEthernet::HandlePropertyEvent::Protocol");
    outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerEthernet::HandlePropertyEvent::Protocol", nullptr);
}

void ControllerEthernetPropertyAdapter::HandleExpanded(wxPropertyGridEvent& event, bool expanded) {
    _ethernet.SetExpanded(expanded);
}
