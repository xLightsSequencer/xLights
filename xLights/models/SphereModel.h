#ifndef SPHEREMODEL_H
#define SPHEREMODEL_H

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

protected:
    virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
    virtual void InitModel() override;
private:
    void SetSphereCoord();
};

#endif // SPHEREMODEL_H
