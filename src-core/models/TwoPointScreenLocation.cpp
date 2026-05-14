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
#include "TwoPointScreenLocation.h"


#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Model.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../utils/VectorMath.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "RulerObject.h"

#include <log.h>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);
extern CursorType GetResizeCursor(int cornerIndex, int PreviewRotation);
extern void rotate_point(float cx, float cy, float angle, float &x, float &y);

#define SNAP_RANGE                  5

static float BB_OFF = 5.0f;

static glm::mat4 Identity(glm::mat4(1.0f));

TwoPointScreenLocation::TwoPointScreenLocation() : ModelScreenLocation(3)
{
    _hasX2 = true;
    mSelectableHandles = 3;
}

TwoPointScreenLocation::~TwoPointScreenLocation() {
}

void TwoPointScreenLocation::Init() {
}

void TwoPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    origin = glm::vec3(worldPos_x, worldPos_y, worldPos_z);

    // if both points are exactly equal, then the line is length 0 and the scaling matrix
    // will not be usable.  We'll offset the x coord slightly so the scaling matrix
    // will not be a 0 matrix
    float x = x2;
    if (x2 == 0.0f && y2 == 0.0f && z2 == 0.0f) {
        x = 0.001f;
    }

    point2 = glm::vec3(x + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);

    glm::vec3 a = point2 - origin;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector2(origin, point2);
    length = glm::length(a);
    scalex = scaley = scalez = length / RenderWi;
    glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    matrix = TranslateMatrix * rotationMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);
        ModelMatrix = TranslateMatrix * rotationMatrix;
        glm::vec4 ctr = matrix * glm::vec4(center, 1.0f);
        center = glm::vec3(ctr);
    }

    draw_3d = is_3d;
}

void TwoPointScreenLocation::TranslatePoint(float &x, float &y, float &z) const {
    glm::vec4 v = matrix * glm::vec4(glm::vec3(x, y, z), 1.0f);
    x = v.x;
    y = v.y;
    z = v.z;
}
void TwoPointScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->SetModelMatrix(matrix);
}


bool TwoPointScreenLocation::IsContained(IModelPreview* preview, int x1_, int y1_, int x2_, int y2_) const {
    int xs = x1_ < x2_ ? x1_ : x2_;
    int xf = x1_ > x2_ ? x1_ : x2_;
    int ys = y1_ < y2_ ? y1_ : y2_;
    int yf = y1_ > y2_ ? y1_ : y2_;

    if (draw_3d) {
        return VectorMath::TestVolumeOBBIntersection(
            xs, ys, xf, yf,
            preview->getWidth(),
            preview->getHeight(),
            aabb_min, aabb_max,
            preview->GetProjViewMatrix(),
            ModelMatrix);
    }
    else {
        glm::vec3 min = glm::vec3(ModelMatrix * glm::vec4(aabb_min, 1.0f));
        glm::vec3 max = glm::vec3(ModelMatrix * glm::vec4(aabb_max, 1.0f));

        if (min.x >= xs && max.x <= xf && min.y >= ys && max.y <= yf) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool TwoPointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    // NOTE:  This routine is designed for the 2D layout model selection only

    bool return_value = false;

    if (VectorMath::TestRayOBBIntersection2D(
        ray_origin,
        aabb_min,
        aabb_max,
        mSelectableHandles == 4 ? TranslateMatrix : ModelMatrix)
        ) {
        return_value = true;
    }

    return return_value;
}


namespace {
// TwoPoint int handle → Endpoint Id. NO_HANDLE → nullopt.
std::optional<handles::Id> TwoPointLegacyToId(int h) {
    if (h == NO_HANDLE) return std::nullopt;
    handles::Id id;
    id.role = handles::Role::Endpoint;
    id.index = h;
    return id;
}
}

void TwoPointScreenLocation::SetActiveHandleToCentre()
{
    // TwoPoint's "centre" body marker is Endpoint(CENTER_HANDLE) —
    // the midpoint sphere drawn between START and END in 3D.
    // Diverges from the base default (CentreCycle) because
    // TwoPoint's GetHandles + IsHandle checks pattern-match
    // specifically on Role::Endpoint.
    active_handle = TwoPointLegacyToId(CENTER_HANDLE);
    highlighted_handle.reset();
    SetAxisTool(axis_tool);
}

void TwoPointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (mode != MSLTOOL::TOOL_SCALE && mode != MSLTOOL::TOOL_XY_TRANS) {
        axis_tool = mode;
    }
    else {
        axis_tool = MSLTOOL::TOOL_TRANSLATE;
    }
}

void TwoPointScreenLocation::AdvanceAxisTool()
{
    ModelScreenLocation::AdvanceAxisTool();
    if (axis_tool == MSLTOOL::TOOL_SCALE) {
        ModelScreenLocation::AdvanceAxisTool();
    }
}

bool TwoPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase)
    {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    
    auto va = program->getAccumulator();
    int startVert = va->getCount();

    va->PreAlloc(16);
    if (std::abs(point2.y - origin.y) <= 0.1) {
        va->AddVertex(worldPos_x, worldPos_y, xlRED);
        va->AddVertex(point2.x, point2.y, xlRED);
    } else if (std::abs(point2.x - origin.x) <= 0.1) {
        va->AddVertex(worldPos_x, worldPos_y, handleColor);
        va->AddVertex(point2.x, point2.y, handleColor);
    }
    int startTriangles = va->getCount();

    const float hw = GetRectHandleWidth(zoom, scale);
    // Source of truth: 2D Endpoint descriptors are at (worldPos) and
    // point2. START gets the highlighting-green sphere; END uses the
    // standard handleColor.
    for (const auto& d : GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate)) {
        if (d.id.role != handles::Role::Endpoint) continue;
        const float sx = d.worldPos.x;
        const float sy = d.worldPos.y;
        const xlColor c = (d.id.index == START_HANDLE) ? xlGREENTRANSLUCENT : handleColor;
        va->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), c);
    }
    int count = va->getCount();

    program->addStep([startVert, startTriangles, count, va, program](xlGraphicsContext *ctx) {
        if (startVert != startTriangles) {
            ctx->drawLines(va, startVert, startTriangles - startVert);
        }
        ctx->drawTriangles(va, startTriangles, count - startTriangles);
    });
    return true;
}


bool TwoPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    auto va = program->getAccumulator();
    int startVert = va->getCount();
    va->PreAlloc(10);

    if (active_handle.has_value()) {
        xlColor h1c = xlBLUETRANSLUCENT;
        xlColor h2c = xlBLUETRANSLUCENT;
        xlColor h3c = xlORANGETRANSLUCENT;
        if (fromBase) {
            h1c = FROM_BASE_HANDLES_COLOUR;
            h2c = FROM_BASE_HANDLES_COLOUR;
            h3c = FROM_BASE_HANDLES_COLOUR;
        } else
        if (_locked) {
            h1c = LOCKED_HANDLES_COLOUR;
            h2c = LOCKED_HANDLES_COLOUR;
            h3c = LOCKED_HANDLES_COLOUR;
        } else {
            h1c = IsHandle(highlighted_handle, handles::Role::Endpoint, START_HANDLE) ? xlYELLOWTRANSLUCENT : xlGREENTRANSLUCENT;
            h2c = IsHandle(highlighted_handle, handles::Role::Endpoint, END_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
            h3c = IsHandle(highlighted_handle, handles::Role::Endpoint, CENTER_HANDLE) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        const float hw = GetRectHandleWidth(zoom, scale);
        // 3D Endpoint descriptors give the three sphere positions
        // (CENTER / START / END); colour comes from the per-index tint.
        for (const auto& d : GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate)) {
            if (d.id.role != handles::Role::Endpoint) continue;
            xlColor c;
            switch (d.id.index) {
                case START_HANDLE:  c = h1c; break;
                case END_HANDLE:    c = h2c; break;
                case CENTER_HANDLE: c = h3c; break;
                default:            continue;
            }
            va->AddSphereAsTriangles(d.worldPos.x, d.worldPos.y, d.worldPos.z, hw, c);
        }

        int endTriangles = va->getCount();
        // Only the Shear branch adds line vertices that the outer addStep
        // below should render. DrawAxisTool and DrawActiveAxisIndicator
        // submit their own addSteps; if startLines straddled their
        // vertices, the outer drawLines would reinterpret their
        // triangle data as garbage line pairs and paint over the
        // gizmo. Default to an empty [startLines, endLines) range.
        int startLines = endTriangles;
        int endLines   = endTriangles;
        if (!_locked) {
            glm::vec3 active_handle_pos = GetActiveHandlePosition();
            DrawAxisTool(active_handle_pos, program, zoom, scale);
            if (IsRole(active_handle, handles::Role::Shear) && active_axis != MSLAXIS::NO_AXIS) {
                // Shear draws a planar cross (X + Y lines) rather than a
                // single-axis indicator. Inline; the base helper handles
                // the simpler single-axis case.
                startLines = va->getCount();
                va->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                va->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                va->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                va->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                endLines = va->getCount();
            } else {
                DrawActiveAxisIndicator(active_handle_pos, program);
            }
        }
        program->addStep([startVert, endTriangles, startLines, endLines, program, va](xlGraphicsContext *ctx) {
            ctx->drawTriangles(va, startVert, endTriangles - startVert);
            if (endLines != startLines) {
                ctx->drawLines(va, startLines, endLines - startLines);
            }
        });
    } else if (drawBounding) {
        // the bounding box is so close to a single line don't draw it once it's selected
        DrawBoundingBox(va, fromBase);
        int endLines = va->getCount();
        program->addStep([startVert, endLines, program, va](xlGraphicsContext *ctx) {
            ctx->drawLines(va, startVert, endLines - startVert);
        });
    }
    return true;
}
void TwoPointScreenLocation::DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const {
    xlColor Box3dColor = xlWHITETRANSLUCENT;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;
    
    glm::vec3 start = origin - glm::vec3(5, 5, 5);
    glm::vec3 end = point2 + glm::vec3(5, 5, 5);
    
    glm::vec3 start2 = origin + glm::vec3(5, 5, 5);
    glm::vec3 end2 = point2 - glm::vec3(5, 5, 5);
    
    float len1 = std::abs(glm::length(end - start));
    float len2 = std::abs(glm::length(end2 - start2));
    if (len2 > len1) {
        start = start2;
        end = end2;
    }

    if (abs(start.x) < 4) {
        start.x = -5;
    }
    if (abs(start.y) < 4) {
        start.y = 5;
    }
    if (abs(start.z) < 4) {
        start.z = 5;
    }

    if (abs(end.x) < 4) {
        end.x = 5;
    }
    if (abs(end.y) < 4) {
        end.y = 5;
    }
    if (abs(end.z) < 4) {
        end.z = 5;
    }

    glm::mat4 mat;
    mat[0] = glm::vec4(1, 0, 0, 0);
    mat[1] = glm::vec4(0, 1, 0, 0);
    mat[2] = glm::vec4(0, 0, 1, 0);
    mat[3] = glm::vec4(0, 0, 0, 1);
    
    DrawBoundingBoxLines(Box3dColor, start, end, mat, *vac);
}


namespace {
// SpaceMouse session for TwoPointScreenLocation. Three flavours:
//   - CentreCycle / no id : rotate + translate the whole model
//   - Endpoint(START)     : translate the start point, drag the
//                            end with it as a rotation pivot
//   - Endpoint(END)       : translate the end point, drag the
//                            start with it as a rotation pivot
// Ports the legacy MoveHandle3D math 1:1 — only the dispatch and
// state holders change.
class TwoPointSpaceMouseSession : public handles::SpaceMouseSession {
public:
    TwoPointSpaceMouseSession(TwoPointScreenLocation* loc,
                              std::optional<handles::Id> id)
        : _loc(loc), _id(id) {}

    handles::SpaceMouseResult Apply(float scale,
                                     const glm::vec3& rot,
                                     const glm::vec3& mov) override {
        if (!_loc) return handles::SpaceMouseResult::Unchanged;
        const bool isStart = _id.has_value() && _id->role == handles::Role::Endpoint
                             && _id->index == START_HANDLE;
        const bool isEnd   = _id.has_value() && _id->role == handles::Role::Endpoint
                             && _id->index == END_HANDLE;
        if (isStart) {
            _loc->ApplySpaceMouseStartHandle(scale, rot, mov);
        } else if (isEnd) {
            _loc->ApplySpaceMouseEndHandle(scale, rot, mov);
        } else {
            // CentreCycle / Endpoint(CENTER) / no id
            constexpr float rscale = 10.0f;
            _loc->Rotate(ModelScreenLocation::MSLAXIS::X_AXIS,  rot.x * rscale);
            _loc->Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rot.z * rscale);
            _loc->Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS,  rot.y * rscale);
            _loc->AddOffset(mov.x * scale, -mov.z * scale, mov.y * scale);
        }
        return handles::SpaceMouseResult::Dirty;
    }

    [[nodiscard]] std::optional<handles::Id> GetHandleId() const override {
        return _id;
    }

private:
    TwoPointScreenLocation*    _loc;
    std::optional<handles::Id> _id;
};
} // namespace

void TwoPointScreenLocation::ApplySpaceMouseStartHandle(float scale,
                                                         const glm::vec3& rot,
                                                         const glm::vec3& mov) {
    worldPos_x += mov.x * scale;
    worldPos_y += -mov.z * scale;
    worldPos_z += mov.y * scale;

    x2 -= mov.x * scale;
    y2 -= -mov.z * scale;
    z2 -= mov.y * scale;

    glm::vec3 sp(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 ep(x2, y2, z2);

    glm::mat4 translateToOrigin = glm::translate(Identity, -sp);
    glm::mat4 translateBack = glm::translate(Identity, sp);

    glm::mat4 R = glm::rotate(Identity, glm::radians(rot.x * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    R = glm::rotate(R, glm::radians(-rot.z * 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    R = glm::rotate(R, glm::radians(rot.y * 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ep = glm::vec3(translateBack * R * translateToOrigin * glm::vec4(ep, 1.0f));
    x2 = ep.x;
    y2 = ep.y;
    z2 = ep.z;
}

void TwoPointScreenLocation::ApplySpaceMouseEndHandle(float scale,
                                                       const glm::vec3& rot,
                                                       const glm::vec3& mov) {
    x2 += mov.x * scale;
    y2 += -mov.z * scale;
    z2 += mov.y * scale;

    glm::vec3 sp(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 ep(x2, y2, z2);

    glm::mat4 translateToOrigin = glm::translate(Identity, -ep);
    glm::mat4 translateBack = glm::translate(Identity, ep);

    glm::mat4 R = glm::rotate(Identity, glm::radians(rot.x * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    R = glm::rotate(R, glm::radians(-rot.z * 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    R = glm::rotate(R, glm::radians(rot.y * 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 sp2 = glm::vec3(translateBack * R * translateToOrigin * glm::vec4(sp, 1.0f));
    worldPos_x = sp2.x;
    worldPos_y = sp2.y;
    worldPos_z = sp2.z;

    x2 += sp.x - worldPos_x;
    y2 += sp.y - worldPos_y;
    z2 += sp.z - worldPos_z;
}

std::unique_ptr<handles::SpaceMouseSession>
TwoPointScreenLocation::BeginSpaceMouseSession(const std::optional<handles::Id>& id) {
    return std::make_unique<TwoPointSpaceMouseSession>(this, id);
}


CursorType TwoPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) {
    if (preview != nullptr) {
        FindPlaneIntersection( x, y, preview );
        if( preview->Is3D() ) {
            active_handle = TwoPointLegacyToId(END_HANDLE);
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    x2 = y2 = z2 = 0.0f;
    handle = END_HANDLE;
    return CursorType::Sizing;
}

std::string TwoPointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    return fmt::format("Length {}", RulerObject::MeasureLengthDescription(origin, point2));
}

void TwoPointScreenLocation::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (_locked) return;
 
    if (angle.y != 0.0f) {
        float posx = GetHcenterPos();
        //float posy = GetVcenterPos();
        float posz = GetDcenterPos();

        float offset = angle.y;
        Rotate(MSLAXIS::Y_AXIS, -offset);
        rotate_point(position.x, position.z, glm::radians(-offset), posx, posz);
        SetHcenterPos(posx);
        SetDcenterPos(posz);
    }
    else {
        ModelScreenLocation::RotateAboutPoint(position, angle);
    }
}

void TwoPointScreenLocation::UpdateBoundingBox()
{
    glm::vec3 start = origin - glm::vec3(5, 5, 5);
    glm::vec3 end = point2 + glm::vec3(5, 5, 5);

    if (abs(start.x) < 4) {
        start.x = -5;
    }
    if (abs(start.y) < 4) {
        start.y = 5;
    }
    if (abs(start.z) < 4) {
        start.z = 5;
    }

    if (abs(end.x) < 4) {
        end.x = 5;
    }
    if (abs(end.y) < 4) {
        end.y = 5;
    }
    if (abs(end.z) < 4) {
        end.z = 5;
    }

    start = start - origin;
    end = end - origin;

    glm::vec4 c1(std::min(start.x, end.x), std::min(start.y, end.y), std::min(start.z, end.z), 1.0f);
    glm::vec4 c2(std::max(end.x, start.x), std::max(end.y, start.y), std::max(end.z, start.z), 1.0f);

    glm::mat4 mat;
    mat[0] = glm::vec4(1, 0, 0, 0);
    mat[1] = glm::vec4(0, 1, 0, 0);
    mat[2] = glm::vec4(0, 0, 1, 0);
    mat[3] = glm::vec4(0, 0, 0, 1);

    c1 = mat * c1;
    c2 = mat * c2;

    aabb_min = glm::vec3(c1.x, c1.y, c1.z);
    aabb_max = glm::vec3(c2.x, c2.y, c2.z);
}

bool TwoPointScreenLocation::Rotate(MSLAXIS axis, float factor) {
    if (_locked) return false;
    glm::vec3 start_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 end_pt = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    glm::vec3 center(x2/2.0 + worldPos_x, y2/2.0 + worldPos_y, z2/2.0 + worldPos_z);
    glm::mat4 translateToOrigin = glm::translate(Identity, -center);
    glm::mat4 translateBack = glm::translate(Identity, center);
    glm::mat4 rot_mat = Identity;

    switch (axis) {
    case MSLAXIS::X_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case MSLAXIS::Y_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(-factor), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case MSLAXIS::Z_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    default:
        break;
    }
    start_pt = glm::vec3(translateBack * rot_mat * translateToOrigin* glm::vec4(start_pt, 1.0f));
    end_pt = glm::vec3(translateBack * rot_mat * translateToOrigin* glm::vec4(end_pt, 1.0f));
    worldPos_x = start_pt.x;
    worldPos_y = start_pt.y;
    worldPos_z = start_pt.z;
    x2 = end_pt.x - worldPos_x;
    y2 = end_pt.y - worldPos_y;
    z2 = end_pt.z - worldPos_z;
    return true;
}

bool TwoPointScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;
    glm::vec3 start_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 end_pt = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    glm::mat4 translateToOrigin = glm::translate(Identity, -center);
    glm::mat4 translateBack = glm::translate(Identity, center);
    glm::mat4 scalingMatrix = glm::scale(Identity, factor);
    start_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(start_pt, 1.0f));
    end_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(end_pt, 1.0f));
    worldPos_x = start_pt.x;
    worldPos_y = start_pt.y;
    worldPos_z = start_pt.z;
    x2 = end_pt.x - worldPos_x;
    y2 = end_pt.y - worldPos_y;
    z2 = end_pt.z - worldPos_z;
    return true;
}

void TwoPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    aabb_min = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
    aabb_max = glm::vec3(RenderWi * scalex, BB_OFF, BB_OFF);
}

glm::vec2 TwoPointScreenLocation::GetScreenOffset(IModelPreview* preview) const
{
    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
        preview->getHeight(),
        center,                          // X,Y,Z coords of the position when not transformed at all.
        preview->GetProjViewMatrix(),    // Projection / View matrix
        Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

float TwoPointScreenLocation::GetHcenterPos() const {
    return worldPos_x + (x2 / 2.0f);
}

float TwoPointScreenLocation::GetVcenterPos() const {
    return worldPos_y + (y2 / 2.0f);
}

float TwoPointScreenLocation::GetDcenterPos() const {
    return worldPos_z + (z2 / 2.0f);
}

void TwoPointScreenLocation::SetHcenterPos(float f) {
    worldPos_x = f - (x2 / 2.0f);
}

void TwoPointScreenLocation::SetVcenterPos(float f) {
    worldPos_y = f - (y2 / 2.0f);
}

void TwoPointScreenLocation::SetDcenterPos(float f) {
    worldPos_z = f - (z2 / 2.0f);
}

void TwoPointScreenLocation::SetPosition(float posx, float posy) {

    if (_locked) return;

    float diffx = x2 / 2.0 - posx;
    float diffy = y2 / 2.0 - posy;

    worldPos_x -= diffx;
    worldPos_y -= diffy;
}

float TwoPointScreenLocation::GetTop() const {
    return std::max(worldPos_y, y2 + worldPos_y);
}

float TwoPointScreenLocation::GetLeft() const {
    return std::min(worldPos_x, x2 + worldPos_x);
}

float TwoPointScreenLocation::GetMWidth() const
{
    return x2;
}

float TwoPointScreenLocation::GetMHeight() const
{
    return y2;
}

float TwoPointScreenLocation::GetRight() const {
    return std::max(worldPos_x, x2 + worldPos_x);
}

float TwoPointScreenLocation::GetBottom() const {
    return std::min(worldPos_y, y2 + worldPos_y);
}

float TwoPointScreenLocation::GetFront() const {
    return std::max(worldPos_z, z2 + worldPos_z);
}

float TwoPointScreenLocation::GetBack() const {
    return std::min(worldPos_z, z2 + worldPos_z);
}

void TwoPointScreenLocation::SetTop(float i) {
    float newtop = i;
    if (y2 < 0) {
        worldPos_y = newtop;
    } else {
        worldPos_y = newtop - y2;
    }
}

void TwoPointScreenLocation::SetBottom(float i) {
    float newbot = i;
    if (y2 > 0) {
        worldPos_y = newbot;
    }
    else {
        worldPos_y = newbot - y2;
    }
}

void TwoPointScreenLocation::SetLeft(float i) {
    float newx = i;
    if (x2 > 0) {
        worldPos_x = newx;
    } else {
        worldPos_x = newx - x2;
    }
}

void TwoPointScreenLocation::SetRight(float i) {
    float newx = i;
    if (x2 < 0) {
        worldPos_x = newx;
    } else {
        worldPos_x = newx - x2;
    }
}

void TwoPointScreenLocation::SetFront(float i) {
    float newfront = i;
    if (z2 < 0) {
        worldPos_z = newfront;
    }
    else {
        worldPos_z = newfront - z2;
    }
}

void TwoPointScreenLocation::SetBack(float i) {
    float newback = i;
    if (z2 > 0) {
        worldPos_z = newback;
    }
    else {
        worldPos_z = newback - z2;
    }
}

void TwoPointScreenLocation::SetMWidth(float w)
{
    x2 = w;
}

void TwoPointScreenLocation::SetMDepth(float w)
{
}

float TwoPointScreenLocation::GetMDepth() const
{
    return 1.0;
}

void TwoPointScreenLocation::SetMHeight(float h)
{
    y2  = h;
}

// ============================================================
// TwoPoint endpoint drag.
// ============================================================

namespace {

// 3D axis-translate session for any one of CENTER / START / END.
// `id.index` carries which sub-handle the user grabbed; `id.axis`
// carries which world axis arrow they pulled. The drag is locked
// to the XY plane at the saved Z for X/Y axes, XZ plane at the
// saved Y for Z.
class TwoPointTranslateSession : public handles::DragSession {
public:
    TwoPointTranslateSession(TwoPointScreenLocation* loc,
                             std::string modelName,
                             handles::Id handleId,
                             const handles::WorldRay& startRay,
                             glm::vec3 activeHandlePos)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedActivePos(activeHandlePos),
          _savedWorldPos(loc->GetWorldPosition()),
          _savedX2(loc->GetX2()),
          _savedY2(loc->GetY2()),
          _savedZ2(loc->GetZ2()) {
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

        const int subHandle = _handleId.index;
        switch (_handleId.axis) {
            case handles::Axis::X:
                ApplyAxis(subHandle, 0, dragDelta.x);
                break;
            case handles::Axis::Y:
                ApplyAxis(subHandle, 1, dragDelta.y);
                break;
            case handles::Axis::Z:
                ApplyAxis(subHandle, 2, dragDelta.z);
                break;
        }
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetWorldPosition(_savedWorldPos);
        _loc->SetX2(_savedX2);
        _loc->SetY2(_savedY2);
        _loc->SetZ2(_savedZ2);
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
    void ApplyAxis(int subHandle, int axisIdx, float delta) {
        // axisIdx: 0=X, 1=Y, 2=Z.
        glm::vec3 newWorld = _savedWorldPos;
        glm::vec3 newOff(_savedX2, _savedY2, _savedZ2);
        if (subHandle == CENTER_HANDLE) {
            // Move both ends together.
            newWorld[axisIdx] = _savedWorldPos[axisIdx] + delta;
        } else if (subHandle == START_HANDLE) {
            // Start moves; end stays fixed → adjust offset.
            newWorld[axisIdx] = _savedWorldPos[axisIdx] + delta;
            newOff[axisIdx]   = (axisIdx == 0 ? _savedX2 : axisIdx == 1 ? _savedY2 : _savedZ2) - delta;
        } else if (subHandle == END_HANDLE) {
            // End moves; start fixed.
            newOff[axisIdx]   = (axisIdx == 0 ? _savedX2 : axisIdx == 1 ? _savedY2 : _savedZ2) + delta;
        } else {
            return;
        }
        _loc->SetWorldPosition(newWorld);
        _loc->SetX2(newOff.x);
        _loc->SetY2(newOff.y);
        _loc->SetZ2(newOff.z);
    }

    void ComputeConstraintPlane(handles::Axis axis) {
        // Same convention as Boxed translate: XY plane at saved Z
        // for X/Y axes, XZ plane at saved Y for Z. Plane passes
        // through the active-handle position so the picked
        // intersect lies near the gizmo arrow.
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

    TwoPointScreenLocation* _loc;
    std::string             _modelName;
    handles::Id             _handleId;
    glm::vec3               _savedActivePos;
    glm::vec3               _savedWorldPos;
    float                   _savedX2;
    float                   _savedY2;
    float                   _savedZ2;
    glm::vec3               _savedIntersect{0.0f};
    glm::vec3               _planePoint    {0.0f};
    glm::vec3               _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                    _changed = false;
};

// 3D axis-rotate session for any one of CENTER / START / END.
// `id.index` carries the sub-handle (which determines the pivot:
// midpoint for CENTER, start point for START, end point for END).
// `id.axis` is the cube the user grabbed — `NextAxis`-shifted to
// the actual rotation axis (same convention as Boxed).
class TwoPointRotateSession : public handles::DragSession {
public:
    TwoPointRotateSession(TwoPointScreenLocation* loc,
                          std::string modelName,
                          handles::Id handleId,
                          const handles::WorldRay& startRay,
                          glm::vec3 pivot)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _rotationAxis(handles::NextAxis(handleId.axis)),
          _pivot(pivot),
          _savedWorldPos(loc->GetWorldPosition()),
          _savedX2(loc->GetX2()),
          _savedY2(loc->GetY2()),
          _savedZ2(loc->GetZ2()) {
        ComputeConstraintPlane(_rotationAxis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _pivot;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;

        const glm::vec3 startVec = _savedIntersect - _pivot;
        const glm::vec3 endVec   = cur - _pivot;
        double total_change = 0.0;
        switch (_rotationAxis) {
            case handles::Axis::X: {
                double s = std::atan2(startVec.y, startVec.z) * 180.0 / M_PI;
                double e = std::atan2(endVec.y,   endVec.z)   * 180.0 / M_PI;
                total_change = e - s;
                break;
            }
            case handles::Axis::Y: {
                double s = std::atan2(startVec.x, startVec.z) * 180.0 / M_PI;
                double e = std::atan2(endVec.x,   endVec.z)   * 180.0 / M_PI;
                total_change = e - s;
                break;
            }
            case handles::Axis::Z: {
                double s = std::atan2(startVec.y, startVec.x) * 180.0 / M_PI;
                double e = std::atan2(endVec.y,   endVec.x)   * 180.0 / M_PI;
                total_change = e - s;
                break;
            }
        }
        const double delta = _accumulated - total_change;
        if (delta == 0.0) return handles::UpdateResult::Unchanged;
        _accumulated = total_change;

        // +delta on X, -delta on Y/Z — matches the on-screen gizmo handedness.
        const float signedDelta =
            (_rotationAxis == handles::Axis::X) ? static_cast<float>(delta) : static_cast<float>(-delta);
        ApplyDelta(signedDelta);
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetWorldPosition(_savedWorldPos);
        _loc->SetX2(_savedX2);
        _loc->SetY2(_savedY2);
        _loc->SetZ2(_savedZ2);
        _accumulated = 0.0;
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
        switch (axis) {
            case handles::Axis::X:
                _planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
                _planePoint  = glm::vec3(_pivot.x, 0.0f, 0.0f);
                break;
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _pivot.y, 0.0f);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _pivot.z);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    void ApplyDelta(float deltaDeg) {
        // Rotation around _pivot on _rotationAxis applied to whichever
        // endpoint(s) the active sub-handle covers.
        glm::vec3 axisVec(0.0f);
        switch (_rotationAxis) {
            case handles::Axis::X: axisVec = glm::vec3(1.0f, 0.0f, 0.0f); break;
            case handles::Axis::Y: axisVec = glm::vec3(0.0f, 1.0f, 0.0f); break;
            case handles::Axis::Z: axisVec = glm::vec3(0.0f, 0.0f, 1.0f); break;
        }
        glm::mat4 toOrigin   = glm::translate(glm::mat4(1.0f), -_pivot);
        glm::mat4 back       = glm::translate(glm::mat4(1.0f),  _pivot);
        glm::mat4 rot        = glm::rotate(glm::mat4(1.0f), glm::radians(deltaDeg), axisVec);
        glm::mat4 xform      = back * rot * toOrigin;

        glm::vec3 startPt = _loc->GetWorldPosition();
        glm::vec3 endPt(startPt.x + _loc->GetX2(),
                        startPt.y + _loc->GetY2(),
                        startPt.z + _loc->GetZ2());
        const int sub = _handleId.index;
        if (sub == CENTER_HANDLE) {
            startPt = glm::vec3(xform * glm::vec4(startPt, 1.0f));
            endPt   = glm::vec3(xform * glm::vec4(endPt,   1.0f));
        } else if (sub == START_HANDLE) {
            // Pivot is the start; only the end moves.
            endPt = glm::vec3(xform * glm::vec4(endPt, 1.0f));
        } else if (sub == END_HANDLE) {
            // Pivot is the end; only the start moves.
            startPt = glm::vec3(xform * glm::vec4(startPt, 1.0f));
        } else {
            return;
        }
        _loc->SetWorldPosition(startPt);
        _loc->SetX2(endPt.x - startPt.x);
        _loc->SetY2(endPt.y - startPt.y);
        _loc->SetZ2(endPt.z - startPt.z);
    }

    TwoPointScreenLocation* _loc;
    std::string             _modelName;
    handles::Id             _handleId;
    handles::Axis           _rotationAxis;
    glm::vec3               _pivot;
    glm::vec3               _savedWorldPos;
    float                   _savedX2;
    float                   _savedY2;
    float                   _savedZ2;
    glm::vec3               _savedIntersect{0.0f};
    glm::vec3               _planePoint    {0.0f};
    glm::vec3               _planeNormal   {0.0f, 0.0f, 1.0f};
    double                  _accumulated = 0.0;
    bool                    _changed = false;
};

class TwoPointEndpointSession : public handles::DragSession {
public:
    TwoPointEndpointSession(TwoPointScreenLocation* loc,
                            std::string modelName,
                            handles::Id handleId)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedWorldPos(loc->GetWorldPosition()),
          _savedX2(loc->GetX2()),
          _savedY2(loc->GetY2()),
          _savedZ2(loc->GetZ2()),
          _savedPoint2(loc->GetPoint2()) {}

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        // 2D start drags worldPos and recomputes x2/y2 to keep the
        // saved point2 fixed; end drags x2/y2 directly.
        const float SNAP_RANGE_LOCAL = 5.0f;
        float newx = ray.origin.x;
        float newy = ray.origin.y;
        const bool shift = handles::HasModifier(mods, handles::Modifier::Shift);
        const int idx = _handleId.index;

        if (shift) {
            if (idx == START_HANDLE) {
                if (std::fabs(newx - _savedPoint2.x) <= SNAP_RANGE_LOCAL) {
                    newx = _savedPoint2.x;
                }
                if (std::fabs(newy - _savedPoint2.y) <= SNAP_RANGE_LOCAL) {
                    newy = _savedPoint2.y;
                }
            } else if (idx == END_HANDLE) {
                if (std::fabs(newx - _savedWorldPos.x) <= SNAP_RANGE_LOCAL) {
                    newx = _savedWorldPos.x;
                } else if (std::fabs(newy - _savedWorldPos.y) <= SNAP_RANGE_LOCAL) {
                    newy = _savedWorldPos.y;
                }
            }
        }

        if (idx == START_HANDLE) {
            glm::vec3 newWorld(newx, newy, _savedWorldPos.z);
            _loc->SetWorldPosition(newWorld);
            _loc->SetX2(_savedPoint2.x - newx);
            _loc->SetY2(_savedPoint2.y - newy);
        } else if (idx == END_HANDLE) {
            _loc->SetX2(newx - _savedWorldPos.x);
            _loc->SetY2(newy - _savedWorldPos.y);
        } else {
            return handles::UpdateResult::Unchanged;
        }
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetWorldPosition(_savedWorldPos);
        _loc->SetX2(_savedX2);
        _loc->SetY2(_savedY2);
        _loc->SetZ2(_savedZ2);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Endpoint : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    TwoPointScreenLocation* _loc;
    std::string             _modelName;
    handles::Id             _handleId;
    glm::vec3               _savedWorldPos;
    float                   _savedX2;
    float                   _savedY2;
    float                   _savedZ2;
    glm::vec3               _savedPoint2;
    bool                    _changed = false;
};

// 3D drag-to-size for newly-created TwoPoint models. Intersects
// the drag ray with the plane FindPlaneIntersection chose at
// click time (XY / XZ / YZ) and writes x2/y2/z2 to (intersect -
// worldPos). Free planar motion — no axis lock — so dragging
// diagonally produces a diagonal line in the chosen plane, which
// matches what users get from desktop's MoveHandle3D path.
class TwoPointPlaneCreateSession : public handles::DragSession {
public:
    TwoPointPlaneCreateSession(TwoPointScreenLocation* loc,
                                std::string modelName,
                                handles::Id handleId,
                                ModelScreenLocation::MSLPLANE plane)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _plane(plane),
          _savedX2(loc->GetX2()),
          _savedY2(loc->GetY2()),
          _savedZ2(loc->GetZ2()),
          _origin(loc->GetWorldPosition()) {
        switch (_plane) {
            case ModelScreenLocation::MSLPLANE::XY_PLANE:
                _normal = glm::vec3(0, 0, 1); break;
            case ModelScreenLocation::MSLPLANE::YZ_PLANE:
                _normal = glm::vec3(1, 0, 0); break;
            case ModelScreenLocation::MSLPLANE::XZ_PLANE:
            default:
                _normal = glm::vec3(0, 1, 0); break;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        const float denom = glm::dot(ray.direction, _normal);
        if (std::fabs(denom) < 1e-6f) {
            // Ray is parallel to the plane — no defined intersection.
            return handles::UpdateResult::Unchanged;
        }
        const float t = glm::dot(_origin - ray.origin, _normal) / denom;
        if (t < 0.0f) {
            // Intersection is behind the camera; ignore so we
            // don't snap to a wildly distant point when the user
            // drags off-plane.
            return handles::UpdateResult::Unchanged;
        }
        const glm::vec3 hit = ray.origin + ray.direction * t;
        _loc->SetX2(hit.x - _origin.x);
        _loc->SetY2(hit.y - _origin.y);
        _loc->SetZ2(hit.z - _origin.z);
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetX2(_savedX2);
        _loc->SetY2(_savedY2);
        _loc->SetZ2(_savedZ2);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Endpoint : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    TwoPointScreenLocation*       _loc;
    std::string                   _modelName;
    handles::Id                   _handleId;
    ModelScreenLocation::MSLPLANE _plane;
    glm::vec3                     _normal{0, 1, 0};
    glm::vec3                     _origin{0, 0, 0};
    float                         _savedX2 = 0;
    float                         _savedY2 = 0;
    float                         _savedZ2 = 0;
    bool                          _changed = false;
};

// placement gesture for newly-created TwoPoint models.
// Routes to:
//   3D: TwoPointPlaneCreateSession on END_HANDLE — free drag in
//       the plane FindPlaneIntersection picked (XY/XZ/YZ).
//   2D: TwoPointEndpointSession on END_HANDLE.
class TwoPointCreationSession : public handles::DragSession {
public:
    TwoPointCreationSession(TwoPointScreenLocation* loc,
                            std::string modelName,
                            const handles::WorldRay& /*clickRay*/,
                            handles::ViewMode mode)
        : _modelName(modelName) {
        if (mode == handles::ViewMode::ThreeD) {
            handles::Id id;
            id.role  = handles::Role::Endpoint;
            id.index = END_HANDLE;
            _inner = std::make_unique<TwoPointPlaneCreateSession>(
                loc, modelName, id, loc->GetActivePlane());
            _innerId = id;
        } else {
            handles::Id id;
            id.role  = handles::Role::Endpoint;
            id.index = END_HANDLE;
            _inner = std::make_unique<TwoPointEndpointSession>(
                loc, modelName, id);
            _innerId = id;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        return _inner->Update(ray, mods);
    }

    void Revert() override { _inner->Revert(); }

    CommitResult Commit() override {
        auto r = _inner->Commit();
        return CommitResult{ _modelName, r.dirty };
    }

    handles::Id GetHandleId() const override { return _innerId; }

private:
    std::string                            _modelName;
    std::unique_ptr<handles::DragSession>  _inner;
    handles::Id                            _innerId;
};

} // namespace

std::vector<handles::Descriptor> TwoPointScreenLocation::GetHandles(
    handles::ViewMode mode, handles::Tool tool,
    const handles::ViewParams& view) const {
    std::vector<handles::Descriptor> out;
    out.reserve(mode == handles::ViewMode::TwoD ? 2 : 6);

    if (mode == handles::ViewMode::TwoD) {
        auto emit = [&](int index, glm::vec3 worldPos) {
            handles::Descriptor d;
            d.id.role = handles::Role::Endpoint;
            d.id.index = index;
            d.worldPos = worldPos;
            d.suggestedRadius = 5.0f;
            d.editable = !IsLocked();
            out.push_back(d);
        };
        emit(START_HANDLE, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
        emit(END_HANDLE,   glm::vec3(worldPos_x + x2, worldPos_y + y2, worldPos_z + z2));
        (void)tool;
        (void)view;
        return out;
    }

    if (mode != handles::ViewMode::ThreeD) return out;

    // 3D selection-only descriptors at the CENTER /
    // START / END sphere world positions. CreateDragSession
    // returns nullptr for selectionOnly markers; LayoutPanel
    // translates the id back into a SetActiveHandle call.
    auto pushSphere = [&](int legacyHandle, glm::vec3 worldPos) {
        handles::Descriptor d;
        d.id.role  = handles::Role::Endpoint;
        d.id.index = legacyHandle;
        d.worldPos = worldPos;
        d.suggestedRadius = 5.0f;
        d.editable = !IsLocked();
        d.selectionOnly = true;
        out.push_back(d);
    };
    pushSphere(START_HANDLE, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    pushSphere(END_HANDLE,   point2);
    pushSphere(CENTER_HANDLE, center);

    // 3D axis gizmo emitted at the active sub-
    // handle position when the user is in translate / rotate
    // mode.
    if (tool != handles::Tool::Translate && tool != handles::Tool::Rotate) {
        return out;
    }
    if (!IsRole(active_handle, handles::Role::Endpoint)) {
        return out;
    }
    const int activeIdx = active_handle->index;
    glm::vec3 activePos;
    switch (activeIdx) {
        case CENTER_HANDLE: activePos = center; break;
        case START_HANDLE:  activePos = glm::vec3(worldPos_x, worldPos_y, worldPos_z); break;
        case END_HANDLE:    activePos = point2; break;
        default:            return out;
    }
    const float kArrowLen = view.axisArrowLength;
    const float kHitOffset = kArrowLen - view.axisHeadLength * 0.5f;
    const handles::Role role = (tool == handles::Tool::Translate)
        ? handles::Role::AxisArrow
        : handles::Role::AxisRing;
    auto emitAxis = [&](handles::Axis a, glm::vec3 dir) {
        handles::Descriptor d;
        d.id.role  = role;
        d.id.axis  = a;
        d.id.index = activeIdx;     // carry the sub-handle through
        d.worldPos = activePos + dir * kHitOffset;
        d.suggestedRadius = view.axisRadius;
        d.editable = !IsLocked();
        out.push_back(d);
    };
    emitAxis(handles::Axis::X, glm::vec3(1, 0, 0));
    emitAxis(handles::Axis::Y, glm::vec3(0, 1, 0));
    emitAxis(handles::Axis::Z, glm::vec3(0, 0, 1));
    return out;
}

std::unique_ptr<handles::DragSession> TwoPointScreenLocation::CreateDragSession(
    const std::string& modelName,
    const handles::Id& id,
    const handles::WorldRay& startRay) {
    if (_locked) return nullptr;
    if (id.role == handles::Role::Endpoint) {
        if (id.index != START_HANDLE && id.index != END_HANDLE) return nullptr;
        (void)startRay;
        return std::make_unique<TwoPointEndpointSession>(this, modelName, id);
    }
    if (id.role == handles::Role::AxisArrow || id.role == handles::Role::AxisRing) {
        if (id.index != CENTER_HANDLE && id.index != START_HANDLE && id.index != END_HANDLE) {
            return nullptr;
        }
        glm::vec3 activePos;
        switch (id.index) {
            case CENTER_HANDLE: activePos = center; break;
            case START_HANDLE:  activePos = glm::vec3(worldPos_x, worldPos_y, worldPos_z); break;
            case END_HANDLE:    activePos = point2; break;
            default:            return nullptr;
        }
        if (id.role == handles::Role::AxisArrow) {
            return std::make_unique<TwoPointTranslateSession>(this, modelName, id, startRay, activePos);
        }
        return std::make_unique<TwoPointRotateSession>(this, modelName, id, startRay, activePos);
    }
    return nullptr;
}

std::unique_ptr<handles::DragSession> TwoPointScreenLocation::BeginCreate(
    const std::string& modelName,
    const handles::WorldRay& clickRay,
    handles::ViewMode mode) {
    if (_locked) return nullptr;
    return std::make_unique<TwoPointCreationSession>(this, modelName, clickRay, mode);
}
