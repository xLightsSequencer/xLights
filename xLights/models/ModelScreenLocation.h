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

#define CUR_MODEL_POS_VER      "5"

#define NO_HANDLE              -1
#define CENTER_HANDLE          0
#define L_TOP_HANDLE           1
#define R_TOP_HANDLE           2
#define R_BOT_HANDLE           3
#define L_BOT_HANDLE           4
#define ROTATE_HANDLE          5
#define L_TOP_HANDLE_Z         6
#define R_TOP_HANDLE_Z         7
#define R_BOT_HANDLE_Z         8
#define L_BOT_HANDLE_Z         9
#define START_HANDLE           1
#define END_HANDLE             2
#define SHEAR_HANDLE           3

#define FROM_BASE_HANDLES_COLOUR xlPURPLETRANSLUCENT
#define LOCKED_HANDLES_COLOUR xlREDTRANSLUCENT

// Lower 20 bits reserved to store handle positions and these
// constants are modifiers to indicate special handles
#define HANDLE_MASK    0x00FFFFF
#define HANDLE_SEGMENT 0x1000000
#define HANDLE_AXIS    0x0200000
#define HANDLE_CP0     0x0400000
#define HANDLE_CP1     0x0800000

class wxXmlNode;
class ModelPreview;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxCursor;
class PreviewCamera;

#include <shared_mutex>
#include <vector>
#include "Node.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class BezierCurveCubic3D;
class xlGraphicsProgram;
class xlGraphicsContext;
class xlVertexColorAccumulator;

class ModelScreenLocation
{
protected:
    float GetAxisArrowLength(float zoom, int scale) const;
    float GetAxisHeadLength(float zoom, int scale) const;
    float GetAxisRadius(float zoom, int scale) const;
    float GetRectHandleWidth(float zoom, int scale) const;

    public:

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

    enum class MSLUPGRADE {
        MSLUPGRADE_NOT_NEEDED,
        MSLUPGRADE_SKIPPED,
        MSLUPGRADE_EXEC_DONE,
        MSLUPGRADE_EXEC_READ
    };

    MSLAXIS NextAxis(MSLAXIS axis);
    virtual void Read(wxXmlNode* node) = 0;
    virtual void Write(wxXmlNode *node) = 0;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode *node) = 0;
    void Reload() { rotation_init = true; }

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const = 0;
    virtual void TranslatePoint(float &x, float &y, float &z) const = 0;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const = 0;

    virtual std::string GetDimension(float factor = 1.0) const = 0;
    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const = 0;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const = 0;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const;

    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const { return false; };
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const { return false; };
    void DrawAxisTool(glm::vec3& pos, xlGraphicsProgram *program, float zoom, int scale) const;

    
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) = 0;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) = 0;
    virtual int MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) = 0;
    virtual void MouseDown(bool value) { mouse_down = value; }

    virtual bool Rotate(MSLAXIS axis, float factor) = 0;
    virtual bool Scale(const glm::vec3& factor) = 0;

    virtual void SelectHandle(int handle) {}
    virtual int GetSelectedHandle() const {return NO_HANDLE;}
    virtual int GetNumHandles() const {return NO_HANDLE;}
    virtual void SelectSegment(int segment) {}
    virtual int GetSelectedSegment() const {return NO_HANDLE;}
    virtual bool SupportsCurves() const {return false;}
    virtual bool HasCurve(int segment) const {return false;}
    virtual void SetCurve(int segment, bool create = true) {}
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) {}
    virtual void InsertHandle(int after_handle, float zoom, int scale) {}
    virtual void DeleteHandle(int handle) {}
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) = 0;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) = 0;
    virtual void UpdateBoundingBox(float width, float height, float depth);

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const = 0;
    virtual void AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor = 1.0) const = 0;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) = 0;
    virtual bool IsCenterBased() const = 0;
    virtual float GetVScaleFactor() const {return 1.0;}

    virtual void SetPosition(float posx, float posy) = 0;
    virtual void AddOffset(float deltax, float deltay, float deltaz);

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const = 0;
    virtual glm::vec2 GetScreenPosition(int screenwidth, int screenheight, ModelPreview* preview, PreviewCamera* camera, float &sx, float &sy, float &sz) const;
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
    void AdjustRenderSize(float NewWi, float NewHt, float NewDp, wxXmlNode* node);
    bool IsLocked() const { return _locked; }
    void Lock(bool value = true) { _locked = value; }
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

    void AddASAPWork(uint32_t work, const std::string& from);
    void SetDefaultMatrices() const;  // for models that draw themselves
    virtual void SetActiveHandle(int handle);
    int GetActiveHandle() const { return active_handle; }
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
    bool DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch);
    void TranslateVector(glm::vec3& point) const;
    virtual int GetDefaultHandle() const { return CENTER_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const { return MSLTOOL::TOOL_TRANSLATE; }
    virtual void MouseOverHandle(int handle);
    int GetNumSelectableHandles() const { return mSelectableHandles; }
    virtual bool IsXYTransHandle() const { return false; }
    virtual bool IsElevationHandle() const { return false; }
    bool GetSupportsZScaling() const { return supportsZScaling; }
    void SetSupportsZScaling(bool b) {
        supportsZScaling = b;
    }
    MSLPLANE GetPreferredSelectionPlane() { return preferred_selection_plane; }
    void SetPreferredSelectionPlane( MSLPLANE plane ) { preferred_selection_plane = plane; }
    void SetActivePlane( MSLPLANE plane ) { active_plane = plane; }
    void FindPlaneIntersection( int x, int y, ModelPreview* preview );
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
    glm::vec3 GetActiveHandlePosition() const { return active_handle_pos; }
    virtual glm::vec3 GetHandlePosition(int handle) const;
    glm::vec3 GetRotationAngles() const { return angles; }
    glm::mat4 GetModelMatrix() const { return ModelMatrix; }

protected:
    ModelScreenLocation(int points);
    virtual ~ModelScreenLocation() {};
    virtual wxCursor CheckIfOverAxisHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const;
    MSLPLANE GetBestIntersection( MSLPLANE prefer, bool& rotate, ModelPreview* preview );

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

    // used for handle movement
    glm::vec3 saved_intersect = glm::vec3(0.0f);
    glm::vec3 saved_position = glm::vec3(0.0f);
    glm::vec3 saved_size = glm::vec3(0.0f);
    glm::vec3 saved_scale = glm::vec3(1.0f);
    glm::vec3 saved_rotate = glm::vec3(0.0);
    glm::vec3 drag_delta = glm::vec3(0.0);
    glm::vec3 angles = glm::vec3(0.0);

    mutable bool draw_3d = false;

    mutable std::vector<glm::vec3> handle_aabb_min;
    mutable std::vector<glm::vec3> handle_aabb_max;
    mutable std::vector<xlPoint> mHandlePosition;
    mutable glm::vec3 active_handle_pos = glm::vec3(0.0f);
    int mSelectableHandles = 0;
    bool _locked = false;
    int active_handle = -1;
    int highlighted_handle = -1;
    MSLAXIS active_axis = MSLAXIS::NO_AXIS;
    MSLTOOL axis_tool = MSLTOOL::TOOL_TRANSLATE;
    int tool_size = 1;
    bool supportsZScaling = false;
    bool createWithDepth =  false;
    bool _startOnXAxis = false;
    bool rotation_init = true;
    bool mouse_down = false;
    MSLPLANE preferred_selection_plane = MSLPLANE::XY_PLANE;
    MSLPLANE active_plane = MSLPLANE::NO_PLANE;
};
