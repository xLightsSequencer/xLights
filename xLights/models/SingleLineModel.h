#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Model.h"

class SingleLineModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        SingleLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    
        SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        SingleLineModel(const ModelManager &manager);
        virtual ~SingleLineModel();

        void InitLine();
    
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        
        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return false; }

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
};
