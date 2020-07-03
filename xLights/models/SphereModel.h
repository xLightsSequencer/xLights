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

class SphereModel : public MatrixModel
{
public:
    SphereModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    virtual ~SphereModel();

    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool SupportsXlightsModel() override { return true; }
    virtual bool SupportsExportAsCustom() const override { return true; }
    virtual bool SupportsWiringView() const override { return false; }
    virtual void ExportXlightsModel() override;
    virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    virtual int NodeRenderOrder() override { return 1; }
    virtual void ExportAsCustomXModel() const override;

protected:
    virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
    virtual void InitModel() override;
private:
    void SetSphereCoord();
    double _startLatitude = -86;
    double _endLatitude = 86;
    double _sphereDegrees = 360;
};

