
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
#include "../UtilFunctions.h"
#include "../Pixels.h"

#define NO_VALUE_INT -9999
#define NO_VALUE_STRING "unknown"

#include <numeric>

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
    _smartRemoteType = _model->GetSmartRemoteType();

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
int UDControllerPortModel::GetChannelsPerPixel() const
{
    return _model->GetNodeChannelCount(_model->GetStringType());
}

int UDControllerPortModel::GetLightsPerNode() const
{
    return _model->GetLightsPerNode();
}

int UDControllerPortModel::GetDMXChannelOffset() const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("channel"))
        return wxAtoi(node->GetAttribute("channel"));
    return 1;
}

int UDControllerPortModel::GetBrightness(int currentBrightness) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("brightness"))  return wxAtoi(node->GetAttribute("brightness"));
    return currentBrightness;
}

int UDControllerPortModel::GetStartNullPixels(int currentStartNullPixels) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("nullNodes"))
        return wxAtoi(node->GetAttribute("nullNodes"));
    return currentStartNullPixels;
}

int UDControllerPortModel::GetEndNullPixels(int currentEndNullPixels) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("endNullNodes"))
        return wxAtoi(node->GetAttribute("endNullNodes"));
    return currentEndNullPixels;
}

char UDControllerPortModel::GetSmartRemoteLetter() const
{
    if (_smartRemote == 0) return ' ';
    if (_smartRemote < 100) return char('A' + _smartRemote - 1);
    return char('A' + _smartRemote - 100);
}

float UDControllerPortModel::GetAmps(int defaultBrightness) const
{
    return ((float)AMPS_PER_PIXEL * (float)INTROUNDUPDIV(Channels() * GetLightsPerNode(), GetChannelsPerPixel()) * GetBrightness(defaultBrightness)) / 100.0F;
}

int UDControllerPortModel::GetSmartTs(int currentSmartTs) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("ts"))
        return wxAtoi(node->GetAttribute("ts"));
    return currentSmartTs;
}

float UDControllerPortModel::GetGamma(int currentGamma)  const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("gamma"))
        return wxAtof(node->GetAttribute("gamma"));
    return currentGamma;
}

std::string UDControllerPortModel::GetColourOrder(const std::string& currentColourOrder) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("colorOrder"))
        return node->GetAttribute("colorOrder");
    return currentColourOrder;
}

std::string UDControllerPortModel::GetDirection(const std::string& currentDirection) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("reverse"))
        return wxAtoi(node->GetAttribute("reverse")) == 1 ? "Reverse" : "Forward";
    return currentDirection;
}

int UDControllerPortModel::GetGroupCount(int currentGroupCount) const
{

    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("groupCount")) {
        return wxAtoi(node->GetAttribute("groupCount"));
    }
    return currentGroupCount;
}

int UDControllerPortModel::GetZigZag(int currentZigZag) const
{
    wxXmlNode* node = _model->GetControllerConnection();
    if (node != nullptr && node->HasAttribute("zigZag")) {
        return wxAtoi(node->GetAttribute("zigZag"));
    }
    return currentZigZag;
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
            _startChannel, _endChannel, Channels(),  INTROUNDUPDIV(Channels() , GetChannelsPerPixel()), GetUniverse(), GetUniverseStartChannel());
    }
    else {
        logger_base.debug("                Model %s. String %d. Controller Connection %s. Start Channel %d. End Channel %d.",
            (const char*)_model->GetName().c_str(), _string + 1, (const char*)_model->GetControllerConnectionRangeString().c_str(),
            _startChannel, _endChannel);
    }
}


bool UDControllerPortModel::Check(Controller* controller, const ControllerCaps* rules, std::string& res) const {

    bool success = true;
    if (!ChannelsOnOutputs(controller->GetOutputs())) {
        res += wxString::Format("WARN: Model %s uses channels not being sent to this controller.\n", GetName());
    }
    if (_model->GetSmartRemote() + _model->GetSRMaxCascade() - 1 > rules->GetSmartRemoteCount()) {
        res += wxString::Format("ERR: Model %s has invalid smart remote %c with cascade of %d.\n", GetName(), _model->GetSmartRemoteLetter(), _model->GetSRMaxCascade());
        success = false;
    }
    if (rules->GetMaxStartNullPixels() >= 0 && GetStartNullPixels(-1) > rules->GetMaxStartNullPixels()) {
        res += wxString::Format("ERR: Model %s has too many start NULL pixels : %d. Maximum %d.\n", GetName(), GetStartNullPixels(-1), rules->GetMaxStartNullPixels());
        success = false;
    }
    if (rules->GetMaxEndNullPixels() >= 0 && GetEndNullPixels(-1) > rules->GetMaxEndNullPixels()) {
        res += wxString::Format("ERR: Model %s has too many end NULL pixels : %d. Maximum %d.\n", GetName(), GetEndNullPixels(-1), rules->GetMaxEndNullPixels());
        success = false;
    }
    if (rules->GetMaxGroupPixels() >= 0 && GetGroupCount(-1) > rules->GetMaxGroupPixels()) {
        res += wxString::Format("ERR: Model %s has too many grouped pixels : %d. Maximum %d.\n", GetName(), GetGroupCount(-1), rules->GetMaxGroupPixels());
        success = false;
    }
    if (rules->GetMinGroupPixels() >= 0 && GetGroupCount(999) < rules->GetMinGroupPixels()) {
        res += wxString::Format("ERR: Model %s has too few grouped pixels : %d. Minimum %d.\n", GetName(), GetGroupCount(999), rules->GetMinGroupPixels());
        success = false;
    }
    if (rules->GetMaxZigZagPixels() >= 0 && GetZigZag(-1) > rules->GetMaxZigZagPixels()) {
        res += wxString::Format("ERR: Model %s has too many zig zagged pixels : %d. Maximum %d.\n", GetName(), GetZigZag(-1), rules->GetMaxZigZagPixels());
        success = false;
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
UDControllerPortModel* UDControllerPort::GetFirstModel() const
{
    if (_models.size() == 0) return nullptr;
    UDControllerPortModel* first = _models.front();
    for (const auto& it : _models) {
        if (*it < *first) {
            first = it;
        }
    }
    return first;
}

UDControllerPortModel* UDControllerPort::GetFirstModel(int sr) const
{
    if (_models.size() == 0)
        return nullptr;
    UDControllerPortModel* first = nullptr;
    for (const auto& it : _models) {
        if (it->GetSmartRemote() == sr) {
            if (first == nullptr || *it < *first) {
                first = it;
            }
        }
    }
    return first;
}

UDControllerPortModel* UDControllerPort::GetLastModel() const
{
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

Model* UDControllerPort::GetModelBefore(Model* m) const
{
    Model* last = nullptr;
    for (const auto& it : _models) {
        if (it->GetModel() == m) return last;
        last = it->GetModel();
    }
    return last;
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

    _om = om;

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
                        if (eliminateOverlaps) (*it2)->GetModel()->SetControllerPort(0);
                        _models.erase(it2);
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

    for (const auto& it : _models) {
        if (it->GetModel() == m && (string == it->GetString() || (string == 0 && it->GetString() == -1))) {
            return true;
        }
    }
    return false;
}

int UDControllerPort::CountEmptySmartRemotesBefore(int sr) const
{
    int count = 0;

    for (int s = 1; s < sr; ++s)
    {
        if (GetModelCount(s) == 0) ++count;
    }
    return count;
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
bool UDControllerPort::SetAllModelsToValidProtocols(const std::vector<std::string>& protocols, const std::string& force)
{
    if (protocols.size() == 0) return false;

    std::string p = force;
    bool changed = false;
    for (const auto& it : _models)
    {
        if (it->IsFirstModelString())
        {
            if (p == "") {
                std::string np = "";
                // this tries to find a protocol the controller can handle that is compatible
                if (GetType() == "PIXEL") {
                    np = ChooseBestControllerPixel(protocols, it->GetModel()->GetControllerProtocol());
                }
                else {
                    np = ChooseBestControllerSerial(protocols, it->GetModel()->GetControllerProtocol());
                }
                if (np == it->GetModel()->GetControllerProtocol()) {
                    p = it->GetModel()->GetControllerProtocol();
                }
                else
                {
                    changed = true;
                    if (p == "") {
                        if (np == "") {
                            it->GetModel()->SetControllerProtocol(protocols.front());
                        }
                        else {
                            it->GetModel()->SetControllerProtocol(np);
                        }
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
    std::string last;
    int lastsr{ 0 };

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
    int curRemote = 0;
    for (const auto& it : _models) {
        bool first = false;
        int brightness = it->GetBrightness(NO_VALUE_INT);
        int startNullPixels = it->GetStartNullPixels(NO_VALUE_INT);
        int endNullPixels = it->GetEndNullPixels(NO_VALUE_INT);
        int smartRemote = it->GetSmartRemote();
        std::string reverse = it->GetDirection(NO_VALUE_STRING);
        std::string colourOrder = it->GetColourOrder(NO_VALUE_STRING);
        float gamma = it->GetGamma(NO_VALUE_INT);
        int groupCount = it->GetGroupCount(NO_VALUE_INT);
        int zigZag = it->GetZigZag(NO_VALUE_INT);
        int ts = it->GetSmartTs(NO_VALUE_INT);

        if (current == nullptr || !mergeSequential) {
            if (smartRemote != 0) {
                curRemote++;
                for (int sr = curRemote; sr < smartRemote; sr++) {
                    // we seem to have missed one so create a dummy
                    current = new UDVirtualString();
                    _virtualStrings.push_back(current);
                    curRemote++;
                    current->_endChannel = it->GetStartChannel() + 2;
                    current->_startChannel = it->GetStartChannel();
                    current->_description = "DUMMY";
                    current->_isDummy = true;
                    current->_protocol = it->GetProtocol();
                    current->_universe = it->GetUniverse();
                    current->_universeStartChannel = it->GetUniverseStartChannel();
                    current->_channelsPerPixel = GetChannelsPerPixel(it->GetProtocol());
                    current->_smartRemote = sr;
                    current->_smartRemoteType = it->GetSmartRemoteType();
                    current->_gammaSet = false;
                    current->_gamma = 0;
                    current->_startNullPixelsSet = false;
                    current->_startNullPixels = 0;
                    current->_endNullPixelsSet = false;
                    current->_endNullPixels = 0;
                    current->_brightnessSet = false;
                    current->_brightness = 0;
                    current->_groupCountSet = false;
                    current->_groupCount = 0;
                    current->_zigZagSet = false;
                    current->_zigZag = 0;
                    current->_reverseSet = false;
                    current->_reverse = "";
                    current->_colourOrderSet = false;
                    current->_colourOrder = "";
                    current->_tsSet = false;
                    current->_ts = 0;
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
            if ((brightness != NO_VALUE_INT && current->_brightness != brightness) ||
                (startNullPixels != NO_VALUE_INT) ||                                                    // we always create a new virtual string for models with start nulls
                (endNullPixels != NO_VALUE_INT || (endNullPixels == NO_VALUE_INT && current->_endNullPixels != 0)) || // we dont assume end nulls carries across automatically between props and we always create a new virtual string for models with end nulls
                (current->_smartRemote != smartRemote) ||
                (reverse == NO_VALUE_STRING && current->_reverse == "Reverse") ||
                (reverse != NO_VALUE_STRING && (current->_reverse != reverse || current->_reverse == "Reverse")) ||
                (colourOrder != NO_VALUE_STRING && current->_colourOrder != colourOrder) ||
                (gamma != NO_VALUE_INT && current->_gamma != gamma) ||
                (groupCount != NO_VALUE_INT && current->_groupCount != groupCount) ||
                (zigZag != NO_VALUE_INT || (zigZag == NO_VALUE_INT && current->_zigZag != 0)) || // we dont assume zigzag carries across automatically between props and we always create a new virtual string for models with zig zag set
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
                        current->_isDummy = true;
                        current->_protocol = it->GetProtocol();
                        current->_universe = it->GetUniverse();
                        current->_universeStartChannel = it->GetUniverseStartChannel();
                        current->_channelsPerPixel = GetChannelsPerPixel(it->GetProtocol());
                        current->_smartRemote = sr;
                        current->_smartRemoteType = it->GetSmartRemoteType();
                        current->_gammaSet = false;
                        current->_gamma = 0;
                        current->_startNullPixelsSet = false;
                        current->_startNullPixels = 0;
                        current->_endNullPixelsSet = false;
                        current->_endNullPixels = 0;
                        current->_brightnessSet = false;
                        current->_brightness = 0;
                        current->_groupCountSet = false;
                        current->_groupCount = 0;
                        current->_zigZagSet = false;
                        current->_zigZag = 0;
                        current->_reverseSet = false;
                        current->_reverse = "";
                        current->_colourOrderSet = false;
                        current->_colourOrder = "";
                        current->_tsSet = false;
                        current->_ts = 0;
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
            if (current->_channelsPerPixel == 1)
                current->_channelsPerPixel = GetChannelsPerPixel(it->GetProtocol()); // this happens if a channel block is dropped first on a port and we dont want that
            current->_smartRemote = it->GetSmartRemote();
            current->_smartRemoteType = it->GetSmartRemoteType();

            if (gamma == NO_VALUE_INT) {
                current->_gammaSet = false;
                current->_gamma = 0;
            }
            else {
                current->_gammaSet = true;
                current->_gamma = gamma;
            }
            if (startNullPixels == NO_VALUE_INT) {
                current->_startNullPixelsSet = false;
                current->_startNullPixels = 0;
            }
            else {
                current->_startNullPixelsSet = true;
                current->_startNullPixels = startNullPixels;
            }
            if (endNullPixels == NO_VALUE_INT) {
                current->_endNullPixelsSet = false;
                current->_endNullPixels = 0;
            }
            else {
                current->_endNullPixelsSet = true;
                current->_endNullPixels = endNullPixels;
            }
            if (brightness == NO_VALUE_INT) {
                current->_brightnessSet = false;
                current->_brightness = 0;
            }
            else {
                current->_brightnessSet = true;
                current->_brightness = brightness;
            }
            if (groupCount == NO_VALUE_INT) {
                current->_groupCountSet = false;
                current->_groupCount = 0;
            }
            else {
                current->_groupCountSet = true;
                current->_groupCount = groupCount;
            }
            if (zigZag == NO_VALUE_INT) {
                current->_zigZagSet = false;
                current->_zigZag = 0;
            } else {
                current->_zigZagSet = true;
                current->_zigZag = zigZag;
            }
            if (ts == NO_VALUE_INT) {
                current->_tsSet = false;
                current->_ts = 0;
            }
            else {
                current->_tsSet = true;
                current->_ts = ts;
            }
            if (reverse == NO_VALUE_STRING) {
                current->_reverseSet = false;
                current->_reverse = "";
            }
            else {
                current->_reverseSet = true;
                current->_reverse = reverse;
            }
            if (colourOrder == NO_VALUE_STRING) {
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

int UDControllerPort::GetModelCount(int sr) const
{
    int count = 0;
    for (const auto& it : _virtualStrings)
    {
        if (it->_smartRemote == sr && !it->_isDummy)
            ++count;
    }
    return count;
}
#pragma endregion

#pragma region Getters and Setters
int32_t UDControllerPort::GetStartChannel() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        if (_type == "Serial") {
            return GetFirstModel()->GetStartChannel() - GetFirstModel()->GetDMXChannelOffset() + 1;
        }
        else {
            return GetFirstModel()->GetStartChannel();
        }
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

        if (_separateUniverses) {
            int c = 0;
            for (const auto& it : _models) {
                c += it->Channels();
            }
            return c;
        } else {
            return GetEndChannel() - GetStartChannel() + 1;
        }
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
        if (_type == "Serial") {

            if (_om == nullptr) {
                wxASSERT(false);
                return -1;
            }

            // need to offset the first DMX channel
            auto abs = GetFirstModel()->GetStartChannel();
            abs = abs - GetFirstModel()->GetDMXChannelOffset() + 1;
            int32_t sc;
            Output* o = _om->GetOutput(abs, sc);
            if (o != nullptr) {
                return o->GetUniverse();
            }
            return -1;
        }
        else {
            return GetFirstModel()->GetUniverse();
        }
    }
}

int UDControllerPort::GetUniverseStartChannel() const {

    if (_models.size() == 0) {
        return -1;
    }
    else {
        if (_type == "Serial") {

            if (_om == nullptr) {
                wxASSERT(false);
                return -1;
            }

            // need to offset the first DMX channel
            auto abs = GetFirstModel()->GetStartChannel();
            abs = abs - GetFirstModel()->GetDMXChannelOffset() + 1;
            int32_t sc;
            Output* o = _om->GetOutput(abs, sc);
            if (o != nullptr) {
                return sc;
            }
            return -1;
        }
        else {
            return GetFirstModel()->GetUniverseStartChannel();
        }
    }
}

bool UDControllerPort::IsPixelProtocol() const {
    return ::IsPixelProtocol(_protocol);
}

float UDControllerPort::GetAmps(int defaultBrightness, int sr) const
{
    float amps = 0.0f;
    int currentBrightness = defaultBrightness;

    if (_type == "Pixel") {
        for (const auto& m : _models) {
            if (m->GetSmartRemote() == sr) {
                currentBrightness = m->GetBrightness(currentBrightness);
                amps += m->GetAmps(currentBrightness);
            }
        }
    }
    return amps;
}

float UDControllerPort::GetAmps(int defaultBrightness) const
{
    float amps = 0.0f;
    int currentBrightness = defaultBrightness;

    if (_type == "Pixel") {
        for (const auto& m : _models) {
            currentBrightness = m->GetBrightness(currentBrightness);
            amps += m->GetAmps(currentBrightness);
        }
    }
    return amps;
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

bool UDControllerPort::AllSmartRemoteTypesSame() const
{
    if(_models.size() < 2) {
        return true;
    }
    
    for (const auto& it : _models) {
        if (it->GetSmartRemoteType() != _models.front()->GetSmartRemoteType()) {
            return false;
        }
    }
    return true;
}

bool UDControllerPort::AllSmartRemoteTypesSame(int smartRemote) const {
    std::string smType;

    for (const auto& it : _models) {
        if (it->GetSmartRemote() == smartRemote) {
            if (smType.empty()) {
                smType = it->GetSmartRemoteType();
            } else if (it->GetSmartRemoteType() != smType) {
                return false;
            }
        }
    }
    return true;
}

std::string UDControllerPort::GetSmartRemoteType(int smartRemote) const {

    for (const auto& it : _models) {
        if (it->GetSmartRemote() == smartRemote) {
            return it->GetSmartRemoteType();
        }
    }
    return "";
}

void UDControllerPort::Dump() const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("            Port %d. Protocol %s. Valid %s. Invalid Reason '%s'. Channels %d. Pixels %d. Start #%d:%d.", _port, (const char*)_protocol.c_str(), toStr( _valid ), (const char*)_invalidReason.c_str(), Channels(),  INTROUNDUPDIV(Channels(), GetChannelsPerPixel(_protocol)), GetUniverse(), GetUniverseStartChannel());
    for (const auto& it : _models) {
        it->Dump();
    }
}

bool UDControllerPort::Check(Controller* c, bool pixel, const ControllerCaps* rules, std::string& res) const {

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
        // max channels is always expressed in terms of 3 channel pixels
        if (Channels() > rules->GetMaxPixelPortChannels()) {
            res += wxString::Format("ERR: Pixel port %d has %d nodes allocated but maximum is %d.\n", _port, (int)Channels() / 3, rules->GetMaxPixelPortChannels() / 3).ToStdString();
            success = false;
        }

        for (int i = 0; i < rules->GetSmartRemoteCount(); ++i) {
            if (!AllSmartRemoteTypesSame(i + 1)) {
                res += wxString::Format("ERR: Pixel port %d has different types of smart remotes types for %s.\n", _port, std::string(1, 'A' + i)).ToStdString();
                success = false;
            }
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
                if (rules->IsValidSerialProtocol(_protocol)) {
                    // we dont warn about serial gaps ... they are normal
                }
                else {
                    res += wxString::Format("WARN: Gap in models on pixel port %d channel %d to %d.\n", _port, ch, it->GetStartChannel()).ToStdString();
                }
            }
            else {
                res += wxString::Format("WARN: Gap in models on pixel port %d smart remote %c channel %d to %d.\n", _port, it->GetSmartRemoteLetter(), ch, it->GetStartChannel()).ToStdString();
            }
        }
        lastSmartRemote = it->GetSmartRemote();
        ch = it->GetEndChannel();
    }

    for (const auto& it : _models) {
        // all models must be fully contained within the outputs on this controller
        success &= it->Check(c, rules, res);
    }

    return success;
}

std::vector<std::string> UDControllerPort::ExportAsCSV(ExportSettings::SETTINGS const& settings, float brightness) const
{
    std::vector<std::string> columns;
    std::string port = wxString::Format("%s Port %d", _type, _port);
    if ((settings & ExportSettings::SETTINGS_PORT_ABSADDRESS) && GetStartChannel() != -1) {
        port += "(SC:" + std::to_string(GetStartChannel()) + ")";
    }
    if ((settings & ExportSettings::SETTINGS_PORT_UNIADDRESS) && GetUniverse() != -1) {
        port += "(UNI:#" + std::to_string(GetUniverse()) + ":" + std::to_string(GetUniverseStartChannel()) + ")";
    }
    if ((settings & ExportSettings::SETTINGS_PORT_CHANNELS) && Channels() != 0) {
        port += "(CHANS:" + std::to_string(Channels()) + ")";
    }
    if ((settings & ExportSettings::SETTINGS_PORT_PIXELS) && Channels() != 0 && _type != "Serial") {
        port += "(PIX:" + std::to_string( INTROUNDUPDIV(Channels(), GetChannelsPerPixel(GetProtocol()))) + ")";
    }
    if (settings & ExportSettings::SETTINGS_PORT_CURRENT && _type != "Serial" ) {
        port += wxString::Format("(CUR:%0.2fA)", GetAmps(brightness));
    }
    columns.push_back(port);

    for (const auto& it : GetModels()) {
        std::string model;
        if (it->GetSmartRemote() > 0) {
            char remote = it->GetSmartRemoteLetter();
            model += "Remote ";
            model += remote;
            model += ":";
        }
        model += it->GetName();
        if ((settings & ExportSettings::SETTINGS_MODEL_DESCRIPTIONS) && it->GetModel()->description != "") {
            model += "(DESC:" + it->GetModel()->description + ")";
        }
        if (settings & ExportSettings::SETTINGS_MODEL_ABSADDRESS) {
            model += "(SC:" + std::to_string(it->GetStartChannel()) + ")";
        }
        if (settings & ExportSettings::SETTINGS_MODEL_UNIADDRESS) {
            model += "(UNI:#" + std::to_string(it->GetUniverse()) + ":" + std::to_string(it->GetUniverseStartChannel()) + ")";
        }
        if (settings & ExportSettings::SETTINGS_MODEL_CHANNELS) {
            model += "(CHANS:" + std::to_string(it->Channels()) + ")";
        }
        if (settings & ExportSettings::SETTINGS_MODEL_PIXELS && _type != "Serial") {
            model += "(PIX:" + std::to_string(it->Channels() / it->GetChannelsPerPixel()) + ")";
        }
        if (settings & ExportSettings::SETTINGS_MODEL_CURRENT && _type != "Serial") {
            model += wxString::Format("(CUR:%0.2fA)", it->GetAmps(brightness));
        }
        columns.push_back(model);
    }

    return columns;
}

int UDControllerPort::GetSmartRemoteCount() const
{
    int count = 0;
    for (const auto& it : _models)
    {
        count = std::max(count, it->GetSmartRemote());
    }
    return count;
}

std::string UDControllerPort::ExportAsJSON() const
{
    std::string json = "{\"port\":" + std::to_string(_port) + ",\"startchannel\":" + std::to_string(GetStartChannel()) +
    ",\"universe\":" + std::to_string(GetUniverse()) + ",\"universestartchannel\":" + std::to_string(GetUniverseStartChannel()) +
    ",\"channels\":" + std::to_string(Channels());

    if (Channels() != 0 && _type != "Serial") {
        json += ",\"pixels\":";
        json += std::to_string( INTROUNDUPDIV(Channels(), GetChannelsPerPixel(GetProtocol())));
    }
    
    json += ",\"models\":[";
    bool first {true};
    for (const auto& it : GetModels()) {
        if(!first) json += ",";
        json += "{\"name\":\"" + JSONSafe(it->GetName()) + "\"";
        if (it->GetSmartRemote() > 0) {
            json += ",\"smartremote\":\"" + std::to_string(it->GetSmartRemoteLetter()) + "\"";
        }
        json += ",\"description\":\"";
        auto desp = it->GetModel()->description;
        json += JSONSafe(desp);
        json += "\"";
        json += ",\"startchannel\":" + std::to_string(it->GetStartChannel());
        json += ",\"universe\":" + std::to_string(it->GetUniverse());
        json +=  ",\"universestartchannel\":" + std::to_string(it->GetUniverseStartChannel());
        json += ",\"channels\":" + std::to_string(it->Channels());
        
        if (_type != "Serial") {
            json += ",\"pixels\":" + std::to_string(it->Channels() / it->GetChannelsPerPixel());
        }
        json += "}";
        first = false;
    }
    json += "]}";
    return json;
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
    for (const auto& it : _virtualMatrixPorts) {
        if (it.second->ContainsModel(m, string)) {
            return true;
        }
    }
    for (const auto& it : _ledPanelMatrixPorts) {
        if (it.second->ContainsModel(m, string)) {
            return true;
        }
    }
    return false;
}
#pragma endregion

#pragma region Constructors and Destructors
UDController::UDController(Controller* controller, OutputManager* om, ModelManager* mm, bool eliminateOverlaps) :
    _controller(controller), _outputManager(om), _modelManager(mm) {

    _ipAddress = _controller->GetColumn2Label();

    Rescan(eliminateOverlaps);
}

UDController::~UDController() {
    ClearPorts();
}
void UDController::ClearPorts() {
    for (const auto& it : _pixelPorts) {
        delete it.second;
    }
    _pixelPorts.clear();

    for (const auto& it : _serialPorts) {
        delete it.second;
    }
    _serialPorts.clear();

    for (const auto& it : _virtualMatrixPorts) {
        delete it.second;
    }
    _virtualMatrixPorts.clear();

    for (const auto& it : _ledPanelMatrixPorts) {
        delete it.second;
    }
    _ledPanelMatrixPorts.clear();
}

void UDController::Rescan(bool eliminateOverlaps) {
    ClearPorts();

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
                } else {
                    // model uses channels in this universe
                    if (it.second->IsPixelProtocol()) {
                        int strings = it.second->GetNumPhysicalStrings();
                        if (strings == 1) {
                            int port = it.second->GetControllerPort(1);
                            GetControllerPixelPort(port)->AddModel(it.second, _controller, _outputManager, -1, eliminateOverlaps);
                        } else {
                            for (int i = 0; i < strings; i++) {
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
                    } else if (it.second->IsVirtualMatrixProtocol()) {
                        int port = it.second->GetControllerPort(1);
                        GetControllerVirtualMatrixPort(port)->AddModel(it.second, _controller, _outputManager, -1, eliminateOverlaps);
                    } else if (it.second->IsLEDPanelMatrixProtocol()) {
                        int port = it.second->GetControllerPort(1);
                        GetControllerLEDPanelMatrixPort(port)->AddModel(it.second, _controller, _outputManager, -1, eliminateOverlaps);
                    } else if (it.second->IsSerialProtocol()) {
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

int UDController::GetSmartRemoteCount(int port)
{
    int count = 0;
    int basePort = ((port - 1) / 4) * 4;
    for (int p = basePort; p < basePort + 4; ++p)
    {
        auto pp = GetControllerPixelPort(p + 1);
        count = std::max(count, pp->GetSmartRemoteCount());
    }
    return count;
}

UDControllerPort* UDController::GetControllerSerialPort(int port) {
    if (!HasSerialPort(port)) {
        _serialPorts[port] = new UDControllerPort("Serial", port);
    }
    return _serialPorts[port];
}
UDControllerPort* UDController::GetControllerVirtualMatrixPort(int port) {
    if (_virtualMatrixPorts.find(port) == _virtualMatrixPorts.end()) {
        _virtualMatrixPorts[port] = new UDControllerPort("Virtual Matrix", port);
    }
    return _virtualMatrixPorts[port];
}
UDControllerPort* UDController::GetControllerLEDPanelMatrixPort(int port) {
    if (_ledPanelMatrixPorts.find(port) == _ledPanelMatrixPorts.end()) {
        _ledPanelMatrixPorts[port] = new UDControllerPort("LED Panel", port);
    }
    return _ledPanelMatrixPorts[port];
}

UDControllerPort* UDController::GetPortContainingModel(Model* model) const
{
    for (const auto& it : _pixelPorts) {
        if (it.second->ContainsModel(model, 0)) return it.second;
    }
    for (const auto& it : _serialPorts) {
        if (it.second->ContainsModel(model, 0)) return it.second;
    }
    for (const auto& it : _virtualMatrixPorts) {
        if (it.second->ContainsModel(model, 0)) return it.second;
    }
    for (const auto& it : _ledPanelMatrixPorts) {
        if (it.second->ContainsModel(model, 0)) return it.second;
    }
    return nullptr;
}

UDControllerPortModel* UDController::GetControllerPortModel(const std::string& modelName, int str) const
{
    for (const auto& it : _pixelPorts) {
        auto m = it.second->GetModel(modelName, str);
        if (m != nullptr) return m;
    }
    for (const auto& it : _serialPorts) {
        auto m = it.second->GetModel(modelName, str);
        if (m != nullptr) return m;
    }
    for (const auto& it : _virtualMatrixPorts) {
        auto m = it.second->GetModel(modelName, str);
        if (m != nullptr) return m;
    }
    for (const auto& it : _ledPanelMatrixPorts) {
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
int UDController::GetMaxLEDPanelMatrixPort() const {
    int last = 0;
    for (const auto& it : _ledPanelMatrixPorts) {
        if (it.second->GetPort() > last) {
            last = it.second->GetPort();
        }
    }
    return last;
}
int UDController::GetMaxVirtualMatrixPort() const {
    int last = 0;
    for (const auto& it : _virtualMatrixPorts) {
        if (it.second->GetPort() > last) {
            last = it.second->GetPort();
        }
    }
    return last;
}

void UDController::TagSmartRemotePorts()
{
    for (const auto& it : _pixelPorts) {
        if (it.second->AtLeastOneModelIsUsingSmartRemote()) {
            for (uint32_t i = ((it.first-1) / 4) * 4 + 1; i < ((it.first-1) / 4) * 4 + 5; ++i) {
                GetControllerPixelPort(i)->TagSmartRemotePort();
            }
        }
    }
}

bool UDController::HasSerialPort(int port) const {
    for (const auto& it : _serialPorts) {
        if (it.second->GetPort() == port) {
            return true;
        }
    }
    return false;
}
bool UDController::HasLEDPanelMatrixPort(int port) const {
    for (const auto& it : _ledPanelMatrixPorts) {
        if (it.second->GetPort() == port) {
            return true;
        }
    }
    return false;
}bool UDController::HasVirtualMatrixPort(int port) const {
    for (const auto& it : _virtualMatrixPorts) {
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
    for (const auto& it : _pixelPorts) {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    for (const auto& it : _serialPorts) {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    for (const auto& it : _virtualMatrixPorts) {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    for (const auto& it : _ledPanelMatrixPorts) {
        auto mm = it.second->GetModelAfter(m);
        if (mm != nullptr) return mm;
    }
    return nullptr;
}

bool UDController::HasModels() const
{
    for (const auto& it : _pixelPorts) {
        if (it.second->GetFirstModel() != nullptr) {
            return true;
        }
    }
    for (const auto& it : _serialPorts) {
        if (it.second->GetFirstModel() != nullptr) {
            return true;
        }
    }
    for (const auto& it : _virtualMatrixPorts) {
        if (it.second->GetFirstModel() != nullptr) {
            return true;
        }
    }
    for (const auto& it : _ledPanelMatrixPorts) {
        if (it.second->GetFirstModel() != nullptr) {
            return true;
        }
    }

    return false;
}

bool UDController::SetAllModelsToControllerName(const std::string& controllerName)
{
    bool changed = false;
    for (const auto& it : _pixelPorts) {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        changed |= it.second->EnsureAllModelsAreChained();
    }

    for (const auto& it : _serialPorts) {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        // we dont chain serial models
        //changed |= it.second->EnsureAllModelsAreChained();
    }
    for (const auto& it : _virtualMatrixPorts) {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        //changed |= it.second->EnsureAllModelsAreChained();
    }
    for (const auto& it : _ledPanelMatrixPorts) {
        changed |= it.second->SetAllModelsToControllerName(controllerName);
        //changed |= it.second->EnsureAllModelsAreChained();
    }

    return changed;
}

bool UDController::SetAllModelsToValidProtocols(const std::vector<std::string>& pixelProtocols, const std::vector<std::string>& serialProtocols, bool allsame)
{
    std::string force;
    bool changed = false;
    for (const auto& it : _pixelPorts) {
        changed |= it.second->SetAllModelsToValidProtocols(pixelProtocols, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr) {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }

    force = "";
    for (const auto& it : _serialPorts) {
        changed |= it.second->SetAllModelsToValidProtocols(serialProtocols, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr) {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }

    force = "";
    for (const auto& it : _virtualMatrixPorts) {
        std::vector<std::string> vmProtocol = { "Virtual Matrix" };
        changed |= it.second->SetAllModelsToValidProtocols(vmProtocol, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr) {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }
    force = "";
    for (const auto& it : _ledPanelMatrixPorts) {
        std::vector<std::string> vmProtocol = { "LED Panel Matrix" };
        changed |= it.second->SetAllModelsToValidProtocols(vmProtocol, force);
        if (allsame && force == "" && it.second->GetFirstModel() != nullptr) {
            force = it.second->GetFirstModel()->GetModel()->GetControllerProtocol();
        }
    }

    return changed;
}

bool UDController::ClearSmartRemoteOnAllModels()
{
    bool changed = false;
    for (const auto& it : _pixelPorts) {
        changed |= it.second->ClearSmartRemoteOnAllModels();
    }
    for (const auto& it : _serialPorts) {
        changed |= it.second->ClearSmartRemoteOnAllModels();
    }
    for (const auto& it : _virtualMatrixPorts) {
        changed |= it.second->ClearSmartRemoteOnAllModels();
    }
    for (const auto& it : _ledPanelMatrixPorts) {
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

bool UDController::IsValid() const {

    for (const auto& it : _pixelPorts) {
        if (!it.second->IsValid()) return false;
    }
    for (const auto& it : _serialPorts) {
        if (!it.second->IsValid()) return false;
    }
    for (const auto& it : _virtualMatrixPorts) {
        if (!it.second->IsValid()) return false;
    }
    for (const auto& it : _ledPanelMatrixPorts) {
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
    logger_base.debug("   Virtual Matrices %d.", (int)_virtualMatrixPorts.size());
    for (const auto& it : _virtualMatrixPorts) {
        it.second->Dump();
    }
    logger_base.debug("   LED Panel Matrices %d.", (int)_ledPanelMatrixPorts.size());
    for (const auto& it : _ledPanelMatrixPorts) {
        it.second->Dump();
    }
}

bool UDController::IsError(const std::string& check)
{
    return Contains(check, "ERR:");
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
        std::vector<int> blocksAreSmart;
        blocksAreSmart.resize(100); // just add a lot ... lazy but i cant rely on may pixel port for the controller
        for (auto& it : blocksAreSmart) {
            it = 0;
        }
        for (const auto& it : _pixelPorts) {

            int block = (it.first - 1) / 4;
            blocksAreSmart[block] += it.second->AtLeastOneModelIsUsingSmartRemote() ? 1 : 0;

            if (rules->SupportsVirtualStrings()) {
                it.second->CreateVirtualStrings(rules->MergeConsecutiveVirtualStrings());
            }

            if (rules->SupportsUniversePerString() && _controller->GetType() == CONTROLLER_ETHERNET) {
                it.second->SetSeparateUniverses(((ControllerEthernet*)_controller)->IsUniversePerString());
            }

            success &= it.second->Check(_controller, true, rules, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxPixelPort()) {
                res += wxString::Format("ERR: Pixel port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxPixelPort()).ToStdString();
                success = false;
            }
        }

        int prevBlock{-1};
        std::vector<std::string> smType;
        // try to detect errors with a mix of smart remote and non smart remote ports/models
        for (const auto& it : _pixelPorts) {

            int block = (it.first - 1) / 4;
            
            if (prevBlock != block) {
                smType = std::vector<std::string>(rules->GetSmartRemoteCount(), "");
                prevBlock = block;
            }

            if (blocksAreSmart[block] > 0) {
                if (it.second->AtLeastOneModelIsNotUsingSmartRemote()){
                    res += wxString::Format("ERR: Pixel port %d has a model configured not on a smart remote but this block of 4 ports has at least one model that is configured as on a smart remote.\n", it.second->GetPort()).ToStdString();
                    success = false;
                }

                if (rules->AllSmartRemoteTypesPerPortMustBeSame()) {
                    for (int i = 0; i < rules->GetSmartRemoteCount(); ++i) {
                        if (smType[0].empty()) {
                            smType[0] = it.second->GetSmartRemoteType(i + 1);
                        } else if (it.second->GetSmartRemoteType(i + 1) != smType[0] && !it.second->GetSmartRemoteType(i + 1).empty()) {
                            res += wxString::Format("ERR: Pixel Port:%d SRID:%s has different smart remotes types for this block of 4 ports.\n", it.second->GetPort(), std::string(1, 'A' + i)).ToStdString();
                            success = false;
                        }
                    }
                } else {
                    for (int i = 0; i < rules->GetSmartRemoteCount(); ++i) {
                        if (smType[i].empty()) {
                            smType[i] = it.second->GetSmartRemoteType(i + 1);
                        } else if (it.second->GetSmartRemoteType(i + 1) != smType[i] && !it.second->GetSmartRemoteType(i + 1).empty()) {
                            res += wxString::Format("ERR: Pixel Port:%d SRID:%s has different smart remotes types for this block of 4 ports.\n", it.second->GetPort(), std::string(1, 'A' + i)).ToStdString();
                            success = false;
                        }
                    }
                }
            }
        }

        if(rules->AllSmartRemoteTypesPerPortMustBeSame()) {
            for (const auto& it : _pixelPorts) {
                if (!it.second->AllSmartRemoteTypesSame()){
                    res += wxString::Format("ERR: Pixel port %d has a models not configured all as the same smart remote type.\n", it.second->GetPort()).ToStdString();
                    success = false;
                }
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
        if (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_KINET) {
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

            if (Lower(protocol) != Lower(it.second->GetProtocol())) {
                res += wxString::Format("ERR: Pixel ports only support a single protocol at a time. %s and %s found.\n", protocol, it.second->GetProtocol()).ToStdString();
                success = false;
            }
        }

        protocol = "";
        for (const auto& it : _serialPorts) {
            if (protocol == "") protocol = it.second->GetProtocol();

            if (Lower(protocol) != Lower(it.second->GetProtocol())) {
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
    } else {
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

    if (rules->AllInputUniversesMustBe510()) {
        for (const auto& it : outputs) {
            if (it->GetChannels() != 510) {
                res += wxString::Format("ERR: All universes must be 510 channels. %d found.\n", (int)it->GetChannels()).ToStdString();
                success = false;
            }
        }
    }

    if (rules->GetNumberOfBanks() > 1) {
        if (_pixelPorts.size() <= rules->GetMaxPixelPort()) {//dont do bank checking if pixel port count is over the max of the controller, it isnt going to work anyways
            int const banksize = rules->GetBankSize();
            std::vector<int> bankSizes(rules->GetNumberOfBanks(), 0);
            std::vector<int> bankLargestPort(rules->GetNumberOfBanks(), 0);

            for (const auto& it : _pixelPorts) {
                int const bank = (it.second->GetPort() - 1) / banksize;
                if (it.second->Channels() > bankSizes[bank]) {
                    bankSizes[bank] = it.second->Channels();
                    bankLargestPort[bank] = it.second->GetPort();
                }
            }

            int const sum = std::accumulate(bankSizes.begin(), bankSizes.end(), 0);
            if (sum > rules->GetMaxPixelPortChannels()) {
                // always expressed in terms of 3 channel pixels
                res += wxString::Format("ERR: Controllers 'Bank' channel count [%d (%d)] is over the maximum [%d (%d)].\n", sum, sum / 3, rules->GetMaxPixelPortChannels(), rules->GetMaxPixelPortChannels() / 3).ToStdString();
                res += "     Largest ports on banks: ";
                for (int i = 0; i < rules->GetNumberOfBanks(); i++) {
                    if (i != 0) res += ", ";
                    res += wxString::Format(" Bank %d - Port %d [%d (%d)]", i + 1, bankLargestPort[i], bankSizes[i], bankSizes[i] / 3);
                }
                res += "\n";
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
    } else {
        for (const auto& it : _serialPorts) {
            success &= it.second->Check(_controller, false, rules, res);

            if (it.second->GetPort() < 1 || it.second->GetPort() > rules->GetMaxSerialPort()) {
                res += wxString::Format("ERR: Serial port %d is not valid on this controller. Valid ports %d-%d.\n", it.second->GetPort(), 1, rules->GetMaxSerialPort()).ToStdString();
                success = false;
            }
        }
    }
    if (!rules->SupportsVirtualMatrix() && _virtualMatrixPorts.size() > 0) {
        res += wxString::Format("ERR: Attempt to upload Virtual Matrix but this controller does not support Virtual Matrices.\n");
        success = false;
    }
    if (!rules->SupportsLEDPanelMatrix() && _ledPanelMatrixPorts.size() > 0) {
        res += wxString::Format("ERR: Attempt to upload LED Panel Matrix but this controller does not support LED Panel Matrices.\n");
        success = false;
    }

    return success;
}

std::vector<std::vector<std::string>> UDController::ExportAsCSV(ExportSettings::SETTINGS const& settings, float brightness, int& columnSize)
{
    std::vector<std::vector<std::string>> lines;
    columnSize = 0;

    for (int i = 1; i <= GetMaxPixelPort(); i++) {
        if (columnSize < GetControllerPixelPort(i)->GetModels().size())
            columnSize = GetControllerPixelPort(i)->GetModels().size();
        lines.push_back(GetControllerPixelPort(i)->ExportAsCSV(settings, brightness));
	}
	for (int i = 1; i <= GetMaxSerialPort(); i++) {
        if (columnSize < GetControllerSerialPort(i)->GetModels().size())
            columnSize = GetControllerSerialPort(i)->GetModels().size();

        lines.push_back(GetControllerSerialPort(i)->ExportAsCSV(settings, brightness));
    }

    for (auto &vm : _virtualMatrixPorts) {
        if (columnSize < vm.second->GetModels().size())
            columnSize = vm.second->GetModels().size();
        lines.push_back(vm.second->ExportAsCSV(settings, brightness));
    }

    for (auto &vm : _ledPanelMatrixPorts) {
        if (columnSize < vm.second->GetModels().size())
            columnSize = vm.second->GetModels().size();
        lines.push_back(vm.second->ExportAsCSV(settings, brightness));
    }

    std::vector<std::string> header({ "Output" });
    for (int i = 1; i <= columnSize; i++) {
        header.push_back( wxString::Format("Model %d", i));
    }

    lines.insert(lines.begin(), header);
    return lines;
}

std::string UDController::ExportAsJSON()
{
    std::string json = "{\"pixelports\": [" ;
    for (int i = 1; i <= GetMaxPixelPort(); i++) {
        if(i != 1) json += ",";
        json += GetControllerPixelPort(i)->ExportAsJSON();
    }
    json += "], \"serialports\": [" ;
    for (int i = 1; i <= GetMaxSerialPort(); i++) {
        if(i != 1) json += ",";
        json += GetControllerSerialPort(i)->ExportAsJSON();
    }
    json += "], \"virtualmatrixports\": [" ;
    for (auto &vm : _virtualMatrixPorts) {
        if(vm.first != 1) json += ",";
        json += vm.second->ExportAsJSON();
    }
    json += "], \"ledpanelmatrixports\": [" ;
    for (auto &vm : _ledPanelMatrixPorts) {
        if(vm.first != 1) json += ",";
        json += vm.second->ExportAsJSON();
    }
    json += "]}" ;
    return json;
}

Output* UDController::GetFirstOutput() const {

    return _controller->GetFirstOutput();
}
#pragma endregion

#pragma endregion
