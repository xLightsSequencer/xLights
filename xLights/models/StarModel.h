#ifndef STARMODEL_H
#define STARMODEL_H

#include "Model.h"


class StarModel : public Model
{
    public:
        StarModel(wxXmlNode *node, NetInfoClass &netInfo);
        virtual ~StarModel();
    protected:
    private:
};

#endif // STARMODEL_H
