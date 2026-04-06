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

class ArchesModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
    public:
        ArchesModel(const ModelManager &manager);
        virtual ~ArchesModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        virtual int GetNumPhysicalStrings() const override { return 1; }

        virtual int GetLightsPerNode() const override { return _lightsPerNode; }
        virtual int GetNumStrings() const override { return _numArches; }
        virtual int NodesPerString() const override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual std::string GetDimension() const override;

        virtual bool ModelSupportsLayerSizes() const override { return true; }
        virtual void OnLayerSizesChange(bool countChanged) override;

        [[nodiscard]] int GetNumArches() const { return _numArches; }
        [[nodiscard]] int GetNodesPerArch() const { return _nodesPerArch; }
        void SetNumArches(int val) { _numArches = val; }
        void SetNodesPerArch(int val) { _nodesPerArch = val; }
        void SetLightsPerNode(int val) { _lightsPerNode = val; }

        [[nodiscard]] bool GetZigZag() const { return _zigzag; }
        [[nodiscard]] int GetHollow() const { return _hollow; }
        [[nodiscard]] int GetGap() const { return _gap; }
        [[nodiscard]] int GetArc() const { return _arc; }

        void SetZigZag(bool zigzag) { _zigzag = zigzag; }
        void SetHollow(int hollow) { _hollow = hollow; }
        void SetGap(int gap) { _gap = gap; }
        void SetArc(int arc) { _arc = arc; }

        void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcChannelsPerString() override;
        virtual bool IsNodeFirst(int n) const override;
        virtual int GetStrandLength(int strand) const override;
        virtual int GetMappedStrand(int strand) const override;

    private:
        void SetLayerdArchCoord(int archcount, int maxLen);
        void SetArchCoord();
        int _numArches = 1;
        int _nodesPerArch = 1;
        int _lightsPerNode = 1;
        int _arc = 180;
        int _gap = 0;
        int _hollow = 70;
        bool _zigzag = false;
};
