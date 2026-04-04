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

        [[nodiscard]] virtual std::list<std::string> CheckModelSettings() override;

        // we cant do low def on single node matrices
        [[nodiscard]] virtual bool SupportsLowDefinitionRender() const override { return SingleNode != true; }
        [[nodiscard]] bool isVerticalMatrix() const { return _vMatrix; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        virtual int GetNumStrings() const override { return _numStrings; }
        virtual int NodesPerString() const override;
        [[nodiscard]] int GetNumMatrixStrings() const { return _numStrings; }
        [[nodiscard]] int GetNodesPerString() const { return _nodesPerString; }
        [[nodiscard]] int GetStrandsPerString() const override { return _strandsPerString; }
        void SetNumMatrixStrings(int val) { _numStrings = val; }
        void SetNodesPerString(int val) { _nodesPerString = val; }
        void SetStrandsPerString(int val) { _strandsPerString = val; }

        [[nodiscard]] bool HasAlternateNodes() const { return _alternateNodes; }
        [[nodiscard]] bool IsNoZigZag() const { return _noZigZag; }
        [[nodiscard]] int GetLowDefFactor() const { return _lowDefFactor; }

        void SetAlternateNodes(bool val) { _alternateNodes = val; }
        void SetNoZigZag(bool val) { _noZigZag = val; }
        void SetVertical(bool val) { _vMatrix = val; }

    protected:
        virtual void InitModel() override;
        void InitSingleChannelModel();

        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();
    
        int _numStrings = 1;
        int _nodesPerString = 1;
        int _strandsPerString = 1;
        bool _vMatrix = false;
        bool _alternateNodes = false;
        bool _noZigZag = false;
    private:
};
