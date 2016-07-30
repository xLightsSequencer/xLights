#ifndef POLYLINEMODEL_H
#define POLYLINEMODEL_H

#include "Model.h"


class PolyLineModel : public ModelWithScreenLocation<PolyPointScreenLocation>
{
    public:
        PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        PolyLineModel(const ModelManager &manager);
        virtual ~PolyLineModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        int GetPolyLineSize(int polyLineLayer) const {
            return polyLineSizes[polyLineLayer];
        }
        virtual int GetNumStrands() const override;
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual void InsertHandle(int after_handle) override;
        virtual void DeleteHandle(int handle) override;

        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual int OnPropertyGridSelection(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual void OnPropertyGridItemCollapsed(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual void OnPropertyGridItemExpanded(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        static std::vector<std::string> POLYLINE_BUFFER_STYLES;
        virtual void InitModel() override;
        int num_segments;
        struct xlPolyPoint {
            float x;
            float y;
            float length;
            mutable glm::mat3 *matrix;
        };
        float total_length;

        static std::string SegAttrName(int idx)
        {
            return wxString::Format(wxT("Seg%d"),idx+1).ToStdString();
        }

    private:
        std::vector<int> polyLineSizes;
        bool hasIndivSeg;
        bool segs_collapsed;
};

#endif // SINGLELINEMODEL_H
