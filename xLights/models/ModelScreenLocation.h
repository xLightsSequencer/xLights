#ifndef MODELSCREENLOCATION_H
#define MODELSCREENLOCATION_H


#define OVER_NO_HANDLE              -1
#define OVER_L_TOP_HANDLE           0
#define OVER_L_TOP_HANDLE_Z         5
#define OVER_R_TOP_HANDLE           1
#define OVER_R_TOP_HANDLE_Z         6
#define OVER_L_BOTTOM_HANDLE        2
#define OVER_L_BOTTOM_HANDLE_Z      7
#define OVER_R_BOTTOM_HANDLE        3
#define OVER_R_BOTTOM_HANDLE_Z      8
#define OVER_ROTATE_HANDLE          4
#define OVER_CENTER_HANDLE          9

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define TOOL_TRANSLATE 0
#define TOOL_SCALE     1
#define TOOL_ROTATE    2

class wxXmlNode;
class ModelPreview;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxCursor;

#include <vector>
#include "Node.h"
#include "Shapes.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>

namespace DrawGLUtils {
    class xlAccumulator;
    class xl3Accumulator;
}

class ModelScreenLocation
{
public:
    virtual void Read(wxXmlNode *node) = 0;
    virtual void Write(wxXmlNode *node) = 0;

    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) = 0;
    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const = 0;
    virtual void TranslatePoint(float &x, float &y, float &z) const = 0;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(int x,int y) const = 0;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const = 0;
	virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const = 0;
	virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const = 0;
	virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) = 0;
    virtual void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch) = 0;
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

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const = 0;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) = 0;
    virtual bool IsCenterBased() const = 0;
    virtual float GetVScaleFactor() const {return 1.0;}

    virtual void SetOffset(float xPct, float yPct) = 0;
    virtual void AddOffset(float xPct, float yPct, float zPct) = 0;

    virtual float GetHcenterOffset() const = 0;
    virtual float GetVcenterOffset() const = 0;
    virtual void SetHcenterOffset(float f) = 0;
    virtual void SetVcenterOffset(float f) = 0;

    //in screen coordinates
    virtual int GetTop() const = 0;
    virtual int GetLeft() const = 0;
    virtual int GetRight() const = 0;
    virtual int GetBottom() const = 0;
    virtual void SetTop(int i) = 0;
    virtual void SetLeft(int i) = 0;
    virtual void SetRight(int i) = 0;
    virtual void SetBottom(int i) = 0;
    virtual void SetMWidth(int w) = 0;
    virtual void SetMHeight(int h) = 0;
    virtual int GetMWidth() const = 0;
    virtual int GetMHeight() const = 0;

    void SetRenderSize(float NewWi, float NewHt) {
        RenderHt=NewHt;
        RenderWi=NewWi;
    }
    bool IsLocked() const { return _locked; }
    void Lock(bool value = true) { _locked = value; }
    float GetRenderHt() const { return RenderHt; }
    float GetRenderWi() const { return RenderWi; }
    float RenderHt, RenderWi;  // size of the rendered output
    int previewW, previewH;

    glm::mat4& GetModelMatrix() { return ModelMatrix; }
    glm::vec3& GetAABB_Min() { return aabb_min; }
    glm::vec3& GetAABB_Max() { return aabb_max; }

    struct xlPoint {
        float x;
        float y;
        float z;
    };

    std::vector<xlPoint>& GetHandlePositions() { return mHandlePosition; }
    void SetActiveHandle(int handle) { active_handle = handle; }
    int GetActiveHandle() { return active_handle; }
    void SetActiveAxis(int axis) { active_axis = axis; }
    int GetActiveAxis() { return active_axis; }
    void AdvanceAxisTool() { axis_tool += 1; axis_tool %= 2; }
    void SetAxisTool(int mode) { axis_tool = mode; }
    bool DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch);
    void DrawAxisTool(float x, float y, float z, DrawGLUtils::xl3Accumulator &va) const;
 
protected:
    ModelScreenLocation(int points);
    virtual ~ModelScreenLocation() {};

    float worldPos_x;
    float worldPos_y;
    float worldPos_z;
    float scalex;
    float scaley;
    float scalez;
    mutable glm::mat4 ModelMatrix;
    glm::vec3 aabb_min;
    glm::vec3 aabb_max;

    // used for handle movement
    glm::vec3 saved_intersect;  
    glm::vec3 saved_position;
    glm::vec3 saved_size;
    glm::vec3 saved_scale;
    glm::vec3 drag_delta;

    mutable bool draw_3d;

    mutable std::vector<xlPoint> mHandlePosition;
    bool _locked;
    int active_handle;
    int active_axis;
    int axis_tool;
};

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch);
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

    virtual float GetHcenterOffset() const override {
        return (float)worldPos_x;
    }
    virtual float GetVcenterOffset() const override {
        return (float)worldPos_y;
    }
    virtual void SetHcenterOffset(float f) override {
		worldPos_x = f;
    }
    virtual void SetVcenterOffset(float f) override {
		worldPos_y = f;
    }

    virtual void SetOffset(float xPct, float yPct) override {
		worldPos_x =xPct;
		worldPos_y =yPct;
    }
    void SetScale(float x, float y) {
        scalex = x;
        scaley = y;
    }
    void BoxedScreenLocation::AddOffset(float xPct, float yPct, float zPct);

    void SetRotation(int r) {
        PreviewRotation = r;
    }
    void SetPerspective2D(float p) {
        perspective = p;
    }

    virtual int GetTop() const override;
    virtual int GetLeft() const override;
    virtual int GetRight() const override;
    virtual int GetBottom() const override;
    virtual void SetTop(int i) override;
    virtual void SetLeft(int i) override;
    virtual void SetRight(int i) override;
    virtual void SetBottom(int i) override;
    virtual void SetMWidth(int w) override;
    virtual void SetMHeight(int h) override;
    virtual int GetMWidth() const override;
    virtual int GetMHeight() const override;

    int GetRotation() const {
        return PreviewRotation;
    }

    float GetScaleX() { return scalex; }
    float GetScaleY() { return scaley; }

private:
    int PreviewRotation;
    float perspective;

    mutable float radians;
    mutable float centerx;
    mutable float centery;

    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
};

//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation();

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch);
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

    virtual float GetHcenterOffset() const override;
    virtual float GetVcenterOffset() const override;
    virtual void SetHcenterOffset(float f) override;
    virtual void SetVcenterOffset(float f) override;
    virtual bool IsCenterBased() const override {return false;};

    virtual void SetOffset(float xPct, float yPct) override;
    virtual void AddOffset(float xPct, float yPct, float zPct) override;
    virtual int GetTop() const override;
    virtual int GetLeft() const override;
    virtual int GetRight() const override;
    virtual int GetBottom() const override;
    virtual void SetTop(int i) override;
    virtual void SetLeft(int i) override;
    virtual void SetRight(int i) override;
    virtual void SetBottom(int i) override;
    virtual void SetMWidth(int w) override;
    virtual void SetMHeight(int h) override;
    virtual int GetMWidth() const override;
    virtual int GetMHeight() const override;

    virtual float GetYShear() const {return 0.0;}
    void SetYMinMax(float min, float max) {
        minMaxSet = true;
        ymin = min;
        ymax = max;
    }
    void FlipCoords();

protected:
    virtual void ProcessOldNode(wxXmlNode *n);

    float x1, y1;
    float x2, y2;
    float ymin, ymax;
    bool minMaxSet;

    wxXmlNode *old;
    mutable glm::mat3 *matrix;
};


class ThreePointScreenLocation : public TwoPointScreenLocation {
public:
    ThreePointScreenLocation();
    virtual ~ThreePointScreenLocation();
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual float GetVScaleFactor() const override;
    virtual float GetYShear() const override;

    void SetHeight(float h) {
        height = h;
    }
    float GetHeight() const {
        return height;
    }
    virtual void SetMWidth(int w) override;
    virtual void SetMHeight(int h) override;
    virtual int GetMWidth() const override;
    virtual int GetMHeight() const override;
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
protected:
    virtual void ProcessOldNode(wxXmlNode *n) override;
private:
    bool modelHandlesHeight;
    bool supportsAngle;
    bool supportsShear;
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

    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual void DrawHandles(DrawGLUtils::xl3Accumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual void MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch);
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

    virtual float GetHcenterOffset() const override;
    virtual float GetVcenterOffset() const override;
    virtual void SetHcenterOffset(float f) override;
    virtual void SetVcenterOffset(float f) override;
    virtual bool IsCenterBased() const override {return false;};

    virtual void SetOffset(float xPct, float yPct) override;
    virtual void AddOffset(float xPct, float yPct, float zPct) override;
    virtual int GetTop() const override;
    virtual int GetLeft() const override;
    virtual int GetRight() const override;
    virtual int GetBottom() const override;
    virtual void SetTop(int i) override;
    virtual void SetLeft(int i) override;
    virtual void SetRight(int i) override;
    virtual void SetBottom(int i) override;
    virtual void SetMWidth(int w) override;
    virtual void SetMHeight(int h) override;
    virtual int GetMWidth() const override;
    virtual int GetMHeight() const override;

    virtual float GetYShear() const {return 0.0;}

protected:
    struct xlPolyPoint {
        float x;
        float y;
        mutable xlPoint cp0;
        mutable xlPoint cp1;
        mutable bool has_curve;
        mutable BezierCurveCubic* curve;
        mutable glm::mat3 *matrix;
    };
    mutable std::vector<xlPolyPoint> mPos;
    int num_points;
    int selected_handle;
    mutable float minX, minY, maxX, maxY;
    mutable int selected_segment;
    mutable glm::mat3 *main_matrix;
    void FixCurveHandles();

};

#endif // MODELSCREENLOCATION_H

