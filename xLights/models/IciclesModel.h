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

class IciclesModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
public:
    IciclesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    virtual ~IciclesModel();
    
    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool SupportsExportAsCustom() const override { return true; }
    virtual bool SupportsXlightsModel() override { return true; }
    virtual bool SupportsWiringView() const override { return true; }
    virtual std::string GetDimension() const override;
    virtual void ExportXlightsModel() override;
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override;

protected:
    virtual void InitModel() override;

private:
    void SetIciclesCoord();
    bool _alternateNodes = false;
};
