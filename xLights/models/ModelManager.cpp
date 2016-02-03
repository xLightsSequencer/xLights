#include "ModelManager.h"
#include "Model.h"

#include <wx/xml/xml.h>
#include <wx/arrstr.h>


#include "StarModel.h"
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
                createModel(e, netInfo);
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

Model *ModelManager::createModel(wxXmlNode *node, NetInfoClass &netInfo) {
    std::string type = node->GetAttribute("DisplayAs").ToStdString();
    Model *model;
    if (type == "Star") {
        model = new StarModel(node, netInfo);
    } else {
        model = new Model();
        model->SetFromXml(node, netInfo);
    }
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
