/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <chrono>
#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <string_view>
#include <regex>
#include <pugixml.hpp>

#include "CustomModel.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ModelManager.h"
#include "ModelScreenLocation.h"
#include "RulerObject.h"
#include "SubModel.h"
#include "Color.h"
#include "../render/DimmingCurve.h"
#include "utils/ExternalHooks.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "Pixels.h"
#include "UtilFunctions.h"
#include "../controllers/ControllerCaps.h"
#include "../outputs/Controller.h"
#include "../outputs/ControllerSerial.h"
#include "../outputs/IPOutput.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../utils/ip_utils.h"
#include "../utils/NodeUtils.h"
#include "../render/RenderContext.h"
#include "../utils/TraceLog.h"
#include "xLightsVersion.h"
#include "../render/SequenceFile.h"
#include "../XmlSerializer/FileSerializingVisitor.h"
#include "../XmlSerializer/XmlSerializer.h"

#include <log.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../render/UICallbacks.h"
#include "../utils/AppCallbacks.h"
#include "../utils/string_utils.h"

#define MOST_STRINGS_WE_EXPECT 480
#define MOST_CONTROLLER_PORTS_WE_EXPECT 128

const long Model::ID_LAYERSIZE_INSERT = 20000;
const long Model::ID_LAYERSIZE_DELETE = 20001;

static const std::vector<std::string> RGBW_HANDLING = { "R=G=B -> W", "RGB Only", "White Only", "Advanced", "White On All" };

static const std::vector<std::string> PIXEL_STYLES = { "Square", "Smooth", "Solid Circle", "Blended Circle" };

static const char* CONTROLLER_COLORORDER_VALUES[] = {
    "RGB", "RBG", "GBR", "GRB", "BRG", "BGR",
    "RGBW", "RBGW", "GBRW", "GRBW", "BRGW", "BGRW",
    "WRGB", "WRBG", "WGBR", "WGRB", "WBRG", "WBGR"
};
std::vector<std::string> Model::CONTROLLER_COLORORDER(CONTROLLER_COLORORDER_VALUES, CONTROLLER_COLORORDER_VALUES + 18);

static const std::string DEFAULT("Default");
static const std::string PER_PREVIEW("Per Preview");
static const std::string SINGLE_LINE("Single Line");
static const std::string AS_PIXEL("As Pixel");
static const std::string VERT_PER_STRAND("Vertical Per Strand");
static const std::string HORIZ_PER_STRAND("Horizontal Per Strand");
static const std::string HORIZ_PER_MODELSTRAND("Horizontal Per Model/Strand");
static const std::string VERT_PER_MODELSTRAND("Vertical Per Model/Strand");

static const std::string PER_PREVIEW_NO_OFFSET("Per Preview No Offset");

const std::vector<std::string> Model::DEFAULT_BUFFER_STYLES{ DEFAULT, PER_PREVIEW, SINGLE_LINE, AS_PIXEL };

static const std::string EFFECT_PREVIEW_CACHE("ModelPreviewEffectCache");
static const std::string MODEL_PREVIEW_CACHE_2D("ModelPreviewCache2D");
static const std::string MODEL_PREVIEW_CACHE_3D("ModelPreviewCache3D");
static const std::string LAYOUT_PREVIEW_CACHE_2D("LayoutPreviewCache2D");
static const std::string LAYOUT_PREVIEW_CACHE_3D("LayoutPreviewCache3D");

Model::Model(const ModelManager& manager)
: modelManager(manager),
  _controllerConnection(this)
{
}

Model::~Model()
{
    RemoveAllSubModels();
    deleteUIObjects();
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
    }
}


std::map<std::string, std::map<std::string, std::string>> Model::GetDimmingInfo() const
{
    return dimmingInfo;
}

void Model::SetDimmingInfo(const std::map<std::string, std::map<std::string, std::string>>& info)
{
    dimmingInfo = info;
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
        modelDimmingCurve = nullptr;
    }
    if (!dimmingInfo.empty()) {
        modelDimmingCurve = DimmingCurve::createFromInfo(dimmingInfo);
    }
}

std::vector<std::string> Model::GetLayoutGroups(const ModelManager& mm)
{
    std::vector<std::string> lg;
    lg.push_back("Default");
    lg.push_back("All Previews");
    lg.push_back("Unassigned");

    for (const auto& name : mm.GetLayoutGroupNames()) {
        lg.push_back(name);
    }

    return lg;
}

void Model::SetControllerName(const std::string& controller, bool skip_work)
{
    auto n = Trim(controller);
    if (n == USE_START_CHANNEL) {
        n = "";
    }
    if (n == _controllerName) return;
    _controllerName = n;

    // if we are moving the model to no controller then clear the start channel and model chain
    if (_controllerName == NO_CONTROLLER) {
        SetStartChannel("");
        SetModelChain("");
        _controllerConnection.SetCtrlPort(0);
    }

    if (!skip_work) {
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                    OutputModelManager::WORK_UPDATE_PROPERTYGRID |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "ControllerConnection::SetName");
    }
    IncrementChangeCount();
}

bool Model::RenameController(const std::string& oldName, const std::string& newName)
{
    assert(newName != "");

    bool changed = false;

    if (_controllerName == oldName) {
        SetControllerName(newName, false);
        changed = true;
    }
    if (StartsWith(ModelStartChannel, "!" + oldName)) {
        SetStartChannel("!" + newName + ModelStartChannel.substr(oldName.size() + 1));
        changed = true;
    }
    return changed;
}


void Model::Rename(std::string const& newName)
{
    auto oldname = GetName();
    name = Trim(newName);
    std::string aliasBehavior;
    if (auto* ui = GetModelManager().GetUICallbacks()) {
        aliasBehavior = ui->GetRenameModelAliasPromptBehavior();
    }
    bool shouldPrompt = aliasBehavior == "Always Prompt" &&
        oldname != modelManager.GetLastGeneratedModelName() ;

    if (oldname != "" && newName != "Iamgoingtodeletethismodel" && shouldPrompt ) {
        if (auto* ui = GetModelManager().GetUICallbacks()) {
            if (ui->PromptYesNo("Would you like to save the old name as an alias for this prop. This could be useful if you have sequences already sequenced against this prop using the old name.", "Save old name as alias")) {
                AddAlias("oldname:" + oldname);
            }
        }
    }

    if( aliasBehavior == "Always Yes" ) {
        AddAlias("oldname:" + oldname);
    }
}

void Model::SetStartChannel(std::string const& startChannel)
{
    if (startChannel == ModelStartChannel) return;

    ModelStartChannel = startChannel;
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetStartChannel");
    IncrementChangeCount();
}

void Model::ClearIndividualStartChannels()
{
    _hasIndivChans = false;
    _indivStartChannels.clear();
}

void Model::GetSerialProtocolSpeeds(const std::string& protocol, std::vector<std::string>& cs, int& idxs) const {
    if (protocol == "dmx" || Contains(protocol, "DMX")) {
        cs.push_back("250000");
    } else if (protocol == "renard" || protocol == "Renard") {
        cs.push_back("19200");
        cs.push_back("38400");
        cs.push_back("57600");
        cs.push_back("115200");
    } else if (protocol == "lor" || protocol == "LOR") {
        cs.push_back("19200");
        cs.push_back("57600");
        cs.push_back("115200");
        cs.push_back("500000");
        cs.push_back("1000000");
    } else {
        cs.push_back("19200");
        cs.push_back("57600");
        cs.push_back("115200");
        cs.push_back("250000");
        cs.push_back("500000");
        cs.push_back("1000000");
    }

    int protocolSpeed = GetControllerProtocolSpeed();

    // now work out the index
    int i = 0;
    for (const auto& it : cs) {
        if ((int)std::strtol(it.c_str(), nullptr, 10) == protocolSpeed) {
            idxs = i;
            break;
        }
        ++i;
    }
}

void Model::GetControllerProtocols(std::vector<std::string>& cp, int& idx)
{
    auto caps = GetControllerCaps();
    Controller* c = GetController();
    std::string protocol = GetControllerProtocol();
    if (c) {
        ControllerSerial* cs = dynamic_cast<ControllerSerial*>(c);
        if (cs) {
            cp.push_back(cs->GetProtocol());
            idx = 0;
            return;
        }
    }
    std::string protocolLC = Lower(protocol);

    if (caps == nullptr) {
        for (const auto& it : GetAllPixelTypes(true, false)) {
            cp.push_back(it);
        }
    } else {
        auto controllerProtocols = caps->GetAllProtocols();
        for (const auto& it : GetAllPixelTypes(controllerProtocols, true, true, true)) {
            cp.push_back(it);
        }
    }

    // if this protocol is not supported by the controller ... choose a compatible one if one exists ... otherwise we blank it out
    if (std::find(begin(cp), end(cp), protocol) == end(cp) && std::find(begin(cp), end(cp), protocolLC) == end(cp)) {
        // not in the list ... maybe there is a compatible protocol we can choose
        std::string np = "";
        if (caps != nullptr) {
            auto controllerProtocols = caps->GetAllProtocols();
            if (::IsPixelProtocol(protocol)) {
                np = ChooseBestControllerPixel(controllerProtocols, protocol);
            } else {
                np = ChooseBestControllerSerial(controllerProtocols, protocol);
            }
        }
        if (protocol != np)
            SetControllerProtocol(np);
    }

    // now work out the index
    int i = 0;
    for (const auto& it : cp) {
        if (it == protocol || it == protocolLC) {
            idx = i;
            break;
        }
        i++;
    }
}

std::vector<std::string> Model::GetSmartRemoteValues(int smartRemoteCount) const
{
    std::vector<std::string> res;
    auto caps = GetControllerCaps();
    bool hinkspix = (caps && caps->GetVendor() == "HinksPix");

    for (int i = 0; i < smartRemoteCount; ++i) {
        if (hinkspix) {
            res.push_back(std::to_string(i));
        } else {
            res.push_back(std::string(1, char(65 + i)));
        }
    }
    return res;
}

uint32_t Model::ApplyLowDefinition(uint32_t val) const
{
    if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().IsLowDefinitionRender())
        return val;
    return (val * _lowDefFactor) / 100;
}

std::string Model::GetPixelStyleDescription(PIXEL_STYLE pixelStyle)
{
    if ((int)pixelStyle < (int)PIXEL_STYLES.size()) {
        return PIXEL_STYLES[(int)pixelStyle];
    }
    return "";
}

int Model::GetNumPhysicalStrings() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return GetNumStrings();
    } else {
        int strings = GetNumStrings() / ts;
        if (strings == 0)
            strings = 1;
        return strings;
    }
}

ControllerCaps* Model::GetControllerCaps() const
{
    auto c = GetController();
    if (c == nullptr)
        return nullptr;
    return c->GetControllerCaps();
}

Controller* Model::GetController() const
{
    std::string controller = GetControllerName();
    if (controller == "") {
        if (StartsWith(ModelStartChannel, "!") && Contains(ModelStartChannel, ":")) {
            controller = Trim(BeforeFirst(AfterFirst(ModelStartChannel, '!'), ':'));
        }
    }
    if (controller == "") {
        int32_t start;
        Controller* cp = modelManager.GetOutputManager()->GetController(GetFirstChannel() + 1, start);
        if (cp != nullptr) {
            return cp;
        }
    }
    if (controller == "")
        return nullptr;

    return modelManager.GetOutputManager()->GetController(controller);
}

bool Model::IsAlias(const std::string& alias, bool oldnameOnly) const
{
    std::string lowAlias = Lower(alias);
    std::string oldLowAlias = Lower("oldname:" + alias);
    for (auto &a : GetAliases()) {
        if ((!oldnameOnly && lowAlias == a) || oldLowAlias == a) {
            return true;
        }
    }
    return false;
}

void Model::AddAlias(const std::string& alias)
{
    if (IsAlias(alias))
        return;

    std::string lAlias = ::Lower(alias);
    // a model name cant be its own alias
    if (lAlias == Lower(Name()))
        return;

    aliases.emplace_back(lAlias);
    IncrementChangeCount();
}

void Model::DeleteAlias(const std::string& alias)
{
    std::string lAlias = ::Lower(alias);
    while (IsAlias(alias)) { // while should not be required ... but just in case it ever ends up there more than once
        IncrementChangeCount();
        aliases.remove(lAlias);
    }
}

bool Model::DeleteAllAliases() {
    bool changed = aliases.size() > 0;
    aliases.clear();
    return changed;
}

const std::list<std::string> &Model::GetAliases() const
{
    return aliases;
}

void Model::SetAliases(const std::list<std::string>& aliases)
{
    AddAlias("dummy"); // this ensures the owning element exists
    this->aliases.clear();
    for (const auto& it : aliases) {
        this->aliases.emplace_back(::Lower(it));
    }
    IncrementChangeCount();
}


std::string Model::GetIndividualStartChannel(size_t s) const
{
    if (s >= _indivStartChannels.size() ) return xlEMPTY_STRING;
    return _indivStartChannels[s];
}


void Model::WriteFaceInfo(pugi::xml_node rootXml, const FaceStateData& faceInfo) {
    if (!faceInfo.empty()) {
        for (const auto& it : faceInfo) {
            pugi::xml_node f = rootXml.prepend_child("faceInfo");
            std::string name = it.first;
            f.append_attribute("Name") = name;
            for (const auto& it2 : it.second) {
                f.append_attribute(it2.first) = it2.second;
            }
        }
    }
}

void Model::AddSubmodel(SubModel* sm)
{
    subModels.push_back(sm);
    sortedSubModels[sm->GetName()] = sm;
}

std::string Model::SerialiseFace() const
{
    std::string res;

    if (!faceInfo.empty()) {
        for (const auto& it : faceInfo) {
            res += "    <faceInfo Name=\"" + it.first + "\" ";
            for (const auto& it2 : it.second) {
                res += it2.first + "=\"" + it2.second + "\" ";
            }
            res += "/>\n";
        }
        Replace(res, "&", "&amp;");
    }

    return res;
}

void Model::UpdateFaceInfoNodes()
{
    faceInfoNodes.clear();
    for (const auto& it : faceInfo) {
        if (faceInfo[it.first]["Type"] == "NodeRange") {
            for (const auto& it2 : it.second) {
                if (it2.first != "Type" && !Contains(it2.first, "Color") && it2.second != "") {
                    std::list<int> nodes;
                    std::string_view sv(it2.second);
                    size_t pos = 0;
                    while (pos != std::string::npos && pos <= sv.size()) {
                        size_t next = sv.find(',', pos);
                        std::string_view valstr = sv.substr(pos, next == std::string::npos ? next : next - pos);
                        pos = (next == std::string::npos) ? next : next + 1;

                        int start, end;
                        auto dashpos = valstr.find('-');
                        if (dashpos != std::string_view::npos) {
                            start = (int)std::strtol(std::string(valstr.substr(0, dashpos)).c_str(), nullptr, 10);
                            end = (int)std::strtol(std::string(valstr.substr(dashpos + 1)).c_str(), nullptr, 10);
                            if (end < start) {
                                std::swap(start, end);
                            }
                        } else {
                            start = end = (int)std::strtol(std::string(valstr).c_str(), nullptr, 10);
                        }
                        if (start > end) {
                            start = end;
                        }
                        start--;
                        end--;
                        for (int n = start; n <= end; ++n) {
                            if (n >= 0) {
                                nodes.push_back(n);
                            }
                        }
                    }
                    faceInfoNodes[it.first][it2.first] = nodes;
                }
            }
        }
    }
}

void Model::UpdateStateInfoNodes()
{
    stateInfoNodes.clear();
    for (const auto& it : stateInfo) {
        if (stateInfo[it.first]["Type"] == "NodeRange") {
            for (const auto& it2 : it.second) {
                if (it2.first != "Type" && !Contains(it2.first, "Color") && it2.second != "") {
                    std::list<int> nodes;
                    std::string_view sv(it2.second);
                    size_t pos = 0;
                    while (pos != std::string::npos && pos <= sv.size()) {
                        size_t next = sv.find(',', pos);
                        std::string_view valstr = sv.substr(pos, next == std::string::npos ? next : next - pos);
                        pos = (next == std::string::npos) ? next : next + 1;

                        int start, end;
                        auto dashpos = valstr.find('-');
                        if (dashpos != std::string_view::npos) {
                            start = (int)std::strtol(std::string(valstr.substr(0, dashpos)).c_str(), nullptr, 10);
                            end = (int)std::strtol(std::string(valstr.substr(dashpos + 1)).c_str(), nullptr, 10);
                            if (end < start) {
                                std::swap(start, end);
                            }
                        } else {
                            start = end = (int)std::strtol(std::string(valstr).c_str(), nullptr, 10);
                        }
                        if (start > end) {
                            start = end;
                        }
                        start--;
                        end--;
                        for (int n = start; n <= end; ++n) {
                            if (n >= 0) {
                                nodes.push_back(n);
                            }
                        }
                    }
                    stateInfoNodes[it.first][it2.first] = nodes;
                }
            }
        }
    }
}

void Model::WriteStateInfo(pugi::xml_node rootXml, const FaceStateData& stateInfo, bool forceCustom) {
    if (!stateInfo.empty()) {
        for (const auto& it : stateInfo) {
            std::string name = it.first;
            if (!Trim(name).empty()) {
                pugi::xml_node f = rootXml.prepend_child("stateInfo");
                f.append_attribute("Name") = name;
                if (forceCustom) {
                    f.append_attribute("CustomColors") = "1";
                }
                for (const auto& it2 : it.second) {
                    if (!Trim(it2.first).empty())
                        f.append_attribute(it2.first) = it2.second;
                }
            }
        }
    }
}

std::string Model::SerialiseState() const
{
    std::string res;

    if (!stateInfo.empty()) {
        for (const auto& it : stateInfo) {
            res += "    <stateInfo Name=\"" + it.first + "\" ";
            for (const auto& it2 : it.second) {
                if (!Trim(it2.first).empty()) {
                    res += it2.first + "=\"" + it2.second + "\" ";
                }
            }
            res += "/>\n";
        }
        Replace(res, "&", "&amp;");
    }

    return res;
}

void Model::AddModelGroups(pugi::xml_node n, const std::string& name, bool& merge, bool& ask)
{
    std::string grpModels = n.attribute("models").as_string("");
    if (grpModels.empty())
        return;

    const_cast<ModelManager&>(modelManager).AddModelGroups(n, name, merge, ask);
}

void Model::ImportExtraModels(pugi::xml_node n, ModelManager& modelMgr, const std::string& layoutGroup) {

    int x = GetHcenterPos();
    int y = GetVcenterPos();

    // import the shadow models as well
    for (auto m = n.first_child(); m; m = m.next_sibling()) {
        bool cancelled = false;
        Model* model = modelMgr.CreateDefaultModel("Custom", "1"); // start with a custom model
        model = model->CreateDefaultModelFromSavedModelNode(model, m, modelMgr, cancelled);
        XmlSerializer serializer;
        model = serializer.DeserializeModel(m, modelMgr, true);

        if (!cancelled && model != nullptr) {
            x += 20;
            model->SetLayoutGroup(layoutGroup);
            model->Selected(false);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            model->SetWidth(GetWidth(), true);
            model->SetHeight(GetHeight(), true);
            if (dynamic_cast<BoxedScreenLocation*>(&model->GetModelScreenLocation()) != nullptr) {
                BoxedScreenLocation* sl = dynamic_cast<BoxedScreenLocation*>(&model->GetModelScreenLocation());
                sl->SetScale(1, 1);
            }
            model->SetControllerName(NO_CONTROLLER); // this will force the start channel to a non controller start channel ... then the user can associate them using visualiser
            modelMgr.AddModel(model);
            AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
                        OutputModelManager::WORK_RELOAD_MODELLIST, "Model::ImportExtraModels");
            IncrementChangeCount();
        } else {
            spdlog::error("Unable to import {}. Create failed.", m.name());
        }
    }
}

std::string Model::ComputeStringStartChannel(int i)
{
    if (i == 0) {
        return ModelStartChannel;
    }

    if (i < (int)_indivStartChannels.size()) {
        std::string existingStartChannelAsString = _indivStartChannels[i];
        if (!existingStartChannelAsString.empty()) {
            return existingStartChannelAsString;
        }
    }

    const std::string& stch = ModelStartChannel;
    const std::string& priorStringStartChannelAsString = (i - 1 < (int)_indivStartChannels.size()) ? _indivStartChannels[i-1] : stch;
    int priorLength = CalcChannelsPerString();
    // This will be required once custom model supports multiple strings ... working on that
    // if (DisplayAs == "Custom")
    //{
    //    priorLength = GetStrandLength(i - 1) * GetChanCountPerNode();
    //}
    int32_t priorStringStartChannel = GetNumberFromChannelString(priorStringStartChannelAsString);
    int32_t startChannel = priorStringStartChannel + priorLength;
    if (stch.find(':') != std::string::npos) {
        // split priorStringStartChannelAsString on ':'
        std::vector<std::string> comps;
        std::string::size_type pos = 0, found;
        while ((found = priorStringStartChannelAsString.find(':', pos)) != std::string::npos) {
            comps.push_back(priorStringStartChannelAsString.substr(pos, found - pos));
            pos = found + 1;
        }
        comps.push_back(priorStringStartChannelAsString.substr(pos));

        if (!comps.empty() && !comps[0].empty() && comps[0][0] == '#') {
            int32_t ststch;
            Output* o = modelManager.GetOutputManager()->GetOutput(startChannel, ststch);
            if (comps.size() == 2) {
                if (o != nullptr) {
                    return "#" + std::to_string(o->GetUniverse()) + ":" + std::to_string(ststch);
                } else {
                    return std::to_string(startChannel);
                }
            } else {
                if (o != nullptr) {
                    return comps[0] + ":" + std::to_string(o->GetUniverse()) + ":" + std::to_string(ststch);
                } else {
                    return std::to_string(startChannel);
                }
            }
        } else if (!comps.empty() && !comps[0].empty() &&
                   (comps[0][0] == '>' || comps[0][0] == '@' || comps[0][0] == '!')) {
            long offset = 0;
            if (comps.size() > 1) {
                char* end = nullptr;
                offset = std::strtol(comps[1].c_str(), &end, 10);
            }
            return comps[0] + ":" + std::to_string(offset + priorLength);
        } else {
            // This used to be on:sc but this is no longer supported
            return std::to_string(startChannel);
        }
    }
    return std::to_string(startChannel);
}

bool Model::ModelRenamed(const std::string& oldName, const std::string& newName)
{
    bool changed = false;
    std::string sc = ModelStartChannel;
    if ((sc[0] == '@' || sc[0] == '<' || sc[0] == '>') && sc.size() > 1) {
        std::string mn = sc.substr(1, sc.find(':') - 1);
        if (mn == oldName) {
            sc = sc[0] + newName + sc.substr(sc.find(':'), sc.size());
            ModelStartChannel = sc;
            changed = true;
        }
    }

    if (GetShadowModelFor() == oldName) {
        SetShadowModelFor(newName);
        changed = true;
    }

    if (_modelChain == ">" + oldName) {
        _modelChain = ">" + newName;
        changed = true;
    }

    for (size_t i = 0; i < stringStartChan.size() && i < _indivStartChannels.size(); ++i) {
        if ((sc[0] == '@' || sc[0] == '<' || sc[0] == '>') && sc.size() > 1) {
            std::string mn = sc.substr(1, sc.find(':') - 1);
            if (mn == oldName) {
                sc = sc[0] + newName + sc.substr(sc.find(':'), sc.size());
                _indivStartChannels[i] = sc;
                changed = true;
            }
        }
    }
    if (changed) {
        Setup();
    }
    return changed;
}

// Returns true if s is a non-empty string of digits (no '.') representing a positive integer
static bool IsPositiveInteger(const std::string& s) {
    if (s.empty() || s.find('.') != std::string::npos) return false;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
    }
    long v = std::strtol(s.c_str(), nullptr, 10);
    return v > 0;
}

bool Model::IsValidStartChannelString() const
{
    const std::string& sc = this->ModelStartChannel;

    if (IsPositiveInteger(sc))
        return true; // absolute

    if (sc.find(':') == std::string::npos)
        return false; // all other formats need a colon

    auto parts = Split(sc, ':');

    if (parts.size() > 3)
        return false;

    if (parts[0][0] == '#') {
        if (parts.size() == 2) {
            Output* o = modelManager.GetOutputManager()->GetOutput(std::strtol(Trim(parts[0].substr(1)).c_str(), nullptr, 10), "");
            if (o != nullptr && IsPositiveInteger(Trim(parts[1]))) {
                return true;
            }
        } else if (parts.size() == 3) {
            std::string ip = Trim(parts[0].substr(1));
            Output* o = modelManager.GetOutputManager()->GetOutput(std::strtol(Trim(parts[1]).c_str(), nullptr, 10), ip);
            if (ip_utils::IsIPValidOrHostname(ip) && o != nullptr && IsPositiveInteger(Trim(parts[2]))) {
                return true;
            }
        }
    } else if (parts[0][0] == '>' || parts[0][0] == '@') {
        if ((parts.size() == 2) &&
            (Trim(parts[0]).substr(1) != GetName()) && // self referencing
            IsPositiveInteger(Trim(parts[1]))) {
            // dont bother checking the model name ... other processes will check for that
            return true;
        }
    } else if (parts[0][0] == '!') {
        if ((parts.size() == 2) &&
            (modelManager.GetOutputManager()->GetController(Trim(parts[0].substr(1))) != nullptr) &&
            IsPositiveInteger(Trim(parts[1]))) {
            return true;
        }
    }

    return false;
}

int Model::GetNumberFromChannelString(const std::string& sc) const
{
    bool v = false;
    std::string dependsonmodel;
    return GetNumberFromChannelString(sc, v, dependsonmodel);
}

int Model::GetNumberFromChannelString(const std::string& str, bool& valid, std::string& dependsonmodel) const
{
    std::string sc(Trim(str));
    valid = true;
    if (sc.find(":") != std::string::npos) {
        std::string start = sc.substr(0, sc.find(":"));
        sc = sc.substr(sc.find(":") + 1);
        if (start[0] == '@' || start[0] == '<' || start[0] == '>') {
            int returnChannel = (int)std::strtol(sc.c_str(), nullptr, 10);
            bool chain = start[0] == '>';
            bool fromStart = start[0] == '@';
            start = Trim(start.substr(1, start.size()));
            if (start == GetName() && !CouldComputeStartChannel) {
                valid = false;
            } else {
                if (start != GetName()) {
                    dependsonmodel = start;
                }
                Model* m = modelManager[start];
                if (m != nullptr && m->CouldComputeStartChannel && (!chain || (chain && m->GetControllerName() != NO_CONTROLLER))) {
                    if (fromStart) {
                        int i = m->GetFirstChannel();
                        if (i == -1 && m == this && stringStartChan.size() > 0) {
                            i = stringStartChan[0];
                        }
                        int res = i + returnChannel;
                        if (res < 1) {
                            valid = false;
                            res = 1;
                        }
                        return res;
                    } else {
                        int res = m->GetLastChannel() + returnChannel + 1;
                        if (res < 1) {
                            valid = false;
                            res = 1;
                        }
                        return res;
                    }
                } else {
                    valid = false;
                }
            }
        } else if (start[0] == '!') {
            if (sc.find_first_of(':') == std::string::npos) {
                std::string cs = Trim(start.substr(1));
                Controller* c = modelManager.GetOutputManager()->GetController(cs);
                if (c != nullptr && c->GetProtocol() != OUTPUT_PLAYER_ONLY) {
                    return c->GetStartChannel() - 1 + (int)std::strtol(sc.c_str(), nullptr, 10);
                }
            }
            valid = false;
            return 1;
        } else if (start[0] == '#') {
            // str is like "#ip:universe:channel" or "#universe:channel" — split from char 1
            auto cs = Split(str.substr(1), ':');
            if (cs.size() == 3) {
                // #ip:universe:channel
                int returnUniverse = (int)std::strtol(cs[1].c_str(), nullptr, 10);
                int returnChannel = (int)std::strtol(cs[2].c_str(), nullptr, 10);

                int res = modelManager.GetOutputManager()->GetAbsoluteChannel(Trim(cs[0]), returnUniverse - 1, returnChannel - 1);
                if (res < 1) {
                    res = 1;
                    valid = false;
                }
                return res;
            } else if (cs.size() == 2) {
                // #universe:channel
                int returnChannel = (int)std::strtol(sc.c_str(), nullptr, 10);
                int returnUniverse = (int)std::strtol(cs[0].c_str(), nullptr, 10);

                // find output based on universe number ...
                int res = modelManager.GetOutputManager()->GetAbsoluteChannel("", returnUniverse - 1, returnChannel - 1);
                if (res < 1) {
                    res = 1;
                    valid = false;
                }
                return res;
            } else {
                valid = false;
                return 1;
            }
        }
    }
    int returnChannel = (int)std::strtol(sc.c_str(), nullptr, 10);
    if (returnChannel < 1) {
        valid = false;
        returnChannel = 1;
    }

    return returnChannel;
}

std::list<int> Model::ParseFaceNodes(std::string channels)
{
    std::list<int> res;

    std::string_view sv(channels);
    size_t pos = 0;
    while (pos != std::string::npos && pos <= sv.size()) {
        size_t next = sv.find(',', pos);
        std::string_view valstr = sv.substr(pos, next == std::string::npos ? next : next - pos);
        pos = (next == std::string::npos) ? next : next + 1;

        int start, end;
        auto dashpos = valstr.find('-');
        if (dashpos != std::string_view::npos) {
            start = (int)std::strtol(std::string(valstr.substr(0, dashpos)).c_str(), nullptr, 10);
            end = (int)std::strtol(std::string(valstr.substr(dashpos + 1)).c_str(), nullptr, 10);
            if (end < start)
                std::swap(start, end);
        } else {
            start = end = (int)std::strtol(std::string(valstr).c_str(), nullptr, 10);
        }
        if (start > end) {
            start = end;
        }
        start--;
        end--;
        for (int n = start; n <= end; ++n) {
            res.push_back(n);
        }
    }

    return res;
}

void Model::SetNodeNames(std::string const& nodes)
{
    _nodeNamesString = nodes;
    nodeNames.clear();
    std::string tempstr = nodes;
    while (!tempstr.empty()) {
        std::string t2;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr.substr(1);
        } else if (auto pos = tempstr.find(','); pos != std::string::npos) {
            t2 = tempstr.substr(0, pos);
            tempstr = tempstr.substr(pos + 1);
        } else {
            t2 = tempstr;
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }
}

void Model::SetStrandNames(std::string const& strands)
{
    _strandNamesString = strands;
    strandNames.clear();
    std::string tempstr = strands;
    while (!tempstr.empty()) {
        std::string t2;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr.substr(1);
        } else if (auto pos = tempstr.find(','); pos != std::string::npos) {
            t2 = tempstr.substr(0, pos);
            tempstr = tempstr.substr(pos + 1);
        } else {
            t2 = tempstr;
            tempstr = "";
        }
        strandNames.push_back(t2);
    }
}

void Model::UpdateChannels()
{
    // alternative function that only performs channel calculation instead of calling the full Setup function
    CouldComputeStartChannel = false;
    std::string dependsonmodel;
    int32_t StartChannel = GetNumberFromChannelString(ModelStartChannel, CouldComputeStartChannel, dependsonmodel);

    // calculate starting channel numbers for each string
    size_t NumberOfStrings = GetNumStrings();
    int ChannelsPerString = CalcChannelsPerString();

    SetStringStartChannels(NumberOfStrings, StartChannel, ChannelsPerString);
    Nodes.clear();
    InitModel();

    // Submodels clone parent nodes during Setup, so when parent channels
    // change (e.g. after RecalcStartChannels resolves chain dependencies),
    // submodels must be re-setup to pick up the new ActChan values.
    for (auto& sm : subModels) {
        sm->Setup();
    }

    IncrementChangeCount();
}

void Model::Setup()
{
    if (modelManager.GetOutputModelManager() != nullptr) {
        GetBaseObjectScreenLocation().SetOutputModelManager(modelManager.GetOutputModelManager());
    }

    auto swStart = std::chrono::steady_clock::now();

    StrobeRate = 0;

    SingleNode = HasSingleNode(StringType);
    int ncc = GetNodeChannelCount(StringType);
    SingleChannel = (ncc == 1) && StringType != "Node Single Color";
    if (SingleNode) {
        rgbOrder = "RGB";
    } else if (ncc == 4 && StringType[0] == 'W') {
        rgbOrder = StringType.substr(1, 4);
    } else {
        rgbOrder = StringType.substr(0, 3);
    }
    //if (ncc > 3) {
    //    std::string s = ModelNode->GetAttribute("RGBWHandling").ToStdString();
    //    SetRGBWHandling(s);
    if (ncc <= 3) {
        rgbwHandlingType = 1; // RGB
    }

    // _modelTagColour = wxNullColour; Shouldn't need to keep resetting this anymore

    GetModelScreenLocation().Init();

    UpdateChannels();
    
    auto swElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - swStart).count();
    if (swElapsed > 10) {
        spdlog::debug("{} model {} took {}ms to initialise.", DisplayAsTypeToString(DisplayAs).c_str(), (const char*)name.c_str(), swElapsed);
    }
    for (auto &sm : subModels) {
        sm->Setup();
    }
}

std::string Model::GetControllerConnectionString() const
{
    if (GetControllerProtocol() == "")
        return "";
    std::string ret = fmt::format("{}:{}", GetControllerProtocol(), GetControllerPort(1));

    ret += GetControllerConnectionAttributeString();
    return ret;
}

std::string Model::GetControllerConnectionRangeString() const
{
    if (GetControllerProtocol() == "")
        return "";
    std::string ret = fmt::format("{}:{}", GetControllerProtocol(), GetControllerPort(1));
    if (GetControllerPort(1) == 0) {
        ret = GetControllerProtocol();
    }
    if (GetNumPhysicalStrings() > 1 && GetControllerPort(1) != 0 && !IsMatrixProtocol()) {
        ret = fmt::format("{}-{}", ret, GetControllerPort(GetNumPhysicalStrings()));
    }

    ret += GetControllerConnectionAttributeString();

    return ret;
}

std::string Model::GetControllerConnectionPortRangeString() const
{
    std::string ret = std::to_string(GetControllerPort(1));
    if (GetNumPhysicalStrings() > 1 && GetControllerPort(1) != 0 && !IsMatrixProtocol()) {
        ret = fmt::format("{}-{}", ret, GetControllerPort(GetNumPhysicalStrings()));
    }
    return ret;
}

bool compare_pairstring(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b)
{
    return a.first > b.first;
}

std::string Model::GetControllerConnectionAttributeString() const
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("cc");
    XmlSerializingVisitor v(root);
    v.Visit(_controllerConnection);
    pugi::xml_node n = root.first_child();

    std::string ret;
    for (auto attr = n.first_attribute(); attr; attr = attr.next_attribute()) {
        if (std::string_view(attr.name()) != "Protocol" && std::string_view(attr.name()) != "Port") {
            ret += ":";
            ret += std::string(attr.name()) + "=" + attr.value();
        }
    }
    return ret;
}

void Model::ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP)
{
    bool changed = false;
    if (Contains(ModelStartChannel, oldIP)) {
        std::string sc = ModelStartChannel;
        Replace(sc, oldIP, newIP);
        SetStartChannel(sc);
        changed = true;
    }

    size_t NumberOfStrings = GetNumStrings();
    for (size_t i = 0; i < NumberOfStrings && i < _indivStartChannels.size(); ++i) {
        std::string sc = _indivStartChannels[i];
        if (Contains(sc, oldIP)) {
            Replace(sc, oldIP, newIP);
            _indivStartChannels[i] = sc;
            changed = true;
        }

    }
    if (changed) {
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                    OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "Model::ReplaceIPInStartChannels");
    }
}

std::string Model::DecodeSmartRemote(int sr) const
{
    if (sr == 0)
        return "None";

    auto caps = GetControllerCaps();
    bool hinkspix = (caps && caps->GetVendor() == "HinksPix");

    if (hinkspix) {
        return std::to_string(sr - 1);
    }

    return std::string(1, ('A' + sr - 1));
}


void Model::RemoveSubModel(const std::string& name)
{
    for (auto it = subModels.begin(); it != subModels.end(); ) {
        Model* m = *it;
        if (m->GetName() == name) {
            sortedSubModels.erase(name);
            delete m;
            it = subModels.erase(it);
            return;
        } else {
            ++it;
        }
    }
}

void Model::RemoveAllSubModels()
{
    for (auto it = subModels.begin(); it != subModels.end(); ) {
        Model* m = *it;

        sortedSubModels.erase(m->GetName());

        delete m;

        it = subModels.erase(it);
    }
}

Model* Model::GetSubModel(const std::string& name) const
{
    auto it = sortedSubModels.find(name);
    if (it != sortedSubModels.end()) {
        return it->second;
    }
    return nullptr;
}

std::string Model::GenerateUniqueSubmodelName(const std::string suggested) const
{
    if (GetSubModel(suggested) == nullptr)
        return suggested;

    int i = 2;
    for (;;) {
        auto name = fmt::format("{}_{}", suggested, i++);
        if (GetSubModel(name) == nullptr)
            return name;
    }
}

int Model::CalcChannelsPerString()
{
    int ChannelsPerString = NodesPerString() * GetNodeChannelCount(StringType);
    if (SingleChannel)
        ChannelsPerString = 1;
    else if (SingleNode)
        ChannelsPerString = GetNodeChannelCount(StringType);

    return ChannelsPerString;
}

void Model::SetStringStartChannels(int NumberOfStrings, int StartChannel, int ChannelsPerString)
{
    stringStartChan.clear();
    stringStartChan.resize(NumberOfStrings);
    int num_indiv_start_channels = _indivStartChannels.size();
    for (int i = 0; i < NumberOfStrings; ++i) {
        if (_hasIndivChans && (i < num_indiv_start_channels)) {
            bool b = false;
            std::string dependsonmodel;
            stringStartChan[i] = GetNumberFromChannelString(_indivStartChannels[i], b, dependsonmodel) - 1;
            CouldComputeStartChannel &= b;
        } else if (_hasIndivNodes && i < (int)_indivStartNodes.size()) {
            int node = _indivStartNodes[i];
            if (node < 1) node = 1;
            stringStartChan[i] = (StartChannel - 1) + (node - 1) * GetNodeChannelCount(StringType);
        } else {
            stringStartChan[i] = (StartChannel - 1) + i * ChannelsPerString;
        }
    }
}

int Model::ComputeStringStartNode(int x) const
{
    if (x == 0) return 1;

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return (int)(x * nodesPerString + 1);
}

int Model::FindNodeAtXY(int bufx, int bufy)
{
    for (int i = 0; i < (int)Nodes.size(); ++i) {
        if ((bufx == -1 || Nodes[i]->Coords[0].bufX == bufx) && (bufy == -1 || Nodes[i]->Coords[0].bufY == bufy)) {
            return i;
        }
    }
    assert(false);
    return -1;
}

void Model::GetNodeChannelValues(size_t nodenum, unsigned char* buf)
{
    assert(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->GetForChannels(buf);
    }
}

void Model::SetNodeChannelValues(size_t nodenum, const unsigned char* buf)
{
    assert(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->SetFromChannels(buf);
    }
}

xlColor Model::GetNodeColor(size_t nodenum) const
{
    assert(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    xlColor color;
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->GetColor(color);
    } else {
        color = xlWHITE;
    }
    return color;
}

const xlColor &Model::GetNodeMaskColor(size_t nodenum) const
{
    if (nodenum >= Nodes.size())
        return xlWHITE; // this shouldnt happen but it does if you have a custom model with no nodes in it
    return Nodes[nodenum]->GetMaskColor();
}

void Model::SetNodeColor(size_t nodenum, const xlColor& c)
{
    assert(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->SetColor(c);
    }
}

bool Model::IsNodeInBufferRange(size_t nodeNum, int x1, int y1, int x2, int y2)
{
    if (nodeNum < Nodes.size()) {
        for (auto &a : Nodes[nodeNum]->Coords) {
            if (a.bufX >= x1 && a.bufX <= x2 && a.bufY >= y1 && a.bufY <= y2) {
                return true;
            }
        }
    }
    return false;
}

// only valid for rgb nodes and dumb strings (not traditional strings)
char Model::GetChannelColorLetter(uint8_t chidx)
{
    return rgbOrder[chidx];
}

char Model::EncodeColour(const xlColor& c)
{
    if (c.red > 0 && c.green == 0 && c.blue == 0) {
        return 'R';
    }
    if (c.red == 0 && c.green > 0 && c.blue == 0) {
        return 'G';
    }
    if (c.red == 0 && c.green == 0 && c.blue > 0) {
        return 'B';
    }
    if (c.red > 0 && c.red == c.green && c.red == c.blue) {
        return 'W';
    }

    return 'X';
}

// Accepts any absolute channel number and if it happens to be used by this model a single character representing the channel colour is returned.
// If the channel does not map to the model this returns ' '
char Model::GetAbsoluteChannelColorLetter(int32_t absoluteChannel)
{
    int32_t fc = GetFirstChannel();
    if (absoluteChannel < fc + 1 || absoluteChannel > (int32_t)GetLastChannel() + 1)
        return ' ';

    if (SingleChannel) {
        return EncodeColour(GetNodeMaskColor(0));
    }
    int ccpn = std::max(GetChanCountPerNode(), 1);
    return GetChannelColorLetter((absoluteChannel - fc - 1) % ccpn);
}

std::string Model::GetControllerPortSortString() const
{
    auto controller = GetControllerName();
    if (controller.empty()) {
        controller = PadLeft("Z", 'Z', 140);
    }
    auto port = GetControllerPort();
    auto sc = GetFirstChannel(); // we assume within a port models are in channel order

    return fmt::format("{}:{:08d}:{:08d}", controller, port, sc);
}

std::string Model::GetStartChannelInDisplayFormat(OutputManager* outputManager)
{
    auto s = Trim(ModelStartChannel);
    if (!IsValidStartChannelString()) {
        return "(1)";
    } else if (s[0] == '>') {
        return s + fmt::format(" ({})", GetFirstChannel() + 1);
    } else if (s[0] == '@') {
        if (_hasIndivChans) {
            return s;
        } else {
            return s + fmt::format(" ({})", GetFirstChannel() + 1);
        };
    } else if (s[0] == '!') {
        return s + fmt::format(" ({})", GetFirstChannel() + 1);
    } else if (s[0] == '#') {
        return GetFirstChannelInStartChannelFormat(outputManager);
    } else {
        return std::to_string(GetFirstChannel() + 1);
    }
}

std::string Model::GetLastChannelInStartChannelFormat(OutputManager* outputManager)
{
    return GetChannelInStartChannelFormat(outputManager, GetLastChannel() + 1);
}

std::string Model::GetChannelInStartChannelFormat(OutputManager* outputManager, uint32_t channel)
{
    std::list<std::string> visitedModels;
    visitedModels.push_back(GetName());

    std::string modelFormat = Trim(ModelStartChannel);
    char firstChar = modelFormat[0];

    bool done = false;
    while (!done && (firstChar == '@' || firstChar == '>') && CountChar(modelFormat, ':') == 1) {
        std::string referencedModel = Trim(modelFormat.substr(1, modelFormat.find(':') - 1));
        Model* m = modelManager[referencedModel];

        if (m != nullptr && std::find(visitedModels.begin(), visitedModels.end(), referencedModel) == visitedModels.end()) {
            modelFormat = Trim(m->ModelStartChannel);
            firstChar = modelFormat[0];
        } else {
            done = true;
        }
        visitedModels.push_back(referencedModel);
    }

    if (!modelFormat.empty()) {
        if (modelFormat[0] == '#') {
            firstChar = '#';
        } else if (modelFormat[0] == '!') {
            firstChar = '!';
        } else if (CountChar(modelFormat, ':') == 1) {
            firstChar = '0';
        }
    } else {
        firstChar = '0';
        modelFormat = "0";
    }

    if (firstChar == '#') {
        // universe:channel
        int32_t startChannel;
        Output* output = outputManager->GetOutput(channel, startChannel);

        if (output == nullptr) {
            return std::to_string(channel);
        }

        // This should not be the case any more
        // if (output->IsOutputCollection())
        //{
        //    output = output->GetActualOutput(channel);
        //    startChannel = channel - output->GetStartChannel() + 1;
        //}

        if (CountChar(modelFormat, ':') == 1) {
            return "#" + std::to_string(output->GetUniverse()) + ":" + std::to_string(startChannel) + " (" + std::to_string(channel) + ")";
        } else {
            std::string ip = "<err>";
            if (output->IsIpOutput()) {
                ip = ((IPOutput*)output)->GetIP();
            }
            return "#" + ip + ":" + std::to_string(output->GetUniverse()) + ":" + std::to_string(startChannel) + " (" + std::to_string(channel) + ")";
        }
    } else if (firstChar == '!') {
        auto comps = Split(modelFormat, ':');
        auto c = outputManager->GetController(Trim(comps[0].substr(1)));
        int32_t start = 1;
        if (c != nullptr) {
            start = c->GetStartChannel();
        }
        unsigned int lastChannel = GetLastChannel() + 1;
        return Trim(BeforeFirst(modelFormat, ':')) + ":" + std::to_string(lastChannel - start + 1) + " (" + std::to_string(lastChannel) + ")";
    } else if (firstChar == '@' || firstChar == '>' || CountChar(modelFormat, ':') == 0) {
        // absolute
        return std::to_string(channel);
    } else {
        // This used to be output:sc ... but that is no longer valid
        return std::to_string(channel);
    }
}

std::string Model::GetFirstChannelInStartChannelFormat(OutputManager* outputManager)
{
    return GetChannelInStartChannelFormat(outputManager, GetFirstChannel() + 1);
}

uint32_t Model::GetLastChannel() const
{
    uint32_t LastChan = 0;
    size_t NodeCount = GetNodeCount();
    for (size_t idx = 0; idx < NodeCount; ++idx) {
        if (Nodes[idx]->ActChan == (unsigned int)-1) {
            return (unsigned int)NodeCount * Nodes[idx]->GetChanCount() - 1;
        }
        unsigned int lc = std::max(LastChan, Nodes[idx]->ActChan + Nodes[idx]->GetChanCount() - 1);
        if (lc > LastChan) {
            LastChan = lc;
        }
    }
    return LastChan;
}

// zero based channel number, i.e. 0 is the first channel
uint32_t Model::GetFirstChannel() const
{
    uint32_t FirstChan = 0xFFFFFFFF;
    size_t NodeCount = GetNodeCount();
    for (size_t idx = 0; idx < NodeCount; ++idx) {
        uint32_t fc = std::min(FirstChan, Nodes[idx]->ActChan);
        if (fc < FirstChan) {
            FirstChan = fc;
        }
    }
    return FirstChan;
}

unsigned int Model::GetNumChannels() const
{
    return GetLastChannel() - GetFirstChannel() + 1;
}

void Model::SetPosition(double posx, double posy)
{
    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().SetPosition(posx, posy);
    IncrementChangeCount();
}

// initialize screen coordinates
void Model::SetLineCoord()
{
    float x, y;
    float idx = 0;
    size_t NodeCount = GetNodeCount();
    int numlights = GetNumStrings() * NodesPerString();
    float half = numlights / 2;
    GetModelScreenLocation().SetRenderSize(numlights, numlights * 2);

    for (size_t n = 0; n < NodeCount; ++n) {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; ++c) {
            x = idx;
            x = IsLtoR ? x - half : half - x;
            y = 0;
            Nodes[n]->Coords[c].screenX = x;
            Nodes[n]->Coords[c].screenY = y + numlights;
            idx++;
        }
    }
}

void Model::SetBufferSize(int NewHt, int NewWi)
{
    BufferHt = NewHt;
    BufferWi = NewWi;
    IncrementChangeCount();
}

// not valid for Frame or Custom
int Model::NodesPerString() const
{
    // Base implementation returns 1. Subclasses override with their specific node counts.
    return 1;
}

int Model::NodesPerString(int string) const
{
    int numStrings = GetNumStrings();
    if (numStrings <= 1) {
        return NodesPerString();
    }
    if (SingleNode) {
        return 1;
    }
    int v1 = 0, v2 = 0;
    if (_hasIndivNodes) {
        v1 = GetIndivStartNode(string);
        if (string < numStrings - 1) {
            v2 = GetIndivStartNode(string + 1);
        }
    } else {
        v1 = ComputeStringStartNode(string);
        if (string < numStrings - 1) {
            v2 = ComputeStringStartNode(string + 1);
        }
    }
    int num_nodes;
    if (string < numStrings - 1) {
        num_nodes = v2 - v1;
    } else {
        num_nodes = GetNodeCount() - v1 + 1;
    }
    int ts = GetSmartTs();
    return (ts <= 1) ? num_nodes : num_nodes * ts;
}

int32_t Model::NodeStartChannel(size_t nodenum) const
{
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan : 0; // avoid memory access error if no nods -DJ
}

int32_t Model::NodeEndChannel(size_t nodenum) const
{
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan + Nodes[nodenum]->GetChanCount() - 1 : 0; // avoid memory access error if no nods -DJ
}

const std::string& Model::NodeType(size_t nodenum) const
{
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->GetNodeType() : NodeBaseClass::RGB; // avoid memory access error if no nods -DJ
}

void Model::GetBufferSize(const std::string& tp, const std::string& camera, const std::string& transform, int& bufferWi, int& bufferHi, int stagger) const
{
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    if (type == DEFAULT) {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
    } else if (type == SINGLE_LINE) {
        bufferHi = 1;
        bufferWi = Nodes.size();
    } else if (type == AS_PIXEL) {
        bufferHi = 1;
        bufferWi = 1;
    } else if (type == VERT_PER_STRAND || type == VERT_PER_MODELSTRAND) {
        bufferHi = GetNumStrands();
        bufferWi = 1;
        for (int x = 0; x < bufferHi; ++x) {
            bufferWi = std::max(bufferWi, GetStrandLength(x));
        }
    } else if (type == HORIZ_PER_STRAND || type == HORIZ_PER_MODELSTRAND) {
        bufferWi = GetNumStrands();
        bufferHi = 1;
        for (int x = 0; x < bufferWi; ++x) {
            bufferHi = std::max(bufferHi, GetStrandLength(x));
        }
    } else {
        // if (type == PER_PREVIEW) {
        // default is to go ahead and build the full node buffer
        std::vector<NodeBaseClassPtr> newNodes;
        InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi, stagger);
    }
    AdjustForTransform(transform, bufferWi, bufferHi);
}

void Model::AdjustForTransform(const std::string& transform,
                               int& bufferWi, int& bufferHi) const
{
    if (transform == "Rotate CC 90" || transform == "Rotate CW 90") {
        int x = bufferHi;
        bufferHi = bufferWi;
        bufferWi = x;
    }
}

static inline void SetCoords(NodeBaseClass::CoordStruct& it2, int x, int y)
{
    it2.bufX = x;
    it2.bufY = y;
}

static inline void SetCoords(NodeBaseClass::CoordStruct& it2, int x, int y, int maxX, int maxY, int scale)
{
    if (maxX != -1) {
        x = x * maxX;
        x = x / scale;
    }
    if (maxY != -1) {
        y = y * maxY;
        y = y / scale;
    }
    it2.bufX = x;
    it2.bufY = y;
}

// this is really slow
char GetPixelDump(int x, int y, std::vector<NodeBaseClassPtr>& newNodes)
{
    for (auto n = newNodes.begin(); n != newNodes.end(); ++n) {
        for (auto &c : (*n)->Coords) {
            if (c.bufX == x && c.bufY == y) {
                return '*';
            }
        }
    }

    return '-';
}

void Model::DumpBuffer(std::vector<NodeBaseClassPtr>& newNodes,
                       int bufferWi, int bufferHt) const
{
    

    spdlog::debug("Dumping render buffer for '{}':", (const char*)GetFullName().c_str());
    for (int y = bufferHt - 1; y >= 0; y--) {
        std::string line = "";
        for (int x = 0; x < bufferWi; ++x) {
            line += GetPixelDump(x, y, newNodes);
        }
        spdlog::debug(">    {}", (const char*)line.c_str());
    }
}

void Model::ApplyTransform(const std::string& type,
                           std::vector<NodeBaseClassPtr>& newNodes,
                           int& bufferWi, int& bufferHi, int startNode) const
{
    //"Rotate CC 90", "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal", "Rotate CC 90 Flip Horizontal", "Rotate CW 90 Flip Horizontal"
    if (type == "None") {
        return;
    } else if (type == "Rotate 180") {
        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferWi - it2.bufX - 1, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Flip Vertical") {
        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Flip Horizontal") {
        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferWi - it2.bufX - 1, it2.bufY);
            }
        }
    } else if (type == "Rotate CW 90") {
        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferHi - it2.bufY - 1, it2.bufX);
            }
        }
        std::swap(bufferWi, bufferHi);
    } else if (type == "Rotate CC 90") {
        for (int x = startNode; x < (int)newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufY, bufferWi - it2.bufX - 1);
            }
        }
        std::swap(bufferWi, bufferHi);
    } else if (type == "Rotate CC 90 Flip Horizontal") {
        for (int x = startNode; x < (int)newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufY, bufferWi - it2.bufX - 1);
            }
        }
        std::swap(bufferWi, bufferHi);

        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Rotate CW 90 Flip Horizontal") {
        for (size_t x = startNode; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferHi - it2.bufY - 1, it2.bufX);
            }
        }
        std::swap(bufferWi, bufferHi);

        for (size_t x = 0; x < newNodes.size(); ++x) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    }
}

const std::string Model::AdjustBufferStyle(const std::string &style) const {
    auto styles = GetBufferStyles();
    if (std::find(styles.begin(), styles.end(), style) == styles.end()) {
        if (style.substr(0, 9) == "Per Model") {
            return style.substr(10);
        } else {
            return "Default";
        }
    }
    return style;
}


void Model::InitRenderBufferNodes(const std::string& tp, const std::string& camera,
                                  const std::string& transform,
                                  std::vector<NodeBaseClassPtr>& newNodes, int& bufferWi, int& bufferHt, int stagger, bool deep) const
{
    
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;
    int firstNode = newNodes.size();

    // want to see if i can catch something that causes this to crash
    if (firstNode + Nodes.size() <= 0) {
        // This seems to happen when an effect is dropped on a strand with zero pixels
        // Like a polyline segment with no nodes
        spdlog::warn("Model::InitRenderBufferNodes firstNode + Nodes.size() = {}. {}::'{}'. This commonly happens on a polyline segment with zero pixels or a custom model with no nodes but with effects dropped on it.", (int32_t)firstNode + Nodes.size(), DisplayAsTypeToString(DisplayAs).c_str(), (const char*)GetFullName().c_str());
    }

    // Don't add model group nodes if its a 3D preview render buffer
    if (!((camera != "2D") && GetDisplayAs() == DisplayAsType::ModelGroup && (type == PER_PREVIEW || type == PER_PREVIEW_NO_OFFSET))) {
        newNodes.reserve(firstNode + Nodes.size());
        for (auto& it : Nodes) {
            if (it == nullptr) {
                spdlog::critical("Model::InitRenderBufferNodes node is null in model '{}'. Skipping.", (const char*)GetFullName().c_str());
                continue;
            }
            newNodes.push_back(NodeBaseClassPtr(it.get()->clone()));
            if (newNodes.back()->model == nullptr) {
                spdlog::critical("Model::InitRenderBufferNodes cloned node has null model in '{}', ActChan={}.", (const char*)GetFullName().c_str(), newNodes.back()->ActChan);
                newNodes.back()->model = this;
            }
        }
    }

    if (type == DEFAULT) {
        bufferHt = this->BufferHt;
        bufferWi = this->BufferWi;
    } else if (type == SINGLE_LINE) {
        bufferHt = 1;
        bufferWi = newNodes.size();
        int cnt = 0;
        for (int x = firstNode; x < (int)newNodes.size(); ++x) {
            if (newNodes[x] == nullptr) {
                spdlog::critical("XXX Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                assert(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, cnt, 0);
            }
            cnt++;
        }
    } else if (type == AS_PIXEL) {
        bufferHt = 1;
        bufferWi = 1;
        for (int x = firstNode; x < (int)newNodes.size(); ++x) {
            if (newNodes[x] == nullptr) {
                spdlog::critical("XXX Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                assert(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, 0, 0);
            }
        }
    } else if (type == HORIZ_PER_STRAND || type == HORIZ_PER_MODELSTRAND) {
        bufferWi = GetNumStrands();
        bufferHt = 1;
        for (int x = 0; x < bufferWi; ++x) {
            bufferHt = std::max(bufferHt, GetStrandLength(x));
        }
        int cnt = 0;
        int strand = 0;
        int strandLen = GetStrandLength(GetMappedStrand(0));
        for (int x = firstNode; x < (int)newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                if (strand < GetNumStrands()) {
                    strandLen = GetStrandLength(GetMappedStrand(strand));
                } else {
                    // not sure what to do here ... we have more nodes than strands ... so lets just start again
                    strandLen = GetStrandLength(GetMappedStrand(0));
                    strand = 0;
                }
                cnt = 0;
            } else {
                if (newNodes[x] == nullptr) {
                    spdlog::critical("AAA Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    assert(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {
                    SetCoords(it2, strand, cnt, -1, bufferHt, strandLen);
                }
                cnt++;
                x++;
            }
        }
    } else if (type == VERT_PER_STRAND || type == VERT_PER_MODELSTRAND) {
        bufferHt = GetNumStrands();
        bufferWi = 1;
        for (int x = 0; x < bufferHt; ++x) {
            bufferWi = std::max(bufferWi, GetStrandLength(x));
        }
        int cnt = 0;
        int strand = 0;
        int strandLen = GetStrandLength(GetMappedStrand(0));
        for (int x = firstNode; x < (int)newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                if (strand < GetNumStrands()) {
                    strandLen = GetStrandLength(GetMappedStrand(strand));
                } else {
                    // not sure what to do here ... we have more nodes than strands ... so lets just start again
                    strandLen = GetStrandLength(GetMappedStrand(0));
                    strand = 0;
                }
                cnt = 0;
            } else {
                if (newNodes[x] == nullptr) {
                    spdlog::critical("BBB Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    assert(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {
                    SetCoords(it2, cnt, strand, bufferWi, -1, strandLen);
                }
                cnt++;
                x++;
            }
        }
    } else if (type == PER_PREVIEW || type == PER_PREVIEW_NO_OFFSET) {
        float maxX = -1000000.0;
        float minX = 1000000.0;
        float maxY = -1000000.0;
        float minY = 1000000.0;

        IModelPreview* modelPreview = nullptr;
        PreviewCamera* pcamera = nullptr;
        if (auto* rc = modelManager.GetRenderContext()) {
            modelPreview = rc->GetHousePreview();
            pcamera = rc->GetNamedCamera3D(camera);
        }

        if (pcamera != nullptr && camera != "2D") {
            GetModelScreenLocation().PrepareToDraw(true, false);
        } else {
            GetModelScreenLocation().PrepareToDraw(false, false);
        }

        // For 3D render view buffers recursively process each individual model...should be able to handle nested model groups
        if (GetDisplayAs() == DisplayAsType::ModelGroup && camera != "2D") {
            const ModelGroup *mg = dynamic_cast<const ModelGroup*>(this);
            int nc = 0;
            for (auto &c : mg->ActiveModels()) {
                nc += c->GetNodeCount();
            }
            if (nc) {
                newNodes.reserve(nc);
            }
            for (auto &c : mg->ActiveModels()) {
                int bw, bh;
                c->InitRenderBufferNodes("Per Preview No Offset", camera, transform, newNodes, bw, bh, stagger);
            }
        }

        // We save the transformed coordinates here so we dont have to calculate them all twice
        std::vector<float> outx;
        std::vector<float> outy;
        outx.reserve(newNodes.size() - firstNode); // common case is one coord per node so size for that
        outy.reserve(newNodes.size() - firstNode);
        for (int x = firstNode; x < (int)newNodes.size(); ++x) {
            if (newNodes[x] == nullptr) {
                spdlog::critical("CCC Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                assert(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                float sx = it2.screenX;
                float sy = it2.screenY;

                if (SupportsModelScreenLocation()) {
                    if (pcamera == nullptr || camera == "2D") {
                        // Handle all of the 2D classic transformations
                        // float sz = 0;
                        // reintroducing the z coordinate as otherwise with some rotations we end up with a zero width buffer
                        float sz = it2.screenZ;
                        GetModelScreenLocation().TranslatePoint(sx, sy, sz);
                    } else if (GetDisplayAs() != DisplayAsType::ModelGroup) { // ignore for groups since they will have already calculated their node positions from recursion call above
                        // Handle 3D from an arbitrary camera position
                        float sz = it2.screenZ;
                        GetModelScreenLocation().TranslatePoint(sx, sy, sz);

                        // really not sure if 400,400 is the best thing to pass in here ... but it seems to work
                        glm::vec2 loc = GetModelScreenLocation().GetScreenPosition(400, 400, modelPreview, pcamera, sx, sy, sz);
                        loc.y *= -1.0f;
                        sx = loc.x;
                        sy = loc.y;
                        it2.screenX = sx;
                        it2.screenY = sy;
                    }
                }

                // save the transformed value
                outx.push_back(sx);
                outy.push_back(sy);

                if (sx > maxX) {
                    maxX = sx;
                }
                if (sx < minX) {
                    minX = sx;
                }
                if (sy > maxY) {
                    maxY = sy;
                }
                if (sy < minY) {
                    minY = sy;
                }
            }
        }

        // Work out scaling factor for per preview camera views as these can build some
        // exteme locations which translate into crazy sized render buffers
        // this allows us to scale it back to the desired grid size
        float factor = 1.0;
        if (GetDisplayAs() == DisplayAsType::ModelGroup && type == PER_PREVIEW) {
            int maxDimension = ((ModelGroup*)this)->GetGridSize();
            if (maxDimension != 0 && (maxX - minX > maxDimension || maxY - minY > maxDimension)) {
                // we need to resize all the points by this amount
                spdlog::debug("Model Group ({}), Actual Grid Size of {:.0f} exceeded the Max Grid Size of {}; scaling to fit.",
                    (const char*)GetFullName().c_str(),
                    ((maxX - minX) > (maxY - minY) ? (maxX - minX) : (maxY - minY)),
                    maxDimension);
                factor = std::max(((float)(maxX - minX)) / (float)maxDimension, ((float)(maxY - minY)) / (float)maxDimension);
                // But if it is already smaller we dont want to make it bigger
                if (factor < 1.0) {
                    factor = 1.0;
                }
            }
        }
        if ((type != PER_PREVIEW_NO_OFFSET) && (((maxX - minX) > 2048) || ((maxY - minY) > 2048))) {
            // this will result in a GIANT render buffer, lets reduce to something we can reasonably render
            float fx = ((float)(maxX - minX)) / 2048.0f;
            float fy = ((float)(maxY - minY)) / 2048.0f;
            factor = fx > fy ? fx : fy;
        }

// if we have a dense model with lots of pixels but (int)(maxx - minx) and (int)(maxy - miny) are really small then it generates a render buffer that is quite small with lots of nodes in each cell
// We need a factor that scales up the screen locations to separate the pixels
// The empty space factor is the number of empty cells expected per filled cell in the average model ... of course in models where there are dense and sparse areas this wont necessarily be true
#define MODEL_EMPTY_SPACE_FACTOR 4.0f
        if (type == PER_PREVIEW && GetDisplayAs() != DisplayAsType::ModelGroup && factor == 1.0 && (newNodes.size() * (MODEL_EMPTY_SPACE_FACTOR + 1.0) > (maxX - minX) * (maxY - minY))) {
            float deltaX = maxX - minX;
            float deltaY = maxY - minY;

            // Don't allow "aspect" (down below) become zero because this will lead to a divide by zero for "factor", then all following calculations will be "nan".
            if (deltaX == 0) {
                deltaX = 0.01f;
            }

            // Don't allow "deltaY" to be zero because this will lead to a divide by zero for "aspect", then all following calculations will be "nan".
            if (deltaY == 0) {
                deltaY = 0.01f;
            }

            float aspect = deltaX / deltaY;
            float mx = ((float)newNodes.size()) * (MODEL_EMPTY_SPACE_FACTOR + 1.0f) * aspect;
            float x = std::sqrt(mx);
            factor = deltaX / x;
            if (std::max(deltaX / factor, deltaY / factor) > 400) { // if this results in an overly large scaling ... ie a buffer > 400 in any dimension
                factor = std::max(deltaX, deltaY) / 400;            // work out a scaling that gives a 400x400 buffer
            }
        }

        minX /= factor;
        maxX /= factor;
        minY /= factor;
        maxY /= factor;
        // spdlog::debug("Factor '{}':", factor);

        float offx = minX;
        float offy = minY;
        bool noOff = type == PER_PREVIEW_NO_OFFSET;

        if (noOff) {
            offx = 0;
            offy = 0;
        }
        bufferHt = bufferWi = -1;

        // we can skip all the scaling for individual models that are being recursively handled from a ModelGroup
        if (!(pcamera != nullptr && camera != "2D" && GetDisplayAs() != DisplayAsType::ModelGroup && noOff)) {
            auto itx = outx.begin();
            auto ity = outy.begin();
            for (int x = firstNode; x < (int)newNodes.size(); ++x) {
                if (newNodes[x] == nullptr) {
                    spdlog::critical("DDD Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    assert(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {
                    // grab the previously transformed coordinate
                    float sx = *itx / factor;
                    float sy = *ity / factor;

                    SetCoords(it2, std::round(sx - offx), std::round(sy - offy));
                    if (it2.bufX > bufferWi) {
                        bufferWi = it2.bufX;
                    }
                    if (it2.bufY > bufferHt) {
                        bufferHt = it2.bufY;
                    }

                    if (noOff) {
                        it2.screenX = sx;
                        it2.screenY = sy;
                    }

                    ++itx;
                    ++ity;
                }
            }
        }

        if (!noOff) {
            bufferHt++;
            bufferWi++;
        } else {
            bufferHt = std::round(maxY - minY + 1.0f);
            bufferWi = std::round(maxX - minX + 1.0f);
        }
        // DumpBuffer(newNodes, bufferWi, bufferHt);
    } else {
        bufferHt = this->BufferHt;
        bufferWi = this->BufferWi;
    }

    // Zero buffer sizes are bad
    // This can happen when a strand is zero length ... maybe also a custom model with no nodes
    if (bufferHt == 0) {
        spdlog::warn("Model::InitRenderBufferNodes BufferHt was 0 ... overridden to be 1.");
        bufferHt = 1;
    }
    if (bufferWi == 0) {
        spdlog::warn("Model::InitRenderBufferNodes BufferWi was 0 ... overridden to be 1.");
        bufferWi = 1;
    }
    if (bufferWi * bufferHt > 2100000) {
        if (bufferHt > 100000) {
            spdlog::warn("Model::InitRenderBufferNodes BufferHt was overly large ... overridden to be 100000.");
            bufferHt = 100000;
        }
        if (bufferWi > 100000) {
            spdlog::warn("Model::InitRenderBufferNodes BufferWi was overly large ... overridden to be 100000.");
            bufferWi = 100000;
        }
    }

    ApplyTransform(transform, newNodes, bufferWi, bufferHt);
}

std::string Model::GetNextName()
{
    if (nodeNames.size() > Nodes.size()) {
        return nodeNames[Nodes.size()];
    }
    return "";
}

bool Model::FiveChannelNodes() const
{
    return Contains(StringType, "RGBWW");
}

bool Model::FourChannelNodes() const
{
    // true if string contains WRGB or any variant thereof
    // I do the W search first to try to abort quickly for strings unlikely to be 4 channel
    return (Contains(StringType, "W") &&
            (Contains(StringType, "RGBW") ||
             Contains(StringType, "WRGB") ||
             Contains(StringType, "WRBG") ||
             Contains(StringType, "RBGW") ||
             Contains(StringType, "WGRB") ||
             Contains(StringType, "GRBW") ||
             Contains(StringType, "WGBR") ||
             Contains(StringType, "GBRW") ||
             Contains(StringType, "WBRG") ||
             Contains(StringType, "BRGW") ||
             Contains(StringType, "WBGR") ||
             Contains(StringType, "BGRW")));
}

std::list<std::string> Model::GetShadowedBy() const
{
    return GetModelManager().GetModelsShadowing(this);
}

// set size of Nodes vector and each Node's Coords vector
void Model::SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string& rgbOrder)
{
    size_t n;
    if (SingleNode) {
        if (StringType == "Single Color Red") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRed(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Single Color Green") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassGreen(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Superstring") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassSuperString(n, NodesPerString, superStringColours, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Single Color Blue") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassBlue(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Single Color White") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Strobes White 3fps" || StringType == "Strobes") {
            StrobeRate = 7; // 1 out of every 7 frames
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Single Color Custom") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n, NodesPerString, customColor, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "Single Color Intensity") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassIntensity(n, NodesPerString, customColor, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "4 Channel RGBW") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n, NodesPerString, "RGB", true, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType == "4 Channel WRGB") {
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n, NodesPerString, "RGB", false, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        } else {
            // 3 Channel RGB
            for (n = 0; n < NumStrings; ++n) {
                Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n, NodesPerString, "RGB", GetNextName())));
                Nodes.back()->model = this;
            }
        }
    } else if (NodesPerString == 0) {
        if (StringType == "Node Single Color") {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(0, 0, customColor, GetNextName())));
        } 
        else if (FiveChannelNodes()) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBWW(0, 0, rgbOrder, rgbwHandlingType, GetNextName())));
            Nodes.back()->model = this;
        }
        else if (FourChannelNodes()) {
            bool wLast = StringType[3] == 'W';
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(0, 0, rgbOrder, wLast, rgbwHandlingType, GetNextName())));
        } else {
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0, rgbOrder, GetNextName())));
        }
        Nodes.back()->model = this;
    } else if (StringType[3] == ' ') {
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; ++n) {
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n / NodesPerString, 1, rgbOrder, GetNextName())));
            Nodes.back()->model = this;
        }
    } else if (StringType == "Node Single Color") {
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; ++n) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n / NodesPerString, 1, customColor, GetNextName())));
            Nodes.back()->model = this;
        }
    } else if (StringType == "RGBWW Nodes") {
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; ++n) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBWW(n / NodesPerString, 1, rgbOrder, rgbwHandlingType, GetNextName())));
            Nodes.back()->model = this;
        }
    } else {
        bool wLast = StringType[3] == 'W';
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; ++n) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n / NodesPerString, 1, rgbOrder, wLast, rgbwHandlingType, GetNextName())));
            Nodes.back()->model = this;
        }
    }
}

size_t Model::GetNodeChannelCount(const std::string& nodeType) const
{
    if (nodeType.compare(0, 12, "Single Color") == 0) {
        return 1;
    } else if (nodeType == "Strobes White 3fps") {
        return 1;
    } else if (nodeType == "Strobes") {
        return 1;
    } else if (nodeType == "4 Channel RGBW") {
        return 4;
    } else if (nodeType == "4 Channel WRGB") {
        return 4;
    } else if (nodeType == "RGBWW Nodes") {
        return 5;
    } else if (nodeType[0] == 'W' || nodeType[3] == 'W') {
        // various WRGB and RGBW types
        return 4;
    } else if (nodeType == "Superstring") {
        return std::max(1, (int)superStringColours.size());
    } else if (nodeType == "Node Single Color") {
        return 1;
    }
    return 3;
}


bool Model::ContainsChannel(uint32_t startChannel, uint32_t endChannel) const
{
    // we could just check first and last channel but then that would not work for submodels
    for (const auto& node : Nodes) {
        if (node->ContainsChannel(startChannel, endChannel))
            return true;
    }
    return false;
}

bool Model::ContainsChannel(int strand, uint32_t startChannel, uint32_t endChannel) const
{
    uint32_t sc = GetChannelForNode(strand, 0);
    uint32_t ec = GetChannelForNode(strand, GetStrandLength(strand) - 1) + GetChanCountPerNode() - 1;
    return !(endChannel < sc || startChannel > ec);
}

bool Model::ContainsChannel(const std::string& submodelName, uint32_t startChannel, uint32_t endChannel) const
{
    auto sm = GetSubModel(submodelName);
    return sm->ContainsChannel(startChannel, endChannel);
}


uint32_t Model::GetChannelForNode(int strandIndex, int node) const
{
    if (node < 0)
        return -1;

    size_t nodesOnPriorStrands = 0;
    for (int s = 0; s < strandIndex; ++s) {
        nodesOnPriorStrands += GetStrandLength(s);
    }

    size_t n = nodesOnPriorStrands + node;

    if (n >= Nodes.size()) {
        assert(false);
        return -1;
    }

    return Nodes[n]->ActChan;
}

// returns a number where the first node is 1
uint32_t Model::GetNodeNumber(size_t nodenum) const
{
    if (nodenum >= Nodes.size())
        return 0;
    int sn = Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn * NodesPerString() + 1;
}

uint32_t Model::GetNodeNumber(int bufY, int bufX) const
{
    uint32_t count = 0;
    for (const auto& it : Nodes) {
        if (it->Coords.size() > 0) {
            if (it->Coords[0].bufX == bufX && it->Coords[0].bufY == bufY) {
                return count;
            }
        }
        count++;
    }
    return -1;
}

uint32_t Model::GetNodeCount() const
{
    return Nodes.size();
}

uint32_t Model::GetActChanCount() const
{
    uint32_t NodeCnt = GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t x = 0; x < NodeCnt; ++x) {
        count += Nodes[x]->GetChanCount();
    }

    return count;
}

uint32_t Model::GetChanCount() const
{
    size_t NodeCnt = GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    int min = 999999999;
    int max = 0;

    for (int x = 0; x < (int)NodeCnt; ++x) {
        int i = Nodes[x]->ActChan;
        if (i < min) {
            min = i;
        }
        i += Nodes[x]->GetChanCount();
        if (i > max) {
            max = i;
        }
    }
    return max - min;
}

NodeBaseClass* Model::GetNode(uint32_t node) const
{
    if (node < Nodes.size()) {
        return Nodes[node].get();
    }
    return nullptr;
}

int Model::GetChanCountPerNode() const
{
    size_t NodeCnt = GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    return Nodes[0]->GetChanCount();
}

uint32_t Model::GetCoordCount(size_t nodenum) const
{
    return nodenum < Nodes.size() ? Nodes[nodenum]->Coords.size() : 0;
}

int Model::GetNodeStringNumber(size_t nodenum) const
{
    return nodenum < Nodes.size() ? Nodes[nodenum]->StringNum : 0;
}

void Model::GetNode3DScreenCoords(int nodeidx, std::vector<std::tuple<float, float, float>>& pts)
{
    for (size_t x = 0; x < Nodes[nodeidx]->Coords.size(); ++x) {
        pts.push_back(std::make_tuple(Nodes[nodeidx]->Coords[x].screenX, Nodes[nodeidx]->Coords[x].screenY, Nodes[nodeidx]->Coords[x].screenZ));
    }
}

void Model::GetNodeCoords(int nodeidx, std::vector<xlPoint>& pts)
{
    if (nodeidx >= (int)Nodes.size())
        return;
    for (size_t x = 0; x < Nodes[nodeidx]->Coords.size(); ++x) {
        pts.push_back(xlPoint(Nodes[nodeidx]->Coords[x].bufX, Nodes[nodeidx]->Coords[x].bufY));
    }
}

bool Model::IsCustom()
{
    return (DisplayAs == DisplayAsType::Custom);
}

// convert # to AA format so it matches Custom Model grid display:
// this makes it *so* much easier to visually compare with Custom Model grid display
// A - Z == 1 - 26
// AA - AZ == 27 - 52
// BA - BZ == 53 - 78
// etc
static std::string AA(int x)
{
    std::string retval;
    --x;
    //    if (x >= 26 * 26) { retval += 'A' + x / (26 * 26); x %= 26 * 26; }
    if (x >= 26) {
        retval += 'A' + x / 26 - 1;
        x %= 26;
    }
    retval += 'A' + x;
    return retval;
}

// add just the node#s to a choice list:
// NO add parsed info to choice list or check list box:
size_t Model::GetChannelCoords(std::vector<std::string>& choices)
{ // wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3)
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; ++n) {
        if (Nodes[n]->Coords.empty())
            continue;
        choices.push_back(GetNodeXY(n));
    }
    return choices.size();
}

// get parsed node info:
std::string Model::GetNodeXY(const std::string& nodenumstr)
{
    size_t NodeCount = GetNodeCount();
    try {
        int32_t nodenum = (int32_t)std::strtol(nodenumstr.c_str(), nullptr, 10);
        for (size_t inx = 0; inx < NodeCount; ++inx) {
            if (Nodes[inx]->Coords.empty())
                continue;
            if ((int32_t)GetNodeNumber(inx) == nodenum)
                return GetNodeXY(inx);
        }
    } catch (...) {
    }
    return nodenumstr; // not found?
}

std::string Model::GetNodeXY(int nodeinx)
{
    if ((nodeinx < 0) || (nodeinx >= (int)GetNodeCount()))
        return "";
    if (Nodes[nodeinx]->Coords.empty())
        return "";
    if (GetCoordCount(nodeinx) > 1) // show count and first + last coordinates
        if (IsCustom())
            return fmt::format("{}: {}# @{}{}-{}{}", GetNodeNumber(nodeinx), GetCoordCount(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY, AA(Nodes[nodeinx]->Coords.back().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.back().bufY); // NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else
            return fmt::format("{}: {}# @({},{})=({},{})", GetNodeNumber(nodeinx), GetCoordCount(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY, Nodes[nodeinx]->Coords.back().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.back().bufY); // NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
    else                                                                                                                                                                                                                                                                                         // just show singleton
        if (IsCustom())
            return fmt::format("{}: @{}{}", GetNodeNumber(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY);
        else
            return fmt::format("{}: @({},{})", GetNodeNumber(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY);
}

// extract first (X,Y) from string formatted above:
bool Model::ParseFaceElement(const std::string& multi_str, std::vector<xlPoint>& first_xy)
{
    //    first_xy->x = first_xy->y = 0;
    //    first_xy.clear();
    std::string_view sv(multi_str);
    size_t tpos = 0;
    while (tpos != std::string::npos && tpos <= sv.size()) {
        size_t tnext = sv.find('+', tpos);
        std::string_view token = sv.substr(tpos, tnext == std::string::npos ? tnext : tnext - tpos);
        tpos = (tnext == std::string::npos) ? tnext : tnext + 1;

        std::string str(token);
        if (str.empty())
            continue;
        auto atpos = str.find('@');
        if (atpos == std::string::npos)
            continue; // return false;

        std::string xystr = str.substr(atpos + 1);
        if (xystr.empty())
            continue; // return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.erase(0, 1);
            auto commapos = xystr.find(',');
            if (commapos == std::string::npos)
                continue;
            xval = std::strtol(xystr.substr(0, commapos).c_str(), nullptr, 10);
            auto parenpos = xystr.find(')', commapos + 1);
            if (parenpos == std::string::npos)
                continue;
            yval = std::strtol(xystr.substr(commapos + 1, parenpos - commapos - 1).c_str(), nullptr, 10);
        } else {
            int parts = 0;
            size_t pos = 0;
            while (pos < xystr.size() && (xystr[pos] >= 'A') && (xystr[pos] <= 'Z')) {
                xval *= 26;
                xval += xystr[pos] - 'A' + 1;
                ++pos;
                parts |= 1;
            }
            while (pos < xystr.size() && (xystr[pos] >= '0') && (xystr[pos] <= '9')) {
                yval *= 10;
                yval += xystr[pos] - '0';
                ++pos;
                parts |= 2;
            }
            if (parts != 3)
                continue; // return false;
            if (pos < xystr.size() && (xystr[pos] != '-'))
                continue; // return false;
        }
        xlPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); // true;
}

// extract first (X,Y) from string formatted above:
bool Model::ParseStateElement(const std::string& multi_str, std::vector<xlPoint>& first_xy)
{
    //    first_xy->x = first_xy->y = 0;
    //    first_xy.clear();
    std::string_view sv(multi_str);
    size_t tpos = 0;
    while (tpos != std::string::npos && tpos <= sv.size()) {
        size_t tnext = sv.find('+', tpos);
        std::string_view token = sv.substr(tpos, tnext == std::string::npos ? tnext : tnext - tpos);
        tpos = (tnext == std::string::npos) ? tnext : tnext + 1;

        std::string str(token);
        if (str.empty())
            continue;
        auto atpos = str.find('@');
        if (atpos == std::string::npos)
            continue; // return false;

        std::string xystr = str.substr(atpos + 1);
        if (xystr.empty())
            continue; // return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.erase(0, 1);
            auto commapos = xystr.find(',');
            if (commapos == std::string::npos)
                continue;
            xval = std::strtol(xystr.substr(0, commapos).c_str(), nullptr, 10);
            auto parenpos = xystr.find(')', commapos + 1);
            if (parenpos == std::string::npos)
                continue;
            yval = std::strtol(xystr.substr(commapos + 1, parenpos - commapos - 1).c_str(), nullptr, 10);
        } else {
            int parts = 0;
            size_t pos = 0;
            while (pos < xystr.size() && (xystr[pos] >= 'A') && (xystr[pos] <= 'Z')) {
                xval *= 26;
                xval += xystr[pos] - 'A' + 1;
                ++pos;
                parts |= 1;
            }
            while (pos < xystr.size() && (xystr[pos] >= '0') && (xystr[pos] <= '9')) {
                yval *= 10;
                yval += xystr[pos] - '0';
                ++pos;
                parts |= 2;
            }
            if (parts != 3)
                continue; // return false;
            if (pos < xystr.size() && (xystr[pos] != '-'))
                continue; // return false;
        }
        xlPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); // true;
}

void Model::ExportAsCustomXModel(BaseSerializingVisitor& visitor) const
{
    // Calculate node grid from screen coordinates
    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    size_t nodeCount = GetNodeCount();
    for (size_t i = 0; i < nodeCount; ++i) {
        float Sbufx = Nodes[i]->Coords[0].screenX;
        float Sbufy = Nodes[i]->Coords[0].screenY;
        if (Sbufx < minsx) minsx = Sbufx;
        if (Sbufx > maxsx) maxsx = Sbufx;
        if (Sbufy < minsy) minsy = Sbufy;
        if (Sbufy > maxsy) maxsy = Sbufy;
    }

    int scale = 1;
    while (!FindCustomModelScale(scale)) {
        ++scale;
        if (scale > 100) { // I(Scott) am afraid of infinite while loops
            scale = 1;
            break;
        }
    }

    int minx = std::floor(minsx);
    int miny = std::floor(minsy);
    int maxx = std::ceil(maxsx);
    int maxy = std::ceil(maxsy);
    int sizex = (maxx - minx + 1) * scale;
    int sizey = (maxy - miny + 1) * scale;

    int* nodeLayout = (int*)malloc(sizey * sizex * sizeof(int));
    memset(nodeLayout, 0xFF, sizey * sizex * sizeof(int));

    for (int i = 0; i < (int)nodeCount; ++i) {
        int x = (Nodes[i]->Coords[0].screenX - minx) * scale;
        int y = (sizey - ((Nodes[i]->Coords[0].screenY - miny) * scale) - 1);
        nodeLayout[y * sizex + x] = i + 1;
    }

    std::vector<std::vector<std::vector<int>>> data;
    auto layer = std::vector<std::vector<int>>();
    for (int y = 0; y < sizey; ++y) {
        std::vector<int> row;
        for (int x = 0; x < sizex; ++x) {
            row.push_back(nodeLayout[y * sizex + x]);
        }
        layer.push_back(row);
    }
    data.push_back(layer);
    free(nodeLayout);

    // Build the DefaultRenderBuffer submodel node array
    int buffW = GetDefaultBufferWi();
    int buffH = GetDefaultBufferHt();
    std::vector<std::vector<std::string>> nodearray(buffH, std::vector<std::string>(buffW, ""));
    for (uint32_t i = 0; i < nodeCount; ++i) {
        int bufx = Nodes[i]->Coords[0].bufX;
        int bufy = Nodes[i]->Coords[0].bufY;
        if (bufy >= 0 && bufy < (int)nodearray.size() &&
            bufx >= 0 && bufx < (int)nodearray[bufy].size()) {
            nodearray[bufy][bufx] = std::to_string(i + 1);
        }
    }

    // Build root element attributes
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name",        GetName());
    attrs.Add("CustomWidth",  std::to_string(sizex));
    attrs.Add("CustomHeight", std::to_string(sizey));
    attrs.Add("Depth",       "1");
    attrs.Add("StringType",  GetStringType());
    attrs.Add("Transparency", std::to_string(GetTransparency()));
    attrs.Add("PixelSize",   std::to_string(GetPixelSize()));
    attrs.Add("Antialias",   std::to_string((int)GetPixelStyle()));
    attrs.Add("StrandNames", GetStrandNames());
    attrs.Add("NodeNames",   GetNodeNames());
    if (!GetPixelCount().empty())   attrs.Add("PixelCount",   GetPixelCount());
    if (!GetPixelType().empty())    attrs.Add("PixelType",    GetPixelType());
    if (!GetPixelSpacing().empty()) attrs.Add("PixelSpacing", GetPixelSpacing());
    attrs.Add("LayoutGroup",  GetLayoutGroup());
    attrs.Add("CustomModel",  CustomModel::ToCustomModel(data));
    attrs.Add("CustomModelCompressed", CustomModel::ToCompressed(data));
    attrs.Add("SourceVersion", xlights_version_string);
    visitor.AddSuperStrings(*this, attrs);

    visitor.WriteOpenTag("custommodel", attrs);

    // Write faces and states child elements
    visitor.WriteFacesAndStates(this);

    // Write DefaultRenderBuffer submodel
    {
        BaseSerializingVisitor::AttrCollector bufAttrs;
        bufAttrs.Add("name",   "DefaultRenderBuffer");
        bufAttrs.Add("layout", "horizontal");
        bufAttrs.Add("type",   "ranges");
        for (int x = 0; x < (int)nodearray.size(); ++x) {
            bufAttrs.Add("line" + std::to_string(x), NodeUtils::CompressNodes(Join(nodearray[x], ",")));
        }
        visitor.WriteOpenTag("subModel", bufAttrs, true);
    }

    // Write remaining submodels
    visitor.WriteSubmodels(this);

    // Write dimensions element (when ruler is active)
    visitor.WriteDimensionsElement(*this);

    visitor.WriteCloseTag();
}

std::string Model::ExportSuperStringColors() const
{
    if (superStringColours.size() == 0) {
        return "";
    }
    std::string colors;
    for (int i = 0; i < (int)superStringColours.size(); ++i) {
        std::string c = superStringColours[i];
        colors += "SuperStringColour" + std::to_string(i) + "=\"" + c + "\" ";
    }
    return colors;
}

void Model::ImportSuperStringColours(pugi::xml_node root)
{
    bool found = true;
    int index = 0;
    while (found) {
        auto an = fmt::format("SuperStringColour{}", index);
        auto attr = root.attribute(an);
        if (!attr.empty()) {
            superStringColours.push_back(xlColor(std::string(attr.as_string())));
        } else {
            found = false;
        }

        index++;
    }
}

bool Model::FindCustomModelScale(int scale) const
{
    size_t nodeCount = GetNodeCount();
    if (nodeCount <= 1) {
        return true;
    }
    for (int i = 0; i < (int)nodeCount; ++i) {
        for (int j = i + 1; j < (int)nodeCount; ++j) {
            int x1 = (Nodes[i]->Coords[0].screenX * scale);
            int y1 = (Nodes[i]->Coords[0].screenY * scale);
            int x2 = (Nodes[j]->Coords[0].screenX * scale);
            int y2 = (Nodes[j]->Coords[0].screenY * scale);
            if (x1 == x2 && y1 == y2) {
                return false;
            }
        }
    }
    return true;
}

std::string Model::GetStartLocation() const
{
    if (!IsLtoR) {
        if (!isBotToTop)
            return "Top Right";
        else
            return "Bottom Right";
    } else {
        if (!isBotToTop)
            return "Top Left";
        else
            return "Bottom Left";
    }
}

std::string Model::ChannelLayoutHtml(OutputManager* outputManager, bool darkMode)
{
    size_t NodeCount = GetNodeCount();

    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi, 0);

    std::string direction = GetStartLocation();

    int32_t sc;
    Controller* c = outputManager->GetController(this->GetFirstChannel() + 1, sc);

    std::string html = "<html><body><table border=0>";
    html += "<tr><td>Name:</td><td>" + name + "</td></tr>";
    html += "<tr><td>Display As:</td><td>" + DisplayAsTypeToString(DisplayAs) + "</td></tr>";
    html += "<tr><td>String Type:</td><td>" + StringType + "</td></tr>";
    html += "<tr><td>Start Corner:</td><td>" + direction + "</td></tr>";
    html += fmt::format("<tr><td>Total nodes:</td><td>{}</td></tr>", NodeCount);
    html += fmt::format("<tr><td>Height:</td><td>{}</td></tr>", BufferHt);

    if (c != nullptr) {
        html += fmt::format("<tr><td>Controller:</td><td>{}</td></tr>", c->GetLongDescription());
    }

    if (GetControllerProtocol() != "") {
        html += fmt::format("<tr><td>Pixel protocol:</td><td>{}</td></tr>", GetControllerProtocol());
        if (GetNumStrings() == 1) {
            html += fmt::format("<tr><td>Controller Connection:</td><td>{}</td></tr>", GetControllerPort(1));
        } else {
            html += fmt::format("<tr><td>Controller Connections:</td><td>{}-{}</td></tr>", GetControllerPort(1), GetControllerPort(GetNumPhysicalStrings()));
        }
    }
    html += "</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

   for (size_t i = 0; i < NodeCount; ++i) {
        size_t idx = Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
        if (idx < chmap.size()) {
            chmap[idx] = i + 1;
        }
    }
    for (int y = BufferHt - 1; y >= 0; y--) {
        html += "<tr>";
        for (int x = 0; x < BufferWi; ++x) {
            int n = chmap[y * BufferWi + x];
            if (n == 0) {
                html += "<td>&nbsp&nbsp&nbsp</td>";
            } else {
                int s = Nodes[n - 1]->StringNum + 1;
                std::string bgcolor = (s % 2 == 1) ? "#ADD8E6" : "#90EE90";
                if (darkMode)
                    bgcolor = (s % 2 == 1) ? "#3F7C85" : "#962B09";
                while (n > NodesPerString()) {
                    n -= NodesPerString();
                }
                html += fmt::format("<td bgcolor='{}'>n{}s{}</td>", bgcolor, n, s);
            }
        }
        html += "</tr>";
    }
    html += "</table></body></html>";
    return html;
}

// initialize screen coordinates
void Model::CopyBufCoord2ScreenCoord()
{
    size_t NodeCount = GetNodeCount();
    int xoffset = BufferWi / 2;
    int yoffset = BufferHt / 2;
    for (size_t n = 0; n < NodeCount; ++n) {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; ++c) {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY - yoffset;
        }
    }
    GetModelScreenLocation().SetRenderSize(BufferWi, BufferHt, GetModelScreenLocation().GetRenderDp());
}

bool Model::HitTest(IModelPreview* preview, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    return GetModelScreenLocation().HitTest(ray_origin, ray_direction);
}

CursorType Model::InitializeLocation(int& handle, int x, int y, IModelPreview* preview)
{
    return GetModelScreenLocation().InitializeLocation(handle, x, y, Nodes, preview);
}

void Model::ApplyTransparency(xlColor& color, int transparency, int blackTransparency)
{
    int colorAlpha = 255;
    if (color == xlBLACK) {
        if (blackTransparency) {
            float t = 100.0f - blackTransparency;
            t *= 2.55f;
            int i = std::floor(t);
            colorAlpha = i > 255 ? 255 : (i < 0 ? 0 : i);
        }
    } else if (transparency || blackTransparency) {
        int maxCol = std::max(color.red, std::max(color.green, color.blue));
        if (transparency) {
            float t = 100.0f - transparency;
            t *= 2.55f;
            colorAlpha = std::floor(t);
        }
        if (maxCol < 64 && blackTransparency) {
            // if we're getting close to black, we'll start migrating toward the black's transparency setting
            float t = 100.0f - blackTransparency;
            t *= 2.55f;
            int blackAlpha = std::floor(t);
            t = maxCol * colorAlpha + (64 - maxCol) * blackAlpha;
            t /= 64;
            colorAlpha = std::floor(t);
        }
    }
    color.alpha = colorAlpha;
}

void Model::DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solidProgram, xlGraphicsProgram* transparentProgram, bool is_3d,
                                 const xlColor* c, bool allowSelected, bool wiring, bool highlightFirst, int highlightpixel,
                                 float* boundingBox)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }
    size_t NodeCount = Nodes.size();
    xlColor color;
    xlColor saveColor;
    if (c != nullptr) {
        color = *c;
        saveColor = *c;
    }

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    ModelScreenLocation& screenLocation = GetModelScreenLocation();
    screenLocation.PrepareToDraw(is_3d, allowSelected);

    const std::string& cacheKey = allowSelected
                                      ? (is_3d ? LAYOUT_PREVIEW_CACHE_3D : LAYOUT_PREVIEW_CACHE_2D)
                                      : (is_3d ? MODEL_PREVIEW_CACHE_3D : MODEL_PREVIEW_CACHE_2D);
    if (uiObjectsInvalid) {
        deleteUIObjects();
    }
    bool created = false;
    auto cache = uiCaches[cacheKey];
    // Circle styles bake getBackingScaleFactor() into the geometry; rebuild if it changed.
    if (cache != nullptr &&
        (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) &&
        cache->backingScaleFactor != (float)preview->getBackingScaleFactor()) {
        delete cache;
        uiCaches[cacheKey] = nullptr;
        cache = nullptr;
    }

    // Depth sort applies to 3D previews with non-SQUARE pixel styles. The sort axis is
    // the 3rd row of (ViewMatrix * ModelMatrix) — for any node at local (x, y, z) the
    // camera-space Z is axis.dot(x, y, z) + const, so sorting nodes by that dot product
    // gives back-to-front order for the current camera orientation.
    const bool depthSort = is_3d && _pixelStyle != PIXEL_STYLE::PIXEL_STYLE_SQUARE;
    glm::vec3 currentSortAxis(0.0f);
    if (depthSort) {
        glm::mat4 mv = preview->GetViewMatrix() * screenLocation.GetModelMatrix();
        currentSortAxis = glm::vec3(mv[0][2], mv[1][2], mv[2][2]);
    }
    // Rebuild the cache if the camera has rotated enough that the baked node order no
    // longer composites correctly. cos(~14°) ≈ 0.97 — a small enough threshold that drift
    // during a rotation drag is hard to see, large enough to avoid rebuilding every frame.
    if (cache != nullptr && depthSort &&
        glm::dot(cache->viewSortAxis, cache->viewSortAxis) > 0.0f) {
        float d = glm::dot(glm::normalize(cache->viewSortAxis),
                           glm::normalize(currentSortAxis));
        if (d < 0.97f) {
            delete cache;
            uiCaches[cacheKey] = nullptr;
            cache = nullptr;
        }
    }
    if (cache == nullptr) {
        screenLocation.UpdateBoundingBox(Nodes);
        cache = new PreviewGraphicsCacheInfo();
        uiCaches[cacheKey] = cache;
        created = true;

        size_t vcount = 0;
        for (const auto& it : Nodes) {
            vcount += it.get()->Coords.size();
        }
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            int f = pixelSize;
            if (pixelSize < 16) {
                f = 16;
            }
            vcount = vcount * f * 3;
        }
        if (vcount > maxVertexCount) {
            maxVertexCount = vcount;
        }
        bool needTransparent = false;
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE || transparency != 0 || blackTransparency != 0) {
            needTransparent = true;
        }
        cache->isTransparent = needTransparent;
        cache->backingScaleFactor = (float)preview->getBackingScaleFactor();
        cache->program = ctx->createGraphicsProgram();
        cache->vica = ctx->createVertexIndexedColorAccumulator();
        cache->vica->SetName(GetName() + (is_3d ? " - 3DPreview" : " - 2DPreview"));
        cache->vica->PreAlloc(vcount);
        cache->vica->SetColorCount(_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE ? NodeCount * 2 : NodeCount);

        float modelPixelSize = pixelSize;
        // pixelSize is in world coordinate sizes, not model size.  Thus, we need to reverse the matrices to
        // get the size to use for the pixelStyle 3/4 that use triangles.
        // Use getBackingScaleFactor() rather than calcPixelSize() to avoid double-applying the
        // view-matrix scale (scale2d) that is already applied by the camera transform.
        // The factor of 2 converts from modelPixelSize (diameter in local coords) to a radius
        // that, after ApplyModelViewMatrices (×scalex) and the ViewMatrix (×zoom×scale2d), equals
        // half the GL_POINTS diameter: backingScale × pixelSize × zoom × scale2d / 2.
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            float x1 = -1, y1 = -1, z1 = -1;
            float x2 = 1, y2 = 1, z2 = 1;
            GetModelScreenLocation().TranslatePoint(x1, y1, z1);
            GetModelScreenLocation().TranslatePoint(x2, y2, z2);

            glm::vec3 a = glm::vec3(x2, y2, z2) - glm::vec3(x1, y1, z1);
            float length = std::max(std::max(std::abs(a.x), std::abs(a.y)), std::abs(a.z));
            modelPixelSize = 2.0f * (float)pixelSize * (float)preview->getBackingScaleFactor() / std::abs(length);
        }

        int nodeRenderOrder = NodeRenderOrder();
        std::vector<int> nodeOrder;
        nodeOrder.reserve(NodeCount);
        if (depthSort) {
            // Precompute a finite sort key per node. Doing the comparator-time work
            // up front keeps std::sort's comparator a trivial float compare, so no
            // NaN or pointer-chasing inside the comparator can violate strict-weak
            // ordering (which would make libc++ read past the range and crash).
            // Nodes with empty Coords contribute no geometry; skip them entirely.
            const glm::vec3 axis = currentSortAxis;
            std::vector<std::pair<float, int>> keys;
            keys.reserve(NodeCount);
            for (int n = 0; n < (int)NodeCount; ++n) {
                if (!Nodes[n] || Nodes[n]->Coords.empty()) {
                    continue;
                }
                const auto& c = Nodes[n]->Coords[0];
                float z = axis.x * c.screenX + axis.y * c.screenY + axis.z * c.screenZ;
                if (!std::isfinite(z)) {
                    z = 0.0f;
                }
                keys.emplace_back(z, n);
            }
            // Sort ascending by camera-space Z so the farthest node (most negative Z in the
            // OpenGL convention) renders first — i.e. back-to-front.
            std::sort(keys.begin(), keys.end(),
                      [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
                          return a.first < b.first;
                      });
            for (const auto& kv : keys) {
                nodeOrder.push_back(kv.second);
            }
            cache->viewSortAxis = currentSortAxis;
        } else {
            int first = 0;
            int last = NodeCount;
            int buffFirst = -1;
            int buffLast = -1;
            bool left = true;
            while (first < last) {
                int n;
                if (left) {
                    n = first;
                    first++;
                    if (nodeRenderOrder == 1) {
                        if (buffFirst == -1) {
                            buffFirst = Nodes[n]->Coords[0].bufX;
                        }
                        if (first < (int)NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                            left = false;
                        }
                    }
                } else {
                    last--;
                    n = last;
                    if (buffLast == -1) {
                        buffLast = Nodes[n]->Coords[0].bufX;
                    }
                    if (last > 0 && buffLast != Nodes[last - 1]->Coords[0].bufX) {
                        left = true;
                    }
                }
                nodeOrder.push_back(n);
            }
        }

        bool firstCoord = true;
        for (int n : nodeOrder) {
            size_t CoordCount = GetCoordCount(n);
            for (size_t c2 = 0; c2 < CoordCount; ++c2) {
                // draw node on screen
                float sx = Nodes[n]->Coords[c2].screenX;
                float sy = Nodes[n]->Coords[c2].screenY;
                float sz = Nodes[n]->Coords[c2].screenZ;

                if (firstCoord) {
                    cache->boundingBox[0] = sx;
                    cache->boundingBox[1] = sy;
                    cache->boundingBox[2] = sz;
                    cache->boundingBox[3] = sx;
                    cache->boundingBox[4] = sy;
                    cache->boundingBox[5] = sz;
                    firstCoord = false;
                } else {
                    cache->boundingBox[0] = std::min(sx, cache->boundingBox[0]);
                    cache->boundingBox[1] = std::min(sy, cache->boundingBox[1]);
                    cache->boundingBox[2] = std::min(sz, cache->boundingBox[2]);
                    cache->boundingBox[3] = std::max(sx, cache->boundingBox[3]);
                    cache->boundingBox[4] = std::max(sy, cache->boundingBox[4]);
                    cache->boundingBox[5] = std::max(sz, cache->boundingBox[5]);
                }
                if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH) {
                    cache->vica->AddVertex(sx, sy, sz, n);
                } else {
                    int eidx = n;
                    if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                        eidx += NodeCount;
                    }
                    cache->vica->AddCircleAsTriangles(sx, sy, sz, ((float)modelPixelSize) / 2.0f, n, eidx, pixelSize);
                }
            }
        }
        cache->program->addStep([=, this](xlGraphicsContext* ctx) {
            if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                ctx->drawTriangles(cache->vica, 0, cache->vica->getCount());
            } else {
                IModelPreview* preview = static_cast<IModelPreview*>(ctx->getContextualValue("modelPreview"));
                float pointSize = preview->calcPixelSize(pixelSize) * preview->getViewScale();
                ctx->drawPoints(cache->vica, pointSize, _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, 0, cache->vica->getCount());
            }
        });
    }
    for (int n = 0; n < (int)NodeCount; ++n) {
        if (n + 1 == highlightpixel) {
            color = xlMAGENTA;
        } else if (highlightFirst && Nodes.size() > 1) {
            if (IsNodeFirst(n)) {
                color = xlCYAN;
            } else {
                color = saveColor;
            }
        } else if (c == nullptr) {
            Nodes[n]->GetColor(color);
            if (Nodes[n]->model->modelDimmingCurve != nullptr) {
                Nodes[n]->model->modelDimmingCurve->reverse(color);
            }
            if (Nodes[n]->model->StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
        }
        ApplyTransparency(color, transparency, blackTransparency);
        cache->vica->SetColor(n, color);
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            xlColor c2(color);
            c2.alpha = 0;
            cache->vica->SetColor(n + NodeCount, c2);
        }
    }
    if (created) {
        cache->vica->Finalize(false, true);
    } else {
        cache->vica->FlushColors(0, _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE ? NodeCount * 2 : NodeCount);
    }
    if (boundingBox) {
        boundingBox[0] = cache->boundingBox[0];
        boundingBox[1] = cache->boundingBox[1];
        boundingBox[2] = cache->boundingBox[2];
        boundingBox[3] = cache->boundingBox[3];
        boundingBox[4] = cache->boundingBox[4];
        boundingBox[5] = cache->boundingBox[5];
    }
    xlGraphicsProgram* p = cache->isTransparent ? transparentProgram : solidProgram;
    if (wiring && NodeCount > 1 && cache->va == nullptr) {
        cache->va = ctx->createVertexAccumulator();
        cache->va->SetName(GetName() + (is_3d ? " - 3DPWiring" : " - 2DWiring"));
        cache->va->PreAlloc(NodeCount);
        for (int x = 0; x < (int)NodeCount; ++x) {
            float sx = Nodes[x]->Coords[0].screenX;
            float sy = Nodes[x]->Coords[0].screenY;
            float sz = Nodes[x]->Coords[0].screenZ;
            cache->va->AddVertex(sx, sy, sz);
        }
        cache->va->Finalize(false);
    }

    p->addStep([=, this](xlGraphicsContext* ctx) {
        // cache has the model in model coordinates
        // we need to scale/translate/etc.... to world
        ctx->PushMatrix();
        if (!is_3d) {
            // not 3d, flatten to the 0 plane
            ctx->Scale(1.0, 1.0, 0.0);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
        cache->program->runSteps(ctx);
        if (wiring && cache->va != nullptr) {
            ctx->drawLineStrip(cache->va, xlColor(0x49, 0x80, 0x49));
        }
        ctx->PopMatrix();
    });

    if ((Selected() || (Highlighted() && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted(), IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

float Model::GetPreviewDimScale(IModelPreview* preview, int& w, int& h)
{
    w = preview->getWidth();
    h = preview->getHeight();
    float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
    float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;
    float scale = scaleY < scaleX ? scaleY : scaleX;

    return scale;
}

void Model::GetScreenLocation(float& sx, float& sy, const NodeBaseClass::CoordStruct& it2, int w, int h, float scale)
{
    sx = it2.screenX;
    sy = it2.screenY;

    if (!GetModelScreenLocation().IsCenterBased()) {
        sx -= GetModelScreenLocation().RenderWi / 2.0;
        sy *= GetModelScreenLocation().GetVScaleFactor();
        if (GetModelScreenLocation().GetVScaleFactor() < 0) {
            sy += GetModelScreenLocation().RenderHt / 2.0;
        } else {
            sy -= GetModelScreenLocation().RenderHt / 2.0;
        }
        sy = ((sy * scale) + (h / 2));
        sx = (sx * scale) + (w / 2);
    } else {
        // Must match DisplayEffectOnWindow's Translate(w/2 - ml*scale, h/2 - mb*scale)
        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;
        sx = ((sx - ml) * scale) + (w / 2);
        sy = ((sy - mb) * scale) + (h / 2);
    }
}

std::string Model::GetNodeNear(IModelPreview* preview, xlPoint pt, bool flip)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    float pointScale = scale;
    if (pointScale > 2.5) {
        pointScale = 2.5;
    }
    if (pointScale > GetModelScreenLocation().RenderHt) {
        pointScale = GetModelScreenLocation().RenderHt;
    }
    if (pointScale > GetModelScreenLocation().RenderWi) {
        pointScale = GetModelScreenLocation().RenderWi;
    }

    float px = pt.x;
    float py = pt.y;
    if (flip)
        py = h - pt.y;

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);

            if (sx >= (px - pointScale) && sx <= (px + pointScale) &&
                sy >= (py - pointScale) && sy <= (py + pointScale)) {
                return std::to_string(i);
            }
        }
        i++;
    }
    return "";
}

bool Model::GetScreenLocations(IModelPreview* preview, std::map<int, std::pair<float, float>>& coords)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        if (c.size() != 1)
            return false;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);
            coords[i] = std::make_pair(sx, sy);
        }
        ++i;
    }
    return true;
}

std::vector<int> Model::GetNodesInBoundingBox(IModelPreview* preview, xlPoint start, xlPoint end)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    std::vector<int> nodes;

    float startpx = start.x;
    float startpy = /*h -*/ start.y;
    float endpx = end.x;
    float endpy = /*h -*/ end.y;

    if (startpx > endpx) {
        float tmp = startpx;
        startpx = endpx;
        endpx = tmp;
    }

    if (startpy > endpy) {
        float tmp = startpy;
        startpy = endpy;
        endpy = tmp;
    }

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);

            if (sx >= startpx && sx <= endpx &&
                sy >= startpy && sy <= endpy) {
                nodes.push_back(i);
            }
        }
        i++;
    }
    return nodes;
}

bool Model::IsMultiCoordsPerNode() const
{
    for (const auto& it : Nodes) {
        if (it.get()->Coords.size() > 1)
            return true;
    }
    return false;
}

void Model::DisplayEffectOnWindow(IModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool mustEnd = false;
    xlGraphicsContext* ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }

    if (ctx) {
        if (uiObjectsInvalid) {
            deleteUIObjects();
        }

        int w, h;
        float scale = GetPreviewDimScale(preview, w, h);

        size_t NodeCount = Nodes.size();
        bool created = false;

        int renderWi = GetModelScreenLocation().RenderWi;
        int renderHi = GetModelScreenLocation().RenderHt;

        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        auto cache = uiCaches[EFFECT_PREVIEW_CACHE];
        // Circle styles bake the radius (which depends on scale/w/h/backingScale) into the geometry,
        // so we must also invalidate the cache when the preview panel size or backing scale changes.
        if (cache == nullptr || cache->renderWi != renderWi || cache->renderHi != renderHi || cache->modelChangeCount != (int)this->changeCount || cache->width != w || cache->height != h ||
            cache->backingScaleFactor != (float)preview->getBackingScaleFactor()) {
            if (cache != nullptr) {
                delete cache;
            }
            cache = new PreviewGraphicsCacheInfo();
            uiCaches[EFFECT_PREVIEW_CACHE] = cache;

            cache->width = w;
            cache->height = h;
            cache->renderWi = renderWi;
            cache->renderHi = renderHi;
            cache->modelChangeCount = this->changeCount;
            cache->backingScaleFactor = (float)preview->getBackingScaleFactor();

            created = true;

            float pointScale = scale;
            if (pointScale > 2.5) {
                pointScale = 2.5;
            }
            if (pointScale > GetModelScreenLocation().RenderHt) {
                pointScale = GetModelScreenLocation().RenderHt;
            }
            if (pointScale > GetModelScreenLocation().RenderWi) {
                pointScale = GetModelScreenLocation().RenderWi;
            }

            PIXEL_STYLE lastPixelStyle = _pixelStyle;
            int lastPixelSize = pixelSize;

            // layer calculation and map to output
            unsigned int vcount = 0;
            for (const auto& it : Nodes) {
                vcount += it.get()->Coords.size();
            }
            if (vcount > maxVertexCount) {
                maxVertexCount = vcount;
            }

            cache->vica = ctx->createVertexIndexedColorAccumulator();
            cache->vica->SetName(GetName() + " - Preview");
            cache->program = ctx->createGraphicsProgram();

            cache->vica->SetColorCount(NodeCount * 2); // upper one is for the clear edges of blended circles
            cache->vica->PreAlloc(maxVertexCount);

            int startVertex = 0;

            int first = 0;
            int last = NodeCount;
            int buffFirst = -1;
            int buffLast = -1;
            bool left = true;
            
            int nodeRenderOrder = NodeRenderOrder();
            while (first < last) {
                int n;
                if (left) {
                    n = first;
                    ++first;
                    if (nodeRenderOrder == 1) {
                        if (buffFirst == -1) {
                            buffFirst = Nodes[n]->Coords[0].bufX;
                        }
                        if (first < (int)NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                            left = false;
                        }
                    }
                } else {
                    --last;
                    n = last;
                    if (buffLast == -1) {
                        buffLast = Nodes[n]->Coords[0].bufX;
                    }
                    if (last > 0 && buffLast != Nodes[last - 1]->Coords[0].bufX) {
                        left = true;
                    }
                }

                size_t CoordCount = GetCoordCount(n);
                for (size_t c = 0; c < CoordCount; ++c) {
                    // draw node on screen
                    float newsx = Nodes[n]->Coords[c].screenX;
                    float newsy = Nodes[n]->Coords[c].screenY;

                    if (lastPixelStyle != Nodes[n]->model->_pixelStyle || lastPixelSize != Nodes[n]->model->pixelSize) {
                        if (cache->vica->getCount() && (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                                                        lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH ||
                                                        Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                                                        Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH)) {
                            int count = cache->vica->getCount();
                            cache->program->addStep([=, this](xlGraphicsContext* ctx) {
                                if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                                    ctx->drawTriangles(cache->vica, startVertex, count - startVertex);
                                } else {
                                    IModelPreview* preview = static_cast<IModelPreview*>(ctx->getContextualValue("modelPreview"));
                                    float pointSize = preview->calcPixelSize(lastPixelSize * pointScale);
                                    ctx->drawPoints(cache->vica, pointSize, lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, startVertex, count - startVertex);
                                }
                            });
                            startVertex = count;
                        }
                        lastPixelStyle = Nodes[n]->model->_pixelStyle;
                        lastPixelSize = Nodes[n]->model->pixelSize;
                    }

                    if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                        lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH) {
                        cache->vica->AddVertex(newsx, newsy, n);
                    } else {
                        uint32_t ecolor = n;
                        if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                            ecolor += NodeCount;
                        }
                        // Radius must equal GL_POINTS radius in window pixels, converted to local coords.
                        // GL_POINTS diameter = calcPixelSize(pixelSize*pointScale) ≈ backingScale*pixelSize*pointScale.
                        // After CTX Scale(scale), local_radius * scale = window_pixel_radius.
                        // So: local_radius = backingScale * pixelSize * pointScale / (2 * scale).
                        float circleRadius = (float)preview->getBackingScaleFactor() * lastPixelSize * pointScale / (2.0f * scale);
                        cache->vica->AddCircleAsTriangles(newsx, newsy, 0, circleRadius, n, ecolor);
                    }
                }
            }
            if (int(cache->vica->getCount()) > startVertex) {
                int count = cache->vica->getCount();
                cache->program->addStep([=, this](xlGraphicsContext* ctx) {
                    if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                        ctx->drawTriangles(cache->vica, startVertex, count - startVertex);
                    } else {
                        IModelPreview* preview = static_cast<IModelPreview*>(ctx->getContextualValue("modelPreview"));
                        float pointSize = preview->calcPixelSize(lastPixelSize * pointScale);
                        ctx->drawPoints(cache->vica, pointSize, lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, startVertex, count - startVertex);
                    }
                });
            }
        }
        int maxFlush = (int)NodeCount;
        for (int n = 0; n < (int)NodeCount; ++n) {
            xlColor color;
            Nodes[n]->GetColor(color);
            if (Nodes[n]->model->modelDimmingCurve != nullptr) {
                Nodes[n]->model->modelDimmingCurve->reverse(color);
            }
            if (Nodes[n]->model->StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
            xlColor c2(color);
            ApplyTransparency(c2, Nodes[n]->model->transparency, Nodes[n]->model->blackTransparency);
            cache->vica->SetColor(n, c2);
            if (Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                c2.alpha = 0;
                cache->vica->SetColor(n + NodeCount, c2);
                maxFlush = n + NodeCount + 1;
            }
        }
        if (created) {
            cache->vica->Finalize(false, true);
        } else {
            cache->vica->FlushColors(0, maxFlush);
        }

        preview->getCurrentSolidProgram()->addStep([=, this](xlGraphicsContext* ctx) {
            // cache has the model in model coordinates
            // we need to scale/translate/etc.... to world
            ctx->PushMatrix();
            if (!GetModelScreenLocation().IsCenterBased()) {
                // Non-center-based models (e.g. polylines) have screenX/Y in [0, RenderWi/RenderHt].
                // The inner translate centers the model at origin, so the outer translate is just
                // the panel center — no ml/mb offset needed (ml would shift it to lower-left).
                ctx->Translate(w / 2.0f, h / 2.0f, 0.0f);
                ctx->Scale(scale, scale, 1.0);
                ctx->Translate(-GetModelScreenLocation().RenderWi / 2.0,
                               GetModelScreenLocation().GetVScaleFactor() < 0 ? GetModelScreenLocation().RenderHt / 2.0 : -GetModelScreenLocation().RenderHt / 2.0,
                               0.0f);
                ctx->Scale(1.0, GetModelScreenLocation().GetVScaleFactor(), 1.0);
            } else {
                ctx->Translate(w / 2.0f - ml * scale,
                               h / 2.0f - mb * scale, 0.0f);
                ctx->Scale(scale, scale, 1.0);
            }
            cache->program->runSteps(ctx);
            ctx->PopMatrix();
        });
        if (mustEnd) {
            preview->EndDrawing();
        }
    }
}

glm::vec3 Model::MoveHandle(IModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY, bool& update_rgbeffects)
{
    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return GetModelScreenLocation().GetHandlePosition(handle);

    int i = GetModelScreenLocation().MoveHandle(preview, handle, ShiftKeyPressed, mouseX, mouseY);
    if (i == MODEL_NEEDS_INIT) {
        Setup();
    } else if (i == MODEL_UPDATE_RGBEFFECTS) {
        update_rgbeffects = true;
    }
    IncrementChangeCount();

    return GetModelScreenLocation().GetHandlePosition(handle);
}

int Model::GetSelectedHandle()
{
    return GetModelScreenLocation().GetSelectedHandle();
}

int Model::GetNumHandles()
{
    return GetModelScreenLocation().GetNumHandles();
}

int Model::GetSelectedSegment()
{
    return GetModelScreenLocation().GetSelectedSegment();
}

bool Model::SupportsCurves()
{
    return GetModelScreenLocation().SupportsCurves();
}

bool Model::HasCurve(int segment)
{
    return GetModelScreenLocation().HasCurve(segment);
}

void Model::SetCurve(int segment, bool create)
{
    return GetModelScreenLocation().SetCurve(segment, create);
}

void Model::AddHandle(IModelPreview* preview, int mouseX, int mouseY)
{
    GetModelScreenLocation().AddHandle(preview, mouseX, mouseY);
}

void Model::InsertHandle(int after_handle, float zoom, int scale)
{
    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().InsertHandle(after_handle, zoom, scale);
}

void Model::DeleteHandle(int handle)
{
    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().DeleteHandle(handle);
}

int Model::GetStrandLength(int strand) const
{
    int numStrands = std::max(1, GetNumStrands());
    return GetNodeCount() / numStrands;
}

int Model::MapToNodeIndex(int strand, int node) const
{
    
    // if ((DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix") || DisplayAs == wxT("Matrix")) && SingleChannel) {
    //     return node;
    // }
    if (GetNumStrands() == 1) {
        return node;
    }
    if (SingleNode) {
        return strand;
    }
    int strandsPerStr = GetStrandsPerString();
    if (strandsPerStr == 0) {
        spdlog::critical("Map node to index with illegal strandsPerString = 0.");
        return node;
    }
    return (strand * NodesPerString() / strandsPerStr) + node;
}

void Model::RecalcStartChannels()
{
    modelManager.RecalcStartChannels();
}


std::string Model::GetDimension() const
{
    return GetModelScreenLocation().GetDimension();
}

Model* Model::CreateDefaultModelFromSavedModelNode(Model* model, pugi::xml_node node, ModelManager& modelMgr, bool& cancelled) const {

    // This code should work to import model with old formats as well as new Serializer formats
    auto n = node;
    if (std::string_view(node.name()) == "models") {
        n = node.first_child();
        if (!n) return model;
    }
    // grab the attributes I want to keep
    std::string sc = model->GetModelStartChannel();
    auto x = model->GetHcenterPos();
    auto y = model->GetVcenterPos();
    auto lg = model->GetLayoutGroup();

    XmlSerializer serializer;
    modelMgr.ClearUsedRuler();

    if (std::string_view(node.name()) == "custommodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        return model;
    } else if (std::string_view(node.name()) == "polylinemodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        return model;
    } else if (std::string_view(node.name()) == "multipointmodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        return model;
    } else if (std::string_view(node.name()) == "matrixmodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(1, 1);
        }
        return model;
    } else if (std::string_view(node.name()) == "archesmodel") {
        int l = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetLeft();
        int r = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetRight();
        int t = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetTop();
        int b = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetBottom();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr && !modelMgr.UsedRuler()) {
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetMWidth(std::abs(r - l));
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetRight(r);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetLeft(l);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetBottom(b);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetMHeight(2 * (float)std::abs(t - b) / (float)std::abs(r - l));
        }
    } else if (std::string_view(node.name()) == "starmodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(1, 1);
        return model;
    } else if (std::string_view(node.name()) == "treemodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(1, 1);
        return model;
    } else if (std::string_view(node.name()) == "dmxmodel") {
        // grab the attributes I want to keep
        auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
        auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if( model != nullptr ) {
            // Multiply by 5 because default custom model has width and height set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
        } else {
            cancelled = true;
        }
        return model;
    } else if (std::string_view(node.name()) == "dmxgeneral") {
        auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
        auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            // Multiply by 5 because default custom model has width and height set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
        }
        return model;
    } else if (std::string_view(node.name()) == "dmxservo") {
        // grab the attributes I want to keep
        std::string startChannel = model->GetModelStartChannel();
        auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
        auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            // Multiply by 5 because default custom model has width and height set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
        }
        return model;
    } else if (std::string_view(node.name()) == "dmxservo3axis" ||
               std::string_view(node.name()) == "dmxservo3d") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            model->GetModelScreenLocation().SetScaleMatrix(glm::vec3(1, 1, 1));
        }
        return model;
    } else if (std::string_view(node.name()) == "circlemodel") {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        return model;
    } else if (std::string_view(node.name()) == "spheremodel") {
        auto scale = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleMatrix();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScaleMatrix(scale);
        }
        return model;
    } else if (std::string_view(node.name()) == "iciclemodel") {
        auto scale = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleMatrix();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScaleMatrix(scale);
        }
        return model;
    } else if (std::string_view(node.name()) == "Cubemodel") {
        auto scale = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleMatrix();
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model != nullptr) {
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScaleMatrix(scale);
        }
    } else {
        if (model != nullptr) { delete model; }
        model = serializer.DeserializeModel(n, modelMgr, true);
        if (model == nullptr) {
            spdlog::error("GetXlightsModel no code to convert to " + std::string(node.name()));
            TraceLog::AddTraceMessage("GetXlightsModel no code to convert to " + std::string(node.name()));
            cancelled = true;
        }
    }

    if (model != nullptr) {
        model->SetStartChannel(sc);
        model->SetHcenterPos(x);
        model->SetVcenterPos(y);
        model->SetLayoutGroup(lg);
        model->Selected(true);
    }
    return model;
}

std::string Model::SerialiseSubmodel() const
{
    std::string res;

    const std::vector<Model*>& submodelList = GetSubModels();

    for (Model* s : submodelList) {
        const SubModel* submodel = dynamic_cast<const SubModel*>(s);
        if (submodel == nullptr) {
            continue; // Skip if not a valid SubModel
        }

        // Build the XML string manually
        res += "    <subModel";
        res += " name=\"" + XmlSafe(s->GetName()) + "\"";
        res += " layout=\"" + XmlSafe(submodel->GetSubModelLayout()) + "\"";
        res += " type=\"" + XmlSafe(submodel->GetSubModelType()) + "\"";

        std::string bufferStyle = submodel->GetSubModelBufferStyle();
        if (!bufferStyle.empty()) {
            res += " bufferstyle=\"" + XmlSafe(bufferStyle) + "\"";
        }

        // Add range or buffer data
        if (submodel->IsRanges()) {
            for (int x = 0; x < submodel->GetNumRanges(); ++x) {
                res += " line" + std::to_string(x) + "=\"" + XmlSafe(submodel->GetRange(x)) + "\"";
            }
        } else {
            res += " subBuffer=\"" + XmlSafe(submodel->GetSubModelLines()) + "\"";
        }

        // Check if we need to add aliases as child elements
        const std::list<std::string>& aliases = s->GetAliases();
        if (!aliases.empty()) {
            res += ">\n";
            res += "        <aliases>\n";
            for (const auto& alias : aliases) {
                res += "            <alias name=\"" + XmlSafe(alias) + "\"/>\n";
            }
            res += "        </aliases>\n";
            res += "    </subModel>\n";
        } else {
            res += " />\n";
        }
    }

    return res;
}

std::string Model::CreateBufferAsSubmodel() const
{
    int buffW = GetDefaultBufferWi();
    int buffH = GetDefaultBufferHt();
    std::vector<std::vector<std::string>> nodearray(buffH, std::vector<std::string>(buffW, ""));
    uint32_t nodeCount = GetNodeCount();
    for (uint32_t i = 0; i < nodeCount; ++i) {
        int bufx = Nodes[i]->Coords[0].bufX;
        int bufy = Nodes[i]->Coords[0].bufY;
        if (bufy > (int)nodearray.size() || bufy < 0) {
            continue;
        }
        if (bufx > (int)nodearray[bufy].size() || bufx < 0) {
            continue;
        }
        nodearray[bufy][bufx] = std::to_string(i + 1);
    }
    pugi::xml_document new_doc;
    pugi::xml_node child = new_doc.append_child("subModel");
    child.append_attribute("name") = "DefaultRenderBuffer";
    child.append_attribute("layout") = "horizontal";
    child.append_attribute("type") = "ranges";

    for (int x = 0; x < (int)nodearray.size(); ++x) {
        child.append_attribute(fmt::format("line{}", x)) = NodeUtils::CompressNodes(Join(nodearray[x], ","));
    }

    // Save just the node (not the xml declaration) to a string
    std::ostringstream oss;
    new_doc.save(oss, "  ", pugi::format_default | pugi::format_no_declaration);
    return oss.str();
}

std::list<std::string> Model::CheckModelSettings()
{
    std::list<std::string> res;

    if (modelDimmingCurve != nullptr) {
        auto dimmingInfo = GetDimmingInfo();
        if (dimmingInfo.size() > 0) {
            float maxGamma = 0.0;
            int maxBrightness = -100;
            for (auto& it : dimmingInfo) {
                maxGamma = std::max(maxGamma, (float)std::strtod(it.second["gamma"].c_str(), nullptr));
                maxBrightness = std::max(maxBrightness, (int)std::strtol(it.second["brightness"].c_str(), nullptr, 10));
            }
            if (maxGamma == 0.0) {
                res.push_back(fmt::format("    ERR: Model {} has a dimming curve gamma of 0.0 ... this will essentially blank the model so no effects will ever show on it.", GetName()));
            }
            if (maxBrightness == -100) {
                res.push_back(fmt::format("    ERR: Model {} has a dimming curve brightness of -100 ... this will essentially blank the model so no effects will ever show on it.", GetName()));
            }
        }
    }

    res.splice(res.end(), BaseObject::CheckModelSettings());
    return res;
}

std::string Model::GetTagColourAsString() const
{
    return _modelTagColourString;
}

xlColor Model::GetTagColour() {
    if (!_modelTagColourValid) {
        if (_modelTagColourString != xlEMPTY_STRING) {
            _modelTagColour = xlColor(_modelTagColourString);
        } else {
            _modelTagColourString = "#000000";
            _modelTagColour = xlBLACK;
        }
        _modelTagColourValid = true;
    }
    return _modelTagColour;
}

void Model::SetTagColourAsString(std::string const& colour) {
    _modelTagColourString = colour;
    _modelTagColourValid = false;
    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTagColourAsString");
}
void Model::SetTagColour(const xlColor& colour)
{
    _modelTagColour = colour;
    _modelTagColourValid = true;
    _modelTagColourString = std::string(colour);
    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTagColour");
}
void Model::SetPixelSize(int size)
{
    if (size != pixelSize) {
        pixelSize = size;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetPixelSize");
    }
}

void Model::SetTransparency(int t)
{
    if (t != transparency) {
        transparency = t;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTransparency");
    }
}

void Model::SetBlackTransparency(int t)
{
    if (t != blackTransparency) {
        blackTransparency = t;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetBlackTransparency");
    }
}

// Getter methods for export functionality
void Model::SetPixelStyle(PIXEL_STYLE style)
{
    if (_pixelStyle != style) {
        _pixelStyle = style;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetPixelStyle");
    }
}

int32_t Model::GetStringStartChan(int x) const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        if (x < (int)stringStartChan.size()) {
            return stringStartChan[x];
        }
        return 1;
    } else {
        int str = x * ts;
        if (str < (int)stringStartChan.size()) {
            return stringStartChan[str];
        }
        return 1;
    }
}

void Model::SetModelChain(const std::string& modelChain)
{
    

    std::string mc = modelChain;
    if (!mc.empty() && mc != "Beginning" && !StartsWith(mc, ">")) {
        mc = ">" + mc;
    }
    if (mc == "Beginning" || mc == ">") {
        mc = "";
    }

    if (_modelChain == mc) return;
    _modelChain = mc;

    spdlog::debug("Model '{}' chained to '{}'.", (const char*)GetName().c_str(), (const char*)mc.c_str());
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetModelChain");
    IncrementChangeCount();
}

std::string Model::GetModelChain() const
{
    if (_modelChain == "Beginning") {
        return "";
    }
    return _modelChain;
}

void Model::SetSuperStringColours(int count)
{
    while ((int)superStringColours.size() < count) {
        bool r = false;
        bool g = false;
        bool b = false;
        bool w = false;
        bool y = false;

        for (const auto& it : superStringColours) {
            if (it == xlRED)
                r = true;
            if (it == xlGREEN)
                g = true;
            if (it == xlBLUE)
                b = true;
            if (it == xlWHITE)
                w = true;
            if (it == xlYELLOW)
                y = true;
        }

        if (!r)
            superStringColours.push_back(xlRED);
        else if (!g)
            superStringColours.push_back(xlGREEN);
        else if (!b)
            superStringColours.push_back(xlBLUE);
        else if (!w)
            superStringColours.push_back(xlWHITE);
        else if (!y)
            superStringColours.push_back(xlYELLOW);
        else
            superStringColours.push_back(xlRED);
    }

    while ((int)superStringColours.size() > count) {
        superStringColours.pop_back();
    }

    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                OutputModelManager::WORK_RELOAD_MODELLIST |
                OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSuperStringColours");
}

void Model::SetSuperStringColour(int index, xlColor c)
{
    superStringColours[index] = c;

    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                OutputModelManager::WORK_RELOAD_MODELLIST |
                OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSuperStringColour");
}

void Model::AddSuperStringColour(xlColor c)
{
    superStringColours.push_back(c);
}

void Model::SetShadowModelFor(const std::string& shadowModelFor)
{
    if ( shadowModelFor != name ) { // models should not be a shadow model for themselves
        _shadowModelFor = shadowModelFor;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetShadowModelFor");
    }
}

std::string Model::GetRGBWHandling() const {
    return RGBW_HANDLING[rgbwHandlingType];
}

void Model::SetRGBWHandling(std::string const& handling)
{
    for (int x = 0; x < (int)RGBW_HANDLING.size(); ++x) {
        if (RGBW_HANDLING[x] == handling) {
            if (rgbwHandlingType != x) {
                rgbwHandlingType = x;
                IncrementChangeCount();
                AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetRGBWHandling");
            }
            break;
        }
    }
}

bool Model::IsShadowModel() const
{
    return _shadowModelFor.size() > 0;
}

std::string Model::GetShadowModelFor() const
{
    return _shadowModelFor;
}

void Model::AddASAPWork(uint32_t work, const std::string& from)
{
    if (auto* omm = modelManager.GetOutputModelManager()) {
        omm->AddASAPWork(work, from, this, nullptr, GetName());
    }
}

bool Model::CleanupFileLocations(RenderContext* ctx)
{
    bool rc = false;
    for (auto& it : faceInfo) {
        if (it.second.find("Type") != it.second.end() && it.second.at("Type") == "Matrix") {
            for (auto& it2 : it.second) {
                if (it2.first != "CustomColors" && it2.first != "ImagePlacement" && it2.first != "Type" && it2.second != "") {
                    if (FileExists(it2.second)) {
                        if (!ctx->IsInShowFolder(it2.second)) {
                            it2.second = ctx->MoveToShowFolder(it2.second, std::string(1, std::filesystem::path::preferred_separator) + "Faces");
                            rc = true;
                        }
                    }
                }
            }
        }
    }

    if (rc) {
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::CleanupFileLocations");
    }

    return BaseObject::CleanupFileLocations(ctx) || rc;
}

// all when true includes all image files ... even if they dont really exist
std::list<std::string> Model::GetFaceFiles(const std::list<std::string>& facesUsed, bool all, bool includeFaceName) const
{
    std::list<std::string> res;

    for (const auto& it : faceInfo) {
        if (all || std::find(begin(facesUsed), end(facesUsed), it.first) != facesUsed.end()) {
            if (it.second.find("Type") != it.second.end() && it.second.at("Type") == "Matrix") {
                for (const auto& it2 : it.second) {
                    if (it2.first != "CustomColors" && it2.first != "ImagePlacement" && it2.first != "Type" && it2.second != "") {
                        if (all || FileExists(it2.second)) {
                            std::string n = it2.second;
                            if (includeFaceName)
                                n = it.first + "|" + n;
                            res.push_back(n);
                        }
                    }
                }
            }
        }
    }

    return res;
}

bool Model::HasState(std::string const& state) const
{
    auto s = Lower(state);
    for (const auto& it : stateInfo) {
        if (it.first == s) {
            return true;
        }
    }
    return false;
}

void Model::GetMinScreenXY(float& minx, float& miny) const
{
    if (Nodes.size() == 0) {
        minx = 0;
        miny = 0;
        return;
    }

    minx = 99999999.0f;
    miny = 99999999.0f;
    for (const auto& it : Nodes) {
        for (const auto& it2 : it->Coords) {
            minx = std::min(minx, it2.screenX);
            miny = std::min(miny, it2.screenY);
        }
    }
}

void Model::ApplyDimensions(const std::string& units, float width, float height, float depth) {
    auto ruler = RulerObject::GetRuler();

    if (ruler != nullptr && width != 0 && height != 0) {
        float w = ruler->ConvertDimension(units, width);
        float h = ruler->ConvertDimension(units, height);
        float d = ruler->ConvertDimension(units, depth);

        GetModelScreenLocation().SetMWidth(ruler->UnMeasure(w));
        GetModelScreenLocation().SetMHeight(ruler->UnMeasure(h));
        if (depth != 0) {
            GetModelScreenLocation().SetMDepth(ruler->UnMeasure(d));
        }
    }
}


std::string Model::GetRulerDim() const {
    auto ruler = RulerObject::GetRuler();
    std::string u = "";
    if (ruler != nullptr) {
        switch (ruler->GetUnits()) {
            case RULER_UNITS_INCHES:
                u = "i";
                break;
            case RULER_UNITS_FEET:
                u = "f";
                break;
            case RULER_UNITS_YARDS:
                u = "y";
                break;
            case RULER_UNITS_MM:
                u = "mm";
                break;
            case RULER_UNITS_CM:
                u = "cm";
                break;
            case RULER_UNITS_M:
                u = "m";
                break;
        }
    }
    return u;
}

void Model::SaveDisplayDimensions()
{
    _savedWidth = GetModelScreenLocation().GetRestorableMWidth();
    if (_savedWidth == 0)
        _savedWidth = 1;
    _savedHeight = GetModelScreenLocation().GetRestorableMHeight();
    if (_savedHeight == 0)
        _savedHeight = 1;
    _savedDepth = GetModelScreenLocation().GetRestorableMDepth();
    if (_savedDepth == 0)
        _savedDepth = 1;
}

void Model::RestoreDisplayDimensions()
{
    if (!IsDmxDisplayType(DisplayAs) && DisplayAs != DisplayAsType::Image && DisplayAs != DisplayAsType::Label) {
        SetWidth(_savedWidth, true);
        // We dont want to set the height of three point models
        if (dynamic_cast<const ThreePointScreenLocation*>(&(GetModelScreenLocation())) == nullptr) {
            SetHeight(_savedHeight, true);
        }
        SetDepth(_savedDepth, true);
    }
}

std::vector<PWMOutput> Model::GetPWMOutputs() const {
    std::vector<PWMOutput> ret;
    if (IsPWMProtocol()) {
        int cur = 1;
        for (auto &n : Nodes) {
            for (int x = 0; x < (int)n->GetChanCount(); x++) {
                std::string label = n->GetName();
                if (label.empty()) {
                    label = "LED-" + std::to_string(cur) + "-" + n->GetNodeType()[x];
                }
                float g = _controllerConnection.GetGamma();
                int b = _controllerConnection.GetBrightness();
                ret.emplace_back(n->ActChan + 1, PWMOutput::Type::LED, 1, label, b, g);
            }
            cur++;
        }
    }
    return ret;
}

Model::PreviewGraphicsCacheInfo::~PreviewGraphicsCacheInfo()
{
    if (vica)
        delete vica;
    if (program)
        delete program;
    if (vca)
        delete vca;
    if (va)
        delete va;
};

void Model::deleteUIObjects()
{
    for (auto& a : uiCaches) {
        delete a.second;
    }
    uiCaches.clear();
    uiObjectsInvalid = false;
}

// wxDropPatternProperty::ValidateValue moved to ui/modelproperties/PropertyGridHelpers.cpp

std::string Model::GetAttributesAsJSON() const
{
    // Serialize the model to XML using XmlSerializer
    XmlSerializer serializer;
    pugi::xml_document doc = serializer.SerializeModel(this);

    // Get the root node - the model node should be the first child
    pugi::xml_node root = doc.document_element();
    if (!root) {
        return "{}";
    }

    pugi::xml_node modelNode = root.first_child();
    if (!modelNode) {
        return "{}";
    }

    // Use the helper function from XmlSerialize namespace
    return XmlSerialize::GetModelAttributesAsJSON(modelNode);
}

// Determines a simplified class for a model to be used by LLMs for better model understanding
std::string Model::DetermineClass(const std::string& displayAs, bool isSingingFace, bool isSpiralTree, bool isSticks, const std::string& dropPattern) {
    // drop pattern dir is 1 if dropping down and -1 if going up .. 0 if no drop pattern
    int dropPatternDir = 0;
    if (!dropPattern.empty()) {
        auto drops = Split(dropPattern, ',');
        for (const auto& it : drops) {
            int i = (int)std::strtol(it.c_str(), nullptr, 10);
            if (i != 0) {
                dropPatternDir = i < 0 ? -1 : 1;
                break;
            }
        }
    }

    if (displayAs == "Custom" && isSingingFace) {
        return "SingingFace";
    }

    if ((displayAs == "Candy Canes" && isSticks) || (displayAs == "Poly Line" && dropPatternDir == -1)) {
        return "Sticks";
    }

    if (displayAs == "Icicles" || (displayAs == "Poly Line" && dropPatternDir == 1)) {
        return "Icicles";
    }

    if (displayAs == "Single Line" || displayAs == "Poly Line" || displayAs == "Arches" || displayAs == "Candy Canes" || displayAs == "Circle" || isSpiralTree || displayAs == "Window Frame") {
        return "Line";
    }

    if (displayAs == "Horiz Matrix" || displayAs == "Vert Matrix" || StartsWith(displayAs, "Tree ")) {
        return "Matrix";
    }

    if (displayAs == "Channel Block" || displayAs == "Image") {
        return "Pixel";
    }

    if (displayAs == "DmxMovingHeadAdv") {
        return "Moving Head";
    }

    if (displayAs == "DmxFloodlight" || displayAs == "DmxFloodArea") {
        return "Floodlight";
    }

    if (displayAs == "DmxGeneral" || displayAs == "DmxServo" || displayAs == "DmxSkull") {
        return "DMX Special Purpose";
    }

    return "";
}
