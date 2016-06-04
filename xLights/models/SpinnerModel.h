#ifndef SPINNERMODEL_H
#define SPINNERMODEL_H

#include "Model.h"


class SpinnerModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        SpinnerModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~SpinnerModel();
        virtual int GetNumStrands() const override;

        virtual bool StrandsZigZagOnString() const override { return true;};

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);
    
        SpinnerModel(const ModelManager &manager);
        virtual void InitModel() override;
    
        void InitSpinner();
    
        bool vMatrix;
    private:
};

#endif // SPINNERMODEL_H
