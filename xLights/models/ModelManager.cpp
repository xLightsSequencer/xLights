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
#include "DmxModel.h"
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

#include <log4cpp/Category.hh>

ModelManager::ModelManager(OutputManager* outputManager, xLightsFrame* xl) :
    _outputManager(outputManager),
    xlights(xl),
    layoutsNode(nullptr),
    previewWidth(0),
    previewHeight(0)
{
    //ctor
}

ModelManager::~ModelManager()
{
    clear();
}

void ModelManager::clear() {
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->second != nullptr)
        {
            delete it->second;
            it->second = nullptr;
        }
    }
    models.clear();
}

BaseObject *ModelManager::GetObject(const std::string &name) const
{
    return GetModel(name);
}

bool ModelManager::IsModelValid(Model* m) const
{
    for (auto& it : models)
    {
        if (it.second == m) return true;
    }
    return false;
}

Model *ModelManager::GetModel(const std::string &name) const {
    auto it = models.find(name);
    if (it == models.end()) {
        size_t pos = name.find("/");
        if (pos != std::string::npos) {
            std::string mname = name.substr(0, pos);
            std::string smname = name.substr(pos + 1);
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
    return GetModel(name);
}

bool ModelManager::Rename(const std::string &oldName, const std::string &newName) {
    Model *model = GetModel(oldName);
    if (model == nullptr || model->GetDisplayAs() == "SubModel") {
        return false;
    }
    model->GetModelXml()->DeleteAttribute("name");
    model->GetModelXml()->AddAttribute("name",newName);
    model->name = newName;
    if (dynamic_cast<SubModel*>(model) == nullptr) {
        bool changed = false;
        for (auto& it2 : models) {
            changed |= it2.second->ModelRenamed(oldName, newName);
        }
        models.erase(models.find(oldName));
        models[newName] = model;

        // go through all the model groups looking for things that might need to be renamed
        for (auto& it : models) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr)
            {
                changed |= mg->ModelRenamed(oldName, newName);
            }
        }

        return changed;
    }

    return false;
}

bool ModelManager::RenameSubModel(const std::string &oldName, const std::string &newName) {

    bool changed = false;

    for (auto& m : *this)
    {
        if (m.second->GetDisplayAs() == "ModelGroup")
        {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(m.second);
            changed |= mg->SubModelRenamed(oldName, newName);
        }
    }

    return changed;
}

bool ModelManager::RenameInListOnly(const std::string& oldName, const std::string& newName)
{
    Model *model = GetModel(oldName);
    if (model == nullptr) return false;
    models.erase(models.find(oldName));
    models[newName] = model;
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

void ModelManager::LoadModels(wxXmlNode *modelNode, int previewW, int previewH) {
    clear();
    previewWidth = previewW;
    previewHeight = previewH;
    this->modelNode = modelNode;
    int countValid = 0;
    for (wxXmlNode* e=modelNode->GetChildren(); e!=nullptr; e=e->GetNext()) {
        if (e->GetName() == "model") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                Model *m = createAndAddModel(e, previewW, previewH);
                if (m != nullptr) {
                    countValid += m->CouldComputeStartChannel ? 1 : 0;
                }
            }
        }
    }
    while (countValid != models.size()) {
        int newCountValid = 0;
        for (auto it = models.begin(); it != models.end(); ++it) {
            if (!it->second->CouldComputeStartChannel) {
                it->second->SetFromXml(it->second->GetModelXml());
                newCountValid += it->second->CouldComputeStartChannel ? 1 : 0;
            } else {
                newCountValid++;
            }
        }
        if (countValid == newCountValid) {
            DisplayStartChannelCalcWarning();

            //nothing improved
            return;
        }
        countValid = newCountValid;
    }
}

uint32_t ModelManager::GetLastChannel() const {
    unsigned int max = 0;
    for (auto it = models.begin(); it != models.end(); ++it) {
        max = std::max(max, it->second->GetLastChannel());
    }
    return max;
}

void ModelManager::ResetModelGroups() const
{
    // This goes through all the model groups which hold model pointers and ensure their model pointers are correct
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->second != nullptr && it->second->GetDisplayAs() == "ModelGroup") {
            ((ModelGroup*)(it->second))->ResetModels();
        }
    }
}

std::string ModelManager::GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel) const
{
    std::string last = "";
    unsigned int highestEndChannel = 0;

    for (auto it : models)
    {
        if (it.second->GetDisplayAs() != "ModelGroup" && 
            it.second->GetControllerName() == controllerName && 
            it.second->GetControllerPort() == port && 
            it.second->GetLastChannel() > highestEndChannel && it.first != excludeModel)
        {
            last = it.first;
            highestEndChannel = it.second->GetLastChannel();
        }
    }
    return last;
}

void ModelManager::ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP)
{
    for (auto& it : models)
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

    wxStopWatch sw;
    bool changed = false;
    std::list<std::string> modelsDone;

    for (auto& it : models) {
        it.second->CouldComputeStartChannel = false;
    }

    // first go through all models whose start channels are not dependent on other models
    for (auto& it : models) {
        if (it.second->GetDisplayAs() != "ModelGroup")
        {
            char first = '0';
            if (it.second->ModelStartChannel != "") first = it.second->ModelStartChannel[0];
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

        for (auto& it : models) {
            if (it.second->GetDisplayAs() != "ModelGroup")
            {
                char first = '0';
                if (it.second->ModelStartChannel != "") first = it.second->ModelStartChannel[0];
                if ((first == '>' || first == '@') && !it.second->CouldComputeStartChannel)
                {
                    std::string dependsOn = it.second->ModelStartChannel.substr(1, it.second->ModelStartChannel.find(':') - 1);
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
    for (auto& it : models) {
        if (it.second->GetDisplayAs() != "ModelGroup")
        {
            char first = '0';
            if (it.second->ModelStartChannel != "") first = it.second->ModelStartChannel[0];
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
            if (!it.second->CouldComputeStartChannel) countInvalid++;
        }
    }

    ResetModelGroups();

    long end = sw.Time();
    logger_base.debug("RecalcStartChannels takes %ld.", end);

    if (countInvalid > 0) {
        DisplayStartChannelCalcWarning();
    }

    return changed;
}

void ModelManager::DisplayStartChannelCalcWarning() const
{
    static std::string lastwarn = "";
    std::string msg = "Could not calculate start channels for models:\n";
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->second->GetDisplayAs() != "ModelGroup" && !it->second->CouldComputeStartChannel) {
            msg += it->second->name + " : " + it->second->ModelStartChannel + "\n";
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

    for (auto it : *this)
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
        for (auto it : sameOutput)
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
    static log4cpp::Category &logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        ReworkStartChannel.");

    bool  outputsChanged = false;

    OutputManager* outputManager = xlights->GetOutputManager();
    for (auto it : outputManager->GetOutputs())
    {
        if (it->IsAutoLayoutModels())
        {
            std::map<std::string, std::list<Model*>> cmodels;
            for (auto itm : models)
            {
                if (itm.second->GetControllerName() == it->GetDescription() && 
                    itm.second->GetControllerPort() != 0 &&
                    itm.second->GetControllerProtocol() != ""
                    ) // we dont muck with unassigned models or no protocol models
                {
                    wxString cc = wxString::Format("%s:%d:%02d", itm.second->GetControllerProtocol(), itm.second->GetSmartRemote(), itm.second->GetControllerPort()).Lower();
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
                logger_zcpp.debug("Fixing weirdness on %s - %s", (const char*)it->GetDescription().c_str(), (const char*)itcc->first.c_str());
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
                            logger_zcpp.debug("    Model %s set to beginning because the model it is chained to does not exist.", (const char*)itmm->GetName().c_str());
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

                last = "";
                for (auto itm : sortedmodels)
                {
                    std::string sc = "";
                    if (itm->GetModelChain() == last || 
                        itm->GetModelChain() == ">" + last || 
                        ((itm->GetModelChain() == "Beginning" || itm->GetModelChain() == "") && last == ""))
                    {
                        auto osc = itm->ModelStartChannel;
                        sc = "!" + it->GetDescription() + ":" + wxString::Format("%d", ch);
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
                        sc = "!" + it->GetDescription() + ":" + wxString::Format("%d", chstart);
                        itm->SetStartChannel(sc);
                        ch = std::max(ch, (int32_t)(chstart + itm->GetChanCount()));
                        if (osc != itm->ModelStartChannel)
                        {
                            outputsChanged = true;
                        }
                    }

                    logger_zcpp.debug("    Model %s on port %d chained to %s start channel %s.",
                        (const char*)itm->GetName().c_str(),
                        itm->GetControllerPort(),
                        (const char*)itm->GetModelChain().c_str(),
                        (const char*)sc.c_str());
                }
            }

            if (it->GetAutoSize())
            {
                if (it->GetChannels() != std::max((int32_t)1, (int32_t)ch - 1))
                {
                    logger_zcpp.debug("    Resizing output to %d channels.", std::max((int32_t)1, (int32_t)ch - 1));
                    if (it->GetType() == OUTPUT_ZCPP)
                    {
                        it->AllOff();
                        it->EndFrame(0);
                    }
                    it->SetChannels(std::max((int32_t)1, (int32_t)ch - 1));
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ReworkStartChannel", nullptr, it);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ReworkStartChannel", nullptr, it);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ReworkStartChannel", nullptr, it);
                    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SAVE_NETWORKS, "ReworkStartChannel", nullptr, it);
                }
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
    for (auto it : models)
    {
        allModels.push_back(it.second->GetName());
        for (auto it2 : it.second->GetSubModels())
        {
            allModels.push_back(it2->GetFullName());
        }
    }

    // remove any totally non existent models from model groups
    // this stops some end conditions which cant be resolved
    for (auto& it : toBeDone)
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
        for (auto it : processing) {
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
    for (auto it : toBeDone) {
        std::string msg = "Could not process model group " + it->GetName()
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

void ModelManager::RenameController(const std::string& oldName, const std::string& newName)
{
    for (auto it = begin(); it != end(); ++it)
    {
        if (it->second->GetControllerName() == oldName)
        {
            it->second->SetControllerName(newName);
        }
    }
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
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        model = new ChannelBlockModel(node, *this, false);
    } else if (type == "Circle") {
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "50");
        node->AddAttribute("InsideOut", "0");
        model = new CircleModel(node, *this, false);
    } else if (type == "DMX") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "8");
        node->DeleteAttribute("parm2");
        node->AddAttribute("parm2", "1");
        node->DeleteAttribute("DmxStyle");
        node->AddAttribute("DmxStyle", "Moving Head Top");
        node->DeleteAttribute("StringType");
        node->AddAttribute("StringType", "Single Color White");
        node->DeleteAttribute("DmxPanOrient");
        node->AddAttribute("DmxPanOrient", "0");
        node->DeleteAttribute("DmxPanDegOfRot");
        node->AddAttribute("DmxPanDegOfRot", "540");
        model = new DmxModel(node, *this, false);
    } else if (type == "Image") {
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
    return model;
}

Model *ModelManager::CreateModel(wxXmlNode *node, int previewW, int previewH, bool zeroBased ) const {

    if (node->GetName() == "modelGroup") {
        ModelGroup *grp = new ModelGroup(node, *this, previewWidth, previewHeight);
        grp->Reset(zeroBased);
        return grp;
    }
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
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
	} else if (type == "DMX") {
        model = new DmxModel(node, *this, zeroBased);
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
    if (model != nullptr) {
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
