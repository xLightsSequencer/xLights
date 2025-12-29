/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerConnection.h"
#include "Pixels.h"
#include "controllers/ControllerCaps.h"
#include "models/ModelManager.h"

static const int PORTS_PER_SMARTREMOTE = 4;

ControllerConnection::ControllerConnection(Model* model)
: _model(model)
{
}

ControllerConnection::~ControllerConnection()
{
}

void ControllerConnection::SetName(const std::string& controller)
{
    auto n = Trim(controller);
    if (n == _name) return;
    if (n == "xyzzy_kw") return;
    if (!n.empty() && n != USE_START_CHANNEL) {
        _name = n;
    }
    
    // if we are moving the model to no contoller then clear the start channel and model chain
    if (_name == NO_CONTROLLER) {
        _model->SetStartChannel("");
        _model->SetModelChain("");
        SetPort(0);
    }

    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetName");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetName");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetName");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetName");
    _model->AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "ControllerConnection::SetName");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetName");
    _model->IncrementChangeCount();
}

bool ControllerConnection::Rename(const std::string& oldName, const std::string& newName)
{
    wxASSERT(newName != "");

    bool changed = false;

    if (_name == oldName) {
        SetName(newName);
        changed = true;
    }
    if (StartsWith(_model->ModelStartChannel, "!" + oldName)) {
        _model->SetStartChannel("!" + newName + _model->ModelStartChannel.substr(oldName.size() + 1));
        changed = true;
    }
    if (_model->HasIndividualStartChannels()) {
        for (int i = 0; i < _model->GetParm1(); ++i) {
            auto str = _model->StartChanAttrName(i);
            auto sc = _model->GetIndividualStartChannel(i);
            if (StartsWith(sc, "!" + oldName)) {
                _model->SetIndividualStartChannel(i, "!" + newName + sc.substr(oldName.size() + 1));
                changed = true;
            }
        }
    }
    return changed;
}

int ControllerConnection::GetPort(int string) const
{
    // TODO:  Delete?  This looks like legacy attributes
    //wxString p = wxString::Format("%d", string);
    //if (GetControllerConnection()->HasAttribute(p)) {
    //    wxString s = GetControllerConnection()->GetAttribute(p);
    //    return wxAtoi(s);
    //}

    int port;
    int sr;
    GetPortSR(string, port, sr);
    return port;
}

// This is deliberately ! serial so that it defaults to thinking it is pixel
bool ControllerConnection::IsPixelProtocol() const
{
    return GetPort(1) != 0 && !::IsSerialProtocol(_protocol) && !::IsMatrixProtocol(_protocol) && !::IsPWMProtocol(_protocol);
}
bool ControllerConnection::IsSerialProtocol() const
{
    return GetPort(1) != 0 && ::IsSerialProtocol(_protocol);
}
bool ControllerConnection::IsMatrixProtocol() const
{
    return GetPort(1) != 0 && ::IsMatrixProtocol(_protocol);
}
bool ControllerConnection::IsLEDPanelMatrixProtocol() const
{
    return GetPort(1) != 0 && ::IsLEDPanelMatrixProtocol(_protocol);
}
bool ControllerConnection::IsVirtualMatrixProtocol() const
{
    return GetPort(1) != 0 && ::IsVirtualMatrixProtocol(_protocol);
}
bool ControllerConnection::IsPWMProtocol() const
{
    return GetPort(1) != 0 && ::IsPWMProtocol(_protocol);
}

bool ControllerConnection::IsValid() const
{
    return ((IsPixelProtocol() || IsSerialProtocol() || IsMatrixProtocol() || IsPWMProtocol()) && GetPort(1) > 0);
}

void ControllerConnection::SetProtocol(const std::string& protocol)
{
    if (protocol == _protocol) return;
    if (protocol == "xyzzy_kw") return;
    _protocol = protocol;
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetProtocol");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetProtocol");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetProtocol");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetProtocol");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetProtocol");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetSerialProtocolSpeed(int speed) {
    if (speed == _protocolSpeed) return;
    if (speed != 0) {
        _protocolSpeed = speed;
    }
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetSerialProtocolSpeed");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetPort(int port)
{
    if (port == _port) return;
    if (port == -999) return;

    if (port > 0) {
        _port = port;
    }
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetPort");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetPort");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetPort");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetPort");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetPort");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetBrightness(int brightness)
{
     if (brightness == _brightness) return;
    _brightness = brightness;
    _brightnessIsSet = true;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetBrightness");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetBrightness");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetBrightness");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetBrightness");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetStartNulls(int nulls)
{
    if (nulls == _startNulls ) return;
    _startNulls = nulls;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetStartNulls");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetStartNulls");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetStartNulls");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetStartNulls");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetEndNulls(int nulls)
{
    if (nulls == _endNulls ) return;
   _endNulls = nulls;
   _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetEndNulls");
   _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetEndNulls");
   _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetEndNulls");
   _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetEndNulls");
   _model->IncrementChangeCount();
}

void ControllerConnection::SetColorOrder(std::string const& color_order)
{
    if (color_order == _colorOrder) return;
    _colorOrder = color_order;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetColorOrder");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetColorOrder");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetColorOrder");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetColorOrder");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetGroupCount(int grouping)
{
    if (grouping == _groupCount) return;
    _groupCount = grouping;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetGroupCount");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetGroupCount");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetGroupCount");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetGroupCount");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetGamma(float gamma)
{
    if (abs(gamma - _gamma) < 0.01) return;
    _gamma = gamma;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetGamma");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetGamma");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetGamma");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetGamma");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetReverse(int reverse)
{
    if (_reverse == reverse) return;
    _reverse = reverse;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetReverse");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetReverse");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetReverse");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetReverse");
}

void ControllerConnection::SetZigZag(int zigzag)
{
    if (_zigzag == zigzag) return;
    _zigzag = zigzag;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetZigZag");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetZigZag");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetZigZag");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
}

void ControllerConnection::SetDMXChannel(int ch)
{
    if (_dmxChannel != ch) return;
    if (ch > 0) {
        _dmxChannel = ch;
    }
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnection::SetDMXChannel");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetDMXChannel");
    _model->IncrementChangeCount();
}

void ControllerConnection::ClearBrightness()
{
    if (_brightnessIsSet) {
        _brightnessIsSet = false;
        _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::ClearBrightness");
        _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::ClearBrightness");
        _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::ClearBrightness");
        _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::ClearBrightness");
        _model->IncrementChangeCount();
    }
}

void ControllerConnection::GetPortSR(int string, int& outport, int& outsr) const
{
    // we need to work with 0 based strings
    string = string - 1;

    int sr = GetSmartRemote();

    if (_port == 0 || string == 0) {
        outport = _port;
        outsr = sr;
    } else if (sr == 0) {
        outport = _port + string;
        outsr = 0;
    } else {
        bool cascadeOnPort = GetSRCascadeOnPort();
        int max = GetSRMaxCascade();

        if (cascadeOnPort) {
            outport = _port + string / max;
            outsr = sr + (string % max);
        } else {
            int currp = _port;
            int currsr = sr;

            for (int p = 0; p < string; ++p) {
                int newp = currp + 1;
                if ((newp - 1) / PORTS_PER_SMARTREMOTE != (currp - 1) / PORTS_PER_SMARTREMOTE) {
                    int newsr = currsr + 1;
                    if (newsr - sr >= max) {
                        currsr = sr;
                        currp = newp;
                    } else {
                        currsr = newsr;
                        currp = ((currp - 1) / PORTS_PER_SMARTREMOTE) * PORTS_PER_SMARTREMOTE + 1;
                    }
                } else {
                    currp = newp;
                }
            }

            outport = currp;
            outsr = currsr;
        }
    }
}

// string is one based
int ControllerConnection::GetSmartRemoteForString(int string) const
{
    int port;
    int sr;
    GetPortSR(string, port, sr);
    return sr;
}

char ControllerConnection::GetSmartRemoteLetter() const
{
    if (_smartRemote == 0)
        return ' ';
    return char('A' + _smartRemote - 1);
}

char ControllerConnection::GetSmartRemoteLetterForString(int string) const
{
    auto sr = GetSmartRemoteForString(string);
    if (sr == 0)
        return ' ';
    return char('A' + sr - 1);
}

// This sorts the special A->B->C and B->C first to ensure that anything on a particular smart remote comes after things that span multiple ports
int ControllerConnection::GetSortableSmartRemote() const
{
    int sr = GetSmartRemote();
    int max = GetSRMaxCascade();
    if (max == 1)
        return sr + 200;
    return sr;
}

void ControllerConnection::SetSRCascadeOnPort(bool cascade)
{
    if (_smartRemoteCascadeOnPort != cascade) return;
    _smartRemoteCascadeOnPort = cascade;
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetSRCascadeOnPort");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetSRCascadeOnPort");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetSRCascadeOnPort");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetSRCascadeOnPort");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnection::SetSRCascadeOnPort");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetSRCascadeOnPort");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetSRMaxCascade(int max)
{
    if (_smartRemoteMaxCascade != max) return;
    _smartRemoteMaxCascade = max;
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetSRMaxCascade");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetSRMaxCascade");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetSRMaxCascade");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetSRMaxCascade");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetSRMaxCascade");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetSmartRemote(int sr)
{
    if (_smartRemote != sr) return;
    // Find the last model on this smart remote
    if (!_name.empty()) {
        _model->SetModelChain(_model->modelManager.GetLastModelOnPort(_name, _port, _model->GetName(), _protocol, sr));
    }
    if (sr != 0) {
        _smartRemote = sr;
    } else {
        SetSRMaxCascade(1);
        SetSRCascadeOnPort(false);
    }

    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetSmartRemote");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetSmartRemote");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetSmartRemote");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetSmartRemote");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetSmartRemote");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetSmartRemote");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetSmartRemoteType(const std::string& type)
{
    if (_smartRemoteType != type) return;
    if (!type.empty()) {
        _smartRemoteType = type;
    }
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ControllerConnection::SetSmartRemoteType");
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ControllerConnection::SetSmartRemoteType");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerConnection::SetSmartRemoteType");
    _model->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerConnection::SetSmartRemoteType");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ControllerConnection::SetSmartRemoteType");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ControllerConnection::SetSmartRemoteType");
    _model->IncrementChangeCount();
}

void ControllerConnection::SetSmartRemoteTs(int ts)
{
    if (_smartRemoteTs != ts) return;
    _smartRemoteTs = ts;
    _model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
    _model->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
    _model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
    _model->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
    _model->IncrementChangeCount();
}

std::vector<std::string> ControllerConnection::GetSmartRemoteTypes() const
{
    auto caps = _model->GetControllerCaps();
    if (caps == nullptr) {
        return { "" };
    }
    return caps->GetSmartRemoteTypes();
}

std::string ControllerConnection::GetSmartRemoteType() const
{
    auto types = GetSmartRemoteTypes();
    if (types.empty()) {
        return "";
    }
    std::string t = GetSmartRemoteTypes().front();
    std::string s = _smartRemoteType;

    if (std::find(types.begin(), types.end(), s) == types.end()) {
        return t;
    }
    return s;
}

int ControllerConnection::GetSmartRemoteTypeIndex(const std::string& srType) const
{
    auto caps = _model->GetControllerCaps();
    int i = 0;
    if (caps != nullptr) {
        for (const auto& it : caps->GetSmartRemoteTypes()) {
            if (srType == Lower(it)) {
                return i;
            }
            i++;
        }
    }

    return 0;
}

std::string ControllerConnection::GetSmartRemoteTypeName(int idx) const
{
    auto caps = _model->GetControllerCaps();
    if (caps != nullptr) {
        const auto srList = caps->GetSmartRemoteTypes();
        if (idx < srList.size() && idx >= 0) {
            auto it = srList.begin();
            std::advance(it, idx);
            return *it;
        }
    }

    return std::string();
}

int ControllerConnection::GetSmartRemoteCount() const
{
    auto caps = _model->GetControllerCaps();
    if (caps != nullptr) {
        return caps->GetSmartRemoteCount();
    }
    return 3;
}
