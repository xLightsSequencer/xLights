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
        SpinnerModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~SpinnerModel();
        virtual int GetNumStrands() const override; 

        virtual int NodesPerString() const override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
            int &BufferWi, int &BufferHi, int stagger) const override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual void ExportXlightsModel() override;
        int GetHollowPercent() const { return hollow; }
        int GetArcAngle () const { return arc; }
        bool HasZigZag() const { return zigzag; }
        bool HasAlternateNodes() const { return alternate; }
        int GetStartAngle() const { return startangle; }

        virtual bool SupportsVisitors() const override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        SpinnerModel(const ModelManager &manager);
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int CalcChannelsPerString() override;
        int EncodeStartLocation();
        void DecodeStartLocation(int sl);

        void SetSpinnerCoord();
        int hollow;
        int arc;
        bool zigzag = false; // if true then numbering alternates in and out along arms
        bool alternate = false;
        int startangle = 90;
    private:
};
