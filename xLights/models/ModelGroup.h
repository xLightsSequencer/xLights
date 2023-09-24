#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>
#include <set>
#include <string>

#include "Model.h"

class ModelManager;

class ModelGroup : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:

        static bool AllModelsExist(wxXmlNode* node, const ModelManager& models);
        static bool RemoveNonExistentModels(wxXmlNode* node, const std::set<std::string>& allmodels);

        ModelGroup(wxXmlNode *node, const ModelManager &manager, int previewW, int previewH);
        ModelGroup(wxXmlNode* node, const ModelManager& m, int w, int h, const std::string& mgname, const std::string& mname);
        virtual ~ModelGroup();

        // void TestNodeInit() const; // This function should be uncommented for testing issues where model group buffer styles create different numbers of nodes
        virtual unsigned int GetFirstChannel() const override;
        virtual unsigned int GetLastChannel() const override;
        void ModelRemoved(const std::string &name);
        virtual bool ModelRenamed(const std::string &oldName, const std::string &newName) override;
        bool SubModelRenamed(const std::string &oldName, const std::string &newName);
        void AddModel(const std::string &name);
        int GetGridSize() const;
        int GetXCentreOffset() const;
        int GetYCentreOffset() const;
        std::string GetDefaultCamera() const;

        bool IsSelected() const { return selected;}
        const std::vector<std::string> &ModelNames() const { return modelNames;}
        const std::vector<Model *> &Models() const { return models;}
        const std::vector<Model *> &ActiveModels() const { return activeModels;}
        Model* GetModel(std::string modelName) const;
        Model* GetFirstModel() const;
        std::list<Model*> GetFlatModels(bool removeDuplicates = true, bool activeOnly = true) const;
        bool ContainsModelGroup(ModelGroup* mg);
        bool ContainsModelGroup(ModelGroup* mg, std::set<Model*>& visited);
        bool DirectlyContainsModel(Model* m) const;
        bool DirectlyContainsModel(std::string const& m) const;
        bool ContainsModel(Model* m) const;
        bool ContainsModel(Model* m, std::list<const Model*>& visited) const;
        bool ContainsModelOrSubmodel(Model* m) const;
        bool ContainsModelOrSubmodel(Model* m, std::list<const Model*>& visited) const;
        bool OnlyContainsModel(const std::string& name) const;
        int GetModelCount() const { return models.size(); }
        std::string SerialiseModelGroup(const std::string& forModel) const;
        bool RemoveDuplicates();
        bool IsModelFromBase(const std::string& modelName) const;

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep = false) const override;
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }

        virtual int GetNumStrands() const override { return 0;}

        bool Reset(bool zeroBased = false);
        void ResetModels();

        bool CheckForChanges() const;

    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:

        std::vector<std::string> modelNames;
        std::vector<Model *> models;
        std::vector<Model *> activeModels;
        bool selected;
        std::string defaultBufferStyle;
};

