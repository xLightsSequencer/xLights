
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

#include "ControllerSerial.h"
#include "../OutputModelManager.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../controllers/ControllerCaps.h"
#include "OutputManager.h"
#include "Output.h"
#include "SerialOutput.h"
#include "DLightOutput.h"
#include "DMXOutput.h"
#include "OpenDMXOutput.h"
#include "OpenPixelNetOutput.h"
#include "RenardOutput.h"
#include "LOROutput.h"
#include "LOROptimisedOutput.h"
#include "PixelNetOutput.h"
#include "xxxSerialOutput.h"
#include "../models/ModelManager.h"

#pragma region Property Choices
wxPGChoices ControllerSerial::__types;
wxPGChoices ControllerSerial::__ports;
wxPGChoices ControllerSerial::__speeds;

wxPGChoices ControllerSerial::GetProtocols() const
{
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(this);

    if (caps == nullptr) return __types;

    wxPGChoices types;
    for (const auto& it : caps->GetInputProtocols()) {
        if (it == "dmx") types.Add(OUTPUT_DMX);
        else if (it == "lor") types.Add(OUTPUT_LOR);
        else if (it == "lor optimised") types.Add(OUTPUT_LOR_OPT);
        else if (it == "opendmx") types.Add(OUTPUT_OPENDMX);
        else if (it == "pixelnet") types.Add(OUTPUT_PIXELNET);
        else if (it == "openpixelnet") types.Add(OUTPUT_OPENPIXELNET);
        else if (it == "renard") types.Add(OUTPUT_RENARD);
        else if (it == "dlight") types.Add(OUTPUT_DLIGHT);
        else if (it == "xxx serial") {
            if (SpecialOptions::GetOption("xxx") == "true" || GetProtocol() == OUTPUT_xxxSERIAL) {
                types.Add(OUTPUT_xxxSERIAL);
            }
        }
    }
    return types;
}

void ControllerSerial::InitialiseTypes(bool forceXXX) {

    if (__types.GetCount() == 0)  {
        __types.Add(OUTPUT_DMX);
        __types.Add(OUTPUT_LOR);
        __types.Add(OUTPUT_LOR_OPT);
        __types.Add(OUTPUT_OPENDMX);
        __types.Add(OUTPUT_PIXELNET);
        __types.Add(OUTPUT_OPENPIXELNET);
        __types.Add(OUTPUT_RENARD);
        __types.Add(OUTPUT_DLIGHT);
        if (forceXXX || SpecialOptions::GetOption("xxx") == "true") {
            __types.Add(OUTPUT_xxxSERIAL);
        }
    }
    else if (forceXXX) {
        bool found = false;
        for (size_t i = 0; i < __types.GetCount(); i++) {
            if (__types.GetLabel(i) == OUTPUT_xxxSERIAL) {
                found = true;
                break;
            }
        }
        if (!found) {
            __types.Add(OUTPUT_xxxSERIAL);
        }
    }

    if (__speeds.GetCount() == 0) {
        auto s = SerialOutput::GetPossibleBaudRates();
        for (const auto& it : s) {
            __speeds.Add(it);
        }
    }

    if (__ports.GetCount() == 0) {
        auto p = SerialOutput::GetPossibleSerialPorts();
        for (const auto& it : p) {
            __ports.Add(it);
        }
    }
}
#pragma endregion

#pragma region Constructors and Destructors
ControllerSerial::ControllerSerial(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir) {
    _type = node->GetAttribute("Protocol");
    InitialiseTypes(_type == OUTPUT_xxxSERIAL);
    SetPort(node->GetAttribute("Port"));
    SetSpeed(wxAtoi(node->GetAttribute("Speed")));
    _dirty = false;
}

ControllerSerial::ControllerSerial(OutputManager* om) : Controller(om) {
    InitialiseTypes(false);
    _name = om->UniqueName("Serial_");
    SerialOutput* o = new DMXOutput();
    o->SetChannels(512);
    _outputs.push_back(o);
    _type = OUTPUT_DMX;
    SetPort(_outputManager->GetFirstUnusedCommPort());
    SetSpeed(o->GetBaudRate());
}

wxXmlNode* ControllerSerial::Save() {

    wxXmlNode* um = Controller::Save();

    um->AddAttribute("Port", _port);
    um->AddAttribute("Speed", wxString::Format("%d", _speed));
    um->AddAttribute("Protocol", _type);

    return um;
}
#pragma endregion

#pragma region Getters and Setters
void ControllerSerial::SetPort(const std::string& port) {
    if (_outputs.front() != nullptr) {
        if (_port != port) {
            _outputs.front()->SetCommPort(port);
            _port = port;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetSpeed(int speed) {
    if (_outputs.front() != nullptr) {
        if (_speed != speed) {
            _outputs.front()->SetBaudRate(speed);
            _speed = speed;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetChannels(int channels) {
    if (_outputs.front() != nullptr) {
        if (_outputs.front()->GetChannels() != channels) {
            _outputs.front()->SetChannels(channels);
            _dirty = true;
        }
    }
}

void ControllerSerial::SetProtocol(const std::string& type)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_outputs.front() != nullptr) {
        if (_outputs.front()->GetType() != type) {
            _type = type;
            auto const s = _outputs.front()->GetBaudRate();
            auto const p = _outputs.front()->GetCommPort();
            auto const c = _outputs.front()->GetChannels();
            delete _outputs.front();
            _outputs.pop_front();

            Output* o = nullptr;
            if (type == OUTPUT_DLIGHT) {
                o = new DLightOutput();
            }
            else if (type == OUTPUT_DMX) {
                o = new DMXOutput();
            }
            else if (type == OUTPUT_LOR) {
                o = new LOROutput();
            }
            else if (type == OUTPUT_LOR_OPT) {
                o = new LOROptimisedOutput();
                SetAutoSize(false);
            }
            else if (type == OUTPUT_OPENDMX) {
                o = new OpenDMXOutput();
            }
            else if (type == OUTPUT_OPENPIXELNET) {
                o = new OpenPixelNetOutput();
            }
            else if (type == OUTPUT_RENARD) {
                o = new RenardOutput();
            }
            else if (type == OUTPUT_PIXELNET) {
                o = new PixelNetOutput();
            }
            else if (type == OUTPUT_xxxSERIAL) {
                o = new xxxSerialOutput();
            }
            else {
                wxASSERT(false);
                logger_base.error("Could not create serial output of type %s.", (const char*)type.c_str());
            }

            if (o != nullptr) {
                o->SetCommPort(p);
                o->SetBaudRate(s);
                o->SetChannels(c);
                _outputs.push_front(o);
                _dirty = true;
            }
        }
    }
}
#pragma endregion

#pragma region Virtual Functions
void ControllerSerial::SetId(int id) {

    Controller::SetId(id);
    if (GetProtocol() == OUTPUT_LOR_OPT) {
        dynamic_cast<LOROptimisedOutput*>(GetFirstOutput())->SetId(id);
    }
    else {
        dynamic_cast<SerialOutput*>(GetFirstOutput())->SetId(id);
    }
}

std::string ControllerSerial::GetLongDescription() const {
    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + " " + GetProtocol() + " " + GetPort();
    res += " (" + std::string(wxString::Format(wxT("%d"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ")";

    return res;
}

void ControllerSerial::Convert(wxXmlNode* node, std::string showDir) {

    Controller::Convert(node, showDir);

    _outputs.push_back(Output::Create(this, node, showDir));
    if (_name == "" || StartsWith(_name, "Serial_")) {
        if (_outputs.back()->GetDescription_CONVERT() != "") {
            _name = _outputManager->UniqueName(_outputs.back()->GetDescription_CONVERT());
        }
        else {
            _name = _outputManager->UniqueName("Unnamed");
        }
    }
    if (_outputs.back() == nullptr) {
        // this shouldnt happen unless we are loading a future file with an output type we dont recognise
        _outputs.pop_back();
    }

    wxASSERT(_outputs.size() == 1);

    if (_outputs.size() > 0) {
        _port = _outputs.front()->GetCommPort();
        _speed = _outputs.front()->GetBaudRate();
        _type = _outputs.front()->GetType();
        _id = _outputs.front()->GetUniverse();
        if (GetProtocol() == OUTPUT_xxxSERIAL) {
            InitialiseTypes(true);
        }
    }
}

std::string ControllerSerial::GetChannelMapping(int32_t ch) const
{
    return wxString::Format("Channel %ld maps to ...\nType: %s\nName: %s\nComPort: %s\nChannel: %ld\n%s",
        ch, 
        GetProtocol(), 
        GetName(), 
        GetPort(), 
        ch - GetStartChannel() + 1,
        (IsActive() ? _("") : _("INACTIVE\n")));
}

Output::PINGSTATE ControllerSerial::Ping() {

    _lastPingResult = dynamic_cast<SerialOutput*>(_outputs.front())->Ping();
    return GetLastPingState();
}

std::string ControllerSerial::GetExport() const {

    return wxString::Format("%s,%ld,%ld,%s,%s,,%s,%d,\"%s\",%d,%ld,%s,%s,%s",
        GetName(),
        GetStartChannel(),
        GetEndChannel(),
        GetVMV(),
        GetProtocol(),
        GetPort(),
        GetSpeed(),
        GetDescription(),
        GetId(),
        GetChannels(),
        (IsActive() ? _("") : _("DISABLED")),
        (IsSuppressDuplicateFrames() ? _("SuppressDuplicates") : _("")),
        (IsAutoSize() ? _("AutoSize") : _(""))
    );
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ControllerSerial::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager)
{
    Controller::AddProperties(propertyGrid, modelManager);

    wxPGProperty* p = propertyGrid->Append(new wxEnumProperty("Port", "Port", __ports, Controller::EncodeChoices(__ports, _port)));
    p->SetHelpString("This must be unique across all controllers.");

    p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", __speeds, Controller::EncodeChoices(__speeds, wxString::Format("%d", _speed))));
    if (dynamic_cast<SerialOutput*>(_outputs.front())) {
        if (!dynamic_cast<SerialOutput*>(_outputs.front())->AllowsBaudRateSetting()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetBackgroundColour(*wxLIGHT_GREY);
            p->SetHelpString("Speed is fixed for this protocol.");
        }
    }

    auto protocols = GetProtocols();
    p = propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, Controller::EncodeChoices(protocols, _type)));

    if (GetFirstOutput()->GetType() != OUTPUT_LOR_OPT)
    {
        p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _outputs.front()->GetChannels()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", _outputs.front()->GetMaxChannels());

        if (IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetBackgroundColour(*wxLIGHT_GREY);
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        }
        else {
            p->SetEditor("SpinCtrl");
        }
    }

    p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1)));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    if (_outputs.size() > 0) _outputs.front()->AddProperties(propertyGrid, true);
}

bool ControllerSerial::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "Port") {
        SetPort(Controller::DecodeChoices(__ports, event.GetValue().GetLong()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Port", nullptr);
        return true;
    }
    else if (name == "Speed") {
        SetSpeed(wxAtoi(Controller::DecodeChoices(__speeds, event.GetValue().GetLong())));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Speed", nullptr);
        return true;
    }
    else if (name == "Protocol") {
        auto protocols = GetProtocols();
        SetProtocol(Controller::DecodeChoices(protocols, event.GetValue().GetLong()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Protocol");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        return true;
    }
    else if (name == "Channels") {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    if (_outputs.size() > 0) {
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager)) return true;
    }

    return false;
}

void ControllerSerial::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    Controller::ValidateProperties(om, propGrid);

    for (const auto& it : om->GetControllers()) {
        auto s = dynamic_cast<ControllerSerial*>(it);

        // Port must be unique
        auto p = propGrid->GetPropertyByName("Port");
        if (s != nullptr && it->GetName() != GetName() && s->GetPort() == GetPort() && GetPort() != "NotConnected") {
            p->SetBackgroundColour(*wxRED);
            break;
        }
        else{
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

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

    p = propGrid->GetPropertyByName("Channels");
    if (p != nullptr) {
        if (_outputs.front()->GetMaxChannels() < GetChannels() || GetChannels() < 1) {
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }
}
#endif
#pragma endregion
