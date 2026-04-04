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

class SpinnerModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        SpinnerModel(const ModelManager &manager);
        virtual ~SpinnerModel();
        virtual int GetNumStrands() const override; 

        virtual int NodesPerString() const override;

        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
            int &BufferWi, int &BufferHi, int stagger) const override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual int GetNumStrings() const override { return _numStrings; }

        [[nodiscard]] int GetNumSpinnerStrings() const { return _numStrings; }
        [[nodiscard]] int GetNodesPerArm() const { return _nodesPerArm; }
        [[nodiscard]] int GetArmsPerString() const { return _armsPerString; }
        void SetNumSpinnerStrings(int val) { _numStrings = val; }
        void SetNodesPerArm(int val) { _nodesPerArm = val; }
        void SetArmsPerString(int val) { _armsPerString = val; }

        int GetHollowPercent() const { return _hollow; }
        int GetArcAngle () const { return _arc; }
        bool HasZigZag() const { return _zigzag; }
        bool HasAlternateNodes() const { return _alternate; }
        int GetStartAngle() const { return _startAngle; }
        void SetHollow(int val) { _hollow = val; }
        void SetStartAngle(int angle) { _startAngle = angle; }
        void SetArc(int val) { _arc = val; }
        void SetZigZag(bool val) { _zigzag = val; }
        void SetAlternate(bool val) { _alternate = val; }

        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int CalcChannelsPerString() override;
    public:
        int EncodeStartLocation();
        void DecodeStartLocation(int sl);
    protected:
        void SetSpinnerCoord();

        int _numStrings = 1;
        int _nodesPerArm = 1;
        int _armsPerString = 1;
        int _hollow = 20;
        int _arc = 360;
        bool _zigzag = false; // if true then numbering alternates in and out along arms
        bool _alternate = false;
        int _startAngle = 0;
    private:
};
