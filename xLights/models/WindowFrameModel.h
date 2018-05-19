#ifndef WINDOWFRAMEMODEL_H
#define WINDOWFRAMEMODEL_H

#include "Model.h"


class WindowFrameModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        WindowFrameModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~WindowFrameModel();
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual int NodesPerString() override;

    protected:
        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    private:
        void InitFrame();
        int rotation;
};

#endif // WINDOWFRAMEMODEL_H
