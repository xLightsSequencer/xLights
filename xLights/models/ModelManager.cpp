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
#include <cmath>
#include <format>
#include <thread>

#include <pugixml.hpp>

#include "../render/UICallbacks.h"
#include "ArchesModel.h"
#include "BaseObject.h"
#include "CandyCaneModel.h"
#include "ChannelBlockModel.h"
#include "CircleModel.h"
#include "CubeModel.h"
#include "CustomModel.h"
#include "utils/ExternalHooks.h"
#include "IciclesModel.h"
#include "ImageModel.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ModelManager.h"
#include "MultiPointModel.h"
#include "models/OutputModelManager.h"
#include "Parallel.h"
#include "PolyLineModel.h"
#include "SingleLineModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "StarModel.h"
#include "SubModel.h"
#include "TreeModel.h"
#include "UtilFunctions.h"
#include "../utils/DisplayMessages.h"
#include "../utils/string_utils.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlDeserializingModelFactory.h"
#include "../graphics/IModelPreview.h"
#include "Pixels.h"
#include "../controllers/ControllerCaps.h"
#include "../render/Element.h"
#include "../xLightsMain.h"
#include "DMX/DmxFloodArea.h"
#include "DMX/DmxFloodlight.h"
#include "DMX/DmxGeneral.h"
#include "DMX/DmxMovingHead.h"
#include "DMX/DmxMovingHeadAdv.h"
#include "DMX/DmxServo.h"
#include "DMX/DmxServo3D.h"
#include "DMX/DmxSkull.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"
#include <log.h>

#ifdef GetObject
#undef GetObject  // Windows wingdi.h defines GetObject as GetObjectW
#endif

ModelManager::ModelManager(OutputManager* outputManager, xLightsFrame* xl) :
    layoutGroups(nullptr),
    _outputManager(outputManager),
    xlights(xl),
    previewWidth(0),
    previewHeight(0),
    _modelsLoading(false)
{
    // ctor
}

UICallbacks* ModelManager::GetUICallbacks() const {
    return xlights ? xlights->GetUICallbacks() : nullptr;
}

OutputModelManager* ModelManager::GetOutputModelManager() const {
    return xlights ? xlights->GetOutputModelManager() : nullptr;
}

bool ModelManager::IsLowDefinitionRender() const {
    return xlights ? xlights->IsLowDefinitionRender() : false;
}

ModelManager::~ModelManager()
{
    // Because loading models is async we have to ensure this is done before we destroy anything
    while (_modelsLoading)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    clear();
}

void ModelManager::clear()
{
    std::lock_guard<std::recursive_mutex> _lock(_modelMutex);
    for (auto& it : models) {
        if (it.second != nullptr) {
            delete it.second;
            it.second = nullptr;
        }
    }
    models.clear();
}

inline BaseObject* ModelManager::GetObject(const std::string& name) const
{
    return GetModel(name);
}

bool ModelManager::IsModelValid(const Model* m) const
{
    // we do this because it is a BaseObject recast as a Model so it clearly isn't a model and so we assume it is valid
    if (m == nullptr)
        return true;

    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second == m)
            return true;

        for (auto it2 : it.second->GetSubModels()) {
            if (it2 == m)
                return true;
        }
    }
    return false;
}

Model* ModelManager::GetModel(const std::string& name) const
{
    std::unique_lock<std::recursive_mutex> lock(_modelMutex);
    auto it = models.find(Trim(name));
    lock.unlock();

    if (it == models.end()) {
        size_t pos = name.find("/");
        if (pos != std::string::npos) {
            std::string mname = Trim(name.substr(0, pos));
            std::string smname = Trim(name.substr(pos + 1));
            Model* m = GetModel(mname);
            if (m != nullptr) {
                return m->GetSubModel(smname);
            }
        }
        return nullptr;
    }
    return it->second;
}

Model* ModelManager::operator[](const std::string& name) const
{
    return GetModel(Trim(name));
}

bool ModelManager::Rename(const std::string& oldName, const std::string& newName)
{
    auto on = Trim(oldName);
    auto nn = Trim(newName);
    Model* model = GetModel(on);
    if (model == nullptr || model->GetDisplayAs() == DisplayAsType::SubModel) {
        return false;
    }
    model->Rename(nn);
    model->name = nn;
    if (dynamic_cast<SubModel*>(model) == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(_modelMutex);
        bool changed = false;
        for (auto& it2 : models) {
            changed |= it2.second->ModelRenamed(on, nn);
        }
        models.erase(models.find(on));
        models[nn] = model;

        // go through all the model groups looking for things that might need to be renamed
        for (const auto& it : models) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                changed |= mg->ModelRenamed(on, nn);
            }
        }

        return changed;
    }

    return false;
}

bool ModelManager::RenameSubModel(const std::string& oldName, const std::string& newName)
{
    bool changed = false;

    auto on = Trim(oldName);
    auto nn = Trim(newName);

    for (auto& m : *this) {
        if (m.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(m.second);
            changed |= mg->SubModelRenamed(on, nn);
        }
    }

    return changed;
}

bool ModelManager::RenameInListOnly(const std::string& oldName, const std::string& newName)
{
    auto on = Trim(oldName);
    auto nn = Trim(newName);

    Model* model = GetModel(on);
    if (model == nullptr)
        return false;

    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    models.erase(models.find(on));
    models[nn] = model;
    return true;
}

bool ModelManager::IsModelOverlapping(const Model* model) const
{
    int32_t start = model->GetFirstChannel(); // model->GetNumberFromChannelString(model->ModelStartChannel);
    int32_t end = start + model->GetChanCount() - 1;
    // int32_t sstart = model->GetFirstChannel() + 1;
    // assert(sstart == start);
    // int32_t send = model->GetLastChannel() + 1;
    // assert(send == end);
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup && it.second->GetName() != model->GetName()) {
            int32_t s = it.second->GetFirstChannel(); // GetNumberFromChannelString(it->second->ModelStartChannel);
            int32_t e = s + it.second->GetChanCount() - 1;
            // int32_t ss = it->second->GetFirstChannel() + 1;
            // assert(ss == s);
            // int32_t se = it->second->GetLastChannel() + 1;
            // assert(se == e);
            if (start <= e && end >= s)
                return true;
        }
    }
    return false;
}

void ModelManager::LoadModels(pugi::xml_node modelNode, int previewW, int previewH)
{
    
    // spdlog::debug("ModelManager loading models.");

    _modelsLoading = true;
    clear();
    previewWidth = previewW;
    previewHeight = previewH;
    auto timerStart = std::chrono::steady_clock::now();
    std::list<pugi::xml_node> modelsToLoad;
    for (pugi::xml_node e = modelNode.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "model") {
            std::string name = Trim(e.attribute("name").as_string());
            if (!name.empty()) {
                modelsToLoad.push_back(e);
            }
        }
    }
    std::function<void(pugi::xml_node&, int)> f = [this, previewW, previewH](pugi::xml_node e, int idx) {
        createAndAddModel(e, previewW, previewH);
    };
    RunInAutoReleasePool([&]() {parallel_for(modelsToLoad, f);});
    // printf("%d Models loaded in %ldms", (int)modelsToLoad.size(), timer.Time());
    auto timerElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timerStart).count();
    spdlog::debug("Models loaded in {}ms", timerElapsed);
    _modelsLoading = false;

    // Check all recorded shadow models actually exist
    for (auto& it : models) {
        if (it.second->GetShadowModelFor() != "") {
            auto m = models.find(it.second->GetShadowModelFor());
            if (m == models.end()) {
                // showing model does not exist
                it.second->SetShadowModelFor("");
            }
        }
    }

    // Must recalculate start channels synchronously after parallel loading.
    // During parallel_for, models with chained start channels (>ModelName:1)
    // cannot resolve correctly because their dependency may not be loaded yet.
    // RecalcStartChannels resolves chains in topological order now that all
    // models are loaded.
    RecalcStartChannels();
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ModelManager::LoadModels");
}

uint32_t ModelManager::GetLastChannel() const
{
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    unsigned int max = 0;
    for (const auto& it : models) {
        max = std::max(max, it.second->GetLastChannel());
    }
    return max;
}

void ModelManager::ResetModelGroups() const
{
    // 
    // spdlog::debug("ModelManager resetting groups.");

    // This goes through all the model groups which hold model pointers and ensure their model pointers are correct
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second != nullptr && it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ((ModelGroup*)(it.second))->ResetModels();
        }
    }
    for (const auto& it : models) {
        if (it.second != nullptr && it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ((ModelGroup*)(it.second))->CheckForChanges();
        }
    }
}

std::string ModelManager::GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel, const std::string& protocol) const
{
    std::string last = "";
    unsigned int highestEndChannel = 0;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup &&
            it.second->GetControllerName() == controllerName &&
            it.second->GetControllerPort() == port &&
            it.second->GetControllerProtocol() == protocol &&
            it.second->GetLastChannel() > highestEndChannel && it.first != excludeModel) {
            last = it.first;
            highestEndChannel = it.second->GetLastChannel();
        }
    }
    return last;
}

std::string ModelManager::GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel, const std::string& protocol, int smartReceiver) const
{
    std::string last = "";
    unsigned int highestEndChannel = 0;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup &&
            it.second->GetControllerName() == controllerName &&
            it.second->GetControllerPort() == port &&
            it.second->GetControllerProtocol() == protocol &&
            it.second->GetSmartRemote() == smartReceiver &&
            it.second->GetLastChannel() > highestEndChannel && it.first != excludeModel) {
            last = it.first;
            highestEndChannel = it.second->GetLastChannel();
        }
    }
    return last;
}

void ModelManager::ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP)
{
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (Contains(it.second->ModelStartChannel, oldIP)) {
                it.second->ReplaceIPInStartChannels(oldIP, newIP);
            }
        }
    }
}

void ModelManager::AddModelGroups(pugi::xml_node n, int w, int h, const std::string& mname, bool& merge, bool& ask) {
    // 
    // spdlog::debug("ModelManager adding groups.");

    std::string grpModels = n.attribute("models").as_string();
    if (grpModels.empty())
    {
        return;
    }

    std::string mgname = n.attribute("name").as_string();
    bool alias { false };
    if (models.find(mgname) != models.end()) {
        for (const auto& [name, mm] : models) {
            if (mm->GetDisplayAs() == DisplayAsType::ModelGroup) {
                if (mm->IsAlias(mgname, false)) {
                    mgname = name;
                    alias = true;
                    break;
                }
            }
        }
    }

    if (models.find(mgname) != models.end()) {
        if (ask && !alias) {
            if (auto* ui = GetUICallbacks()) {
                if (ui->PromptYesNo("Model contains Model Group(s) that Already Exist.\n Would you Like to Add this Model to the Existing Groups?",
                                    "Model Group(s) Already Exists"))
                    merge = true;
            }
            ask = false;
        }
        if (merge || alias) { // merge
            Model* mg = GetModel(mgname);
            if (mg->GetDisplayAs() == DisplayAsType::ModelGroup) {
                ModelGroup* mmg = dynamic_cast<ModelGroup*>(mg);
                bool found = false;
                std::vector<std::string> prevousNames;
                auto oldModelNames = mmg->ModelNames(); // we copy the name list as we are going to be modifying the group while we iterate over these
                for (const auto& it : oldModelNames) {
                    auto mmnmn = mmg->ModelNames();
                    if (Contains(it, "/")) { // only add new SubModel if the name matches an old SubModel name, I don't understand why?
                        auto slashPos = it.find('/');
                        std::string afterSlash = (slashPos != std::string::npos) ? it.substr(slashPos + 1) : it;
                        std::string mgmn = mname + "/" + afterSlash;
                        std::string em = "EXPORTEDMODEL/" + afterSlash;
                        if (ContainsBetweenCommas(grpModels, em)) {
                            if (std::find(mmnmn.begin(), mmnmn.end(), mgmn) == mmnmn.end() &&
                                std::find(prevousNames.begin(), prevousNames.end(), mgmn) == prevousNames.end() &&
                                !mmg->DirectlyContainsModel(mgmn)) {
                                mmg->AddModel(mgmn);
                                prevousNames.push_back(mgmn);
                                found = true;
                            }
                        } else { // look for zero padded
                            std::string submodel = it.substr(it.find('/') + 1);
                            size_t pos = submodel.find_last_of(' ');
                            std::string num_str;
                            if (pos != std::string::npos) {
                                std::string before_space = submodel.substr(0, pos);
                                size_t num_start = before_space.find_last_of(' ') + 1;
                                if (num_start == 0 || num_start >= before_space.length()) num_start = 0;
                                size_t i = num_start;
                                while (i < before_space.length() && std::isdigit(before_space[i])) {
                                    num_str += before_space[i];
                                    ++i;
                                }
                            }
                            if (num_str.empty()) {
                                size_t i = 0;
                                while (i < submodel.length() && std::isdigit(submodel[i])) {
                                    num_str += submodel[i];
                                    ++i;
                                }
                            }
                            if (!num_str.empty()) {
                                try {
                                    int num = (int)std::strtol(num_str.c_str(), nullptr, 10);
                                    std::string itZeroPad;
                                    if (pos != std::string::npos) {
                                        size_t num_start = submodel.find_last_of(' ', pos - 1) + 1;
                                        if (num_start >= submodel.length()) num_start = 0;
                                        itZeroPad = it.substr(0, it.find('/') + 1) + submodel.substr(0, num_start) +
                                            (num < 10 ? "0" : "") + std::to_string(num);
                                        size_t num_end = num_start + num_str.length();
                                        if (num_end < submodel.length()) {
                                            itZeroPad += submodel.substr(num_end);
                                        }
                                    } else {
                                        itZeroPad = it.substr(0, it.find('/') + 1) +
                                            (num < 10 ? "0" : "") + std::to_string(num);
                                    }
                                    auto zpSlashPos = itZeroPad.find('/');
                                    std::string zpAfterSlash = (zpSlashPos != std::string::npos) ? itZeroPad.substr(zpSlashPos + 1) : itZeroPad;
                                    std::string mgmn = mname + "/" + zpAfterSlash;
                                    std::string em = "EXPORTEDMODEL/" + zpAfterSlash;
                                    if (ContainsBetweenCommas(grpModels, em) &&
                                        std::find(mmnmn.begin(), mmnmn.end(), mgmn) == mmnmn.end() &&
                                        std::find(prevousNames.begin(), prevousNames.end(), mgmn) == prevousNames.end() &&
                                        !mmg->DirectlyContainsModel(mgmn)) {
                                        mmg->AddModel(mgmn);
                                        prevousNames.push_back(mgmn);
                                        found = true;
                                    }
                                } catch (const std::exception&) {
                                }
                            }
                        }
                    } else {
                        if (ContainsBetweenCommas(grpModels, it) && std::find(mmnmn.begin(), mmnmn.end(), mname) == mmnmn.end() &&
                            std::find(prevousNames.begin(), prevousNames.end(), mname) == prevousNames.end() &&
                            !mmg->DirectlyContainsModel(mname)) {
                            mmg->AddModel(mname);
                            prevousNames.push_back(mname);
                            found = true;
                        }
                    }
                }

                if (!found) {
                    // Add SubModel if not found with keiths way.
                    // I think it makes sense to add the model if the group is in the xmodel file.
                    const auto& newNames = Split(grpModels, ',');
                    for (const auto& it : newNames) {
                        auto& mmnmn = mmg->ModelNames();
                        if (Contains(it, "/")) {
                            std::string mgmn = it;
                            auto expPos = mgmn.find("EXPORTEDMODEL");
                            if (expPos != std::string::npos) {
                                mgmn.replace(expPos, 13, mname);
                            }
                            if (std::find(mmnmn.begin(), mmnmn.end(), mgmn) == mmnmn.end() &&
                                std::find(prevousNames.begin(), prevousNames.end(), mgmn) == prevousNames.end() &&
                                !mmg->DirectlyContainsModel(mgmn)) {
                                prevousNames.push_back(mgmn);
                                mmg->AddModel(mgmn);
                            }
                        }
                    }
                }
            }
            return;
        }
    }

    // create new groups
    std::string nn = mgname;
    int i = 1;
    while (models.find(nn) != models.end()) {
        nn = std::format("{}_{}", mgname, i++);
    }

    // Create a temporary document with a copy of the node and modified attributes
    pugi::xml_document tempDoc;
    tempDoc.append_copy(n);
    pugi::xml_node tempNode = tempDoc.first_child();
    tempNode.attribute("name").set_value(nn);

    // Fix model names by replacing EXPORTEDMODEL with the actual model name
    auto modelsList = Split(std::string(tempNode.attribute("models").as_string()), ',');
    std::string fixedModels;
    for (auto& it : modelsList) {
        if (!fixedModels.empty()) fixedModels += ",";
        Replace(it, "EXPORTEDMODEL", mname);
        fixedModels += it;
    }
    tempNode.attribute("models").set_value(fixedModels);

    // Use Deserialize to create the ModelGroup
    XmlDeserializingModelFactory factory;
    Model* model = factory.Deserialize(tempNode, xlights, false);
    if (model != nullptr) {
        model->GetModelScreenLocation().previewW = w;
        model->GetModelScreenLocation().previewH = h;
        ModelGroup* mg = dynamic_cast<ModelGroup*>(model);
        if (mg != nullptr) {
            mg->RebuildBuffers();
            AddModel(mg);
        }
    }
}

bool ModelManager::RecalcStartChannels() const
{
    
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    auto swStart = std::chrono::steady_clock::now();
    bool changed = false;
    std::set<std::string> modelsDone;
    std::set<std::string> modelsOnNoController;

    for (const auto& it : models) {
        it.second->CouldComputeStartChannel = false;
    }

    // first go through all models whose start channels are not dependent on other models
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            char first = '0';
            if (!Trim(it.second->ModelStartChannel).empty())
                first = Trim(it.second->ModelStartChannel)[0];
            if (first != '>' && first != '@') {
                modelsDone.emplace(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->UpdateChannels();
                if (oldsc != it.second->GetFirstChannel()) {
                    changed = true;
                }
            }
            if (it.second->GetControllerName() == NO_CONTROLLER) {
                modelsOnNoController.emplace(it.first);
            }
        }
    }

    // now go through all undone models that depend on something
    bool workDone = false;
    do {
        workDone = false;

        for (const auto& it : models) {
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                char first = '0';
                if (!Trim(it.second->ModelStartChannel).empty())
                    first = Trim(it.second->ModelStartChannel)[0];
                if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel) {
                    std::string dependsOn = Trim(Trim(it.second->ModelStartChannel).substr(1, Trim(it.second->ModelStartChannel).find(':') - 1));

                    if (first == '>' && modelsOnNoController.find(dependsOn) != modelsOnNoController.end()) {
                        modelsOnNoController.emplace(it.first);
                    }

                    if (modelsDone.find(dependsOn) != modelsDone.end() && modelsOnNoController.find(dependsOn) == modelsOnNoController.end()) {
                        // the depends on model is done
                        modelsDone.emplace(it.first);
                        auto oldsc = it.second->GetFirstChannel();
                        it.second->UpdateChannels();
                        if (oldsc != it.second->GetFirstChannel()) {
                            changed = true;
                        }
                        if (it.second->CouldComputeStartChannel) {
                            workDone = true;
                        }
                    }
                }
            }
        }

    } while (workDone);

    // now process anything unprocessed
    int countInvalid = 0;
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            char first = '0';
            if (!Trim(it.second->ModelStartChannel).empty())
                first = Trim(it.second->ModelStartChannel)[0];
            if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel) {
                modelsDone.emplace(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->UpdateChannels();
                if (oldsc != it.second->GetFirstChannel()) {
                    changed = true;
                }
            }
            if (!it.second->CouldComputeStartChannel) {
                countInvalid++;
            }
        }
    }

    ResetModelGroups();

    // Commenting out as this doesn't need to happen unless we have changes and when we do it is redundant as the only
    // current caller of this method, xLightsMain>>RecalcStartChannels, already adds RELOAD_MODELLIST work if changes exist
    // xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "RecalcStartChannels");

    auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - swStart).count();
    spdlog::debug("RecalcStartChannels takes {}ms.", end);

    if (countInvalid > 0) {
        DisplayStartChannelCalcWarning();
    }
    return changed;
}

void ModelManager::DisplayStartChannelCalcWarning() const
{
    static std::string lastwarn = "";
    std::string msg = "Could not calculate start channels for models:\n";
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup && !it.second->CouldComputeStartChannel) {
            msg += it.second->name + " : " + it.second->ModelStartChannel + "\n";
        }
    }

    if (msg != lastwarn) {
        DisplayWarning(msg);
        lastwarn = msg;
    }
}

bool ModelManager::IsValidControllerModelChain(Model* m, std::string& tip) const
{
    std::list<Model*> sameOutput;
    tip = "";
    auto controllerName = m->GetControllerName();
    if (controllerName.empty())
        return true; // we dont check these

    auto port = m->GetControllerPort();
    if (port == 0)
        return true; // we dont check these

    auto chain = m->GetModelChain();
    if (StartsWith(chain, ">")) {
        chain = chain.substr(1);
    }
    auto startModel = m->GetName();
    auto smartRemote = m->GetSmartRemote();
    auto isPixel = m->IsPixelProtocol();

    if (!isPixel) {
        // For DMX outputs then beginning is always ok
        if (chain.empty()) {
            return true;
        }
    }

    for (const auto& it : *this) {
        if (it.first != startModel) {
            if (it.second->GetControllerName() == controllerName && it.second->GetControllerPort() == port && it.second->GetSmartRemote() == smartRemote && it.second->IsPixelProtocol() == isPixel) {
                auto c = it.second->GetModelChain();
                if (StartsWith(c, ">")) {
                    c = c.substr(1);
                }

                // valid if no other model shares my chain
                if (chain == c) {
                    tip = "Model shares chain with " + it.second->GetName();
                    return false; // two models chain this place in the chain
                }
                sameOutput.push_back(it.second);
            }
        }
    }

    // if no other models then chain must be blank
    if (sameOutput.size() == 0) {
        if (!chain.empty())
            tip = "Only model on an output must not chain to anything.";
        return (chain.empty());
    }

    if (chain == "")
        return true; // this model is the beginning

    // valid if i can follow the chain to blank
    int checks = 0;
    std::string current = startModel;
    std::string next = chain;
    while (checks <= (int)sameOutput.size()) {
        bool found = false;
        for (const auto& it : sameOutput) {
            if (it->GetName() == next) {
                next = it->GetModelChain();
                if (StartsWith(next, ">")) {
                    next = next.substr(1);
                }
                if (next.empty())
                    return true; // we found the beginning
                found = true;
                current = it->GetName();
                break;
            }
        }

        if (!found) {
            tip = "Chained to " + next + " but that model is not on this port";
            return false; // chained to non existent model
        }

        checks++;
    }
    tip = "Unable to find the beginning of the model chain on this output ... most likely you have a loop.";
    return false;
}

bool ModelManager::ReworkStartChannel() const
{
    auto work_logger = spdlog::get("work");
    work_logger->debug("        ReworkStartChannel.");

    bool outputsChanged = false;

    OutputManager* outputManager = xlights->GetOutputManager();
    for (const auto& it : outputManager->GetControllers()) {
        auto caps = it->GetControllerCaps();

        std::string serialPrefix;
        if (caps && caps->DMXAfterPixels()) {
            serialPrefix = "zzz";
        }

        std::map<std::string, std::list<Model*>> cmodels;
        std::lock_guard<std::recursive_mutex> lock(_modelMutex);
        for (auto itm : models) {
            if (itm.second->GetControllerName() == it->GetName() &&
                ((itm.second->GetControllerPort() != 0 && itm.second->GetControllerProtocol() != "") ||
                 (caps != nullptr && caps->GetMaxPixelPort() == 0 && caps->GetMaxSerialPort() == 0 && caps->GetMaxLEDPanelMatrixPort() == 0 && caps->GetMaxVirtualMatrixPort() == 0))) // we dont muck with unassigned models or no protocol models
            {
                std::string cc;
                if (IsPixelProtocol(itm.second->GetControllerProtocol())) {
                    cc = std::format("{}:{:02d}:{:02d}", itm.second->GetControllerProtocol(), itm.second->GetControllerPort(), itm.second->GetSortableSmartRemote());
                } else {
                    cc = std::format("{}{}:{:02d}", serialPrefix, itm.second->GetControllerProtocol(), itm.second->GetControllerPort());
                }
                std::transform(cc.begin(), cc.end(), cc.begin(), ::tolower);
                if (cmodels.find(cc) == cmodels.end()) {
                    std::list<Model*> ml;
                    cmodels[cc] = ml;
                }
                cmodels[cc].push_back(itm.second);
            }
        }

        // first of all fix any weirdness ...
        for (const auto& itcc : cmodels) {
            work_logger->debug("Fixing weirdness on {} - {}", (const char*)it->GetName().c_str(), (const char*)itcc.first.c_str());
            work_logger->debug("    Models at start:");

            // build a list of model names on the port
            std::list<std::string> models;
            for (auto& itmm : itcc.second) {
                work_logger->debug("        {} Chained to '{}'", (const char*)itmm->GetName().c_str(), (const char*)itmm->GetModelChain().c_str());
                models.push_back(itmm->GetName());
            }

             work_logger->debug("    Fixing weirdness:");

            // If a model refers to a chained model not on the port then move it to beginning ... so next step can move it again
            bool beginningFound = false;
            for (const auto& itmm : itcc.second) {
                auto ch = itmm->GetModelChain();
                if (ch.empty() || ch == "Beginning") {
                    beginningFound = true;
                } else {
                    ch = ch.substr(1); // string off leading >
                    if (std::find(models.begin(), models.end(), ch) == models.end()) {
                        work_logger->debug("    Model {} set to beginning because the model it is chained to '{}' does not exist.", (const char*)itmm->GetName().c_str(), (const char*)ch.c_str());
                        itmm->SetModelChain("");
                        beginningFound = true;
                        outputsChanged = true;
                    }
                }
            }

            // If no model is set as beginning ... then just make the first one beginning
            if (!beginningFound) {
                work_logger->debug("    Model {} set to beginning because no other model was.", (const char*)itcc.second.front()->GetName().c_str());
                itcc.second.front()->SetModelChain("");
                outputsChanged = true;
            }

            // Now I would love to give any more than the first model a default to chain to but this is
            // not as easy as it looks ... so for now i am going to leave multiple models at the beginning
            // and let the user sort it out rather than creating loops
        }

         work_logger->debug("    Sorting models:");
        int32_t ch = 1;
        std::list<Model*> allSortedModels;
        for (auto itcc = cmodels.begin(); itcc != cmodels.end(); ++itcc) {
            // order the models
            std::list<Model*> sortedmodels;
            std::string last = "";

            int32_t chstart = ch;

            if (itcc->second.size() > 0 && (itcc->second.front()->IsPixelProtocol() || itcc->second.front()->IsVirtualMatrixProtocol())) {
                while ((*itcc).second.size() > 0) {
                    bool pushed = false;
                    for (auto itms = itcc->second.begin(); itms != itcc->second.end(); ++itms) {
                        if ((((*itms)->GetModelChain() == "Beginning" || (*itms)->GetModelChain() == "") && last == "") ||
                            (*itms)->GetModelChain() == last ||
                            (*itms)->GetModelChain() == ">" + last) {
                            sortedmodels.push_back(*itms);
                            pushed = true;
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        }
                    }

                    if (!pushed && (*itcc).second.size() > 0) {
                        // chain is broken ... so just put the rest in in the original order
                        // assert(false);
                        work_logger->error("    Model chain is broken so just stuffing the remaining {} models in in their original order.", (*itcc).second.size());
                        while ((*itcc).second.size() > 0) {
                            sortedmodels.push_back(itcc->second.front());
                            itcc->second.pop_front();
                        }
                    }
                }
            } else if (itcc->second.front()->IsLEDPanelMatrixProtocol()) {
                while ((*itcc).second.size() > 0) {
                    sortedmodels.push_back((*itcc).second.front());
                    (*itcc).second.pop_front();
                }
            } else {
                // dmx protocols work differently ... they can be chained or by specified dmx channel
                int dmx = 1;
                while ((*itcc).second.size() > 0 && dmx <= 512) {
                    for (auto itms = itcc->second.begin(); itms != itcc->second.end(); ++itms) {
                        if (((*itms)->GetModelChain() == "Beginning" || (*itms)->GetModelChain() == "") && (*itms)->GetControllerDMXChannel() == dmx) {
                            sortedmodels.push_back(*itms);
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        } else if (last != "" && ((*itms)->GetModelChain() == last || (*itms)->GetModelChain() == ">" + last)) {
                            sortedmodels.push_back(*itms);
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        }
                    }
                    dmx++;
                }

                if ((*itcc).second.size() > 0) {
                    // models left over so stuff them on the end
                    work_logger->error("    DMX Model chain is broken or there are duplicate models so just stuffing the remaining {} models in in their original order.", (*itcc).second.size());
                    while ((*itcc).second.size() > 0) {
                        sortedmodels.push_back(itcc->second.front());
                        itcc->second.pop_front();
                    }
                }
            }

            for (auto itm : sortedmodels) {
                std::string sc = "";
                if (itm->IsPixelProtocol() || itm->IsVirtualMatrixProtocol()) {
                    if (itm->GetModelChain() == last ||
                        itm->GetModelChain() == ">" + last ||
                        ((itm->GetModelChain() == "Beginning" || itm->GetModelChain() == "") && last == "")) {
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + std::to_string(ch);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch += itm->GetChanCount();
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    } else {
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + std::to_string(chstart);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch = std::max(ch, (int32_t)(chstart + itm->GetChanCount()));
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    }
                } else if (itm->IsLEDPanelMatrixProtocol()) {
                    // This code only allows one model on a LED Matrix panel ... and I am not sure this is a valid limitation
                    // so if this is a chained model ... move it off the controller
                    if (itm->GetModelChain() != "Beginning" && itm->GetModelChain() != "") {
                        itm->SetControllerName(NO_CONTROLLER);

                        // because we have now moved a model off a controller we really need to do this all again
                        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ReworkStartChannel");

                        spdlog::warn("Attempt to place a second model {} on led panel port when only one is allowed. Only the first model has been retained. The others have been removed.", (const char*)itm->GetName().c_str());

                    } else {
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + std::to_string(chstart);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch = std::max(ch, (int32_t)(chstart + itm->GetChanCount()));
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    }
                } else {
                    // Handle controllers that must start serial outputs on a new universe and the first model is not DMX port 1
                    // This relies on serial ports being added first to any controller channels
                    if (itm == sortedmodels.front()) {
                        if ((it->GetProtocol() == OUTPUT_E131 || it->GetProtocol() == OUTPUT_ARTNET) && caps && caps->NeedsFullUniverseForDMX()) {
                            ch += itm->GetControllerDMXChannel() - 1;
                        }
                    }

                    // when chained the use next channel
                    if (last != "" && itm->GetControllerDMXChannel() == 0 &&
                        (itm->GetModelChain() == last ||
                         itm->GetModelChain() == ">" + last)) {
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + std::to_string(ch);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch += itm->GetChanCount();
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    } else {
                        // when not chained use dmx channel
                        uint32_t msc = chstart + itm->GetControllerDMXChannel() - 1;
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + std::to_string(msc);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch = std::max(ch, (int32_t)msc + (int32_t)itm->GetChanCount());
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    }

                    // Handle controllers that must start serial outputs on a new universe last model does not consume the full universe
                    if ((it->GetProtocol() == OUTPUT_E131 || it->GetProtocol() == OUTPUT_ARTNET) && caps && caps->NeedsFullUniverseForDMX()) {
                        if (itm == sortedmodels.back()) {
                            int unisize = it->GetFirstOutput()->GetChannels();
                            if ((ch - 1) % unisize != 0) {
                                ch += unisize - ((ch - 1) % unisize);
                            }
                        }
                    }
                }

                 work_logger->debug("    Model {} on port {} chained to {} start channel {}.",
                                  (const char*)itm->GetName().c_str(),
                                  itm->GetControllerPort(),
                                  (const char*)itm->GetModelChain().c_str(),
                                  (const char*)sc.c_str());
            }

            allSortedModels.splice(allSortedModels.end(), sortedmodels);
        }

        if (it->IsAutoSize()) {
            auto eth = dynamic_cast<const ControllerEthernet*>(it);
            if (it->GetChannels() != std::max((int32_t)1, (int32_t)ch - 1) || (eth != nullptr && eth->SupportsUniversePerString())) {
                work_logger->debug("    Resizing output to {} channels.", std::max((int32_t)1, (int32_t)ch - 1));

                auto oldC = it->GetChannels();
                // Set channel size won't always change the number of channels for some protocols
                it->SetChannelSize(std::max((int32_t)1, (int32_t)ch - 1), allSortedModels);
                if (it->GetChannels() != oldC || (eth != nullptr && (eth->IsUniversePerString() || eth->SupportsUniversePerString()))) {
                    outputManager->SomethingChanged();

                    if (it->GetChannels() != oldC || (eth != nullptr && eth->IsUniversePerString() && xlights->IsSequencerInitialize())) { 
                        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ReworkStartChannel");
                    }
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ReworkStartChannel");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ReworkStartChannel");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SAVE_NETWORKS, "ReworkStartChannel");
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ReworkStartChannel");
                }
            }
        }
    }

    // now we want to deal with any models specified as being on "No Controller"
    // first we need to work out the last used channel by all controllers and models other than those on No Controller
    uint32_t lastChannel = 0;
    for (const auto& it : outputManager->GetControllers()) {
        lastChannel = std::max(lastChannel, (uint32_t)it->GetEndChannel());
    }

    std::list<std::string> modelsToSet;
    {
        std::lock_guard<std::recursive_mutex> lock(_modelMutex);
        for (auto itm : models) {
            std::list<std::string> visited;
            if (ModelHasNoDependencyOnNoController(itm.second, visited) || itm.second->GetControllerName() == NO_CONTROLLER) {
                if (itm.second->GetControllerName() != NO_CONTROLLER) {
                    lastChannel = std::max(lastChannel, itm.second->GetLastChannel() + 1);
                } else {
                    modelsToSet.push_back(itm.first);
                }
            }
        }
    }

    // now we need to go through the models in a deterministic order and assign them ... so we sort them first
    modelsToSet.sort();
    for (const auto& it : modelsToSet) {
        Model* m = GetModel(it);
        auto osc = m->ModelStartChannel;
        m->SetStartChannel(std::to_string(lastChannel + 1));
        m->ClearIndividualStartChannels();
        lastChannel += m->GetChanCount();
        if (osc != m->ModelStartChannel) {
            outputsChanged = true;
        }
    }

    return outputsChanged;
}

bool ModelManager::ModelHasNoDependencyOnNoController(Model* m, std::list<std::string>& visited) const
{
    if (std::find(visited.begin(), visited.end(), m->GetName()) != visited.end())
        return false;

    visited.push_back(m->GetName());

    if (!m->CouldComputeStartChannel) // this should stop this looping forever due to chain loops
        return false;

    std::string sc = m->ModelStartChannel;
    if (sc != "" && (sc[0] == '>' || sc[0] == '@')) {
        std::string dependson = BeforeFirst(sc.substr(1), ':');
        Model* mm = GetModel(dependson);
        if (mm != nullptr) {
            if (mm->GetControllerName() == NO_CONTROLLER)
                return false;
            return ModelHasNoDependencyOnNoController(mm, visited);
        }
    }

    return true;
}

bool ModelManager::LoadGroups(pugi::xml_node groupNode, int previewW, int previewH)
{
    // 
    // spdlog::debug("ModelManager loading groups.");
    bool changed = false;
    std::list<pugi::xml_node> toBeDone;
    std::set<std::string> allModels;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    XmlDeserializingModelFactory factory;

    // do all the models without embedded groups first or where the model order means everything exists
    for (pugi::xml_node e = groupNode.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "modelGroup") {
            std::string name = e.attribute("name").as_string();
            if (!name.empty()) {
                allModels.insert(name);
                if (ModelGroup::AllModelsExist(e, *this)) {
                    Model* model = factory.Deserialize(e, xlights, false);
                    if (model != nullptr) {
                        model->GetModelScreenLocation().previewW = previewW;
                        model->GetModelScreenLocation().previewH = previewH;
                        ModelGroup* mg = dynamic_cast<ModelGroup*>(model);
                        if (mg != nullptr) {
                            mg->RebuildBuffers();
                            models[model->name] = model;
                        }
                    }
                } else {
                    toBeDone.push_back(e);
                }
            }
        }
    }

    // add in models and SubModels
    for (const auto& it : models) {
        allModels.insert(it.second->GetName());
        for (auto it2 : it.second->GetSubModels()) {
            allModels.insert(it2->GetFullName());
        }
    }

    // remove any totally non existent models from model groups
    // this stops some end conditions which cant be resolved
    for (const auto& it : toBeDone) {
        changed |= ModelGroup::RemoveNonExistentModels(it, allModels);
    }

    // try up to however many models we have left
    int maxIter = toBeDone.size();
    while (maxIter > 0 && toBeDone.size() > 0) {
        maxIter--;
        std::list<pugi::xml_node> processing(toBeDone);
        toBeDone.clear();
        for (const auto& it : processing) {
            if (ModelGroup::AllModelsExist(it, *this)) {
                Model* model = factory.Deserialize(it, xlights, false);
                if (model != nullptr) {
                    model->GetModelScreenLocation().previewW = previewW;
                    model->GetModelScreenLocation().previewH = previewH;
                    ModelGroup* mg = dynamic_cast<ModelGroup*>(model);
                    if (mg != nullptr) {
                        bool reset = mg->RebuildBuffers();
                        assert(reset);
                        models[model->name] = model;
                    }
                }
            } else {
                toBeDone.push_back(it);
            }
        }
    }

    // anything left in toBeDone is now due to model loops
    for (const auto& it : toBeDone) {
        std::string name = it.attribute("name").as_string();
        std::string msg = "Could not process model group " + name + " likely due to model groups loops. See Check Sequence for details.";
        DisplayWarning(msg);
        assert(false);
        Model* model = factory.Deserialize(it, xlights, false);
        if (model != nullptr) {
            model->GetModelScreenLocation().previewW = previewW;
            model->GetModelScreenLocation().previewH = previewH;
            ModelGroup* mg = dynamic_cast<ModelGroup*>(model);
            if (mg != nullptr) {
                bool reset = mg->RebuildBuffers();
                assert(!reset);
                models[model->name] = model;
            }
        }
    }

    return changed;
}

bool ModelManager::RenameController(const std::string& oldName, const std::string& newName)
{
    bool changed = false;

    for (auto& it : *this) {
        changed |= it.second->RenameController(oldName, newName);
    }

    return changed;
}

// generate the next similar model name to the candidateName we are given
std::string ModelManager::GenerateModelName(const std::string& candidateName) const
{
    lastGeneratedModelName = GenerateObjectName(candidateName);
    return GetLastGeneratedModelName();
}

Model* ModelManager::CreateDefaultModel(const std::string& type, const std::string& startChannel) const
{
    Model* model;

    std::string protocol = "ws2811";

    if (type == "Star") {
        auto* m = new StarModel(*this);
        m->SetNumStarStrings(1);
        m->SetStarNodesPerString(50);
        m->SetStarPoints(5);
        m->SetStarStartLocation("Bottom Ctr-CW");
        model = m;
    } else if (type == "Arches") {
        auto* m = new ArchesModel(*this);
        m->SetNumArches(1);
        m->SetNodesPerArch(50);
        m->SetLightsPerNode(1);
        model = m;
    } else if (type == "Candy Canes") {
        auto* m = new CandyCaneModel(*this);
        m->SetNumCanes(3);
        m->SetNodesPerCane(18);
        m->SetLightsPerNode(1);
        model = m;
    } else if (type == "Channel Block") {
        auto* m = new ChannelBlockModel(*this);
        m->SetNumChannels(16);
        protocol = xlEMPTY_STRING;
        m->SetStringType("Single Color White");
        m->SetPixelSize(12);
        model = m;
    } else if (type == "Circle") {
        auto* m = new CircleModel(*this);
        m->SetNumCircleStrings(1);
        m->SetCircleNodesPerString(50);
        m->SetInsideOut(false);
        model = m;
    } else if (type == "DmxMovingHead") {
        auto* m = new DmxMovingHead(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(8);
        m->SetDmxStyle("Moving Head Top");
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxGeneral") {
        auto* m = new DmxGeneral(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(8);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxMovingHeadAdv") {
        auto* m = new DmxMovingHeadAdv(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(8);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxFloodlight") {
        auto* m = new DmxFloodlight(*this);
        m->SetDmxChannelCount(3);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxFloodArea") {
        auto* m = new DmxFloodArea(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(3);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxSkull") {
        auto* m = new DmxSkull(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(26);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxServo") {
        auto* m = new DmxServo(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(2);
        m->SetStringType("Single Color White");
        model = m;
    } else if (type == "DmxServo3d" || type == "DmxServo3Axis") {
        auto* m = new DmxServo3d(*this);
        protocol = xlEMPTY_STRING;
        m->SetDmxChannelCount(2);
        m->SetStringType("Single Color White");
        if (type == "DmxServo3Axis") {
            m->SetNumServos(3);
            m->SetNumStatic(1);
            m->SetNumMotion(3);
            m->SetDmxChannelCount(6);
        }
        model = m;
    } else if (type == "Image") {
        model = new ImageModel(*this);
        protocol = xlEMPTY_STRING;
        dynamic_cast<ImageModel*>(model)->SetImageFile("");
        model->SetStringType("Single Color White");
    } else if (type == "Window Frame") {
        auto* m = new WindowFrameModel(*this);
        m->SetTopNodes(16);
        m->SetSideNodes(50);
        m->SetBottomNodes(16);
        model = m;
    } else if (type == "Wreath") {
        auto* m = new WreathModel(*this);
        m->SetNumWreathStrings(1);
        m->SetWreathNodesPerString(50);
        model = m;
    } else if (type.find("Sphere") == 0) {
        auto* m = new SphereModel(*this);
        m->SetNumMatrixStrings(10);
        m->SetNodesPerString(10);
        m->SetStrandsPerString(1);
        model = m;
    } else if (type == "Single Line") {
        auto* m = new SingleLineModel(*this);
        m->SetNumLines(1);
        m->SetNodesPerLine(50);
        m->SetLightsPerNode(1);
        model = m;
    } else if (type == "Poly Line") {
        auto* m = new PolyLineModel(*this);
        m->SetTotalLightCount(50);
        model = m;
    } else if (type == "MultiPoint") {
        model = new MultiPointModel(*this);
    } else if (type == "Cube") {
        auto* m = new CubeModel(*this);
        m->SetCubeWidth(5);
        m->SetCubeHeight(5);
        m->SetCubeDepth(5);
        m->SetCubeStyle("Horizontal Left/Right");
        model = m;
    } else if (type == "Custom") {
        auto* m = new CustomModel(*this);
        m->SetCustomWidth(5);
        m->SetCustomHeight(5);
        model = m;
    } else if (type.find("Tree") == 0) {
        auto* m = new TreeModel(*this);
        m->SetNumMatrixStrings(16);
        m->SetNodesPerString(50);
        m->SetStrandsPerString(1);
        model = m;
    } else if (type == "Matrix") {
        auto* m = new MatrixModel(*this);
        m->SetNumMatrixStrings(16);
        m->SetNodesPerString(50);
        m->SetStrandsPerString(1);
        m->SetStartSide("T");
        model = m;
    } else if (type == "Spinner") {
        auto* m = new SpinnerModel(*this);
        m->SetNumSpinnerStrings(1);
        m->SetNodesPerArm(10);
        m->SetArmsPerString(5);
        model = m;
    } else if (type == "Icicles") {
        auto* m = new IciclesModel(*this);
        m->SetNumIcicleStrings(1);
        m->SetLightsPerString(80);
        m->SetDropPattern("3,4,5,4");
        model = m;
    } else {
        DisplayError(std::format("'{}' is not a valid model type for a model", type));
        return nullptr;
    }

    model->SetControllerProtocol(protocol);
    model->SetControllerName(NO_CONTROLLER);

    model->SetName(GenerateModelName(type));
    model->SetStartChannel(startChannel);

    model->Setup();

    return model;
}

Model* ModelManager::CreateModel(pugi::xml_node node, int previewW, int previewH) const
{
    if (std::string_view(node.name()) == "modelGroup") {
        XmlDeserializingModelFactory factory;
        return factory.Deserialize(node, xlights, false);
    }

    Model* model;
    XmlSerializer serializer;
    model = serializer.DeserializeModel(node, xlights, false);
    
    if (model != nullptr) {
        model->GetModelScreenLocation().previewW = previewW;
        model->GetModelScreenLocation().previewH = previewH;
    }

    return model;
}

void ModelManager::AddModel(Model* model)
{
    // Lock before we add models ... this is required because LoadModels loads this in parallel

    if (model != nullptr) {
        std::lock_guard<std::recursive_mutex> _lock(_modelMutex);
        auto it = models.find(model->name);
        if (it != models.end()) {
            delete it->second;
            it->second = nullptr;
            ResetModelGroups();
        }
        models[model->name] = model;
    }
}

void ModelManager::ReplaceModel(const std::string &name, Model* nm) {
    if (nm != nullptr && name != "") {
        std::lock_guard<std::recursive_mutex> _lock(_modelMutex);
        Model *oldm = models[name];
        models[nm->name] = nm;
        ResetModelGroups();       
        delete oldm;
    }
}

Model* ModelManager::createAndAddModel(pugi::xml_node node, int previewW, int previewH)
{
    Model* model = CreateModel(node, previewW, previewH);
    AddModel(model);
    return model;
}

std::string ModelManager::GetModelsOnChannels(uint32_t start, uint32_t end, int perLine) const
{
    std::string res;
    std::string line;

    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (perLine > 0 && CountChar(line, ',') >= perLine - 1) {
                if (res != "")
                    res += "\n";
                res += line;
                line = "";
            }
            if (it.second->GetFirstChannel() + 1 <= end && it.second->GetLastChannel() + 1 >= start) {
                if (line != "")
                    line += ", ";
                line += it.first;
            }
        }
    }

    if (line != "") {
        if (res != "")
            res += "\n";
        res += line;
    }

    return res;
}

std::vector<std::string> ModelManager::GetGroupsContainingModel(const Model* model) const
{
    std::vector<std::string> res;
    if (model == nullptr) {
        
        spdlog::error("ModelManager::GetGroupsContainingModel called with nullptr");
        return res;
    }

    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            auto mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg == nullptr) {
                
                spdlog::error("ModelManager::GetGroupsContainingModel - Model '{}' claims to be ModelGroup but cast failed", 
                    it.first.c_str());
                continue;
            }
            if (mg->ContainsModel(model)) {
                res.push_back(it.first);
            } else {
                for (const auto& sm : model->GetSubModels()) {
                    if (mg->ContainsModel(sm)) {
                        res.push_back(it.first);
                        break;
                    }
                }
            }
        }
    }
    return res;
}

std::vector<std::string> ModelManager::GetGroupsContainingModelOrSubmodel(const Model* model) const
{
    std::vector<std::string> res;
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            auto mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg->ContainsModelOrSubmodel(model)) {
                res.push_back(it.first);
            } else {
                for (const auto& sm : model->GetSubModels()) {
                    if (mg->ContainsModelOrSubmodel(sm)) {
                        res.push_back(it.first);
                        break;
                    }
                }
            }
        }
    }
    return res;
}

std::vector<Model*> ModelManager::GetModelGroups(const Model* model) const {
    std::vector<Model*> res;
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            auto mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg->ContainsModel(model)) {
                res.push_back(mg);
            }
        }
    }
    return res;
}

std::string ModelManager::GenerateNewStartChannel(const std::string& lastModel) const
{
    std::string startChannel = "1";

    if (!lastModel.empty() && models.count(lastModel) > 0) {
        startChannel = ">" + lastModel + ":1";
    } else {
        unsigned int highestch = 0;
        Model* highest = nullptr;
        for (const auto& it : models) {
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                if (it.second->GetLastChannel() > highestch) {
                    highestch = it.second->GetLastChannel();
                    highest = it.second;
                }
            }
        }
        if (highest != nullptr) {
            startChannel = ">" + highest->GetName() + ":1";
        }
    }
    return startChannel;
}

std::string MergeModels(const std::string& ml1, const std::string& ml2)
{
    std::string res = "";

    std::list<std::string> models;

    std::istringstream lineStream(ml1);
    std::string cell;
    while (std::getline(lineStream, cell, ',')) {
        if (std::find(begin(models), end(models), cell) == end(models))
            models.push_back(cell);
    }

    lineStream = std::istringstream(ml2);
    while (std::getline(lineStream, cell, ',')) {
        if (std::find(begin(models), end(models), cell) == end(models))
            models.push_back(cell);
    }

    for (const auto& it : models) {
        if (res != "")
            res += ",";
        res += it;
    }

    return res;
}

// Helper: find a child XML node with a given tag name and "name" attribute value
static pugi::xml_node FindChildByNameAttr(pugi::xml_node parent, const std::string& childTag, const std::string& name)
{
    for (pugi::xml_node n = parent.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == childTag && std::string_view(n.attribute("name").as_string()) == name) {
            return n;
        }
    }
    return pugi::xml_node();
}

// Helper: set an attribute on a node (update or add)
static void SetXmlAttribute(pugi::xml_node node, const std::string& attr, const std::string& value)
{
    pugi::xml_attribute a = node.attribute(attr);
    if (a) {
        a.set_value(value);
    } else {
        node.append_attribute(attr) = value;
    }
}

// Helper: compare name attributes on child nodes for XML comparison
static bool CheckNameAttrs(pugi::xml_node nn, pugi::xml_node cc)
{
    if (!nn.attribute("name").empty()) {
        return std::string_view(cc.attribute("name").as_string()) == std::string_view(nn.attribute("name").as_string());
    } else if (!nn.attribute("Name").empty()) {
        return std::string_view(cc.attribute("Name").as_string()) == std::string_view(nn.attribute("Name").as_string());
    }
    return true;
}


static std::set<std::string> FLOAT_ATTRIBUTES = {
    XmlNodeKeys::WorldPosXAttribute,
    XmlNodeKeys::WorldPosYAttribute,
    XmlNodeKeys::WorldPosZAttribute,
    XmlNodeKeys::RotateXAttribute,
    XmlNodeKeys::RotateYAttribute,
    XmlNodeKeys::RotateZAttribute,
    XmlNodeKeys::ScaleXAttribute,
    XmlNodeKeys::ScaleYAttribute,
    XmlNodeKeys::ScaleZAttribute,
    XmlNodeKeys::LengthAttribute,
    XmlNodeKeys::X2Attribute,
    XmlNodeKeys::Y2Attribute,
    XmlNodeKeys::Z2Attribute,
};

#ifdef DEBUG_MERGEFROMBASE
static std::set<std::string> IGNORE_ATTRIBUTES = {
    "ModelBrightness",
    "RotateX",
    "CustomModel"
};
#endif
static std::set<std::string> BASE_EMPTY = {
    "StrandNames",
    "NodeNames",
    "PixelSpacing",
    "PixelCount",
    "PixelType"
};

// Helper: compare two XML nodes to detect changes (standalone version operating on raw nodes)
static bool IsXmlNodeChanged(pugi::xml_node local, pugi::xml_node base)
{
    // Check if base has attributes that differ from local
    bool changed = false;
    for (pugi::xml_attribute a : base.attributes()) {
        std::string_view aName = a.name();
        std::string_view aValue = a.value();
        pugi::xml_attribute localAttr = local.attribute(a.name());
        if (localAttr.empty() || std::string_view(localAttr.as_string()) != aValue) {
            // SourceVersion is just the xLights version that last saved the model — not a real change
            if (aName == "SourceVersion") continue;
            if (aName != "StartChannel" || local.attribute("Controller").empty()) {
                // For float attributes, compare rounded to 4 decimal places as older versions of xLights only output to 4 decimals
                if (FLOAT_ATTRIBUTES.count(std::string(aName)) > 0 && !localAttr.empty()) {
                    double baseVal = a.as_double(0.0);
                    double localVal = localAttr.as_double(0.0);
                    if (std::round(baseVal * 10000.0) == std::round(localVal * 10000.0)) {
                        continue;
                    }
                }
                // In some cases, old versions of xLights would put an empty attribute but the new version will not put the attribute at all.
                if (!(BASE_EMPTY.contains(std::string(aName)) && aValue.empty() && localAttr.empty())) {
#ifdef DEBUG_MERGEFROMBASE
                    if (!changed) {
                        printf("%s\n", local.attribute("name").as_string());
                    }
                    printf("  %s:  %s\n", a.name(), a.value());
                    printf("          %s\n", localAttr.as_string());
                    changed = true;
#else
                    return true;
#endif
                }
            }
        }
    }
    if (changed) {
        return true;
    }

    // Check child nodes
    for (pugi::xml_node nn = base.first_child(); nn; nn = nn.next_sibling()) {
        bool found = false;
        for (pugi::xml_node cc = local.first_child(); cc; cc = cc.next_sibling()) {
            if (std::string_view(cc.name()) == std::string_view(nn.name()) && CheckNameAttrs(nn, cc)) {
                found = true;
                for (pugi::xml_attribute a : nn.attributes()) {
                    pugi::xml_attribute ccAttr = cc.attribute(a.name());
                    if (ccAttr.empty() || std::string_view(ccAttr.as_string()) != std::string_view(a.value())) {
                        return true;
                    }
                }
            }
        }
        if (!found) {
            return true;
        }
    }
    return false;
}

// Helper: copy preserved local controller attributes onto a new node replacing a FromBase model
static void PreserveLocalControllerAttrs(pugi::xml_node localNode, pugi::xml_node newNode)
{
    // Find the local port from ControllerConnection child
    std::string port;
    for (pugi::xml_node p = localNode.first_child(); p; p = p.next_sibling()) {
        if (std::string_view(p.name()) == "ControllerConnection") {
            port = p.attribute("Port").as_string();
        }
    }

    if (!port.empty()) {
        // Preserve ModelChain if local has one
        std::string modelChain = localNode.attribute("ModelChain").as_string();
        if (!modelChain.empty()) {
            SetXmlAttribute(newNode, "ModelChain", modelChain);
        }
        SetXmlAttribute(newNode, "StartChannel", localNode.attribute("StartChannel").as_string());
        SetXmlAttribute(newNode, "Controller", localNode.attribute("Controller").as_string());
    }
}

// Helper: copy the local ControllerConnection port onto the new node's ControllerConnection child if it has no port
static void PreserveLocalControllerPort(pugi::xml_node localNode, pugi::xml_node newNode)
{
    std::string port;
    for (pugi::xml_node p = localNode.first_child(); p; p = p.next_sibling()) {
        if (std::string_view(p.name()) == "ControllerConnection") {
            port = p.attribute("Port").as_string();
        }
    }
    if (!port.empty()) {
        for (pugi::xml_node bp = newNode.first_child(); bp; bp = bp.next_sibling()) {
            if (std::string_view(bp.name()) == "ControllerConnection") {
                if (bp.attribute("Port").empty()) {
                    bp.append_attribute("Port") = port;
                }
            }
        }
    }
}
static void RemoveControllerConnection(pugi::xml_node node) {
    pugi::xml_node cc = node.child("ControllerConnection");
    if (cc) {
        node.remove_child(cc);
    }
}

// Shared method: merge base XML into current XML, updating currentModelsNode and currentGroupsNode in-place.
// Populates changedModels and changedGroups with names of models/groups that were added or updated.
// Only updates models/groups that are new or have FromBase="1" on the current side.
static bool MergeBaseIntoCurrentXml(pugi::xml_node currentModelsNode, pugi::xml_node currentGroupsNode,
                                    pugi::xml_node baseModelsNode, pugi::xml_node baseGroupsNode,
                                    std::vector<std::string>& changedModels,
                                    std::vector<std::string>& changedGroups)
{
    
    bool changed = false;

    if (baseModelsNode && currentModelsNode) {
        for (pugi::xml_node bm = baseModelsNode.first_child(); bm; bm = bm.next_sibling()) {
            if (std::string_view(bm.name()) != "model") continue;
            std::string name = bm.attribute("name").as_string();
            if (name.empty()) continue;

            pugi::xml_node local = FindChildByNameAttr(currentModelsNode, "model", name);

            if (!local) {
                // Model does not exist locally -- add it
                pugi::xml_node copy = currentModelsNode.append_copy(bm);
                SetXmlAttribute(copy, "FromBase", "1");
                changedModels.push_back(name);
                changed = true;
                spdlog::debug("MergeBase: Adding model from base: '{}'.", name.c_str());
            } else if (std::string_view(local.attribute("FromBase").as_string()) == "1") {
                // Model exists and came from base -- update if changed
                if (IsXmlNodeChanged(local, bm)) {
                    pugi::xml_node copy = currentModelsNode.insert_copy_after(bm, local);
                    SetXmlAttribute(copy, "FromBase", "1");
                    PreserveLocalControllerAttrs(local, copy);
                    PreserveLocalControllerPort(local, copy);

                    currentModelsNode.remove_child(local);
                    changedModels.push_back(name);
                    changed = true;
                    spdlog::debug("MergeBase: Updating model from base: '{}'.", name.c_str());
                }
            }
            // If model exists locally without FromBase, skip silently
        }
    }

    if (baseGroupsNode && currentGroupsNode) {
        for (pugi::xml_node bg = baseGroupsNode.first_child(); bg; bg = bg.next_sibling()) {
            if (std::string_view(bg.name()) != "modelGroup") continue;
            std::string name = bg.attribute("name").as_string();
            if (name.empty()) continue;

            pugi::xml_node local = FindChildByNameAttr(currentGroupsNode, "modelGroup", name);

            if (!local) {
                // Group does not exist locally -- add it
                pugi::xml_node copy = currentGroupsNode.append_copy(bg);
                SetXmlAttribute(copy, "FromBase", "1");
                changedGroups.push_back(name);
                changed = true;
                spdlog::debug("MergeBase: Adding model group from base: '{}'.", name.c_str());
            } else if (std::string_view(local.attribute("FromBase").as_string()) == "1") {
                // Group exists and came from base -- merge model lists and update if changed
                std::string baseModelList = bg.attribute("models").as_string();
                std::string localModelList = local.attribute("models").as_string();
                std::string mergedList = MergeModels(baseModelList, localModelList);

                // Create a temp doc with copy of bg to modify for comparison
                pugi::xml_document tempDoc;
                pugi::xml_node copy = tempDoc.append_copy(bg);
                SetXmlAttribute(copy, "models", mergedList);
                SetXmlAttribute(copy, "FromBase", "1");
                copy.remove_attribute("selected");
                copy.remove_attribute("BaseModels");

                RemoveControllerConnection(copy);

                // Make a local copy for comparison purposes (without ControllerConnection)
                pugi::xml_document localCopyDoc;
                pugi::xml_node localCopy = localCopyDoc.append_copy(local);
                RemoveControllerConnection(localCopy);

                if (IsXmlNodeChanged(localCopy, copy)) {
                    SetXmlAttribute(copy, "BaseModels", baseModelList);
                    // Preserve local X/Y centre offsets
                    std::string xOffset = local.attribute("GridCentreX").as_string();
                    std::string yOffset = local.attribute("GridCentreY").as_string();
                    if (!xOffset.empty()) SetXmlAttribute(copy, "GridCentreX", xOffset);
                    if (!yOffset.empty()) SetXmlAttribute(copy, "GridCentreY", yOffset);

                    currentGroupsNode.insert_copy_after(copy, local);
                    currentGroupsNode.remove_child(local);
                    changedGroups.push_back(name);
                    changed = true;
                    spdlog::debug("MergeBase: Updating model group from base: '{}'.", name.c_str());
                }
            }
            // If group exists locally without FromBase, skip silently
        }
    }

    return changed;
}

// Load base XML document and find <models> and <modelGroups> nodes
static bool LoadBaseXmlNodes(const std::string& baseShowDir, pugi::xml_document& doc,
                             pugi::xml_node& baseModels, pugi::xml_node& baseGroups)
{
    baseModels = pugi::xml_node();
    baseGroups = pugi::xml_node();
    std::string path = baseShowDir + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result) return false;

    pugi::xml_node root = doc.document_element();
    if (root) {
        for (pugi::xml_node mm = root.first_child(); mm; mm = mm.next_sibling()) {
            if (std::string_view(mm.name()) == "models") baseModels = mm;
            else if (std::string_view(mm.name()) == "modelGroups") baseGroups = mm;
        }
    }
    return true;
}

bool ModelManager::MergeBaseXml(const std::string& baseShowDir, pugi::xml_node localModelsNode, pugi::xml_node localGroupsNode)
{
    pugi::xml_document baseDoc;
    pugi::xml_node baseModels;
    pugi::xml_node baseGroups;
    if (!LoadBaseXmlNodes(baseShowDir, baseDoc, baseModels, baseGroups)) return false;

    std::vector<std::string> changedModels;
    std::vector<std::string> changedGroups;
    return MergeBaseIntoCurrentXml(localModelsNode, localGroupsNode,
                                   baseModels, baseGroups,
                                   changedModels, changedGroups);
}

bool ModelManager::MergeFromBase(const std::string& baseShowDir, bool prompt)
{
    bool changed = false;

    pugi::xml_document baseDoc;
    pugi::xml_node baseModels;
    pugi::xml_node baseGroups;
    if (!LoadBaseXmlNodes(baseShowDir, baseDoc, baseModels, baseGroups)) return false;
    if (!baseModels) return false;

    // Handle prompt mode: ask user about non-FromBase models that clash with base
    if (prompt) {
        for (pugi::xml_node bm = baseModels.first_child(); bm; bm = bm.next_sibling()) {
            if (std::string_view(bm.name()) != "model") continue;
            std::string name = bm.attribute("name").as_string();
            if (name.empty()) continue;
            auto curr = GetModel(name);
            if (curr != nullptr && !curr->IsFromBase()) {
                if (auto* ui = GetUICallbacks()) {
                    if (ui->PromptYesNo(std::format("Model {} found that clashes with base show directory. Do you want to take the base show directory version?", name),
                                        "Model clash")) {
                        curr->SetFromBase(true);
                    }
                }
            }
        }
        if (baseGroups) {
            for (pugi::xml_node bg = baseGroups.first_child(); bg; bg = bg.next_sibling()) {
                if (std::string_view(bg.name()) != "modelGroup") continue;
                std::string name = bg.attribute("name").as_string();
                if (name.empty()) continue;
                auto curr = GetModel(name);
                if (curr != nullptr && !curr->IsFromBase()) {
                    if (auto* ui = GetUICallbacks()) {
                        if (ui->PromptYesNo(std::format("Model Group {} found that clashes with base show directory. Do you want to take the base show directory version?", name),
                                            "Model group clash")) {
                            curr->SetFromBase(true);
                        }
                    }
                }
            }
        }
    }

    // Serialize all current models to a temporary XML document for comparison
    pugi::xml_document tempDoc;
    pugi::xml_node currentModelsNode = tempDoc.append_child("models");
    pugi::xml_node currentGroupsNode = tempDoc.append_child("modelGroups");

    {
        XmlSerializingVisitor visitor{currentModelsNode};
        for (auto m = begin(); m != end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
                model->Accept(visitor);
            }
        }
    }
    {
        XmlSerializingVisitor visitor{currentGroupsNode};
        for (auto m = begin(); m != end(); ++m) {
            Model* model = m->second;
            if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
                model->Accept(visitor);
            }
        }
    }

    // Run the shared merge
    std::vector<std::string> changedModels;
    std::vector<std::string> changedGroups;
    changed = MergeBaseIntoCurrentXml(currentModelsNode, currentGroupsNode,
                                       baseModels, baseGroups,
                                       changedModels, changedGroups);

    // Apply changes: replace changed models with new ones created from the updated XML
    for (const auto& name : changedModels) {
        pugi::xml_node updatedNode = FindChildByNameAttr(currentModelsNode, "model", name);
        if (updatedNode) {
            auto curr = GetModel(name);
            if (curr != nullptr) {
                Model* newm = CreateModel(updatedNode, previewWidth, previewHeight);
                ReplaceModel(name, newm);
            } else {
                createAndAddModel(updatedNode, previewWidth, previewHeight);
            }
        }
    }
    for (const auto& name : changedGroups) {
        pugi::xml_node updatedNode = FindChildByNameAttr(currentGroupsNode, "modelGroup", name);
        if (updatedNode) {
            auto curr = GetModel(name);
            if (curr != nullptr) {
                Model* newm = CreateModel(updatedNode, previewWidth, previewHeight);
                ReplaceModel(name, newm);
            } else {
                createAndAddModel(updatedNode, previewWidth, previewHeight);
            }
        }
    }

    if (changed) {
        RecalcStartChannels();
    }

    return changed;
}

bool ModelManager::Delete(const std::string& name)
{
    // some layouts end up with illegal names
    std::string mn = Model::SafeModelName(name);

    if (xlights->CurrentSeqXmlFile != nullptr) {
        Element* elem_to_delete = xlights->GetSequenceElements().GetElement(mn);
        if (elem_to_delete != nullptr) {
            // does model have any effects on it
            bool effects_exist = false;
            for (size_t i = 0; i < elem_to_delete->GetEffectLayerCount() && !effects_exist; ++i) {
                auto layer = elem_to_delete->GetEffectLayer(i);
                if (layer->GetEffectCount() > 0) {
                    effects_exist = true;
                }
            }

            if (effects_exist) {
                if (auto* ui = GetUICallbacks()) {
                    if (!ui->PromptYesNo("Model '" + name + "' exists in the currently open sequence and has effects on it. Delete all effects and layers on this model?", "Confirm Delete?"))
                        return false;
                }
            }

            // Delete the model from the sequencer grid and views
            xlights->GetSequenceElements().DeleteElement(mn);
        }
    }

    // now delete the model
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->first == mn) {
            Model* model = it->second;

            if (model != nullptr) {
                for (auto& it2 : models) {
                    if (it2.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                        ModelGroup* group = (ModelGroup*)it2.second;
                        group->ModelRemoved(mn);
                    }
                }
                models.erase(it);
                ResetModelGroups();

                // If models are chained to us then make their start channel ... our start channel
                std::string chainedtous = ">" + model->GetName() + ":1";
                for (auto it3 : models) {
                    if (it3.second->ModelStartChannel == chainedtous) {
                        it3.second->SetStartChannel(model->ModelStartChannel);
                    }
                }

                delete model;
                xlights->UnsavedRgbEffectsChanges = true;
                return true;
            }
        }
    }
    return false;
}

std::map<std::string, Model*>::const_iterator ModelManager::begin() const
{
    return models.begin();
}

std::map<std::string, Model*>::const_iterator ModelManager::end() const
{
    return models.end();
}

unsigned int ModelManager::size() const
{
    return models.size();
}

bool ModelManager::IsModelShadowing(const Model* m) const
{
    for (const auto& it : models) {
        if (it.second->GetShadowModelFor() == m->GetName()) {
            return true;
        }
    }
    return false;
}

std::list<std::string> ModelManager::GetModelsShadowing(const Model* m) const
{
    std::list<std::string> res;
    for (const auto& it : models) {
        if (it.second->GetShadowModelFor() == m->GetName()) {
            res.push_back(it.first);
        }
    }
    return res;
}
