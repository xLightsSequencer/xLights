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
#include <string>

class StarModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        StarModel(const ModelManager &manager);
        virtual ~StarModel();
    
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetMappedStrand(int strand) const override;

        int GetStarSize(int starLayer) const { return GetLayerSize(starLayer); }
        virtual int GetNumStrands() const override;
        virtual bool AllNodesAllocated() const override;

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
    
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool ModelSupportsLayerSizes() const override { return true; }
        virtual void OnLayerSizesChange(bool countChanged) override;
        float GetStarRatio() const { return _starRatio; }
        void SetStarRatio(float ratio) { _starRatio = ratio; }
        int GetInnerPercent() const { return _innerPercent; }
        void SetInnerPercent(int percent) { _innerPercent = percent; }
        std::string GetStartLocation() const override { return _starStartLocation; }
        void SetStarStartLocation(const std::string & location) { _starStartLocation = location; }
        std::string ConvertFromDirStartSide();
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        static std::vector<std::string> STAR_BUFFER_STYLES;
        virtual void InitModel() override;
        wxRealPoint GetPointOnCircle(double radius, double angle);
        double LineLength(wxRealPoint start, wxRealPoint end);
        wxRealPoint GetPositionOnLine(wxRealPoint start, wxRealPoint end, double distance);

    private:
        // The ratio between the inner and outer radius of the star; default is 2.618034.
    float _starRatio = 2.618034f;
        // The ratio between the inner start and outer star radius (if more than 1 layer)
        int _innerPercent = -1;
        std::string _starStartLocation = "Bottom Ctr-CW";
};
