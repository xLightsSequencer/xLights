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

class TwinklyModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
public:
    TwinklyModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    virtual ~TwinklyModel();

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual bool SupportsExportAsCustom() const override
    {
        return false;
    }
    virtual bool SupportsWiringView() const override
    {
        return false;
    }

    virtual bool SupportsXlightsModel() override
    {
        return false;
    }

    virtual bool ModelSupportsLayerSizes() const override
    {
        return true;
    }

    // set layout from twinkly device
    bool SetLayout(const wxJSONValue& layout);

protected:
    virtual void InitModel() override;

private:
    static constexpr double EXPECTED_HEIGHT = 100;
    struct LightPosition {
        double x, y, z;
    };

    void SaveToXml();

    std::vector<LightPosition> lights;
};
