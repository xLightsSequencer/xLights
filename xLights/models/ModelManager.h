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
    
        void Rename(const std::string &oldName, const std::string &newName);
        void AddModel(Model *m);
        void Delete(const std::string &name);
    
        void LoadModels(wxXmlNode *modelNode, NetInfoClass &ni,
                        int previewW, int previewH);
        void LoadGroups(wxXmlNode *groupNode, NetInfoClass &ni,
                        int previewW, int previewH);
    
        void clear();
    
        std::map<std::string, Model*>::const_iterator begin() const;
        std::map<std::string, Model*>::const_iterator end() const;
        int size() const;
    
        //Make sure the Model is deleted when done with
        static Model *CreateModel(wxXmlNode *node, const NetInfoClass &ni, bool zeroBased = false);
        Model *CreateModel(wxXmlNode *node);
        Model *CreateDefaultModel(const std::string &type, const NetInfoClass &ni);
    protected:
        Model *createAndAddModel(wxXmlNode *node, const NetInfoClass &ni);
    private:
    
    wxXmlNode *modelNode;
    wxXmlNode *groupNode;
    NetInfoClass *netInfo;
    int previewWidth;
    int previewHeight;
    std::map<std::string, Model *> models;
};

#endif // MODELMANAGER_H
