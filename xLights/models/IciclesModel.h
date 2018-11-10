#ifndef ICICLESMODEL_H
#define ICICLESMODEL_H

#include "Model.h"


class IciclesModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
public:
    IciclesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    virtual ~IciclesModel();
    
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool SupportsExportAsCustom() const override { return true; }
    virtual bool SupportsWiringView() const override { return true; }

protected:
    virtual void InitModel() override;

private:
    void SetIciclesCoord();
    bool _alternateNodes = false;
};

#endif // ICICLESMODEL_H
