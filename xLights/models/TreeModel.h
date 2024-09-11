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

#include "MatrixModel.h"

class TreeModel : public MatrixModel {
public:
    TreeModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    virtual ~TreeModel();

    [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    [[nodiscard]] virtual bool SupportsXlightsModel() override {
        return true;
    }
    [[nodiscard]] virtual bool SupportsExportAsCustom() const override {
        return true;
    }
    virtual void ExportAsCustomXModel3D() const override;
    [[nodiscard]] virtual bool SupportsExportAsCustom3D() const override {
        return true;
    }
    [[nodiscard]] virtual bool SupportsWiringView() const override {
        return true;
    }
    virtual void ExportXlightsModel() override;
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    [[nodiscard]] virtual int NodeRenderOrder() override {
        return 1;
    }
    [[nodiscard]] virtual bool SupportsLowDefinitionRender() const override {
        return false; // we need to override this as the matrix model can set it to true
    }
    virtual void GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int& BufferDp, int stagger) const override;
    virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
                                       std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int& BufferDp, int stagger, bool deep = false) const override;
    [[nodiscard]] virtual bool Supports3DBuffer() const override {
        return DisplayAs != "Tree Flat" && DisplayAs != "Tree Ribbon";
    }

protected:
    virtual void AddStyleProperties(wxPropertyGridInterface* grid) override;
    virtual void InitModel() override;

private:
    int treeType = 0;
    long degrees = 360;
    float rotation = 3.0f;
    float spiralRotations = 0.0f;
    float botTopRatio = 6.0f;
    float perspective = 0.2f;
    void SetTreeCoord(long degrees);
};
