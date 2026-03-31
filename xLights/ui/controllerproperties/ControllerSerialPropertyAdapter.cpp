
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerSerialPropertyAdapter.h"
#include "OutputPropertyAdapters.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "outputs/Controller.h"
#include "outputs/ControllerSerial.h"
#include "outputs/OutputManager.h"
#include "outputs/SerialOutput.h"
#include "outputs/DDPOutput.h"
#include "OutputModelManager.h"
#include "controllers/ControllerCaps.h"
#include "UtilFunctions.h"
#include "SpecialOptions.h"
#include "models/ModelManager.h"

static wxPGChoices __serialTypes;
static wxPGChoices __serialPorts;
static wxPGChoices __serialSpeeds;

static void InitialiseSerialTypes(bool forceXXX) {
    if (__serialTypes.GetCount() == 0) {
        __serialTypes.Add(OUTPUT_DMX);
        __serialTypes.Add(OUTPUT_LOR);
        __serialTypes.Add(OUTPUT_LOR_OPT);
        __serialTypes.Add(OUTPUT_OPENDMX);
        __serialTypes.Add(OUTPUT_PIXELNET);
        __serialTypes.Add(OUTPUT_OPENPIXELNET);
        __serialTypes.Add(OUTPUT_RENARD);
        __serialTypes.Add(OUTPUT_DLIGHT);
        __serialTypes.Add(OUTPUT_GENERICSERIAL);
        if (forceXXX || SpecialOptions::GetOption("xxx") == "true") {
            __serialTypes.Add(OUTPUT_xxxSERIAL);
        }
    } else if (forceXXX) {
        bool found = false;
        for (size_t i = 0; i < __serialTypes.GetCount(); i++) {
            if (__serialTypes.GetLabel(i) == OUTPUT_xxxSERIAL) { found = true; break; }
        }
        if (!found) __serialTypes.Add(OUTPUT_xxxSERIAL);
    }
    if (__serialSpeeds.GetCount() == 0) {
        for (const auto& it : SerialOutput::GetPossibleBaudRates()) __serialSpeeds.Add(it);
    }
    if (__serialPorts.GetCount() == 0) {
        for (const auto& it : SerialOutput::GetPossibleSerialPorts()) __serialPorts.Add(it);
    }
}

static wxPGChoices GetSerialProtocols(const ControllerSerial& serial) {
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(&serial);
    if (caps == nullptr) {
        InitialiseSerialTypes(serial.GetProtocol() == OUTPUT_xxxSERIAL);
        return __serialTypes;
    }
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
        else if (it == "generic serial") types.Add(OUTPUT_GENERICSERIAL);
        else if (it == "xxx serial") {
            if (SpecialOptions::GetOption("xxx") == "true" || serial.GetProtocol() == OUTPUT_xxxSERIAL)
                types.Add(OUTPUT_xxxSERIAL);
        } else if (it == "ddp-input") {
            for (const auto& it2 : caps->GetSerialProtocols()) types.Add(it2);
        }
    }
    return types;
}

ControllerSerialPropertyAdapter::ControllerSerialPropertyAdapter(Controller& controller)
    : ControllerPropertyAdapter(controller)
    , _serial(static_cast<ControllerSerial&>(controller)) {
}

void ControllerSerialPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {
    ControllerPropertyAdapter::AddProperties(propertyGrid, modelManager, expandProperties);
    wxPGProperty* p;

    ControllerCaps* caps = ControllerCaps::GetControllerConfig(&_serial);
    auto serialOutput = _serial.GetSerialOutput();
    auto port = _serial.GetPort();
    auto protocol = _serial.GetProtocol();
    auto speed = _serial.GetSpeed();

    if (caps != nullptr && caps->GetModel() == "FPP") {
        //FPP based serial devices
        std::string portStr = port;
        std::string ip;
        if (port.find(":") != std::string::npos) {
            ip = port.substr(0, port.find(":"));
            portStr = port.substr(port.find(":") + 1);
        }
        p = propertyGrid->Append(new wxStringProperty("IP Address", "IP", ip));
        p->SetHelpString("FPP IP address or host name.");

        p = propertyGrid->Append(new wxStringProperty("FPP Proxy IP/Hostname", "FPPProxy", _serial.GetControllerFPPProxy()));
        p->SetHelpString("This is typically the WIFI IP of a FPP instance that bridges two networks.");

        wxPGChoices ports;
        for (int x = 0; x < 6; x++) {
            ports.Add("ttyS" + std::to_string(x));
        }
        for (int x = 0; x < 6; x++) {
            ports.Add("ttyUSB" + std::to_string(x));
        }
        for (int x = 0; x < 6; x++) {
            ports.Add("ttyACM" + std::to_string(x));
        }
        ports.Add("ttyAMA0");

        auto protocols = GetSerialProtocols(_serial);
        propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, EncodeChoices(protocols, protocol)));

        p = propertyGrid->Append(new wxEnumProperty("Port", "Port", ports, EncodeChoices(ports, portStr)));
        if (StartsWith(portStr, "tty")) {
            if (protocol != OUTPUT_DMX && protocol != OUTPUT_OPENDMX && protocol != OUTPUT_PIXELNET && protocol != OUTPUT_OPENPIXELNET) {
                wxPGChoices speeds;
                speeds.Add("9600");
                speeds.Add("19200");
                speeds.Add("38400");
                speeds.Add("57600");
                speeds.Add("115200");
                speeds.Add("230400");
                speeds.Add("460800");
                speeds.Add("921600");
                p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", speeds, EncodeChoices(speeds, wxString::Format("%d", speed))));
            }
        } else if (StartsWith(portStr, "i2c")) {
            wxPGChoices i2cDevices;
            for (int x = 0; x < 128; x++) {
                char buf[12];
                snprintf(buf, sizeof(buf), "0x%02X", x);
                i2cDevices.Add(buf);
            }
            p = propertyGrid->Append(new wxEnumProperty("I2C Device", "I2CDevice", i2cDevices, EncodeChoices(i2cDevices, wxString::Format("0x%02X", speed))));
        } else if (StartsWith(portStr, "spidev")) {
            p = propertyGrid->Append(new wxUIntProperty("Speed (kHz)", "SPISpeed", speed));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 999999);
        }
    } else {
        auto portChoices = __serialPorts;
        p = propertyGrid->Append(new wxEnumProperty("Port", "Port", portChoices, EncodeChoices(portChoices, port)));
        p->SetHelpString("This must be unique across all controllers.");

        auto protocols = GetSerialProtocols(_serial);
        propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, EncodeChoices(protocols, protocol)));

        auto speedChoices = __serialSpeeds;
        p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", speedChoices, EncodeChoices(speedChoices, wxString::Format("%d", speed))));
        if (serialOutput) {
            if (!serialOutput->AllowsBaudRateSetting()) {
                p->ChangeFlag(wxPGFlags::ReadOnly, true);
                p->SetBackgroundColour(*wxLIGHT_GREY);
                p->SetHelpString("Speed is fixed for this protocol.");
            }
        }
    }

    if (protocol == "Generic Serial") {
        p = propertyGrid->Append(new wxStringProperty("Prefix", "Prefix", _serial.GetSaveablePreFix()));
        p = propertyGrid->Append(new wxStringProperty("Postfix", "Postfix", _serial.GetSaveablePostFix()));
    }
    if (serialOutput && serialOutput->GetType() != OUTPUT_LOR_OPT) {
        p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _serial.GetFirstOutput()->GetChannels()));
        p->SetAttribute("Min", 1);

        auto max = serialOutput->GetMaxChannels();
        if (caps) {
            max = std::min(max, caps->GetMaxSerialPortChannels());
        }
        p->SetAttribute("Max", max);

        if (_serial.IsAutoSize()) {
            p->ChangeFlag(wxPGFlags::ReadOnly, true);
            p->SetBackgroundColour(*wxLIGHT_GREY);
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
        }
    }

    p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(_serial.GetStartChannel(), _serial.GetEndChannel(), -1)));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->SetHelpString(modelManager->GetModelsOnChannels(_serial.GetStartChannel(), _serial.GetEndChannel(), 4));

    if (serialOutput) {
        auto adapter = OutputPropertyAdapter::Create(*serialOutput);
        adapter->AddProperties(propertyGrid, p, &_serial, true, expandProperties);
    }
}

bool ControllerSerialPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (ControllerPropertyAdapter::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "IP") {
        std::string ip = event.GetValue().GetString();
        if (_serial.GetModel() == "FPP") {
            std::string port = _serial.GetPort();
            if (port.find(":") != std::string::npos) {
                port = port.substr(port.find(":") + 1);
            }
            ip = ip + ":" + port;
        }
        _serial.SetPort(ip);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        return true;
    } else if (name == "FPPProxyIP") {
        std::string ip = event.GetValue().GetString();
        _serial.SetFPPProxy(ip);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        return true;
    } else if (name == "Port") {
        std::string port = DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong());
        if (_serial.GetModel() == "FPP") {
            auto currentPort = _serial.GetPort();
            if (currentPort.find(":") != std::string::npos) {
                port = currentPort.substr(0, currentPort.find(":") + 1) + port;
            }
        }
        _serial.SetPort(port);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        return true;
    } else if (name == "I2CDevice") {
        _serial.SetSpeed(std::strtoul(DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong()).c_str(), nullptr, 16));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        return true;
    } else if (name == "SPISpeed") {
        _serial.SetSpeed(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        return true;
    } else if (name == "Speed") {
        _serial.SetSpeed(wxAtoi(DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong())));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_SETTING_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        return true;
    } else if (name == "Prefix") {
        _serial.SetPrefix(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Prefix");
        return true;
    } else if (name == "Postfix") {
        _serial.SetPostfix(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Postfix");
        return true;
    } else if (name == "Protocol") {

        auto const& outputs = _serial.GetOutputs();
        if (outputs.size() > 0) {
            wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
            auto adapter = OutputPropertyAdapter::Create(*outputs.front());
            adapter->RemoveProperties(grid);
        }

        auto protocols = GetSerialProtocols(_serial);
        _serial.SetProtocol(DecodeChoices(protocols, event.GetValue().GetLong()));

        if (outputs.size() > 0) {
            wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());
            std::list<wxPGProperty*> expandProperties;
            auto before = grid->GetProperty("Models");
            auto adapter = OutputPropertyAdapter::Create(*outputs.front());
            adapter->AddProperties(grid, before, &_serial, true, expandProperties);
        }

        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerSerial::HandlePropertyEvent::Protocol");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        return true;
    } else if (name == "Channels") {
        _serial.SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CONFIG_CHANGE, "ControllerSerial::HandlePropertyEvent::Channels");
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    auto serialOutput = _serial.GetSerialOutput();
    if (serialOutput) {
        auto adapter = OutputPropertyAdapter::Create(*serialOutput);
        if (adapter->HandlePropertyEvent(event, outputModelManager, &_serial)) return true;
    }

    return false;
}

void ControllerSerialPropertyAdapter::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    ControllerPropertyAdapter::ValidateProperties(om, propGrid);

    for (const auto& it : om->GetControllers()) {
        auto s = dynamic_cast<ControllerSerial*>(it);

        // Port must be unique
        auto p = propGrid->GetPropertyByName("Port");
        if (p != nullptr) {
            if (s != nullptr && it->GetName() != _serial.GetName() && s->GetPort() == _serial.GetPort() && _serial.GetPort() != "NotConnected") {
                p->SetBackgroundColour(*wxRED);
                break;
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    auto protocol = _serial.GetProtocol();
    auto p = propGrid->GetPropertyByName("Protocol");
    auto caps = ControllerCaps::GetControllerConfig(&_serial);
    if (caps != nullptr && p != nullptr) {
        // controller must support the protocol
        if (!caps->IsValidInputProtocol(Lower(protocol)) && !caps->IsValidSerialProtocol(Lower(protocol))) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    auto serialOutput = _serial.GetSerialOutput();
    if (p != nullptr && serialOutput) {
        if (serialOutput->GetMaxChannels() < _serial.GetChannels() || _serial.GetChannels() < 1) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (_serial.GetModel() == "FPP") {
        p = propGrid->GetPropertyByName("IP");
        if (p != nullptr) {
            if (_serial.GetPort().find(":") == std::string::npos) {
                p->SetBackgroundColour(*wxRED);
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }
}
