#ifndef MODELGROUP_H
#define MODELGROUP_H

#include <vector>
#include <string>

#include "Model.h"
#include "WholeHouseModel.h"

class ModelManager;

class ModelGroup : public WholeHouseModel
{
    public:
        ModelGroup(wxXmlNode *node, NetInfoClass &netInfo, ModelManager &manager, int previewW, int previewH);
        virtual ~ModelGroup();

    
        bool IsSelected() const { return selected;}
        const std::vector<std::string> &ModelNames() const { return modelNames;}
    
    
        virtual const std::vector<std::string> &GetBufferStyles() const override { return GROUP_BUFFER_STYLES; };
        virtual void GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
    protected:
        static std::vector<std::string> GROUP_BUFFER_STYLES;

    private:
        wxXmlNode* CreateModelNodeFromGroup(int previewW, int previewH, wxXmlNode *e);
        wxXmlNode* BuildWholeHouseModel(int previewW, int previewH,
                                        const std::string &modelName,
                                        const wxXmlNode *node,
                                        std::vector<Model*> &models);

        std::vector<std::string> modelNames;
        bool selected;
        const ModelManager &manager;
};

#endif // MODELGROUP_H
