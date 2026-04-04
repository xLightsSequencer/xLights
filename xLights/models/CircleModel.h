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

class CircleModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        CircleModel(const ModelManager &manager);
        virtual ~CircleModel();

        [[nodiscard]] virtual int GetStrandLength(int strand) const override;
        [[nodiscard]] virtual int MapToNodeIndex(int strand, int node) const override;
        [[nodiscard]] virtual int GetNumStrands() const override;
        [[nodiscard]] virtual bool AllNodesAllocated() const override;

        [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return true; }
        [[nodiscard]] virtual bool SupportsWiringView() const override { return true; }

        [[nodiscard]] virtual bool ModelSupportsLayerSizes() const override { return true; }
        virtual void OnLayerSizesChange(bool countChanged) override;

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        virtual int GetNumStrings() const override { return _numStrings; }
        virtual int NodesPerString() const override;
        [[nodiscard]] int GetNumCircleStrings() const { return _numStrings; }
        [[nodiscard]] int GetNodesPerString() const { return _nodesPerString; }
        void SetNumCircleStrings(int val) { _numStrings = val; }
        void SetCircleNodesPerString(int val) { _nodesPerString = val; }

        [[nodiscard]] int GetCenterPercent() const { return _centerPercent; }
        void SetCenterPercent(int val) { _centerPercent = val; }

        [[nodiscard]] bool IsInsideOut() const { return _insideOut; }
        void SetInsideOut(bool val) { _insideOut = val; }

    protected:
        virtual void InitModel() override;
        
    private:
        void SetCircleCoord();
        void InitCircle();
        int maxSize();

        int _numStrings = 1;
        int _nodesPerString = 1;
        int _centerPercent = 0;
        bool _insideOut = false;
};
