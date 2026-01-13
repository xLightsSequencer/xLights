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

#include "MatrixModel.h"

class SphereModel : public MatrixModel
{
public:
    SphereModel(const ModelManager &manager);
    virtual ~SphereModel();

    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool SupportsXlightsModel() override { return true; }
    virtual bool SupportsWiringView() const override { return false; }
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;
    virtual int NodeRenderOrder() override { return 1; }
    virtual void ExportAsCustomXModel3D() const override;
    bool Find3DCustomModelScale(int scale, float minx, float miny, float minz, float w, float h, float d) const;
    virtual bool SupportsExportAsCustom3D() const override { return true; }
    int GetStartLatitude() const { return _startLatitude; }
    int GetEndLatitude() const { return _endLatitude; }
    int GetSphereDegrees() const { return _sphereDegrees; }
    int GetLowDefFactor() const { return _lowDefFactor; }
    void SetStartLatitude(int lat) { _startLatitude = lat; }
    void SetEndLatitude(int lat) { _endLatitude = lat; }
    void SetDegrees(int deg) { _sphereDegrees = deg; }

    virtual bool SupportsVisitors() const override { return true; }
    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:
    virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
    virtual void InitModel() override;
private:
    void SetSphereCoord();
    double _startLatitude = -86;
    double _endLatitude = 86;
    double _sphereDegrees = 360;
};
