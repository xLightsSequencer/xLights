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

    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    virtual bool SupportsExportAsCustom() const override { return false; }
    virtual bool SupportsWiringView() const override { return false; }
    virtual int GetNumPhysicalStrings() const override;
    virtual void DeleteHandle(int handle) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool IsNodeFirst(int node) const override;
    
    virtual int GetNumStrings() const override{ return _strings; }
    float GetModelHeight() const { return _height; }
    void SetNumStrings(int strings) { _strings = strings; }
    void SetModelHeight(float height) { _height = height; }

    const std::string StartNodeAttrName(int idx) const override
    {
        return "MultiNode" + std::to_string(idx + 1); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }

protected:
    virtual void InitModel() override;
    void InitLine();
    struct xlMultiPoint {
        float x;
        float y;
        float z;
    };

    void NormalizePointData();

    float _height = 1.0f;
    int _strings = 1;
};
