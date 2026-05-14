/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <spdlog/fmt/fmt.h>
#include "ThreePointScreenLocation.h"


#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../utils/VectorMath.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "RulerObject.h"

#include <log.h>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);
extern void rotate_point(float cx, float cy, float angle, float &x, float &y);

static glm::mat4 Identity(glm::mat4(1.0f));

ThreePointScreenLocation::ThreePointScreenLocation()
{
    mSelectableHandles = 4;
}

ThreePointScreenLocation::~ThreePointScreenLocation() {
}

namespace {
// Legacy ThreePoint int → descriptor Id. 0/1/2 = Endpoint; 3 = Shear.
std::optional<handles::Id> ThreePointLegacyToId(int h) {
    if (h == NO_HANDLE) return std::nullopt;
    handles::Id id;
    if (h == SHEAR_HANDLE) {
        id.role = handles::Role::Shear;
        id.index = SHEAR_HANDLE;
    } else {
        id.role = handles::Role::Endpoint;
        id.index = h;
    }
    return id;
}
}

CursorType ThreePointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) {
    if (preview != nullptr) {
        FindPlaneIntersection( x, y, preview );
        if( preview->Is3D() ) {
            active_handle = ThreePointLegacyToId(END_HANDLE);
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    x2 = y2 = z2 = 0.0f;
    handle = END_HANDLE;
    return CursorType::Sizing;
}

void ThreePointScreenLocation::Init() {
    TwoPointScreenLocation::Init();
}

std::string ThreePointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    float width = RulerObject::Measure(origin, point2);
    return fmt::format("Length {} Height {}",
        RulerObject::MeasureLengthDescription(origin, point2),
        RulerObject::PrescaledMeasureDescription((width * height) / 2.0 * factor));
}

float ThreePointScreenLocation::GetRealWidth() const
{
    return RulerObject::Measure(origin, point2);
}

float ThreePointScreenLocation::GetRealHeight() const
{
    float width = RulerObject::Measure(origin, point2);
    return (width * height) / 2.0 * 1.0;
}

inline float toRadians(int degrees) {
    return 2.0*M_PI*float(degrees) / 360.0;
}

void ThreePointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    
    origin = glm::vec3(worldPos_x, worldPos_y, worldPos_z);

    // if both points are exactly equal, then the line is length 0 and the scaling matrix
    // will not be usable.  We'll offset the x coord slightly so the scaling matrix
    // will not be a 0 matrix
    float x = x2;
    if (x2 == 0.0f && y2 == 0.0f && z2 == 0.0f) {
        x = 0.001f;
    }

    point2 = glm::vec3(x + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);

    glm::vec3 point2_calc = point2;
    glm::vec3 origin_calc = origin;
    bool swapped = false;
    if( x2 < 0.0f ) { // green square right of blue square
        point2_calc = origin;
        origin_calc = point2;
        swapped = true;
    }
    
    glm::vec3 a = point2_calc - origin_calc;
    scalex = scaley = scalez = glm::length(a) / RenderWi;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);

    glm::mat4 scalingMatrix;
    if (modelHandleHeight) {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    } else {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley * height, scalez));
    }
    shearMatrix = Identity;
    if (supportsShear) {
        shearMatrix = glm::mat4(glm::shearY(glm::mat3(1.0f), GetYShear()));
    }
    glm::mat4 RotateY = glm::rotate(Identity, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 RotateX = glm::rotate(Identity, glm::radians((float)rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    if (swapped) {
        rotationMatrix = rotationMatrix * RotateY;
    }
    matrix = TranslateMatrix * rotationMatrix * RotateX * shearMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);
        ModelMatrix = TranslateMatrix * rotationMatrix * RotateX;
        glm::vec4 ctr = matrix * glm::vec4(center, 1.0f);
        center = glm::vec3(ctr);
    }

    draw_3d = is_3d;
}

bool ThreePointScreenLocation::IsContained(IModelPreview* preview, int x1_, int y1_, int x2_, int y2_) const {
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
            TranslateMatrix);
    } else {
        glm::vec3 min = glm::vec3(TranslateMatrix * glm::vec4(aabb_min, 1.0f));
        glm::vec3 max = glm::vec3(TranslateMatrix * glm::vec4(aabb_max, 1.0f));

        if (min.x >= xs && max.x <= xf && min.y >= ys && max.y <= yf) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool ThreePointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    return TwoPointScreenLocation::HitTest(ray_origin, ray_direction);
}

void ThreePointScreenLocation::SetMWidth(float w)
{
    TwoPointScreenLocation::SetMWidth(w);
}

float ThreePointScreenLocation::GetMWidth() const
{
    return TwoPointScreenLocation::GetMWidth();
}


void ThreePointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (IsRole(active_handle, handles::Role::Shear)) {
        axis_tool = MSLTOOL::TOOL_XY_TRANS;
    } else {
        TwoPointScreenLocation::SetAxisTool(mode);
    }
}

void ThreePointScreenLocation::AdvanceAxisTool()
{
    if (IsRole(active_handle, handles::Role::Shear)) {
        axis_tool = MSLTOOL::TOOL_XY_TRANS;
    } else {
        TwoPointScreenLocation::AdvanceAxisTool();
    }
}

void ThreePointScreenLocation::SetActiveAxis(MSLAXIS axis)
{
    if (IsRole(active_handle, handles::Role::Shear)) {
        if (axis != MSLAXIS::NO_AXIS) {
            active_axis = MSLAXIS::X_AXIS;
        } else {
            active_axis = MSLAXIS::NO_AXIS;
        }
    } else {
        ModelScreenLocation::SetActiveAxis(axis);
    }
}
bool ThreePointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    if (active_handle.has_value()) {

        float HandleHt = RenderHt;
        if (HandleHt > RenderWi)
            HandleHt = RenderWi;
        float ymax = HandleHt;
        
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        vac->PreAlloc(38);

        float x = RenderWi / 2;
        if (supportsAngle) {
            ymax = HandleHt * height;
            rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
        }

        glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 0.0f), 1.0f));
        float sx = v1.x;
        float sy = v1.y;
        float sz = v1.z;
        vac->AddVertex(center.x, center.y, center.z, xlWHITE);
        vac->AddVertex(sx, sy, sz, xlWHITE);

        xlColor h4c = xlBLUETRANSLUCENT;
        if (fromBase)
        {
            h4c = FROM_BASE_HANDLES_COLOUR;
        } else
        if (_locked) {
            h4c = LOCKED_HANDLES_COLOUR;
        } else {
            h4c = IsHandle(highlighted_handle, handles::Role::Shear, SHEAR_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
        }

        float hw = GetRectHandleWidth(zoom, scale);
        vac->AddSphereAsTriangles(sx, sy, sz, hw, h4c);

        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, 2);
            ctx->drawTriangles(vac, startVertex + 2, count - startVertex - 2);
        });
    }

    TwoPointScreenLocation::DrawHandles(program, zoom, scale, drawBounding, fromBase);
    return true;
}

bool ThreePointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    float sx1 = center.x;
    float sy1 = center.y;

    float HandleHt = RenderHt;
    if (HandleHt > RenderWi)
        HandleHt = RenderWi;
    float ymax = HandleHt;

    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(18);

    float x = RenderWi / 2;
    if (supportsAngle) {
        ymax = HandleHt * height;
        rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
    }

    glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 1), 1.0f));
    float sx = v1.x;
    float sy = v1.y;
    vac->AddVertex(sx1, sy1, xlWHITE);
    vac->AddVertex(sx, sy, xlWHITE);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase)
    {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    float hw = GetRectHandleWidth(zoom, scale);
    vac->AddRectAsTriangles(sx - hw/2.0, sy - hw/2.0, sx + hw, sy + hw, handleColor);
    int count = vac->getCount();
    program->addStep([=](xlGraphicsContext *ctx) {
        ctx->drawLines(vac, startVertex, 2);
        ctx->drawTriangles(vac, startVertex + 2, count - 2);
    });

    TwoPointScreenLocation::DrawHandles(program, zoom, scale, fromBase);
    return true;
}

void ThreePointScreenLocation::DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const {
    xlColor Box3dColor = xlWHITETRANSLUCENT;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;
    DrawBoundingBoxLines(Box3dColor, aabb_min, aabb_max, draw_3d ? ModelMatrix : TranslateMatrix, *vac);
}



namespace {
// SpaceMouse session for ThreePointScreenLocation. Adds a Shear
// handle on top of TwoPoint's CENTER / START / END behavior:
// dragging Shear adjusts angle / shear / height (subclass-defined
// — Arch / Window Frame each set their `supports*` flags).
class ThreePointSpaceMouseSession : public handles::SpaceMouseSession {
public:
    ThreePointSpaceMouseSession(ThreePointScreenLocation* loc,
                                std::optional<handles::Id> id)
        : _loc(loc), _id(id),
          _twoPointInner(loc->TwoPointScreenLocation::BeginSpaceMouseSession(id)) {}

    handles::SpaceMouseResult Apply(float scale,
                                     const glm::vec3& rot,
                                     const glm::vec3& mov) override {
        if (!_loc) return handles::SpaceMouseResult::Unchanged;
        if (_id.has_value() && _id->role == handles::Role::Shear) {
            _loc->ApplySpaceMouseShearHandle(scale, rot, mov);
            return handles::SpaceMouseResult::NeedsInit;
        }
        if (_twoPointInner) {
            return _twoPointInner->Apply(scale, rot, mov);
        }
        return handles::SpaceMouseResult::Unchanged;
    }

    [[nodiscard]] std::optional<handles::Id> GetHandleId() const override {
        return _id;
    }

private:
    ThreePointScreenLocation*                       _loc;
    std::optional<handles::Id>                      _id;
    std::unique_ptr<handles::SpaceMouseSession>     _twoPointInner;
};
} // namespace

void ThreePointScreenLocation::ApplySpaceMouseShearHandle(float /*scale*/,
                                                           const glm::vec3& /*rot*/,
                                                           const glm::vec3& mov) {
    if (supportsAngle) {
        angle -= mov.x * 10.0f;
        height += -mov.z;
    } else if (supportsShear) {
        shear -= mov.x * 10.0f;
        height += -mov.z;
    } else {
        height += -mov.z;
    }
    if (std::abs(height) < 0.01f) {
        height = (height < 0.0f) ? -0.01f : 0.01f;
    }
}

std::unique_ptr<handles::SpaceMouseSession>
ThreePointScreenLocation::BeginSpaceMouseSession(const std::optional<handles::Id>& id) {
    return std::make_unique<ThreePointSpaceMouseSession>(this, id);
}
float ThreePointScreenLocation::GetVScaleFactor() const {
    if (modelHandleHeight) {
        return 1.0;
    }
    return height;
}

void ThreePointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    if (Nodes.size() > 0) {
        aabb_min = glm::vec3(100000.0f, 100000.0f, 100000.0f);
        aabb_max = glm::vec3(0.0f, 0.0f, 0.0f);

        if (draw_3d) {
            shearMatrix = glm::mat4(glm::shearY(glm::mat3(1.0f), GetYShear()));
            glm::mat4 scalingMatrix;
            if (modelHandleHeight) {
                scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
            }
            else {
                scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley * height, scalez));
            }

            for (const auto& it : Nodes) {
                for (const auto& coord : it.get()->Coords) {

                    float sx = coord.screenX;
                    float sy = coord.screenY;
                    float sz = coord.screenZ;

                    glm::vec3 shear_point = glm::vec3(shearMatrix * scalingMatrix * glm::vec4(glm::vec3(sx, sy, sz), 1.0f));
                    sx = shear_point.x;
                    sy = shear_point.y;
                    sz = shear_point.z;

                    //aabb vectors need to be the untranslated / unrotated
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
        }
        else {
            aabb_min.z = 0.0f;
            aabb_max.z = 0.0f;
            for (const auto& it : Nodes) {
                for (const auto& coord : it.get()->Coords) {

                    float sx = coord.screenX;
                    float sy = coord.screenY;
                    float sz = coord.screenZ;
                    glm::vec4 v = matrix * glm::vec4(glm::vec3(sx, sy, sz), 1.0f);
                    v.x -= worldPos_x;
                    v.y -= worldPos_y;

                    //aabb vectors need to be the untranslated / unrotated
                    if (v.x < aabb_min.x) {
                        aabb_min.x = v.x;
                    }
                    if (v.y < aabb_min.y) {
                        aabb_min.y = v.y;
                    }
                    if (v.x > aabb_max.x) {
                        aabb_max.x = v.x;
                    }
                    if (v.y > aabb_max.y) {
                        aabb_max.y = v.y;
                    }
                }
            }
        }

        // Set minimum bounding rectangle
        if (aabb_max.x - aabb_min.x < 4) {
            aabb_max.x += 5;
            aabb_min.x -= 5;
        }
        if (aabb_max.y - aabb_min.y < 4) {
            aabb_max.y += 5;
            aabb_min.y -= 5;
        }
        if (aabb_max.z - aabb_min.z < 4) {
            aabb_max.z += 5;
            aabb_min.z -= 5;
        }
    }
}

// ============================================================
// ThreePoint shear handle. Endpoints / centre inherit TwoPoint.
// ============================================================

glm::vec3 ThreePointScreenLocation::GetShearHandleWorldPosition() const {
    // Matches DrawHandles' shear-sphere position formula so the
    // descriptor and the drawn sphere stay in lockstep.
    float HandleHt = RenderHt;
    if (HandleHt > RenderWi) HandleHt = RenderWi;
    float ymax = HandleHt;
    float x = RenderWi / 2.0f;
    if (supportsAngle) {
        ymax = HandleHt * height;
        rotate_point(RenderWi / 2.0f, 0.0f, toRadians(angle), x, ymax);
    }
    glm::vec3 v = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 0.0f), 1.0f));
    return v;
}

namespace {

// 3D shear-handle drag (axis_tool == TOOL_XY_TRANS). The user
// pulls the SHEAR sphere along the XY plane through its saved
// world position; the cursor delta is then mapped back into the
// model's local axis frame via the inverse of the rotation that
// aligns +X with (point2 - origin), so subsequent supportsAngle
// / supportsShear / default branches run on planar coords.
class ThreePoint3DShearSession : public handles::DragSession {
public:
    ThreePoint3DShearSession(ThreePointScreenLocation* loc,
                             std::string modelName,
                             handles::Id handleId,
                             const handles::WorldRay& startRay,
                             glm::vec3 shearWorldPos)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedShearPos(shearWorldPos),
          _savedAngle(loc->GetAngle()),
          _savedHeight(loc->GetHeight()),
          _savedShear(loc->GetShear()) {
        _planeNormal = glm::vec3(0.0f, 0.0f, 1.0f);
        _planePoint  = glm::vec3(0.0f, 0.0f, _savedShearPos.z);
        if (!Intersect(startRay, _savedIntersect)) {
            _savedIntersect = _savedShearPos;
        }
    }

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        glm::vec3 cur(0.0f);
        if (!Intersect(ray, cur)) return handles::UpdateResult::Unchanged;
        const glm::vec3 dragDelta = cur - _savedIntersect;

        glm::vec3 a = _loc->GetPoint2() - _loc->GetPoint1();
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 inv_rotation   = glm::inverse(rotationMatrix);
        glm::vec3 cent           = glm::vec3(inv_rotation * glm::vec4(_loc->GetCenterPosition(), 1.0f));
        glm::vec3 originLocal    = glm::vec3(inv_rotation * glm::vec4(_savedShearPos + dragDelta, 1.0f));

        const float posx = originLocal.x - cent.x;
        const float posy = originLocal.y - cent.y;
        float HandleHt = _loc->GetRenderHt();
        if (HandleHt > _loc->GetRenderWi()) HandleHt = _loc->GetRenderWi();
        float ymax = HandleHt;
        if (ymax < 0.01f) ymax = 0.01f;

        const bool shift = handles::HasModifier(mods, handles::Modifier::Shift);
        if (_loc->GetSupportsAngle()) {
            if (posy == 0.0f) return handles::UpdateResult::Unchanged;
            float tan = posx / posy;
            int angle1 = -static_cast<int>(glm::degrees(std::atan(tan)));
            int newAngle = 0;
            if (posy >= 0)        newAngle = angle1;
            else if (posx <= 0)   newAngle = 180 + angle1;
            else                  newAngle = -180 + angle1;
            if (shift) newAngle = (newAngle / 5) * 5;
            float length = std::sqrt(posy * posy + posx * posx);
            float newHeight = length / (HandleHt * _loc->GetScaleMatrix().y);
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetAngle(newAngle);
            _loc->SetHeight(newHeight);
        } else if (_loc->GetSupportsShear()) {
            glm::mat4 m = glm::inverse(_loc->GetModelMatrix());
            glm::vec3 v = glm::vec3(m * glm::vec4(_savedShearPos + dragDelta, 1.0f));
            const float renderWi = _loc->GetRenderWi();
            float newShear = _loc->GetShear();
            const float halfWi = renderWi / 2.0f;
            if (_loc->GetHeight() < 0.0f) {
                newShear -= (v.x - halfWi) / renderWi;
            } else {
                newShear += (v.x - halfWi) / renderWi;
            }
            if (newShear < -3.0f) newShear = -3.0f;
            else if (newShear > 3.0f) newShear = 3.0f;
            _loc->SetShear(newShear);
            float newHeight = posy / (HandleHt * _loc->GetScaleMatrix().y);
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetHeight(newHeight);
        } else {
            float newHeight = _loc->GetHeight() * posy / ymax;
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetHeight(newHeight);
        }
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetAngle(_savedAngle);
        _loc->SetHeight(_savedHeight);
        _loc->SetShear(_savedShear);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Shear : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    bool Intersect(const handles::WorldRay& ray, glm::vec3& out) const {
        return VectorMath::GetPlaneIntersect(
            ray.origin, ray.direction, _planePoint, _planeNormal, out);
    }

    ThreePointScreenLocation* _loc;
    std::string               _modelName;
    handles::Id               _handleId;
    glm::vec3                 _savedShearPos;
    int                       _savedAngle;
    float                     _savedHeight;
    float                     _savedShear;
    glm::vec3                 _savedIntersect{0.0f};
    glm::vec3                 _planePoint    {0.0f};
    glm::vec3                 _planeNormal   {0.0f, 0.0f, 1.0f};
    bool                      _changed = false;
};

class ThreePointShearSession : public handles::DragSession {
public:
    ThreePointShearSession(ThreePointScreenLocation* loc,
                           std::string modelName,
                           handles::Id handleId)
        : _loc(loc),
          _modelName(std::move(modelName)),
          _handleId(handleId),
          _savedAngle(loc->GetAngle()),
          _savedHeight(loc->GetHeight()),
          _savedShear(loc->GetShear()) {}

    handles::UpdateResult Update(const handles::WorldRay& ray,
                                  handles::Modifier mods) override {
        // Mirrors `ThreePointScreenLocation::MoveHandle`'s
        // SHEAR_HANDLE branch.
        const float posx = ray.origin.x - _loc->GetCenterPosition().x;
        const float posy = ray.origin.y - _loc->GetCenterPosition().y;
        float HandleHt = _loc->GetRenderHt();
        if (HandleHt > _loc->GetRenderWi()) HandleHt = _loc->GetRenderWi();
        float ymax = HandleHt;
        if (ymax < 0.01f) ymax = 0.01f;

        const bool shift = handles::HasModifier(mods, handles::Modifier::Shift);
        if (_loc->GetSupportsAngle()) {
            if (posy == 0.0f) return handles::UpdateResult::Unchanged;
            float tan = posx / posy;
            int angle1 = -static_cast<int>(glm::degrees(std::atan(tan)));
            if (_loc->GetX2() < 0.0f) {
                angle1 = -angle1;
            }
            int newAngle = 0;
            if (posy >= 0) {
                newAngle = angle1;
            } else if (posx <= 0) {
                newAngle = 90 + (90 + angle1);
            } else {
                newAngle = -90 - (90 - angle1);
            }
            if (shift) newAngle = (newAngle / 5) * 5;
            float length = std::sqrt(posy * posy + posx * posx);
            float newHeight = length / (HandleHt * _loc->GetScaleMatrix().y);
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetAngle(newAngle);
            _loc->SetHeight(newHeight);
        } else if (_loc->GetSupportsShear()) {
            // supportsShear branch (Tree-style models).
            // Mirror legacy: cursor → model-local via inverse
            // model matrix, then differential update of `shear`
            // gated by current height sign, plus height = posy /
            // (HandleHt * scaley). Clamp shear to ±3.
            glm::mat4 m = glm::inverse(_loc->GetModelMatrix());
            glm::vec3 v = glm::vec3(m * glm::vec4(ray.origin, 1.0f));
            const float renderWi = _loc->GetRenderWi();
            float newShear = _loc->GetShear();
            const float halfWi = renderWi / 2.0f;
            if (_loc->GetHeight() < 0.0f) {
                newShear -= (v.x - halfWi) / renderWi;
            } else {
                newShear += (v.x - halfWi) / renderWi;
            }
            if (newShear < -3.0f) newShear = -3.0f;
            else if (newShear > 3.0f) newShear = 3.0f;
            _loc->SetShear(newShear);
            float newHeight = posy / (HandleHt * _loc->GetScaleMatrix().y);
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetHeight(newHeight);
        } else {
            // Default branch — height-only scale by posy / ymax.
            float newHeight = _savedHeight * posy / ymax;
            if (std::fabs(newHeight) < 0.01f) {
                newHeight = newHeight < 0.0f ? -0.01f : 0.01f;
            }
            _loc->SetHeight(newHeight);
        }
        _changed = true;
        return handles::UpdateResult::NeedsInit;
    }

    void Revert() override {
        _loc->SetAngle(_savedAngle);
        _loc->SetHeight(_savedHeight);
        _loc->SetShear(_savedShear);
        _changed = false;
    }

    CommitResult Commit() override {
        return CommitResult{
            _modelName,
            _changed ? handles::DirtyField::Shear : handles::DirtyField::None
        };
    }

    handles::Id GetHandleId() const override { return _handleId; }

private:
    ThreePointScreenLocation* _loc;
    std::string               _modelName;
    handles::Id               _handleId;
    int                       _savedAngle;
    float                     _savedHeight;
    float                     _savedShear;
    bool                      _changed = false;
};

} // namespace

std::vector<handles::Descriptor> ThreePointScreenLocation::GetHandles(
    handles::ViewMode mode, handles::Tool tool,
    const handles::ViewParams& view) const {
    auto out = TwoPointScreenLocation::GetHandles(mode, tool, view);
    if (mode == handles::ViewMode::ThreeD) {
        // The shear sphere is drawn whenever the model is selected,
        // so the descriptor needs to be emitted unconditionally so
        // an initial click can land on it. When the shear isn't yet
        // active, the descriptor is selectionOnly — click promotes it
        // to active_handle = SHEAR_HANDLE (which also forces axis_tool
        // = TOOL_XY_TRANS via ThreePoint::SetActiveHandle). Once active
        // + XY_TRANS, the descriptor is draggable and routes to the
        // 3D shear session.
        if (_locked) return out;
        const bool draggable = (IsRole(active_handle, handles::Role::Shear) &&
                                axis_tool == MSLTOOL::TOOL_XY_TRANS);
        handles::Descriptor d;
        d.id.role  = handles::Role::Shear;
        d.id.index = SHEAR_HANDLE;
        d.worldPos = GetShearHandleWorldPosition();
        d.suggestedRadius = 5.0f;
        d.editable = true;
        d.selectionOnly = !draggable;
        out.push_back(d);
        return out;
    }
    if (mode != handles::ViewMode::TwoD) return out;
    if (_locked) return out;
    handles::Descriptor d;
    d.id.role = handles::Role::Shear;
    d.id.index = SHEAR_HANDLE;
    d.worldPos = GetShearHandleWorldPosition();
    d.suggestedRadius = 5.0f;
    d.editable = true;
    out.push_back(d);
    return out;
}

std::unique_ptr<handles::DragSession> ThreePointScreenLocation::CreateDragSession(
    const std::string& modelName,
    const handles::Id& id,
    const handles::WorldRay& startRay) {
    if (id.role == handles::Role::Shear) {
        if (_locked) return nullptr;
        if (id.index != SHEAR_HANDLE) return nullptr;
        if (IsRole(active_handle, handles::Role::Shear) && axis_tool == MSLTOOL::TOOL_XY_TRANS) {
            // 3D shear (XY_TRANS): cursor lives on an XY plane
            // through the shear sphere's saved world position.
            return std::make_unique<ThreePoint3DShearSession>(this, modelName, id, startRay,
                                                                GetShearHandleWorldPosition());
        }
        (void)startRay;
        return std::make_unique<ThreePointShearSession>(this, modelName, id);
    }
    return TwoPointScreenLocation::CreateDragSession(modelName, id, startRay);
}
