#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <map>
#include <string>
#include <vector>

class Model;
class wxXmlNode;
class NetInfoClass;

class ModelManager
{
    public:
        ModelManager();
        virtual ~ModelManager();
    
    
        Model *operator[](const std::string &name) const;
        Model *GetModel(const std::string &name) const;
    
    
        void Load(wxXmlNode *modelNode, wxXmlNode *groupNode, NetInfoClass &ni,
                  int previewW, int previewH);
    
        void clear();
    
        std::map<std::string, Model*>::const_iterator begin() const;
        std::map<std::string, Model*>::const_iterator end() const;
        int size() const;
    
    protected:
        Model *createModel(wxXmlNode *node, NetInfoClass &ni);
    private:
    
    
    std::map<std::string, Model *> models;
};

#endif // MODELMANAGER_H
