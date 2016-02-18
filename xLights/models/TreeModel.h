#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "MatrixModel.h"


class TreeModel : public MatrixModel
{
    public:
        TreeModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~TreeModel();
    
        virtual int GetNumStrands() const override;

    protected:
        virtual void InitModel() override;
    private:
        void SetTreeCoord(long degrees);
};

#endif // TREEMODEL_H
