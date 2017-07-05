#ifndef MODELGROUP_H
#define MODELGROUP_H

#include <vector>
#include <string>

#include "Model.h"

class ModelManager;

class ModelGroup : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        ModelGroup(wxXmlNode *node, const ModelManager &manager, int previewW, int previewH);
        virtual ~ModelGroup();

        void ModelRemoved(const std::string &name);
        virtual bool ModelRenamed(const std::string &oldName, const std::string &newName) override;
        void AddModel(const std::string &name);

        bool IsSelected() const { return selected;}
        const std::vector<std::string> &ModelNames() const { return modelNames;}
        const std::vector<Model *> &Models() const { return models;}


        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
        virtual int GetNumStrands() const override { return 0;}

        bool Reset(bool zeroBased = false);
    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:
        void CheckForChanges() const;

        std::vector<std::string> modelNames;
        std::vector<Model *> models;
        bool selected;
        std::string defaultBufferStyle;
};

#endif // MODELGROUP_H
