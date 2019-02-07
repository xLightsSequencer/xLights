#ifndef MODELSCREENLOCATION_H
#define MODELSCREENLOCATION_H


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

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define TOOL_TRANSLATE 0
#define TOOL_SCALE     1
#define TOOL_ROTATE    2
#define TOOL_XY_TRANS  3
#define NUM_TOOLS      4

#define UPGRADE_NOT_NEEDED 0
#define UPGRADE_SKIPPED    1
#define UPGRADE_EXEC_DONE  2
#define UPGRADE_EXEC_READ  3

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

namespace DrawGLUtils {
    class xlAccumulator;
    class xl3Accumulator;
}

class BezierCurveCubic3D;

class ModelScreenLocation
{
public:
    virtual void Read(wxXmlNode *node) = 0;
    virtual void Write(wxXmlNode *node) = 0;
    virtual int CheckUpgrade(wxXmlNode *node) = 0;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const = 0;
    virtual void TranslatePoint(float &x, float &y, float &z) const = 0;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const = 0;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const = 0;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle) const;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const = 0;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const = 0;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) = 0;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) = 0;

    virtual bool Rotate(int axis, float factor) = 0;
    virtual bool Scale(float factor) = 0;

    virtual void SelectHandle(int handle) = 0;
    virtual int GetSelectedHandle() = 0;
    virtual int GetNumHandles() = 0;
    virtual void SelectSegment(int segment) = 0;
    virtual int GetSelectedSegment() = 0;
    virtual bool HasCurve(int segment) = 0;
    virtual void SetCurve(int segment, bool create = true) = 0;
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) = 0;
    virtual void InsertHandle(int after_handle) = 0;
    virtual void DeleteHandle(int handle) = 0;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) = 0;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) = 0;
    virtual void DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const; // useful for hit test debugging
    void UpdateBoundingBox(float width, float height);

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const = 0;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) = 0;
    virtual bool IsCenterBased() const = 0;
    virtual float GetVScaleFactor() const {return 1.0;}

    virtual void SetPosition(float posx, float posy) = 0;
    virtual void AddOffset(float deltax, float deltay, float deltaz);

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) = 0;
    virtual glm::vec2 GetScreenPosition(int screenwidth, int screenheight, ModelPreview* preview, PreviewCamera* camera, float &sx, float &sy, float &sz) const;
    virtual float GetHcenterPos() const = 0;
    virtual float GetVcenterPos() const = 0;
    virtual void SetHcenterPos(float f) = 0;
    virtual void SetVcenterPos(float f) = 0;

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

    void SetRenderSize(float NewWi, float NewHt, float NewDp = 0.0f);
    bool IsLocked() const { return _locked; }
    void Lock(bool value = true) { _locked = value; }
    float GetRenderHt() const { return RenderHt; }
    float GetRenderWi() const { return RenderWi; }
    float GetRenderDp() const { return RenderDp; }
    float RenderHt, RenderWi, RenderDp;  // size of the rendered output
    int previewW, previewH;

    struct xlPoint {
        float x;
        float y;
        float z;
    };

    void SetDefaultMatrices() const;  // for models that draw themselves
    virtual void SetActiveHandle(int handle);
    int GetActiveHandle() const { return active_handle; }
    virtual void SetActiveAxis(int axis);
    int GetActiveAxis() const { return active_axis; }
    virtual void AdvanceAxisTool() { axis_tool += 1; axis_tool %= (NUM_TOOLS-1); }
    virtual void SetAxisTool(int mode) { axis_tool = mode; }
    bool DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch);
    void DrawAxisTool(glm::vec3& pos, DrawGLUtils::xl3Accumulator &va) const;
    void TranslateVector(glm::vec3& point) const;
    virtual int GetDefaultHandle() { return CENTER_HANDLE; }
    virtual int GetDefaultTool() { return TOOL_TRANSLATE; }
    virtual void MouseOverHandle(int handle);
    int GetNumSelectableHandles() const { return mSelectableHandles; }
    virtual bool IsXYTransHandle() const { return false; }
    bool GetSupportsZScaling() const { return supportsZScaling; }
    void SetSupportsZScaling(bool b) {
        supportsZScaling = b;
    }
    void SetStartOnXAxis(bool b) {
        _startOnXAxis = b;
    }
    glm::vec3 GetWorldPosition() const { return glm::vec3(worldPos_x, worldPos_y, worldPos_z); }
    glm::vec3 GetRotation() const { return glm::vec3(rotatex, rotatey, rotatez); }
    glm::vec3 GetScaleMatrix() const { return glm::vec3(scalex, scaley, scalez); }

protected:
    ModelScreenLocation(int points);
    virtual ~ModelScreenLocation() {};
    virtual wxCursor CheckIfOverAxisHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle) const;

    mutable float worldPos_x;
    mutable float worldPos_y;
    mutable float worldPos_z;
    mutable float scalex;
    mutable float scaley;
    mutable float scalez;
    int rotatex;
    int rotatey;
    int rotatez;
    mutable glm::mat4 ModelMatrix;
    mutable glm::mat4 TranslateMatrix;
    mutable glm::vec3 aabb_min;
    mutable glm::vec3 aabb_max;

    // used for handle movement
    glm::vec3 saved_intersect;
    glm::vec3 saved_position;
    glm::vec3 saved_size;
    glm::vec3 saved_scale;
    glm::vec3 saved_rotate;
    glm::vec3 drag_delta;

    mutable bool draw_3d;

    mutable std::vector<glm::vec3> handle_aabb_min;
    mutable std::vector<glm::vec3> handle_aabb_max;
    mutable std::vector<xlPoint> mHandlePosition;
    mutable glm::vec3 active_handle_pos;
    int mSelectableHandles;
    bool _locked;
    int active_handle;
    int highlighted_handle;
    int active_axis;
    int axis_tool;
    bool supportsZScaling;
    bool _startOnXAxis;
};

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual int CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(int axis, float factor) override;
    virtual bool Scale(float factor) override;

    virtual void SelectHandle(int handle) override {}
    virtual int GetSelectedHandle() override {return -1;}
    virtual int GetNumHandles() override {return -1;}
    virtual void SelectSegment(int segment) override {}
    virtual int GetSelectedSegment() override {return -1;}
    virtual bool HasCurve(int segment) override {return false;}
    virtual void SetCurve(int segment, bool create = true) override {}
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override {}
    virtual void InsertHandle(int after_handle) override {}
    virtual void DeleteHandle(int handle) override {}
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool IsCenterBased() const override {return true;};

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) override;

    virtual float GetHcenterPos() const override {
        return (float)worldPos_x;
    }
    virtual float GetVcenterPos() const override {
        return (float)worldPos_y;
    }
    virtual void SetHcenterPos(float f) override {
		worldPos_x = f;
    }
    virtual void SetVcenterPos(float f) override {
		worldPos_y = f;
    }

    virtual void SetPosition(float posx, float posy) override {
		worldPos_x = posx;
		worldPos_y = posy;
    }
    void SetScale(float x, float y) {
        scalex = x;
        scaley = y;
    }

    void SetRotation(int r) {
        rotatez = r;
    }
    void SetPerspective2D(float p) {
        perspective = p;
    }

    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual void SetMDepth(float d) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;

    int GetRotation() const {
        return rotatez;
    }

    float GetScaleX() { return scalex; }
    float GetScaleY() { return scaley; }

    virtual int GetDefaultHandle() override { return CENTER_HANDLE; }
    virtual int GetDefaultTool() override { return TOOL_SCALE; }

private:
    float perspective;

    mutable float centerx;
    mutable float centery;
};

//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation();

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual int CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(int axis, float factor) override;
    virtual bool Scale(float factor) override;
    virtual void SelectHandle(int handle) override {}
    virtual int GetSelectedHandle() override {return -1;}
    virtual int GetNumHandles() override {return -1;}
    virtual void SelectSegment(int segment) override {}
    virtual int GetSelectedSegment() override {return -1;}
    virtual bool HasCurve(int segment) override {return false;}
    virtual void SetCurve(int segment, bool create = true) override {}
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override {}
    virtual void InsertHandle(int after_handle) override {}
    virtual void DeleteHandle(int handle) override {}
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual bool IsCenterBased() const override {return false;};

    virtual void SetPosition(float posx, float posy) override;
    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    virtual void SetMDepth(float d) override;

    virtual float GetYShear() const {return 0.0;}

    virtual int GetDefaultHandle() override { return END_HANDLE; }
    virtual int GetDefaultTool() override { return TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(int mode) override;

protected:
    float x2, y2, z2;
    mutable glm::vec3 origin;
    mutable glm::vec3 point2;
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center;
    float saved_angle;
    bool minMaxSet;

    wxXmlNode *old;
    mutable glm::mat4 matrix;
};


class ThreePointScreenLocation : public TwoPointScreenLocation {
public:
    ThreePointScreenLocation();
    virtual ~ThreePointScreenLocation();
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual void DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const override; // useful for hit test debugging

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual float GetVScaleFactor() const override;
    virtual float GetYShear() const override;

    void SetHeight(float h) {
        height = h;
    }
    float GetHeight() const {
        return height;
    }
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    void SetModelHandleHeight(bool b) {
        modelHandlesHeight = b;
    }
    void SetSupportsShear(bool b) {
        supportsShear = b;
    }
    void SetSupportsAngle(bool b) {
        supportsAngle = b;
    }
    void SetAngle(int a) {
        angle = a;
    }
    int GetAngle() const {
        return angle;
    }

    virtual int GetDefaultHandle() override { return END_HANDLE; }
    virtual int GetDefaultTool() override { return TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(int mode) override;
    virtual void SetActiveAxis(int axis) override;
    virtual bool IsXYTransHandle() const override { return active_handle == SHEAR_HANDLE; }

private:
    bool modelHandlesHeight;
    bool supportsAngle;
    bool supportsShear;
    mutable glm::mat4 shearMatrix;
    float height;
    int angle;
    float shear;
};

//Location that uses multiple points
class PolyPointScreenLocation : public ModelScreenLocation {
public:
    PolyPointScreenLocation();
    virtual ~PolyPointScreenLocation();

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual int CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual void DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const override; // useful for hit test debugging
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(int axis, float factor) override;
    virtual bool Scale(float factor) override;
    virtual void SelectHandle(int handle) override;
    virtual int GetSelectedHandle() override {return selected_handle;}
    virtual int GetNumHandles() override {return num_points;}
    virtual void SelectSegment(int segment) override;
    virtual int GetSelectedSegment() override {return selected_segment;}
    virtual bool HasCurve(int segment) override {return mPos[segment].has_curve;}
    virtual void SetCurve(int seg_num, bool create = true) override;
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override;
    virtual void InsertHandle(int after_handle) override;
    virtual void DeleteHandle(int handle) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual bool IsCenterBased() const override {return false;};

    virtual void SetPosition(float posx, float posy) override;
    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual void SetMDepth(float d) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;

    virtual int GetDefaultHandle() override { return END_HANDLE; }
    virtual int GetDefaultTool() override { return TOOL_XY_TRANS; }
    virtual float GetYShear() const {return 0.0;}
    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(int mode) override;
    virtual void SetActiveAxis(int axis) override;

protected:
    struct xlPolyPoint {
        float x;
        float y;
        float z;
        mutable xlPoint cp0;
        mutable xlPoint cp1;
        mutable bool has_curve;
        mutable BezierCurveCubic3D* curve;
        mutable glm::mat4 *matrix;
        mutable glm::mat4 *mod_matrix;
        mutable float seg_scale;
    };
    mutable std::vector<xlPolyPoint> mPos;
    int num_points;
    int selected_handle;
    mutable std::mutex _mutex;
    mutable float minX, minY, maxX, maxY, minZ, maxZ;
    mutable int selected_segment;
    mutable std::vector<glm::vec3> seg_aabb_min;
    mutable std::vector<glm::vec3> seg_aabb_max;
    mutable glm::mat4 main_matrix;
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center;
    float saved_angle;
    void FixCurveHandles();
    void AdjustAllHandles(glm::mat4& mat);
};

#endif // MODELSCREENLOCATION_H

