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
        virtual bool SupportsExportAsCustom3D() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;
        virtual int NodeRenderOrder() override {return 1;}
        // we need to override this as the matrix model can set it to true
        virtual bool SupportsLowDefinitionRender() const override { return false; }
        int GetTreeType() const { return treeType; }
        float GetTreeDegrees() const { return degrees; }
        float GetTreeRotation() const { return rotation; }
        float GetSpiralRotations() const { return spiralRotations; }
        float GetBottomTopRatio() const { return botTopRatio; }
        float GetTreePerspective() const { return perspective; }
        std::string GetTreeDescription() const { return _displayAs; }

        virtual bool SupportsVisitors() const override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

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
        std::string _displayAs;
        void SetTreeCoord(long degrees);
};
