#ifndef SINGLELINEMODEL_H
#define SINGLELINEMODEL_H

#include "Model.h"


class SingleLineModel : public Model
{
    public:
        SingleLineModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
    
        SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        virtual ~SingleLineModel();

        void InitLine();

    protected:
        virtual void InitModel() override;

    private:
};

#endif // SINGLELINEMODEL_H
