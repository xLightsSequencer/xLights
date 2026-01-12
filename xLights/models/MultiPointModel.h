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
#include "MultiPointScreenLocation.h"

class MultiPointModel : public ModelWithScreenLocation<MultiPointScreenLocation>
{
public:
    MultiPointModel(const ModelManager& manager);
    virtual ~MultiPointModel();

    virtual int GetLightsPerNode() const override { return 1; } // default to one unless a model supports this
    virtual int MapToNodeIndex(int strand, int node) const override;

    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    virtual int GetNumPhysicalStrings() const override;
    virtual void DeleteHandle(int handle) override;

    virtual bool SupportsXlightsModel() override { return true; }
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool IsNodeFirst(int node) const override;
    virtual int NodesPerString() const override;
    virtual int NodesPerString(int string) const override;
    virtual int MapPhysicalStringToLogicalString(int string) const override;
    
    int GetNumStrings() const { return _strings; }
    float GetModelHeight() const { return _height; }
    void SetNumStrings(int strings) { _strings = strings; }
    void SetModelHeight(float height) { _height = height; }

    const std::string StartNodeAttrName(int idx) const
    {
        return wxString::Format(wxT("MultiNode%i"), idx + 1).ToStdString(); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }

protected:
    virtual void InitModel() override;
    void InitLine();
    struct xlMultiPoint {
        float x;
        float y;
        float z;
    };

    int ComputeStringStartNode(int x) const;
    int GetCustomNodeStringNumber(int node) const;
    void NormalizePointData();

    float _height = 1.0f;
    int _strings = 1;
    std::vector<int> stringStartNodes;
};
