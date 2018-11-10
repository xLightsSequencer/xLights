#ifndef SUBMODEL_H
#define SUBMODEL_H

#include "Model.h"

class SubModel : public Model {
public:
    SubModel(Model *p, wxXmlNode *n);
    virtual ~SubModel() {}

    virtual std::string GetFullName() const override { return parent->GetFullName() + "/" + name;}

    virtual const ModelScreenLocation &GetModelScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetModelScreenLocation() override { return parent->GetModelScreenLocation(); };

    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const override { return parent->GetModelScreenLocation(); }
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() override { return parent->GetModelScreenLocation(); };
    virtual void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override {}

    virtual const std::string &GetLayoutGroup() const override { return parent->GetLayoutGroup(); }

    virtual void AddProperties(wxPropertyGridInterface *grid, OutputManager* outputManager) override {}
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override {}
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}
    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    bool IsNodesAllValid() const { return _nodesAllValid; }

    Model* GetParent() const { return parent; }
private:
    Model *parent;
    bool _nodesAllValid;
};


#endif // SUBMODEL_H
