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

class xlTexture;

class LabelModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
public:
    explicit LabelModel(const ModelManager &manager);
    ~LabelModel() override;


    void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                       int &BufferWi, int &BufferHi, int stagger) const override;
    void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                               std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;

    void DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext *ctx,
                              xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                              const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                              bool highlightFirst = false, int highlightpixel = 0,
                              float *boundingBox = nullptr) override;
    void DisplayEffectOnWindow(IModelPreview* preview, double pointSize) override;

    [[nodiscard]] bool SupportsExportAsCustom() const override { return false; }
    [[nodiscard]] bool SupportsWiringView() const override { return false; }
    [[nodiscard]] int GetNumPhysicalStrings() const override { return 1; }
    [[nodiscard]] std::list<std::string> CheckModelSettings() override;

    [[nodiscard]] std::string GetLabelText() const { return _labelText; }
    [[nodiscard]] int GetLabelFontSize() const { return _fontSize; }
    [[nodiscard]] xlColor GetLabelTextColor() const { return _textColor; }

    void SetLabelText(const std::string& text) { _labelText = text; }
    void SetLabelFontSize(int size) { _fontSize = (size > 0) ? size : 14; }
    void SetLabelTextColor(const xlColor& color) { _textColor = color; }

    void Accept(BaseObjectVisitor& visitor) const override { visitor.Visit(*this); }

protected:
    void InitModel() override;

    std::string _labelText;
    int _fontSize;
    xlColor _textColor;

    mutable xlTexture* _fontTexture = nullptr;
    mutable int _cachedFontSize = 0;
};
