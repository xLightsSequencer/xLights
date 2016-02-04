#include "ModelManager.h"
#include "Model.h"

#include <wx/xml/xml.h>
#include <wx/arrstr.h>


#include "StarModel.h"
#include "ArchesModel.h"
#include "TreeModel.h"
#include "CustomModel.h"
#include "WholeHouseModel.h"
#include "SingleLineModel.h"
#include "ModelGroup.h"

ModelManager::ModelManager()
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


void ModelManager::Load(wxXmlNode *modelNode, wxXmlNode *groupNode, NetInfoClass &netInfo, int previewW, int previewH) {
    clear();
    for (wxXmlNode* e=modelNode->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "model") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                createAndAddModel(e, netInfo);
            }
        }
    }
    for (wxXmlNode* e=groupNode->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "modelGroup") {
            std::string name = e->GetAttribute("name").ToStdString();
            if (!name.empty()) {
                ModelGroup *model = new ModelGroup(e, netInfo, *this, previewW, previewH);
                auto it = models.find(model->name);
                if (it != models.end()) {
                    delete it->second;
                }
                models[model->name] = model;
            }
        }
    }
}
Model *ModelManager::CreateModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased) {
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
    Model *model;
    if (type == "Star") {
        model = new StarModel(node, netInfo, zeroBased);
    } else if (type == "Arches") {
        model = new ArchesModel(node, netInfo, zeroBased);
    } else if (type == "Single Line") {
        model = new SingleLineModel(node, netInfo, zeroBased);
    } else if (type == "Custom") {
        model = new CustomModel(node, netInfo, zeroBased);
    } else if (type.find("Tree") == 0) {
        model = new TreeModel(node, netInfo, zeroBased);
    } else if (type == "WholeHouse") {
        model = new WholeHouseModel(node, netInfo, zeroBased);
    } else {
        model = new Model();
        model->SetFromXml(node, netInfo, zeroBased);
    }
    return model;
}
Model *ModelManager::createAndAddModel(wxXmlNode *node, const NetInfoClass &netInfo) {
    Model *model = CreateModel(node, netInfo);
    auto it = models.find(model->name);
    if (it != models.end()) {
        delete it->second;
    }
    models[model->name] = model;
    return model;
}




std::map<std::string, Model*>::const_iterator ModelManager::begin() const {
    return models.begin();
}
std::map<std::string, Model*>::const_iterator ModelManager::end() const {
    return models.end();
}
int ModelManager::size() const {
    return models.size();
}
