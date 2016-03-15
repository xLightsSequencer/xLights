#ifndef SPHEREMODEL_H
#define SPHEREMODEL_H

#include "Model.h"


class SphereModel : public Model
{
    public:
        SphereModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~SphereModel();

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void InitModel() override;
    private:
        void InitSphere();

};

#endif // SPHEREMODEL_H
