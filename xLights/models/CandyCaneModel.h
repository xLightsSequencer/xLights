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

class CandyCaneModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
    public:
        CandyCaneModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
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

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;
        virtual bool IsNodeFirst(int node) const override;

    private:
        void SetCaneCoord();
		bool _reverse = false;
		bool _sticks = false;
        bool _alternateNodes = false;
        float _caneheight = 1.0;
};
