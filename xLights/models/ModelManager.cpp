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

#include "ArchesModel.h"
#include "CandyCaneModel.h"
#include "ChannelBlockModel.h"
#include "CheckboxSelectDialog.h"
#include "CircleModel.h"
#include "CubeModel.h"
#include "CustomModel.h"
#include "IciclesModel.h"
#include "ImageModel.h"
#include "Model.h"
#include "ModelGroup.h"
#include "ModelManager.h"
#include "MultiPointModel.h"
#include "Parallel.h"
#include "PolyLineModel.h"
#include "SingleLineModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "StarModel.h"
#include "SubModel.h"
#include "TreeModel.h"
#include "UtilFunctions.h"
#include "WholeHouseModel.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "../ModelPreview.h"
#include "../Pixels.h"
#include "../controllers/ControllerCaps.h"
#include "../sequencer/Element.h"
#include "../xLightsMain.h"
#include "DMX/DmxFloodArea.h"
#include "DMX/DmxFloodlight.h"
#include "DMX/DmxGeneral.h"
#include "DMX/DmxMovingHead.h"
#include "DMX/DmxMovingHead3D.h"
#include "DMX/DmxServo.h"
#include "DMX/DmxServo3D.h"
#include "DMX/DmxSkull.h"
#include "DMX/DmxSkulltronix.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"
#include <log4cpp/Category.hh>

ModelManager::ModelManager(OutputManager* outputManager, xLightsFrame* xl) :
    _outputManager(outputManager),
    xlights(xl),
    layoutsNode(nullptr),
    previewWidth(0),
    previewHeight(0),
    _modelsLoading(false)
{
    // ctor
}

ModelManager::~ModelManager()
{
    // Because loading models is async we have to ensure this is done before we destroy anything
    while (_modelsLoading)
        wxMilliSleep(1);

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
    if (model == nullptr || model->GetDisplayAs() == "SubModel") {
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
        if (m.second->GetDisplayAs() == "ModelGroup") {
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
    // wxASSERT(sstart == start);
    // int32_t send = model->GetLastChannel() + 1;
    // wxASSERT(send == end);
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() != "ModelGroup" && it.second->GetName() != model->GetName()) {
            int32_t s = it.second->GetFirstChannel(); // GetNumberFromChannelString(it->second->ModelStartChannel);
            int32_t e = s + it.second->GetChanCount() - 1;
            // int32_t ss = it->second->GetFirstChannel() + 1;
            // wxASSERT(ss == s);
            // int32_t se = it->second->GetLastChannel() + 1;
            // wxASSERT(se == e);
            if (start <= e && end >= s)
                return true;
        }
    }
    return false;
}

void ModelManager::LoadModels(wxXmlNode* modelNode, int previewW, int previewH)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("ModelManager loading models.");

    _modelsLoading = true;
    clear();
    previewWidth = previewW;
    previewHeight = previewH;
    this->modelNode = modelNode;
    wxStopWatch timer;
    std::list<wxXmlNode*> modelsToLoad;
    for (wxXmlNode* e = modelNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "model") {
            std::string name = e->GetAttribute("name").Trim(true).Trim(false).ToStdString();
            if (!name.empty()) {
                modelsToLoad.push_back(e);
            }
        }
    }
    std::function<void(wxXmlNode*&, int)> f = [this, previewW, previewH](wxXmlNode* e, int idx) {
        createAndAddModel(e, previewW, previewH);
    };
    parallel_for(modelsToLoad, f);
    // printf("%d Models loaded in %ldms", (int)modelsToLoad.size(), timer.Time());
    logger_base.debug("Models loaded in %ldms", timer.Time());
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

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ModelManager::LoadModels");
    // RecalcStartChannels();
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
    // static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("ModelManager resetting groups.");

    // This goes through all the model groups which hold model pointers and ensure their model pointers are correct
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second != nullptr && it.second->GetDisplayAs() == "ModelGroup") {
            ((ModelGroup*)(it.second))->ResetModels();
        }
    }
    for (const auto& it : models) {
        if (it.second != nullptr && it.second->GetDisplayAs() == "ModelGroup") {
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
        if (it.second->GetDisplayAs() != "ModelGroup" &&
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
        if (it.second->GetDisplayAs() != "ModelGroup" &&
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
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (Contains(it.second->ModelStartChannel, oldIP)) {
                it.second->ReplaceIPInStartChannels(oldIP, newIP);
            }
        }
    }
}

std::string ModelManager::SerialiseModelGroupsForModel(Model* m) const
{
    wxArrayString allGroups;
    wxArrayString onlyGroups;
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() == "ModelGroup") {
            if (dynamic_cast<ModelGroup*>(it.second)->OnlyContainsModel(m->Name())) {
                onlyGroups.Add(it.first);
                allGroups.Add(it.first);
            } else if (dynamic_cast<ModelGroup*>(it.second)->ContainsModelOrSubmodel(m)) {
                allGroups.Add(it.first);
            }
        }
    }

    std::string res;
    if (allGroups.size() == 0) {
        return res;
    }

    CheckboxSelectDialog dlg(GetXLightsFrame(), "Select Groups to Export - cancel to include no groups", allGroups, onlyGroups);
    if (dlg.ShowModal() == wxID_OK) {
        onlyGroups = dlg.GetSelectedItems();
    } else {
        return res;
    }

    if (onlyGroups.size() == 0) {
        return res;
    }

    for (const auto& it : models) {
        if (onlyGroups.Index(it.first) != wxNOT_FOUND) {
            res += dynamic_cast<ModelGroup*>(it.second)->SerialiseModelGroup(m->Name());
        }
    }

    return res;
}

void ModelManager::AddModelGroups(wxXmlNode* n, int w, int h, const std::string& mname, bool& merge, bool& ask)
{
    // static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("ModelManager adding groups.");

    auto grpModels = n->GetAttribute("models");
    if (grpModels.length() == 0)
        return;

    auto mgname = n->GetAttribute("name");
    if (models.find(mgname) != models.end()) {
        if (ask) {
            wxMessageDialog confirm(GetXLightsFrame(), _("Model contains Model Group(s) that Already Exist.\n Would you Like to Add this Model to the Existing Groups?"), _("Model Group(s) Already Exists"), wxYES_NO);
            int returnCode = confirm.ShowModal();
            if (returnCode == wxID_YES)
                merge = true;
            ask = false;
        }
        if (merge) { // merge
            Model* mg = GetModel(mgname);
            if (mg->GetDisplayAs() == "ModelGroup") {
                ModelGroup* mmg = dynamic_cast<ModelGroup*>(mg);
                bool found = false;
                std::vector<wxString> prevousNames;
                auto oldModelNames = mmg->ModelNames(); // we copy the name list as we are going to be modifying the group while we iterate over these
                for (const auto& it : oldModelNames) {
                    auto mmnmn = mmg->ModelNames();
                    if (Contains(it, "/")) { // only add new SubModel if the name matches an old SubModel name, I don't understand why?
                        auto mgmn = wxString(it);
                        mgmn = mname + "/" + mgmn.AfterFirst('/');
                        std::string em = "EXPORTEDMODEL/" + mgmn.AfterFirst('/');
                        if (ContainsBetweenCommas(grpModels, em) && std::find(mmnmn.begin(), mmnmn.end(), mgmn.ToStdString()) == mmnmn.end() &&
                            std::find(prevousNames.begin(), prevousNames.end(), mgmn) == prevousNames.end() &&
                            !mmg->DirectlyContainsModel(mgmn)) {
                            mmg->AddModel(mgmn);
                            prevousNames.push_back(mgmn);
                            found = true;
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
                    const auto& newNames = wxSplit(grpModels, ',');
                    for (const auto& it : newNames) {
                        auto& mmnmn = mmg->ModelNames();
                        if (Contains(it, "/")) {
                            auto mgmn = wxString(it);
                            mgmn.Replace("EXPORTEDMODEL", mname);
                            if (std::find(mmnmn.begin(), mmnmn.end(), mgmn.ToStdString()) == mmnmn.end() &&
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
        nn = wxString::Format("%s_%d", mgname, i++).ToStdString();
    }
    ModelGroup* mg = new ModelGroup(n, *this, w, h, nn, mname);
    AddModel(mg);
}

bool ModelManager::RecalcStartChannels() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    wxStopWatch sw;
    bool changed = false;
    std::set<std::string> modelsDone;
    std::set<std::string> modelsOnNoController;

    for (const auto& it : models) {
        it.second->CouldComputeStartChannel = false;
    }

    // first go through all models whose start channels are not dependent on other models
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            char first = '0';
            if (!Trim(it.second->ModelStartChannel).empty())
                first = Trim(it.second->ModelStartChannel)[0];
            if (first != '>' && first != '@') {
                modelsDone.emplace(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->SetFromXml(it.second->GetModelXml());
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
            if (it.second->GetDisplayAs() != "ModelGroup") {
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
                        it.second->SetFromXml(it.second->GetModelXml());
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
        if (it.second->GetDisplayAs() != "ModelGroup") {
            char first = '0';
            if (!Trim(it.second->ModelStartChannel).empty())
                first = Trim(it.second->ModelStartChannel)[0];
            if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel) {
                modelsDone.emplace(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->SetFromXml(it.second->GetModelXml());
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

    long end = sw.Time();
    logger_base.debug("RecalcStartChannels takes %ldms.", end);

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
        if (it.second->GetDisplayAs() != "ModelGroup" && !it.second->CouldComputeStartChannel) {
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
    while (checks <= sameOutput.size()) {
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        ReworkStartChannel.");

    bool outputsChanged = false;

    OutputManager* outputManager = xlights->GetOutputManager();
    for (const auto& it : outputManager->GetControllers()) {
        auto caps = it->GetControllerCaps();

        wxString serialPrefix;
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
                wxString cc;
                if (IsPixelProtocol(itm.second->GetControllerProtocol())) {
                    cc = wxString::Format("%s:%02d:%02d", itm.second->GetControllerProtocol(), itm.second->GetControllerPort(), itm.second->GetSortableSmartRemote()).Lower();
                } else {
                    cc = wxString::Format("%s%s:%02d", serialPrefix, itm.second->GetControllerProtocol(), itm.second->GetControllerPort()).Lower();
                }
                if (cmodels.find(cc) == cmodels.end()) {
                    std::list<Model*> ml;
                    cmodels[cc] = ml;
                }
                cmodels[cc].push_back(itm.second);
            }
        }

        // first of all fix any weirdness ...
        for (const auto& itcc : cmodels) {
            logger_zcpp.debug("Fixing weirdness on %s - %s", (const char*)it->GetName().c_str(), (const char*)itcc.first.c_str());
            logger_zcpp.debug("    Models at start:");

            // build a list of model names on the port
            std::list<std::string> models;
            for (auto& itmm : itcc.second) {
                logger_zcpp.debug("        %s Chained to '%s'", (const char*)itmm->GetName().c_str(), (const char*)itmm->GetModelChain().c_str());
                models.push_back(itmm->GetName());
            }

            logger_zcpp.debug("    Fixing weirdness:");

            // If a model refers to a chained model not on the port then move it to beginning ... so next step can move it again
            bool beginningFound = false;
            for (const auto& itmm : itcc.second) {
                auto ch = itmm->GetModelChain();
                if (ch.empty() || ch == "Beginning") {
                    beginningFound = true;
                } else {
                    ch = ch.substr(1); // string off leading >
                    if (std::find(models.begin(), models.end(), ch) == models.end()) {
                        logger_zcpp.debug("    Model %s set to beginning because the model it is chained to '%s' does not exist.", (const char*)itmm->GetName().c_str(), (const char*)ch.c_str());
                        itmm->SetModelChain("");
                        beginningFound = true;
                        outputsChanged = true;
                    }
                }
            }

            // If no model is set as beginning ... then just make the first one beginning
            if (!beginningFound) {
                logger_zcpp.debug("    Model %s set to beginning because no other model was.", (const char*)itcc.second.front()->GetName().c_str());
                itcc.second.front()->SetModelChain("");
                outputsChanged = true;
            }

            // Now I would love to give any more than the first model a default to chain to but this is
            // not as easy as it looks ... so for now i am going to leave multiple models at the beginning
            // and let the user sort it out rather than creating loops
        }

        logger_zcpp.debug("    Sorting models:");
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
                        // wxASSERT(false);
                        logger_zcpp.error("    Model chain is broken so just stuffing the remaining %d models in in their original order.", (*itcc).second.size());
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
                    logger_zcpp.error("    DMX Model chain is broken or there are duplicate models so just stuffing the remaining %d models in in their original order.", (*itcc).second.size());
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
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", ch);
                        itm->SetStartChannel(sc);
                        itm->ClearIndividualStartChannels();
                        last = itm->GetName();
                        ch += itm->GetChanCount();
                        if (osc != itm->ModelStartChannel) {
                            outputsChanged = true;
                        }
                    } else {
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", chstart);
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

                        logger_base.warn("Attempt to place a second model %s on led panel port when only one is allowed. Only the first model has been retained. The others have been removed.", (const char*)itm->GetName().c_str());

                    } else {
                        std::string osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", chstart);
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
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", ch);
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
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", msc);
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

                logger_zcpp.debug("    Model %s on port %d chained to %s start channel %s.",
                                  (const char*)itm->GetName().c_str(),
                                  itm->GetControllerPort(),
                                  (const char*)itm->GetModelChain().c_str(),
                                  (const char*)sc.c_str());
            }

            allSortedModels.splice(allSortedModels.end(), sortedmodels);
        }

        if (it->IsAutoSize()) {
            auto eth = dynamic_cast<const ControllerEthernet*>(it);
            if (it->GetChannels() != std::max((int32_t)1, (int32_t)ch - 1) || (eth != nullptr && eth->IsUniversePerString())) {
                logger_zcpp.debug("    Resizing output to %d channels.", std::max((int32_t)1, (int32_t)ch - 1));

                auto oldC = it->GetChannels();
                // Set channel size won't always change the number of channels for some protocols
                it->SetChannelSize(std::max((int32_t)1, (int32_t)ch - 1), allSortedModels);
                if (it->GetChannels() != oldC || (eth != nullptr && eth->IsUniversePerString())) {
                    outputManager->SomethingChanged();

                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ReworkStartChannel");
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
        m->SetStartChannel(wxString::Format("%u", lastChannel + 1));
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

    if (m->HasIndividualStartChannels()) {
        size_t c = m->GetNumPhysicalStrings();
        for (size_t i = 0; i < c; ++i) {
            wxString sc = m->GetIndividualStartChannel(i);
            if (sc != "" && (sc[0] == '>' || sc[0] == '@')) {
                std::string dependson = sc.substr(1).BeforeFirst(':');
                Model* mm = GetModel(dependson);
                if (mm != nullptr) {
                    if (mm->GetControllerName() == NO_CONTROLLER)
                        return false;
                    if (!ModelHasNoDependencyOnNoController(mm, visited))
                        return false;
                }
            }
        }
    } else {
        wxString sc = m->ModelStartChannel;
        if (sc != "" && (sc[0] == '>' || sc[0] == '@')) {
            std::string dependson = sc.substr(1).BeforeFirst(':');
            Model* mm = GetModel(dependson);
            if (mm != nullptr) {
                if (mm->GetControllerName() == NO_CONTROLLER)
                    return false;
                return ModelHasNoDependencyOnNoController(mm, visited);
            }
        }
    }

    return true;
}

bool ModelManager::LoadGroups(wxXmlNode* groupNode, int previewW, int previewH)
{
    // static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // logger_base.debug("ModelManager loading groups.");

    this->groupNode = groupNode;
    bool changed = false;

    std::list<wxXmlNode*> toBeDone;
    std::set<std::string> allModels;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    // do all the models without embedded groups first or where the model order means everything exists
    for (wxXmlNode* e = groupNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                allModels.insert(name);
                if (ModelGroup::AllModelsExist(e, *this)) {
                    ModelGroup* model = new ModelGroup(e, *this, previewW, previewH);
                    models[model->name] = model;
                    model->SetLayoutGroup(e->GetAttribute("LayoutGroup", "Unassigned").ToStdString());
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
        std::list<wxXmlNode*> processing(toBeDone);
        toBeDone.clear();
        for (const auto& it : processing) {
            if (ModelGroup::AllModelsExist(it, *this)) {
                ModelGroup* model = new ModelGroup(it, *this, previewW, previewH);
                bool reset = model->Reset();
                wxASSERT(reset);
                models[model->name] = model;
                model->SetLayoutGroup(it->GetAttribute("LayoutGroup", "Unassigned").ToStdString());
            } else {
                toBeDone.push_back(it);
            }
        }
    }

    // anything left in toBeDone is now due to model loops
    for (const auto& it : toBeDone) {
        std::string name = it->GetAttribute("name").ToStdString();
        std::string msg = "Could not process model group " + name + " likely due to model groups loops. See Check Sequence for details.";
        DisplayWarning(msg);
        wxASSERT(false);
        ModelGroup* model = new ModelGroup(it, *this, previewW, previewH);
        bool reset = model->Reset();
        wxASSERT(!reset); // this should have failed
        models[model->name] = model;
        model->SetLayoutGroup(it->GetAttribute("LayoutGroup", "Unassigned").ToStdString());
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
    return GenerateObjectName(candidateName);
}

Model* ModelManager::CreateDefaultModel(const std::string& type, const std::string& startChannel) const
{
    Model* model;
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "model");
    node->AddAttribute("DisplayAs", type);
    node->AddAttribute("StringType", "RGB Nodes");
    node->AddAttribute("StartSide", "B");
    node->AddAttribute("Dir", "L");
    node->AddAttribute("Antialias", "1");
    node->AddAttribute("PixelSize", "2");
    node->AddAttribute("Transparency", "0");
    node->AddAttribute("parm1", "1");
    node->AddAttribute("parm2", "50");
    node->AddAttribute("parm3", "1");
    node->AddAttribute("StartChannel", startChannel);
    node->AddAttribute("LayoutGroup", "Unassigned");

    std::string name = GenerateModelName(type);
    node->AddAttribute("name", name);
    std::string protocol = "ws2811";

    if (type == "Star") {
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "5");
        node->AddAttribute("StarStartLocation", "Bottom Ctr-CW");
        node->DeleteAttribute("Dir");
        node->DeleteAttribute("StartSide");
        model = new StarModel(node, *this, false);
    } else if (type == "Arches") {
        model = new ArchesModel(node, *this, false);
    } else if (type == "Candy Canes") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "3");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "18");
        model = new CandyCaneModel(node, *this, false);
    } else if (type == "Channel Block") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        node->DeleteAttribute("PixelSize");
        node->AddAttribute("PixelSize", "12");
        model = new ChannelBlockModel(node, *this, false);
    } else if (type == "Circle") {
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "50");
        node->AddAttribute("InsideOut", "0");
        model = new CircleModel(node, *this, false);
    } else if (type == "DmxMovingHead") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "8");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("DmxStyle");
        node->AddAttribute("DmxStyle", "Moving Head Top");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxMovingHead(node, *this, false);
    } else if (type == "DmxGeneral") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "8");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxGeneral(node, *this, false);
    } else if (type == "DmxMovingHead3D") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "8");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxMovingHead3D(node, *this, false);
    } else if (type == "DmxFloodlight") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "3");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxFloodlight(node, *this, false);
    } else if (type == "DmxFloodArea") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "3");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxFloodArea(node, *this, false);
    } else if (type == "DmxSkull") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "26");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxSkull(node, *this, false);
    } else if (type == "DmxServo") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "2");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new DmxServo(node, *this, false);
    } else if (type == "DmxServo3d" || type == "DmxServo3Axis") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "2");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        if (type == "DmxServo3Axis") {
            node->DeleteAttribute("DisplayAs");
            node->AddAttribute("DisplayAs", "DmxServo3d");
            node->DeleteAttribute("NumServos");
            node->AddAttribute("NumServos", "3");
            node->DeleteAttribute("NumStatic");
            node->AddAttribute("NumStatic", "1");
            node->DeleteAttribute("NumMotion");
            node->AddAttribute("NumMotion", "3");
            node->DeleteAttribute("parm1");
            node->AddAttribute("parm1", "6");
        }
        model = new DmxServo3d(node, *this, false);
    } else if (type == "Image") {
        protocol = "";
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "1");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("Image");
        node->AddAttribute("Image", "");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new ImageModel(node, *this, false);
    } else if (type == "Window Frame") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "16");
        node->AddAttribute("Rotation", "CW");
        model = new WindowFrameModel(node, *this, false);
    } else if (type == "Wreath") {
        model = new WreathModel(node, *this, false);
    } else if (type.find("Sphere") == 0) {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "10");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "10");
        model = new SphereModel(node, *this, false);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, *this, false);
    } else if (type == "Poly Line") {
        model = new PolyLineModel(node, *this, false);
    } else if (type == "MultiPoint") {
        model = new MultiPointModel(node, *this, false);
    } else if (type == "Cube") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "5");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "5");
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "5");
        node->DeleteAttribute("Style");
        node->AddAttribute("Style", "Horizontal Left/Right");
        model = new CubeModel(node, *this, false);
    } else if (type == "Custom") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "5");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "5");
        node->AddAttribute("CustomModel", ",,,,;,,,,;,,,,;,,,,;,,,,");
        model = new CustomModel(node, *this, false);
    } else if (type.find("Tree") == 0) {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", "Tree 360");
        model = new TreeModel(node, *this, false);
    } else if (type == "Matrix") {
        node->DeleteAttribute("StartSide");
        node->AddAttribute("StartSide", "T");
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", "Horiz Matrix");
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        model = new MatrixModel(node, *this, false);
    } else if (type == "Spinner") {
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "10");
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "5");
        node->DeleteAttribute("Hollow");
        node->AddAttribute("Hollow", "20");
        node->DeleteAttribute("Arc");
        node->AddAttribute("Arc", "360");
        model = new SpinnerModel(node, *this, false);
    } else if (type == "Icicles") {
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "80");
        node->AddAttribute("DropPattern", "3,4,5,4");
        model = new IciclesModel(node, *this, false);
    } else {
        DisplayError(wxString::Format("'%s' is not a valid model type for model '%s'", type, node->GetAttribute("name")).ToStdString());
        return nullptr;
    }

    model->SetControllerProtocol(protocol);
    model->SetControllerName(NO_CONTROLLER);

    return model;
}

Model* ModelManager::CreateModel(wxXmlNode* node, int previewW, int previewH, bool zeroBased) const
{
    if (node->GetName() == "modelGroup") {
        ModelGroup* grp = new ModelGroup(node, *this, previewWidth, previewHeight);
        grp->Reset(zeroBased);
        return grp;
    }
    std::string type = node->GetAttribute("DisplayAs").ToStdString();

    // Upgrade older DMX models
    if (type == "DMX") {
        std::string style = node->GetAttribute("DmxStyle", "DMX");
        if (style == "Moving Head Top" ||
            style == "Moving Head Side" ||
            style == "Moving Head Bars" ||
            style == "Moving Head TopBars" ||
            style == "Moving Head SideBars") {
            type = "DmxMovingHead";
        } else if (style == "Moving Head 3D") {
            type = "DmxMovingHead3D";
        } else if (style == "Flood Light") {
            type = "DmxFloodlight";
        } else if (style == "Skulltronix Skull") {
            type = "DmxSkulltronix";
        } else {
            type = "DmxMovingHead";
        }
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", type);
    } else if (type == "DmxMovingHead3D") {
        std::string version = node->GetAttribute("versionNumber").ToStdString();
        // After version 2020.3 the DmxMovingHead3D is being moved back to the DmxMovingHead class so the 3D version is mesh only
        if (version == "4") {
            type = "DmxMovingHead";
            node->DeleteAttribute("DisplayAs");
            node->AddAttribute("DisplayAs", type);
            node->DeleteAttribute("DmxStyle");
            node->AddAttribute("DmxStyle", "Moving Head 3D");
        }
    } else if (type == "DmxServo3Axis") {
        type = "DmxServo3d";
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", type);
        node->DeleteAttribute("NumServos");
        node->AddAttribute("NumServos", "3");
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "6");
    }

    Model* model;
    if (type == "Star") {
        model = new StarModel(node, *this, zeroBased);
    } else if (type == "Arches") {
        model = new ArchesModel(node, *this, zeroBased);
    } else if (type == "Candy Canes") {
        model = new CandyCaneModel(node, *this, zeroBased);
    } else if (type == "Channel Block") {
        model = new ChannelBlockModel(node, *this, zeroBased);
    } else if (type == "Circle") {
        model = new CircleModel(node, *this, zeroBased);
    } else if (type == "DmxMovingHead") {
        model = new DmxMovingHead(node, *this, zeroBased);
    } else if (type == "DmxGeneral") {
        model = new DmxGeneral(node, *this, zeroBased);
    } else if (type == "DmxMovingHead3D") {
        model = new DmxMovingHead3D(node, *this, zeroBased);
    } else if (type == "DmxFloodlight") {
        model = new DmxFloodlight(node, *this, zeroBased);
    } else if (type == "DmxFloodArea") {
        model = new DmxFloodArea(node, *this, zeroBased);
    } else if (type == "DmxSkull") {
        model = new DmxSkull(node, *this, zeroBased);
    } else if (type == "DmxSkulltronix") {
        model = new DmxSkulltronix(node, *this, zeroBased);
        wxMessageBox("Alert!  The Skulltronix model type is deprecated and may soon be deleted.  Please switch to either the DmxSkull model or one of the DmxServo models if you were just using this model to control servos not in a skull.", "Alert", wxOK | wxCENTER);
    } else if (type == "DmxServo") {
        model = new DmxServo(node, *this, zeroBased);
    } else if (type == "DmxServo3d") {
        model = new DmxServo3d(node, *this, zeroBased);
    } else if (type == "Image") {
        model = new ImageModel(node, *this, zeroBased);
    } else if (type == "Window Frame") {
        model = new WindowFrameModel(node, *this, zeroBased);
    } else if (type == "Wreath") {
        model = new WreathModel(node, *this, zeroBased);
    } else if (type.find("Sphere") == 0) {
        model = new SphereModel(node, *this, zeroBased);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, *this, zeroBased);
    } else if (type == "Poly Line") {
        model = new PolyLineModel(node, *this, zeroBased);
    } else if (type == "MultiPoint") {
        model = new MultiPointModel(node, *this, zeroBased);
    } else if (type == "Cube") {
        model = new CubeModel(node, *this, zeroBased);
    } else if (type == "Custom") {
        model = new CustomModel(node, *this, zeroBased);
    } else if (type.find("Tree") == 0) {
        model = new TreeModel(node, *this, zeroBased);
    } else if (type.find("Icicles") == 0) {
        model = new IciclesModel(node, *this, zeroBased);
    } else if (type == "WholeHouse") {
        model = new WholeHouseModel(node, *this, zeroBased);
    } else if (type == "Vert Matrix" || type == "Horiz Matrix") {
        model = new MatrixModel(node, *this, zeroBased);
    } else if (type == "Spinner") {
        model = new SpinnerModel(node, *this, zeroBased);
    } else {
        DisplayError(wxString::Format("'%s' is not a valid model type for model '%s'", type, node->GetAttribute("name")).ToStdString());
        return nullptr;
    }
    model->GetModelScreenLocation().previewW = previewW;
    model->GetModelScreenLocation().previewH = previewH;
    if (model->GetModelScreenLocation().CheckUpgrade(node) == ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_EXEC_READ) {
        model->GetModelScreenLocation().Read(node);
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

        if ("ModelGroup" == model->GetDisplayAs()) {
            if (model->GetModelXml()->GetParent() != groupNode) {
                if (model->GetModelXml()->GetParent() != nullptr) {
                    model->GetModelXml()->GetParent()->RemoveChild(model->GetModelXml());
                }
                groupNode->AddChild(model->GetModelXml());
            }
        } else {
            if (model->GetModelXml()->GetParent() != modelNode) {
                if (model->GetModelXml()->GetParent() != nullptr) {
                    model->GetModelXml()->GetParent()->RemoveChild(model->GetModelXml());
                }
                modelNode->AddChild(model->GetModelXml());
            }
        }
    }
}
void ModelManager::ReplaceModel(const std::string &name, Model* nm) {
    if (nm != nullptr && name != "") {
        std::lock_guard<std::recursive_mutex> _lock(_modelMutex);
        Model *oldm = models[name];
        models[nm->name] = nm;
        ResetModelGroups();

        if ("ModelGroup" == nm->GetDisplayAs()) {
            if (nm->GetModelXml()->GetParent() != groupNode) {
                if (nm->GetModelXml()->GetParent() != nullptr) {
                    nm->GetModelXml()->GetParent()->RemoveChild(nm->GetModelXml());
                }
                groupNode->AddChild(nm->GetModelXml());
            }
        } else {
            if (nm->GetModelXml()->GetParent() != modelNode) {
                if (nm->GetModelXml()->GetParent() != nullptr) {
                    nm->GetModelXml()->GetParent()->RemoveChild(nm->GetModelXml());
                }
                modelNode->AddChild(nm->GetModelXml());
            }
        }
        oldm->GetModelXml()->GetParent()->RemoveChild(oldm->GetModelXml());
        delete oldm->GetModelXml();
        delete oldm;
    }
}

Model* ModelManager::createAndAddModel(wxXmlNode* node, int previewW, int previewH)
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
        if (it.second->GetDisplayAs() != "ModelGroup") {
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

std::vector<std::string> ModelManager::GetGroupsContainingModel(Model* model) const
{
    std::vector<std::string> res;
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() == "ModelGroup") {
            auto mg = dynamic_cast<ModelGroup*>(it.second);
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

std::vector<std::string> ModelManager::GetGroupsContainingModelOrSubmodel(Model* model) const
{
    std::vector<std::string> res;
    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() == "ModelGroup") {
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

std::string ModelManager::GenerateNewStartChannel(const std::string& lastModel) const
{
    std::string startChannel = "1";

    if (!lastModel.empty() && models.count(lastModel) > 0) {
        startChannel = ">" + lastModel + ":1";
    } else {
        unsigned int highestch = 0;
        Model* highest = nullptr;
        for (const auto& it : models) {
            if (it.second->GetDisplayAs() != "ModelGroup") {
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

bool ModelManager::MergeFromBase(const std::string& baseShowDir, bool prompt)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool changed = false;

    wxXmlDocument doc;
    doc.Load(baseShowDir + wxFileName::GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE);
    if (!doc.IsOk()) {
        return false;
    }

    // TODO may also want to bring in some show level things

    wxXmlNode* models = nullptr;
    wxXmlNode* modelgroups = nullptr;
    for (wxXmlNode* m = doc.GetRoot(); m != nullptr; m = m->GetNext()) {
        for (wxXmlNode* mm = m->GetChildren(); mm != nullptr; mm = mm->GetNext()) {
            if (mm->GetName() == "models") {
                models = mm;
            } else if (mm->GetName() == "modelGroups") {
                modelgroups = mm;
            }
        }
    }

    if (models != nullptr) {
        // compare models and load changes/new models
        for (wxXmlNode* m = models->GetChildren(); m != nullptr; m = m->GetNext()) {
            // we only update existing models that came from the base
            auto name = m->GetAttribute("name");

            auto curr = GetModel(name);

            if (curr == nullptr) {
                // model does not exist
                changed = true;
                m->AddAttribute("FromBase", "1");
                createAndAddModel(new wxXmlNode(*m), xlights->modelPreview->getWidth(), xlights->modelPreview->getHeight());
                logger_base.debug("Adding model from base show folder: '%s'.", (const char*)name.c_str());
            } else {
                bool force = false;
                if (prompt && !curr->IsFromBase()) {
                    force = wxMessageBox(wxString::Format("Model %s found that clashes with base show directory. Do you want to take the base show directory version?", name), "Model clash", wxICON_QUESTION | wxYES_NO, xlights) == wxYES;
                }

                if (force || curr->IsFromBase()) {
                    // model does exist ... update it
                    if (force || curr->IsXmlChanged(m)) {
                        m->AddAttribute("FromBase", "1");
                        changed = true;
                        Model  *newm = CreateModel(new wxXmlNode(*m));
                        ReplaceModel(name, newm);
                        logger_base.debug("Updating model from base show folder: '%s'.", (const char*)name.c_str());
                    }
                } else {
                    logger_base.debug("Model '%s' NOT updated from base show folder as it never came from there.", (const char*)name.c_str());
                }
            }
        }
    }

    if (modelgroups != nullptr) {
        // compare model groups and load changes/new model groups
        for (wxXmlNode* m = modelgroups->GetChildren(); m != nullptr; m = m->GetNext()) {
            auto name = m->GetAttribute("name");

            auto curr = GetModel(name);

            if (curr == nullptr) {
                changed = true;
                m->AddAttribute("FromBase", "1");
                createAndAddModel(new wxXmlNode(*m), xlights->modelPreview->getWidth(), xlights->modelPreview->getHeight());
                logger_base.debug("Adding model group from base show folder: '%s'.", (const char*)name.c_str());
            } else {

                bool force = false;
                if (prompt && !curr->IsFromBase()) {
                    force = wxMessageBox(wxString::Format("Model Group %s found that clashes with base show directory. Do you want to take the base show directory version?", name), "Model group clash", wxICON_QUESTION | wxYES_NO, xlights) == wxYES;
                }

                // we only update existing models that came from the base via a previous import
                if (force || curr->IsFromBase()) {
                    auto mg = dynamic_cast<ModelGroup*>(curr);

                    if (mg != nullptr) {
                        // we need to merge in any models in the current rgbeffects file after the ones inhereted from the base
                        auto models1 = m->GetAttribute("models");
                        auto models2 = mg->ModelNames();
                        std::string mm2 = "";
                        for (const auto& it : models2) {
                            if (mm2 != "")
                                mm2 += ",";
                            mm2 += it;
                        }
                        m->DeleteAttribute("models");
                        m->AddAttribute("models", MergeModels(models1, mm2));
                        if (force || curr->IsXmlChanged(m)) {
                            m->AddAttribute("FromBase", "1");
                            m->AddAttribute("BaseModels", models1); // keep a copy of the models from the base show folder as we may want to prevent these being removed
                            changed = true;                            
                            Model  *newm = CreateModel(new wxXmlNode(*m));
                            ReplaceModel(name, newm);
                            logger_base.debug("Updating model group from base show folder: '%s'.", (const char*)name.c_str());
                        }
                    }
                } else {
                    logger_base.debug("Model Group '%s' NOT updated from base show folder as it never came from there.", (const char*)name.c_str());
                }
            }
        }
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
                if (wxMessageBox("Model '" + name + "' exists in the currently open sequence and has effects on it. Delete all effects and layers on this model?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) != wxYES)
                    return false;
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
                model->GetModelXml()->GetParent()->RemoveChild(model->GetModelXml());

                for (auto& it2 : models) {
                    if (it2.second->GetDisplayAs() == "ModelGroup") {
                        ModelGroup* group = (ModelGroup*)it2.second;
                        group->ModelRemoved(mn);
                    }
                }
                models.erase(it);
                ResetModelGroups();

                // If models are chained to us then make their start channel ... our start channel
                std::string chainedtous = wxString::Format(">%s:1", model->GetName()).ToStdString();
                for (auto it3 : models) {
                    if (it3.second->ModelStartChannel == chainedtous) {
                        it3.second->SetStartChannel(model->ModelStartChannel);
                    }
                }

                delete model->GetModelXml();
                delete model;
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
