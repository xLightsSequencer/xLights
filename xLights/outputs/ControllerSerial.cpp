
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
#include "DDPOutput.h"
#include "DLightOutput.h"
#include "DMXOutput.h"
#include "OpenDMXOutput.h"
#include "OpenPixelNetOutput.h"
#include "RenardOutput.h"
#include "LOROutput.h"
#include "LOROptimisedOutput.h"
#include "PixelNetOutput.h"
#include "xxxSerialOutput.h"
#include "GenericSerialOutput.h"
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
        else if (it == "generic serial") types.Add(OUTPUT_GENERICSERIAL);
        else if (it == "xxx serial") {
            if (SpecialOptions::GetOption("xxx") == "true" || GetProtocol() == OUTPUT_xxxSERIAL) {
                types.Add(OUTPUT_xxxSERIAL);
            }
        } else if (it == "ddp-input") {
            for (const auto& it2 : caps->GetSerialProtocols()) {
                types.Add(it2);
            }
        }
    }
    return types;
}

void ControllerSerial::InitialiseTypes(bool forceXXX) {

    if (__types.GetCount() == 0) {
        __types.Add(OUTPUT_DMX);
        __types.Add(OUTPUT_LOR);
        __types.Add(OUTPUT_LOR_OPT);
        __types.Add(OUTPUT_OPENDMX);
        __types.Add(OUTPUT_PIXELNET);
        __types.Add(OUTPUT_OPENPIXELNET);
        __types.Add(OUTPUT_RENARD);
        __types.Add(OUTPUT_DLIGHT);
        __types.Add(OUTPUT_GENERICSERIAL);
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
std::vector<uint8_t> ControllerSerial::Encode(const std::string& s)
{
    std::vector<uint8_t> res;

    int state = 0;
    char c1 = ' ';

    for (auto c : s) {
        if (state == 0) {
            // normal state
            if (c == '\\') {
                state = 1;
            }
            else {
                res.push_back(c);
            }
        }
        else if (state == 1) {
            // last char was a backslash
            if (c == '\\') {
                res.push_back('\\');
                state = 0;
            }
            else if (c == 'x' || c == 'X') {
                state = 2;
            }
            else if (c == 't') {
                res.push_back('\t');
                state = 0;
            }
            else if (c == 'r') {
                res.push_back('\r');
                state = 0;
            }
            else if (c == 'n') {
                res.push_back('\n');
                state = 0;
            }
            else {
                state = 0;
            }
        }
        else if (state == 2) {
            // last two chars were \x
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
                c1 = c;
                state = 3;
            }
            else {
                state = 0;
            }
        }
        else if (state == 3) {
            // last three chars were \x9
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
                res.push_back(HexToChar(c1, c));
            }
            state = 0;
        }
    }

    return res;
}

ControllerSerial::ControllerSerial(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir) {
    _type = node->GetAttribute("Protocol");
    _serialOutput = dynamic_cast<SerialOutput*>(_outputs.front());
    _serialOutput->SetId(GetId());
    InitialiseTypes(_type == OUTPUT_xxxSERIAL);
    SetSpeed(wxAtoi(node->GetAttribute("Speed")));
    SetPrefix(node->GetAttribute("Prefix"));
    SetPostfix(node->GetAttribute("Postfix"));
    SetFPPProxy(node->GetAttribute("FPPProxy"));
    VMVChanged();
    SetPort(node->GetAttribute("Port"));
    _dirty = false;
}

ControllerSerial::ControllerSerial(OutputManager* om) : Controller(om) {
    InitialiseTypes(false);
    _name = om->UniqueName("Serial_");
    _serialOutput = new DMXOutput();
    _serialOutput->SetChannels(512);
    _serialOutput->SetId(GetId());
    _outputs.push_back(_serialOutput);
    _type = OUTPUT_DMX;
    SetPort(_outputManager->GetFirstUnusedCommPort());
    SetSpeed(_serialOutput->GetBaudRate());
#ifdef __WXOSX__
    _model = "FPP";
    _vendor = "FPP";
    VMVChanged();
#endif
}

ControllerSerial::ControllerSerial(OutputManager* om, const ControllerSerial& from) :
    Controller(om, from)
{
    SetSpeed(from._speed);
    SetPrefix(from._saveablePrefix);
    SetPostfix(from._saveablePostfix);
    SetFPPProxy(from._fppProxy);
    SetPort(from._port);
    _serialOutput = static_cast<SerialOutput*>(from._serialOutput->Copy());
}

wxXmlNode* ControllerSerial::Save() {
    Output *o = _outputs.front();
    if (o != _serialOutput) {
        _outputs.pop_front();
        _outputs.push_back(_serialOutput);
    }

    wxXmlNode* um = Controller::Save();

    um->AddAttribute("Port", _port);
    um->AddAttribute("Speed", wxString::Format("%d", _speed));
    um->AddAttribute("Protocol", _type);
    um->AddAttribute("Prefix", _saveablePrefix);
    um->AddAttribute("Postfix", _saveablePostfix);
    um->AddAttribute("FPPProxy", _fppProxy);

    if (o != _serialOutput) {
        _outputs.pop_front();
        _outputs.push_back(o);
    }

    return um;
}

bool ControllerSerial::UpdateFrom(Controller* from)
{
    bool changed = Controller::UpdateFrom(from);

    ControllerSerial* fromSerial = static_cast<ControllerSerial*>(from);

    if (_port != fromSerial->_port) {
        changed = true;
        _port = fromSerial->_port;
    }

    if (_speed != fromSerial->_speed) {
        changed = true;
        _speed = fromSerial->_speed;
    }
    if (_type != fromSerial->_type) {
        changed = true;
        _type = fromSerial->_type;
    }
    if (_saveablePrefix != fromSerial->_saveablePrefix) {
        changed = true;
        _saveablePrefix = fromSerial->_saveablePrefix;
    }
    if (_saveablePostfix != fromSerial->_saveablePostfix) {
        changed = true;
        _saveablePostfix = fromSerial->_saveablePostfix;
    }
    if (_fppProxy != fromSerial->_fppProxy) {
        changed = true;
        _fppProxy = fromSerial->_fppProxy;
    }

    if (_serialOutput->GetLongDescription() != fromSerial->_serialOutput->GetLongDescription())
    {
        changed = true;
        delete _serialOutput;
        _serialOutput = static_cast<SerialOutput*>(fromSerial->_serialOutput->Copy());
    }

    return changed;
}

Controller* ControllerSerial::Copy(OutputManager* om)
{
    return new ControllerSerial(om, *this);
}
#pragma endregion

#pragma region Getters and Setters
void ControllerSerial::SetFPPProxy(const std::string& proxy) {
    if (_fppProxy != proxy) {
        _fppProxy = proxy;
        _dirty = true;
        GetFirstOutput()->SetFPPProxyIP(_fppProxy);
    }
}
void ControllerSerial::VMVChanged(wxPropertyGrid *grid) {
    if (_model == "FPP") {
        if (GetFirstOutput()->GetType() != "DDP") {
            if (_serialOutput && GetFirstOutput() != _serialOutput) delete _serialOutput;
            _serialOutput = dynamic_cast<SerialOutput *>(_outputs.front());
            int sc = _serialOutput->GetStartChannel();
            int ch = _serialOutput->GetChannels();
            _outputs.pop_front();
            int i = 0;
            DDPOutput *out = new DDPOutput();
            if (sc >= 0) {
                out->SetTransientData(sc, i);
            }
            out->SetKeepChannelNumber(true);
            std::string ip = "";
            if (_port.find(":") != -1) {
                ip = _port.substr(0, _port.find(":"));
            }
            out->SetIP(ip, IsActive());
            out->SetFPPProxyIP(_fppProxy != "" ? _fppProxy : _outputManager->GetGlobalFPPProxy());
            out->SetChannels(ch);
            _outputs.push_back(out);
            _dirty = true;
        }
    } else {
        SetProtocol(_type);
    }
}


void ControllerSerial::SetPort(const std::string& port) {
    if (_serialOutput) {
        if (_port != port) {
            if (_model != "FPP") {
                _serialOutput->SetCommPort(port);
            } else {
                std::string ip = "";
                if (port.find(":") != -1) {
                    ip = port.substr(0, port.find(":"));
                    if (_serialOutput) {
                        _serialOutput->SetCommPort(port.substr(port.find(":") + 1));
                    }
                }
                GetFirstOutput()->SetIP(ip, IsActive());
            }
            _port = port;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetSpeed(int speed) {
    if (_serialOutput) {
        if (_speed != speed) {
            _serialOutput->SetBaudRate(speed);
            _speed = speed;
            _dirty = true;
        }
    }
}

void ControllerSerial::SetPrefix(const std::string& prefix)
{
    if (_serialOutput) {
        if (_saveablePrefix != prefix) {
            _saveablePrefix = prefix;
            _prefix = Encode(prefix);
            _serialOutput->SetPrefix(_prefix);
            _dirty = true;
        }
    }
}

void ControllerSerial::SetPostfix(const std::string& postfix)
{
    if (_serialOutput) {
        if (_saveablePostfix != postfix) {
            _saveablePostfix = postfix;
            _postfix = Encode(postfix);
            _serialOutput->SetPostfix(_postfix);
            _dirty = true;
        }
    }
}

void ControllerSerial::SetChannels(int channels) {
    if (_outputs.front() != nullptr) {
        if (_outputs.front()->GetChannels() != channels) {
            _outputs.front()->SetChannels(channels);
            if (_serialOutput) {
                _serialOutput->SetChannels(channels);
            }
            _dirty = true;
        }
    }
}

void ControllerSerial::SetProtocol(const std::string& type)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_type == type) {
        return;
    }

    auto const c = _serialOutput->GetChannels();
    _type = type;
    _dirty = true;

    SerialOutput* o = nullptr;
    if (type == OUTPUT_DLIGHT) {
        o = new DLightOutput();
    } else if (type == OUTPUT_DMX) {
        o = new DMXOutput();
    } else if (type == OUTPUT_LOR) {
        o = new LOROutput();
    } else if (type == OUTPUT_LOR_OPT) {
        o = new LOROptimisedOutput();
#ifndef EXCLUDENETWORKUI
        SetAutoSize(false, nullptr);
#endif
    } else if (type == OUTPUT_OPENDMX) {
        o = new OpenDMXOutput();
    } else if (type == OUTPUT_OPENPIXELNET) {
        o = new OpenPixelNetOutput();
    } else if (type == OUTPUT_RENARD) {
        o = new RenardOutput();
    } else if (type == OUTPUT_PIXELNET) {
        o = new PixelNetOutput();
    } else if (type == OUTPUT_xxxSERIAL) {
        o = new xxxSerialOutput();
    } else if (type == OUTPUT_GENERICSERIAL) {
        o = new GenericSerialOutput();
    } else {
        wxASSERT(false);
        logger_base.error("Could not create serial output of type %s.", (const char*)type.c_str());
    }

    if (o != nullptr) {
        o->SetCommPort(_port);
        o->SetBaudRate(_speed);
        _speed = o->GetBaudRate(); // because the old speed may have been overridden
        o->SetChannels(c);
        o->SetId(_id);

        if (_serialOutput == _outputs.front()) {
            _outputs.pop_front();
            _outputs.push_front(o);
        }
        delete _serialOutput;
        _serialOutput = o;
    }
}
#pragma endregion

#pragma region Virtual Functions
void ControllerSerial::SetId(int id) {
    Controller::SetId(id);
    if (_model == "FPP") {
        dynamic_cast<DDPOutput*>(GetFirstOutput())->SetId(id);
    }
    _serialOutput->SetId(id);
    _serialOutput->SetUniverse(id);
}

std::string ControllerSerial::GetFPPProxy() const {
    if (_fppProxy != "") return _fppProxy;
    return _outputManager->GetGlobalFPPProxy();
}

std::string ControllerSerial::GetLongDescription() const {
    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + " " + GetProtocol() + " " + GetPort();
    res += " (" + std::string(wxString::Format(wxT("%d"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string ControllerSerial::GetShortDescription() const {
	std::string res = "";

	if (!IsActive()) res += "INACTIVE ";
	res += GetName() + " " + GetProtocol() + " " + GetPort();
	if (!_description.empty()) {
		res += " ";
		res += _description;
	}

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
    } else {
        _serialOutput = dynamic_cast<SerialOutput*>(_outputs.back());
    }

    wxASSERT(_outputs.size() == 1);

    if (_outputs.size() > 0) {
        _port = _serialOutput->GetCommPort();
        _speed = _serialOutput->GetBaudRate();
        _type = _serialOutput->GetType();
        _id = _serialOutput->GetUniverse();
        if (GetProtocol() == OUTPUT_xxxSERIAL) {
            InitialiseTypes(true);
        }
    }
}

std::string ControllerSerial::GetChannelMapping(int32_t ch) const
{
    return wxString::Format("Channel %d maps to ...\nType: %s\nName: %s\nComPort: %s\nChannel: %d\n%s",
        ch, 
        GetProtocol(), 
        GetName(), 
        GetPort(), 
        ch - GetStartChannel() + 1,
        (IsActive() ? _("") : _("INACTIVE\n")));
}

Output::PINGSTATE ControllerSerial::Ping()
{
    if (_model == "FPP") {
        IPOutput* ipOutput = dynamic_cast<IPOutput*>(GetFirstOutput());
        if (ipOutput != nullptr) {
            _lastPingResult = ipOutput->Ping(ipOutput->GetResolvedIP(), _fppProxy == "" ? _outputManager->GetGlobalFPPProxy() : _fppProxy);
        }
        else {
            _lastPingResult = Output::PINGSTATE::PING_UNAVAILABLE;
        }
    }
    else {
        _lastPingResult = _serialOutput->Ping();
    }
    return GetLastPingState();
}

std::string ControllerSerial::GetExport() const {

    return wxString::Format("%s,%d,%d,%s,%s,,%s,%d,\"%s\",%d,%ld,%s,%s,%s,%s,%s,,%s",
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
        (IsAutoSize() ? _("AutoSize") : _("")),
        (IsAutoLayout() ? _("AutoLayout") : _("")),
        (IsAutoUpload() ? _("AutoUpload") : _("")),
        GetFPPProxy()
    );
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ControllerSerial::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties)
{
    Controller::AddProperties(propertyGrid, modelManager, expandProperties);
    wxPGProperty *p;

    ControllerCaps* caps = ControllerCaps::GetControllerConfig(this);
    if (caps != nullptr && caps->GetModel() == "FPP") {
        //FPP based serial devices
        std::string port = _port;
        std::string ip;
        if (_port.find(":") != -1) {
            ip = _port.substr(0, _port.find(":"));
            port = _port.substr(_port.find(":") + 1);
        }
        p = propertyGrid->Append(new wxStringProperty("IP Address", "IP", ip));
        p->SetHelpString("FPP IP address or host name.");

        p = propertyGrid->Append(new wxStringProperty("FPP Proxy IP/Hostname", "FPPProxy", _fppProxy));
        p->SetHelpString("This is typically the WIFI IP of a FPP instance that bridges two networks.");


        wxPGChoices ports;
        //FIXME - query FPP  for list of ports
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
        /*
         FIXME - coming soon
        ports.Add("i2c1");
        ports.Add("i2c2");
        ports.Add("spidev0.0");
        ports.Add("spidev0.1");
        ports.Add("spidev1.0");
        ports.Add("spidev1.1");

         */

        auto protocols = GetProtocols();
        propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, Controller::EncodeChoices(protocols, _type)));

        p = propertyGrid->Append(new wxEnumProperty("Port", "Port", ports, Controller::EncodeChoices(ports, port)));
        if (StartsWith(port, "tty")) {
            if (GetProtocol() != OUTPUT_DMX && GetProtocol() != OUTPUT_OPENDMX && GetProtocol() != OUTPUT_PIXELNET && GetProtocol() != OUTPUT_OPENPIXELNET) {
                wxPGChoices speeds;
                speeds.Add("9600");
                speeds.Add("19200");
                speeds.Add("38400");
                speeds.Add("57600");
                speeds.Add("115200");
                speeds.Add("230400");
                speeds.Add("460800");
                speeds.Add("921600");
                p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", speeds, Controller::EncodeChoices(speeds, wxString::Format("%d", _speed))));
            }
        } else if (StartsWith(port, "i2c")) {
            wxPGChoices i2cDevices;
            for (int x = 0; x < 128; x++) {
                char buf[12];
                sprintf(buf, "0x%02X", x);
                i2cDevices.Add(buf);
            }
            p = propertyGrid->Append(new wxEnumProperty("I2C Device", "I2CDevice", i2cDevices, Controller::EncodeChoices(i2cDevices, wxString::Format("0x%02X", _speed))));
        } else if (StartsWith(port, "spidev")) {
            p = propertyGrid->Append(new wxUIntProperty("Speed (kHz)", "SPISpeed", _speed));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 999999);
        }
    } else {
        p = propertyGrid->Append(new wxEnumProperty("Port", "Port", __ports, Controller::EncodeChoices(__ports, _port)));
        p->SetHelpString("This must be unique across all controllers.");

        auto protocols = GetProtocols();
        propertyGrid->Append(new wxEnumProperty("Protocol", "Protocol", protocols, Controller::EncodeChoices(protocols, _type)));

        p = propertyGrid->Append(new wxEnumProperty("Speed", "Speed", __speeds, Controller::EncodeChoices(__speeds, wxString::Format("%d", _speed))));
        if (dynamic_cast<SerialOutput*>(_outputs.front())) {
            if (!dynamic_cast<SerialOutput*>(_outputs.front())->AllowsBaudRateSetting()) {
                p->ChangeFlag(wxPG_PROP_READONLY, true);
                p->SetBackgroundColour(*wxLIGHT_GREY);
                p->SetHelpString("Speed is fixed for this protocol.");
            }
        }
    }

    if (GetProtocol() == "Generic Serial") {
        p = propertyGrid->Append(new wxStringProperty("Prefix", "Prefix", _saveablePrefix));
        p = propertyGrid->Append(new wxStringProperty("Postfix", "Postfix", _saveablePostfix));
    }
    if (_serialOutput && _serialOutput->GetType() != OUTPUT_LOR_OPT) {
        p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _outputs.front()->GetChannels()));
        p->SetAttribute("Min", 1);

        auto max = _serialOutput->GetMaxChannels();
        if (caps) {
            max = std::min(max, caps->GetMaxSerialPortChannels());
        }
        p->SetAttribute("Max", max);

        if (IsAutoSize()) {
            p->ChangeFlag(wxPG_PROP_READONLY, true);
            p->SetBackgroundColour(*wxLIGHT_GREY);
            p->SetHelpString("Channels cannot be changed when an output is set to Auto Size.");
        } else {
            p->SetEditor("SpinCtrl");
        }
    }

    p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1)));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->SetHelpString(modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), 4));

    _serialOutput->AddProperties(propertyGrid, p, this, true, expandProperties);
}

bool ControllerSerial::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "IP") {
        std::string ip = event.GetValue().GetString();
        if (_model == "FPP") {
            std::string port = _port;
            if (port.find(":") != -1) {
                port = port.substr(port.find(":") + 1);
            }
            ip = ip + ":" + port;
        }
        SetPort(ip);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Port", nullptr);
        return true;
    } else if (name == "FPPProxyIP") {
        std::string ip = event.GetValue().GetString();
        SetFPPProxy(ip);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Port", nullptr);
        return true;
    } else if (name == "Port") {
        std::string port = Controller::DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong());
        if (_model == "FPP") {
            if (_port.find(":") != -1) {
                port = _port.substr(0, _port.find(":") + 1) + port;
            }
        }
        SetPort(port);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Port");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Port", nullptr);
        return true;
    } else if (name == "I2CDevice") {
        SetSpeed(std::stoul(Controller::DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong()),  nullptr, 16));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Speed", nullptr);
        return true;
    } else if (name == "SPISpeed") {
        SetSpeed(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Speed", nullptr);
        return true;
    } else if (name == "Speed") {
        SetSpeed(wxAtoi(Controller::DecodeChoices(event.GetProperty()->GetChoices(), event.GetValue().GetLong())));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Speed");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Speed", nullptr);
        return true;
    } else if (name == "Prefix") {
        SetPrefix(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Prefix");
        return true;
    } else if (name == "Postfix") {
        SetPostfix(event.GetValue().GetString());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Postfix");
        return true;
    } else if (name == "Protocol") {
        auto protocols = GetProtocols();
        SetProtocol(Controller::DecodeChoices(protocols, event.GetValue().GetLong()));
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Protocol");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Protocol", nullptr);
        return true;
    } else if (name == "Channels") {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ControllerSerial::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerSerial::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    if (_serialOutput->HandlePropertyEvent(event, outputModelManager, this)) return true;

    return false;
}

void ControllerSerial::ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const {

    Controller::ValidateProperties(om, propGrid);

    for (const auto& it : om->GetControllers()) {
        auto s = dynamic_cast<ControllerSerial*>(it);

        // Port must be unique
        auto p = propGrid->GetPropertyByName("Port");
        if (p != nullptr) {
            if (s != nullptr && it->GetName() != GetName() && s->GetPort() == GetPort() && GetPort() != "NotConnected") {
                p->SetBackgroundColour(*wxRED);
                break;
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }

    auto p = propGrid->GetPropertyByName("Protocol");
    auto caps = ControllerCaps::GetControllerConfig(this);
    if (caps != nullptr && p != nullptr) {
        // controller must support the protocol
        if (!caps->IsValidInputProtocol(Lower(_type)) && !caps->IsValidSerialProtocol(Lower(_type))) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = propGrid->GetPropertyByName("Channels");
    if (p != nullptr) {
        if (_serialOutput->GetMaxChannels() < GetChannels() || GetChannels() < 1) {
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (_model == "FPP") {
        p = propGrid->GetPropertyByName("IP");
        if (p != nullptr) {
            if (_port.find(":") == std::string::npos) {
                p->SetBackgroundColour(*wxRED);
            } else {
                p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }
}
#endif
#pragma endregion
