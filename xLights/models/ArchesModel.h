#ifndef ARCHESMODEL_H
#define ARCHESMODEL_H

#include "Model.h"


class ArchesModel : public Model
{
    public:
        ArchesModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~ArchesModel();
    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString();

    private:
        void SetArchCoord();
};

#endif // ARCHESMODEL_H
