#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Model.h"

class PolyLineModel : public ModelWithScreenLocation<PolyPointScreenLocation>
{
    public:
        PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        PolyLineModel(const ModelManager &manager);
        virtual ~PolyLineModel();

        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        int GetPolyLineSize(int polyLineLayer) const;
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual int GetNumStrands() const override;
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual int GetNumPhysicalStrings() const override { return 1; }

        virtual void InsertHandle(int after_handle, float zoom, int scale) override;
        virtual void DeleteHandle(int handle) override;

        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

        virtual bool SupportsXlightsModel() override {return true;}
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        virtual void ExportXlightsModel() override;

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
            float z;
            float length;
            mutable bool has_curve;
            mutable BezierCurveCubic3D* curve;
            mutable glm::mat4 *matrix;
        };
        float total_length;

        static std::string SegAttrName(int idx)
        {
            return wxString::Format(wxT("Seg%d"),idx+1).ToStdString();
        }
        void SetSegsCollapsed(bool collapsed);

        std::vector<int> polyLineSizes;
        bool hasIndivSeg;
        bool segs_collapsed;
        virtual void DistributeLightsAcrossCurveSegment(int lights, int segment, size_t &idx, std::vector<xlPolyPoint> &pPos,
                                                        std::vector<int>& dropSizes, unsigned int& drop_index, float& mheight, int& xx, int maxH);
        void NormalizePointData();
        std::vector<int> polyLineSegDropSizes;
        unsigned int numDropPoints;
        float height;
        bool _alternateNodes = false;
};
