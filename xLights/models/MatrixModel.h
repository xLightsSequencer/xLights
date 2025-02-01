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

#include "Model.h"

class MatrixModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        MatrixModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~MatrixModel();
        virtual int GetNumStrands() const override;

        bool SupportsChangingStringCount() const override{ return true; };
        bool ChangeStringCount(long count, std::string& message) override;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual std::list<std::string> CheckModelSettings() override;

        // we cant do low def on single node matrices
        virtual bool SupportsLowDefinitionRender() const override { return SingleNode != true; }
        bool isVerticalMatrix() const { return vMatrix; }

        virtual bool SupportsVisitors() override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        bool HasAlternateNodes() const { return _alternateNodes; }
        bool IsNoZigZag() const { return _noZig; }
        int GetLowDefFactor() const { return _lowDefFactor; }

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);
    
        MatrixModel(const ModelManager &manager);
        virtual void InitModel() override;
        void InitSingleChannelModel();

        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();
    
        bool vMatrix = false;
        bool _alternateNodes = false;
        bool _noZig = false;
    private:
};
