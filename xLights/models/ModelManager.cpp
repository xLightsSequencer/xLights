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

#include "ModelManager.h"
#include "Model.h"
#include "SubModel.h"
#include "StarModel.h"
#include "ArchesModel.h"
#include "CandyCaneModel.h"
#include "ChannelBlockModel.h"
#include "CircleModel.h"
#include "TreeModel.h"
#include "CubeModel.h"
#include "CustomModel.h"
#include "DMX/DmxServo.h"
#include "DMX/DmxServo3D.h"
#include "DMX/DmxSkull.h"
#include "DMX/DmxSkulltronix.h"
#include "DMX/DmxFloodlight.h"
#include "DMX/DmxFloodArea.h"
#include "DMX/DmxMovingHead.h"
#include "DMX/DmxMovingHead3D.h"
#include "ImageModel.h"
#include "WholeHouseModel.h"
#include "SingleLineModel.h"
#include "PolyLineModel.h"
#include "ModelGroup.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "IciclesModel.h"
#include "../sequencer/Element.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "outputs/Output.h"
#include "outputs/Controller.h"

#include <log4cpp/Category.hh>

ModelManager::ModelManager(OutputManager* outputManager, xLightsFrame* xl) :
    _outputManager(outputManager),
    xlights(xl),
    layoutsNode(nullptr),
    previewWidth(0),
    previewHeight(0),
    _modelsLoading(false)
{
    //ctor
}

ModelManager::~ModelManager() {

    // Because loading models is async we have to ensure this is done before we destroy anything
    while (_modelsLoading) wxMilliSleep(1);

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

inline BaseObject *ModelManager::GetObject(const std::string &name) const {
    return GetModel(name);
}

bool ModelManager::IsModelValid(Model* m) const {

    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second == m) return true;

        for (auto it2 : it.second->GetSubModels()) {
            if (it2 == m) return true;
        }
    }
    return false;
}

Model *ModelManager::GetModel(const std::string &name) const {
    std::unique_lock<std::recursive_mutex> lock(_modelMutex);
    auto it = models.find(Trim(name));
    lock.unlock();

    if (it == models.end()) {
        size_t pos = name.find("/");
        if (pos != std::string::npos) {
            std::string mname = Trim(name.substr(0, pos));
            std::string smname = Trim(name.substr(pos + 1));
            Model *m = GetModel(mname);
            if (m != nullptr) {
                return m->GetSubModel(smname);
            }
        }
        return nullptr;
    }
    return it->second;
}

Model *ModelManager::operator[](const std::string &name) const {
    return GetModel(Trim(name));
}

bool ModelManager::Rename(const std::string &oldName, const std::string &newName) {
    auto on = Trim(oldName);
    auto nn = Trim(newName);
    Model *model = GetModel(on);
    if (model == nullptr || model->GetDisplayAs() == "SubModel") {
        return false;
    }
    model->GetModelXml()->DeleteAttribute("name");
    model->GetModelXml()->AddAttribute("name", nn);
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
            if (mg != nullptr)
            {
                changed |= mg->ModelRenamed(on, nn);
            }
        }

        return changed;
    }

    return false;
}

bool ModelManager::RenameSubModel(const std::string &oldName, const std::string &newName) {

    bool changed = false;

    auto on = Trim(oldName);
    auto nn = Trim(newName);

    for (auto& m : *this)
    {
        if (m.second->GetDisplayAs() == "ModelGroup")
        {
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

    Model *model = GetModel(on);
    if (model == nullptr) return false;
    
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    models.erase(models.find(on));
    models[nn] = model;
    return true;
}

bool ModelManager::IsModelOverlapping(Model* model) const
{
    int32_t start = model->GetNumberFromChannelString(model->ModelStartChannel);
    int32_t end = start + model->GetChanCount() - 1;
    //int32_t sstart = model->GetFirstChannel() + 1;
    //wxASSERT(sstart == start);
    //int32_t send = model->GetLastChannel() + 1;
    //wxASSERT(send == end);
    for (auto it = this->begin(); it != this->end(); ++it)
    {
        if (it->second->GetDisplayAs() != "ModelGroup" && it->second->GetName() != model->GetName())
        {
            int32_t s = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
            int32_t e = s + it->second->GetChanCount() - 1;
            //int32_t ss = it->second->GetFirstChannel() + 1;
            //wxASSERT(ss == s);
            //int32_t se = it->second->GetLastChannel() + 1;
            //wxASSERT(se == e);
            if (start <= e && end >= s) return true;
        }
    }
    return false;
}

void ModelManager::LoadModels(wxXmlNode* modelNode, int previewW, int previewH)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _modelsLoading = true;
    clear();
    previewWidth = previewW;
    previewHeight = previewH;
    this->modelNode = modelNode;
    wxStopWatch timer;
    {
        std::vector<std::future<Model*>> modelsLoaded;
        for (wxXmlNode* e = modelNode->GetChildren(); e != nullptr; e = e->GetNext()) {
            if (e->GetName() == "model") {
                std::string name = e->GetAttribute("name").Trim(true).Trim(false).ToStdString();
                if (!name.empty()) {
                    modelsLoaded.push_back(std::async(std::launch::async, [this, e, previewW, previewH]() {
                        return createAndAddModel(e, previewW, previewH);
                        }));
                }
            }
        }
    }
    logger_base.debug("Models loaded in %ldms", timer.Time());
    _modelsLoading = false;

    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ModelManager::LoadModels");
    //RecalcStartChannels();
}

uint32_t ModelManager::GetLastChannel() const {
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    unsigned int max = 0;
    for (const auto& it : models) {
        max = std::max(max, it.second->GetLastChannel());
    }
    return max;
}

void ModelManager::ResetModelGroups() const
{
    // This goes through all the model groups which hold model pointers and ensure their model pointers are correct
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models) {
        if (it.second != nullptr && it.second->GetDisplayAs() == "ModelGroup") {
            ((ModelGroup*)(it.second))->ResetModels();
        }
    }
}

std::string ModelManager::GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel, const std::string& protocol) const
{
    std::string last = "";
    unsigned int highestEndChannel = 0;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    for (const auto& it : models)
    {
        if (it.second->GetDisplayAs() != "ModelGroup" && 
            it.second->GetControllerName() == controllerName && 
            it.second->GetControllerPort() == port && 
            it.second->GetControllerProtocol() == protocol &&
            it.second->GetLastChannel() > highestEndChannel && it.first != excludeModel)
        {
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

    for (const auto& it : models)
    {
        if (it.second->GetDisplayAs() != "ModelGroup" &&
            it.second->GetControllerName() == controllerName &&
            it.second->GetControllerPort() == port &&
            it.second->GetControllerProtocol() == protocol &&
            it.second->GetSmartRemote() == smartReceiver &&
            it.second->GetLastChannel() > highestEndChannel&& it.first != excludeModel)
        {
            last = it.first;
            highestEndChannel = it.second->GetLastChannel();
        }
    }
    return last;
}

void ModelManager::ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP)
{
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);
    for (const auto& it : models)
    {
        if (it.second->GetDisplayAs() != "ModelGroup")
        {
            if (Contains(it.second->ModelStartChannel, oldIP))
            {
                it.second->ReplaceIPInStartChannels(oldIP, newIP);
            }
        }
    }
}

bool ModelManager::RecalcStartChannels() const {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    wxStopWatch sw;
    bool changed = false;
    std::list<std::string> modelsDone;

    for (const auto& it : models) {
        it.second->CouldComputeStartChannel = false;
    }

    // first go through all models whose start channels are not dependent on other models
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != "ModelGroup")
        {
            char first = '0';
            if (Trim(it.second->ModelStartChannel) != "") first = Trim(it.second->ModelStartChannel)[0];
            if (first != '>' && first != '@')
            {
                modelsDone.push_back(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->SetFromXml(it.second->GetModelXml());
                if (oldsc != it.second->GetFirstChannel())
                {
                    changed = true;
                }
            }
        }
    }

    // now go through all undone models that depend on something
    bool workDone = false;
    do
    {
        workDone = false;

        for (const auto& it : models) {
            if (it.second->GetDisplayAs() != "ModelGroup")
            {
                char first = '0';
                if (Trim(it.second->ModelStartChannel) != "") first = Trim(it.second->ModelStartChannel)[0];
                if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel)
                {
                    std::string dependsOn = Trim(Trim(it.second->ModelStartChannel).substr(1, Trim(it.second->ModelStartChannel).find(':') - 1));
                    if (std::find(modelsDone.begin(), modelsDone.end(), dependsOn) != modelsDone.end())
                    {
                        // the depends on model is done
                        modelsDone.push_back(it.first);
                        auto oldsc = it.second->GetFirstChannel();
                        it.second->SetFromXml(it.second->GetModelXml());
                        if (oldsc != it.second->GetFirstChannel())
                        {
                            changed = true;
                        }
                        if (it.second->CouldComputeStartChannel)
                        {
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
        if (it.second->GetDisplayAs() != "ModelGroup")
        {
            char first = '0';
            if (Trim(it.second->ModelStartChannel) != "") first = Trim(it.second->ModelStartChannel)[0];
            if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel)
            {
                modelsDone.push_back(it.first);
                auto oldsc = it.second->GetFirstChannel();
                it.second->SetFromXml(it.second->GetModelXml());
                if (oldsc != it.second->GetFirstChannel())
                {
                    changed = true;
                }
            }
            if (!it.second->CouldComputeStartChannel) {
                countInvalid++;
            }
        }
    }

    ResetModelGroups();

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

    if (msg != lastwarn)
    {
        DisplayWarning(msg);
        lastwarn = msg;
    }
}

bool ModelManager::IsValidControllerModelChain(Model* m, std::string& tip) const
{
    std::list<Model*> sameOutput;
    tip = "";
    auto controllerName = m->GetControllerName();
    if (controllerName == "") return true; // we dont check these

    auto port = m->GetControllerPort();
    if (port == 0) return true; // we dont check these
    
    auto chain = m->GetModelChain();
    if (StartsWith(chain, ">"))
    {
        chain = chain.substr(1);
    }
    auto startModel = m->GetName();
    auto smartRemote = m->GetSmartRemote();
    auto isPixel = m->IsPixelProtocol();

    if (!isPixel)
    {
        // For DMX outputs then beginning is always ok
        if (chain == "")
        {
            return true;
        }
    }

    for (const auto& it : *this)
    {
        if (it.first != startModel)
        {
            if (it.second->GetControllerName() == controllerName && it.second->GetControllerPort() == port && it.second->GetSmartRemote() == smartRemote && it.second->IsPixelProtocol() == isPixel)
            {
                auto c = it.second->GetModelChain();
                if (StartsWith(c, ">"))
                {
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
    if (sameOutput.size() == 0)
    {
        if (chain != "") tip = "Only model on an output must not chain to anything.";
        return (chain == "");
    }

    if (chain == "") return true; // this model is the beginning

    // valid if i can follow the chain to blank
    int checks = 0;
    auto current = startModel;
    auto next = chain;
    while (checks <= sameOutput.size())
    {
        bool found = false;
        for (const auto& it : sameOutput)
        {
            if (it->GetName() == next)
            {
                next = it->GetModelChain();
                if (StartsWith(next, ">"))
                {
                    next = next.substr(1);
                }
                if (next == "") return true; // we found the beginning
                found = true;
                current = it->GetName();
                break;
            }
        }

        if (!found)
        {
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
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        ReworkStartChannel.");

    bool  outputsChanged = false;

    OutputManager* outputManager = xlights->GetOutputManager();
    for (const auto& it : outputManager->GetControllers())
    {
        std::map<std::string, std::list<Model*>> cmodels;
        std::lock_guard<std::recursive_mutex> lock(_modelMutex);
        for (auto itm : models)
        {
            if (itm.second->GetControllerName() == it->GetName() &&
                itm.second->GetControllerPort() != 0 &&
                itm.second->GetControllerProtocol() != ""
                ) // we dont muck with unassigned models or no protocol models
            {
                wxString cc;
                if (Model::IsPixelProtocol(itm.second->GetControllerProtocol()))
                {
                    cc = wxString::Format("%s:%02d:%02d", itm.second->GetControllerProtocol(), itm.second->GetControllerPort(), itm.second->GetSortableSmartRemote()).Lower();
                }
                else
                {
                    cc = wxString::Format("%s:%02d", itm.second->GetControllerProtocol(), itm.second->GetControllerPort()).Lower();
                }
                if (cmodels.find(cc) == cmodels.end())
                {
                    std::list<Model*> ml;
                    cmodels[cc] = ml;
                }
                cmodels[cc].push_back(itm.second);
            }
        }

        // first of all fix any weirdness ...
        for (auto itcc = cmodels.begin(); itcc != cmodels.end(); ++itcc)
        {
            logger_zcpp.debug("Fixing weirdness on %s - %s", (const char*)it->GetName().c_str(), (const char*)itcc->first.c_str());
            logger_zcpp.debug("    Models at start:");

            // build a list of model names on the port
            std::list<std::string> models;
            for (auto itmm : itcc->second)
            {
                logger_zcpp.debug("        %s Chained to '%s'", (const char*)itmm->GetName().c_str(), (const char*)itmm->GetModelChain().c_str());
                models.push_back(itmm->GetName());
            }

            logger_zcpp.debug("    Fixing weirdness:");

            // If a model refers to a chained model not on the port then move it to beginning ... so next step can move it again
            bool beginningFound = false;
            for (auto itmm : itcc->second)
            {
                auto ch = itmm->GetModelChain();
                if (ch == "" || ch == "Beginning")
                {
                    beginningFound = true;
                }
                else
                {
                    ch = ch.substr(1); // string off leading >
                    if (std::find(models.begin(), models.end(), ch) == models.end())
                    {
                        logger_zcpp.debug("    Model %s set to beginning because the model it is chained to '%s' does not exist.", (const char*)itmm->GetName().c_str(), (const char*)ch.c_str());
                        itmm->SetModelChain("");
                        beginningFound = true;
                        outputsChanged = true;
                    }
                }
            }

            // If no model is set as beginning ... then just make the first one beginning
            if (!beginningFound)
            {
                logger_zcpp.debug("    Model %s set to beginning because no other model was.", (const char*)itcc->second.front()->GetName().c_str());
                itcc->second.front()->SetModelChain("");
                outputsChanged = true;
            }

            // Now I would love to give any more than the first model a default to chain to but this is
            // not as easy as it looks ... so for now i am going to leave multiple models at the beginning
            // and let the user sort it out rather than creating loops
        }

        logger_zcpp.debug("    Sorting models:");
        int32_t ch = 1;
        for (auto itcc = cmodels.begin(); itcc != cmodels.end(); ++itcc)
        {
            // order the models
            std::list<Model*> sortedmodels;
            std::string last = "";

            int32_t chstart = ch;

            if (itcc->second.size() > 0 && itcc->second.front()->IsPixelProtocol())
            {
                while ((*itcc).second.size() > 0)
                {
                    bool pushed = false;
                    for (auto itms = itcc->second.begin(); itms != itcc->second.end(); ++itms)
                    {
                        if ((((*itms)->GetModelChain() == "Beginning" || (*itms)->GetModelChain() == "") && last == "") ||
                            (*itms)->GetModelChain() == last ||
                            (*itms)->GetModelChain() == ">" + last)
                        {
                            sortedmodels.push_back(*itms);
                            pushed = true;
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        }
                    }

                    if (!pushed && (*itcc).second.size() > 0)
                    {
                        // chain is broken ... so just put the rest in in the original order
                        // wxASSERT(false);
                        logger_zcpp.error("    Model chain is broken so just stuffing the remaining %d models in in their original order.", (*itcc).second.size());
                        while ((*itcc).second.size() > 0)
                        {
                            sortedmodels.push_back(itcc->second.front());
                            itcc->second.pop_front();
                        }
                    }
                }
            }
            else
            {
                // dmx protocols wowk differently ... they can be chained or by specified dmx channel
                int dmx = 1;
                while ((*itcc).second.size() > 0 && dmx <= 512)
                {
                    for (auto itms = itcc->second.begin(); itms != itcc->second.end(); ++itms)
                    {
                        if (((*itms)->GetModelChain() == "Beginning" || (*itms)->GetModelChain() == "") && (*itms)->GetControllerDMXChannel() == dmx)
                        {
                            sortedmodels.push_back(*itms);
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        }
                        else if (last != "" && ((*itms)->GetModelChain() == last ||
                            (*itms)->GetModelChain() == ">" + last))
                        {
                            sortedmodels.push_back(*itms);
                            last = (*itms)->GetName();
                            itcc->second.erase(itms);
                            break;
                        }
                    }
                    dmx++;
                }

                if ((*itcc).second.size() > 0)
                {
                    // models left over so stuff them on the end
                    logger_zcpp.error("    DMX Model chain is broken or there are duplicate models so just stuffing the remaining %d models in in their original order.", (*itcc).second.size());
                    while ((*itcc).second.size() > 0)
                    {
                        sortedmodels.push_back(itcc->second.front());
                        itcc->second.pop_front();
                    }
                }
            }

            for (auto itm : sortedmodels)
            {
                std::string sc = "";
                if (itm->IsPixelProtocol())
                {
                    if (itm->GetModelChain() == last ||
                        itm->GetModelChain() == ">" + last ||
                        ((itm->GetModelChain() == "Beginning" || itm->GetModelChain() == "") && last == ""))
                    {
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", ch);
                        itm->SetStartChannel(sc);
                        last = itm->GetName();
                        ch += itm->GetChanCount();
                        if (osc != itm->ModelStartChannel)
                        {
                            outputsChanged = true;
                        }
                    }
                    else
                    {
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", chstart);
                        itm->SetStartChannel(sc);
                        last = itm->GetName();
                        ch = std::max(ch, (int32_t)(chstart + itm->GetChanCount()));
                        if (osc != itm->ModelStartChannel)
                        {
                            outputsChanged = true;
                        }
                    }
                }
                else
                {
                    // when chained the use next channel
                    if (last != "" &&
                        (itm->GetModelChain() == last ||
                            itm->GetModelChain() == ">" + last))
                    {
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", ch);
                        itm->SetStartChannel(sc);
                        last = itm->GetName();
                        ch += itm->GetChanCount();
                        if (osc != itm->ModelStartChannel)
                        {
                            outputsChanged = true;
                        }
                    }
                    else
                    {
                        // when not chained use dmx channel
                        uint32_t msc = chstart + itm->GetControllerDMXChannel() - 1;
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetName() + ":" + wxString::Format("%d", msc);
                        itm->SetStartChannel(sc);
                        last = itm->GetName();
                        ch = std::max(ch, (int32_t)msc + (int32_t)itm->GetChanCount());
                        if (osc != itm->ModelStartChannel)
                        {
                            outputsChanged = true;
                        }
                    }
                }

                logger_zcpp.debug("    Model %s on port %d chained to %s start channel %s.",
                    (const char*)itm->GetName().c_str(),
                    itm->GetControllerPort(),
                    (const char*)itm->GetModelChain().c_str(),
                    (const char*)sc.c_str());
            }
        }

        if (it->IsAutoSize())
        {
            if (it->GetChannels() != std::max((int32_t)1, (int32_t)ch - 1))
            {
                logger_zcpp.debug("    Resizing output to %d channels.", std::max((int32_t)1, (int32_t)ch - 1));                
                
                it->SetChannelSize(std::max((int32_t)1, (int32_t)ch - 1));
                outputManager->SomethingChanged();

                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ReworkStartChannel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ReworkStartChannel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ReworkStartChannel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SAVE_NETWORKS, "ReworkStartChannel");
                xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "ReworkStartChannel");
            }
        }
    }
    return outputsChanged;
}

bool ModelManager::LoadGroups(wxXmlNode* groupNode, int previewW, int previewH) {
    this->groupNode = groupNode;
    bool changed = false;

    std::list<wxXmlNode*> toBeDone;
    std::list<std::string> allModels;
    std::lock_guard<std::recursive_mutex> lock(_modelMutex);

    // do all the models without embedded groups first or where the model order means everything exists
    for (wxXmlNode* e = groupNode->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                allModels.push_back(name);
                if (ModelGroup::AllModelsExist(e, *this))
                {
                    ModelGroup* model = new ModelGroup(e, *this, previewW, previewH);
                    bool reset = model->Reset();
                    wxASSERT(reset);
                    models[model->name] = model;
                    model->SetLayoutGroup(e->GetAttribute("LayoutGroup", "Unassigned").ToStdString());
                }
                else
                {
                    toBeDone.push_back(e);
                }
            }
        }
    }

    // add in models and submodels
    for (const auto& it : models)
    {
        allModels.push_back(it.second->GetName());
        for (auto it2 : it.second->GetSubModels())
        {
            allModels.push_back(it2->GetFullName());
        }
    }

    // remove any totally non existent models from model groups
    // this stops some end conditions which cant be resolved
    for (const auto& it : toBeDone)
    {
        changed |= ModelGroup::RemoveNonExistentModels(it, allModels);
    }

    // try up to however many models we have left
    int maxIter = toBeDone.size();
    while (maxIter > 0 && toBeDone.size() > 0)
    {
        maxIter--;
        std::list<wxXmlNode*> processing(toBeDone);
        toBeDone.clear();
        for (const auto& it : processing) {
            if (ModelGroup::AllModelsExist(it, *this))
            {
                ModelGroup* model = new ModelGroup(it, *this, previewW, previewH);
                bool reset = model->Reset();
                wxASSERT(reset);
                models[model->name] = model;
                model->SetLayoutGroup(it->GetAttribute("LayoutGroup", "Unassigned").ToStdString());
            }
            else
            {
                toBeDone.push_back(it);
            }
        }
    }

    // anything left in toBeDone is now due to model loops
    for (const auto& it : toBeDone) {
        std::string name = it->GetAttribute("name").ToStdString();
        std::string msg = "Could not process model group " + name
            + " likely due to model groups loops. See Check Sequence for details.";
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

    for (auto& it : *this)
    {
        changed |= it.second->RenameController(oldName, newName);
    }

    return changed;
}

// generate the next similar model name to the candidateName we are given
std::string ModelManager::GenerateModelName(const std::string& candidateName) const
{
    return GenerateObjectName(candidateName);
}

Model* ModelManager::CreateDefaultModel(const std::string &type, const std::string &startChannel) const {
    Model *model;
    wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "model");
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
            node->AddAttribute("NumServos", "3");
            node->AddAttribute("NumStatic", "1");
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

    return model;
}

Model *ModelManager::CreateModel(wxXmlNode *node, int previewW, int previewH, bool zeroBased ) const {

    if (node->GetName() == "modelGroup") {
        ModelGroup *grp = new ModelGroup(node, *this, previewWidth, previewHeight);
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
        }
        else if (style == "Moving Head 3D") {
            type = "DmxMovingHead3D";
        }
        else if (style == "Flood Light") {
            type = "DmxFloodlight";
        }
        else if (style == "Skulltronix Skull") {
            type = "DmxSkulltronix";
        }
        else {
            type = "DmxMovingHead";
        }
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", type);
    }
    else if (type == "DmxMovingHead3D") {
        std::string version = node->GetAttribute("versionNumber").ToStdString();
        // After version 2020.3 the DmxMovingHead3D is being moved back to the DmxMovingHead class so the 3D version is mesh only
        if (version == "4") {
            type = "DmxMovingHead";
            node->DeleteAttribute("DisplayAs");
            node->AddAttribute("DisplayAs", type);
            node->DeleteAttribute("DmxStyle");
            node->AddAttribute("DmxStyle", "Moving Head 3D");
        }
    }
    else if (type == "DmxServo3Axis") {
        type = "DmxServo3d";
        node->DeleteAttribute("DisplayAs");
        node->AddAttribute("DisplayAs", type);
        node->AddAttribute("NumServos", "3");
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "6");
    }

    Model *model;
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
    if (model->GetModelScreenLocation().CheckUpgrade(node) == UPGRADE_EXEC_READ) {
        model->GetModelScreenLocation().Read(node);
    }
    return model;
}

void ModelManager::AddModel(Model *model) {

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

Model *ModelManager::createAndAddModel(wxXmlNode *node, int previewW, int previewH) {
    Model *model = CreateModel(node, previewW, previewH);
    AddModel(model);
    return model;
}

std::string ModelManager::GetModelsOnChannels(uint32_t start, uint32_t end, int perLine) const {

    std::string res;
    std::string line;

    for (const auto& it : *this) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            if (perLine > 0 && CountChar(line, ',') >= perLine - 1) {
                if (res != "") res += "\n";
                res += line;
                line = "";
            }
            if (it.second->GetFirstChannel()+1 <= end && it.second->GetLastChannel() + 1 >= start) {
                if (line != "") line += ", ";
                line += it.first;
            }
        }
    }

    if (line != "") {
        if (res != "") res += "\n";
        res += line;
    }

	return res;
}

void ModelManager::Delete(const std::string &name) {

    if( xlights->CurrentSeqXmlFile != nullptr )
    {
        Element* elem_to_delete = xlights->GetSequenceElements().GetElement(name);
        if (elem_to_delete != nullptr)
        {
            // does model have any effects on it
            bool effects_exist = false;
            for (size_t i = 0; i < elem_to_delete->GetEffectLayerCount() && !effects_exist; ++i)
            {
                auto layer = elem_to_delete->GetEffectLayer(i);
                if (layer->GetEffectCount() > 0)
                {
                    effects_exist = true;
                }
            }

            if (effects_exist)
            {
                if (wxMessageBox("Model '" + name + "' exists in the currently open sequence and has effects on it. Delete all effects and layers on this model?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) != wxYES) return;
            }

            // Delete the model from the sequencer grid and views
            xlights->GetSequenceElements().DeleteElement(name);
        }
    }

    // now delete the model
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->first == name) {
            Model *model = it->second;

            if (model != nullptr) {
                model->GetModelXml()->GetParent()->RemoveChild(model->GetModelXml());

                for (auto& it2 : models) {
                    if (it2.second->GetDisplayAs() == "ModelGroup") {
                        ModelGroup *group = (ModelGroup*)it2.second;
                        group->ModelRemoved(name);
                    }
                }
                models.erase(it);
                ResetModelGroups();

                // If models are chained to us then make their start channel ... our start channel
                std::string chainedtous = wxString::Format(">%s:1", model->GetName()).ToStdString();
                for (auto it3: models)
                {
                    if (it3.second->ModelStartChannel == chainedtous)
                    {
                        it3.second->SetStartChannel(model->ModelStartChannel);
                    }
                }

                delete model->GetModelXml();
                delete model;
                return;
            }
        }
    }
}

std::map<std::string, Model*>::const_iterator ModelManager::begin() const {
    return models.begin();
}
std::map<std::string, Model*>::const_iterator ModelManager::end() const {
    return models.end();
}
unsigned int ModelManager::size() const {
    return models.size();
}
