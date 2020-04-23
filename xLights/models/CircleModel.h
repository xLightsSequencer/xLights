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

class CircleModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        CircleModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~CircleModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual bool AllNodesAllocated() const override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; } 
        virtual bool SupportsWiringView() const override { return true; }

		virtual bool SupportsXlightsModel() override { return true; }
		virtual void ExportXlightsModel() override;
		virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;


    protected:
        virtual void InitModel() override;
        
    private:
        void SetCircleCoord();
        void InitCircle();
    
        int maxSize();

        std::vector<int> circleSizes;
        bool insideOut = false;
};
