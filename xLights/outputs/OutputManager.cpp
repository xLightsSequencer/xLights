
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
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/filename.h>

#include "OutputManager.h"
#include "ControllerEthernet.h"
#include "ControllerNull.h"
#include "ControllerSerial.h"
#include "SerialOutput.h"
#include "E131Output.h"
#include "ZCPPOutput.h"
#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "xxxEthernetOutput.h"
#include "OPCOutput.h"
#include "TestPreset.h"
#include "../osxMacUtils.h"
#include "../Parallel.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

#pragma region Static Variables
int OutputManager::_lastSecond = -10;
int OutputManager::_currentSecond = -10;
int OutputManager::_lastSecondCount = 0;
int OutputManager::_currentSecondCount = 0;
bool OutputManager::__isSync = false;
bool OutputManager::_isRetryOpen = false;
bool OutputManager::_isInteractive = true;
#pragma endregion

#pragma region Private Functions
bool OutputManager::SetGlobalOutputtingFlag(bool state, bool force) {

    if (state != _outputting && !force) return false;

    wxConfig* xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr) {
        xlconfig->Write(_("OutputActive"), state);
        delete xlconfig;
        return true;
    }
    return false;
}

bool OutputManager::ConvertStartChannel(const std::string sc, std::string& newsc) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool changed = false;

    // if it is of form <number>:<number> then we need to fix it
    auto parts = wxSplit(sc, ':');
    if (parts.size() == 2 && parts[0].size() > 0) {
        if (isdigit(parts[0][0])) {
            int on = wxAtoi(parts[0]);
            int scc = wxAtoi(parts[1]);

            if (on > 0) {
                auto it = _conversionOutputs.begin();
                std::advance(it, on - 1);
                if (it != _conversionOutputs.end()) {
                    if (it->first->GetType() == OUTPUT_E131 || (it->first->GetType() == OUTPUT_ARTNET)) {
                        // convert to #ip:univ:sc or just #univ:sc if multicast
                        auto ipo = dynamic_cast<IPOutput*>(it->first);

                        if (ipo->GetIP() == "MULTICAST") {
                            newsc = wxString::Format("#%d:%d", it->first->GetUniverse(), scc);
                            changed = true;
                            logger_base.debug("Networks conversion MULTICAST %s converted start channel '%s' to '%s'.", (const char*)it->first->GetType().c_str(), (const char*)sc.c_str(), (const char*)newsc.c_str());
                        }
                        else {
                            newsc = wxString::Format("#%s:%d:%d", it->first->GetIP(), it->first->GetUniverse(), scc);
                            changed = true;
                            logger_base.debug("Networks conversion %s converted start channel '%s' to '%s'.", (const char*)it->first->GetType().c_str(), (const char*)sc.c_str(), (const char*)newsc.c_str());
                        }
                    }
                    else {
                        // these convert to a controller 1:1 so just use the referenced controller

                        // convert to !name:sc if description isnt blank
                        newsc = wxString::Format("!%s:%d", it->second->GetName(), scc);
                        changed = true;
                        logger_base.debug("Networks conversion %s converted start channel '%s' to '%s'.", (const char*)it->first->GetType().c_str(), (const char*)sc.c_str(), (const char*)newsc.c_str());
                    }
                }
            }
        }
        else if (parts[0][0] == '!') {
            // output name may need to be updated
            auto on = parts[0].substr(1);
            int scc = wxAtoi(parts[1]);

            for (const auto& it : _conversionOutputs) {
                if (it.first->GetDescription_CONVERT() == on) {

                    // find the first channel offset within this controller of this output
                    int nsc = 0;
                    for (const auto& it2 : it.second->GetOutputs())                         {
                        if (it2->GetDescription_CONVERT() == on) break;
                        nsc += it2->GetChannels();
                    }

                    newsc = wxString::Format("!%s:%d", it.second->GetName(), nsc + scc);
                    changed = true;
                    logger_base.debug("Networks conversion %s converted start channel '%s' to '%s'.", (const char*)it.first->GetType().c_str(), (const char*)sc.c_str(), (const char*)newsc.c_str());
                    break;
                }
            }
        }
    }
    return changed;
}

void OutputManager::AsyncPingAll() {

    std::for_each(begin(_controllers), end(_controllers), [](Controller* c) { c->AsyncPing(); });
}
#pragma endregion

#pragma region Constructors and Destructors
OutputManager::OutputManager() {

    _dirty = false;
}

OutputManager::~OutputManager()
{
    if (_outputting) {
        StopOutput();
    }

    // destroy all out output objects
    DeleteAllControllers();

    for (auto&& tp : _testPresets) {
        delete tp;
    }
}
#pragma endregion 

#pragma region Save and Load
bool OutputManager::Load(const std::string& showdir, bool syncEnabled) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // Remove any existing outputs
    DeleteAllControllers();

    wxFileName fn(showdir + "/" + GetNetworksFileName());
    _filename = fn.GetFullPath();

    wxXmlDocument doc;
    doc.Load(fn.GetFullPath());

    Controller* cu = nullptr;
    std::string lasttype = "";
    std::string lastport = "";
    int lastuniv = -1;

    if (doc.IsOk()) {
        _globalFPPProxy = doc.GetRoot()->GetAttribute("GlobalFPPProxy");

        std::map<std::string, bool> multiip;
        for (auto e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
            if (e->GetName() == "network")
            {
                auto port = e->GetAttribute("ComPort");
                auto univ = wxAtoi(e->GetAttribute("BaudRate", "-1"));

                if (multiip.find(port) == multiip.end()) {
                    multiip[port] = false;
                }
                else {
                    if (port != lastport || univ != lastuniv + 1) {
                        multiip[port] = true;
                    }
                }
                lastport = port;
                lastuniv = univ;
            }
        }

        lastuniv = -1;
        lastport = "";
        for (auto e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext()) {
            if (e->GetName() == "network") {
                Output* conversionOutput = Output::Create(nullptr, e, showdir);

                auto type = e->GetAttribute("NetworkType");
                auto port = e->GetAttribute("ComPort");
                int univ = wxAtoi(e->GetAttribute("BaudRate", "-1"));
                int univcount = wxAtoi(e->GetAttribute("NumUniverses", "1"));
                if (type == OUTPUT_xxxETHERNET) {
                    univ = wxAtoi(e->GetAttribute("Port", "1"));
                }
                bool dups = multiip[port];

                if (type != lasttype ||
                    lastport != port ||
                    (lastuniv != -1 && univ != -1 && univ != lastuniv + 1)) {
                    // trigger the creation of a new controller
                    cu = nullptr;
                }

                if (cu == nullptr) {
                    if (type == OUTPUT_NULL) {
                        cu = new ControllerNull(this);
                    }
                    else if (type == OUTPUT_ZCPP) {
                        cu = new ControllerEthernet(this);
                    }
                    else if (type == OUTPUT_DDP) {
                        cu = new ControllerEthernet(this);
                    }
                    else if (type == OUTPUT_E131) {
                        cu = new ControllerEthernet(this, dups);
                    }
                    else if (type == OUTPUT_ARTNET) {
                        cu = new ControllerEthernet(this, dups);
                    }
                    else if (type == OUTPUT_xxxETHERNET) {
                        cu = new ControllerEthernet(this, dups);
                    }
                    else if (type == OUTPUT_DMX ||
                        type == OUTPUT_xxxSERIAL ||
                        type == OUTPUT_PIXELNET ||
                        type == OUTPUT_LOR ||
                        type == OUTPUT_LOR_OPT ||
                        type == OUTPUT_DLIGHT ||
                        type == OUTPUT_RENARD ||
                        type == OUTPUT_OPENDMX ||
                        type == OUTPUT_OPENPIXELNET) {
                        cu = new ControllerSerial(this);
                    }
                    AddController(cu, -1);
                    cu->DeleteAllOutputs();
                    cu->SetActive(conversionOutput->IsEnabled() ? "Active" : "Inactive");
                }
                cu->Convert(e, showdir);

                // keep the converted output connected to the controller
                _conversionOutputs.push_back({ conversionOutput, cu });

                lasttype = type;
                lastport = port;
                lastuniv = univ + univcount - 1;
                _didConvert = true;
            }
            else if (e->GetName() == "e131sync") {
                _syncUniverse = wxAtoi(e->GetAttribute("universe"));
            }
            else if (e->GetName() == "suppressframes") {
                _suppressFrames = wxAtoi(e->GetAttribute("frames"));
            }
            else if (e->GetName() == "testpreset") {
                logger_base.debug("Loading test presets.");
                TestPreset* tp = new TestPreset(e);

                bool exists = false;
                for (const auto& it : _testPresets) {
                    if (it->GetName() == tp->GetName()) {
                        exists = true;
                        break;
                    }
                }

                if (exists) {
                    // dont load this preset ... it is a duplicate
                    delete tp;
                }
                else {
                    _testPresets.push_back(tp);
                }
            }
            else if (e->GetName() == "Controller") {
                _controllers.push_back(Controller::Create(this, e, showdir));
                // remove any that were not created ... usually as this version does not have that controller supported
                if (_controllers.back() == nullptr) _controllers.pop_back();
            }
        }
    }
    else {
        logger_base.warn("Error loading networks file: %s.", (const char*)fn.GetFullPath().c_str());
        return false;
    }

    for (const auto& it : _controllers)         {
        it->SetGlobalFPPProxy(_globalFPPProxy);
    }

    logger_base.debug("Networks loaded.");

    AsyncPingAll();
    UpdateUnmanaged();

    SomethingChanged();

    return true;
}

bool OutputManager::Save() {

    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "Networks");

    root->AddAttribute("computer", wxGetHostName());
    root->AddAttribute("GlobalFPPProxy", _globalFPPProxy);

    doc.SetRoot(root);

    if (_syncUniverse != 0) {
        wxXmlNode* newNode = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "e131sync");
        newNode->AddAttribute("universe", wxString::Format("%d", _syncUniverse));
        root->AddChild(newNode);
    }

    if (_suppressFrames != 0) {
        wxXmlNode* newNode = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "suppressframes");
        newNode->AddAttribute("frames", wxString::Format("%d", _suppressFrames));
        root->AddChild(newNode);
    }

    for (const auto& it : _controllers) {
        root->AddChild(it->Save());
    }

    for (const auto& it : _testPresets) {
        root->AddChild(it->Save());
    }

    if (doc.Save(_filename)) {
        _dirty = false;
    }

    return (_dirty == false);
}

// This function is used purely to locate and fix any outputnumber:startchannel model start channels when the networks file is first converted
bool OutputManager::ConvertModelStartChannels(wxXmlNode* modelsNode) const {

    // check if any conversion has been done
    if (_conversionOutputs.size() == 0) return false;

    bool changed = false;

    for (wxXmlNode* model = modelsNode->GetChildren(); model != nullptr; model = model->GetNext()) {
        if (model->GetName() == "model") {

            // Do main model start channel
            std::string sc = UnXmlSafe(model->GetAttribute("StartChannel").ToStdString());
            std::string newsc;
            if (ConvertStartChannel(sc, newsc)) {
                model->DeleteAttribute("StartChannel");
                model->AddAttribute("StartChannel", XmlSafe(newsc));
                changed = true;
            }

            // Do any string start channels
            if (model->GetAttribute("Advanced", "0") == "1") {
                int strings = wxAtoi(model->GetAttribute("parm1", "0"));
                for (int i = 1; i <= strings; i++) {
                    auto s = wxString::Format("String%d", i);
                    std::string sc = UnXmlSafe(model->GetAttribute(s).ToStdString());
                    if (ConvertStartChannel(sc, newsc)) {
                        model->DeleteAttribute(s);
                        model->AddAttribute(s, XmlSafe(newsc));
                        changed = true;
                    }
                }
            }
        }
    }
    return changed;
}
#pragma endregion

#pragma region Static Functions
void OutputManager::RegisterSentPacket() {

    int second = wxGetLocalTime() % 60;

    if (second == _currentSecond) {
        _currentSecondCount++;
    }
    else {
        if (second == _currentSecond + 1 || (second == 0 && _currentSecond == 59)) {
            _lastSecond = _currentSecond;
            _lastSecondCount = _currentSecondCount;
        }
        else {
            _lastSecond = second - 1;
            if (_lastSecond < 0) _lastSecond = 59;
            _lastSecondCount = 0;
        }
        _currentSecond = second;
        _currentSecondCount = 1;
    }
}

std::string OutputManager::GetExportHeader() {

    return "Controller Name,Start Absolute,End Absolute,Type,Protocol,IP,Comm Port,Baud Rate,Description,Universe/Id,Channels,Active,Suppress Duplicates,Auto Size,FPP Proxy";
}
#pragma endregion

#pragma region Controller Management
std::list<ControllerEthernet*> OutputManager::GetControllers(const std::string& ip, const std::string hostname) {

    std::list<ControllerEthernet*> res;

    for (const auto& it : _controllers) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr) {
            if (eth->GetIP() == ip || eth->GetResolvedIP() == ip || eth->GetIP() == hostname) {
                res.push_back(eth);
            }
        }
    }
    return res;
}

std::list<ControllerEthernet*> OutputManager::GetControllers(const std::string& ip) {

    std::list<ControllerEthernet*> res;

    for (const auto it : _controllers) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr) {
            if (eth->GetIP() == ip || eth->GetResolvedIP() == ip) {
                res.push_back(eth);
            }
        }
    }
    return res;
}

void OutputManager::AddController(Controller* controller, int pos)
{
    // Make sure global FPP proxy has been set
    controller->SetGlobalFPPProxy(_globalFPPProxy);

    if (pos < 0 || pos > _controllers.size()) {
        _controllers.push_back(controller);
    }
    else {
        auto it = _controllers.begin();
        std::advance(it, pos);
        _controllers.insert(it, controller);
    }
    UpdateUnmanaged();
}

void OutputManager::DeleteController(const std::string& controllerName) {

    for (auto it = begin(_controllers); it != end(_controllers); ++it) {
        if ((*it)->GetName() == controllerName) {
            delete* it;
            _controllers.erase(it);
            break;
        }
    }
    UpdateUnmanaged();
}

void OutputManager::DeleteAllControllers() {

    while (_controllers.size() > 0) {
        delete _controllers.front();
        _controllers.pop_front();
    }

    while (_conversionOutputs.size() > 0) {
        delete _conversionOutputs.front().first;
        _conversionOutputs.pop_front();
    }
}

void OutputManager::MoveController(Controller* controller, int toControllerNumber) {

    std::list<Controller*> res;
    int i = 1;
    bool added = false;
    for (const auto& it : _controllers) {
        if (i == toControllerNumber + 1) {
            res.push_back(controller);
            added = true;
            i++;
        }
        if (it == controller) {
            // do nothing we are moving this
        }
        else {
            res.push_back(it);
            i++;
        }
    }

    if (!added) {
        res.push_back(controller);
    }

    _controllers = res;
    SomethingChanged();
}

Controller* OutputManager::GetController(const std::string& name) const {

    auto n = Trim(name);
    for (const auto& it : _controllers) {
        if (it->GetName() == n) return it;
    }
    return nullptr;
}

Controller* OutputManager::GetControllerIndex(int index) const {

    auto it = _controllers.begin();
    std::advance(it, index);
    if (it == _controllers.end()) return nullptr;
    return *it;
}

Controller* OutputManager::GetController(int32_t absoluteChannel, int32_t& startChannel) const {

    for (const auto& it : _controllers) {
        if (absoluteChannel >= it->GetStartChannel() && absoluteChannel <= it->GetEndChannel()) {
            startChannel = absoluteChannel - it->GetStartChannel() + 1;
            return it;
        }
    }
    return nullptr;
}

Controller* OutputManager::GetControllerWithIP(const std::string& ip)
{
    // Finds the first controller with the ip address
    for (const auto& it : _controllers) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr) {
            if (eth->GetIP() == ip) return eth;
        }
    }
	return nullptr;
}

int OutputManager::GetControllerIndex(Controller* c) {
    int i = 0;
    for (const auto& it : _controllers) {
        if (c == it) return i;
        i++;
    }
    return -1;
}

// ip can be ip, hostname or port
int OutputManager::GetControllerCount(const std::string& type, const std::string& ip) const {

    int count = 0;
    for (const auto& it : _controllers) {
        if (it->GetType() == type && it->GetColumn2Label() == ip) {
            count++;
        }
    }
    return count;
}

std::list<std::string> OutputManager::GetControllerNames() const {

    std::list<std::string> res;
    for (const auto& it : _controllers) {
        if (it->IsLookedUpByControllerName()) {
            res.push_back(it->GetName());
        }
    }
    return res;
}

std::list<std::string> OutputManager::GetAutoLayoutControllerNames() const {

    std::list<std::string> res;
    for (const auto& it : _controllers) {
        if (it->IsAutoLayout()) {
            res.push_back(it->GetName());
        }
    }
    return res;
}

#pragma endregion

#pragma region Output Management
int OutputManager::GetOutputCount() const {

    return std::accumulate(begin(_controllers), end(_controllers), 0, [](int accumulator, Controller* const c) { return accumulator + c->GetOutputCount(); });
}

std::list<Output*> OutputManager::GetAllOutputs(const std::string& ip, const std::string& hostname) const {

    std::list<Output*> res;
    auto outputs = GetAllOutputs();
    for (const auto& it : outputs) {
        if (ip == "" || (it->IsIpOutput() && (it->GetIP() == ip || it->GetResolvedIP() == ip || it->GetIP() == hostname))) {
            res.push_back(it);
        }
    }

    return res;
}

std::list<Output*> OutputManager::GetAllOutputs() const {

    std::list<Output*> res;
    for (const auto& it : _controllers) {
        for (const auto& it2 : it->GetOutputs()) {
            res.push_back(it2);
        }
    }

    return res;
}

// get an output based on an output number - zero based
// THIS SHOULD BE ONLY USED IN CONVERSION AS OUTPUTNUMBER IS NO LONGER A VALID ADDRESSING MECHANISM
Output* OutputManager::GetOutput_CONVERT(int outputNumber) const {

    if (outputNumber >= (int)_conversionOutputs.size() || outputNumber < 0) {
        return nullptr;
    }

    auto iter = _conversionOutputs.begin();
    std::advance(iter, outputNumber);
    return iter->first;
}

// get an output based on an absolute channel number
Output* OutputManager::GetOutput(int32_t absoluteChannel, int32_t& startChannel) const {

    auto outputs = GetAllOutputs();
    for (const auto& it : outputs) {
        if (absoluteChannel >= it->GetStartChannel() && absoluteChannel <= it->GetEndChannel()) {
            startChannel = absoluteChannel - it->GetStartChannel() + 1;
            return it;
        }
    }
    return nullptr;
}

// get an output based on a universe/id number
Output* OutputManager::GetOutput(int universe, const std::string& ip) const {

    for (const auto& it : _controllers) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && (eth->GetProtocol() == OUTPUT_E131 || eth->GetProtocol() == OUTPUT_ARTNET || eth->GetProtocol() == OUTPUT_xxxETHERNET || eth->GetProtocol() == OUTPUT_OPC)) {
            if (ip == "" || ip == eth->GetIP() || ip == eth->GetResolvedIP()) {
                for (const auto& it2 : eth->GetOutputs()) {
                    if (it2->GetUniverse() == universe) {
                        return it2;
                    }
                }
            }
        }
        else if (eth != nullptr) {
            if (ip == "" || ip == eth->GetIP() || ip == eth->GetResolvedIP()) {
                if (it->GetId() == universe) {
                    return it->GetFirstOutput();
                }
            }
        }
        else {
            if (it->GetId() == universe) {
                return it->GetFirstOutput();
            }
        }
    }
    return nullptr;
}
#pragma endregion

#pragma region Channel Mapping
int32_t OutputManager::GetTotalChannels() const {

    if (_controllers.size() == 0) return 0;
    return _controllers.back()->GetEndChannel();
}

int32_t OutputManager::GetAbsoluteChannel(int controllerIndex, int32_t startChannel) const {

    if (controllerIndex >= (int)_controllers.size()) return -1;

    auto it = _controllers.begin();
    std::advance(it, controllerIndex);
    return (*it)->GetStartChannel() + startChannel;
}

int32_t OutputManager::GetAbsoluteChannel(const std::string& ip, int universe, int32_t startChannel) const {

    auto o = GetAllOutputs(ip);
    auto it = o.begin();
    while (it != o.end()) {
        if (universe + 1 == (*it)->GetUniverse() && (ip == "" || ip == (*it)->GetIP() || ip == (*it)->GetResolvedIP())) {
            break;
        }
        ++it;
    }

    if (it == o.end()) return -1;
    return (*it)->GetStartChannel() + startChannel;
}

int32_t OutputManager::DecodeStartChannel(const std::string& startChannelString) {

    // Decodes Absolute, Output:StartChannel, #Universe:StartChannel, and #IP:Universe:StartChannel
    // If there is an error 0 is returned

    if (startChannelString == "") return 0;

    if (startChannelString.find(':') != std::string::npos) {
        if (startChannelString[0] == '#') {
            auto parts = wxSplit(&startChannelString[1], ':');
            if (parts.size() > 3) return 0;
            if (parts.size() == 2) {
                int uni = wxAtoi(parts[0]);
                long sc = wxAtol(parts[1]);
                if (uni < 1) return 0;
                if (sc < 1) return 0;
                Output* o = GetOutput(uni, "");
                if (o == nullptr) return 0;
                return o->GetStartChannel() + sc - 1;
            }
            else {
                std::string ip = parts[0].ToStdString();
                int uni = wxAtoi(parts[1]);
                long sc = wxAtol(parts[2]);
                if (ip == "") return 0;
                if (uni < 1) return 0;
                if (sc < 1) return 0;
                Output* o = GetOutput(uni, ip);
                if (o == nullptr) return 0;
                return o->GetStartChannel() + sc - 1;
            }
        }
        else {
            auto parts = wxSplit(startChannelString, ':');
            if (parts.size() > 2) return 0;
            int output = wxAtoi(parts[0]);
            long sc = wxAtol(parts[1]);
            if (output < 1) return 0;
            if (sc < 1) return 0;
            Output* o = GetOutput_CONVERT(output - 1);
            if (o == nullptr) return 0;
            return o->GetStartChannel() + sc - 1;
        }
    }
    else {
        return wxAtol(startChannelString);
    }
}
#pragma endregion

#pragma region Getters and Setters
std::string OutputManager::GetFirstUnusedCommPort() const {

    auto ports = SerialOutput::GetAvailableSerialPorts();
    if (ports.size() == 1) {
        if (ports.front() == "(no available ports)") return "NotConnected";
#ifdef __LINUX__
        if (ports.front() == "port enumeration not supported on Linux") return "NotConnected";
#endif
    }
    for (const auto& it : ports) {
        bool used = false;
        for (const auto& it2 : _controllers) {
            auto s = dynamic_cast<ControllerSerial*>(it2);
            if (s != nullptr && s->GetPort() == it) {
                used = true;
                break;
            }
        }
        if (!used) return it;
    }

    // All CommPorts used
    return "NotConnected";
}

std::list<int> OutputManager::GetIPUniverses(const std::string& ip) const {

    std::list<int> res;
    for (const auto& it : GetAllOutputs()) {
        if (ip == "" || (ip == it->GetIP() || ip == it->GetResolvedIP())) {
            wxASSERT(!it->IsOutputCollection_CONVERT());
            if (std::find(res.begin(), res.end(), it->GetUniverse()) == res.end()) {
                res.push_back(it->GetUniverse());
            }
        }
    }
    return res;
}

std::list<std::string> OutputManager::GetIps() const {

    std::list<std::string> res;
    for (const auto& it : GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr) {
            if (std::find(res.begin(), res.end(), eth->GetIP()) == res.end()) {
                res.push_back(eth->GetIP());
            }
        }
    }
    return res;
}

void OutputManager::SetGlobalFPPProxy(const std::string& globalFPPProxy)
{
    if (_globalFPPProxy != globalFPPProxy) {
        _globalFPPProxy = globalFPPProxy; _dirty = true;
        for (const auto& it : _controllers) {
            it->SetGlobalFPPProxy(globalFPPProxy);
        }
    }
}

bool OutputManager::Discover(wxWindow* frame, std::map<std::string, std::string>& renames) {

    bool found = false;

    auto zcpp = ZCPPOutput::Discover(this);

    for (const auto& it : zcpp) {
        auto c = GetControllers(it->GetIP());
        if (c.size() == 0) {
            // no match on ip ... but what about name
            bool updated = false;
            for (const auto& itc : _controllers) {
                auto eth = dynamic_cast<ControllerEthernet*>(itc);
                if (eth != nullptr && eth->GetProtocol() == OUTPUT_ZCPP && eth->GetName() == it->GetName()) {
                    if (wxMessageBox("The discovered ZCPP controller matches an existing ZCPP controllers Description but has a different IP address. Do you want to update the IP address for that existing controller in xLights?", "Mismatch IP", wxYES_NO, frame) == wxYES)
                    {
                        updated = true;
                        eth->SetIP(it->GetIP());
                        found = true;
                    }
                }
            }

            if (!updated) {
                // we need to ensure the id is still unique
                it->EnsureUniqueId();
                _controllers.push_back(it);
                found = true;
            }
        }
        else if (c.size() == 1) {
            // ip address matches
            if (it->GetName() == c.front()->GetName()) {
                // names also match ... no need to do anything
            }
            else {
                if (c.front()->GetProtocol() == OUTPUT_ZCPP) {
                    // existing zcpp with same ip but different description ... maybe we should update the description
                    if (wxMessageBox("The discovered ZCPP controller matches an existing ZCPP controllers IP address but has a different description. Do you want to update the description in xLights?", "Mismatch controller description", wxYES_NO, frame) == wxYES) {
                        renames[c.front()->GetName()] = it->GetName();
                        c.front()->SetName(it->GetName());
                        found = true;
                    }
                }
            }
        }
    }

    // There is no E1.31 Discovery
    //auto e131 = E131Output::Discover(this);
    //for (const auto& it : e131) {
    //    auto c = GetControllers(it->GetIP());
    //    if (c.size() == 0) {
    //        _controllers.push_back(it);
    //        found = true;
    //    }
    //}

    auto artnet = ArtNetOutput::Discover(this);
    for (const auto& it : artnet) {
        auto c = GetControllers(it->GetIP());
        if (c.size() == 0) {
            _controllers.push_back(it);
            found = true;
        }
    }

    // There is no DDP discovery
    //auto ddp = DDPOutput::Discover(this);
    //for (const auto& it : ddp) {
    //    auto c = GetControllers(it->GetIP());
    //    if (c.size() == 0) {
    //        _controllers.push_back(it);
    //        found = true;
    //    }
    //}

    return found;
}

void OutputManager::SetShowDir(const std::string& showDir) {

    wxFileName fn(showDir + "/" + GetNetworksFileName());
    _filename = fn.GetFullPath();
}

void OutputManager::SuspendAll(bool suspend) {

    auto outputs = GetAllOutputs();
    for_each(begin(outputs), end(outputs), [suspend](auto c) { return c->Suspend(suspend); });
}

int OutputManager::GetPacketsPerSecond() const {

    if (IsOutputting()) {
        return _lastSecondCount;
    }
    return 0;
}

// Mark all controllers with the same IP address as unmanaged
void OutputManager::UpdateUnmanaged() {

    // start with everything managed
    for (auto& it : _controllers) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && (eth->GetProtocol() == OUTPUT_E131 || eth->GetProtocol() == OUTPUT_ARTNET || eth->GetProtocol() == OUTPUT_xxxETHERNET || eth->GetProtocol() == OUTPUT_OPC)) {
            eth->SetManaged(true);
        }
    }

    auto it1 = _controllers.begin();
    while (it1 != _controllers.end())
    {
        auto eth1 = dynamic_cast<ControllerEthernet*>(*it1);
        // only need to look at managed items
        if (eth1 != nullptr && eth1->IsManaged() && (eth1->GetProtocol() == OUTPUT_E131 || eth1->GetProtocol() == OUTPUT_ARTNET || eth1->GetProtocol() == OUTPUT_xxxETHERNET || eth1->GetProtocol() == OUTPUT_OPC) && eth1->GetIP() != "MULTICAST") {
            auto it2 = it1;
            ++it2;
            while (it2 != _controllers.end()) {
                auto eth2 = dynamic_cast<ControllerEthernet*>(*it2);
                if (eth2 != nullptr && (eth2->GetProtocol() == OUTPUT_E131 || eth2->GetProtocol() == OUTPUT_ARTNET || eth2->GetProtocol() == OUTPUT_xxxETHERNET || eth2->GetProtocol() == OUTPUT_OPC)) {
                    if (eth1->GetIP() == eth2->GetIP()) {
                        eth1->SetManaged(false);
                        eth2->SetManaged(false);
                    }
                }
                ++it2;
            }
        }
        ++it1;
    }
}

std::string OutputManager::UniqueName(const std::string& prefix) {

    if (GetController(prefix) == nullptr) return prefix;

    wxString n;
    int i = 1;
    do {
        n = wxString::Format("%s_%d", prefix, i++);
    } while (GetController(n) != nullptr);
    return n.ToStdString();
}

int OutputManager::UniqueId() {
    int i = 0;
    while (GetOutput(++i, "") != nullptr);
    return i;
}

// Need to call this whenever something may have changed in an output to ensure all the transient data it updated
void OutputManager::SomethingChanged() const {
    int nullcnt = 0;
    int start = 1;
    for (auto& it : _controllers) {
        it->SetTransientData(start, nullcnt);
    }
}

bool OutputManager::IsDirty() const {

    if (_dirty) return _dirty;
    return std::any_of(begin(_controllers), end(_controllers), [](Controller* c) {return c->IsDirty(); });
}

void OutputManager::SetForceFromIP(const std::string& forceFromIP) {

    IPOutput::SetLocalIP(forceFromIP);
}

bool OutputManager::AtLeastOneOutputUsingProtocol(const std::string& protocol) const {

    for (const auto& it : GetAllOutputs()) {
        if (it->GetType() == protocol) {
            return true;
        }
    }
    return false;
}

std::string OutputManager::GetChannelName(int32_t channel) {

    int32_t startChannel = 0;
    ++channel;

    auto c = GetController(channel, startChannel);
    if (c == nullptr) {
        return wxString::Format(wxT("Ch %ld: invalid"), channel).ToStdString();
    }
    else {
        return wxString::Format(wxT("Ch %ld: Net %i #%ld"),
            channel,
            GetControllerIndex(c) + 1,
            (long)(channel - c->GetStartChannel() + 1)).ToStdString();
    }
}

bool OutputManager::IsOutputOpenInAnotherProcess() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxConfig* xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr) {
        if (xlconfig->HasEntry(_("OutputActive"))) {
            bool state;
            xlconfig->Read(_("OutputActive"), &state);
            delete xlconfig;

            if (state) {
                logger_base.warn("Output already seems to be happening. This may generate odd results.");
            }

            return state;
        }
    }
    return false;
}
#pragma endregion

#pragma region Start and Stop
bool OutputManager::StartOutput() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_outputting) return false;
    if (!_outputCriticalSection.TryEnter()) return false;

    logger_base.debug("Starting light output.");

    int started = 0;
    bool ok = true;
    bool err = false;

    for (const auto& it : GetAllOutputs()) {

        // make sure global FPP proxy is up to date ...
        it->SetGlobalFPPProxyIP(_globalFPPProxy);

        bool preok = ok;
        ok = it->Open() && ok;
        if (!ok && ok != preok) {
            auto name = it->GetIP();
            if (name == "") name = it->GetCommPort();

            logger_base.error("An error occured opening output %d (%s). Do you want to continue trying to start output?", started + 1, (const char*)name.c_str());
            if (OutputManager::IsInteractive()) {
                if (wxMessageBox(wxString::Format(wxT("An error occured opening output %d (%s). Do you want to continue trying to start output?"), started + 1, name), "Continue?", wxYES_NO) == wxNO) return _outputting;
            }
            err = true;
        }
        if (ok) started++;
        _outputting = (started > 0);
    }
    
    if (err && !_outputting) {
        // fake it so we dont keep getting error messages
        _outputting = true;
    }

    _outputCriticalSection.Leave();

    if (_outputting) {
        DisableSleepModes();
        SetGlobalOutputtingFlag(true);
    }

    return _outputting; // even partially started is ok
}

void OutputManager::StopOutput() {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    logger_base.debug("Stopping light output.");

    _outputting = false;

    for (const auto& it : GetAllOutputs()) {
        it->Close();
    }

    SetGlobalOutputtingFlag(false);
    _outputCriticalSection.Leave();

    EnableSleepModes();
}

size_t OutputManager::TxNonEmptyCount() {

    size_t res = 0;
    for (const auto& it : GetAllOutputs()) {
        res += it->TxNonEmptyCount();
    }
    return res;
}

bool OutputManager::TxEmpty() {

    for (const auto& it : GetAllOutputs()) {
        if (!it->TxEmpty()) return false;
    }
    return true;
}
#pragma endregion

#pragma region Frame Handling
void OutputManager::StartFrame(long msec) {

    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;
    
    for (const auto& it : GetAllOutputs()) {
        it->StartFrame(msec);
    }
    _outputCriticalSection.Leave();
}

void OutputManager::ResetFrame() {

    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    for (const auto& it : GetAllOutputs()) {
        it->ResetFrame();
    }
    _outputCriticalSection.Leave();
}

void OutputManager::EndFrame() {

    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    auto outputs = GetAllOutputs();
    if (_parallelTransmission) {
        std::function<void(Output*&, int)> f = [this](Output*&o, int n) {
            o->EndFrame(_suppressFrames);
        };
        parallel_for(outputs, f);
    }
    else {
        for (const auto& it : outputs) {
            it->EndFrame(_suppressFrames);
        }
    }

    if (IsSyncEnabled()) {
        if (_syncUniverse != 0) {
            if (AtLeastOneOutputUsingProtocol(OUTPUT_E131)) {
                E131Output::SendSync(_syncUniverse);
            }
        }

        if (AtLeastOneOutputUsingProtocol(OUTPUT_ARTNET)) {
            ArtNetOutput::SendSync();
        }

        if (AtLeastOneOutputUsingProtocol(OUTPUT_DDP)) {
            DDPOutput::SendSync();
        }

        if (AtLeastOneOutputUsingProtocol(OUTPUT_ZCPP)) {
            ZCPPOutput::SendSync();
        }
    }
    _outputCriticalSection.Leave();
}

void OutputManager::SendHeartbeat() {

    for (const auto& it : GetAllOutputs()) {
        it->SendHeartbeat();
    }
}
#pragma endregion

#pragma region Data Setting
// channel here is zero based
void OutputManager::SetOneChannel(int32_t channel, unsigned char data) {

    int32_t sc = 0;
    Output* output = GetOutput(channel + 1, sc);
    if (output != nullptr) {
        if (output->IsEnabled()) {
            output->SetOneChannel(sc - 1, data);
        }
    }
}

// channel here is zero based
void OutputManager::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    if (size == 0) return;

    int32_t stch;
    Output* o = GetOutput(channel + 1, stch);

    // if this doesnt map to an output then skip it
    if (o == nullptr) return;

    // get an iterator to the output which contains our first channel
    auto outputs = GetAllOutputs();
    auto it = outputs.begin();
    while (*it != o && it != outputs.end()) ++it;

    size_t left = size;
    while (left > 0 && o != nullptr) {
        wxASSERT(!o->IsOutputCollection_CONVERT());
        size_t mx = o->GetChannels() - stch + 1;
        size_t send = std::min(left, mx);
        if (o->IsEnabled()) {
            o->SetManyChannels(stch - 1, &data[size - left], send);
        }
        stch = 1;
        left -= send;

        // Move to the next output
        ++it;
        if (it == outputs.end()) {
            o = nullptr;
        }
        else {
            o = *it;
        }
    }
}

void OutputManager::AllOff(bool send) {

    if (!_outputCriticalSection.TryEnter()) return;

    for (const auto& it : GetAllOutputs()) {
        it->AllOff();
        if (send) {
            it->EndFrame(_suppressFrames);
        }
    }
    _outputCriticalSection.Leave();
}
#pragma endregion 

#pragma region Test Presets
std::list<std::string> OutputManager::GetTestPresets() {

    std::list<std::string> res;
    for (const auto& it : _testPresets) {
        res.push_back(it->GetName());
    }

    return res;
}

TestPreset* OutputManager::GetTestPreset(std::string preset) {

    for (const auto& it : _testPresets) {
        if (preset == it->GetName()) {
            return it;
        }
    }
    return nullptr;
}

// create a preset. If one with the same name exists then it is erased
TestPreset* OutputManager::CreateTestPreset(std::string preset) {

    auto apreset = GetTestPreset(preset);
    if (apreset != nullptr) {
        _testPresets.remove(apreset);
        delete apreset;
    }

    auto p = new TestPreset(preset);
    _testPresets.push_back(p);
    return p;
}
#pragma endregion 
