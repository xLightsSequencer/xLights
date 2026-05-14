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

#define CUR_MODEL_POS_VER      "8"

#define NO_HANDLE              -1
#define CENTER_HANDLE          0
#define L_TOP_HANDLE           1
#define R_TOP_HANDLE           2
#define R_BOT_HANDLE           3
#define L_BOT_HANDLE           4
#define ROTATE_HANDLE          5
#define START_HANDLE           1
#define END_HANDLE             2
#define SHEAR_HANDLE           3

#define FROM_BASE_HANDLES_COLOUR xlPURPLETRANSLUCENT
#define LOCKED_HANDLES_COLOUR xlREDTRANSLUCENT

#define MODEL_UNCHANGED       0
#define MODEL_NEEDS_INIT      1
#define MODEL_UPDATE_RGBEFFECTS 2

// Lower 20 bits reserved to store handle positions and these
// constants are modifiers to indicate special handles
#define HANDLE_MASK    0x00FFFFF
#define HANDLE_SEGMENT 0x1000000
#define HANDLE_AXIS    0x0200000
#define HANDLE_CP0     0x0400000
#define HANDLE_CP1     0x0800000

class IModelPreview;
class PreviewCamera;

#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>
#include "../utils/CursorType.h"
#include "handles/Handles.h"
#include "handles/DragSession.h"
#include "handles/SpaceMouseSession.h"

// Match helpers for std::optional<handles::Id>. Used by DrawHandles
// to colour highlighted handles, by GetHandles emission to gate per-
// active-handle descriptors, etc. Returning false for a nullopt id
// is the common "no handle" case.
inline bool IsRole(const std::optional<handles::Id>& h, handles::Role role) {
    return h && h->role == role;
}
inline bool IsHandle(const std::optional<handles::Id>& h, handles::Role role, int index) {
    return h && h->role == role && h->index == index;
}
inline bool IsHandle(const std::optional<handles::Id>& h, handles::Role role, int index, int segment) {
    return h && h->role == role && h->index == index && h->segment == segment;
}
inline bool IsAxisHandle(const std::optional<handles::Id>& h, handles::Axis axis) {
    if (!h) return false;
    return (h->role == handles::Role::AxisArrow ||
            h->role == handles::Role::AxisCube  ||
            h->role == handles::Role::AxisRing) &&
           h->axis == axis;
}

#include "Node.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class BezierCurveCubic3D;
class OutputModelManager;
class xlGraphicsProgram;
class xlGraphicsContext;
class xlVertexColorAccumulator;

class ModelScreenLocation
{
public:
    // exposed so the layout front-end can size new-API
    // descriptor positions to match the legacy gizmo across the
    // camera's zoom range.
    float GetAxisArrowLength(float zoom, int scale) const;
    float GetAxisHeadLength(float zoom, int scale) const;
    float GetAxisRadius(float zoom, int scale) const;
    float GetRectHandleWidth(float zoom, int scale) const;

    enum class MSLPLANE {
        XY_PLANE,
        XZ_PLANE,
        YZ_PLANE,
        NO_PLANE,
        GROUND = XZ_PLANE
    };

    enum class MSLAXIS {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NO_AXIS
    };

    enum class MSLTOOL {
        TOOL_TRANSLATE,
        TOOL_SCALE,
        TOOL_ROTATE,
        TOOL_XY_TRANS,
        TOOL_ELEVATE,
        TOOL_NONE
    };

    MSLAXIS NextAxis(MSLAXIS axis);
    virtual void Init() = 0;
    void Reload() { rotation_init = true; }

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const = 0;
    virtual void TranslatePoint(float &x, float &y, float &z) const = 0;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const = 0;

    virtual std::string GetDimension(float factor = 1.0) const = 0;
    virtual bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const = 0;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const;

    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const { return false; };
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const { return false; };
protected:
    // Rendering helpers — only called from subclass DrawHandles
    // overrides. Public visibility would invite layout-side callers
    // that bypass the descriptor pipeline.
    void DrawAxisTool(const glm::vec3& pos, xlGraphicsProgram *program, float zoom, int scale) const;
    // Draws the long red/green/blue indicator line along `active_axis`
    // through `pos`. No-op when `active_axis == NO_AXIS`. Used by every
    // subclass's 3D DrawHandles to visualise placement / shear / etc.
    void DrawActiveAxisIndicator(glm::vec3 const& pos, xlGraphicsProgram* program) const;
public:

    // Descriptor pipeline hooks. Default impls return empty /
    // nullptr — every ScreenLocation subclass overrides as needed.
    // See `plans/handle-system-refactor.md`.
    [[nodiscard]] virtual std::vector<handles::Descriptor> GetHandles(
        handles::ViewMode /*mode*/, handles::Tool /*tool*/,
        const handles::ViewParams& /*view*/ = {}) const {
        return {};
    }
    virtual std::unique_ptr<handles::DragSession> CreateDragSession(
        const std::string& /*modelName*/,
        const handles::Id& /*id*/,
        const handles::WorldRay& /*startRay*/) {
        return nullptr;
    }

    // placement gesture. Caller must have already invoked
    // `InitializeLocation` to drop the model at the click point
    // and set per-axis state. The returned session drives the
    // subsequent move events.
    [[nodiscard]] virtual std::unique_ptr<handles::DragSession> BeginCreate(
        const std::string& /*modelName*/,
        const handles::WorldRay& /*clickRay*/,
        handles::ViewMode /*mode*/) {
        return nullptr;
    }


    // SpaceMouse 6-DOF input. Mouse-driven gizmo drags go through the
    // descriptor session API (`CreateDragSession` + `BeginCreate`)
    // and never hit this path. `id` identifies which body handle
    // the SpaceMouse is acting on (typically CentreCycle for a
    // whole-model translate+rotate). Default impl returns nullptr;
    // each ScreenLocation subclass overrides to wire up its own
    // handle-specific math.
    virtual std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& /*id*/) {
        return nullptr;
    }
    virtual void MouseDown(bool value) { mouse_down = value; }

    virtual bool Rotate(MSLAXIS axis, float factor) = 0;
    virtual bool Scale(const glm::vec3& factor) = 0;

    virtual void SelectHandle(int handle) {}
    // Id-returning accessor, mirrors GetActiveHandleId(). Only
    // PolyPoint stores a separate selected_handle; everywhere else
    // returns nullopt.
    virtual std::optional<handles::Id> GetSelectedHandleId() const { return std::nullopt; }
    virtual int GetNumHandles() const {return NO_HANDLE;}
    virtual void SelectSegment(int segment) {}
    virtual int GetSelectedSegment() const {return NO_HANDLE;}
    virtual bool SupportsCurves() const {return false;}
    virtual bool HasCurve(int segment) const {return false;}
    virtual void SetCurve(int segment, bool create = true) {}
    virtual void AddHandle(IModelPreview* preview, int mouseX, int mouseY) {}
    virtual void InsertHandle(int after_handle, float zoom, int scale) {}
    virtual void DeleteHandle(int handle) {}
    virtual CursorType InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) = 0;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) = 0;
    virtual void UpdateBoundingBox(float width, float height, float depth);

    virtual bool IsCenterBased() const = 0;
    virtual float GetVScaleFactor() const {return 1.0;}

    virtual void SetPosition(float posx, float posy) = 0;
    virtual void AddOffset(float deltax, float deltay, float deltaz);

    virtual glm::vec2 GetScreenOffset(IModelPreview* preview) const = 0;
    virtual glm::vec2 GetScreenPosition(int screenwidth, int screenheight, IModelPreview* preview, PreviewCamera* camera, float &sx, float &sy, float &sz) const;
    virtual float GetHcenterPos() const = 0;
    virtual float GetVcenterPos() const = 0;
    virtual float GetDcenterPos() const = 0;
    virtual void SetHcenterPos(float f) = 0;
    virtual void SetVcenterPos(float f) = 0;
    virtual void SetDcenterPos(float f) = 0;

    //in screen coordinates
    virtual float GetTop() const = 0;
    virtual float GetLeft() const = 0;
    virtual float GetRight() const = 0;
    virtual float GetBottom() const = 0;
    virtual float GetFront() const = 0;
    virtual float GetBack() const = 0;
    virtual void SetTop(float i) = 0;
    virtual void SetLeft(float i) = 0;
    virtual void SetRight(float i) = 0;
    virtual void SetBottom(float i) = 0;
    virtual void SetFront(float i) = 0;
    virtual void SetBack(float i) = 0;
    virtual void SetMWidth(float w) = 0;
    virtual void SetMHeight(float h) = 0;
    virtual void SetMDepth(float d) = 0;
    virtual float GetMWidth() const = 0;
    virtual float GetMHeight() const = 0;
    virtual float GetMDepth() const = 0;
    virtual float GetRestorableMWidth() const { return GetMWidth(); }
    virtual float GetRestorableMHeight() const { return GetMHeight(); }
    virtual float GetRestorableMDepth() const { return GetMDepth(); }
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle);
    virtual void SetEdit(bool val) { }
    virtual bool GetEdit() const { return false; }
    virtual void SetToolSize(int sz) { tool_size = sz; };
    int GetToolSize() const { return tool_size; }
    virtual void* GetRawData() { return nullptr; }
    virtual float GetRealWidth() const
    {
        return 0;
    }
    virtual float GetRealHeight() const
    {
        return 0;
    }
    virtual float GetRealDepth() const
    {
        return 0;
    }

    void SetRenderSize(float NewWi, float NewHt, float NewDp = 0.0f);
    void AdjustRenderSize(float NewWi, float NewHt, float NewDp);
    bool IsLocked() const { return _locked; }
    void SetLocked(bool value) { _locked = value; }
    void Lock(bool value = true) { _locked = value; }
    float GetWorldPos_X() const { return worldPos_x; }
    float GetWorldPos_Y() const { return worldPos_y; }
    float GetWorldPos_Z() const { return worldPos_z; }
    void SetWorldPos(float x, float y, float z) { worldPos_x = x; worldPos_y = y; worldPos_z = z; }
    void SetWorldPos_X(float x) { worldPos_x = x; }
    void SetWorldPos_Y(float y) { worldPos_y = y; }
    void SetWorldPos_Z(float z) { worldPos_z = z; }
    float GetRotateX() const { return rotatex; }
    float GetRotateY() const { return rotatey; }
    float GetRotateZ() const { return rotatez; }
    // All rotation setters funnel through SetRotation(vec3) so
    // rotate_quat stays in sync with rotatex/y/z. TranslatePoint and
    // GetHandles read the quat; bypassing it leaves the bounding
    // box / handles axis-aligned while the model rotates.
    void SetRotate(float x, float y, float z) { SetRotation(glm::vec3(x, y, z)); }
    void SetRotateX(float x) { SetRotation(glm::vec3(x, rotatey, rotatez)); }
    void SetRotateY(float y) { SetRotation(glm::vec3(rotatex, y, rotatez)); }
    void SetRotateZ(float z) { SetRotation(glm::vec3(rotatex, rotatey, z)); }
    float GetRenderHt() const { return RenderHt; }
    float GetRenderWi() const { return RenderWi; }
    float GetRenderDp() const { return RenderDp; }
    float RenderHt = 0.0f;
    float RenderWi = 0.0f;
    float RenderDp = 0.0f; // size of the rendered output
    int previewW = -1;
    int previewH = -1;

    struct xlPoint {
        float x;
        float y;
        float z;
    };

    void SetOutputModelManager(OutputModelManager* omm) { _outputModelManager = omm; }
    void AddASAPWork(uint32_t work, const std::string& from);
    void SetDefaultMatrices() const;  // for models that draw themselves
    // Axis-gizmo roles (AxisArrow / AxisCube / AxisRing) are modifiers
    // on the current body handle — storing one would break
    // `IsRole(active_handle, …)` checks in subclass GetHandles and
    // the gizmo would stop being emitted.
    virtual void SetActiveHandle(const std::optional<handles::Id>& id) {
        if (id && (id->role == handles::Role::AxisArrow ||
                   id->role == handles::Role::AxisCube ||
                   id->role == handles::Role::AxisRing)) {
            return;
        }
        active_handle = id;
        highlighted_handle.reset();
        SetAxisTool(axis_tool);
    }
    // "Select the body" — the new-Id form of the legacy
    // `SetActiveHandle(CENTER_HANDLE)` call. Defaults to a
    // CentreCycle Id; TwoPointScreenLocation overrides to use
    // Endpoint(CENTER) since TwoPoint's body marker has always
    // been the centre sphere between START and END.
    virtual void SetActiveHandleToCentre() {
        handles::Id id;
        id.role = handles::Role::CentreCycle;
        SetActiveHandle(std::optional<handles::Id>(id));
    }
    // "Select the default handle after model creation" — replaces
    // the legacy `SetActiveHandle(GetDefaultHandle())` pattern.
    // Most models (Boxed / Terrain): same as the centre body. Two-
    // and Three-Point default to the END endpoint sphere so the
    // freshly-placed model's gizmo lands on the drag target.
    // PolyPoint defaults to vertex 1 (the second vertex), matching
    // its legacy END_HANDLE meaning.
    virtual void SetActiveHandleToDefault() { SetActiveHandleToCentre(); }
    const std::optional<handles::Id>& GetActiveHandleId() const { return active_handle; }
    virtual void SetActiveAxis(MSLAXIS axis);
    MSLAXIS GetActiveAxis() const { return active_axis; }
    virtual void AdvanceAxisTool()
    {
        switch (axis_tool) {
        case MSLTOOL::TOOL_TRANSLATE:
            axis_tool = MSLTOOL::TOOL_SCALE;
            break;
        case MSLTOOL::TOOL_SCALE:
            axis_tool = MSLTOOL::TOOL_ROTATE;
            break;
        case MSLTOOL::TOOL_ROTATE:
        default:
            axis_tool = MSLTOOL::TOOL_TRANSLATE;
            break;
        }
    }
    virtual void SetAxisTool(MSLTOOL mode) { axis_tool = mode; }
    MSLTOOL GetAxisTool() const { return axis_tool; }
    // Ray-plane intersection helper. Builds a constraint plane based
    // on `axis_tool` + `active_axis` + `active_plane` (and an optional
    // `planePoint` to anchor the plane through), intersects the cursor
    // ray with it, and returns the intersect via the out-param.
    // Returns false if the ray misses the plane.
    bool DragHandle(IModelPreview* preview, int mouseX, int mouseY,
                    glm::vec3& outIntersect,
                    glm::vec3 planePoint = glm::vec3(0.0f));
    void TranslateVector(glm::vec3& point) const;
    virtual MSLTOOL GetDefaultTool() const { return MSLTOOL::TOOL_TRANSLATE; }
    virtual void MouseOverHandle(std::optional<handles::Id> handle);
    const std::optional<handles::Id>& GetHighlightedHandleId() const { return highlighted_handle; }
    int GetNumSelectableHandles() const { return mSelectableHandles; }
    virtual bool IsXYTransHandle() const { return false; }
    virtual bool IsElevationHandle() const { return false; }
    bool GetSupportsZScaling() const { return supportsZScaling; }
    void SetSupportsZScaling(bool b) { supportsZScaling = b; }
    [[nodiscard]] bool hasX2() const { return _hasX2; }

    MSLPLANE GetPreferredSelectionPlane() { return preferred_selection_plane; }
    void SetPreferredSelectionPlane( MSLPLANE plane ) { preferred_selection_plane = plane; }
    MSLPLANE GetActivePlane() const { return active_plane; }
    void SetActivePlane( MSLPLANE plane ) { active_plane = plane; }
    // Choose the best world plane (XY / XZ / YZ) for projecting a
    // drag based on the camera angles, then write it to
    // `active_plane`. Sessions that read `GetActivePlane()` (e.g.
    // PolyPointCreationSession) will then constrain drags to the
    // plane most aligned with the current view. Caller should
    // invoke right before opening a drag session.
    void RefreshActivePlaneFromCamera(IModelPreview* preview) {
        if (!preview) return;
        bool rotate = false;
        active_plane = GetBestIntersection(preferred_selection_plane, rotate, preview);
    }
    void FindPlaneIntersection( int x, int y, IModelPreview* preview );
    void CreateWithDepth(bool b) {
        createWithDepth = b;
    }
    void SetStartOnXAxis(bool b) {
        _startOnXAxis = b;
    }
    glm::vec3 GetWorldPosition() const { return glm::vec3(worldPos_x, worldPos_y, worldPos_z); }
    void SetWorldPosition(const glm::vec3& worldPos) { worldPos_x = worldPos.x; worldPos_y = worldPos.y; worldPos_z = worldPos.z; }
    glm::vec3 GetRotation() const { return glm::vec3(rotatex, rotatey, rotatez); }
    void SetRotation(const glm::vec3& rotate)
    {
        rotatex = rotate.x;
        rotatey = rotate.y;
        rotatez = rotate.z;

        glm::mat4 Identity(glm::mat4(1.0f));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        rotate_quat = glm::quat_cast(rz * ry * rx);
        rotation_init = false;
    }
    glm::quat GetRotationQuat() const { return rotate_quat; }
    glm::vec3 GetScaleMatrix() const { return glm::vec3(scalex, scaley, scalez); }
    void SetScaleMatrix(const glm::vec3& scale) const {
        scalex = scale.x; scaley = scale.y; scalez = scale.z;
    }
    glm::vec3 GetCenterPosition() const { return glm::vec3(GetHcenterPos(), GetVcenterPos(), GetDcenterPos()); }
    // External readers (e.g. LayoutPanel's multi-select rotate-
    // about-point math) get an on-demand answer via GetHandles().
    // The `active_handle_pos` field remains for DrawHandles' own
    // gizmo math, where it's recomputed locally each frame.
    // Id-keyed: iterates GetHandles() and matches by Id directly.
    // Returns (0,0,0) for nullopt or an Id not present in the
    // descriptor list.
    glm::vec3 GetHandlePositionById(const std::optional<handles::Id>& id) const;
    glm::vec3 GetActiveHandlePosition() const { return GetHandlePositionById(active_handle); }
    glm::vec3 GetRotationAngles() const { return angles; }
    glm::mat4 GetModelMatrix() const { return ModelMatrix; }

protected:
    ModelScreenLocation(int points);
    virtual ~ModelScreenLocation() {};
    MSLPLANE GetBestIntersection( MSLPLANE prefer, bool& rotate, IModelPreview* preview );

    mutable float worldPos_x = 0.0f;
    mutable float worldPos_y = 0.0f;
    mutable float worldPos_z = 0.0f;
    mutable float scalex = 1.0f;
    mutable float scaley = 1.0f;
    mutable float scalez = 1.0f;
    float rotatex = 0.0f;
    float rotatey = 0.0f;
    float rotatez = 0.0f;
    mutable glm::mat4 ModelMatrix;
    mutable glm::mat4 TranslateMatrix;
    mutable glm::quat rotate_quat = glm::quat(1.0, glm::vec3(0.0));
    mutable glm::vec3 aabb_min = glm::vec3(0.0f);
    mutable glm::vec3 aabb_max = glm::vec3(0.0f);

    glm::vec3 angles = glm::vec3(0.0);

    mutable bool draw_3d = false;

    int mSelectableHandles = 0;
    bool _locked = false;
    std::optional<handles::Id> active_handle;
    std::optional<handles::Id> highlighted_handle;
    MSLAXIS active_axis = MSLAXIS::NO_AXIS;
    MSLTOOL axis_tool = MSLTOOL::TOOL_TRANSLATE;
    int tool_size = 1;
    bool _hasX2 = false;
    bool supportsZScaling = false;
    bool createWithDepth =  false;
    bool _startOnXAxis = false;
    bool rotation_init = true;
    bool mouse_down = false;
    MSLPLANE preferred_selection_plane = MSLPLANE::XY_PLANE;
    MSLPLANE active_plane = MSLPLANE::NO_PLANE;
    OutputModelManager* _outputModelManager = nullptr;
};
