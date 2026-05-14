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

#include "BoxedScreenLocation.h"

class xlGraphicsProgram;

//Default location that uses a terrain grid with handles that can adjust in elevation
class TerrainScreenLocation : public BoxedScreenLocation {
public:
    TerrainScreenLocation();
    virtual ~TerrainScreenLocation() {}

    virtual void Init() override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;
    
    [[nodiscard]] std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& id) override;
    // Elevation-tool handler — applies the SpaceMouse Z delta to
    // mPos[point] and (when tool_size > 1) the surrounding terrain
    // cells. Public so the session class in the .cpp can call it.
    void ApplySpaceMouseElevation(int point, float scale, const glm::vec3& mov);

    // R-10b: Terrain inherits the base CentreCycle SetActiveHandleToCentre.
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;
    virtual bool IsElevationHandle() const override { return IsRole(active_handle, handles::Role::Vertex); }
    virtual bool CanEdit() const { return edit_active; }
    virtual void SetEdit(bool val) override { edit_active = val; }
    virtual void* GetRawData() override { return (void*)&mPos; }

    int GetNumPointsWide() const { return num_points_wide; }
    int GetNumPointsDeep() const { return num_points_deep; }
    int GetNumPoints() const { return num_points; }

    [[nodiscard]] float GetGridHeight(int idx) const {
        return (idx >= 0 && idx < static_cast<int>(mPos.size())) ? mPos[idx] : 0.0f;
    }
    void SetGridHeight(int idx, float h) {
        if (idx >= 0 && idx < static_cast<int>(mPos.size())) mPos[idx] = h;
    }
    [[nodiscard]] std::vector<handles::Descriptor> GetHandles(
        handles::ViewMode mode, handles::Tool tool,
        const handles::ViewParams& view = {}) const override;
    std::unique_ptr<handles::DragSession> CreateDragSession(
        const std::string& modelName,
        const handles::Id& id,
        const handles::WorldRay& startRay) override;

    void UpdateSize(int wide, int deep, int num_points);

    void SetDataFromString(const std::string& point_data);
    const std::string GetDataAsString() const;

protected:
    
private:
    int num_points_wide = 41;
    int num_points_deep = 21;
    int num_points = 0;
    int spacing = 50;
    bool edit_active = false;
    std::vector<float> mPos;
};
