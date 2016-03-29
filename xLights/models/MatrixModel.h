#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "Model.h"


class MatrixModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        MatrixModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~MatrixModel();
        virtual int GetNumStrands() const override;

        virtual bool StrandsZigZagOnString() const override { return true;};

    
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);
    
        MatrixModel(const ModelManager &manager);
        virtual void InitModel() override;
    
        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();
    
        bool vMatrix;
    private:
};

#endif // MATRIXMODEL_H
