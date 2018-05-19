#ifndef STARMODEL_H
#define STARMODEL_H

#include "Model.h"


class StarModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        StarModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~StarModel();
    
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        int GetStarSize(int starLayer) const {
            return starSizes[starLayer];
        }
        virtual int GetNumStrands() const override;
        virtual bool AllNodesAllocated() const override;

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        static std::vector<std::string> STAR_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
        std::vector<int> starSizes;
};

#endif // STARMODEL_H
