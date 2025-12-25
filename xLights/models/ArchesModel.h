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
        ArchesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~ArchesModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
        virtual int GetNumPhysicalStrings() const override { return 1; }

        virtual int GetLightsPerNode() const override { return parm3; } // default to one unless a model supports this
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;
        virtual std::string GetDimension() const override;
        virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override;

        virtual bool ModelSupportsLayerSizes() const override { return true; }
        virtual void OnLayerSizesChange(bool countChanged) override;

        bool GetZigZag() const { return zigzag; }
        int GetHollow() const { return _hollow; }
        int GetGap() const { return _gap; }
        int GetArc() const { return arc; }

        //virtual bool SupportsVisitors() override {return true;}
        //void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

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
        int arc;
        int _gap = 0;
        int _hollow = 70;
        bool zigzag = true;
};
