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
#include <cmath>
#include <spdlog/fmt/fmt.h>
#include "PolyPointScreenLocation.h"


#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "Shapes.h"
#include "../utils/VectorMath.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "../utils/string_utils.h"
#include "RulerObject.h"

#include <log.h>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);

static float BB_OFF = 5.0f;

static glm::mat4 Identity(glm::mat4(1.0f));

// `id.index` sentinel on AxisArrow/Cube/Ring descriptors emitted
// while the centre handle is active. Per-vertex / per-curve-control
// axis gizmos carry their owning sub-handle's index instead.
// CreateDragSession dispatches on this value to choose between the
// per-handle and the whole-model session classes.
static constexpr int kWholeModelAxisIndex = -1;

// Substitute a tiny non-zero value for a scale factor of exactly
// zero, so downstream divisions don't produce NaN/Inf. Vertices /
// curve CPs collapse onto the same world point at scale 0 anyway;
// the 0.001 floor matches the legacy MoveHandle3D behavior.
static inline float SafeScale(float s) { return s == 0.0f ? 0.001f : s; }

PolyPointScreenLocation::PolyPointScreenLocation() : ModelScreenLocation(2),
   num_points(2), selected_segment(-1) {
    mPos.resize(2);
    mPos[0].x = 0.0f;
    mPos[0].y = 0.0f;
    mPos[0].z = 0.0f;
    mPos[0].length = 0.0f;
    mPos[0].matrix = nullptr;
    mPos[0].mod_matrix = nullptr;
    mPos[0].curve = nullptr;
    mPos[0].has_curve = false;
    mPos[1].x = 0.0f;
    mPos[1].y = 0.0f;
    mPos[1].z = 0.0f;
    mPos[1].length = 0.0f;
    mPos[1].matrix = nullptr;
    mPos[1].mod_matrix = nullptr;
    mPos[1].curve = nullptr;
    mPos[1].has_curve = false;
    seg_aabb_min.resize(1);
    seg_aabb_max.resize(1);
    mSelectableHandles = 3;
}

PolyPointScreenLocation::~PolyPointScreenLocation() {
    for( int i = 0; i < (int)mPos.size(); ++i ) {
        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        if (mPos[i].mod_matrix != nullptr) {
            delete mPos[i].mod_matrix;
        }
        if (mPos[i].curve != nullptr) {
            delete mPos[i].curve;
        }
    }
    mPos.clear();
}

void PolyPointScreenLocation::SetCurve(int seg_num, bool create)
{

    if (_locked) return;
    if (seg_num < 0 || seg_num >= (int)mPos.size() - 1) return;

    if (create) {
        mPos[seg_num].has_curve = true;
        if (mPos[seg_num].curve == nullptr) {
            mPos[seg_num].curve = new BezierCurveCubic3D();
        }
        mPos[seg_num].curve->set_p0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
        mPos[seg_num].curve->set_p1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
        mPos[seg_num].curve->set_cp0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
        mPos[seg_num].curve->set_cp1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
    }
    else {
        mPos[seg_num].has_curve = false;
        if (mPos[seg_num].curve != nullptr) {
            delete mPos[seg_num].curve;
            mPos[seg_num].curve = nullptr;
        }
    }
}

void PolyPointScreenLocation::Init()
{
    if (std::isnan(worldPos_x)) worldPos_x = 0.0;
    if (std::isnan(worldPos_y)) worldPos_y = 0.0;
    if (std::isnan(worldPos_z)) worldPos_z = 0.0;

    if (scalex <= 0 || std::isinf(scalex) || std::isnan(scalex)) {
        scalex = 1.0f;
    }
    if (scaley <= 0 || std::isinf(scaley) || std::isnan(scaley)) {
        scaley = 1.0f;
    }
    if (scalez <= 0 || std::isinf(scalez) || std::isnan(scalez)) {
        scalez = 1.0f;
    }

    mSelectableHandles = num_points + 1;
    seg_aabb_min.resize(num_points - 1);
    seg_aabb_max.resize(num_points - 1);
}

void PolyPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const
{
    std::unique_lock<std::mutex> locker(_mutex);
    minX = 100000.0;
    minY = 100000.0;
    minZ = 100000.0;
    maxX = 0.0;
    maxY = 0.0;
    maxZ = 0.0;

    glm::vec3 scaling(scalex, scaley, scalez);
    glm::vec3 world_pos(worldPos_x, worldPos_y, worldPos_z);

    for (int i = 0; i < num_points - 1; ++i) {
        float x1p = mPos[i].x * scalex + worldPos_x;
        float x2p = mPos[i + 1].x * scalex + worldPos_x;
        float y1p = mPos[i].y * scaley + worldPos_y;
        float y2p = mPos[i + 1].y * scaley + worldPos_y;
        float z1p = mPos[i].z * scalez + worldPos_z;
        float z2p = mPos[i + 1].z * scalez + worldPos_z;

        if (!is_3d) {
            // allows 2D selection to work
            z1p = 0.0f;
            z2p = 0.0f;
        }

        if (mPos[i].x < minX) minX = mPos[i].x;
        if (mPos[i].y < minY) minY = mPos[i].y;
        if (mPos[i].z < minZ) minZ = mPos[i].z;
        if (mPos[i].x > maxX) maxX = mPos[i].x;
        if (mPos[i].y > maxY) maxY = mPos[i].y;
        if (mPos[i].z > maxZ) maxZ = mPos[i].z;

        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }

        if (i == num_points - 2) {
            if (mPos[i + 1].x < minX) minX = mPos[i + 1].x;
            if (mPos[i + 1].y < minY) minY = mPos[i + 1].y;
            if (mPos[i + 1].z < minZ) minZ = mPos[i + 1].z;
            if (mPos[i + 1].x > maxX) maxX = mPos[i + 1].x;
            if (mPos[i + 1].y > maxY) maxY = mPos[i + 1].y;
            if (mPos[i + 1].z > maxZ) maxZ = mPos[i + 1].z;
        }

        if (x1p == x2p && y1p == y2p && z1p == z2p) {
            x2p += 0.001f;
        }
        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);
        glm::vec3 a = pt2 - pt1;
        float scale = glm::length(a) / RenderWi;
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scale, 1.0f, 1.0f));
        glm::mat4 translateMatrix = glm::translate(Identity, glm::vec3(x1p, y1p, z1p));
        glm::mat4 mat = translateMatrix * rotationMatrix * scalingMatrix;

        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        mPos[i].matrix = new glm::mat4(mat);

        if (allow_selected) {
            glm::mat4 mod_mat = translateMatrix * rotationMatrix;
            if (mPos[i].mod_matrix != nullptr) {
                delete mPos[i].mod_matrix;
            }
            mPos[i].mod_matrix = new glm::mat4(mod_mat);
            mPos[i].seg_scale = scale;
        }

        // update curve points
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->SetPositioning(scaling, world_pos);
            mPos[i].curve->UpdatePoints();
        }
    }
    float yscale = (maxY - minY) * scaley;
    if (RenderHt > 1.0f && (maxY - minY < RenderHt)) {
        yscale = RenderHt;
    }
    glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3((maxX - minX) * scalex, yscale, (maxZ - minZ) * scalez));
    TranslateMatrix = glm::translate(Identity, glm::vec3(minX * scalex + worldPos_x, minY * scaley + worldPos_y, minZ * scalez + worldPos_z));
    main_matrix = TranslateMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        ModelMatrix = TranslateMatrix;
    }

    draw_3d = is_3d;
}

void PolyPointScreenLocation::TranslatePoint(float &x, float &y, float &z) const {
    glm::vec3 v = glm::vec3(main_matrix * glm::vec4(x, y, z, 1));
    x = v.x;
    y = v.y;
    z = v.z;
}
void PolyPointScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->SetModelMatrix(main_matrix);
}

bool PolyPointScreenLocation::IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const
{
    int sx1 = std::min(x1, x2);
    int sx2 = std::max(x1, x2);
    int sy1 = std::min(y1, y2);
    int sy2 = std::max(y1, y2);
    float x1p = minX * scalex + worldPos_x;
    float x2p = maxX * scalex + worldPos_x;
    float y1p = minY * scaley + worldPos_y;
    float y2p = maxY * scaley + worldPos_y;

    if (draw_3d) {
        glm::vec3 aabb_min_pp(x1p, y1p, 0.0f);
        glm::vec3 aabb_max_pp(x2p, y2p, 0.0f);
        return VectorMath::TestVolumeOBBIntersection(
            sx1, sy1, sx2, sy2,
            preview->getWidth(),
            preview->getHeight(),
            aabb_min_pp, aabb_max_pp,
            preview->GetProjViewMatrix(),
            Identity);
    }
    else {
        if (x1p >= sx1 && x1p <= sx2 &&
            x2p >= sx1 && x2p <= sx2 &&
            y1p >= sy1 && y1p <= sy2 &&
            y2p >= sy1 && y2p <= sy2) {
            return true;
        }
    }
    return false;
}

bool PolyPointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const
{
    bool ret_value = false;

    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            if (mPos[i].curve->HitTest(ray_origin)) {
                ret_value = true;
                break;
            }
        }
        else {
            if (mPos[i].mod_matrix != nullptr) {
                if (VectorMath::TestRayOBBIntersection2D(
                    ray_origin,
                    seg_aabb_min[i],
                    seg_aabb_max[i],
                    *mPos[i].mod_matrix)
                    ) {
                    ret_value = true;
                    break;
                }
            }
        }
    }
    // check if inside boundary handles
    float sx1 = (ray_origin.x - worldPos_x) / scalex;
    float sy1 = (ray_origin.y - worldPos_y) / scaley;
    if (sx1 >= minX && sx1 <= maxX && sy1 >= minY && sy1 <= maxY) {
        ret_value = true;
    }
    return ret_value;
}

bool PolyPointScreenLocation::HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const
{
    float distance = 1000000000.0f;
    bool ret_value = false;

    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            if (mPos[i].curve->HitTest3D(ray_origin, ray_direction, distance)) {
                if (distance < intersection_distance) {
                    intersection_distance = distance;
                }
                ret_value = true;
            }
        }
        else {
            if (mPos[i].mod_matrix == nullptr) {
                continue;
            }

            if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                seg_aabb_min[i],
                seg_aabb_max[i],
                *mPos[i].mod_matrix,
                distance)
                ) {
                if (distance < intersection_distance) {
                    intersection_distance = distance;
                }
                ret_value = true;
            }
        }
    }

    return ret_value;
}


void PolyPointScreenLocation::SetSelectedHandle(int h) {
    if (h == NO_HANDLE) {
        selected_handle.reset();
        return;
    }
    handles::Id id;
    if (h == CENTER_HANDLE) {
        id.role = handles::Role::CentreCycle;
    } else if (h & HANDLE_CP0) {
        id.role = handles::Role::CurveControl;
        id.index = 0;
        id.segment = h & HANDLE_MASK;
    } else if (h & HANDLE_CP1) {
        id.role = handles::Role::CurveControl;
        id.index = 1;
        id.segment = h & HANDLE_MASK;
    } else {
        // Vertex: int handle is 1-based, descriptor index is 0-based.
        id.role = handles::Role::Vertex;
        id.index = h - 1;
    }
    selected_handle = id;
}

void PolyPointScreenLocation::SetActiveHandle(const std::optional<handles::Id>& id)
{
    // Axis-gizmo roles are modifiers on the body handle — see the
    // base default comment for why we ignore them here.
    if (id && (id->role == handles::Role::AxisArrow ||
               id->role == handles::Role::AxisCube ||
               id->role == handles::Role::AxisRing)) {
        return;
    }
    // selected_handle and active_handle must stay in lockstep, and we
    // can set both Id directly without lossy int round-tripping.
    active_handle = id;
    selected_handle = id;
    highlighted_handle.reset();
    SetAxisTool(axis_tool);
}

void PolyPointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (IsRole(active_handle, handles::Role::CentreCycle)) {
        ModelScreenLocation::SetAxisTool(mode);
    }
    else {
        if (mode == MSLTOOL::TOOL_TRANSLATE || mode == MSLTOOL::TOOL_XY_TRANS) {
            axis_tool = mode;
        }
        else {
            axis_tool = MSLTOOL::TOOL_TRANSLATE;
        }
    }
}

void PolyPointScreenLocation::AdvanceAxisTool()
{
    if (IsRole(active_handle, handles::Role::CentreCycle)) {
        ModelScreenLocation::AdvanceAxisTool();
    }
    else {
        axis_tool = MSLTOOL::TOOL_TRANSLATE;
    }
}

void PolyPointScreenLocation::SetActiveAxis(MSLAXIS axis)
{
   ModelScreenLocation::SetActiveAxis(axis);
}
bool PolyPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    std::unique_lock<std::mutex> locker(_mutex);

    if (active_handle.has_value() || mouse_down) {
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        vac->PreAlloc(10 * num_points + 12);
        xlColor h1c, h2c, h3c;
        if (fromBase)
        {
            h1c = FROM_BASE_HANDLES_COLOUR;
            h2c = FROM_BASE_HANDLES_COLOUR;
            h3c = FROM_BASE_HANDLES_COLOUR;
        } else
        if (_locked) {
            h1c = LOCKED_HANDLES_COLOUR;
            h2c = LOCKED_HANDLES_COLOUR;
            h3c = LOCKED_HANDLES_COLOUR;
        } else {
            h1c = IsHandle(highlighted_handle, handles::Role::Vertex, 0) ? xlYELLOWTRANSLUCENT : xlGREENTRANSLUCENT;
            h2c = xlBLUETRANSLUCENT;
            h3c = IsRole(highlighted_handle, handles::Role::CentreCycle) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        // Centroid — formula matches `GetHcenterPos/Vcenter/Dcenter`,
        // so the drawn position is identical to what GetHandles()
        // emits for the CentreCycle descriptor.
        const float cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
        const float cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
        const float cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;
        const float hw = GetRectHandleWidth(zoom, scale);
        vac->AddSphereAsTriangles(cx, cy, cz, hw, h3c);
        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;

        for (int i = 0; i < num_points - 1; ++i) {
            int x1_pos = mPos[i].x * scalex + worldPos_x;
            int x2_pos = mPos[i + 1].x * scalex + worldPos_x;
            int y1_pos = mPos[i].y * scaley + worldPos_y;
            int y2_pos = mPos[i + 1].y * scaley + worldPos_y;
            int z1_pos = mPos[i].z * scalez + worldPos_z;
            int z2_pos = mPos[i + 1].z * scalez + worldPos_z;

            if (i == selected_segment) {
                count = vac->getCount();
                if (count != startVertex) {
                    program->addStep([=](xlGraphicsContext *ctx) {
                        ctx->drawTriangles(vac, startVertex, count - startVertex);
                    });
                    startVertex = count;
                }
                
                if (!mPos[i].has_curve || mPos[i].curve == nullptr) {
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
                } else {
                    // draw bezier curve
                    x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                    for (int x = 1; x < mPos[i].curve->GetNumPoints(); ++x) {
                        x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                        y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                        z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                        vac->AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                        vac->AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
                        x1_pos = x2_pos;
                        y1_pos = y2_pos;
                        z1_pos = z2_pos;
                    }
                    // draw control lines
                    x1_pos = mPos[i].curve->get_p0x() * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_p0y() * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_p0z() * scalez + worldPos_z;
                    x2_pos = mPos[i].curve->get_cp0x() * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_cp0y() * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_cp0z() * scalez + worldPos_z;
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                    x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                    x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                }
                count = vac->getCount();
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }

            // add handle for start of this vector
            float sx = mPos[i].x * scalex + worldPos_x - hw / 2;
            float sy = mPos[i].y * scaley + worldPos_y - hw / 2;
            float sz = mPos[i].z * scalez + worldPos_z - hw / 2;
            vac->AddSphereAsTriangles(sx, sy, sz, hw, i == 0 ? h1c : (IsHandle(highlighted_handle, handles::Role::Vertex, i) ? xlYELLOW : h2c));

            // add final handle
            if (i == num_points - 2) {
                sx = mPos[i + 1].x * scalex + worldPos_x - hw / 2;
                sy = mPos[i + 1].y * scaley + worldPos_y - hw / 2;
                sz = mPos[i + 1].z * scalez + worldPos_z - hw / 2;
                vac->AddSphereAsTriangles(sx, sy, sz, hw, IsHandle(highlighted_handle, handles::Role::Vertex, i + 1) ? xlYELLOW : h2c);
            }
        }

        glm::vec3 cp_handle_pos[2];
        if (selected_segment != -1) {
            // add control point handles for selected segments
            int i = selected_segment;
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                float cxx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - hw / 2;
                float cyy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - hw / 2;
                float czz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - hw / 2;
                h2c = IsHandle(highlighted_handle, handles::Role::CurveControl, 0, i) ? xlYELLOW : xlRED;
                vac->AddSphereAsTriangles(cxx, cyy, czz, hw, h2c);
                mPos[i].cp0.x = mPos[i].curve->get_cp0x();
                mPos[i].cp0.y = mPos[i].curve->get_cp0y();
                mPos[i].cp0.z = mPos[i].curve->get_cp0z();
                cp_handle_pos[0].x = cxx;
                cp_handle_pos[0].y = cyy;
                cp_handle_pos[0].z = czz;
                cxx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - hw / 2;
                cyy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - hw / 2;
                czz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - hw / 2;
                h2c = IsHandle(highlighted_handle, handles::Role::CurveControl, 1, i) ? xlYELLOW : xlRED;
                vac->AddSphereAsTriangles(cxx, cyy, czz, hw, h2c);
                mPos[i].cp1.x = mPos[i].curve->get_cp1x();
                mPos[i].cp1.y = mPos[i].curve->get_cp1y();
                mPos[i].cp1.z = mPos[i].curve->get_cp1z();
                cp_handle_pos[1].x = cxx;
                cp_handle_pos[1].y = cyy;
                cp_handle_pos[1].z = czz;
            }
        }
        count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;

        if (!_locked) {
            glm::vec3 active_handle_pos(0.0f);
            if (IsHandle(active_handle, handles::Role::CurveControl, 0)) {
                active_handle_pos = cp_handle_pos[0];
            }
            else if (IsHandle(active_handle, handles::Role::CurveControl, 1)) {
                active_handle_pos = cp_handle_pos[1];
            }
            else if (IsRole(active_handle, handles::Role::CentreCycle)) {
                active_handle_pos = glm::vec3(cx, cy, cz);
            }
            else if (IsRole(active_handle, handles::Role::Vertex)) {
                // Vertex handle: descriptor index is 0-based. Position
                // mirrors the draw offset (`- hw/2`) so the gizmo lines
                // through the visible handle the same way the old cache
                // did.
                const int vIdx = active_handle->index;
                if (vIdx >= 0 && vIdx < num_points) {
                    active_handle_pos = glm::vec3(
                        mPos[vIdx].x * scalex + worldPos_x - hw / 2,
                        mPos[vIdx].y * scaley + worldPos_y - hw / 2,
                        mPos[vIdx].z * scalez + worldPos_z - hw / 2);
                }
            }
            DrawAxisTool(active_handle_pos, program, zoom, scale);
            if (axis_tool == MSLTOOL::TOOL_XY_TRANS && active_axis != MSLAXIS::NO_AXIS) {
                // XY_TRANS draws a planar cross (X + Y lines) rather than
                // a single-axis indicator. Inline; the base helper handles
                // the simpler single-axis case. Snapshot AFTER
                // DrawAxisTool so the addStep below covers only the
                // 4 cross vertices, not the gizmo triangles.
                int xyStart = vac->getCount();
                vac->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                vac->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                vac->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                vac->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                int xyEnd = vac->getCount();
                program->addStep([xyStart, xyEnd, program, vac](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, xyStart, xyEnd - xyStart);
                });
            } else {
                DrawActiveAxisIndicator(active_handle_pos, program);
            }
        }
    } else if (drawBounding) {
        // draw bounding box for each segment if model is highlighted
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        xlColor Box3dColor = xlWHITETRANSLUCENT;
        if (fromBase)
            Box3dColor = FROM_BASE_HANDLES_COLOUR;
        else if (_locked)
            Box3dColor = LOCKED_HANDLES_COLOUR;
        for (int i = 0; i < num_points - 1; ++i) {
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                mPos[i].curve->DrawBoundingBoxes(Box3dColor, vac);
            } else {
                DrawBoundingBoxLines(Box3dColor, seg_aabb_min[i], seg_aabb_max[i], *mPos[i].mod_matrix, *vac);
            }
        }
        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, count - startVertex);
        });
    }
    return true;
}

bool PolyPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    std::unique_lock<std::mutex> locker(_mutex);
    
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();

    vac->PreAlloc(10 * num_points + 12);

    // add boundary handles
    float hw = GetRectHandleWidth(zoom, scale);
    float boundary_offset = 2.0f * hw;
    float x1 = minX * scalex + worldPos_x - hw / 2 - boundary_offset;
    float y1 = minY * scaley + worldPos_y - hw / 2 - boundary_offset;
    float x2 = maxX * scalex + worldPos_x + hw / 2 + boundary_offset;
    float y2 = maxY * scaley + worldPos_y + hw / 2 + boundary_offset;
    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    vac->AddRectAsTriangles(x1, y1, x1 + hw, y1 + hw, handleColor);
    vac->AddRectAsTriangles(x1, y2, x1 + hw, y2 + hw, handleColor);
    vac->AddRectAsTriangles(x2, y1, x2 + hw, y1 + hw, handleColor);
    vac->AddRectAsTriangles(x2, y2, x2 + hw, y2 + hw, handleColor);


    for (int i = 0; i < num_points - 1; ++i) {
        int x1_pos = mPos[i].x * scalex + worldPos_x;
        int x2_pos = mPos[i + 1].x * scalex + worldPos_x;
        int y1_pos = mPos[i].y * scaley + worldPos_y;
        int y2_pos = mPos[i + 1].y * scaley + worldPos_y;
        [[maybe_unused]] int z1_pos = mPos[i].z * scalez + worldPos_z;
        int z2_pos = mPos[i + 1].z * scalez + worldPos_z;

        if (i == selected_segment) {
            int count = vac->getCount();
            if (count != startVertex) {
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawTriangles(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }
            if (!mPos[i].has_curve || mPos[i].curve == nullptr) {
                vac->AddVertex(x1_pos, y1_pos, xlMAGENTA);
                vac->AddVertex(x2_pos, y2_pos, xlMAGENTA);
            } else {
                // draw bezier curve
                x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                for (int x = 1; x < mPos[i].curve->GetNumPoints(); ++x) {
                    x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                    vac->AddVertex(x1_pos, y1_pos, xlMAGENTA);
                    vac->AddVertex(x2_pos, y2_pos, xlMAGENTA);
                    x1_pos = x2_pos;
                    y1_pos = y2_pos;
                    z1_pos = z2_pos;
                }
                // draw control lines
                x1_pos = mPos[i].curve->get_p0x() * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_p0y() * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_p0z() * scalez + worldPos_z;
                x2_pos = mPos[i].curve->get_cp0x() * scalex + worldPos_x;
                y2_pos = mPos[i].curve->get_cp0y() * scaley + worldPos_y;
                z2_pos = mPos[i].curve->get_cp0z() * scalez + worldPos_z;
                vac->AddVertex(x1_pos, y1_pos, xlRED);
                vac->AddVertex(x2_pos, y2_pos, xlRED);
                x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                vac->AddVertex(x1_pos, y1_pos, xlRED);
                vac->AddVertex(x2_pos, y2_pos, xlRED);
            }
            count = vac->getCount();
            if (count != startVertex) {
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }
        }

        // add handle for start of this vector
        float sx = mPos[i].x * scalex + worldPos_x - hw / 2;
        float sy = mPos[i].y * scaley + worldPos_y - hw / 2;
        float sz = mPos[i].z * scalez + worldPos_z - hw / 2;
        (void)sz;
        vac->AddRectAsTriangles(sx, sy, sx + hw, sy + hw, IsHandle(selected_handle, handles::Role::Vertex, i) ? xlMAGENTATRANSLUCENT : (i == 0 ? xlGREENTRANSLUCENT : handleColor));

        // add final handle
        if (i == num_points - 2) {
            sx = mPos[i + 1].x * scalex + worldPos_x - hw / 2;
            sy = mPos[i + 1].y * scaley + worldPos_y - hw / 2;
            sz = mPos[i + 1].z * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(sx, sy, sx + hw, sy + hw, IsHandle(selected_handle, handles::Role::Vertex, i + 1) ? xlMAGENTATRANSLUCENT : handleColor);
        }
    }

    if (selected_segment != -1) {
        // add control point handles for selected segments
        int i = selected_segment;
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            float cx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - hw / 2;
            float cy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - hw / 2;
            [[maybe_unused]] float cz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(cx, cy, cx + hw, cy + hw, xlREDTRANSLUCENT);
            mPos[i].cp0.x = mPos[i].curve->get_cp0x();
            mPos[i].cp0.y = mPos[i].curve->get_cp0y();
            mPos[i].cp0.z = mPos[i].curve->get_cp0z();
            cx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - hw / 2;
            cy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - hw / 2;
            cz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(cx, cy, cx + hw, cy + hw, xlREDTRANSLUCENT);
            mPos[i].cp1.x = mPos[i].curve->get_cp1x();
            mPos[i].cp1.y = mPos[i].curve->get_cp1y();
            mPos[i].cp1.z = mPos[i].curve->get_cp1z();
        }
    }
    int count = vac->getCount();
    if (count != startVertex) {
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;
    }
    return true;
}


namespace {
// SpaceMouse session for PolyPointScreenLocation. Handle dispatch:
//   - CentreCycle / no id   → rotate + translate whole model
//   - CurveControl(seg, ix) → move that curve control point
//   - Vertex(idx)           → move that vertex
class PolyPointSpaceMouseSession : public handles::SpaceMouseSession {
public:
    PolyPointSpaceMouseSession(PolyPointScreenLocation* loc,
                               std::optional<handles::Id> id)
        : _loc(loc), _id(id) {}

    handles::SpaceMouseResult Apply(float scale,
                                     const glm::vec3& rot,
                                     const glm::vec3& mov) override {
        if (!_loc) return handles::SpaceMouseResult::Unchanged;
        if (_id.has_value()) {
            if (_id->role == handles::Role::CurveControl) {
                _loc->ApplySpaceMouseCurveCp(_id->segment, _id->index, scale, mov);
                return handles::SpaceMouseResult::NeedsInit;
            }
            if (_id->role == handles::Role::Vertex) {
                _loc->ApplySpaceMouseVertex(_id->index, scale, mov);
                return handles::SpaceMouseResult::NeedsInit;
            }
        }
        _loc->ApplySpaceMouseCenter(scale, rot, mov);
        return handles::SpaceMouseResult::Dirty;
    }

    [[nodiscard]] std::optional<handles::Id> GetHandleId() const override {
        return _id;
    }

private:
    PolyPointScreenLocation*    _loc;
    std::optional<handles::Id>  _id;
};
} // namespace

void PolyPointScreenLocation::ApplySpaceMouseCenter(float scale,
                                                     const glm::vec3& rot,
                                                     const glm::vec3& mov) {
    constexpr float rscale = 10.0f;
    Rotate(ModelScreenLocation::MSLAXIS::X_AXIS,  rot.x * rscale);
    Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rot.z * rscale);
    Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS,  rot.y * rscale);
    AddOffset(mov.x * scale, -mov.z * scale, mov.y * scale);
}

void PolyPointScreenLocation::ApplySpaceMouseCurveCp(int segment,
                                                      int cpIndex,
                                                      float scale,
                                                      const glm::vec3& mov) {
    if (segment < 0 || segment >= (int)mPos.size()) return;
    if (cpIndex == 0) {
        mPos[segment].cp0.x += mov.x * scale;
        mPos[segment].cp0.y -= mov.z * scale;
        mPos[segment].cp0.z += mov.y * scale;
        if (mPos[segment].curve != nullptr) {
            mPos[segment].curve->set_cp0(mPos[segment].cp0.x,
                                          mPos[segment].cp0.y,
                                          mPos[segment].cp0.z);
        }
    } else {
        mPos[segment].cp1.x += mov.x * scale;
        mPos[segment].cp1.y -= mov.z * scale;
        mPos[segment].cp1.z += mov.y * scale;
        if (mPos[segment].curve != nullptr) {
            mPos[segment].curve->set_cp1(mPos[segment].cp1.x,
                                          mPos[segment].cp1.y,
                                          mPos[segment].cp1.z);
        }
    }
    FixCurveHandles();
}

void PolyPointScreenLocation::ApplySpaceMouseVertex(int vertexIndex,
                                                     float scale,
                                                     const glm::vec3& mov) {
    if (vertexIndex < 0 || vertexIndex >= (int)mPos.size()) return;
    mPos[vertexIndex].x += mov.x * scale;
    mPos[vertexIndex].y -= mov.z * scale;
    mPos[vertexIndex].z += mov.y * scale;
    FixCurveHandles();
}

std::unique_ptr<handles::SpaceMouseSession>
PolyPointScreenLocation::BeginSpaceMouseSession(const std::optional<handles::Id>& id) {
    return std::make_unique<PolyPointSpaceMouseSession>(this, id);
}


void PolyPointScreenLocation::SelectHandle(int handle) {
    SetSelectedHandle(handle);
    if( handle != -1 && handle < HANDLE_CP0) {
        selected_segment = -1;
    }
}

void PolyPointScreenLocation::SelectSegment(int segment) {
    selected_segment = segment;
    if( segment != -1 ) {
        selected_handle.reset();
    }
}

void PolyPointScreenLocation::AddHandle(IModelPreview* preview, int mouseX, int mouseY) {
    std::unique_lock<std::mutex> locker(_mutex);

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    xlPolyPoint new_point;
    new_point.x = (ray_origin.x - worldPos_x) / scalex;
    new_point.y = (ray_origin.y - worldPos_y) / scaley;
    new_point.z = 0.0f;
    new_point.length = 0.0f;

    if (draw_3d) {
        // use drag handle function to find plane intersection
        glm::vec3 intersect(0.0f);
        const glm::vec3 planePoint(worldPos_x, worldPos_y, worldPos_z);
        DragHandle(preview, mouseX, mouseY, intersect, planePoint);
        new_point.x = (intersect.x - worldPos_x) / scalex;
        new_point.y = (intersect.y - worldPos_y) / scaley;
    }

    new_point.matrix = nullptr;
    new_point.mod_matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    new_point.seg_scale = 1.0f;
    mPos.push_back(new_point);
    num_points++;
    mSelectableHandles++;
    seg_aabb_min.resize(num_points - 1);
    seg_aabb_max.resize(num_points - 1);

}

void PolyPointScreenLocation::InsertHandle(int after_handle, float zoom, int scale) {
    std::unique_lock<std::mutex> locker(_mutex);

    int pos = after_handle;
    if (pos < 0 || pos >= (int)mPos.size() - 1) return;
    float x1_pos = mPos[pos].x;
    float x2_pos = mPos[pos+1].x;
    float y1_pos = mPos[pos].y;
    float y2_pos = mPos[pos+1].y;
    float z1_pos = mPos[pos].z;
    float z2_pos = mPos[pos+1].z;
    xlPolyPoint new_point;
    new_point.x = (x1_pos+x2_pos)/2.0;
    new_point.y = (y1_pos+y2_pos)/2.0;
    new_point.z = (z1_pos+z2_pos)/2.0;
    new_point.length = mPos[pos].length/2.0;
    new_point.matrix = nullptr;
    new_point.mod_matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    mPos.insert(mPos.begin() + pos + 1, new_point);
    mPos[pos].length = new_point.length;
    num_points++;
    SetSelectedHandle(after_handle + 1);
    selected_segment = -1;
    mSelectableHandles++;
    seg_aabb_min.resize(num_points - 1);
    seg_aabb_max.resize(num_points - 1);

}

void PolyPointScreenLocation::DeleteHandle(int handle) {
    
    // this can happen if you click one one of the box handles
    if (handle >= (int)mPos.size()) return;

    // delete any curves associated with this handle
    if( mPos[handle].has_curve ) {
        mPos[handle].has_curve = false;
        if( mPos[handle].curve != nullptr ) {
            delete mPos[handle].curve;
            mPos[handle].curve = nullptr;
        }
    }
    if(handle > 0 ) {
        if( mPos[handle-1].has_curve ) {
            mPos[handle-1].has_curve = false;
            if( mPos[handle-1].curve != nullptr ) {
                delete mPos[handle-1].curve;
                mPos[handle-1].curve = nullptr;
            }
        }
    }

    // now delete the handle
    mPos.erase(mPos.begin() + handle);
    num_points--;
    selected_handle.reset();
    selected_segment = -1;
}

CursorType PolyPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) {
    if (preview != nullptr) {
        active_axis = MSLAXIS::X_AXIS;
        glm::vec3 intersect(0.0f);
        const glm::vec3 planePoint(worldPos_x, worldPos_y, worldPos_z);
        DragHandle(preview, x, y, intersect, planePoint);
        worldPos_x = intersect.x;
        worldPos_y = intersect.y;
        worldPos_z = 0.0f;
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane. Legacy END_HANDLE
            // here means "place vertex 1 as active" (the second polyline
            // vertex; index 0-based = 1).
            handles::Id endId;
            endId.role = handles::Role::Vertex;
            endId.index = 1;
            active_handle = endId;
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }

    mPos[0].x = 0.0f;
    mPos[0].y = 0.0f;
    mPos[0].z = 0.0f;
    mPos[1].x = 0.0f;
    mPos[1].y = 0.0f;
    mPos[1].z = 0.0f;

    handle = 2;
    return CursorType::Sizing;
}

std::string PolyPointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    float len = 0;
    auto last = mPos[0].AsVector();
    for (int i = 1; i < (int)mPos.size(); i++) {
        len += RulerObject::Measure(last, mPos[i].AsVector());
        last = mPos[i].AsVector();
    }
    return fmt::format("Length {}", RulerObject::PrescaledMeasureDescription(len));
}

void PolyPointScreenLocation::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (_locked) return;
    
    rotate_pt = position;
     if (angle.y != 0.0f) {
        float offset = angle.y;
         Rotate(MSLAXIS::Y_AXIS, offset);
    }
    else if (angle.x != 0.0f) {
        float offset = angle.x;
        Rotate(MSLAXIS::X_AXIS, offset);
    }
    else if (angle.z != 0.0f) {
        float offset = angle.z;
        Rotate(MSLAXIS::Z_AXIS, offset);
    }
}

bool PolyPointScreenLocation::Rotate(MSLAXIS axis, float factor)
{
    if (_locked) return false;

    // Rotate all the points
    glm::mat4 translateToOrigin = glm::translate(Identity, -rotate_pt);
    glm::mat4 translateBack = glm::translate(Identity, rotate_pt);
    glm::mat4 Rotate = Identity;
    glm::vec3 pt(worldPos_x, worldPos_y, worldPos_z);

    switch (axis)
    {
    case MSLAXIS::X_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    break;
    case MSLAXIS::Y_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    break;
    case MSLAXIS::Z_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    break;
    default:
        break;
    }

    pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
    glm::vec3 world_new(pt.x, pt.y, pt.z);
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            pt = glm::vec3(mPos[i].curve->get_cp0x() * scalex + worldPos_x, mPos[i].curve->get_cp0y() * scaley + worldPos_y, mPos[i].curve->get_cp0z() * scalez + worldPos_z);
            pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
            mPos[i].curve->set_cp0((pt.x - world_new.x) / scalex, (pt.y - world_new.y) / scaley, (pt.z - world_new.z) / scalez);
            pt = glm::vec3(mPos[i].curve->get_cp1x() * scalex + worldPos_x, mPos[i].curve->get_cp1y() * scaley + worldPos_y, mPos[i].curve->get_cp1z() * scalez + worldPos_z);
            pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
            mPos[i].curve->set_cp1((pt.x - world_new.x) / scalex, (pt.y - world_new.y) / scaley, (pt.z - world_new.z) / scalez);
        }
        pt = glm::vec3(mPos[i].x * scalex + worldPos_x, mPos[i].y * scaley + worldPos_y, mPos[i].z * scalez + worldPos_z);
        pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
        mPos[i].x = (pt.x - world_new.x) / scalex;
        mPos[i].y = (pt.y - world_new.y) / scaley;
        mPos[i].z = (pt.z - world_new.z) / scalez;
    }
    worldPos_x = world_new.x;
    worldPos_y = world_new.y;
    worldPos_z = world_new.z;

    return true;
}

bool PolyPointScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;

    glm::vec3 world_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    float cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
    float cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
    float cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;
    glm::mat4 translateToOrigin = glm::translate(Identity, -glm::vec3(cx, cy, cz));
    glm::mat4 translateBack = glm::translate(Identity, glm::vec3(cx, cy, cz));
    glm::mat4 scalingMatrix = glm::scale(Identity, factor);
    glm::mat4 m = translateBack * scalingMatrix * translateToOrigin;
    AdjustAllHandles(m);
    world_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(world_pt, 1.0f));
    worldPos_x = world_pt.x;
    worldPos_y = world_pt.y;
    worldPos_z = world_pt.z;

    return true;
}

void PolyPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->UpdateBoundingBox(draw_3d);
        }
        else {
            // create normal line segment bounding boxes
            seg_aabb_min[i] = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
            seg_aabb_max[i] = glm::vec3(RenderWi * mPos[i].seg_scale, BB_OFF, BB_OFF);
        }
    }
}

glm::vec2 PolyPointScreenLocation::GetScreenOffset(IModelPreview* preview) const
{
    float cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
    float cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
    float cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;

    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
        preview->getHeight(),
        glm::vec3(cx, cy, cz),           // X,Y,Z coords of the position when not transformed at all.
        preview->GetProjViewMatrix(),    // Projection / View matrix
        Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

glm::vec3 PolyPointScreenLocation::GetActiveSubHandleWorldPos() const {
    if (!active_handle) return glm::vec3(0.0f);
    const auto& id = *active_handle;
    if (id.role == handles::Role::CurveControl) {
        const int seg = id.segment;
        if (seg < 0 || seg >= num_points || !HasSegmentCurve(seg)) return glm::vec3(0.0f);
        const glm::vec3 cp = (id.index == 0) ? GetCurveCp0(seg) : GetCurveCp1(seg);
        return glm::vec3(cp.x * scalex + worldPos_x,
                          cp.y * scaley + worldPos_y,
                          cp.z * scalez + worldPos_z);
    }
    if (id.role == handles::Role::CentreCycle) {
        return glm::vec3(GetHcenterPos(), GetVcenterPos(), GetDcenterPos());
    }
    if (id.role == handles::Role::Vertex) {
        const int vIdx = id.index;
        if (vIdx < 0 || vIdx >= num_points) return glm::vec3(0.0f);
        return glm::vec3(mPos[vIdx].x * scalex + worldPos_x,
                          mPos[vIdx].y * scaley + worldPos_y,
                          mPos[vIdx].z * scalez + worldPos_z);
    }
    return glm::vec3(0.0f);
}

// Centroid formula matches the CentreCycle descriptor's worldPos —
// no DrawHandles pass required.
float PolyPointScreenLocation::GetHcenterPos() const {
    return (maxX + minX) * scalex / 2.0f + worldPos_x;
}

float PolyPointScreenLocation::GetVcenterPos() const {
    return (maxY + minY) * scaley / 2.0f + worldPos_y;
}

float PolyPointScreenLocation::GetDcenterPos() const {
    return (maxZ + minZ) * scalez / 2.0f + worldPos_z;
}

void PolyPointScreenLocation::SetHcenterPos(float f) {
    worldPos_x += f - GetHcenterPos();
}

void PolyPointScreenLocation::SetVcenterPos(float f) {
    worldPos_y += f - GetVcenterPos();
}

void PolyPointScreenLocation::SetDcenterPos(float f) {
    worldPos_z += f - GetDcenterPos();
}

void PolyPointScreenLocation::SetPosition(float posx, float posy) {

    if (_locked) return;

    SetHcenterPos(posx);
    SetVcenterPos(posy);
}

float PolyPointScreenLocation::GetTop() const {
    return maxY * scaley + worldPos_y;
}

float PolyPointScreenLocation::GetLeft() const {
    return minX * scalex + worldPos_x;
}

float PolyPointScreenLocation::GetMHeight() const
{
    return maxY - minY;
}

float PolyPointScreenLocation::GetMWidth() const
{
    return maxX - minX;
}

void PolyPointScreenLocation::SetMWidth(float w)
{
    if (maxX == minX)
    {
        scalex = 1;
    }
    else
    {
        scalex = w / (maxX - minX);
    }
}

void PolyPointScreenLocation::SetMDepth(float d)
{
    if (maxZ == minZ)
    {
        scalez = 1;
    }
    else
    {
        scalez = d / (maxZ - minZ);
    }
}

float PolyPointScreenLocation::GetMDepth() const
{
    return scalez * (maxZ - minZ);
}

void PolyPointScreenLocation::SetMHeight(float h)
{
    if (maxY == minY || h == 0)
    {
        scaley = 1;
    }
    else
    {
        scaley = h / (maxY - minY);
    }
}

float PolyPointScreenLocation::GetRight() const {
    return maxX * scalex + worldPos_x;
}

float PolyPointScreenLocation::GetBottom() const {
    return minY * scaley + worldPos_y;
}

float PolyPointScreenLocation::GetFront() const {
    return maxZ * scalez + worldPos_z;
}

float PolyPointScreenLocation::GetBack() const {
    return minZ * scalez + worldPos_z;
}

void PolyPointScreenLocation::SetTop(float i) {

    if (_locked) return;

    float newtop = i;
    float topy = maxY * scaley + worldPos_y;
    float diff = newtop - topy;
    worldPos_y += diff;
}

void PolyPointScreenLocation::SetLeft(float i) {

    if (_locked) return;

    float newleft = i;
    float leftx = minX * scalex + worldPos_x;
    float diff = newleft - leftx;
    worldPos_x += diff;
}

void PolyPointScreenLocation::SetRight(float i) {

    if (_locked) return;

    float newright = i;
    float rightx = maxX * scalex + worldPos_x;
    float diff = newright - rightx;
    worldPos_x += diff;
}

void PolyPointScreenLocation::SetBottom(float i) {

    if (_locked) return;

    float newbot = i;
    float boty = minY * scaley + worldPos_y;
    float diff = newbot - boty;
    worldPos_y += diff;
}

void PolyPointScreenLocation::SetFront(float i) {

    if (_locked) return;

    float newfront = i;
    float frontz = maxZ * scalez + worldPos_z;
    float diff = newfront - frontz;
    worldPos_z += diff;
}

void PolyPointScreenLocation::SetBack(float i) {

    if (_locked) return;

    float newback = i;
    float backz = minZ * scalez + worldPos_z;
    float diff = newback - backz;
    worldPos_z += diff;
}

void PolyPointScreenLocation::FixCurveHandles() {
    for(int i = 0; i < num_points; ++i ) {
        if( mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->set_p0( mPos[i].x, mPos[i].y, mPos[i].z );
            mPos[i].curve->set_p1( mPos[i+1].x, mPos[i+1].y, mPos[i+1].z );
            mPos[i].curve->UpdatePoints();
        }
    }
}

void PolyPointScreenLocation::AdjustAllHandles(glm::mat4& mat)
{
    for (int i = 0; i < num_points; ++i) {
        glm::vec3 v = glm::vec3(mat * glm::vec4(mPos[i].x - minX, mPos[i].y - minY, mPos[i].z - minZ, 1.0f));
        mPos[i].x = v.x;
        mPos[i].y = v.y;
        mPos[i].z = v.z;
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            float x1 = mPos[i].curve->get_cp0x();
            float y1 = mPos[i].curve->get_cp0y();
            float z1 = mPos[i].curve->get_cp0z();
            v = glm::vec3(mat * glm::vec4(x1 - minX, y1 - minY, z1 - minZ, 1.0f));
            mPos[i].curve->set_cp0(v.x, v.y, v.z);
            x1 = mPos[i].curve->get_cp1x();
            y1 = mPos[i].curve->get_cp1y();
            z1 = mPos[i].curve->get_cp1z();
            v = glm::vec3(mat * glm::vec4(x1 - minX, y1 - minY, z1 - minZ, 1.0f));
            mPos[i].curve->set_cp1(v.x, v.y, v.z);
        }
    }
    FixCurveHandles();
}

void PolyPointScreenLocation::SetDataFromString(const std::string& point_data)
{
    mPos.clear();
    mPos.resize(num_points);
    auto point_array = Split(point_data, ',');
    while (point_array.size() < (size_t)(num_points * 3)) {
        point_array.push_back("0.0");
    }
    for (int i = 0; i < num_points; ++i) {
        mPos[i].x = std::strtof(point_array[i * 3].c_str(), nullptr);
        mPos[i].y = std::strtof(point_array[i * 3 + 1].c_str(), nullptr);
        mPos[i].z = std::strtof(point_array[i * 3 + 2].c_str(), nullptr);
        mPos[i].has_curve = false;
        mPos[i].curve = nullptr;
    }
}

std::string PolyPointScreenLocation::GetPointDataAsString() const
{
    std::string point_data = "";
    for (int i = 0; i < num_points; ++i) {
        point_data += fmt::format("{:f},", mPos[i].x);
        point_data += fmt::format("{:f},", mPos[i].y);
        point_data += fmt::format("{:f}", mPos[i].z);
        if (i != num_points - 1) {
            point_data += ",";
        }
    }
    return point_data;
}

void PolyPointScreenLocation::SetCurveDataFromString(const std::string& cpoint_data)
{
    auto cpoint_array = Split(cpoint_data, ',');
    int num_curves = (int)cpoint_array.size() / 7;
    glm::vec3 scaling(scalex, scaley, scalez);
    glm::vec3 world_pos(worldPos_x, worldPos_y, worldPos_z);
    for (int i = 0; i < num_curves; ++i) {
        int seg_num = (int)std::strtol(cpoint_array[i * 7].c_str(), nullptr, 10);
        mPos[seg_num].has_curve = true;
        if (mPos[seg_num].curve == nullptr) {
            mPos[seg_num].curve = new BezierCurveCubic3D();
        }
        mPos[seg_num].curve->set_p0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
        mPos[seg_num].curve->set_p1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
        mPos[seg_num].curve->set_cp0(std::strtof(cpoint_array[i * 7 + 1].c_str(), nullptr), std::strtof(cpoint_array[i * 7 + 2].c_str(), nullptr), std::strtof(cpoint_array[i * 7 + 3].c_str(), nullptr));
        mPos[seg_num].curve->set_cp1(std::strtof(cpoint_array[i * 7 + 4].c_str(), nullptr), std::strtof(cpoint_array[i * 7 + 5].c_str(), nullptr), std::strtof(cpoint_array[i * 7 + 6].c_str(), nullptr));
        mPos[seg_num].curve->SetPositioning(scaling, world_pos);
        mPos[seg_num].curve->UpdatePoints();
        mPos[seg_num].curve->UpdateMatrices();
    }
}

std::string PolyPointScreenLocation::GetCurveDataAsString() const
{
    std::string cpoint_data = "";
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i].has_curve) {
            cpoint_data += fmt::format("{},{:f},{:f},{:f},{:f},{:f},{:f},", i, mPos[i].curve->get_cp0x(), mPos[i].curve->get_cp0y(), mPos[i].curve->get_cp0z(),
                                       mPos[i].curve->get_cp1x(), mPos[i].curve->get_cp1y(), mPos[i].curve->get_cp1z());
        }
    }
    return cpoint_data;
}

// ============================================================
// PolyPoint vertex + curve control-point drag.
// ============================================================

glm::vec3 PolyPointScreenLocation::GetCurveCp0(int segment) const {
    if (!HasSegmentCurve(segment)) return glm::vec3(0.0f);
    return glm::vec3(mPos[segment].curve->get_cp0x(),
                     mPos[segment].curve->get_cp0y(),
                     mPos[segment].curve->get_cp0z());
}

glm::vec3 PolyPointScreenLocation::GetCurveCp1(int segment) const {
    if (!HasSegmentCurve(segment)) return glm::vec3(0.0f);
    return glm::vec3(mPos[segment].curve->get_cp1x(),
                     mPos[segment].curve->get_cp1y(),
                     mPos[segment].curve->get_cp1z());
}

void PolyPointScreenLocation::SetCurveCp0(int segment, const glm::vec3& world) {
    if (!HasSegmentCurve(segment)) return;
    const float sx = SafeScale(scalex);
    const float sy = SafeScale(scaley);
    const float lx = (world.x - worldPos_x) / sx;
    const float ly = (world.y - worldPos_y) / sy;
    mPos[segment].curve->set_cp0(lx, ly, 0.0f);
}

void PolyPointScreenLocation::SetCurveCp1(int segment, const glm::vec3& world) {
    if (!HasSegmentCurve(segment)) return;
    const float sx = SafeScale(scalex);
    const float sy = SafeScale(scaley);
    const float lx = (world.x - worldPos_x) / sx;
    const float ly = (world.y - worldPos_y) / sy;
    mPos[segment].curve->set_cp1(lx, ly, 0.0f);
}

namespace {

// Encodes which kind of poly handle the user grabbed.
enum class PolyHandleKind : uint8_t {
    Vertex,
    Cp0,
    Cp1,
};

// 2D bounding-box corner resize session. Builds a per-corner
// scale + translate matrix from the cursor world position and
// feeds it through `AdjustAllHandles`.
class PolyBoundaryResizeSession : public handles::DragSession {
public:
    PolyBoundaryResizeSession(PolyPointScreenLocation* loc,
                              std::string modelName,
                              handles::Id handleId)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId) {
        const int n = _loc->GetNumPoints();
        _savedPoints.reserve(n);
        for (int i = 0; i < n; ++i) {
            _savedPoints.push_back(_loc->GetPoint(i));
        }
        _savedCurves.resize(n);
        for (int s = 0; s < n; ++s) {
            if (_loc->HasSegmentCurve(s)) {
                _savedCurves[s].present = true;
                _savedCurves[s].cp0 = _loc->GetCurveCp0(s);
                _savedCurves[s].cp1 = _loc->GetCurveCp1(s);
            }
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        const float boundary_offset = 8.0f;     // matches BOUNDING_RECT_OFFSET literal in BoxedScreenLocation
        const auto wp = _loc->GetWorldPosition();
        const auto sm = _loc->GetScaleMatrix();
        const float sx = SafeScale(sm.x);
        const float sy = SafeScale(sm.y);
        const float minX = _loc->GetMinX();
        const float maxX = _loc->GetMaxX();
        const float minY = _loc->GetMinY();
        const float maxY = _loc->GetMaxY();

        float newx = 0.0f, newy = 0.0f;
        float trans_x = 0.0f, trans_y = 0.0f;
        float scale_x = 1.0f, scale_y = 1.0f;
        switch (_handleId.index) {
            case 0:    // bottom-left
                newx = (ray.origin.x + boundary_offset - wp.x) / sx;
                newy = (ray.origin.y + boundary_offset - wp.y) / sy;
                if (newx >= maxX - 0.01f || newy >= maxY - 0.01f) return handles::UpdateResult::Unchanged;
                if (maxX - minX != 0.0f) { trans_x = newx - minX; scale_x -= trans_x / (maxX - minX); }
                if (maxY - minY != 0.0f) { trans_y = newy - minY; scale_y -= trans_y / (maxY - minY); }
                break;
            case 1:    // top-left
                newx = (ray.origin.x + boundary_offset - wp.x) / sx;
                newy = (ray.origin.y - boundary_offset - wp.y) / sy;
                if (newx >= maxX - 0.01f || newy <= minY + 0.01f) return handles::UpdateResult::Unchanged;
                if (maxX - minX != 0.0f) { trans_x = newx - minX; scale_x -= trans_x / (maxX - minX); }
                if (maxY - minY != 0.0f) { scale_y = (newy - minY) / (maxY - minY); }
                break;
            case 2:    // bottom-right
                newx = (ray.origin.x - boundary_offset - wp.x) / sx;
                newy = (ray.origin.y + boundary_offset - wp.y) / sy;
                if (newx <= minX + 0.01f || newy >= maxY - 0.01f) return handles::UpdateResult::Unchanged;
                if (maxX - minX != 0.0f) { scale_x = (newx - minX) / (maxX - minX); }
                if (maxY - minY != 0.0f) { trans_y = newy - minY; scale_y -= trans_y / (maxY - minY); }
                break;
            case 3:    // top-right
                newx = (ray.origin.x - boundary_offset - wp.x) / sx;
                newy = (ray.origin.y - boundary_offset - wp.y) / sy;
                if (newx <= minX + 0.01f || newy <= minY + 0.01f) return handles::UpdateResult::Unchanged;
                if (maxX - minX != 0.0f) { scale_x = (newx - minX) / (maxX - minX); }
                if (maxY - minY != 0.0f) { scale_y = (newy - minY) / (maxY - minY); }
                break;
            default:
                return handles::UpdateResult::Unchanged;
        }

        glm::mat4 scaling   = glm::scale(glm::mat4(1.0f), glm::vec3(scale_x, scale_y, 1.0f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(minX + trans_x, minY + trans_y, 0.0f));
        glm::mat4 mat       = translate * scaling;
        _loc->ApplyAffineToAllHandles(mat);
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        for (int i = 0; i < static_cast<int>(_savedPoints.size()); ++i) {
            _loc->SetPoint(i, _savedPoints[i]);
        }
        for (int s = 0; s < static_cast<int>(_savedCurves.size()); ++s) {
            if (_savedCurves[s].present && _loc->HasSegmentCurve(s)) {
                const auto wp = _loc->GetWorldPosition();
                const auto sm = _loc->GetScaleMatrix();
                glm::vec3 cp0World(_savedCurves[s].cp0.x * sm.x + wp.x,
                                    _savedCurves[s].cp0.y * sm.y + wp.y,
                                    _savedCurves[s].cp0.z * sm.z + wp.z);
                glm::vec3 cp1World(_savedCurves[s].cp1.x * sm.x + wp.x,
                                    _savedCurves[s].cp1.y * sm.y + wp.y,
                                    _savedCurves[s].cp1.z * sm.z + wp.z);
                _loc->SetCurveCp0(s, cp0World);
                _loc->SetCurveCp1(s, cp1World);
            }
        }
        _loc->FixCurveHandlesPublic();
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Dimensions : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    struct CurveBackup {
        bool      present = false;
        glm::vec3 cp0{0.0f};
        glm::vec3 cp1{0.0f};
    };
    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    std::vector<glm::vec3>   _savedPoints;
    std::vector<CurveBackup> _savedCurves;
    bool                     _changed = false;
};

// 3D axis-translate session for a curve control point.
// Same math as the vertex variant but mutates
// `BezierCurveCubic3D::set_cp0/1` instead of the vertex.
class PolyCurveCp3DTranslateSession : public handles::DragSession {
public:
    PolyCurveCp3DTranslateSession(PolyPointScreenLocation* loc,
                                  std::string modelName,
                                  handles::Id handleId,
                                  const handles::WorldRay& startRay,
                                  glm::vec3 activeHandlePos)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _isCp1(handleId.index == 1),
          _segment(handleId.segment),
          _savedActivePos(activeHandlePos),
          _savedLocal(_isCp1 ? loc->GetCurveCp1(handleId.segment)
                              : loc->GetCurveCp0(handleId.segment)) {
        ComputeConstraintPlane(handleId.axis);
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
        const float sx = SafeScale(sm.x);
        const float sy = SafeScale(sm.y);
        const float sz = SafeScale(sm.z);
        glm::vec3 newLocal = _savedLocal;
        switch (_handleId.axis) {
            case handles::Axis::X:
                newLocal.x = (_savedActivePos.x + dragDelta.x - wp.x) / sx;
                break;
            case handles::Axis::Y:
                newLocal.y = (_savedActivePos.y + dragDelta.y - wp.y) / sy;
                break;
            case handles::Axis::Z:
                newLocal.z = (_savedActivePos.z + dragDelta.z - wp.z) / sz;
                break;
        }
        // SetCurveCp{0,1} expect world coords; rebuild from local.
        glm::vec3 worldCp(newLocal.x * sm.x + wp.x,
                          newLocal.y * sm.y + wp.y,
                          newLocal.z * sm.z + wp.z);
        if (_isCp1) {
            _loc->SetCurveCp1(_segment, worldCp);
        } else {
            _loc->SetCurveCp0(_segment, worldCp);
        }
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        const auto wp = _loc->GetWorldPosition();
        const auto sm = _loc->GetScaleMatrix();
        glm::vec3 worldCp(_savedLocal.x * sm.x + wp.x,
                           _savedLocal.y * sm.y + wp.y,
                           _savedLocal.z * sm.z + wp.z);
        if (_isCp1) {
            _loc->SetCurveCp1(_segment, worldCp);
        } else {
            _loc->SetCurveCp0(_segment, worldCp);
        }
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Curve : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    void ComputeConstraintPlane(handles::Axis axis) {
        switch (axis) {
            case handles::Axis::X:
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _savedActivePos.z);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _savedActivePos.y, 0.0f);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    bool                     _isCp1;
    int                      _segment;
    glm::vec3                _savedActivePos;
    glm::vec3                _savedLocal;
    glm::vec3                _savedIntersect{0.0f};
    glm::vec3                _planePoint    {0.0f};
    glm::vec3                _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                     _changed = false;
};

// 3D axis-translate session for a single PolyPoint vertex.
// `id.index` carries the vertex index (0..num_points-1);
// `id.axis` is the world axis the user grabbed. Plane setup
// matches Boxed translate at the vertex's saved position.
class PolyVertex3DTranslateSession : public handles::DragSession {
public:
    PolyVertex3DTranslateSession(PolyPointScreenLocation* loc,
                                 std::string modelName,
                                 handles::Id handleId,
                                 const handles::WorldRay& startRay,
                                 glm::vec3 activeHandlePos)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedActivePos(activeHandlePos),
          _savedLocal(loc->GetPoint(handleId.index)) {
        ComputeConstraintPlane(handleId.axis);
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
        const float sx = SafeScale(sm.x);
        const float sy = SafeScale(sm.y);
        const float sz = SafeScale(sm.z);
        glm::vec3 newLocal = _savedLocal;
        switch (_handleId.axis) {
            case handles::Axis::X:
                newLocal.x = (_savedActivePos.x + dragDelta.x - wp.x) / sx;
                break;
            case handles::Axis::Y:
                newLocal.y = (_savedActivePos.y + dragDelta.y - wp.y) / sy;
                break;
            case handles::Axis::Z:
                newLocal.z = (_savedActivePos.z + dragDelta.z - wp.z) / sz;
                break;
        }
        _loc->SetPoint(_handleId.index, newLocal);
        _loc->FixCurveHandlesPublic();
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetPoint(_handleId.index, _savedLocal);
        _loc->FixCurveHandlesPublic();
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
    void ComputeConstraintPlane(handles::Axis axis) {
        switch (axis) {
            case handles::Axis::X:
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _savedActivePos.z);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _savedActivePos.y, 0.0f);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    glm::vec3                _savedActivePos;
    glm::vec3                _savedLocal;
    glm::vec3                _savedIntersect{0.0f};
    glm::vec3                _planePoint    {0.0f};
    glm::vec3                _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                     _changed = false;
};

class PolyVertexSession : public handles::DragSession {
public:
    PolyVertexSession(PolyPointScreenLocation* loc,
                      std::string modelName,
                      handles::Id handleId,
                      PolyHandleKind kind,
                      int segOrVertex)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _kind(kind),
          _segOrVertex(segOrVertex) {
        if (_kind == PolyHandleKind::Vertex) {
            _savedLocal = _loc->GetPoint(_segOrVertex);
        } else if (_kind == PolyHandleKind::Cp0) {
            _savedLocal = _loc->GetCurveCp0(_segOrVertex);
        } else {
            _savedLocal = _loc->GetCurveCp1(_segOrVertex);
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        // Vertex stores model-local (subtract worldPos / scale);
        // cp0/cp1 delegate to BezierCurveCubic3D's public helpers.
        if (_kind == PolyHandleKind::Vertex) {
            const float sx = _loc->GetScaleMatrix().x == 0.0f ? 0.001f : _loc->GetScaleMatrix().x;
            const float sy = _loc->GetScaleMatrix().y == 0.0f ? 0.001f : _loc->GetScaleMatrix().y;
            const float wx = _loc->GetWorldPosition().x;
            const float wy = _loc->GetWorldPosition().y;
            const float lx = (ray.origin.x - wx) / sx;
            const float ly = (ray.origin.y - wy) / sy;
            _loc->SetPoint(_segOrVertex, lx, ly, 0.0f);
            _loc->FixCurveHandlesPublic();
        } else if (_kind == PolyHandleKind::Cp0) {
            _loc->SetCurveCp0(_segOrVertex, ray.origin);
        } else {
            _loc->SetCurveCp1(_segOrVertex, ray.origin);
        }
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        if (_kind == PolyHandleKind::Vertex) {
            _loc->SetPoint(_segOrVertex, _savedLocal);
            _loc->FixCurveHandlesPublic();
        } else if (_kind == PolyHandleKind::Cp0) {
            // _savedLocal is in model-local; SetCurveCp0 expects
            // world. Rebuild world from saved local.
            const auto wp = _loc->GetWorldPosition();
            const auto sm = _loc->GetScaleMatrix();
            glm::vec3 world(_savedLocal.x * sm.x + wp.x,
                             _savedLocal.y * sm.y + wp.y,
                             _savedLocal.z * sm.z + wp.z);
            _loc->SetCurveCp0(_segOrVertex, world);
        } else {
            const auto wp = _loc->GetWorldPosition();
            const auto sm = _loc->GetScaleMatrix();
            glm::vec3 world(_savedLocal.x * sm.x + wp.x,
                             _savedLocal.y * sm.y + wp.y,
                             _savedLocal.z * sm.z + wp.z);
            _loc->SetCurveCp1(_segOrVertex, world);
        }
        _changed = false;
    }

    CommitResult Commit() override {
        const auto field = (_kind == PolyHandleKind::Vertex)
            ? handles::DirtyField::Vertex
            : handles::DirtyField::Curve;
        return CommitResult{_modelName, _changed ? field : handles::DirtyField::None};
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    PolyHandleKind           _kind;
    int                      _segOrVertex;
    glm::vec3                _savedLocal{0.0f};
    bool                     _changed = false;
};

// Placement gesture for newly-created, extended, or moved
// PolyPoint vertices. 2D reads ray.origin directly. 3D picks the
// plane to project onto from `loc->GetActivePlane()` (XY / XZ /
// YZ) and anchors that plane through the vertex's current world
// position — so dragging a vertex from a top-down view moves it
// in X+Z (Y stays), from a side view in Y+Z (X stays), and from
// the front in X+Y (Z stays). The caller is expected to seed
// `active_plane` via `GetBestIntersection` (or `FindPlaneIntersection`)
// based on the current camera angles before opening the session.
class PolyPointCreationSession : public handles::DragSession {
public:
    PolyPointCreationSession(PolyPointScreenLocation* loc,
                             std::string modelName,
                             const handles::WorldRay& clickRay,
                             handles::ViewMode mode,
                             int vertexIndex)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _vertexIndex(vertexIndex),
          _is3D(mode == handles::ViewMode::ThreeD),
          _savedLocal(loc->GetPoint(vertexIndex)) {
        if (_is3D) {
            // Anchor the plane through the vertex's current world
            // position so the unprojected intersect snaps to where
            // the user clicked, not to a far-away Z=0 / Y=0 spot.
            const auto wp = loc->GetWorldPosition();
            const auto sm = loc->GetScaleMatrix();
            const glm::vec3 vp = loc->GetPoint(vertexIndex);
            const glm::vec3 anchor(wp.x + vp.x * sm.x,
                                    wp.y + vp.y * sm.y,
                                    wp.z + vp.z * sm.z);
            _plane = loc->GetActivePlane();
            switch (_plane) {
                case ModelScreenLocation::MSLPLANE::XZ_PLANE:
                    _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                    _planePoint  = glm::vec3(0.0f, anchor.y, 0.0f);
                    break;
                case ModelScreenLocation::MSLPLANE::YZ_PLANE:
                    _planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
                    _planePoint  = glm::vec3(anchor.x, 0.0f, 0.0f);
                    break;
                case ModelScreenLocation::MSLPLANE::XY_PLANE:
                default:
                    _plane       = ModelScreenLocation::MSLPLANE::XY_PLANE;
                    _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                    _planePoint  = glm::vec3(0.0f, 0.0f, anchor.z);
                    break;
            }
            if (!Intersect(clickRay, _savedIntersect)) {
                _savedIntersect = anchor;
            }
        }
        _handleId.role  = handles::Role::Vertex;
        _handleId.index = vertexIndex;
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        // Vertex may have been deleted between sessions
        // (FinalizeModel's polyline trim, etc.). Bail rather than
        // crash on out-of-bounds SetPoint.
        if (_vertexIndex < 0 || _vertexIndex >= _loc->GetNumPoints()) {
            return handles::UpdateResult::Unchanged;
        }
        const auto wp = _loc->GetWorldPosition();
        const auto sm = _loc->GetScaleMatrix();
        const float sx = SafeScale(sm.x);
        const float sy = SafeScale(sm.y);
        const float sz = SafeScale(sm.z);
        if (_is3D) {
            glm::vec3 cur(0.0f);
            if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
            // The two in-plane axes move with the cursor; the
            // perpendicular axis keeps its saved local value so
            // the vertex stays on the chosen plane.
            const float lx = (cur.x - wp.x) / sx;
            const float ly = (cur.y - wp.y) / sy;
            const float lz = (cur.z - wp.z) / sz;
            float nx = _savedLocal.x;
            float ny = _savedLocal.y;
            float nz = _savedLocal.z;
            switch (_plane) {
                case ModelScreenLocation::MSLPLANE::XZ_PLANE:
                    nx = lx; nz = lz; break;          // Y locked
                case ModelScreenLocation::MSLPLANE::YZ_PLANE:
                    ny = ly; nz = lz; break;          // X locked
                case ModelScreenLocation::MSLPLANE::XY_PLANE:
                default:
                    nx = lx; ny = ly; break;          // Z locked
            }
            _loc->SetPoint(_vertexIndex, nx, ny, nz);
        } else {
            const float newx = (ray.origin.x - wp.x) / sx;
            const float newy = (ray.origin.y - wp.y) / sy;
            _loc->SetPoint(_vertexIndex, newx, newy, 0.0f);
        }
        _loc->FixCurveHandlesPublic();
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetPoint(_vertexIndex, _savedLocal);
        _loc->FixCurveHandlesPublic();
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

    PolyPointScreenLocation*       _loc;
    std::string                    _modelName;
    handles::Id                    _handleId;
    int                            _vertexIndex;
    bool                           _is3D;
    glm::vec3                      _savedLocal;
    glm::vec3                      _savedIntersect{0.0f};
    glm::vec3                      _planePoint    {0.0f};
    glm::vec3                      _planeNormal   {0.0f, 0.0f, 1.0f};
    ModelScreenLocation::MSLPLANE  _plane = ModelScreenLocation::MSLPLANE::XY_PLANE;
    bool                           _changed = false;
};

// Single-axis translate of the entire PolyPoint via worldPos.
// Active when CENTER is selected and the user grabs an axis arrow.
class PolyCenterTranslateSession : public handles::DragSession {
public:
    PolyCenterTranslateSession(PolyPointScreenLocation* loc,
                                std::string modelName,
                                handles::Id handleId,
                                const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedWorldPos(loc->GetWorldPosition()) {
        ComputeConstraintPlane(handleId.axis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedWorldPos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 delta = cur - _savedIntersect;

        glm::vec3 newPos = _savedWorldPos;
        switch (_handleId.axis) {
            case handles::Axis::X: newPos.x += delta.x; break;
            case handles::Axis::Y: newPos.y += delta.y; break;
            case handles::Axis::Z: newPos.z += delta.z; break;
        }
        if (newPos == _loc->GetWorldPosition()) {
            return handles::UpdateResult::Unchanged;
        }
        _loc->SetWorldPosition(newPos);
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetWorldPosition(_savedWorldPos);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Position : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    void ComputeConstraintPlane(handles::Axis axis) {
        switch (axis) {
            case handles::Axis::X:
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _savedWorldPos.z);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _savedWorldPos.y, 0.0f);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    glm::vec3                _savedWorldPos;
    glm::vec3                _savedIntersect{0.0f};
    glm::vec3                _planePoint    {0.0f};
    glm::vec3                _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                     _changed = false;
};

// Single-axis scale of the entire PolyPoint around the centroid.
// The centroid is the fixed pivot, scale[xyz] grows by
// ratio = (newOffset / oldOffset), and worldPos is adjusted so the
// centroid stays put. Ctrl skips the worldPos adjust (scale
// around worldPos instead).
class PolyCenterScaleSession : public handles::DragSession {
public:
    PolyCenterScaleSession(PolyPointScreenLocation* loc,
                            std::string modelName,
                            handles::Id handleId,
                            const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedScale(loc->GetScaleMatrix()),
          _savedWorldPos(loc->GetWorldPosition()) {
        // Centroid in world coords.
        _savedCentroid = glm::vec3(loc->GetMinX() + (loc->GetMaxX() - loc->GetMinX()) / 2.0f,
                                    loc->GetMinY() + (loc->GetMaxY() - loc->GetMinY()) / 2.0f,
                                    0.0f) * _savedScale + _savedWorldPos;
        ComputeConstraintPlane(handleId.axis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedCentroid;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 dragDelta = cur - _savedIntersect;

        // Avoid divide-by-zero when worldPos is exactly at the
        // centroid on this axis (matches legacy `+= 0.001f` guard).
        glm::vec3 centroid = _savedCentroid;
        if (centroid.x == _savedWorldPos.x) centroid.x += 0.001f;
        if (centroid.y == _savedWorldPos.y) centroid.y += 0.001f;
        if (centroid.z == _savedWorldPos.z) centroid.z += 0.001f;
        const float dx0 = centroid.x - _savedWorldPos.x;
        const float dy0 = centroid.y - _savedWorldPos.y;
        const float dz0 = centroid.z - _savedWorldPos.z;
        const float change_x = (dx0 + dragDelta.x) / dx0;
        const float change_y = (dy0 + dragDelta.y) / dy0;
        const float change_z = (dz0 + dragDelta.z) / dz0;

        const bool ctrl = handles::HasModifier(mods, handles::Modifier::Control);

        glm::vec3 newScale = _savedScale;
        glm::vec3 newPos   = _savedWorldPos;
        switch (_handleId.axis) {
            case handles::Axis::X: {
                const float newHalfX = dx0 * change_x;
                if (newHalfX < 0.0f) return handles::UpdateResult::Unchanged;
                newScale.x = _savedScale.x * change_x;
                if (!ctrl) newPos.x = _savedWorldPos.x - (newHalfX - dx0);
                break;
            }
            case handles::Axis::Y: {
                const float newHalfY = dy0 * change_y;
                if (newHalfY < 0.0f) return handles::UpdateResult::Unchanged;
                newScale.y = _savedScale.y * change_y;
                if (!ctrl) newPos.y = _savedWorldPos.y - (newHalfY - dy0);
                break;
            }
            case handles::Axis::Z: {
                const float newHalfZ = dz0 * change_z;
                if (newHalfZ < 0.0f) return handles::UpdateResult::Unchanged;
                newScale.z = _savedScale.z * change_z;
                if (!ctrl) newPos.z = _savedWorldPos.z - (newHalfZ - dz0);
                break;
            }
        }

        if (newScale == _loc->GetScaleMatrix() && newPos == _loc->GetWorldPosition()) {
            return handles::UpdateResult::Unchanged;
        }
        _loc->SetScaleMatrix(newScale);
        _loc->SetWorldPosition(newPos);
        _changed = true;
        // Scale changes mPos→world mapping. NeedsInit so the
        // mouse-move handler runs Model::Setup() and rebuilds Nodes
        // (otherwise lights stay at pre-scale positions during drag).
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetScaleMatrix(_savedScale);
        _loc->SetWorldPosition(_savedWorldPos);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Dimensions : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    void ComputeConstraintPlane(handles::Axis axis) {
        switch (axis) {
            case handles::Axis::X:
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _savedCentroid.z);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _savedCentroid.y, 0.0f);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    glm::vec3                _savedScale;
    glm::vec3                _savedWorldPos;
    glm::vec3                _savedCentroid;
    glm::vec3                _savedIntersect{0.0f};
    glm::vec3                _planePoint    {0.0f};
    glm::vec3                _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                     _changed = false;
};

// Single-axis rotate of the entire PolyPoint around the centroid,
// using the same atan2-difference accumulator as Boxed/TwoPoint
// rotate sessions.
class PolyCenterRotateSession : public handles::DragSession {
public:
    PolyCenterRotateSession(PolyPointScreenLocation* loc,
                             std::string modelName,
                             handles::Id handleId,
                             const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _rotationAxis(handles::NextAxis(handleId.axis)) {
        // Compute the centroid (pivot) in world coords.
        const auto wp = loc->GetWorldPosition();
        const auto sm = loc->GetScaleMatrix();
        _savedCentroid = glm::vec3(loc->GetMinX() + (loc->GetMaxX() - loc->GetMinX()) / 2.0f,
                                    loc->GetMinY() + (loc->GetMaxY() - loc->GetMinY()) / 2.0f,
                                    0.0f) * sm + wp;
        ComputeConstraintPlane(_rotationAxis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedCentroid;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 startVec = _savedIntersect - _savedCentroid;
        const glm::vec3 endVec   = cur - _savedCentroid;

        double startAngle = 0.0, endAngle = 0.0;
        switch (_rotationAxis) {
            case handles::Axis::X:
                startAngle = std::atan2(startVec.y, startVec.z) * 180.0 / M_PI;
                endAngle   = std::atan2(endVec.y,   endVec.z)   * 180.0 / M_PI;
                break;
            case handles::Axis::Y:
                startAngle = std::atan2(startVec.x, startVec.z) * 180.0 / M_PI;
                endAngle   = std::atan2(endVec.x,   endVec.z)   * 180.0 / M_PI;
                break;
            case handles::Axis::Z:
                startAngle = std::atan2(startVec.y, startVec.x) * 180.0 / M_PI;
                endAngle   = std::atan2(endVec.y,   endVec.x)   * 180.0 / M_PI;
                break;
        }
        const double total = endAngle - startAngle;
        double delta;
        switch (_rotationAxis) {
            case handles::Axis::X: delta = _accumulatedAngle - total; break;
            default:                delta = total - _accumulatedAngle; break;
        }
        _accumulatedAngle = total;
        if (std::fabs(delta) < 1e-6) return handles::UpdateResult::Unchanged;

        ModelScreenLocation::MSLAXIS axisLegacy = ModelScreenLocation::MSLAXIS::X_AXIS;
        switch (_rotationAxis) {
            case handles::Axis::Y: axisLegacy = ModelScreenLocation::MSLAXIS::Y_AXIS; break;
            case handles::Axis::Z: axisLegacy = ModelScreenLocation::MSLAXIS::Z_AXIS; break;
            default:               axisLegacy = ModelScreenLocation::MSLAXIS::X_AXIS; break;
        }
        _loc->SetRotatePoint(_savedCentroid);
        _loc->Rotate(axisLegacy, static_cast<float>(delta));
        _changed = true;
        // Rotate updates mPos for every vertex. NeedsInit so the
        // mouse-move handler runs Model::Setup() and rebuilds Nodes
        // — otherwise lights stay at pre-rotation positions while
        // the handle spheres (which read mPos directly) move.
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        // Roll back by applying the negation of the accumulated angle.
        if (std::fabs(_accumulatedAngle) > 1e-6) {
            ModelScreenLocation::MSLAXIS axisLegacy = ModelScreenLocation::MSLAXIS::X_AXIS;
            switch (_rotationAxis) {
                case handles::Axis::Y: axisLegacy = ModelScreenLocation::MSLAXIS::Y_AXIS; break;
                case handles::Axis::Z: axisLegacy = ModelScreenLocation::MSLAXIS::Z_AXIS; break;
                default:               axisLegacy = ModelScreenLocation::MSLAXIS::X_AXIS; break;
            }
            _loc->SetRotatePoint(_savedCentroid);
            const float undo = (_rotationAxis == handles::Axis::X)
                ? static_cast<float>(_accumulatedAngle)
                : -static_cast<float>(_accumulatedAngle);
            _loc->Rotate(axisLegacy, undo);
            _accumulatedAngle = 0.0;
        }
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Rotation : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    void ComputeConstraintPlane(handles::Axis axis) {
        // Constraint plane is perpendicular to the rotation axis,
        // passing through the centroid pivot.
        switch (axis) {
            case handles::Axis::X:
                _planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
                _planePoint  = _savedCentroid;
                break;
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = _savedCentroid;
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = _savedCentroid;
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    PolyPointScreenLocation* _loc;
    std::string              _modelName;
    handles::Id              _handleId;
    handles::Axis            _rotationAxis;
    glm::vec3                _savedCentroid;
    glm::vec3                _savedIntersect{0.0f};
    glm::vec3                _planePoint    {0.0f};
    glm::vec3                _planeNormal   {0.0f, 0.0f, 1.0f};
    double                   _accumulatedAngle = 0.0;
    bool                     _changed = false;
};

} // namespace

std::vector<handles::Descriptor> PolyPointScreenLocation::GetHandles(
    handles::ViewMode mode, handles::Tool tool,
    const handles::ViewParams& view) const {
    std::vector<handles::Descriptor> out;
    {
        // Rough upper bound: vertices + 1 segment per gap (curve
        // segs emit ~N chord descriptors, so this under-reserves
        // for heavily curved poly-lines; one realloc is still
        // cheaper than ~N per-descriptor reallocs).
        int reserveN = num_points + (num_points - 1) + 4;
        if (mode == handles::ViewMode::ThreeD) reserveN += 1 + 3;
        out.reserve(static_cast<size_t>(reserveN));
    }

    if (mode == handles::ViewMode::ThreeD) {
        // Centroid sphere — selectionOnly. Picking it sets
        // active_handle = CentreCycle so the whole-model axis gizmo
        // (translate / scale / rotate) appears around the centroid.
        {
            const float cx = (minX + (maxX - minX) / 2.0f) * scalex + worldPos_x;
            const float cy = (minY + (maxY - minY) / 2.0f) * scaley + worldPos_y;
            const float cz = (minZ + (maxZ - minZ) / 2.0f) * scalez + worldPos_z;
            handles::Descriptor d;
            d.id.role  = handles::Role::CentreCycle;
            d.id.index = CENTER_HANDLE;
            d.worldPos = glm::vec3(cx, cy, cz);
            d.suggestedRadius = 5.0f;
            d.editable = !IsLocked();
            d.selectionOnly = true;
            out.push_back(d);
        }
        // 3D vertex / CP descriptors. In `Translate` mode
        // they are `selectionOnly` so a click selects the vertex
        // (LayoutPanel translates the id to a SetActiveHandle and
        // axis arrows then appear). In `XYTranslate` mode (PolyPoint
        // default) they are draggable directly — the click opens a
        // free-XY-plane drag session, no axis arrows involved.
        const bool xyTrans = (tool == handles::Tool::XYTranslate);
        for (int i = 0; i < num_points; ++i) {
            handles::Descriptor d;
            d.id.role  = handles::Role::Vertex;
            d.id.index = i;
            d.worldPos = glm::vec3(mPos[i].x * scalex + worldPos_x,
                                    mPos[i].y * scaley + worldPos_y,
                                    mPos[i].z * scalez + worldPos_z);
            d.suggestedRadius = 5.0f;
            d.editable = !IsLocked();
            d.selectionOnly = !xyTrans;
            out.push_back(d);
        }
        // segment descriptors (line hit-test). Click anywhere
        // on a segment to select it. For curve segments emit
        // multiple chord descriptors approximating the curve so
        // hits on the bowed-out part don't miss.
        for (int s = 0; s < num_points - 1; ++s) {
            const bool isCurve = HasSegmentCurve(s) && mPos[s].curve != nullptr;
            if (isCurve) {
                const int n = mPos[s].curve->GetNumPoints();
                for (int i = 0; i < n - 1; ++i) {
                    handles::Descriptor d;
                    d.id.role    = handles::Role::Segment;
                    d.id.index   = s;
                    d.worldPos   = glm::vec3(mPos[s].curve->get_px(i)     * scalex + worldPos_x,
                                              mPos[s].curve->get_py(i)     * scaley + worldPos_y,
                                              mPos[s].curve->get_pz(i)     * scalez + worldPos_z);
                    d.endPos     = glm::vec3(mPos[s].curve->get_px(i + 1) * scalex + worldPos_x,
                                              mPos[s].curve->get_py(i + 1) * scaley + worldPos_y,
                                              mPos[s].curve->get_pz(i + 1) * scalez + worldPos_z);
                    d.editable = !IsLocked();
                    d.selectionOnly = true;
                    out.push_back(d);
                }
            } else {
                handles::Descriptor d;
                d.id.role    = handles::Role::Segment;
                d.id.index   = s;
                d.worldPos   = glm::vec3(mPos[s].x * scalex + worldPos_x,
                                          mPos[s].y * scaley + worldPos_y,
                                          mPos[s].z * scalez + worldPos_z);
                d.endPos     = glm::vec3(mPos[s + 1].x * scalex + worldPos_x,
                                          mPos[s + 1].y * scaley + worldPos_y,
                                          mPos[s + 1].z * scalez + worldPos_z);
                d.editable = !IsLocked();
                d.selectionOnly = true;
                out.push_back(d);
            }
        }
        for (int s = 0; s < num_points; ++s) {
            if (!HasSegmentCurve(s)) continue;
            const auto cp0 = GetCurveCp0(s);
            const auto cp1 = GetCurveCp1(s);
            handles::Descriptor d0;
            d0.id.role    = handles::Role::CurveControl;
            d0.id.index   = 0;
            d0.id.segment = s;
            d0.worldPos   = glm::vec3(cp0.x * scalex + worldPos_x,
                                       cp0.y * scaley + worldPos_y,
                                       cp0.z * scalez + worldPos_z);
            d0.suggestedRadius = 5.0f;
            d0.editable = !IsLocked();
            d0.selectionOnly = true;
            out.push_back(d0);
            handles::Descriptor d1 = d0;
            d1.id.index = 1;
            d1.worldPos = glm::vec3(cp1.x * scalex + worldPos_x,
                                     cp1.y * scaley + worldPos_y,
                                     cp1.z * scalez + worldPos_z);
            out.push_back(d1);
        }

        // 3D vertex + curve-CP axis-translate
        // gizmo emitted at the active sub-handle position when in
        // translate mode. also Scale/Rotate axis cubes/rings
        // for the CENTER handle (whole-model gizmos).
        const bool toolIsTransScaleRot = (tool == handles::Tool::Translate ||
                                           tool == handles::Tool::Scale ||
                                           tool == handles::Tool::Rotate);
        if (!toolIsTransScaleRot) return out;
        if (!active_handle) return out;
        const auto& ah = *active_handle;
        const glm::vec3 activePos = GetActiveSubHandleWorldPos();
        const float kArrowLen = view.axisArrowLength;
        const float kHitOffset = kArrowLen - view.axisHeadLength * 0.5f;
        auto emitAxisArrow = [&](handles::Axis a, glm::vec3 dir, int idx, int seg) {
            handles::Descriptor d;
            d.id.role    = handles::Role::AxisArrow;
            d.id.axis    = a;
            d.id.index   = idx;
            d.id.segment = seg;
            d.worldPos   = activePos + dir * kHitOffset;
            d.suggestedRadius = view.axisRadius;
            d.editable   = !IsLocked();
            out.push_back(d);
        };
        if (ah.role == handles::Role::CurveControl && ah.index == 0) {
            const int seg = ah.segment;
            // id.index = 0 (cp0), id.segment = seg
            emitAxisArrow(handles::Axis::X, glm::vec3(1, 0, 0), 0, seg);
            emitAxisArrow(handles::Axis::Y, glm::vec3(0, 1, 0), 0, seg);
            emitAxisArrow(handles::Axis::Z, glm::vec3(0, 0, 1), 0, seg);
            return out;
        }
        if (ah.role == handles::Role::CurveControl && ah.index == 1) {
            const int seg = ah.segment;
            emitAxisArrow(handles::Axis::X, glm::vec3(1, 0, 0), 1, seg);
            emitAxisArrow(handles::Axis::Y, glm::vec3(0, 1, 0), 1, seg);
            emitAxisArrow(handles::Axis::Z, glm::vec3(0, 0, 1), 1, seg);
            return out;
        }
        if (ah.role == handles::Role::CentreCycle) {
            // CENTER active: emit per-tool gizmos. The
            // `kWholeModelAxisIndex` sentinel routes CreateDragSession
            // to the appropriate PolyCenter*Session.
            const handles::Role centerRole =
                (tool == handles::Tool::Scale)  ? handles::Role::AxisCube  :
                (tool == handles::Tool::Rotate) ? handles::Role::AxisRing  :
                                                  handles::Role::AxisArrow;
            auto emitCenterAxis = [&](handles::Axis a, glm::vec3 dir) {
                handles::Descriptor d;
                d.id.role    = centerRole;
                d.id.axis    = a;
                d.id.index   = kWholeModelAxisIndex;
                d.id.segment = -1;
                d.worldPos   = activePos + dir * kHitOffset;
                d.suggestedRadius = view.axisRadius;
                d.editable   = !IsLocked();
                out.push_back(d);
            };
            emitCenterAxis(handles::Axis::X, glm::vec3(1, 0, 0));
            emitCenterAxis(handles::Axis::Y, glm::vec3(0, 1, 0));
            emitCenterAxis(handles::Axis::Z, glm::vec3(0, 0, 1));
            return out;
        }
        // Scale/Rotate on a vertex/CP isn't supported — only CENTER
        // shows those gizmos. Translate-only past this point.
        if (tool != handles::Tool::Translate) return out;
        if (ah.role != handles::Role::Vertex) return out;
        const int vertexIndex = ah.index;
        if (vertexIndex < 0 || vertexIndex >= num_points) return out;
        emitAxisArrow(handles::Axis::X, glm::vec3(1, 0, 0), vertexIndex, -1);
        emitAxisArrow(handles::Axis::Y, glm::vec3(0, 1, 0), vertexIndex, -1);
        emitAxisArrow(handles::Axis::Z, glm::vec3(0, 0, 1), vertexIndex, -1);
        return out;
    }

    if (mode != handles::ViewMode::TwoD) return out;
    (void)tool;
    (void)view;

    auto pushVertex = [&](int idx, glm::vec3 worldPos) {
        handles::Descriptor d;
        d.id.role  = handles::Role::Vertex;
        d.id.index = idx;
        d.worldPos = worldPos;
        d.suggestedRadius = 5.0f;
        d.editable = !IsLocked();
        out.push_back(d);
    };
    auto pushCurveCp = [&](int seg, int cpIdx, glm::vec3 worldPos) {
        handles::Descriptor d;
        d.id.role    = handles::Role::CurveControl;
        d.id.index   = cpIdx;     // 0 = cp0, 1 = cp1
        d.id.segment = seg;
        d.worldPos   = worldPos;
        d.suggestedRadius = 5.0f;
        d.editable = !IsLocked();
        out.push_back(d);
    };

    for (int i = 0; i < num_points; ++i) {
        const float wx = mPos[i].x * scalex + worldPos_x;
        const float wy = mPos[i].y * scaley + worldPos_y;
        const float wz = mPos[i].z * scalez + worldPos_z;
        pushVertex(i, glm::vec3(wx, wy, wz));
    }
    // 2D segment descriptors. Curve segments emit chord
    // sub-segments along the curve so clicks on the bowed-out
    // part hit; straight segments emit a single chord.
    for (int s = 0; s < num_points - 1; ++s) {
        const bool isCurve = HasSegmentCurve(s) && mPos[s].curve != nullptr;
        if (isCurve) {
            const int n = mPos[s].curve->GetNumPoints();
            for (int i = 0; i < n - 1; ++i) {
                handles::Descriptor d;
                d.id.role    = handles::Role::Segment;
                d.id.index   = s;
                d.worldPos   = glm::vec3(mPos[s].curve->get_px(i)     * scalex + worldPos_x,
                                          mPos[s].curve->get_py(i)     * scaley + worldPos_y,
                                          mPos[s].curve->get_pz(i)     * scalez + worldPos_z);
                d.endPos     = glm::vec3(mPos[s].curve->get_px(i + 1) * scalex + worldPos_x,
                                          mPos[s].curve->get_py(i + 1) * scaley + worldPos_y,
                                          mPos[s].curve->get_pz(i + 1) * scalez + worldPos_z);
                d.editable = !IsLocked();
                d.selectionOnly = true;
                out.push_back(d);
            }
        } else {
            handles::Descriptor d;
            d.id.role    = handles::Role::Segment;
            d.id.index   = s;
            d.worldPos   = glm::vec3(mPos[s].x * scalex + worldPos_x,
                                      mPos[s].y * scaley + worldPos_y,
                                      mPos[s].z * scalez + worldPos_z);
            d.endPos     = glm::vec3(mPos[s + 1].x * scalex + worldPos_x,
                                      mPos[s + 1].y * scaley + worldPos_y,
                                      mPos[s + 1].z * scalez + worldPos_z);
            d.editable = !IsLocked();
            d.selectionOnly = true;
            out.push_back(d);
        }
    }
    for (int s = 0; s < num_points; ++s) {
        if (!HasSegmentCurve(s)) continue;
        const auto cp0 = GetCurveCp0(s);
        const auto cp1 = GetCurveCp1(s);
        pushCurveCp(s, 0, glm::vec3(cp0.x * scalex + worldPos_x,
                                     cp0.y * scaley + worldPos_y,
                                     cp0.z * scalez + worldPos_z));
        pushCurveCp(s, 1, glm::vec3(cp1.x * scalex + worldPos_x,
                                     cp1.y * scaley + worldPos_y,
                                     cp1.z * scalez + worldPos_z));
    }

    // 2D boundary corner handles for box-resize.
    // Mirrors `DrawHandles`'s 4-corner draw at the bbox extents
    // offset by `boundary_offset`.
    const float hw = 5.0f;
    const float boundary_offset = 2.0f * hw;
    const float bx1 = minX * scalex + worldPos_x - hw / 2.0f - boundary_offset;
    const float by1 = minY * scaley + worldPos_y - hw / 2.0f - boundary_offset;
    const float bx2 = maxX * scalex + worldPos_x + hw / 2.0f + boundary_offset;
    const float by2 = maxY * scaley + worldPos_y + hw / 2.0f + boundary_offset;
    auto pushBoundary = [&](int idx, float x, float y) {
        handles::Descriptor d;
        d.id.role  = handles::Role::ResizeCorner;
        d.id.index = idx;
        d.worldPos = glm::vec3(x, y, 0.0f);
        d.suggestedRadius = 5.0f;
        d.editable = !IsLocked();
        out.push_back(d);
    };
    pushBoundary(0, bx1, by1);    // bottom-left
    pushBoundary(1, bx1, by2);    // top-left
    pushBoundary(2, bx2, by1);    // bottom-right
    pushBoundary(3, bx2, by2);    // top-right
    return out;
}

std::unique_ptr<handles::DragSession> PolyPointScreenLocation::CreateDragSession(
    const std::string& modelName,
    const handles::Id& id,
    const handles::WorldRay& startRay) {
    if (_locked) return nullptr;
    if (id.role == handles::Role::Vertex) {
        if (id.index < 0 || id.index >= num_points) return nullptr;
        // 3D `XYTranslate` vertex drag: free-XY-plane intersection.
        // 2D vertex drag: ray.origin is already world XY.
        if (draw_3d) {
            return std::make_unique<PolyPointCreationSession>(
                this, modelName, startRay, handles::ViewMode::ThreeD, id.index);
        }
        return std::make_unique<PolyVertexSession>(this, modelName, id,
                                                    PolyHandleKind::Vertex, id.index);
    }
    if (id.role == handles::Role::CurveControl) {
        if (id.segment < 0 || id.segment >= num_points) return nullptr;
        if (!HasSegmentCurve(id.segment)) return nullptr;
        const PolyHandleKind kind = (id.index == 0) ? PolyHandleKind::Cp0 : PolyHandleKind::Cp1;
        (void)startRay;
        return std::make_unique<PolyVertexSession>(this, modelName, id, kind, id.segment);
    }
    if (id.role == handles::Role::AxisArrow) {
        if (id.segment >= 0) {
            // Curve CP axis-translate (cp0 / cp1 carried in id.index).
            if (id.segment >= num_points) return nullptr;
            if (!HasSegmentCurve(id.segment)) return nullptr;
            if (id.index != 0 && id.index != 1) return nullptr;
            return std::make_unique<PolyCurveCp3DTranslateSession>(this, modelName, id, startRay,
                                                                    GetActiveSubHandleWorldPos());
        }
        // Whole-model translate routes to PolyCenterTranslateSession
        // (updates worldPos along the active axis).
        if (id.index == kWholeModelAxisIndex) {
            return std::make_unique<PolyCenterTranslateSession>(this, modelName, id, startRay);
        }
        if (id.index < 0 || id.index >= num_points) return nullptr;
        return std::make_unique<PolyVertex3DTranslateSession>(this, modelName, id, startRay,
                                                              GetActiveSubHandleWorldPos());
    }
    if (id.role == handles::Role::AxisCube && id.index == kWholeModelAxisIndex) {
        return std::make_unique<PolyCenterScaleSession>(this, modelName, id, startRay);
    }
    if (id.role == handles::Role::AxisRing && id.index == kWholeModelAxisIndex) {
        return std::make_unique<PolyCenterRotateSession>(this, modelName, id, startRay);
    }
    if (id.role == handles::Role::ResizeCorner) {
        if (id.index < 0 || id.index > 3) return nullptr;
        (void)startRay;
        return std::make_unique<PolyBoundaryResizeSession>(this, modelName, id);
    }
    return nullptr;
}

std::unique_ptr<handles::DragSession> PolyPointScreenLocation::BeginCreate(
    const std::string& modelName,
    const handles::WorldRay& clickRay,
    handles::ViewMode mode) {
    if (_locked) return nullptr;
    // First click: drag the END vertex (index 1, since
    // InitializeLocation places vertex 0 at the click point and
    // initialises vertex 1 to the same spot).
    return std::make_unique<PolyPointCreationSession>(
        this, modelName, clickRay, mode, /*vertexIndex*/ 1);
}

std::unique_ptr<handles::DragSession> PolyPointScreenLocation::BeginExtend(
    const std::string& modelName,
    const handles::WorldRay& clickRay,
    handles::ViewMode mode,
    int vertexIndex) {
    if (_locked) return nullptr;
    if (vertexIndex < 0 || vertexIndex >= num_points) return nullptr;
    return std::make_unique<PolyPointCreationSession>(
        this, modelName, clickRay, mode, vertexIndex);
}
