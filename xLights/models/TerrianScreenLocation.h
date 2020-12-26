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

#include "ModelScreenLocation.h"

//Default location that uses a terrain grid with handles that can adjust in elevation
class TerrianScreenLocation : public BoxedScreenLocation {
public:
    TerrianScreenLocation();
    virtual ~TerrianScreenLocation() {}

    virtual void Read(wxXmlNode* node) override;
    virtual void Write(wxXmlNode* node) override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator& va, float zoom, int scale, bool drawBounding = true) const override;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(int mode) override;
    virtual void SetActiveAxis(int axis) override;
    virtual bool IsElevationHandle() const override { return active_handle > 0; }
    virtual bool CanEdit() const { return edit_active; }
    virtual void SetEdit(bool val) { edit_active = val; }
    virtual void* GetRawData() override { return (void*)&mPos; }

private:
    int num_points_wide;
    int num_points_deep;
    int num_points;
    int spacing;
    bool edit_active;
    mutable float minX, minY, maxX, maxY, minZ, maxZ;
    mutable std::vector<float> mPos;
};
