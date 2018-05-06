#ifndef SPHEREMODEL_H
#define SPHEREMODEL_H

#include "Model.h"


class SphereModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        SphereModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~SphereModel();

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual bool IsZScaleable() const override { return false; }

    protected:
        virtual void InitModel() override;
    private:
        void InitSphere();

};

#endif // SPHEREMODEL_H
