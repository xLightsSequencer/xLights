#ifndef STARMODEL_H
#define STARMODEL_H

#include "Model.h"


class StarModel : public Model
{
    public:
        StarModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~StarModel();
    protected:
        virtual void InitModel() override;

    private:
};

#endif // STARMODEL_H
