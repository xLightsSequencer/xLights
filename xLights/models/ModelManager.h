#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

#include "ObjectManager.h"

class Model;
class wxXmlNode;
class OutputManager;
class xLightsFrame;

class ModelManager : public ObjectManager
{
    public:
        ModelManager(OutputManager* outputManager, xLightsFrame* xl);
        ModelManager(const ModelManager& mm) = delete;
        ModelManager& operator=(const ModelManager& mm) = delete;
        ModelManager() = delete;

        virtual ~ModelManager();

        OutputManager* GetOutputManager() const { return _outputManager; }
        Model *operator[](const std::string &name) const;
        Model *GetModel(const std::string &name) const;
        bool IsModelValid(const Model* m) const;
        virtual BaseObject *GetObject(const std::string &name) const override;

        bool RecalcStartChannels() const;
        uint32_t GetLastChannel() const;
        void DisplayStartChannelCalcWarning() const;
        bool ReworkStartChannel() const;

        bool Rename(const std::string &oldName, const std::string &newName);
        bool RenameSubModel(const std::string &oldName, const std::string &newName);
        bool RenameInListOnly(const std::string &oldName, const std::string &newName);
        bool IsModelOverlapping(const Model* model) const;
        bool IsModelShadowing(const Model* m) const;
        std::list<std::string> GetModelsShadowing(const Model* m) const;
        void AddModel(Model* m);
        void ReplaceModel(const std::string &name, Model* nm);
        bool Delete(const std::string &name);
        std::string GenerateModelName(const std::string& candidateModelName) const;
        void ResetModelGroups() const;
        std::string GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel, const std::string& protocol) const;
        std::string GetLastModelOnPort(const std::string& controllerName, int port, const std::string& excludeModel, const std::string& protocol, int smartReceiver) const;
        void ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP);
        std::string SerialiseModelGroupsForModel(Model* m) const;
        void AddModelGroups(wxXmlNode* n, int w, int h, const std::string& name, bool& merge, bool& ask);
        void LoadModels(wxXmlNode *modelNode, int previewW, int previewH);
        bool LoadGroups(wxXmlNode *groupNode, int previewW, int previewH);
        bool ModelHasNoDependencyOnNoController(Model* m, std::list<std::string>& visited) const;

        bool RenameController(const std::string& oldName, const std::string& newName);

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
        bool IsValidControllerModelChain(Model* m, std::string& tip) const;
        Model *createAndAddModel(wxXmlNode *node, int previewW, int previewH);
        std::string GetModelsOnChannels(uint32_t start, uint32_t end, int perLine) const;
        std::vector<std::string> GetGroupsContainingModel(Model* model) const;
        std::vector<std::string> GetGroupsContainingModelOrSubmodel(Model* model) const;
        std::string GenerateNewStartChannel(const std::string& lastModel = "") const;

        int GetPreviewWidth() const { return previewWidth; }
        int GetPreviewHeight() const { return previewHeight; }
        bool MergeFromBase(const std::string& baseShowDir, bool prompt);
        std::string GetLastGeneratedModelName() const { return lastGeneratedModelName; }

    private:

    void MigrateDmxMotors(wxXmlNode *node) const;

    wxXmlNode *layoutsNode = nullptr;
    OutputManager* _outputManager = nullptr;
    xLightsFrame* xlights = nullptr;
    int previewWidth = 0;
    int previewHeight = 0;
    std::map<std::string, Model *> models;
    mutable std::recursive_mutex _modelMutex;
    std::atomic<bool> _modelsLoading;
    mutable std::string lastGeneratedModelName = "";
};

