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

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; } 
        virtual bool SupportsWiringView() const override { return true; }
        virtual bool SupportsXlightsModel() override { return true; }

        virtual bool SupportsVisitors() const override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void InitModel() override;

    private:
        void InitWreath();
};

