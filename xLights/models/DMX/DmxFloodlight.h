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

#include "DmxModel.h"

class DmxFloodlight : public DmxModel {
public:
    DmxFloodlight(const ModelManager &manager);
    virtual ~DmxFloodlight();

    void EnableFixedChannels(xlColorVector& pixelVector) const override;
    [[nodiscard]] std::vector<std::string> GenerateNodeNames() const override;

    virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const override;

    void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

protected:
    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;


    virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
    virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

    void GetColors(xlColor &center, xlColor &edge, bool allowSelected, const xlColor *c);
    virtual void DrawModel(xlVertexColorAccumulator *vac, xlColor &center, xlColor &edge, float beam_length);
};
