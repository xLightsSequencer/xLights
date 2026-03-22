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

class WreathModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        WreathModel(const ModelManager &manager);
        virtual ~WreathModel();

        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual int GetNumStrings() const override { return _numStrings; }
        virtual int NodesPerString() const override;

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

        [[nodiscard]] int GetNumWreathStrings() const { return _numStrings; }
        [[nodiscard]] int GetNodesPerString() const { return _nodesPerString; }
        void SetNumWreathStrings(int val) { _numStrings = val; }
        void SetWreathNodesPerString(int val) { _nodesPerString = val; }

    protected:
        virtual void InitModel() override;

    private:
        int _numStrings = 1;
        int _nodesPerString = 1;
        void InitWreath();
};

