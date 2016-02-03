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
    protected:

    private:
        wxXmlNode* CreateModelNodeFromGroup(ModelManager &manager, int previewW, int previewH, wxXmlNode *e);
        wxXmlNode* BuildWholeHouseModel(ModelManager &manager, int previewW, int previewH,
                                        const std::string &modelName,
                                        const wxXmlNode *node,
                                        std::vector<Model*> &models);

        std::vector<std::string> modelNames;
        bool selected;
};

#endif // MODELGROUP_H
