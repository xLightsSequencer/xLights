#ifndef WREATHMODEL_H
#define WREATHMODEL_H

#include "Model.h"


class WreathModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        WreathModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~WreathModel();

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; } 
        virtual bool SupportsWiringView() const override { return true; }

    protected:
        virtual void InitModel() override;

    private:
        void InitWreath();
};

#endif // WREATHMODEL_H
