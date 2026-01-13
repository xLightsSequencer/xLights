#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Model.h"

class PolyLineModel : public ModelWithScreenLocation<PolyPointScreenLocation>
{
public:
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
    virtual int GetNumStrings() const override{ return _strings; }

    virtual void InsertHandle(int after_handle, float zoom, int scale) override;
    virtual void DeleteHandle(int handle) override;
    void AddHandle();

    virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    virtual bool SupportsXlightsModel() override { return true; }
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual void OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual void OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool IsNodeFirst(int node) const override;
    virtual int NodesPerString(int string) const override;
    virtual int MapPhysicalStringToLogicalString(int string) const override;
    bool HasAlternateNodes() const { return _alternateNodes; }
    int GetDropPoints() const { return _numDropPoints; }
    int GetNumSegments() const { return _numSegments; }
    std::vector<int> GetSegmentsSizes() const { return _polyLineSizes;}
    std::vector<std::string> GetCorners() const { return _polyCorner; }
    [[nodiscard]] std::string GetDropPattern() const { return _dropPatternString; }
    void SetDropPattern(const std::string & pattern);

    bool AreSegsExpanded() const { return _segsCollapsed; }

    virtual bool SupportsVisitors() const override { return true; }
    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    float GetModelHeight() const { return _height; }
    void SetNumStrings(int strings) { _strings = strings; }
    void SetModelHeight(float height) { _height = height; }
    void SetAlternateNodes(bool val) { _alternateNodes = val; }
    void SetNumSegments(int val) { _polyLineSizes.resize(val); _polyLeadOffset.resize(val); _polyTrailOffset.resize(val); _polyLineSegDropSizes.resize(val); _polyCorner.resize(val+1); }
    void SetSegmentSize(int idx, int val);
    void SetLeadOffset(int idx, float val) { _polyLeadOffset[idx] = val; }
    void SetTrailOffset(int idx, float val) { _polyTrailOffset[idx] = val; }
    void SetCornerString( int idx, const std::string & corner) { _polyCorner[idx] = corner; }
    void SetAutoDistribute(bool val) { _autoDistributeLights = val; }
    void ClearPolyLineCreate() { _creatingNewPolyLine = false; }

    const std::string StartNodeAttrName(int idx) const
    {
        return wxString::Format(wxT("PolyNode%i"), idx + 1).ToStdString(); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }

    const std::string SegAttrName(int idx) const
    {
        return wxString::Format(wxT("Seg%d"), idx + 1).ToStdString();
    }
    
    const std::string CornerAttrName(int idx) const
    {
        return wxString::Format(wxT("Corner%d"), idx + 1).ToStdString();
    }

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

    void DistributeLightsEvenly(       std::vector<xlPolyPoint>& pPos,
                                 const std::vector<int>&         dropSizes,
                                 const float&                    mheight,
                                 const int                       maxH,
                                 const int                       numLights );

    void DistributeLightsAcrossIndivSegments(       std::vector<xlPolyPoint>& pPos,
                                              const std::vector<int>&         dropSizes,
                                              const float&                    mheight,
                                              const int                       maxH );

    void DistributeLightsAcrossSegment( const int                       segment,
                                              size_t&                   idx,
                                              std::vector<xlPolyPoint>& pPos,
                                        const std::vector<int>&         dropSizes,
                                              unsigned int&             drop_index,
                                        const float&                    mheight,
                                              int&                      xpos,
                                        const int                       maxH,
                                        const bool                      isCurve );

    int ComputeStringStartNode(int x) const;
    int GetCustomNodeStringNumber(int node) const;

    float _totalLength = 0.0f;
    int _numSegments = 0;
    std::vector<int> _polyLineSizes;
    std::vector<float> _polyLeadOffset;
    std::vector<float> _polyTrailOffset;
    bool _segsCollapsed = true;
    bool _autoDistributeLights = true;
    bool _creatingNewPolyLine = true;
    std::vector<int> _polyLineSegDropSizes;
    std::vector<int> _dropSizes;
    std::string _dropPatternString = "1";
    unsigned int _numDropPoints = 0;
    float _height = 1.0f;
    bool _alternateNodes = false;
    int _strings = 1;
    unsigned int _maxH = 0;
    std::vector<std::string> _polyCorner;
};
