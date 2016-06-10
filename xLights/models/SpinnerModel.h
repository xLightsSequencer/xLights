#ifndef SPINNERMODEL_H
#define SPINNERMODEL_H

#include "Model.h"


class SpinnerModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        SpinnerModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~SpinnerModel();
        virtual int GetNumStrands() const override;

        virtual bool StrandsZigZagOnString() const override { return true;};

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual void GetBufferSize(const std::string &type, const std::string &transform,
            int &BufferWi, int &BufferHi) const override;

    protected:
        SpinnerModel(const ModelManager &manager);
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int CalcCannelsPerString() override;

        void SetSpinnerCoord();
        int hollow;
        int arc;
        bool zigzag; // if true then numbering alternates in and out along arms
    private:
};

#endif // SPINNERMODEL_H
