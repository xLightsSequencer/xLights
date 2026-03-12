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

#include <vector>
#include <set>
#include <string>

#include "Model.h"
#include "Color.h"

class ModelManager;

class ModelGroup : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:

        static bool AllModelsExist(wxXmlNode* node, const ModelManager& models);
        static bool RemoveNonExistentModels(wxXmlNode* node, const std::set<std::string>& allmodels);

        ModelGroup(const ModelManager &manager);
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
        void SetXCentreOffset( float cx );
        void SetYCentreOffset( float cy );
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
        bool ContainsModel(const Model* m) const;
        bool ContainsModel(const Model* m, std::list<const Model*>& visited) const;
        bool ContainsModelOrSubmodel(const Model* m) const;
        bool ContainsModelOrSubmodel(const Model* m, std::list<const Model*>& visited) const;
        bool OnlyContainsModel(const std::string& name) const;
        int GetModelCount() const { return models.size(); }
        bool RemoveDuplicates();
        bool IsModelFromBase(const std::string& modelName) const;

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep = false) const override;
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }

        virtual int GetNumStrands() const override { return 0;}

        void Accept(BaseObjectVisitor& visitor) const override;

        void ResetModels();
        bool RebuildBuffers();

        bool CheckForChanges() const;

        float GetCentreX() const { return centrex; }
        float GetCentreY() const { return centrey; }
        bool GetCentreDefined() const { return centreDefined; }
        void SetCentreX( float cx );
        void SetCentreY( float cy );
        void SetCentreDefined( bool defined );
        void SetCentreMinx( int minx );
        void SetCentreMiny( int miny );
        void SetCentreMaxx( int maxx );
        void SetCentreMaxy( int maxy );
        int GetCentreMinx() const { return minx; }
        int GetCentreMiny() const { return miny; }
        int GetCentreMaxx() const { return maxx; }
        int GetCentreMaxy() const { return maxy; }
        std::string GetLayout() const { return m_layout; }
        void SetLayout(const std::string& layout);
        void SetGridSize(int size);
        void SetDefaultCamera(const std::string& camera);
        
        void SetName(const std::string& newName);
        void SetPreviewSize(int w, int h);
        void SetLayoutGroup(const std::string& group);
        void SetBaseModels(const std::vector<std::string>& baseModels);
        void SetModels(const std::vector<std::string>& models);
        
        bool InitializeFromMembers();

    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:
        int m_gridSize = 400;
        int m_xCentreOffset = 0;
        int m_yCentreOffset = 0;
        std::string m_defaultCamera = "2D";
        std::string m_layout = "minimalGrid";
        std::vector<std::string> m_baseModels;  // Models from base show
        
        std::vector<std::string> modelNames;
        std::vector<Model *> models;
        std::vector<Model *> activeModels;
        bool selected;
        std::string defaultBufferStyle;
        bool centreDefined = false;
        float centrex;
        float centrey;
        int minx = 0;
        int miny = 0;
        int maxx = 0;
        int maxy = 0;
};

