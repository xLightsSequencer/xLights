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

class IciclesModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
public:
    IciclesModel(const ModelManager &manager);
    virtual ~IciclesModel();
    
    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return true; }
    [[nodiscard]] virtual bool SupportsXlightsModel() override { return true; }
    [[nodiscard]] virtual bool SupportsWiringView() const override { return true; }
    [[nodiscard]] virtual std::string GetDimension() const override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override;
    [[nodiscard]] bool HasAlternateNodes() const { return _alternateNodes; }
    [[nodiscard]] std::string GetDropPattern() const { return _dropPatternString; }
    void SetDropPattern(const std::string & pattern);
    void SetAlternateNodes(bool val) { _alternateNodes = val; }

    [[nodiscard]] virtual bool SupportsVisitors() const override { return true; }
    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:
    virtual void InitModel() override;

private:
    void SetIciclesCoord();
    void ParseDropSizes();
    bool _alternateNodes = false;
    std::string _dropPatternString = "3,4,5,4";
    std::vector<size_t> _dropSizes;
    size_t _maxH = 0;
};
