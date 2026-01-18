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

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return true; }
        [[nodiscard]] virtual bool SupportsWiringView() const override { return true; }

        [[nodiscard]] virtual bool SupportsXlightsModel() override { return true; }

        [[nodiscard]] virtual bool ModelSupportsLayerSizes() const override { return true; }
        virtual void OnLayerSizesChange(bool countChanged) override;

        [[nodiscard]] virtual bool SupportsVisitors() const override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        [[nodiscard]] bool IsInsideOut() const { return _insideOut; }
        void SetInsideOut(bool val) { _insideOut = val; }

    protected:
        virtual void InitModel() override;
        
    private:
        void SetCircleCoord();
        void InitCircle();
        int maxSize();

        bool _insideOut = false;
};
