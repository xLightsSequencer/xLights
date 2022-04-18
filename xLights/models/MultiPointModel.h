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
#include "MultiPointScreenLocation.h"

class MultiPointModel : public ModelWithScreenLocation<MultiPointScreenLocation>
{
public:
    MultiPointModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    MultiPointModel(const ModelManager& manager);
    virtual ~MultiPointModel();

    virtual int GetLightsPerNode() const override { return 1; } // default to one unless a model supports this
    virtual int MapToNodeIndex(int strand, int node) const override;

    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    virtual int GetNumPhysicalStrings() const override;
    virtual void DeleteHandle(int handle) override;

    virtual bool SupportsXlightsModel() override { return true; }
    virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    virtual void ExportXlightsModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool IsNodeFirst(int node) const override;
    virtual int NodesPerString() const override;
    virtual int NodesPerString(int string) const override;
    virtual int MapPhysicalStringToLogicalString(int string) const override;

protected:
    virtual void InitModel() override;
    void InitLine();
    struct xlMultiPoint {
        float x;
        float y;
        float z;
    };

    static std::string StartNodeAttrName(int idx)
    {
        return wxString::Format(wxT("MultiNode%i"), idx + 1).ToStdString(); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }
    std::string ComputeStringStartNode(int x) const;
    int GetCustomNodeStringNumber(int node) const;
    void NormalizePointData();

    float height = 1.0f;
    int _strings;
    std::vector<int> stringStartNodes;
};
