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
        ModelManager(NetInfoClass &ni);
        virtual ~ModelManager();

        NetInfoClass &GetNetInfo() const {
            return netInfo;
        }
        Model *operator[](const std::string &name) const;
        Model *GetModel(const std::string &name) const;

        void RecalcStartChannels() const;
        unsigned int GetLastChannel() const;

        bool Rename(const std::string &oldName, const std::string &newName);
        void AddModel(Model *m);
        void Delete(const std::string &name);

        void LoadModels(wxXmlNode *modelNode, int previewW, int previewH);
        void LoadGroups(wxXmlNode *groupNode, int previewW, int previewH);

        void SetLayoutsNode(wxXmlNode* layouts) {layoutsNode = layouts;}
        wxXmlNode* GetLayoutsNode() const {return layoutsNode;}

        void clear();

        std::map<std::string, Model*>::const_iterator begin() const;
        std::map<std::string, Model*>::const_iterator end() const;
        unsigned int size() const;

        //Make sure the Model is deleted when done with
        Model *CreateModel(wxXmlNode *node, bool zeroBased = false) const;
        Model *CreateDefaultModel(const std::string &type, const std::string &startChannel = "1") const;
    protected:
        Model *createAndAddModel(wxXmlNode *node);
    private:

    wxXmlNode *modelNode;
    wxXmlNode *groupNode;
    wxXmlNode *layoutsNode;
    NetInfoClass &netInfo;
    int previewWidth;
    int previewHeight;
    std::map<std::string, Model *> models;
};

#endif // MODELMANAGER_H
