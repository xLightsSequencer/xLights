#include "ModelManager.h"
#include "Model.h"

#include <wx/xml/xml.h>
#include <wx/arrstr.h>
#include <wx/msgdlg.h>

#include "StarModel.h"
#include "ArchesModel.h"
#include "CandyCaneModel.h"
#include "CircleModel.h"
#include "TreeModel.h"
#include "CustomModel.h"
#include "WholeHouseModel.h"
#include "SingleLineModel.h"
#include "ModelGroup.h"
#include "WindowFrameModel.h"
#include "WreathModel.h"
#include "SphereModel.h"
#include "SpinnerModel.h"
#include "IciclesModel.h"

ModelManager::ModelManager(NetInfoClass &ni) : netInfo(ni)
{
    //ctor
}

ModelManager::~ModelManager()
{
    clear();
}

void ModelManager::clear() {
    for (auto it = models.begin(); it != models.end(); it++) {
        delete it->second;
    }
    models.clear();
}

Model *ModelManager::GetModel(const std::string &name) const {
    auto it = models.find(name);
    if (it == models.end()) {
        return nullptr;
    }
    return it->second;
}
Model *ModelManager::operator[](const std::string &name) const {
    return GetModel(name);
}

bool ModelManager::Rename(const std::string &oldName, const std::string &newName) {
    for (auto it = models.begin(); it != models.end(); it++) {
        if (it->first == oldName) {
            Model *model = it->second;

            if (model != nullptr) {
                model->GetModelXml()->DeleteAttribute("name");
                model->GetModelXml()->AddAttribute("name",newName);
                model->name = newName;

                bool changed = false;
                for (auto it2 = models.begin(); it2 != models.end(); it2++) {
                    changed |= it2->second->ModelRenamed(oldName, newName);
                }
                models.erase(it);
                models[newName] = model;
                return changed;
            }
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
    for (wxXmlNode* e=modelNode->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "model") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                Model *m = createAndAddModel(e);
                if (m != nullptr) {
                    m->SetMinMaxModelScreenCoordinates(previewW, previewH);
                }
                countValid += m->CouldComputeStartChannel ? 1 : 0;
            }
        }
    }
    while (countValid != models.size()) {
        int newCountValid = 0;
        for (auto it = models.begin(); it != models.end(); it++) {
            if (!it->second->CouldComputeStartChannel) {
                it->second->SetFromXml(it->second->GetModelXml());
                newCountValid += it->second->CouldComputeStartChannel ? 1 : 0;
            } else {
                newCountValid++;
            }
        }
        if (countValid == newCountValid) {
            std::string msg = "Could not calculate start channels for models:\n";
            for (auto it = models.begin(); it != models.end(); it++) {
                if (!it->second->CouldComputeStartChannel) {
                    msg += it->second->name + "\n";
                }
            }
            wxMessageBox(msg);
            //nothing improved
            return;
        }
        countValid = newCountValid;
    }
}

unsigned int ModelManager::GetLastChannel() const {
    unsigned int max = 0;
    for (auto it = models.begin(); it != models.end(); it++) {
        max = std::max(max, it->second->GetLastChannel());
    }
    return max;
}

void ModelManager::RecalcStartChannels() const {
    int countValid = 0;
    for (auto it = models.begin(); it != models.end(); it++) {
        it->second->SetFromXml(it->second->GetModelXml());
        countValid += it->second->CouldComputeStartChannel ? 1 : 0;
    }
    while (countValid != models.size()) {
        int newCountValid = 0;
        for (auto it = models.begin(); it != models.end(); it++) {
            if (!it->second->CouldComputeStartChannel) {
                it->second->SetFromXml(it->second->GetModelXml());
                newCountValid += it->second->CouldComputeStartChannel ? 1 : 0;
            } else {
                newCountValid++;
            }
        }
        if (countValid == newCountValid) {
            std::string msg = "Could not calculate start channels for models:\n";
            for (auto it = models.begin(); it != models.end(); it++) {
                if (!it->second->CouldComputeStartChannel) {
                    msg += it->second->name + "\n";
                }
            }
            wxMessageBox(msg);
            //nothing improved
            return;
        }
    }
}


void ModelManager::LoadGroups(wxXmlNode *groupNode, int previewW, int previewH) {
    this->groupNode = groupNode;
    for (wxXmlNode* e=groupNode->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                ModelGroup *model = new ModelGroup(e, *this, previewW, previewH);
                auto it = models.find(model->name);
                if (it != models.end()) {
                    delete it->second;
                }
                model->SetLayoutGroup( e->GetAttribute("LayoutGroup", "Unassigned").ToStdString() );
                models[model->name] = model;
            }
        }
    }
}

Model *ModelManager::CreateDefaultModel(const std::string &type, const std::string &startChannel) const {
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

    int cnt = 0;
    std::string name = type;
    while (GetModel(name) != nullptr) {
        cnt++;
        name = type + "-" + std::to_string(cnt);
    }
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
    } else if (type == "Circle") {
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "50");
        model = new CircleModel(node, *this, false);
    } else if (type == "Window Frame") {
        node->DeleteAttribute("parm1");
        node->AddAttribute("parm1", "16");
        node->DeleteAttribute("parm3");
        node->AddAttribute("parm3", "16");
        model = new WindowFrameModel(node, *this, false);
    } else if (type == "Wreath") {
        model = new WreathModel(node, *this, false);
    } else if (type.find("Sphere") == 0) {
        model = new SphereModel(node, *this, false);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, *this, false);
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
        return new ModelGroup(node, *this, previewWidth, previewHeight);
    }
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
    Model *model;
    if (type == "Star") {
        model = new StarModel(node, *this, zeroBased);
    } else if (type == "Arches") {
        model = new ArchesModel(node, *this, zeroBased);
	} else if (type == "Candy Canes") {
		model = new CandyCaneModel(node, *this, zeroBased);
	} else if (type == "Circle") {
        model = new CircleModel(node, *this, zeroBased);
    } else if (type == "Window Frame") {
        model = new WindowFrameModel(node, *this, zeroBased);
    } else if (type == "Wreath") {
        model = new WreathModel(node, *this, zeroBased);
    } else if (type.find("Sphere") == 0) {
        model = new SphereModel(node, *this, zeroBased);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, *this, zeroBased);
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
    for (auto it = models.begin(); it != models.end(); it++) {
        if (it->first == name) {
            Model *model = it->second;

            if (model != nullptr) {
                model->GetModelXml()->GetParent()->RemoveChild(model->GetModelXml());

                for (auto it = models.begin(); it != models.end(); it++) {
                    if (it->second->GetDisplayAs() == "ModelGroup") {
                        ModelGroup *group = (ModelGroup*)it->second;
                        group->ModelRemoved(name);
                    }
                }
                models.erase(it);
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
