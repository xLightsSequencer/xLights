#ifndef MODELGROUP_H
#define MODELGROUP_H

#include <vector>
#include <string>

#include "Model.h"

class ModelManager;

class ModelGroup : public Model
{
    public:
        ModelGroup(wxXmlNode *node, NetInfoClass &netInfo, ModelManager &manager, int previewW, int previewH);
        virtual ~ModelGroup();

    
        bool IsSelected() const { return selected;}
        const std::vector<std::string> &ModelNames() const { return modelNames;}
    
    
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:
        std::vector<std::string> modelNames;
        bool selected;
        std::string defaultBufferStyle;
        const ModelManager &manager;
};

#endif // MODELGROUP_H
