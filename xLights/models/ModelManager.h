#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "ObjectManager.h"

class Model;
class wxXmlNode;
class OutputManager;
class xLightsFrame;

class ModelManager : public ObjectManager
{
    public:
        ModelManager(OutputManager* outputManager, xLightsFrame* xl);
        virtual ~ModelManager();

        OutputManager* GetOutputManager() const { return _outputManager; }
        Model *operator[](const std::string &name) const;
        Model *GetModel(const std::string &name) const;
        virtual BaseObject *GetObject(const std::string &name) const override;

        void OldRecalcStartChannels() const;
        void NewRecalcStartChannels() const;
        unsigned int GetLastChannel() const;
        void DisplayStartChannelCalcWarning() const;
        void ReworkStartChannel() const;

        bool Rename(const std::string &oldName, const std::string &newName);
        bool RenameSubModel(const std::string &oldName, const std::string &newName);
        bool RenameInListOnly(const std::string &oldName, const std::string &newName);
        bool IsModelOverlapping(Model* model) const;
        void AddModel(Model *m);
        void Delete(const std::string &name);
        std::string GenerateModelName(const std::string& candidateModelName) const;
        void ResetModelGroups() const;

        void LoadModels(wxXmlNode *modelNode, int previewW, int previewH);
        bool LoadGroups(wxXmlNode *groupNode, int previewW, int previewH);

        void RenameController(const std::string& oldName, const std::string& newName);

        void SetLayoutsNode(wxXmlNode* layouts) {layoutsNode = layouts;}
        wxXmlNode* GetLayoutsNode() const {return layoutsNode;}

        void clear();

        std::map<std::string, Model*>::const_iterator begin() const;
        std::map<std::string, Model*>::const_iterator end() const;
        unsigned int size() const;

        //Make sure the Model is deleted when done with
        Model *CreateModel(wxXmlNode *node, int previewW = 0, int previewH = 0, bool zeroBased = false) const;
        Model *CreateDefaultModel(const std::string &type, const std::string &startChannel = "1") const;
        xLightsFrame* GetXLightsFrame() const { return xlights; }
    protected:
        Model *createAndAddModel(wxXmlNode *node, int previewW, int previewH);
    private:

    wxXmlNode *layoutsNode;
    OutputManager* _outputManager;
    xLightsFrame* xlights;
    int previewWidth;
    int previewHeight;
    std::map<std::string, Model *> models;
};

#endif // MODELMANAGER_H
