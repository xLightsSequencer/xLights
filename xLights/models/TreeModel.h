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

#include "MatrixModel.h"

class TreeModel : public MatrixModel
{
    public:
        TreeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~TreeModel();
    
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        virtual int NodeRenderOrder() override {return 1;}

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
        virtual void InitModel() override;
    private:
        int treeType;
        long degrees;
        float rotation;
        float spiralRotations;
        float botTopRatio;
        float perspective;
        void SetTreeCoord(long degrees);
};
