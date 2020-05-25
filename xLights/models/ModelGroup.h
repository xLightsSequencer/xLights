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
#include <string>

#include "Model.h"

class ModelManager;

class ModelGroup : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:

        static bool AllModelsExist(wxXmlNode* node, const ModelManager& models);
        static bool RemoveNonExistentModels(wxXmlNode* node, const std::list<std::string>& allmodels);

        ModelGroup(wxXmlNode *node, const ModelManager &manager, int previewW, int previewH);
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

        bool IsSelected() const { return selected;}
        const std::vector<std::string> &ModelNames() const { return modelNames;}
        const std::vector<Model *> &Models() const { return models;}
        Model* GetModel(std::string modelName) const;
        Model* GetFirstModel() const;
        bool ContainsModelGroup(ModelGroup* mg);
        bool ContainsModelGroup(ModelGroup* mg, std::list<Model*>& visited);
        bool ContainsModel(Model* mg);
        bool ContainsModel(Model* mg, std::list<Model*>& visited);
        int GetModelCount() const { return models.size(); }

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }

        virtual int GetNumStrands() const override { return 0;}

        bool Reset(bool zeroBased = false);
        void ResetModels();
    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:
        bool CheckForChanges() const;

        std::vector<std::string> modelNames;
        std::vector<Model *> models;
        bool selected;
        std::string defaultBufferStyle;
};

