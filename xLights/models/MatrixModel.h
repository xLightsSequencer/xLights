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
        MatrixModel(const ModelManager &manager);
        virtual ~MatrixModel();
        [[nodiscard]] virtual int GetNumStrands() const override;

        [[nodiscard]] bool SupportsChangingStringCount() const override{ return true; };
        [[nodiscard]] bool ChangeStringCount(long count, std::string& message) override;
        [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return true; }
        [[nodiscard]] virtual bool SupportsWiringView() const override { return true; }

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        [[nodiscard]] virtual std::list<std::string> CheckModelSettings() override;

        // we cant do low def on single node matrices
        [[nodiscard]] virtual bool SupportsLowDefinitionRender() const override { return SingleNode != true; }
        [[nodiscard]] bool isVerticalMatrix() const { return _vMatrix; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        [[nodiscard]] bool HasAlternateNodes() const { return _alternateNodes; }
        [[nodiscard]] bool IsNoZigZag() const { return _noZigZag; }
        [[nodiscard]] int GetLowDefFactor() const { return _lowDefFactor; }

        void SetAlternateNodes(bool val) { _alternateNodes = val; }
        void SetNoZigZag(bool val) { _noZigZag = val; }
        void SetVertical(bool val) { _vMatrix = val; }

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);
    
        virtual void InitModel() override;
        void InitSingleChannelModel();

        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();
    
        bool _vMatrix = false;
        bool _alternateNodes = false;
        bool _noZigZag = false;
    private:
};
