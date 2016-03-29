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

    
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
};

#endif // SINGLELINEMODEL_H
