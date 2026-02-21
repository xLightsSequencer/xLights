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

class CandyCaneModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
    public:
        CandyCaneModel(const ModelManager &manager);
        virtual ~CandyCaneModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual std::string GetDimension() const override;
        virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override;

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }
        [[nodiscard]] bool IsReverse() const { return _reverse; }
        [[nodiscard]] bool IsSticks() const { return _sticks; }
        [[nodiscard]] bool HasAlternateNodes() const { return _alternateNodes; }
        [[nodiscard]] float GetCandyCaneHeight() const { return _caneheight; }
        void SetReverse(bool reverse) { _reverse = reverse; }
        void SetSticks(bool sticks) { _sticks = sticks; }
        void SetAlternateNodes(bool alternateNodes) { _alternateNodes = alternateNodes; }
        void SetCaneHeight(float caneheight) { _caneheight = caneheight; }

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcChannelsPerString() override;
        virtual bool IsNodeFirst(int node) const override;

    private:
        void SetCaneCoord();
		bool _reverse = false;
		bool _sticks = false;
        bool _alternateNodes = false;
        float _caneheight = 1.0;
};
