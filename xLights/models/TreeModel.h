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
        TreeModel(const ModelManager &manager);
        virtual ~TreeModel();
    
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual void ExportAsCustomXModel3D() const override;
        virtual bool SupportsExportAsCustom3D() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual int NodeRenderOrder() override {return 1;}
        // we need to override this as the matrix model can set it to true
        virtual bool SupportsLowDefinitionRender() const override { return false; }
        int GetTreeType() const { return _treeType; }
        float GetTreeDegrees() const { return _degrees; }
        float GetTreeRotation() const { return _rotation; }
        float GetSpiralRotations() const { return _spiralRotations; }
        float GetBottomTopRatio() const { return _botTopRatio; }
        float GetTreePerspective() const { return _perspective; }
        int GetFirstStrand() const { return _firstStrand; }
        void SetFirstStrand(int val) { _firstStrand = val; }
        void SetTreeDegrees(long deg) { _degrees = deg; }
        void SetTreeType(int type) { _treeType = type; }
        void SetTreeRotation(float rot) { _rotation = rot; }
        void SetTreeSpiralRotations(float rot) { _spiralRotations = rot; }
        void SetTreeBottomTopRatio(float ratio) { _botTopRatio = ratio; }
        void SetPerspective(float pers) { _perspective = pers; }
    
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
        virtual void InitModel() override;
    private:
        int _treeType = 0;
        long _degrees = 360;
        float _rotation = 3.0f;
        float _spiralRotations = 0.0f;
        float _botTopRatio = 6.0f;
        float _perspective = 0.2f;
        int _firstStrand = 0;
        std::string _displayAs;
        void SetTreeCoord(long degrees);
};
