#ifndef SINGLELINEMODEL_H
#define SINGLELINEMODEL_H

#include "Model.h"


class SingleLineModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        SingleLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    
        SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        SingleLineModel(const ModelManager &manager);
        virtual ~SingleLineModel();

        void InitLine();
    
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const override;

        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
};

#endif // SINGLELINEMODEL_H
