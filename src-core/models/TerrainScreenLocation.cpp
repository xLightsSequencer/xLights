/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include <cassert>
#include <spdlog/fmt/fmt.h>
#include "../utils/string_utils.h"
#include "../utils/VectorMath.h"

#include "TerrainScreenLocation.h"
#include "../utils/VectorMath.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../graphics/xlGraphicsContext.h"

#include <log.h>

#define BOUNDING_RECT_OFFSET        8
#define NUM_TERRAIN_HANDLES   861   // default number of points for a 40x20 grid

TerrainScreenLocation::TerrainScreenLocation()
    : BoxedScreenLocation(NUM_TERRAIN_HANDLES+ 9)
{
    mSelectableHandles = NUM_TERRAIN_HANDLES;
    num_points = num_points_wide * num_points_deep;
}

bool TerrainScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {

    auto va = program->getAccumulator();
    va->PreAlloc((mSelectableHandles + 5) * 5);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    }
    else if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }

    float x_offset = (num_points_wide - 1) * spacing / 2;
    float z_offset = (num_points_deep - 1) * spacing / 2;
    float y_min = -10.0f;
    float y_max = 10.0f;
    float handle_width = GetRectHandleWidth(zoom, scale);
    float sx, sy, sz;
    
    int startVert = va->getCount();
    
    if (!edit_active) {
        if (IsRole(active_handle, handles::Role::CentreCycle)) {
            va->AddSphereAsTriangles(worldPos_x, worldPos_y, worldPos_z, (double)(handle_width), xlORANGETRANSLUCENT);
        }
        // calculate elevation boundary
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                int abs_point = j * num_points_wide + i;
                sy = mPos[abs_point];
                if (sy < y_min) {
                    y_min = sy;
                }
                if (sy > y_max) {
                    y_max = sy;
                }
            }
        }
    } else {
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                int abs_point = j * num_points_wide + i;
                sx = i * spacing - x_offset;
                sz = j * spacing - z_offset;
                sy = mPos[abs_point];
                if (sy < y_min) {
                    y_min = sy;
                }
                if (sy > y_max) {
                    y_max = sy;
                }
                TranslatePoint(sx, sy, sz);
                if (IsHandle(highlighted_handle, handles::Role::Vertex, abs_point) ||
                    IsHandle(active_handle, handles::Role::Vertex, abs_point)) {
                    if (!_locked) {
                        if (IsHandle(active_handle, handles::Role::Vertex, abs_point)) {
                            handleColor = xlYELLOWTRANSLUCENT;
                        } else {
                            handleColor = xlBLUETRANSLUCENT;
                        }
                    }
                    va->AddSphereAsTriangles(sx, sy, sz, (double)(handle_width), handleColor);
                }
            }
        }
    }
    int endVert = va->getCount();
    if (startVert != endVert) {
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(va, startVert, endVert - startVert);
        });
    }

    xlColor Box3dColor = xlWHITE;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;

    if (active_handle.has_value()) {
        glm::vec3 active_handle_pos = GetActiveHandlePosition();
        DrawAxisTool(active_handle_pos, program, zoom, scale);
        DrawActiveAxisIndicator(active_handle_pos, program);
    }

    // Snapshot AFTER the gizmo helpers so the outer drawLines step
    // below covers only the bounding-box wireframe — both helpers
    // submit their own addSteps and would be double-rendered (and
    // their triangle vertices misinterpreted as line pairs) if we
    // included their range here.
    startVert = va->getCount();
    //draw the new bounding box
    glm::vec4 c1(aabb_min.x, aabb_max.y, aabb_min.z, 1.0f);
    glm::vec4 c2(aabb_max.x, aabb_max.y, aabb_min.z, 1.0f);
    glm::vec4 c3(aabb_max.x, aabb_min.y, aabb_min.z, 1.0f);
    glm::vec4 c4(aabb_min.x, aabb_min.y, aabb_min.z, 1.0f);
    glm::vec4 c5(aabb_min.x, aabb_max.y, aabb_max.z, 1.0f);
    glm::vec4 c6(aabb_max.x, aabb_max.y, aabb_max.z, 1.0f);
    glm::vec4 c7(aabb_max.x, aabb_min.y, aabb_max.z, 1.0f);
    glm::vec4 c8(aabb_min.x, aabb_min.y, aabb_max.z, 1.0f);

    c1 = ModelMatrix * c1;
    c2 = ModelMatrix * c2;
    c3 = ModelMatrix * c3;
    c4 = ModelMatrix * c4;
    c5 = ModelMatrix * c5;
    c6 = ModelMatrix * c6;
    c7 = ModelMatrix * c7;
    c8 = ModelMatrix * c8;

    va->AddVertex(c1.x, c1.y, c1.z, Box3dColor);
    va->AddVertex(c2.x, c2.y, c2.z, Box3dColor);
    va->AddVertex(c2.x, c2.y, c2.z, Box3dColor);
    va->AddVertex(c3.x, c3.y, c3.z, Box3dColor);
    va->AddVertex(c3.x, c3.y, c3.z, Box3dColor);
    va->AddVertex(c4.x, c4.y, c4.z, Box3dColor);
    va->AddVertex(c4.x, c4.y, c4.z, Box3dColor);
    va->AddVertex(c1.x, c1.y, c1.z, Box3dColor);

    va->AddVertex(c5.x, c5.y, c5.z, Box3dColor);
    va->AddVertex(c6.x, c6.y, c6.z, Box3dColor);
    va->AddVertex(c6.x, c6.y, c6.z, Box3dColor);
    va->AddVertex(c7.x, c7.y, c7.z, Box3dColor);
    va->AddVertex(c7.x, c7.y, c7.z, Box3dColor);
    va->AddVertex(c8.x, c8.y, c8.z, Box3dColor);
    va->AddVertex(c8.x, c8.y, c8.z, Box3dColor);
    va->AddVertex(c5.x, c5.y, c5.z, Box3dColor);

    va->AddVertex(c1.x, c1.y, c1.z, Box3dColor);
    va->AddVertex(c5.x, c5.y, c5.z, Box3dColor);
    va->AddVertex(c2.x, c2.y, c2.z, Box3dColor);
    va->AddVertex(c6.x, c6.y, c6.z, Box3dColor);
    va->AddVertex(c3.x, c3.y, c3.z, Box3dColor);
    va->AddVertex(c7.x, c7.y, c7.z, Box3dColor);
    va->AddVertex(c4.x, c4.y, c4.z, Box3dColor);
    va->AddVertex(c8.x, c8.y, c8.z, Box3dColor);

    endVert = va->getCount();
    if (endVert != startVert) {
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(va, startVert, endVert - startVert);
        });
    }
    return true;
}


namespace {
// SpaceMouse session for TerrainScreenLocation. Vertex handles
// drive the elevation brush; anything else delegates to the
// BoxedScreenLocation session for whole-model translate/rotate.
class TerrainSpaceMouseSession : public handles::SpaceMouseSession {
public:
    TerrainSpaceMouseSession(TerrainScreenLocation* loc,
                             std::optional<handles::Id> id)
        : _loc(loc), _id(id),
          _boxedInner(loc->BoxedScreenLocation::BeginSpaceMouseSession(id)) {}

    handles::SpaceMouseResult Apply(float scale,
                                     const glm::vec3& rot,
                                     const glm::vec3& mov) override {
        if (!_loc || _loc->IsLocked()) return handles::SpaceMouseResult::Unchanged;
        if (_id.has_value() && _id->role == handles::Role::Vertex) {
            _loc->ApplySpaceMouseElevation(_id->index, scale, mov);
            // Legacy code returned 1 (no NEEDS_INIT) for terrain
            // elevation; preserve that — the model regenerates its
            // mesh from mPos[] each draw, no Setup() rebuild needed.
            return handles::SpaceMouseResult::Dirty;
        }
        if (_boxedInner) {
            return _boxedInner->Apply(scale, rot, mov);
        }
        return handles::SpaceMouseResult::Unchanged;
    }

    [[nodiscard]] std::optional<handles::Id> GetHandleId() const override {
        return _id;
    }

private:
    TerrainScreenLocation*                       _loc;
    std::optional<handles::Id>                   _id;
    std::unique_ptr<handles::SpaceMouseSession>  _boxedInner;
};
} // namespace

void TerrainScreenLocation::ApplySpaceMouseElevation(int point,
                                                      float scale,
                                                      const glm::vec3& mov) {
    if (axis_tool != MSLTOOL::TOOL_ELEVATE) return;
    if (point < 0 || point >= (int)mPos.size()) return;

    const float newz = mPos[point] - mov.z * scale;
    mPos[point] = newz;
    if (tool_size > 1) {
        const int row = point / num_points_wide;
        const int col = point % num_points_wide;
        int start_row = row - tool_size + 1;
        int end_row   = row + tool_size - 1;
        int start_col = col - tool_size + 1;
        int end_col   = col + tool_size - 1;
        if (start_row < 0) start_row = 0;
        if (end_row > num_points_deep - 1) end_row = num_points_deep - 1;
        if (start_col < 0) start_col = 0;
        if (end_col > num_points_wide - 1) end_col = num_points_wide - 1;
        for (int j = start_row; j <= end_row; ++j) {
            for (int i = start_col; i <= end_col; ++i) {
                const int abs_point = j * num_points_wide + i;
                mPos[abs_point] = newz;
            }
        }
    }
}

std::unique_ptr<handles::SpaceMouseSession>
TerrainScreenLocation::BeginSpaceMouseSession(const std::optional<handles::Id>& id) {
    return std::make_unique<TerrainSpaceMouseSession>(this, id);
}



void TerrainScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (IsRole(active_handle, handles::Role::Vertex)) {
        axis_tool = MSLTOOL::TOOL_ELEVATE;
    } else {
        if (axis_tool == MSLTOOL::TOOL_ELEVATE) {
            axis_tool = MSLTOOL::TOOL_TRANSLATE;
        }
        ModelScreenLocation::SetAxisTool(mode);
    }
}

void TerrainScreenLocation::AdvanceAxisTool()
{
    if (IsRole(active_handle, handles::Role::Vertex)) {
        axis_tool = MSLTOOL::TOOL_ELEVATE;
    } else {
        ModelScreenLocation::AdvanceAxisTool();
    }
}

void TerrainScreenLocation::SetActiveAxis(MSLAXIS axis)
{
    if (IsRole(active_handle, handles::Role::Vertex)) {
        if (axis != MSLAXIS::NO_AXIS) {
            active_axis = MSLAXIS::Y_AXIS;
        } else {
            active_axis = MSLAXIS::NO_AXIS;
        }
    } else {
        ModelScreenLocation::SetActiveAxis(axis);
    }
}

void TerrainScreenLocation::SetDataFromString(const std::string& point_data)
{
    auto point_array = Split(point_data, ',');
    int i = 0;
    for (int p = 2; p < (int)point_array.size() && i < num_points; ) {
        float val = (float)std::strtof(point_array[p].c_str(), nullptr);
        if (val == 0.0f) {
            p++;
            float num_zeroes = (float)std::strtof(point_array[p].c_str(), nullptr);
            for (int j = 0; j < num_zeroes; ++j) {
                if (i == num_points) {
                    break;
                }
                mPos[i++] = 0.0f;
            }
        } else {
            mPos[i++] = val;
        }
        p++;
    }
    while (i < num_points) mPos[i++] = 0.0f;
}

void TerrainScreenLocation::UpdateSize(int wide, int deep, int points)
{
    num_points_wide = wide;
    num_points_deep = deep;
    num_points = points;
    Init();
}

void TerrainScreenLocation::Init() {
    mPos.resize(num_points);
    mSelectableHandles = num_points + 1;
    for( int i : mPos ) {
        mPos[i] = 0.0f;
    }
}

const std::string TerrainScreenLocation::GetDataAsString() const {
    std::string point_data = "";
    // store the number of points in each axis to allow for smart resizing
    // when grid is altered after terrain points have already been established
    point_data += fmt::format("{:f},{:f},", (float)num_points_wide, (float)num_points_deep);
    int num_zeroes = 0;
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i] != 0) {
            if (num_zeroes > 0) {
                point_data += fmt::format("{:f},{:f},", 0.0f, (float)num_zeroes);
                num_zeroes = 0;
            }
            point_data += fmt::format("{:f}", mPos[i]);
            if (i != num_points - 1) {
                point_data += ",";
            }
        } else {
            num_zeroes++;
            if (i == num_points - 1) {
                point_data += fmt::format("{:f},{:f}", 0.0f, (float)num_zeroes);
            }
        }
    }
    return point_data;
}

// ============================================================
// Terrain TOOL_ELEVATE
// per-grid-point height drag. Tool-size falloff (legacy
// neighbor-blur) skipped in this first cut. CENTER drag
// (translate / scale / rotate) inherits Boxed.
// ============================================================

namespace {

class TerrainElevateSession : public handles::DragSession {
public:
    TerrainElevateSession(TerrainScreenLocation* loc,
                          std::string modelName,
                          handles::Id handleId,
                          const handles::WorldRay& startRay,
                          glm::vec3 activeHandlePos)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _gridIndex(handleId.index),
          _savedActivePos(activeHandlePos),
          _savedHeight(loc->GetGridHeight(handleId.index)) {
        // Plane normal-to-X at savedActivePos.x — vertical YZ
        // plane; the cursor's Y component picks elevation.
        _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
        _planePoint  = glm::vec3(0.0f, 0.0f, _savedActivePos.z);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedActivePos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 dragDelta = cur - _savedIntersect;
        const auto wp = _loc->GetWorldPosition();
        const auto sm = _loc->GetScaleMatrix();
        const float sy = (sm.y == 0.0f) ? 0.001f : sm.y;
        const float newy = (_savedActivePos.y + dragDelta.y - wp.y) / sy;
        _loc->SetGridHeight(_gridIndex, newy);

        // tool-size falloff: when the user has selected
        // a brush larger than 1, set the same height on neighbors
        // within `tool_size - 1` rows/cols of the grabbed point.
        // Mirrors legacy TerrainScreenLocation::MoveHandle3D.
        const int toolSize = _loc->GetToolSize();
        if (toolSize > 1) {
            const int wide = _loc->GetNumPointsWide();
            const int deep = _loc->GetNumPointsDeep();
            if (wide > 0 && deep > 0) {
                if (_savedNeighbors.empty()) {
                    SnapshotNeighbors(toolSize, wide, deep);
                }
                const int row       = _gridIndex / wide;
                const int col       = _gridIndex % wide;
                int start_row = row - toolSize + 1;
                int end_row   = row + toolSize - 1;
                int start_col = col - toolSize + 1;
                int end_col   = col + toolSize - 1;
                if (start_row < 0)             start_row = 0;
                if (end_row > deep - 1)        end_row   = deep - 1;
                if (start_col < 0)             start_col = 0;
                if (end_col > wide - 1)        end_col   = wide - 1;
                for (int j = start_row; j <= end_row; ++j) {
                    for (int i = start_col; i <= end_col; ++i) {
                        const int abs_point = j * wide + i;
                        _loc->SetGridHeight(abs_point, newy);
                    }
                }
            }
        }
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetGridHeight(_gridIndex, _savedHeight);
        for (const auto& [idx, h] : _savedNeighbors) {
            _loc->SetGridHeight(idx, h);
        }
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Vertex : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    void SnapshotNeighbors(int toolSize, int wide, int deep) {
        const int row = _gridIndex / wide;
        const int col = _gridIndex % wide;
        int start_row = row - toolSize + 1;
        int end_row   = row + toolSize - 1;
        int start_col = col - toolSize + 1;
        int end_col   = col + toolSize - 1;
        if (start_row < 0)             start_row = 0;
        if (end_row > deep - 1)        end_row   = deep - 1;
        if (start_col < 0)             start_col = 0;
        if (end_col > wide - 1)        end_col   = wide - 1;
        for (int j = start_row; j <= end_row; ++j) {
            for (int i = start_col; i <= end_col; ++i) {
                const int abs_point = j * wide + i;
                if (abs_point == _gridIndex) continue;
                _savedNeighbors.emplace_back(abs_point, _loc->GetGridHeight(abs_point));
            }
        }
    }

    TerrainScreenLocation* _loc;
    std::string            _modelName;
    handles::Id            _handleId;
    int                    _gridIndex;
    glm::vec3              _savedActivePos;
    float                  _savedHeight;
    glm::vec3              _savedIntersect{0.0f};
    glm::vec3              _planePoint    {0.0f};
    glm::vec3              _planeNormal   {0.0f, 0.0f, 1.0f};
    std::vector<std::pair<int, float>> _savedNeighbors;
    bool                   _changed = false;
};

} // namespace

std::vector<handles::Descriptor> TerrainScreenLocation::GetHandles(
    handles::ViewMode mode, handles::Tool tool,
    const handles::ViewParams& view) const {
    if (mode != handles::ViewMode::ThreeD) {
        return BoxedScreenLocation::GetHandles(mode, tool, view);
    }
    // active_handle == 0 is CENTER (bbox); handle index 1..N
    // identifies a grid point.
    //   - edit_active + Elevate tool: emit grid-point Vertex
    //     descriptors so a click selects a grid point. If a grid
    //     point is already active, also emit the Y-axis arrow at
    //     that point.
    //   - edit_active + non-Elevate tool, or !edit_active: fall
    //     back to Boxed (centre / bbox gizmo).
    if (edit_active && tool == handles::Tool::Elevate) {
        std::vector<handles::Descriptor> out;
        out.reserve(num_points + 1);
        const float x_offset = (num_points_wide - 1) * spacing / 2.0f;
        const float z_offset = (num_points_deep - 1) * spacing / 2.0f;
        glm::vec3 activePointWorld(0.0f);
        bool activePointFound = false;
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                const int abs_point = j * num_points_wide + i;
                float sx = i * spacing - x_offset;
                float sy = mPos[abs_point];
                float sz = j * spacing - z_offset;
                TranslatePoint(sx, sy, sz);
                if (IsHandle(active_handle, handles::Role::Vertex, abs_point)) {
                    activePointWorld = glm::vec3(sx, sy, sz);
                    activePointFound = true;
                }
                handles::Descriptor d;
                d.id.role  = handles::Role::Vertex;
                d.id.index = abs_point;
                d.worldPos = glm::vec3(sx, sy, sz);
                d.suggestedRadius = 5.0f;
                d.editable = !IsLocked();
                d.selectionOnly = true;
                out.push_back(d);
            }
        }
        if (activePointFound && IsRole(active_handle, handles::Role::Vertex) && IsElevationHandle()) {
            const float kArrowLen = view.axisArrowLength;
            const float kHitOffset = kArrowLen - view.axisHeadLength * 0.5f;
            handles::Descriptor d;
            d.id.role  = handles::Role::AxisArrow;
            d.id.axis  = handles::Axis::Y;
            d.id.index = active_handle->index;
            d.worldPos = activePointWorld + glm::vec3(0.0f, kHitOffset, 0.0f);
            d.suggestedRadius = view.axisRadius;
            d.editable = !IsLocked();
            out.push_back(d);
        }
        return out;
    }
    return BoxedScreenLocation::GetHandles(mode, tool, view);
}

std::unique_ptr<handles::DragSession> TerrainScreenLocation::CreateDragSession(
    const std::string& modelName,
    const handles::Id& id,
    const handles::WorldRay& startRay) {
    if (_locked) return nullptr;
    if (id.role == handles::Role::AxisArrow && id.axis == handles::Axis::Y &&
        IsRole(active_handle, handles::Role::Vertex) && IsElevationHandle()) {
        if (id.index < 0 || id.index >= num_points) return nullptr;
        return std::make_unique<TerrainElevateSession>(this, modelName, id, startRay,
                                                        GetActiveHandlePosition());
    }
    return BoxedScreenLocation::CreateDragSession(modelName, id, startRay);
}
