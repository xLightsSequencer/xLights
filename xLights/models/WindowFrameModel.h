#ifndef WINDOWFRAMEMODEL_H
#define WINDOWFRAMEMODEL_H

#include "Model.h"


class WindowFrameModel : public Model
{
    public:
        WindowFrameModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~WindowFrameModel();

    protected:
        virtual void InitModel() override;

    private:
        void InitFrame();
};

#endif // WINDOWFRAMEMODEL_H
