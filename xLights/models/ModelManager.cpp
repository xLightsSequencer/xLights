#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>

#include <cctype>

#include "ModelManager.h"
#include "Model.h"
#include "SubModel.h"
#include "StarModel.h"
#include "ArchesModel.h"
#include "CandyCaneModel.h"
#include "ChannelBlockModel.h"
#include "CircleModel.h"
#include "TreeModel.h"
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

#include <log4cpp/Category.hh>
#include "outputs/Output.h"

ModelManager::ModelManager(OutputManager* outputManager, xLightsFrame* xl) :
    _outputManager(outputManager),
    xlights(xl),
    modelNode(nullptr),
    groupNode(nullptr),
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
        for (auto it2 = models.begin(); it2 != models.end(); ++it2) {
            changed |= it2->second->ModelRenamed(oldName, newName);
        }
        models.erase(models.find(oldName));
        models[newName] = model;

        // go through all the model groups looking for things that might need to be renamed
        for (auto it = models.begin(); it != models.end(); ++it) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it->second);
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

    for (auto m = begin(); m != end(); ++m)
    {
        if (m->second->GetDisplayAs() == "ModelGroup")
        {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(m->second);
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
    long start = model->GetNumberFromChannelString(model->ModelStartChannel);
    long end = start + model->GetChanCount() - 1;
    //long sstart = model->GetFirstChannel() + 1;
    //wxASSERT(sstart == start);
    //long send = model->GetLastChannel() + 1;
    //wxASSERT(send == end);
    for (auto it = this->begin(); it != this->end(); ++it)
    {
        if (it->second->GetDisplayAs() != "ModelGroup" && it->second->GetName() != model->GetName())
        {
            long s = it->second->GetNumberFromChannelString(it->second->ModelStartChannel);
            long e = s + it->second->GetChanCount() - 1;
            //long ss = it->second->GetFirstChannel() + 1;
            //wxASSERT(ss == s);
            //long se = it->second->GetLastChannel() + 1;
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
                Model *m = createAndAddModel(e);
                if (m != nullptr) {
                    m->SetMinMaxModelScreenCoordinates(previewW, previewH);
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

unsigned int ModelManager::GetLastChannel() const {
    unsigned int max = 0;
    for (auto it = models.begin(); it != models.end(); ++it) {
        max = std::max(max, it->second->GetLastChannel());
    }
    return max;
}

void ModelManager::NewRecalcStartChannels() const
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //wxStopWatch sw;
    //sw.Start();

    // this is ugly but it solves some const-ness issues
    std::map<std::string, Model*> ms(models);
    std::string msg = "Could not calculate start channels for models:\n";
    bool failed = false;

    for (auto it = models.begin(); it != models.end(); ++it) {
        it->second->CouldComputeStartChannel = false;
    }

    for (auto it = models.begin(); it != models.end(); ++it)
    {
        if (it->second->GetDisplayAs() != "ModelGroup")
        {
            std::list<std::string> used;
            if (!it->second->UpdateStartChannelFromChannelString(ms, used)) {
                msg += it->second->name + "\n";
                failed = true;
            }
        }
    }
    if (failed) {
        wxMessageBox(msg);
    }

    //long end  = sw.Time();
    //logger_base.debug("New RecalcStartChannels takes %ld.", end);
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

void ModelManager::OldRecalcStartChannels() const {
    //wxStopWatch sw;
    //sw.Start();
    int countValid = 0;
    for (auto it = models.begin(); it != models.end(); ++it) {
        it->second->CouldComputeStartChannel = false;
    }
    for (auto it = models.begin(); it != models.end(); ++it) {
        if( it->second->GetDisplayAs() != "ModelGroup" ) {
            it->second->SetFromXml(it->second->GetModelXml());
            countValid += it->second->CouldComputeStartChannel ? 1 : 0;
        } else {
            countValid++;
        }
    }
    while (countValid != models.size()) {
        int newCountValid = 0;
        for (auto it = models.begin(); it != models.end(); ++it) {
            if( it->second->GetDisplayAs() != "ModelGroup" ) {
                if (!it->second->CouldComputeStartChannel) {
                    it->second->SetFromXml(it->second->GetModelXml());
                    newCountValid += it->second->CouldComputeStartChannel ? 1 : 0;
                } else {
                    newCountValid++;
                }
            } else {
                newCountValid++;
            }
        }
        if (countValid == newCountValid) {
            DisplayStartChannelCalcWarning();

            //nothing improved

            ResetModelGroups();

            return;
        }
        countValid = newCountValid;
    }

    ResetModelGroups();

    //long end = sw.Time();
    //logger_base.debug("Old RecalcStartChannels takes %ld.", end);
}

void ModelManager::DisplayStartChannelCalcWarning() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static std::string lastwarn = "";
    std::string msg = "Could not calculate start channels for models:\n";
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->second->GetDisplayAs() != "ModelGroup" && !it->second->CouldComputeStartChannel) {
            msg += it->second->name + " : " + it->second->ModelStartChannel + "\n";
        }
    }

    if (msg != lastwarn)
    {
        logger_base.warn(msg);
        wxMessageBox(msg);
        lastwarn = msg;
    }
}

void ModelManager::ReworkStartChannel() const
{
    bool  outputsChanged = false;

    OutputManager* outputManager = xlights->GetOutputManager();
    auto outputs = outputManager->GetOutputs();

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->IsLookedUpByControllerName())
        {
            std::map<std::string, std::list<Model*>> cmodels;
            for (auto itm = models.begin(); itm != models.end(); ++itm)
            {
                if (itm->second->GetControllerName() == (*it)->GetDescription())
                {
                    wxString cc = wxString(itm->second->GetControllerConnectionString()).Lower();
                    if (cmodels.find(cc) == cmodels.end())
                    {
                        std::list<Model*> ml;
                        cmodels[cc] = ml;
                    }
                    cmodels[cc].push_back(itm->second);
                }
            }

            long ch = 1;
            for (auto itcc = cmodels.begin(); itcc != cmodels.end(); ++itcc)
            {
                // order the models
                std::list<Model*> sortedmodels;
                std::string last = "";

                long chstart = ch;

                while ((*itcc).second.size() > 0)
                {
                    bool pushed = false;
                    for (auto itms = (*itcc).second.begin(); itms != (*itcc).second.end(); ++itms)
                    {
                        if (((*itms)->GetModelChain() == "Beginning" && last == "") ||
                            (*itms)->GetModelChain() == last || 
                            (*itms)->GetModelChain() == ">" + last)
                        {
                            sortedmodels.push_back(*itms);
                            pushed = true;
                            last = (*itms)->GetName();
                            (*itcc).second.erase(itms);
                            break;
                        }
                    }

                    if (!pushed && (*itcc).second.size() > 0)
                    {
                        // chain is broken ... so just put the rest in in random order
                        while ((*itcc).second.size() > 0)
                        {
                            sortedmodels.push_back(itcc->second.front());
                            itcc->second.pop_front();
                        }
                    }
                }

                last = "";
                for (auto itm = sortedmodels.begin(); itm != sortedmodels.end(); ++itm)
                {
                    if ((*itm)->GetModelChain() == last || (*itm)->GetModelChain() == ">" + last || ((*itm)->GetModelChain() == "Beginning" && last == ""))
                    {
                        (*itm)->SetStartChannel("!" + (*it)->GetDescription() + ":" + wxString::Format("%ld", ch));
                        last = (*itm)->GetName();
                    }
                    else
                    {
                        (*itm)->SetStartChannel("!" + (*it)->GetDescription() + ":" + wxString::Format("%ld", chstart));
                    }
                    ch += (*itm)->GetChanCount();
                }
            }

            if ((*it)->GetAutoSize())
            {
                if ((*it)->GetChannels() != std::max((long)1, (long)ch - 1))
                {
                    (*it)->SetChannels(std::max((long)1, (long)ch - 1));
                    outputsChanged = true;
                }
            }
        }
    }

    if (outputsChanged)
    {
        xlights->GetOutputManager()->SomethingChanged();
        xlights->UpdateNetworkList(false);
        xlights->NetworkChange();
        xlights->SaveNetworksFile();
    }
}

bool ModelManager::LoadGroups(wxXmlNode *groupNode, int previewW, int previewH) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    this->groupNode = groupNode;
    std::list<ModelGroup*> toBeDone;
    bool changed = false;

    for (wxXmlNode* e=groupNode->GetChildren(); e!=nullptr; e=e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                ModelGroup *model = new ModelGroup(e, *this, previewW, previewH);
                if (model->Reset()) {
                    auto it = models.find(model->name);
                    bool reset = false;
                    if (it != models.end()) {
                        delete it->second;
                        it->second = nullptr;
                        reset = true;
                    }
                    models[model->name] = model;
                    if (reset) ResetModelGroups();
                    model->SetLayoutGroup( e->GetAttribute("LayoutGroup", "Unassigned").ToStdString() );
                } else {
                    model->SetLayoutGroup( e->GetAttribute("LayoutGroup", "Unassigned").ToStdString() );
                    toBeDone.push_back(model);
                }
            }
        }
    }
    int maxIter = toBeDone.size();
    while (!toBeDone.empty()) {
        if (maxIter == 0) {
            std::string msg = "Could not process model groups (possibly due to circular groups or a missing model):";
            for (auto it = toBeDone.begin(); it != toBeDone.end(); ++it) {
                ModelGroup *g = *it;

                wxString orig = g->GetModelXml()->GetAttribute("models");
                wxString newM;
                wxArrayString mn = wxSplit(orig, ',');
                std::string modelsRemoved;
                for (auto it2 = mn.begin(); it2 != mn.end(); ++it2) {

                    wxString submodel = "";
                    wxString name = *it2;
                    if (name.Contains("/"))
                    {
                        submodel = name.After('/');
                        name = name.Before('/');
                    }

                    auto m = models.find(name.ToStdString());

                    if (m != models.end() && submodel != "")
                    {
                        if (m->second->GetSubModel(submodel) == nullptr)
                        {
                            m = models.end();
                        }
                    }

                    if (*it2 == "")
                    {
                        // This can happen if a comma is left behind in the string ... just ignore it
                    }
                    else if (m == models.end()) {
                        modelsRemoved += "\n       " + *it2 + " not found";
                    } else {
                        if (newM != "") {
                            newM += ",";
                        }
                        newM += *it2;
                    }
                }
                g->GetModelXml()->DeleteAttribute("models");
                g->GetModelXml()->AddAttribute("models", newM);
                if (g->Reset()) {
                    // Model successfully loaded
                    if (modelsRemoved > "")
                    {
                        msg = "Could not process model group " + g->GetName()
                            + " due to models not being found.  The following models will be removed from the group:"
                            + modelsRemoved;
                        logger_base.warn("ModelManager::LoadGroups %s", (const char *)msg.c_str());
                        wxMessageBox(msg);
                    }
                    maxIter = toBeDone.size();
                    changed = true;
                } else {
                    // Load of the new group failed so reset it
                    msg += "\n" + g->GetName();
                    msg += modelsRemoved;
                    g->GetModelXml()->DeleteAttribute("models");
                    g->GetModelXml()->AddAttribute("models", orig);
                }
            }
            if (maxIter == 0) {
                // I would prefer NOT to send users to the log but the model group does not get loaded so when we run check sequence you can't see the error.
                msg += "\n\nCheck log file for more details.";
                logger_base.warn(msg);
                wxMessageBox(msg);
                //nothing improved
                break;
            }
        }
        maxIter--;
        std::list<ModelGroup*> processing(toBeDone);
        toBeDone.clear();
        for (auto it = processing.begin(); it != processing.end(); ++it) {
            ModelGroup *model = *it;
            if (model->Reset()) {
                auto it2 = models.find(model->name);
                if (it2 != models.end()) {
                    delete it2->second;
                    ResetModelGroups();
                }
                models[model->name] = model;
            } else {
                toBeDone.push_back(model);
            }
        }
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
    // if it is already unique return it
    if (GetModel(candidateName) == nullptr) return candidateName;

    std::string base = candidateName;
    char sep = '-';

    static wxRegEx dashRegex("-[0-9]+$", wxRE_ADVANCED);
    static wxRegEx underscoreRegex("_[0-9]+$", wxRE_ADVANCED);
    static wxRegEx spaceRegex(" [0-9]+$", wxRE_ADVANCED);
    static wxRegEx nilRegex("[A-Za-z][0-9]+$", wxRE_ADVANCED);
    if (dashRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast('-');
    }
    else if (underscoreRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast('_');
        sep = '_';
    }
    else if (spaceRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast(' ');
        sep = ' ';
    }
    else if (nilRegex.Matches(candidateName))
    {
        while (base != "" && std::isdigit(base[base.size() - 1]))
        {
            base = base.substr(0, base.size() - 1);
        }
        sep = 'x';
    }

    // We start at 2 assuming if we are adding multiple then the user will typically rename the first one number one.
    int seq = 2;

    for (;;)
    {
        std::string tryName = base;

        if (sep == 'x')
        {
            tryName += std::to_string(seq++);
        }
        else
        {
            tryName += sep + std::to_string(seq++);
        }

        if (GetModel(tryName) == nullptr) return tryName;
    }
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
        model = new SphereModel(node, *this, false);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, *this, false);
    } else if (type == "Poly Line") {
        model = new PolyLineModel(node, *this, false);
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
        wxMessageBox(type + " is not a valid model type for model " + node->GetAttribute("name"));
        return nullptr;
    }
    if (model != nullptr) {
        model->SetMinMaxModelScreenCoordinates(previewWidth, previewHeight);
    }
    return model;
}

Model *ModelManager::CreateModel(wxXmlNode *node, bool zeroBased) const {
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
        wxMessageBox(type + " is not a valid model type for model " + node->GetAttribute("name"));
        return nullptr;
    }
    if (model != nullptr) {
        model->SetMinMaxModelScreenCoordinates(previewWidth, previewHeight);
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

Model *ModelManager::createAndAddModel(wxXmlNode *node) {
    Model *model = CreateModel(node);
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
                int result = wxMessageBox("Model '" + name + "' exists in the currently open sequence and has effects on it. Delete all effects and layers on this model?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO);
                if (result != wxYES) return;
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

                for (auto it2 = models.begin(); it2 != models.end(); ++it2) {
                    if (it2->second->GetDisplayAs() == "ModelGroup") {
                        ModelGroup *group = (ModelGroup*)it2->second;
                        group->ModelRemoved(name);
                    }
                }
                models.erase(it);

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
                ResetModelGroups();
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
