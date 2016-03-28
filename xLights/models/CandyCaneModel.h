#ifndef CNADYCANEMODEL_H
#define CNADYCANEMODEL_H

#include "Model.h"


class CandyCaneModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
    public:
        CandyCaneModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~CandyCaneModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
    
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;

    private:
        void SetCaneCoord();
		bool _reverse;
		bool _sticks;
        int skew;
        float caneheight;
};

#endif // CANDYCANEMODEL_H
