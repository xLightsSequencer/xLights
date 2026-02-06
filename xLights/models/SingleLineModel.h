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

class SingleLineModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        SingleLineModel(const ModelManager &manager);
        //SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        virtual ~SingleLineModel();

        void InitLine();
    
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        
        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual bool SupportsXlightsModel() override { return true; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

        const Model *GetParent() { return parent; }
    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;
        virtual bool IsNodeFirst(int n) const override;

    private:
        const Model *parent = nullptr;
};
