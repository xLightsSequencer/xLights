#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "Model.h"


class MatrixModel : public Model
{
    public:
        MatrixModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~MatrixModel();
        virtual int GetNumStrands() const override;

        virtual bool StrandsZigZagOnString() const override { return true;};

    protected:
        MatrixModel();
        virtual void InitModel() override;
    
        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();
    private:
};

#endif // MATRIXMODEL_H
