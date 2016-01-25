#include "ModelManager.h"
#include "Model.h"

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

Model *ModelManager::operator[](const std::string &name) const {
    auto it = models.find(name);
    if (it == models.end()) {
        return nullptr;
    }
    return it->second;
}

Model *ModelManager::createModel(wxXmlNode *node, NetInfoClass &netInfo) {
    Model *model = new Model();
    model->SetFromXml(node, netInfo);
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
