
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/xml/xml.h>

#include "ControllerUploadData.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../models/ModelManager.h"
#include "../models/Model.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"

#include <log4cpp/Category.hh>

#pragma region UDControllerPortModel

bool compare_modelsc(const UDControllerPortModel* first, const UDControllerPortModel* second) {
    if (first->GetSmartRemote() == second->GetSmartRemote()) {
        return first->GetStartChannel() < second->GetStartChannel();
    }
    return first->GetSmartRemote() < second->GetSmartRemote();
}

#pragma region Private Functions
bool UDControllerPortModel::ChannelsOnOutputs(const std::list<Output*>& outputs) const {

    int32_t lastChecked = _startChannel - 1;
    int32_t ll = lastChecked;
    while (lastChecked < _endChannel) {
        for (const auto& it : outputs) {
            if (lastChecked + 1 >= it->GetStartChannel() &&
                lastChecked + 1 <= it->GetEndChannel()) {
                lastChecked = it->GetEndChannel();
                break;
            }
        }

        if (ll == lastChecked) {
            return false;
        }
        ll = lastChecked;
    }
    return true;
}
#pragma endregion

#pragma region Constructors and Destructors
UDControllerPortModel::UDControllerPortModel(Model* m, Controller* controller, OutputManager* om, int string)
{
    _model = m;
    _string = string;
    _protocol = _model->GetControllerProtocol();
    _smartRemote = _model->GetSmartRemoteForString(string+1);

    if (string == -1) {
        _startChannel = _model->GetNumberFromChannelString(_model->ModelStartChannel);
        _endChannel = _startChannel + _model->GetChanCount() - 1;
    }
    else {
        _startChannel = _model->GetStringStartChan(string) + 1;
        _endChannel = _startChannel + _model->NodesPerString(string) * _model->GetChanCountPerNode() - 1;
    }

    Output* o = nullptr;
    if (controller != nullptr) {
        o = controller->GetOutput(_startChannel, _universeStartChannel);
    }
    else {
        o = om->GetOutput(_startChannel, _universeStartChannel);
    }

    if (o == nullptr) {
        _universe = -1;
    }
    else {
        _universe = o->GetUniverse();
    }
}
#pragma endregion

#pragma region Getters and Setters
int UDControllerPortModel::GetChannelsPerPixel() {

    return _model->GetNodeChannelCount(_model->GetStringType());
}

int UDControllerPortModel::GetDMXChannelOffset() {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("channel"))  return wxAtoi(node->GetAttribute("channel"));
    return 1;
}

int UDControllerPortModel::GetBrightness(int currentBrightness) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("brightness"))  return wxAtoi(node->GetAttribute("brightness"));
    return currentBrightness;
}

int UDControllerPortModel::GetNullPixels(int currentNullPixels) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("nullNodes"))  return wxAtoi(node->GetAttribute("nullNodes"));
    return currentNullPixels;
}

float UDControllerPortModel::GetGamma(int currentGamma) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("gamma"))  return wxAtof(node->GetAttribute("gamma"));
    return currentGamma;
}

std::string UDControllerPortModel::GetColourOrder(const std::string& currentColourOrder) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("colorOrder"))  return node->GetAttribute("colorOrder");
    return currentColourOrder;
}

std::string UDControllerPortModel::GetDirection(const std::string& currentDirection) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("reverse"))  return wxAtoi(node->GetAttribute("reverse")) == 1 ? "Reverse" : "Forward";
    return currentDirection;
}

int UDControllerPortModel::GetGroupCount(int currentGroupCount) {

    wxXmlNode* node = _model->GetControllerConnection();
    if (node->HasAttribute("groupCount"))  return wxAtoi(node->GetAttribute("groupCount"));
    return currentGroupCount;
}

std::string UDControllerPortModel::GetName() const {
    if (_string == -1) {
        return _model->GetName();
    }
    else {
        return _model->GetName() + "-str-" + wxString::Format("%d", _string + 1).ToStdString();
    }
}

void UDControllerPortModel::Dump() const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_string == -1) {
        logger_base.debug("                Model %s. Controller Connection %s. Start Channel %d. End Channel %d. Channels %d. Pixels %d. Start Channel #%d:%d",
            (const char*)_model->GetName().c_str(), (const char*)_model->GetControllerConnectionRangeString().c_str(),
            _startChannel, _endChannel, Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    }
    else {
        logger_base.debug("                Model %s. String %d. Controller Connection %s. Start Channel %d. End Channel %d.",
            (const char*)_model->GetName().c_str(), _string + 1, (const char*)_model->GetControllerConnectionRangeString().c_str(),
            _startChannel, _endChannel);
    }
}


bool UDControllerPortModel::Check(Controller* controller, const UDControllerPort* port, bool pixel, const ControllerCaps* rules, std::string& res) const {

    bool success = true;
    if (!ChannelsOnOutputs(controller->GetOutputs())) {
        res += wxString::Format("WARN: Model %s uses channels not being sent to this controller.\n", GetName());
    }
    return success;
}
#pragma endregion

#pragma endregion

#pragma region UDControllerPort

#pragma region Constructors and Destructors
UDControllerPort::~UDControllerPort() {

    while (_models.size() > 0) {
        delete _models.back();
        _models.pop_back();
    }
    while (_virtualStrings.size() > 0) {
        delete _virtualStrings.front();
        _virtualStrings.pop_front();
    }
}
#pragma endregion

#pragma region Model Handling
UDControllerPortModel* UDControllerPort::GetFirstModel() const {

    if (_models.size() == 0) return nullptr;
    UDControllerPortModel* first = _models.front();
    for (const auto& it : _models) {
        if (*it < *first) {
            first = it;
        }
    }
    return first;
}

UDControllerPortModel* UDControllerPort::GetLastModel() const {

    if (_models.size() == 0) return nullptr;
    UDControllerPortModel* last = _models.front();
    for (const auto& it : _models) {
        if (it->GetEndChannel() > last->GetEndChannel()) {
            last = it;
        }
    }
    return last;
}

Model* UDControllerPort::GetModelAfter(Model* m) const
{
    bool takenext = false;
    for (const auto& it : _models) {
        if (takenext) return it->GetModel();
        if (it->GetModel() == m) takenext = true;
    }
    return nullptr;
}

UDControllerPortModel* UDControllerPort::GetModel(const std::string& modelName, int str) const
{
    for (const auto& it : _models)
    {
        if (it->GetModel()->GetName() == modelName && (it->GetString() == str || it->GetString() == -1)) return it;
    }
    return nullptr;
}

void UDControllerPort::AddModel(Model* m, Controller* controller, OutputManager* om, int string, bool eliminateOverlaps) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxASSERT(!ContainsModel(m, string));

    _models.push_back(new UDControllerPortModel(m, controller, om, string));
    if (_protocol == "") {
        _protocol = m->GetControllerProtocol();
    }
    _models.sort(compare_modelsc);

    // now remove any overlapping models
    bool erased = true;
    while (erased) {
        erased = false;
        for (auto it = _models.begin(); it != _models.end(); ++it) {
            auto it2 = it;
            ++it2;

            for (; it2 != _models.end(); ++it2) {
                if ((*it2)->GetStartChannel() <= (*it)->GetEndChannel() && 
                    (*it)->GetSmartRemote() == (*it2)->GetSmartRemote()) {
                    // this model overlaps at least slightly
                    if ((*it2)->GetEndChannel() <= (*it)->GetEndChannel()) {
                        // it2 is totally inside it
                        logger_base.debug("CUD add model removed model %s as it totally overlaps with model %s",
                            (const char*)(*it2)->GetName().c_str(),
                            (const char*)(*it)->GetName().c_str()
                        );
                        if (eliminateOverlaps) (*it2)->GetModel()->SetControllerPort(0);
                        _models.erase(it2);
                        erased = true;
                    }
                    else if ((*it)->GetStartChannel() == (*it2)->GetStartChannel() && 
                             (*it2)->GetEndChannel() > (*it)->GetEndChannel() && 
                             (*it)->GetSmartRemote() == (*it2)->GetSmartRemote()) {
                        // i1 totally inside it2
                        logger_base.debug("CUD add model removed model %s as it totally overlaps with model %s",
                            (const char*)(*it)->GetName().c_str(),
                            (const char*)(*it2)->GetName().c_str()
                        );
                        if (eliminateOverlaps) (*it)->GetModel()->SetControllerPort(0);
                        _models.erase(it);
                        erased = true;
                    }
                    else if ((*it)->GetSmartRemote() == (*it2)->GetSmartRemote()) {
                        // so this is the difficult partial overlap case ... to prevent issues i will just erase model 2 and the user will need to fix it
                        logger_base.debug("CUD add model removed model %s as it PARTIALLY overlaps with model %s. This will cause issues but it cannot be handled by the upload.",
                            (const char*)(*it2)->GetName().c_str(),
                            (const char*)(*it)->GetName().c_str()
                        );
                        _models.erase(it2);
                        if (eliminateOverlaps) (*it2)->GetModel()->SetControllerPort(0);
                        erased = true;
                    }
                }
                if (erased) break; // iterators are invalid so need to break out
            }
            if (erased) break; // iterators are invalid so need to break out
        }
    }
}

bool UDControllerPort::ContainsModel(Model* m, int string) const {

    for (const auto& it : _models)
    {
        if (it->GetModel() == m && string == it->GetString()) {
            return true;
        }
    }
    return false;
}

bool UDControllerPort::SetAllModelsToControllerName(const std::string& controllerName)
{
    bool changed = false;
    for (const auto& it : _models)
    {
        if (it->IsFirstModelString())
        {
            if (it->GetModel()->GetControllerName() != controllerName)
            {
                changed = true;
                it->GetModel()->SetControllerName(controllerName);
            }
        }
    }
    return changed;
}

// Set all ports to the valid first protocol
bool UDControllerPort::SetAllModelsToValidProtocols(const std::list<std::string>& protocols, const std::string& force)
{
    if (protocols.size() == 0) return false;

    std::string p = force;
    bool changed = false;
    for (const auto& it : _models)
    {
        if (it->IsFirstModelString())
        {
            if (p == "") {
                if (std::find(protocols.begin(), protocols.end(), it->GetModel()->GetControllerProtocol()) != protocols.end()) {
                    p = it->GetModel()->GetControllerProtocol();
                }
                else
                {
                    changed = true;
                    if (p == "") {
                        it->GetModel()->SetControllerProtocol(protocols.front());
                        p = it->GetModel()->GetControllerProtocol();
                    }
                    else {
                        it->GetModel()->SetControllerProtocol(p);
                    }
                }
            }
            else
            {
                if (p != it->GetModel()->GetControllerProtocol())
                {
                    changed = true;
                    it->GetModel()->SetControllerProtocol(p);
                }
            }
        }
    }
    return changed;
}

bool UDControllerPort::ClearSmartRemoteOnAllModels()
{
    bool changed = false;
    for (const auto& it : _models)
    {
        it->GetModel()->SetSmartRemote(0);
    }
    return changed;
}

// This ensures none of the models on a port overlap ... they are all chained 
// I am a bit concerned that using this stops you overlapping things ... but really if you are going to overlap things
// you shouldnt set the port as that is just going to cause grief.
bool UDControllerPort::EnsureAllModelsAreChained()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool changed = false;
    std::string last = "";
    int lastsr = 0;

    for (const auto& it : _models)
    {
        if (it->IsFirstModelString())
        {
            if (it->GetModel()->GetSmartRemote() != lastsr) {
                last = "";
            }
            lastsr = it->GetModel()->GetSmartRemote();

            if (last == "")
            {
                if (it->GetModel()->GetModelChain() != last)
                {
                    logger_base.debug("Model '%s' was chained to '%s' but fixed to chain to '%s'.", (const char*)it->GetModel()->GetName().c_str(),
                        (const char*)it->GetModel()->GetModelChain().c_str(), (const char*)last.c_str());
                    changed = true;
                    it->GetModel()->SetModelChain(last);
                }
            }
            else
            {
                if (it->GetModel()->GetModelChain() != ">" + last)
                {
                    logger_base.debug("Model '%s' was chained to '%s' but fixed to chain to '%s'.", (const char*)it->GetModel()->GetName().c_str(),
                        (const char*)it->GetModel()->GetModelChain().c_str(), (const char*)((">" + last).c_str()));
                    changed = true;
                    it->GetModel()->SetModelChain(">" + last);
                }
            }
            last = it->GetModel()->GetName();
        }
    }
    return changed;
}
#pragma endregion

#pragma region Virtual String Handling
void UDControllerPort::CreateVirtualStrings(bool mergeSequential) {

    while (_virtualStrings.size() > 0) {
        delete _virtualStrings.front();
        _virtualStrings.pop_front();
    }

    int32_t lastEndChannel = -1000;
    UDVirtualString* current = nullptr;
    for (const auto& it : _models) {
        bool first = false;
        int brightness = it->GetBrightness(-9999);
        int nullPixels = it->GetNullPixels(-9999);
        int smartRemote = it->GetSmartRemote();
        std::string reverse = it->GetDirection("unknown");
        std::string colourOrder = it->GetColourOrder("unknown");
        float gamma = it->GetGamma(-9999);
        int groupCount = it->GetGroupCount(-9999);

        if (current == nullptr || !mergeSequential) {
            if (smartRemote != 0) {
                int curRemote = current == nullptr ? 0 : current->_smartRemote;
                curRemote++;
                for (int sr = curRemote; sr < smartRemote; sr++) {
                    // we seem to have missed one so create a dummy
                    current = new UDVirtualString();
                    _virtualStrings.push_back(current);
                    current->_endChannel = it->GetStartChannel() + 2;
                    current->_startChannel = it->GetStartChannel();
                    current->_description = "DUMMY";
                    current->_protocol = it->GetProtocol();
                    current->_universe = it->GetUniverse();
                    current->_universeStartChannel = it->GetUniverseStartChannel();
                    current->_channelsPerPixel = 3;
                    current->_smartRemote = sr;
                    current->_gammaSet = false;
                    current->_gamma = 0;
                    current->_nullPixelsSet = false;
                    current->_nullPixels = 0;
                    current->_brightnessSet = false;
                    current->_brightness = 0;
                    current->_groupCountSet = false;
                    current->_groupCount = 0;
                    current->_reverseSet = false;
                    current->_reverse = "";
                    current->_colourOrderSet = false;
                    current->_colourOrder = "";
                }
            }

            // this is automatically a new virtual string
            current = new UDVirtualString();
            _virtualStrings.push_back(current);
            first = true;
            lastEndChannel = it->GetEndChannel();
        }
        else {
            wxASSERT(current != nullptr);
            if ((brightness != -9999 && current->_brightness != brightness) ||
                (nullPixels != -9999) ||
                (current->_smartRemote != smartRemote) ||
                (reverse == "unknown" && current->_reverse == "Reverse") ||
                (reverse != "unknown" && (current->_reverse != reverse || current->_reverse == "Reverse")) ||
                (colourOrder != "unknown" && current->_colourOrder != colourOrder) ||
                (gamma != -9999 && current->_gamma != gamma) ||
                (groupCount != -9999 && current->_groupCount != groupCount) ||
                lastEndChannel + 1 != it->GetStartChannel()) {

                // smart remote has changed ... but did we miss one
                if (smartRemote != 0 && current->_smartRemote != smartRemote) {
                    for (int sr = current->_smartRemote + 1; sr < smartRemote; sr++) {
                        // we seem to have missed one so create a dummy
                        current = new UDVirtualString();
                        _virtualStrings.push_back(current);
                        current->_endChannel = it->GetStartChannel() + 2;
                        current->_startChannel = it->GetStartChannel();
                        current->_description = "DUMMY";
                        current->_protocol = it->GetProtocol();
                        current->_universe = it->GetUniverse();
                        current->_universeStartChannel = it->GetUniverseStartChannel();
                        current->_channelsPerPixel = 3;
                        current->_smartRemote = sr;
                        current->_gammaSet = false;
                        current->_gamma = 0;
                        current->_nullPixelsSet = false;
                        current->_nullPixels = 0;
                        current->_brightnessSet = false;
                        current->_brightness = 0;
                        current->_groupCountSet = false;
                        current->_groupCount = 0;
                        current->_reverseSet = false;
                        current->_reverse = "";
                        current->_colourOrderSet = false;
                        current->_colourOrder = "";
                    }
                }

                current = new UDVirtualString();
                _virtualStrings.push_back(current);
                first = true;
            }
            lastEndChannel = it->GetEndChannel();
        }

        wxASSERT(current != nullptr);
        current->_endChannel = it->GetEndChannel();
        current->_models.push_back(it);

        if (first) {
            current->_startChannel = it->GetStartChannel();
            current->_description = it->GetName();
            current->_protocol = it->GetProtocol();
            current->_universe = it->GetUniverse();
            current->_universeStartChannel = it->GetUniverseStartChannel();
            current->_channelsPerPixel = it->GetChannelsPerPixel();
            current->_smartRemote = it->GetSmartRemote();

            if (gamma == -9999) {
                current->_gammaSet = false;
                current->_gamma = 0;
            }
            else {
                current->_gammaSet = true;
                current->_gamma = gamma;
            }
            if (nullPixels == -9999) {
                current->_nullPixelsSet = false;
                current->_nullPixels = 0;
            }
            else {
                current->_nullPixelsSet = true;
                current->_nullPixels = nullPixels;
            }
            if (brightness == -9999) {
                current->_brightnessSet = false;
                current->_brightness = 0;
            }
            else {
                current->_brightnessSet = true;
                current->_brightness = brightness;
            }
            if (groupCount == -9999) {
                current->_groupCountSet = false;
                current->_groupCount = 0;
            }
            else {
                current->_groupCountSet = true;
                current->_groupCount = groupCount;
            }
            if (reverse == "unknown") {
                current->_reverseSet = false;
                current->_reverse = "";
            }
            else {
                current->_reverseSet = true;
                current->_reverse = reverse;
            }
            if (colourOrder == "unknown") {
                current->_colourOrderSet = false;
                current->_colourOrder = "";
            }
            else {
                current->_colourOrderSet = true;
                current->_colourOrder = colourOrder;
            }
        }
    }

    int lastremote = 0;
    int count = 0;
    for (auto& vs : _virtualStrings) {
        if (vs->_smartRemote != lastremote) {
            count = 0;
        }
        vs->_index = count++;
        lastremote = vs->_smartRemote;
    }
}
#pragma endregion

#pragma region Getters and Setters
int32_t UDControllerPort::GetStartChannel() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        return GetFirstModel()->GetStartChannel();
    }
}

int32_t UDControllerPort::GetEndChannel() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        return GetLastModel()->GetEndChannel();
    }
}

int32_t UDControllerPort::Channels() const {

    if (_virtualStrings.size() == 0) {
        if (_models.size() == 0) return 0;

        return GetEndChannel() - GetStartChannel() + 1;
    }
    else {
        int c = 0;
        for (const auto& it : _virtualStrings) {
            c += it->Channels();
        }
        return c;
    }
}

int UDControllerPort::GetUniverse() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        return GetFirstModel()->GetUniverse();
    }
}

int UDControllerPort::GetUniverseStartChannel() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        return GetFirstModel()->GetUniverseStartChannel();
    }
}

bool UDControllerPort::IsPixelProtocol() const {
    return Model::IsPixelProtocol(_protocol);
}

std::string UDControllerPort::GetPortName() const {

    if (_models.size() == 0) {
        return "";
    }
    else {
        return GetFirstModel()->GetName();
    }
}

bool UDControllerPort::AtLeastOneModelIsUsingSmartRemote() const
{
    for (const auto& it : _models) {
        if (it->GetSmartRemote() > 0) return true;
    }
    return false;
}

bool UDControllerPort::AtLeastOneModelIsNotUsingSmartRemote() const
{
    for (const auto& it : _models) {
        if (it->GetSmartRemote() == 0) return true;
    }
    return false;
}

void UDControllerPort::Dump() const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("            Port %d. Protocol %s. Valid %s. Invalid Reason '%s'. Channels %d. Pixels %d. Start #%d:%d.", _port, (const char*)_protocol.c_str(), (_valid ? "TRUE" : "FALSE"), (const char*)_invalidReason.c_str(), Channels(), (int)(Channels() / 3), GetUniverse(), GetUniverseStartChannel());
    for (const auto& it : _models) {
        it->Dump();
    }
}

bool UDControllerPort::Check(Controller* c, const UDController* controller, bool pixel, const ControllerCaps* rules, std::string& res) const {

    bool success = true;

    // protocols must be valid for these output types and controller
    if (pixel) {
        if (!rules->IsValidPixelProtocol(_protocol)) {
            res += wxString::Format("ERR: Invalid protocol on pixel port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else {
            for (const auto& it : _models) {
                if (Lower(it->GetProtocol()) != Lower(_protocol)) {
                    res += wxString::Format("ERR: Model %s on pixel port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.", it->GetName(), _port, it->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }

        // port must not have too many pixels on it
        if (Channels() > rules->GetMaxPixelPortChannels()) {
            res += wxString::Format("ERR: Pixel port %d has %d nodes allocated but maximum is %d.\n", _port, (int)Channels() / 3, rules->GetMaxPixelPortChannels() / 3).ToStdString();
            success = false;
        }
    }
    else {
        if (!rules->IsValidSerialProtocol(_protocol)) {
            res += wxString::Format("ERR: Invalid protocol on serial port %d: %s\n", _port, _protocol).ToStdString();
            success = false;
        }
        else {
            for (const auto& it : _models) {
                if (Lower(it->GetProtocol()) != Lower(_protocol)) {
                    res += wxString::Format("ERR: Model %s on serial port %d has protocol %s but port protocol has been set to %s. This is because you have mixed protocols on your models.\n", it->GetName(), _port, it->GetProtocol(), _protocol).ToStdString();
                    success = false;
                }
            }
        }
        if (Channels() > rules->GetMaxSerialPortChannels()) {
            res += wxString::Format("ERR: Serial port %d has %d channels allocated but maximum is %d.\n", _port, (int)Channels(), rules->GetMaxSerialPortChannels()).ToStdString();
            success = false;
        }
    }

    int32_t ch = -1;
    int lastSmartRemote = 0;
    for (const auto& it : _models) {
        if (ch == -1) ch = it->GetStartChannel() - 1;
        if (it->GetStartChannel() > ch + 1 && lastSmartRemote == it->GetSmartRemote()) {
            if (it->GetSmartRemote() == 0) {
                res += wxString::Format("WARN: Gap in models on port %d channel %d to %d.\n", _port, ch, it->GetStartChannel()).ToStdString();
            }
            else {
                res += wxString::Format("WARN: Gap in models on port %d smart remote %d channel %d to %d.\n", _port, it->GetSmartRemote(), ch, it->GetStartChannel()).ToStdString();
            }
        }
        lastSmartRemote = it->GetSmartRemote();
        ch = it->GetEndChannel();
    }

    for (const auto& it : _models) {
        // all models must be fully contained within the outputs on this controller
        success &= it->Check(c, this, pixel, rules, res);
    }

    return success;
}
#pragma endregion

#pragma endregion

#pragma region UDController

#pragma region Private Functions
bool UDController::ModelProcessed(Model* m, int string) {

    for (const auto& it : _pixelPorts) {
        if (it.second->ContainsModel(m, string)) {
            return true;
        }
    }

    for (const auto& it : _serialPorts) {
        if (it.second->ContainsModel(m, string)) {
            return true;
        }
    }

    return false;
}
#pragma endregion

#pragma region Constructors and Destructors
UDController::UDController(Controller* controller, OutputManager* om, ModelManager* mm, std::string& check, bool eliminateOverlaps) :
    _controller(controller), _outputManager(om), _modelManager(mm) {

    _ipAddress = _controller->GetColumn2Label();

    Rescan(eliminateOverlaps);
}

UDController::~UDController() {

    for (const auto& it : _pixelPorts) {
        delete it.second;
    }
    _pixelPorts.clear();

    for (const auto& it : _serialPorts) {
        delete it.second;
    }
    _serialPorts.clear();
}

void UDController::Rescan(bool eliminateOverlaps) {
    for (const auto& it : _pixelPorts) {
        delete it.second;
    }
    _pixelPorts.clear();

    for (const auto& it : _serialPorts) {
        delete it.second;
    }
    _serialPorts.clear();

    for (const auto& it : *_modelManager) {
        if (!ModelProcessed(it.second, 1) && it.second->GetDisplayAs() != "ModelGroup") {
            int32_t modelstart = it.second->GetNumberFromChannelString(it.second->ModelStartChannel);
            int32_t modelend = modelstart + it.second->GetChanCount() - 1;
            if ((modelstart >= _controller->GetStartChannel() && modelstart <= _controller->GetEndChannel()) ||
                (modelend >= _controller->GetStartChannel() && modelend <= _controller->GetEndChannel())) {
                //logger_base.debug("Model %s start %d end %d found on controller %s output %d start %d end %d.",
                //    (const char *)it->first.c_str(), modelstart, modelend,
                //    (const char *)_ip.c_str(), node, currentcontrollerstartchannel, currentcontrollerendchannel);
                if (!it.second->IsControllerConnectionValid()) {
                    // only warn if we have not already warned
                    if (std::find(_noConnectionModels.begin(), _noConnectionModels.end(), it.second) == _noConnectionModels.end()) {
                        _noConnectionModels.push_back(it.second);
                    }
                }
                else {
                    // model uses channels in this universe
                    if (it.second->IsPixelProtocol()) {
                        if (it.second->GetNumPhysicalStrings() == 1) {
                            int port = it.second->GetControllerPort(1);
                            GetControllerPixelPort(port)->AddModel(it.second, _controller, _outputManager, -1, eliminateOverlaps);
                        }
                        else {
                            for (int i = 0; i < it.second->GetNumPhysicalStrings(); i++) {
                                int port = it.second->GetControllerPort(i+1);
                                int32_t startChannel = it.second->GetStringStartChan(i) + 1;
                                int32_t sc;
                                Controller* c = _outputManager->GetController(startChannel, sc);
                                if (c != nullptr &&
                                    _controller->GetColumn2Label() == c->GetColumn2Label()) {
                                    GetControllerPixelPort(port)->AddModel(it.second, _controller, _outputManager, i, eliminateOverlaps);
                                }
                            }
                        }
                    }
                    else {
                        int port = it.second->GetControllerPort(1);
                        GetControllerSerialPort(port)->AddModel(it.second, _controller, _outputManager, -1, eliminateOverlaps);
                    }
                }
            }
        }
    }
}
#pragma endregion

#pragma region Port Handling
UDControllerPort* UDController::GetControllerPixelPort(int port) {

    for (const auto& it : _pixelPorts) {
        if (it.second->GetPort() == port) {
            return it.second;
        }
    }
    _pixelPorts[port] = new UDControllerPort("Pixel", port);
    return _pixelPorts[port];
}

UDControllerPort* UDController::GetControllerSerialPort(int port) {

    if (!HasSerialPort(port)) {
        _serialPorts[port] = new UDControllerPort("USB/Serial", port);
    }
    return _serialPorts[port];
}
UDControllerPortModel* UDController::GetControllerPortModel(const std::string& modelName, int str)
{
    for (const auto& it : _pixelPorts)
    {
        auto m = it.second->GetModel(modelName, str);
        if (m != nullptr) return m;
    }
    for (const auto& it : _serialPorts)
    {
        auto m = it.second->GetModel(modelName, str);
        if (m != nullptr) return m;
    }
    return nullptr;
}
#pragma endregion

#pragma region Getters and Setters
int UDController::GetMaxSerialPort() const {

    int last = 0;
    for (const auto& it : _serialPorts) {
        if (it.second->GetPort() > last) {
            last = it.second->GetPort();
        }
    }
    return last;
}

int UDController::GetMaxPixelPort() const {

    int last = 0;
    for (const auto& it : _pixelPorts) {
        if (it.second->GetPort() > last) {
            last = it.second->GetPort();
        }
    }
    return last;
}

bool UDController::HasSerialPort(int port) const {

    for (const auto& it : _serialPorts) {
        if (it.second->GetPort() == port) {
            return true;
        }
    }
    return false;
}

int UDController::GetMaxPixelPortChannels() const
{
    int res = 0;

    for (const auto& it : _pixelPorts) {
        res = std::max(res, it.second->Channels());
    }
    return res;
}

Model* UDController::GetModelAfter(Model* m) const
{
    for (const auto& it : _pixelPorts)
    {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    for (const auto& it : _serialPorts)
    {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    return nullptr;
}

bool UDController::SetAllModelsToControllerName(const std::string& controllerName)
{
    bool changed = false;
    for (const auto& it : _pixelPorts)
    {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        changed |= it.second->EnsureAllModelsAreChained();
    }

    for (const auto& it : _serialPorts)
    {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        changed |= it.second->EnsureAllModelsAreChained();
    }

    return changed;
}

bool UDController::SetAllModelsToValidProtocols(const std::list<std::string>& pixelProtocols, const std::list<std::string>& serialProtocols, bool allsame)
{
    std::string force;
    bool changed = false;
    for (const auto& it : _pixelPorts)
    {
        changed |= it.second->SetAllModelsToValidProtocols(pixelProtocols, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr)
        {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }

    force = "";
    for (const auto& it : _serialPorts)
    {
        changed |= it.second->SetAllModelsToValidProtocols(serialProtocols, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr)
        {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }

    return changed;
}

bool UDController::ClearSmartRemoteOnAllModels()
{
    bool changed = false;
    for (const auto& it : _pixelPorts)
    {
        changed |= it.second->ClearSmartRemoteOnAllModels();
    }

    for (const auto& it : _serialPorts)
    {
        changed |= it.second->ClearSmartRemoteOnAllModels();
    }
    return changed;
}

bool UDController::HasPixelPort(int port) const {

    for (const auto& it : _pixelPorts) {
        if (it.second->GetPort() == port) {
            return true;
        }
    }
    return false;
}

bool UDController::IsValid(ControllerCaps* rules) const {

    for (const auto& it : _pixelPorts) {
        if (!it.second->IsValid()) return false;
    }
    for (const auto& it : _serialPorts) {
        if (!it.second->IsValid()) return false;
    }
    return true;
}

void UDController::Dump() const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("UDController Dump");

    logger_base.debug("   IP Address %s.", (const char*)_ipAddress.c_str());
    if (_hostName != "" && _hostName != _ipAddress) {
        logger_base.debug("   HostName %s.", (const char*)_hostName.c_str());
    }
    logger_base.debug("   Outputs:");
    auto outputs = _controller->GetOutputs();
    for (const auto& it : outputs) {
        logger_base.debug("        %s", it->GetLongDescription().c_str());
    }
    logger_base.debug("   Pixel Ports %d. Maximum port Number %d.", (int)_pixelPorts.size(), GetMaxPixelPort());
    for (const auto& it : _pixelPorts) {
        it.second->Dump();
    }
    logger_base.debug("   Serial Ports %d. Maximum port Number %d.", (int)_serialPorts.size(), GetMaxSerialPort());
    for (const auto& it : _serialPorts) {
        it.second->Dump();
    }
}

bool UDController::Check(const ControllerCaps* rules, std::string& res) {

    if (rules == nullptr) {
        res += "No rules to evaluate.";
        return false;
    }

    bool success = true;

    auto outputs = _controller->GetOutputs();

    // all serial ports must be valid ports for this controller
    // all pixel ports must be valid for this controller
    if (rules->GetMaxPixelPort() == 0 && _pixelPorts.size() > 0) {
        res += wxString::Format("ERR: Attempt to upload pixel port %d but this controller does not support pixel ports.\n", _pixelPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else {
        for (const auto& it : _pixelPorts) {
            if (rules->SupportsVirtualStrings()) {
                it.second->CreateVirtualStrings(rules->MergeConsecutiveVirtualStrings());
            }

            success &= it.second->Check(_controller, this, true, rules, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxPixelPort()) {
                res += wxString::Format("ERR: Pixel port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxPixelPort()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsMultipleSimultaneousInputProtocols()) {
        std::string protocol = "";
        for (const auto& o : outputs) {
            if (protocol == "") {
                protocol = o->GetType();
            }
            else
            {
                if (o->GetType() != protocol) {
                    res += wxString::Format("ERR: Controller only support a single input protocol at a time. %s and %s found.\n", protocol, o->GetType()).ToStdString();
                    success = false;
                }
            }
        }
    }

    for (const auto& o : outputs) {
        if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET) {
            if (o->GetChannels() > rules->GetMaxInputUniverseChannels()) {
                res += wxString::Format("ERR: Controller limits universe sizes to max of %d but you are trying to use %d. Universe %d.\n", rules->GetMaxInputUniverseChannels(), o->GetChannels(), o->GetUniverse()).ToStdString();
                success = false;
            }
            if (o->GetChannels() < rules->GetMinInputUniverseChannels()) {
                res += wxString::Format("ERR: Controller limits universe sizes to min of %d but you are trying to use %d. Universe %d.\n", rules->GetMinInputUniverseChannels(), o->GetChannels(), o->GetUniverse()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsMultipleSimultaneousOutputProtocols()) {
        std::string protocol;
        for (const auto& it : _pixelPorts) {
            if (protocol == "") protocol = it.second->GetProtocol();

            if (protocol != it.second->GetProtocol()) {
                res += wxString::Format("ERR: Pixel ports only support a single protocol at a time. %s and %s found.\n", protocol, it.second->GetProtocol()).ToStdString();
                success = false;
            }
        }

        protocol = "";
        for (const auto& it : _serialPorts) {
            if (protocol == "") protocol = it.second->GetProtocol();

            if (protocol != it.second->GetProtocol()) {
                res += wxString::Format("ERR: Serial ports only support a single protocol at a time. %s and %s found.\n", protocol, it.second->GetProtocol()).ToStdString();
                success = false;
            }
        }
    }

    if (!rules->SupportsSmartRemotes()) {
        for (const auto& it : _pixelPorts) {
            for (const auto& it2 : it.second->GetModels()) {
                if (it2->GetSmartRemote() != 0) {
                    res += wxString::Format("ERR: Port %d has model %s with smart remote set ... but this controller does not support smart remotes.\n",
                        it.second->GetPort(), it2->GetName());
                    success = false;
                }
            }
        }
    }
    else {
        for (const auto& it : _pixelPorts) {
            int countNoSmart = 0;
            int countSmart = 0;
            for (const auto& it2 : it.second->GetModels()) {
                if (it2->GetSmartRemote() == 0) {
                    countNoSmart++;
                }
                else {
                    countSmart++;
                }
            }
            if (countSmart > 0 && countNoSmart > 0) {
                res += wxString::Format("ERR: Port %d has a mix of models with smart and non smart remotes. This is not supported.\n",
                    it.second->GetPort());
                success = false;
            }
        }
    }

    if (rules->AllInputUniversesMustBeSameSize()) {
        int size = -1;
        for (const auto& it : outputs) {
            if (size == -1) size = it->GetChannels();
            if (size != it->GetChannels()) {
                res += wxString::Format("ERR: All universes must be the same size. %d and %d found.\n", size, (int)it->GetChannels()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->UniversesMustBeInNumericalOrder()) {
        int last = -1;

        for (const auto& it : outputs) {
            if (last >= it->GetUniverse()) {
                res += wxString::Format("ERR: All universes must be in numerical order. %d followed %d.\n", it->GetUniverse(), last).ToStdString();
                success = false;
            }
            last = it->GetUniverse();
        }
    }

    if (rules->UniversesMustBeSequential()) {
        int seq = -1;

        for (const auto& it : outputs) {
            if (seq == -1) seq = it->GetUniverse() - 1;
            if (seq + 1 != it->GetUniverse()) {
                res += wxString::Format("ERR: All universes must be sequential. %d followed %d.\n", it->GetUniverse(), seq).ToStdString();
                success = false;
            }
            seq = it->GetUniverse();
        }
    }

    ControllerEthernet* eth = dynamic_cast<ControllerEthernet*>(_controller);
    if (eth != nullptr) {
        if (!rules->IsValidInputProtocol(eth->GetProtocol())) {
            res += wxString::Format("ERR: %s is not a protocol this controller supports.\n", eth->GetProtocol()).ToStdString();
            success = false;
        }
    }

    if (rules->GetMaxSerialPort() == 0 && _serialPorts.size() > 0) {
        res += wxString::Format("ERR: Attempt to upload serial port %d but this controller does not support serial ports.\n", _serialPorts.begin()->second->GetPort()).ToStdString();
        success = false;
    }
    else {
        for (const auto& it : _serialPorts) {
            success &= it.second->Check(_controller, this, false, rules, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxSerialPort()) {
                res += wxString::Format("ERR: Serial port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxSerialPort()).ToStdString();
                success = false;
            }
        }
    }

    return success;
}

Output* UDController::GetFirstOutput() const {

    return _controller->GetFirstOutput();
}
#pragma endregion

#pragma endregion
