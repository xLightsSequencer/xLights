
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


#include "ControllerSerial.h"
#include "../models/OutputModelManager.h"
#include "UtilFunctions.h"
#include "../utils/SpecialOptions.h"
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

#include <cassert>
#include <format>

#include <log.h>

#ifdef SetPort
#undef SetPort  // Windows winspool.h defines SetPort as SetPortW
#endif

// Property choices moved to ui/controllerproperties/ControllerSerialPropertyAdapter

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

ControllerSerial::ControllerSerial(OutputManager* om, pugi::xml_node node, const std::string& showDir) : Controller(om, node, showDir) {
    _type = node.attribute("Protocol").as_string("");
    _serialOutput = dynamic_cast<SerialOutput*>(_outputs.front());
    _serialOutput->SetId(GetId());
    // Property choice initialization moved to ControllerSerialPropertyAdapter
    SetSpeed(node.attribute("Speed").as_int(0));
    SetPrefix(node.attribute("Prefix").as_string(""));
    SetPostfix(node.attribute("Postfix").as_string(""));
    SetFPPProxy(node.attribute("FPPProxy").as_string(""));
    VMVChanged();
    SetPort(node.attribute("Port").as_string(""));
    _dirty = false;
}

ControllerSerial::ControllerSerial(OutputManager* om) : Controller(om) {
    // Property choice initialization moved to ControllerSerialPropertyAdapter
    _name = om->UniqueName("Serial_");
    _serialOutput = new DMXOutput();
    _serialOutput->SetChannels(512);
    _serialOutput->SetId(GetId());
    _outputs.push_back(_serialOutput);
    _type = OUTPUT_DMX;
    SetPort(_outputManager->GetFirstUnusedCommPort());
    SetSpeed(_serialOutput->GetBaudRate());
#ifdef __APPLE__
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

pugi::xml_node ControllerSerial::Save(pugi::xml_node parent) {
    Output *o = _outputs.front();
    if (o != _serialOutput) {
        _outputs.pop_front();
        _outputs.push_back(_serialOutput);
    }

    pugi::xml_node um = Controller::Save(parent);

    um.append_attribute("Port") = _port;
    um.append_attribute("Speed") = _speed;
    um.append_attribute("Protocol") = _type;
    um.append_attribute("Prefix") = _saveablePrefix;
    um.append_attribute("Postfix") = _saveablePostfix;
    um.append_attribute("FPPProxy") = _fppProxy;

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
void ControllerSerial::VMVChanged() {
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
            if (_port.find(":") != std::string::npos) {
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
                if (port.find(":") != std::string::npos) {
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
        SetAutoSize(false, nullptr);
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
        SetAutoSize(false, nullptr);
    } else if (type == OUTPUT_GENERICSERIAL) {
        o = new GenericSerialOutput();
    } else {
        assert(false);
        spdlog::error("Could not create serial output of type {}.", type);
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
    res += " (" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";
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

void ControllerSerial::Convert(pugi::xml_node node, std::string showDir) {

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

    assert(_outputs.size() == 1);

    if (_outputs.size() > 0) {
        _port = _serialOutput->GetCommPort();
        _speed = _serialOutput->GetBaudRate();
        _type = _serialOutput->GetType();
        _id = _serialOutput->GetUniverse();
        if (GetProtocol() == OUTPUT_xxxSERIAL) {
            // xxx serial type - adapter handles protocol choices initialization
        }
    }
}

std::string ControllerSerial::GetChannelMapping(int32_t ch) const
{
    return std::format("Channel {} maps to ...\nType: {}\nName: {}\nComPort: {}\nChannel: {}\n{}",
        ch,
        GetProtocol(),
        GetName(),
        GetPort(),
        ch - GetStartChannel() + 1,
        (IsActive() ? "" : "INACTIVE\n"));
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

    return std::format("{},{},{},{},{},,{},{},\"{}\",{},{},{},{},{},{},{},,{}",
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
        (IsActive() ? "" : "DISABLED"),
        (IsSuppressDuplicateFrames() ? "SuppressDuplicates" : ""),
        (IsAutoSize() ? "AutoSize" : ""),
        (IsAutoLayout() ? "AutoLayout" : ""),
        (IsAutoUpload() ? "AutoUpload" : ""),
        GetFPPProxy()
    );
}
#pragma endregion

#pragma region UI
// UI property grid methods moved to ui/controllerproperties/ControllerSerialPropertyAdapter
#pragma endregion
