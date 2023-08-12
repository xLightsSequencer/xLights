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
    PolyLineModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    PolyLineModel(const ModelManager& manager);
    virtual ~PolyLineModel();

    virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
    virtual int GetStrandLength(int strand) const override;
    virtual int MapToNodeIndex(int strand, int node) const override;

    int GetPolyLineSize(int polyLineLayer) const;
    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    virtual int GetNumStrands() const override;
    virtual const std::vector<std::string>& GetBufferStyles() const override;
    virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform, std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep = false) const override;
    virtual int NodesPerString() const override;
    virtual int GetNumPhysicalStrings() const override;

    virtual void InsertHandle(int after_handle, float zoom, int scale) override;
    virtual void DeleteHandle(int handle) override;

    virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    virtual bool SupportsXlightsModel() override { return true; }
    virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    virtual void ExportXlightsModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual void OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual void OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool IsNodeFirst(int node) const override;
    virtual int NodesPerString(int string) const override;
    virtual int MapPhysicalStringToLogicalString(int string) const override;

protected:
    static std::vector<std::string> POLYLINE_BUFFER_STYLES;
    virtual void InitModel() override;
    struct xlPolyPoint {
        float x;
        float y;
        float z;
        float length;
        mutable bool has_curve;
        mutable BezierCurveCubic3D* curve;
        mutable glm::mat4* matrix;
    };

    static std::string SegAttrName(int idx)
    {
        return wxString::Format(wxT("Seg%d"), idx + 1).ToStdString();
    }
    void SetSegsCollapsed(bool collapsed);
    
    void DistributeLightsEvenly( const std::vector<xlPolyPoint>& pPos,
                                 const std::vector<int>&         dropSizes,
                                 const float&                    mheight,
                                 const int                       maxH,
                                 const int                       numLights );

    void DistributeLightsAcrossIndivSegments( const std::vector<xlPolyPoint>& pPos,
                                              const std::vector<int>&         dropSizes,
                                              const float&                    mheight,
                                              const int                       maxH );

    void DistributeLightsAcrossSegment( const int                       segment,
                                              size_t&                   idx,
                                        const std::vector<xlPolyPoint>& pPos,
                                        const std::vector<int>&         dropSizes,
                                              unsigned int&             drop_index,
                                        const float&                    mheight,
                                              int&                      xpos,
                                        const int                       maxH,
                                        const bool                      isCurve );

    static std::string StartNodeAttrName(int idx)
    {
        return wxString::Format(wxT("PolyNode%i"), idx + 1).ToStdString(); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }
    std::string ComputeStringStartNode(int x) const;
    int GetCustomNodeStringNumber(int node) const;

    static std::string CornerAttrName(int idx)
    {
        return wxString::Format(wxT("Corner%d"), idx + 1).ToStdString();
    }

    float total_length = 0.0f;
    int num_segments = 0;
    std::vector<int> polyLineSizes;
    std::vector<float> polyLeadOffset;
    std::vector<float> polyTrailOffset;
    std::vector<float> polyGapSize;
    bool hasIndivSeg = false;
    bool segs_collapsed = true;
    void NormalizePointData();
    std::vector<int> polyLineSegDropSizes;
    unsigned int numDropPoints = 0;
    float height = 1.0f;
    bool _alternateNodes = false;
    int _strings;
    std::vector<int> stringStartNodes;
};
