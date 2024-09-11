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

class SphereModel : public MatrixModel {
public:
    SphereModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    virtual ~SphereModel();

    [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    [[nodiscard]] virtual bool SupportsXlightsModel() override {
        return true;
    }
    [[nodiscard]] virtual bool SupportsWiringView() const override {
        return false;
    }
    virtual void ExportXlightsModel() override;
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
    [[nodiscard]] virtual int NodeRenderOrder() override {
        return 1;
    }
    virtual void ExportAsCustomXModel3D() const override;
    [[nodiscard]] bool Find3DCustomModelScale(int scale, float minx, float miny, float minz, float w, float h, float d) const;
    [[nodiscard]] virtual bool SupportsExportAsCustom3D() const override {
        return true;
    }
    virtual void GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int& BufferDp, int stagger) const override;
    virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
                                       std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int& BufferDp, int stagger, bool deep = false) const override;
    [[nodiscard]] virtual bool Supports3DBuffer() const override {
        return true;
    }

protected:
    virtual void AddStyleProperties(wxPropertyGridInterface* grid) override;
    virtual void InitModel() override;

private:
    void SetSphereCoord();
    double _startLatitude = -86;
    double _endLatitude = 86;
    double _sphereDegrees = 360;
};
