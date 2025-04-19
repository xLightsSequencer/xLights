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

#include "MatrixModel.h"

class TreeModel : public MatrixModel
{
    public:
        TreeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~TreeModel();
    
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual void ExportAsCustomXModel3D() const override;
        virtual bool SupportsExportAsCustom3D() const override
        {
            return true;
        }
        virtual bool SupportsWiringView() const override
        {
            return true;
        }
        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;
        virtual int NodeRenderOrder() override {return 1;}
        virtual bool SupportsLowDefinitionRender() const override
        {
            return false; // we need to override this as the matrix model can set it to true
        }

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
        virtual void InitModel() override;
    private:
        int treeType;
        long degrees;
        float rotation;
        float spiralRotations;
        float botTopRatio;
        float perspective;
        void SetTreeCoord(long degrees);
};
