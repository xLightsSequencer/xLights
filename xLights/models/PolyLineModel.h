#ifndef POLYLINEMODEL_H
#define POLYLINEMODEL_H

#include "Model.h"


class PolyLineModel : public ModelWithScreenLocation<PolyPointScreenLocation>
{
    public:
        PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);

        PolyLineModel(int lights, const Model &base, int strand, int node = -1);
        PolyLineModel(const ModelManager &manager);
        virtual ~PolyLineModel();

        void InitLine();

        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const override;


        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;

        int num_segments;

    private:
        std::vector<int> polyLineSizes;
};

#endif // SINGLELINEMODEL_H
