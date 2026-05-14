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
#include "BoxedScreenLocation.h"


#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Model.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../utils/VectorMath.h"
#include "RulerObject.h"
#include "../utils/AppCallbacks.h"

#include <log.h>

#include <cmath>

extern CursorType GetResizeCursor(int cornerIndex, int PreviewRotation);
extern glm::vec3 rotationMatrixToEulerAngles(const glm::mat3 &R);

inline void TranslatePointDoubles(float radians,float x, float y,float &x1, float &y1) {
    float s = sin(radians);
    float c = cos(radians);
    x1 = c*x-(s*y);
    y1 = s*x+(c*y);
}

#define BOUNDING_RECT_OFFSET 8

static glm::mat4 Identity(glm::mat4(1.0f));


BoxedScreenLocation::BoxedScreenLocation()
    : ModelScreenLocation(10), perspective(0.0f), centerx(0.0), centery(0.0), centerz(0.0)
{
    mSelectableHandles = 1;
}

BoxedScreenLocation::BoxedScreenLocation(int points)
    : ModelScreenLocation(points), perspective(0.0f), centerx(0.0), centery(0.0), centerz(0.0)
{
}

void BoxedScreenLocation::Init() {

    if (!std::isfinite(worldPos_x)) {
        worldPos_x = 0.0F;
    }
    if (!std::isfinite(worldPos_y)) {
        worldPos_y = 0.0F;
    }
    if (!std::isfinite(worldPos_z)) {
        worldPos_z = 0.0F;
    }

    if (scalex < 0) {
        scalex = 1.0f;
    }
    if (scaley < 0) {
        scaley = 1.0f;
    }
    if (scalez < 0) {
        scalez = 1.0f;
    }

    if (rotatex < -180.0f || rotatex > 180.0f) {
        rotatex = 0.0f;
    }
    if (rotatey < -180.0f || rotatey > 180.0f) {
        rotatey = 0.0f;
    }
    if (rotatez < -180.0f || rotatez > 180.0f) {
        rotatez = 0.0f;
    }
    if (rotation_init) {
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        rotate_quat = glm::quat_cast(rz * ry * rx);
        rotation_init = false;
    }
}

void BoxedScreenLocation::TranslatePoint(float &sx, float &sy, float &sz) const {
    sx = (sx*scalex);
    sy = (sy*scaley);
	sz = (sz*scalez);
    glm::vec4 v = rotate_quat * glm::vec4(glm::vec3(sx, sy, sz), 1.0f);
    sx = v.x; sy = v.y; sz = v.z;

    // Give 2D tree model its perspective
    if (!draw_3d) {
        glm::vec4 position = glm::vec4(glm::vec3(sx, sy, sz), 1.0);
        glm::mat4 rm = glm::rotate(Identity, perspective, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec4 model_position = rm * position;
        sx = model_position.x;
        sy = model_position.y;
        sz = model_position.z;
    }

    sx += worldPos_x;
    sy += worldPos_y;
	sz += worldPos_z;
}

void BoxedScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->Translate(worldPos_x,
                   worldPos_y,
                   draw_3d ? worldPos_z : 0);

    ctx ->Rotate(rotatez, 0, 0, 1)
        ->Rotate(rotatey, 0, 1, 0)
        ->Rotate(rotatex, 1, 0, 0)
        ->Scale(scalex, scaley, scalez);

    if (!draw_3d && perspective != 0.0f) {
        // Give 2D tree/cube/sphere models a perspective so you can see the back nodes
        // perspective is in radians, we need degrees fro the Rotate call
        float f = perspective * 180.0f / 3.14159f;
        ctx->Rotate(f, 1, 0, 0);
    }
        
}


bool BoxedScreenLocation::IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const {
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

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
        } else {
            return false;
        }
    }
}

bool BoxedScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    // NOTE:  This routine is designed for the 2D layout model selection only
    bool return_value = false;

    if (VectorMath::TestRayOBBIntersection2D(
        ray_origin,
        aabb_min,
        aabb_max,
        TranslateMatrix)
        ) {
        return_value = true;
    }

    return return_value;
}


CursorType BoxedScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) {
    if (preview != nullptr) {
        FindPlaneIntersection( x, y, preview );
        if (preview->Is3D()) {
            if (supportsZScaling && !_startOnXAxis) {
                worldPos_y = RenderHt / 2.0f;
                active_axis = MSLAXIS::Y_AXIS;
            } else if (active_axis ==  MSLAXIS::Z_AXIS) {
                // Funnel through SetRotation so rotate_quat stays
                // in sync with rotatey — direct member writes leave
                // TranslatePoint reading a stale quat. Explicit
                // qualifier because Boxed's int overload shadows
                // the base vec3 overload.
                ModelScreenLocation::SetRotation(glm::vec3(rotatex, 90.0f, rotatez));
            }
            handle = CENTER_HANDLE;
        } else {
            active_axis = MSLAXIS::Y_AXIS;
            centery = worldPos_y;
            centerx = worldPos_x;
            centerz = worldPos_z;
            scalex = scaley = scalez = 0.0f;
            handle = R_BOT_HANDLE;
        }
    } else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    return CursorType::Sizing;
}

void BoxedScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    if (Nodes.size() > 0) {
        aabb_min = glm::vec3(100000.0f, 100000.0f, 100000.0f);
        aabb_max = glm::vec3(0.0f, 0.0f, 0.0f);

        for (const auto& it : Nodes) {
            for (const auto& coord : it.get()->Coords) {

                float sx = coord.screenX;
                float sy = coord.screenY;
                float sz = coord.screenZ;

                //aabb vectors need to be the untranslated / unrotated limits
                if (sx < aabb_min.x) {
                    aabb_min.x = sx;
                }
                if (sy < aabb_min.y) {
                    aabb_min.y = sy;
                }
                if (sz < aabb_min.z) {
                    aabb_min.z = sz;
                }
                if (sx > aabb_max.x) {
                    aabb_max.x = sx;
                }
                if (sy > aabb_max.y) {
                    aabb_max.y = sy;
                }
                if (sz > aabb_max.z) {
                    aabb_max.z = sz;
                }
            }
        }
        // scale the bounding box for selection logic
        aabb_min.x = aabb_min.x * scalex;
        aabb_min.y = aabb_min.y * scaley;
        aabb_min.z = aabb_min.z * scalez;
        aabb_max.x = aabb_max.x * scalex;
        aabb_max.y = aabb_max.y * scaley;
        aabb_max.z = aabb_max.z * scalez;

        // Set minimum bounding rectangle
        if (aabb_max.y - aabb_min.y < 4) {
            aabb_max.y += 5;
            aabb_min.y -= 5;
        }
        if (aabb_max.x - aabb_min.x < 4) {
            aabb_max.x += 5;
            aabb_min.x -= 5;
        }
        if (aabb_max.z - aabb_min.z < 4) {
            aabb_max.z += 5;
            aabb_min.z -= 5;
        }
    }
}
void BoxedScreenLocation::UpdateBoundingBox(float width, float height, float depth) {
    ModelScreenLocation::UpdateBoundingBox(width, height, depth);
}

void BoxedScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    centerx = worldPos_x;
    centery = worldPos_y;
    centerz = worldPos_z;
    draw_3d = is_3d;
    if (allow_selected) {
        glm::mat4 Translate = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
        glm::mat4 RotationMatrix = glm::toMat4(rotate_quat);
        ModelMatrix = Translate * RotationMatrix;
        TranslateMatrix = Translate;
    }
}

bool BoxedScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(6 * 5 + 2);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase || _locked) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    }
    const float hw = GetRectHandleWidth(zoom, scale);

    // `GetHandles(TwoD, Translate)` is the source of truth — it emits
    // exactly 4 ResizeCorner descriptors + 1 Rotate descriptor at the
    // world positions we render. Draw a small filled square at each.
    glm::vec3 rotatePos(0.0f);
    for (const auto& d : GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate)) {
        const float sx = d.worldPos.x;
        const float sy = d.worldPos.y;
        vac->AddRectAsTriangles(sx - hw / 2, sy - hw / 2, sx + hw / 2, sy + hw / 2, handleColor);
        if (d.id.role == handles::Role::Rotate) {
            rotatePos = d.worldPos;
        }
    }

    // Spoke from the model centre to the rotate handle.
    vac->AddVertex(worldPos_x, worldPos_y, xlWHITE);
    vac->AddVertex(rotatePos.x, rotatePos.y, xlWHITE);

    int count = vac->getCount() - startVertex;
    program->addStep([this, startVertex, count, vac, program](xlGraphicsContext *ctx) {
        ctx->drawTriangles(vac, startVertex, count - 2);
        ctx->drawLines(vac, startVertex + count - 2, 2);
    });
    return true;
}
bool BoxedScreenLocation::DrawHandles(xlGraphicsProgram* program, float zoom, int scale, bool drawBounding, bool fromBase) const
{
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(32 * 5);

    xlColor Box3dColor = xlWHITE;
    if (fromBase) {
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    } else if (_locked) {
        Box3dColor = LOCKED_HANDLES_COLOUR;
    }

    // 8 corners of the bounding cube. The "front" 4 corners (z = +Dp/2)
    // match the L_TOP/R_TOP/R_BOT/L_BOT handle positions; the Z-extruded
    // 4 are decoration only, never emitted from GetHandles.
    auto corner = [&](float lx, float ly, float lz) {
        float sx = lx, sy = ly, sz = lz;
        TranslatePoint(sx, sy, sz);
        return glm::vec3(sx, sy, sz);
    };
    const float halfW   = RenderWi / 2.0f;
    const float halfH   = RenderHt / 2.0f;
    const float halfDp  = RenderDp / 2.0f;
    const float offX    = BOUNDING_RECT_OFFSET / scalex;
    const float offY    = BOUNDING_RECT_OFFSET / scaley;
    const glm::vec3 lt  = corner(-halfW - offX,  halfH + offY,  halfDp);
    const glm::vec3 rt  = corner( halfW + offX,  halfH + offY,  halfDp);
    const glm::vec3 rb  = corner( halfW + offX, -halfH - offY,  halfDp);
    const glm::vec3 lb  = corner(-halfW - offX, -halfH - offY,  halfDp);
    const glm::vec3 ltZ = corner(-halfW - offX,  halfH + offY, -halfDp);
    const glm::vec3 rtZ = corner( halfW + offX,  halfH + offY, -halfDp);
    const glm::vec3 rbZ = corner( halfW + offX, -halfH - offY, -halfDp);
    const glm::vec3 lbZ = corner(-halfW - offX, -halfH - offY, -halfDp);

    auto addLine = [&](const glm::vec3& a, const glm::vec3& b) {
        vac->AddVertex(a.x, a.y, a.z, Box3dColor);
        vac->AddVertex(b.x, b.y, b.z, Box3dColor);
    };
    // Front face.
    addLine(lt, rt);
    addLine(rt, rb);
    addLine(rb, lb);
    addLine(lb, lt);
    // Back face.
    addLine(ltZ, rtZ);
    addLine(rtZ, rbZ);
    addLine(rbZ, lbZ);
    addLine(lbZ, ltZ);
    // Connecting edges.
    addLine(lt, ltZ);
    addLine(rt, rtZ);
    addLine(rb, rbZ);
    addLine(lb, lbZ);

    int lineCount = vac->getCount();
    program->addStep([this, lineCount, startVertex, program, vac](xlGraphicsContext *ctx) {
        ctx->drawLines(vac, startVertex, lineCount - startVertex);
    });

    if (active_handle.has_value()) {
        // Gizmo anchor is the model centre — same point that the
        // CentreCycle descriptor emits. Read it directly here; the
        // descriptor pipeline owns hit-testing, not visualisation.
        const glm::vec3 anchor(GetHcenterPos(), GetVcenterPos(), GetDcenterPos());

        int startTriangles = vac->getCount();
        vac->AddSphereAsTriangles(anchor.x, anchor.y, anchor.z, (double)(GetRectHandleWidth(zoom, scale)), xlORANGETRANSLUCENT);
        int count = vac->getCount();
        program->addStep([startTriangles, count, program, vac](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startTriangles, count - startTriangles);
        });

        DrawAxisTool(anchor, program, zoom, scale);
        DrawActiveAxisIndicator(anchor, program);
    }
    return true;
}

float BoxedScreenLocation::GetRealWidth() const
{
    return RulerObject::Measure(GetRestorableMWidth());
}

float BoxedScreenLocation::GetRealHeight() const 
{
    return RulerObject::Measure(GetRestorableMHeight());
}

float BoxedScreenLocation::GetRealDepth() const
{
    if (supportsZScaling) {
        return RulerObject::Measure(GetRestorableMDepth());
    }
    return 0;
}

std::string BoxedScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    if (supportsZScaling) {
        return fmt::format("Width {} Height {} Depth {}",
            RulerObject::MeasureDescription(GetMWidth()),
            RulerObject::MeasureDescription(GetMHeight()),
            RulerObject::MeasureDescription(GetMDepth()));
    }
    return fmt::format("Width {} Height {}",
        RulerObject::MeasureDescription(GetMWidth()),
        RulerObject::MeasureDescription(GetMHeight()));
}

bool BoxedScreenLocation::Rotate(MSLAXIS axis, float factor)
{
    if (_locked) return false;

    glm::quat rot;
    switch (axis) {
    case MSLAXIS::X_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case MSLAXIS::Y_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case MSLAXIS::Z_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    default:
        break;
    }
 
    rotate_quat = rot * rotate_quat;
    glm::mat4 final_matrix = glm::toMat4(rotate_quat);
    glm::vec3 final_angles = rotationMatrixToEulerAngles(glm::mat3(final_matrix));
    rotatex = glm::degrees(final_angles.x);
    rotatey = glm::degrees(final_angles.y);
    rotatez = glm::degrees(final_angles.z);

    return true;
}

bool BoxedScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;

    scalex *= factor.x;
    scaley *= factor.y;
    scalez *= factor.z;
    return true;
}

// SpaceMouse 6-DOF input. Mouse-driven gizmo drags go through the
// `Boxed*Session` descriptor sessions.


namespace {
// SpaceMouse session for BoxedScreenLocation. Rotates + translates
// the whole model, mirroring the desktop's pre-R-9 CENTER_HANDLE
// branch. The legacy code also had a per-axis "scale gizmo" branch
// driven by `saved_scale * delta`, but `saved_scale` was never
// captured at gesture start (it's only seeded from `AdjustRenderSize`)
// so the runtime math produced 0-or-junk scales. Dropped in R-9 —
// no callers were exercising it via SpaceMouse, and a scale-via-
// SpaceMouse story can be designed cleanly later if needed.
class BoxedSpaceMouseSession : public handles::SpaceMouseSession {
public:
    BoxedSpaceMouseSession(BoxedScreenLocation* loc, std::optional<handles::Id> id)
        : _loc(loc), _id(id) {}

    handles::SpaceMouseResult Apply(float scale,
                                     const glm::vec3& rotations,
                                     const glm::vec3& translations) override {
        if (!_loc) return handles::SpaceMouseResult::Unchanged;
        constexpr float rscale = 10.0f; // 10° per unit at max speed
        _loc->Rotate(ModelScreenLocation::MSLAXIS::X_AXIS,  rotations.x * rscale);
        _loc->Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rotations.z * rscale);
        _loc->Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS,  rotations.y * rscale);
        _loc->AddOffset(translations.x * scale,
                         -translations.z * scale,
                         translations.y * scale);
        return handles::SpaceMouseResult::Dirty;
    }

    [[nodiscard]] std::optional<handles::Id> GetHandleId() const override {
        return _id;
    }

private:
    BoxedScreenLocation*       _loc;
    std::optional<handles::Id> _id;
};
} // namespace

std::unique_ptr<handles::SpaceMouseSession>
BoxedScreenLocation::BeginSpaceMouseSession(const std::optional<handles::Id>& id) {
    return std::make_unique<BoxedSpaceMouseSession>(this, id);
}
        


glm::vec2 BoxedScreenLocation::GetScreenOffset(IModelPreview* preview) const
{
    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
                                                    preview->getHeight(),
                                                    GetWorldPosition(),              // X,Y,Z coords of the position when not transformed at all.
                                                    preview->GetProjViewMatrix(),    // Projection / View matrix
                                                    Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

float BoxedScreenLocation::GetTop() const {
    return worldPos_y + (RenderHt * scaley / 2.0f);
}
float BoxedScreenLocation::GetLeft() const {
    return worldPos_x - (RenderWi * scalex / 2.0f);
}
float BoxedScreenLocation::GetRight() const {
    return worldPos_x + (RenderWi * scalex / 2.0f);
}
float BoxedScreenLocation::GetBottom() const {
    return worldPos_y - (RenderHt * scaley / 2.0f);
}
float BoxedScreenLocation::GetFront() const {
    return worldPos_z + (RenderWi * scalez / 2.0f);
}
float BoxedScreenLocation::GetBack() const {
    return worldPos_z - (RenderWi * scalez / 2.0f);
}

float BoxedScreenLocation::GetRestorableMWidth() const {
    if (RenderWi == 1) {
        return scalex;
    } else {
        return (RenderWi - 1) * scalex;
    }
}
float BoxedScreenLocation::GetRestorableMHeight() const {
    if (RenderHt == 1) {
        return scaley;
    } else {
        return (RenderHt - 1) * scaley;
    }
}
float BoxedScreenLocation::GetMWidth() const {
    if (RenderWi < 0) {
        return 0.0;
    } else {
        return RenderWi * scalex;
    }
}
float BoxedScreenLocation::GetMHeight() const {
    if (RenderHt < 0) {
        return 0.0;
    } else {
        return RenderHt * scaley;
    }
}
void BoxedScreenLocation::SetMWidth(float w) {
    if (RenderWi == 1)
    { 
        if (w != 0) {
            scalex = w;
        } else {
            scalex = 1;
        }
    }
    else
    {
        scalex = w / (RenderWi - 1);
    }
}
void BoxedScreenLocation::SetMDepth(float d) {
    if (RenderWi == 1)
    {
        scalez = 1;
    }
    else
    {
        scalez = d / (RenderWi - 1);
    }
}
float BoxedScreenLocation::GetMDepth() const {
    return scalez * RenderWi;
}
float BoxedScreenLocation::GetRestorableMDepth() const {
    return scalez * (RenderWi-1);
}
void BoxedScreenLocation::SetMHeight(float h) {
    if (RenderHt == 1 || h == 0)
    {
        if (h != 0) {
            scaley = h;
        } else {
            scaley = 1;
        }
    }
    else
    {
        scaley = h / (RenderHt - 1);
    }
}
void BoxedScreenLocation::SetLeft(float x) {
    worldPos_x = x + (RenderWi * scalex / 2.0f);
}
void BoxedScreenLocation::SetRight(float x) {
    worldPos_x = x - (RenderWi * scalex / 2.0f);
}
void BoxedScreenLocation::SetTop(float y) {
    worldPos_y = y - (RenderHt * scaley / 2.0f);
}
void BoxedScreenLocation::SetBottom(float y) {
    worldPos_y = y + (RenderHt * scaley / 2.0f);
}
void BoxedScreenLocation::SetFront(float z) {
    worldPos_z = z - (RenderWi * scalez / 2.0f);
}
void BoxedScreenLocation::SetBack(float z) {
    worldPos_z = z + (RenderWi * scalez / 2.0f);
}

namespace {

// One axis-arrow translate session. Captures the model's
// world-space position + the start-ray's intersection with the
// constraint plane on construction, applies axis-locked deltas
// in Update().
class BoxedTranslateSession : public handles::DragSession {
public:
    BoxedTranslateSession(BoxedScreenLocation* loc,
                          std::string modelName,
                          handles::Id handleId,
                          const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedWorldPos(loc->GetHcenterPos(), loc->GetVcenterPos(), loc->GetDcenterPos()) {
        ComputeConstraintPlane(handleId.axis);
        if (!Intersect(startRay, _savedIntersect)) {
            // Ray parallel to plane — fall back to start position.
            _savedIntersect = _savedWorldPos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        glm::vec3 delta = cur - _savedIntersect;

        glm::vec3 newPos = _savedWorldPos;
        switch (_handleId.axis) {
            case handles::Axis::X: newPos.x += delta.x; break;
            case handles::Axis::Y: newPos.y += delta.y; break;
            case handles::Axis::Z: newPos.z += delta.z; break;
        }

        if (newPos == glm::vec3(_loc->GetHcenterPos(),
                                 _loc->GetVcenterPos(),
                                 _loc->GetDcenterPos())) {
            return handles::UpdateResult::Unchanged;
        }

        _loc->SetHcenterPos(newPos.x);
        _loc->SetVcenterPos(newPos.y);
        _loc->SetDcenterPos(newPos.z);
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetHcenterPos(_savedWorldPos.x);
        _loc->SetVcenterPos(_savedWorldPos.y);
        _loc->SetDcenterPos(_savedWorldPos.z);
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
        // X / Y drag along the XY plane at saved Z; Z drag along
        // the XZ plane at saved Y. Constraining to a plane through
        // the model's saved centre means the cursor ray intersects
        // exactly one point along the chosen axis — that point's
        // axis-component becomes the new world position.
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

    BoxedScreenLocation* _loc;
    std::string          _modelName;
    handles::Id          _handleId;
    glm::vec3            _savedWorldPos;
    glm::vec3            _savedIntersect{0.0f};
    glm::vec3            _planePoint    {0.0f};
    glm::vec3            _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                 _changed = false;
};

// Same constraint-plane intersection as the translate session;
// the active axis becomes a multiplicative ratio on scale[xyz]
// (and on width-paired axes when shift / ctrl are held).
class BoxedScaleSession : public handles::DragSession {
public:
    BoxedScaleSession(BoxedScreenLocation* loc,
                      std::string modelName,
                      handles::Id handleId,
                      const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedWorldPos(loc->GetHcenterPos(),
                          loc->GetVcenterPos(),
                          loc->GetDcenterPos()),
          _savedScale(loc->GetScaleMatrix()),
          _savedSize(loc->GetRenderWi(),
                      loc->GetRenderHt(),
                      loc->GetRenderWi()),
          _supportsZScale(loc->GetSupportsZScaling()) {
        ComputeConstraintPlane(handleId.axis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedWorldPos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 dragDelta = cur - _savedIntersect;
        const glm::vec3 base = _savedSize * _savedScale;
        if (base.x == 0.0f || base.y == 0.0f || base.z == 0.0f) {
            return handles::UpdateResult::Unchanged;
        }
        const float ratio_x = (base.x + dragDelta.x) / base.x;
        const float ratio_y = (base.y + dragDelta.y) / base.y;
        const float ratio_z = (base.z + dragDelta.z) / base.z;
        const bool shift = handles::HasModifier(mods, handles::Modifier::Shift);
        const bool ctrl  = handles::HasModifier(mods, handles::Modifier::Control);

        glm::vec3 newScale = _savedScale;
        glm::vec3 newPos   = _savedWorldPos;

        if (ctrl) {
            switch (_handleId.axis) {
                case handles::Axis::X:
                    newScale.x = _savedScale.x * ratio_x;
                    if (_supportsZScale) newScale.z = newScale.x;
                    newScale.y = _savedScale.y * ratio_x;
                    break;
                case handles::Axis::Y:
                    newScale.y = _savedScale.y * ratio_y;
                    newScale.x = _savedScale.x * ratio_y;
                    if (_supportsZScale) newScale.z = _savedScale.x * ratio_y;
                    break;
                case handles::Axis::Z:
                    if (_supportsZScale) newScale.z = _savedScale.z * ratio_z;
                    newScale.x = _savedScale.z * ratio_z;
                    newScale.y = _savedScale.y * ratio_z;
                    break;
            }
            if (shift) {
                const float current_bottom =
                    _savedWorldPos.y - (_savedScale.y * _loc->GetRenderHt() / 2.0f);
                newPos.y = current_bottom + (newScale.y * _loc->GetRenderHt() / 2.0f);
            }
        } else {
            switch (_handleId.axis) {
                case handles::Axis::X:
                    newScale.x = _savedScale.x * ratio_x;
                    if (shift) {
                        if (_supportsZScale) newScale.z = newScale.x;
                        else                  newScale.y = _savedScale.y * ratio_x;
                    }
                    break;
                case handles::Axis::Y:
                    newScale.y = _savedScale.y * ratio_y;
                    if (shift) {
                        const float current_bottom =
                            _savedWorldPos.y - (_savedScale.y * _loc->GetRenderHt() / 2.0f);
                        newPos.y = current_bottom + (newScale.y * _loc->GetRenderHt() / 2.0f);
                    }
                    break;
                case handles::Axis::Z:
                    if (_supportsZScale) newScale.z = _savedScale.z * ratio_z;
                    if (shift) newScale.x = _savedScale.z * ratio_z;
                    break;
            }
        }

        if (newScale == _savedScale && newPos == _savedWorldPos) {
            return handles::UpdateResult::Unchanged;
        }

        _loc->SetScaleMatrix(newScale);
        if (newPos != _savedWorldPos) {
            _loc->SetHcenterPos(newPos.x);
            _loc->SetVcenterPos(newPos.y);
            _loc->SetDcenterPos(newPos.z);
        }
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetScaleMatrix(_savedScale);
        _loc->SetHcenterPos(_savedWorldPos.x);
        _loc->SetVcenterPos(_savedWorldPos.y);
        _loc->SetDcenterPos(_savedWorldPos.z);
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

    BoxedScreenLocation* _loc;
    std::string          _modelName;
    handles::Id          _handleId;
    glm::vec3            _savedWorldPos;
    glm::vec3            _savedScale;
    glm::vec3            _savedSize;
    glm::vec3            _savedIntersect{0.0f};
    glm::vec3            _planePoint    {0.0f};
    glm::vec3            _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                 _supportsZScale;
    bool                 _changed = false;
};

// Constraint plane is perpendicular to the active axis at the
// model's saved center; rotation is the signed angle traveled
// around that axis in the plane.
//
// Per-frame the legacy code calls `Rotate(axis, delta)` where
// `delta = accumulated_prev_frame - total_change_this_frame`.
// Sum-applied rotation telescopes to `-total_change`, which is
// what makes the model rotate in the cursor's direction. We
// preserve that math verbatim.
class BoxedRotateSession : public handles::DragSession {
public:
    BoxedRotateSession(BoxedScreenLocation* loc,
                       std::string modelName,
                       handles::Id handleId,
                       const handles::WorldRay& startRay)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _rotationAxis(handles::NextAxis(handleId.axis)),
          _savedWorldPos(loc->GetHcenterPos(),
                          loc->GetVcenterPos(),
                          loc->GetDcenterPos()),
          _savedRotation(loc->GetRotateX(), loc->GetRotateY(), loc->GetRotateZ()) {
        ComputeConstraintPlane(_rotationAxis);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedWorldPos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier /*mods*/) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;

        const glm::vec3 startVec = _savedIntersect - _savedWorldPos;
        const glm::vec3 endVec   = cur - _savedWorldPos;
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
        switch (_rotationAxis) {
            case handles::Axis::X:
                _loc->Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, static_cast<float>(delta));
                break;
            case handles::Axis::Y:
                _loc->Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, static_cast<float>(-delta));
                break;
            case handles::Axis::Z:
                _loc->Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, static_cast<float>(-delta));
                break;
        }
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        // Boxed shadows the base SetRotation(vec3) with an int
        // overload that only sets rotatez. Call the base version
        // explicitly so all three axes restore.
        _loc->ModelScreenLocation::SetRotation(_savedRotation);
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
        // Plane normal-to-axis at saved centre. Direction need not
        // be unit-length (GetPlaneIntersect dot-products it).
        switch (axis) {
            case handles::Axis::X:
                _planeNormal = glm::vec3(1.0f, 0.0f, 0.0f);
                _planePoint  = glm::vec3(_savedWorldPos.x, 0.0f, 0.0f);
                break;
            case handles::Axis::Y:
                _planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                _planePoint  = glm::vec3(0.0f, _savedWorldPos.y, 0.0f);
                break;
            case handles::Axis::Z:
                _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
                _planePoint  = glm::vec3(0.0f, 0.0f, _savedWorldPos.z);
                break;
        }
    }

    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    BoxedScreenLocation* _loc;
    std::string          _modelName;
    handles::Id          _handleId;
    handles::Axis        _rotationAxis;
    glm::vec3            _savedWorldPos;
    glm::vec3            _savedRotation;
    glm::vec3            _savedIntersect{0.0f};
    glm::vec3            _planePoint    {0.0f};
    glm::vec3            _planeNormal   {0.0f, 0.0f, 1.0f};
    double               _accumulated = 0.0;
    bool                 _changed = false;
};

// port of `BoxedScreenLocation::MoveHandle`'s 2D corner-
// resize branch. Stateless per-frame: cursor world position +
// model's current state derives the new size and position; no
// latched state needed for the math (only for Revert).
class Boxed2DResizeSession : public handles::DragSession {
public:
    Boxed2DResizeSession(BoxedScreenLocation* loc,
                         std::string modelName,
                         handles::Id handleId)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedWorldPos(loc->GetHcenterPos(),
                          loc->GetVcenterPos(),
                          loc->GetDcenterPos()),
          _savedScale(loc->GetScaleMatrix()) {}

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        const float posx = ray.origin.x;
        const float posy = ray.origin.y;
        const int handle = _handleId.index;
        const float centerx = _loc->GetHcenterPos();
        const float centery = _loc->GetVcenterPos();
        const float renderWi = _loc->GetRenderWi();
        const float renderHt = _loc->GetRenderHt();
        const float scalex = _loc->GetScaleMatrix().x;
        const float scaley = _loc->GetScaleMatrix().y;

        // Crossover guards: stop before flipping past the opposite edge.
        if (handle == L_TOP_HANDLE || handle == R_TOP_HANDLE) {
            if (posy <= centery - renderHt / 2.0f * scaley) return handles::UpdateResult::Unchanged;
        }
        if (handle == L_BOT_HANDLE || handle == R_BOT_HANDLE) {
            if (posy >= centery + renderHt / 2.0f * scaley) return handles::UpdateResult::Unchanged;
        }
        if (handle == R_TOP_HANDLE || handle == R_BOT_HANDLE) {
            if (posx <= centerx - renderWi / 2.0f * scalex) return handles::UpdateResult::Unchanged;
        }
        if (handle == L_TOP_HANDLE || handle == L_BOT_HANDLE) {
            if (posx >= centerx + renderWi / 2.0f * scalex) return handles::UpdateResult::Unchanged;
        }

        float sx = posx - centerx;
        float sy = posy - centery;
        float radians = -glm::radians(_loc->GetRotateZ());
        TranslatePointDoubles(radians, sx, sy, sx, sy);
        sx = std::fabs(sx);
        sy = std::fabs(sy);
        float current_width  = renderWi * scalex;
        float current_height = renderHt * scaley;
        float new_width  = sx + (renderWi / 2.0f * scalex);
        float new_height = sy + (renderHt / 2.0f * scaley);
        new_width  -= BOUNDING_RECT_OFFSET;
        new_height -= BOUNDING_RECT_OFFSET;

        if (handles::HasModifier(mods, handles::Modifier::Shift)) {
            const float aspect = current_width / current_height;
            const float wr = new_width / current_width;
            const float hr = new_height / current_height;
            if (std::fabs(wr - 1.0f) > std::fabs(hr - 1.0f)) {
                new_height = new_width / aspect;
            } else {
                new_width = new_height * aspect;
            }
        }

        float newWorldX = _loc->GetHcenterPos();
        float newWorldY = _loc->GetVcenterPos();
        if (handle == L_TOP_HANDLE || handle == L_BOT_HANDLE) {
            newWorldX += (current_width - new_width) / 2.0f;
        } else {
            newWorldX -= (current_width - new_width) / 2.0f;
        }
        if (handle == L_TOP_HANDLE || handle == R_TOP_HANDLE) {
            newWorldY -= (current_height - new_height) / 2.0f;
        } else {
            newWorldY += (current_height - new_height) / 2.0f;
        }

        const float newSx = new_width  / renderWi;
        const float newSy = new_height / renderHt;
        glm::vec3 newScale(newSx, newSy, _loc->GetScaleMatrix().z);
        if (_loc->GetSupportsZScaling()) {
            newScale.z = newSx;
        }
        _loc->SetScaleMatrix(newScale);
        _loc->SetHcenterPos(newWorldX);
        _loc->SetVcenterPos(newWorldY);
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->SetScaleMatrix(_savedScale);
        _loc->SetHcenterPos(_savedWorldPos.x);
        _loc->SetVcenterPos(_savedWorldPos.y);
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
    BoxedScreenLocation* _loc;
    std::string          _modelName;
    handles::Id          _handleId;
    glm::vec3            _savedWorldPos;
    glm::vec3            _savedScale;
    bool                 _changed = false;
};

// 2D rotate: cursor angle relative to centre drives rotatez,
// with per-quadrant fix-up so the gizmo doesn't flip on axis crossings.
class Boxed2DRotateSession : public handles::DragSession {
public:
    Boxed2DRotateSession(BoxedScreenLocation* loc,
                         std::string modelName,
                         handles::Id handleId)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedRotation(loc->GetRotateX(), loc->GetRotateY(), loc->GetRotateZ()) {}

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        const int sx = static_cast<int>(ray.origin.x - _loc->GetHcenterPos());
        const int sy = static_cast<int>(ray.origin.y - _loc->GetVcenterPos());
        if (sy == 0) return handles::UpdateResult::Unchanged;
        const float tan = static_cast<float>(sx) / static_cast<float>(sy);
        const int angle = -static_cast<int>(glm::degrees(std::atan(tan)));
        float rotatez = 0.0f;
        if (sy >= 0) {
            rotatez = static_cast<float>(angle);
        } else if (sx <= 0) {
            rotatez = static_cast<float>(90 + (90 + angle));
        } else {
            rotatez = static_cast<float>(-90 - (90 - angle));
        }
        if (handles::HasModifier(mods, handles::Modifier::Shift)) {
            rotatez = static_cast<float>(static_cast<int>(rotatez / 5) * 5);
        }
        if (rotatez == _loc->GetRotateZ()) return handles::UpdateResult::Unchanged;
        _loc->SetRotation(static_cast<int>(rotatez));
        _changed = true;
        return handles::UpdateResult::Updated;
    }

    void Revert() override {
        _loc->ModelScreenLocation::SetRotation(_savedRotation);
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
    BoxedScreenLocation* _loc;
    std::string          _modelName;
    handles::Id          _handleId;
    glm::vec3            _savedRotation;
    bool                 _changed = false;
};

// placement gesture for newly-created Boxed models.
// Wraps the existing scale (3D) / corner-resize (2D) sessions so
// that creation reuses their math instead of re-implementing it.
//
//   3D: forces shift (bottom-up scale) + ctrl-when-z_scale
//       (uniform scale).
//   2D: R_BOT_HANDLE corner drag, no forced modifiers.
class BoxedCreationSession : public handles::DragSession {
public:
    BoxedCreationSession(BoxedScreenLocation* loc,
                         std::string modelName,
                         const handles::WorldRay& clickRay,
                         handles::ViewMode mode)
        : _modelName(modelName),
          _supportsZScale(loc->GetSupportsZScaling()),
          _is3D(mode == handles::ViewMode::ThreeD) {
        if (_is3D) {
            handles::Id id;
            id.role  = handles::Role::AxisCube;
            id.axis  = handles::Axis::Y;  // matches InitializeLocation's default
            _inner = std::make_unique<BoxedScaleSession>(
                loc, modelName, id, clickRay);
            _innerId = id;
        } else {
            handles::Id id;
            id.role  = handles::Role::ResizeCorner;
            id.index = R_BOT_HANDLE;
            _inner = std::make_unique<Boxed2DResizeSession>(
                loc, modelName, id);
            _innerId = id;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        if (_is3D) {
            mods = mods | handles::Modifier::Shift;
            if (_supportsZScale) mods = mods | handles::Modifier::Control;
        }
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
    bool                                   _supportsZScale;
    bool                                   _is3D;
};

} // anonymous namespace

std::vector<handles::Descriptor> BoxedScreenLocation::GetHandles(
    handles::ViewMode mode, handles::Tool tool,
    const handles::ViewParams& view) const {
    std::vector<handles::Descriptor> out;
    out.reserve(mode == handles::ViewMode::TwoD ? 5 : 4);

    if (mode == handles::ViewMode::TwoD) {
        // Handle indices use the *_HANDLE constants directly so
        // CreateDragSession's dispatch can route on them.
        constexpr int IDX_L_TOP = L_TOP_HANDLE;
        constexpr int IDX_R_TOP = R_TOP_HANDLE;
        constexpr int IDX_R_BOT = R_BOT_HANDLE;
        constexpr int IDX_L_BOT = L_BOT_HANDLE;

        // Fold BOUNDING_RECT_OFFSET into local coords (divided by
        // scale to cancel TranslatePoint's scale step) so the offset
        // rotates with the model. Adding it in world space leaves
        // the corners drifting off the model whenever rotatez != 0.
        auto emitCorner = [&](int idx, float lx, float ly) {
            float x = lx, y = ly, z = 0.0f;
            TranslatePoint(x, y, z);
            handles::Descriptor d;
            d.id.role = handles::Role::ResizeCorner;
            d.id.index = idx;
            d.worldPos = glm::vec3(x, y, z);
            d.suggestedRadius = 5.0f;
            d.editable = !IsLocked();
            out.push_back(d);
        };
        const float halfW = RenderWi / 2.0f;
        const float halfH = RenderHt / 2.0f;
        const float offX = (scalex == 0.0f) ? 0.0f : BOUNDING_RECT_OFFSET / scalex;
        const float offY = (scaley == 0.0f) ? 0.0f : BOUNDING_RECT_OFFSET / scaley;
        emitCorner(IDX_L_TOP, -halfW - offX,  halfH + offY);
        emitCorner(IDX_R_TOP,  halfW + offX,  halfH + offY);
        emitCorner(IDX_R_BOT,  halfW + offX, -halfH - offY);
        emitCorner(IDX_L_BOT, -halfW - offX, -halfH - offY);

        // Rotate handle: offset above the top edge in local coords
        // so the TRS chain carries it with the model.
        {
            float x = 0.0f;
            float y = halfH + (50.0f / (scaley == 0.0f ? 1.0f : scaley));
            float z = 0.0f;
            TranslatePoint(x, y, z);
            handles::Descriptor d;
            d.id.role = handles::Role::Rotate;
            d.id.index = ROTATE_HANDLE;
            d.worldPos = glm::vec3(x, y, z);
            d.suggestedRadius = 5.0f;
            d.editable = !IsLocked();
            out.push_back(d);
        }
        return out;
    }

    if (mode != handles::ViewMode::ThreeD) return out;

    // first cut: 3D translate gizmo only. Centre cycle handle
    // for tool switching, plus 3 axis arrows. Frontend uses these
    // for hit-testing; handle drawing is still the frontend's job.
    handles::Descriptor centre;
    centre.id.role  = handles::Role::CentreCycle;
    centre.worldPos = glm::vec3(GetHcenterPos(), GetVcenterPos(), GetDcenterPos());
    centre.suggestedRadius = 6.0f;
    centre.editable = !IsLocked();
    centre.selectionOnly = true;     // tap → AdvanceAxisTool, no drag
    out.push_back(centre);

    const float kArrowLen = view.axisArrowLength;
    // Descriptor sits at the centre of the visible arrow head, not the
    // tip — keeps the hit area aligned with what the user sees.
    const float kHitOffset = kArrowLen - view.axisHeadLength * 0.5f;
    auto emitAxisHandle = [&](handles::Role role, handles::Axis a, glm::vec3 dir) {
        handles::Descriptor d;
        d.id.role = role;
        d.id.axis = a;
        d.worldPos = centre.worldPos + dir * kHitOffset;
        d.suggestedRadius = view.axisRadius;
        d.editable = !IsLocked();
        out.push_back(d);
    };

    if (tool == handles::Tool::Translate) {
        emitAxisHandle(handles::Role::AxisArrow, handles::Axis::X, glm::vec3(1, 0, 0));
        emitAxisHandle(handles::Role::AxisArrow, handles::Axis::Y, glm::vec3(0, 1, 0));
        emitAxisHandle(handles::Role::AxisArrow, handles::Axis::Z, glm::vec3(0, 0, 1));
    } else if (tool == handles::Tool::Scale) {
        // axis cubes at the same world offset as the
        // translate arrows. Desktop hit-test only cares about the
        // descriptor position; cube vs arrow visual distinction
        // remains the front-end's job.
        emitAxisHandle(handles::Role::AxisCube, handles::Axis::X, glm::vec3(1, 0, 0));
        emitAxisHandle(handles::Role::AxisCube, handles::Axis::Y, glm::vec3(0, 1, 0));
        if (GetSupportsZScaling()) {
            emitAxisHandle(handles::Role::AxisCube, handles::Axis::Z, glm::vec3(0, 0, 1));
        }
    } else if (tool == handles::Tool::Rotate) {
        // AxisRing descriptors emitted at the axis tips (matching where
        // the desktop's tip cubes hit-test), not the ring shape itself.
        emitAxisHandle(handles::Role::AxisRing, handles::Axis::X, glm::vec3(1, 0, 0));
        emitAxisHandle(handles::Role::AxisRing, handles::Axis::Y, glm::vec3(0, 1, 0));
        emitAxisHandle(handles::Role::AxisRing, handles::Axis::Z, glm::vec3(0, 0, 1));
    }
    return out;
}

std::unique_ptr<handles::DragSession> BoxedScreenLocation::CreateDragSession(
    const std::string& modelName,
    const handles::Id& id,
    const handles::WorldRay& startRay) {
    if (_locked) return nullptr;
    switch (id.role) {
        case handles::Role::AxisArrow:
            return std::make_unique<BoxedTranslateSession>(this, modelName, id, startRay);
        case handles::Role::AxisCube:
            return std::make_unique<BoxedScaleSession>(this, modelName, id, startRay);
        case handles::Role::AxisRing:
            return std::make_unique<BoxedRotateSession>(this, modelName, id, startRay);
        case handles::Role::ResizeCorner:
            return std::make_unique<Boxed2DResizeSession>(this, modelName, id);
        case handles::Role::Rotate:
            return std::make_unique<Boxed2DRotateSession>(this, modelName, id);
        default:
            // CentreCycle and Move (centre drag) have not been ported yet.
            return nullptr;
    }
}

std::unique_ptr<handles::DragSession> BoxedScreenLocation::BeginCreate(
    const std::string& modelName,
    const handles::WorldRay& clickRay,
    handles::ViewMode mode) {
    if (_locked) return nullptr;
    return std::make_unique<BoxedCreationSession>(this, modelName, clickRay, mode);
}
