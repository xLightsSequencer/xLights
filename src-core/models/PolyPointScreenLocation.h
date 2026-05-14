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

#include "ModelScreenLocation.h"

//Location that uses multiple points
class PolyPointScreenLocation : public ModelScreenLocation {
public:
    PolyPointScreenLocation();
    virtual ~PolyPointScreenLocation();

    friend class MultiPointModel;
    friend class PolyLineModel;

    virtual void Init() override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float& x, float& y, float& z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const override;

    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;

    [[nodiscard]] std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& id) override;
    // SpaceMouse per-handle helpers. Public so the session class
    // in the .cpp can call them without needing friend status.
    void ApplySpaceMouseCenter(float scale, const glm::vec3& rot, const glm::vec3& mov);
    void ApplySpaceMouseCurveCp(int segment, int cpIndex, float scale, const glm::vec3& mov);
    void ApplySpaceMouseVertex(int vertexIndex, float scale, const glm::vec3& mov);
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;
    virtual void SelectHandle(int handle) override;
    virtual std::optional<handles::Id> GetSelectedHandleId() const override {
        return selected_handle;
    }
    virtual int GetNumHandles() const override { return num_points; }
    virtual void SelectSegment(int segment) override;
    virtual int GetSelectedSegment() const override { return selected_segment; }
    virtual bool SupportsCurves() const override {return true;}
    virtual bool HasCurve(int segment) const override { return mPos[segment].has_curve; }
    virtual void SetCurve(int seg_num, bool create = true) override;
    virtual void AddHandle(IModelPreview* preview, int mouseX, int mouseY) override;
    virtual void InsertHandle(int after_handle, float zoom, int scale) override;
    virtual void DeleteHandle(int handle) override;
    virtual CursorType InitializeLocation(int& handle, int x, int y, const std::vector<NodeBaseClassPtr>& Nodes, IModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr>& Node) override;

    virtual std::string GetDimension(float factor = 1.0) const override;
    
    virtual glm::vec2 GetScreenOffset(IModelPreview* preview) const override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual float GetDcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual void SetDcenterPos(float f) override;
    virtual bool IsCenterBased() const override { return false; };

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
    virtual float GetMDepth() const override;
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle) override;

    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_XY_TRANS; }
    void SetActiveHandleToDefault() override {
        // Legacy END_HANDLE = 2 → Vertex(index 1), the second
        // vertex placed by polyline create. Lines up with what
        // BeginCreate / GetActiveHandleId expect post-placement.
        handles::Id id;
        id.role = handles::Role::Vertex;
        id.index = 1;
        SetActiveHandle(std::optional<handles::Id>(id));
    }
    virtual float GetYShear() const { return 0.0; }
    virtual void SetActiveHandle(const std::optional<handles::Id>& id) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;

    int GetNumPoints() const { return num_points; }
    void SetNumPoints(int points) { num_points = points; }
    // Legacy int setter — used by callers that still speak `int`. -1
    // clears; vertex-style indices are mapped to `Role::Vertex`.
    void SetSelectedHandle(int h);
    void SetSelectedSegment(int s) { selected_segment = s; }
    glm::vec3 GetPoint(int i) const { return glm::vec3(mPos[i].x, mPos[i].y, mPos[i].z); }
    void SetPoint(int i, float x, float y, float z) { mPos[i].x = x; mPos[i].y = y; mPos[i].z = z; }
    void SetPoint(int i, const glm::vec3& p) { mPos[i].x = p.x; mPos[i].y = p.y; mPos[i].z = p.z; }
    // Public helpers for the new DragSession API.
    bool HasSegmentCurve(int segment) const { return segment >= 0 && segment < num_points && mPos[segment].curve != nullptr; }
    glm::vec3 GetCurveCp0(int segment) const;
    glm::vec3 GetCurveCp1(int segment) const;
    void SetCurveCp0(int segment, const glm::vec3& world);
    void SetCurveCp1(int segment, const glm::vec3& world);
    void FixCurveHandlesPublic() { FixCurveHandles(); }
    void ApplyAffineToAllHandles(glm::mat4& mat) { AdjustAllHandles(mat); }
    float GetMinX() const { return minX; }
    float GetMaxX() const { return maxX; }
    float GetMinY() const { return minY; }
    float GetMaxY() const { return maxY; }
    void SetRotatePoint(const glm::vec3& p) { rotate_pt = p; }

    // Logical world position of the currently-active sub-handle.
    // Resolves CP0/CP1/CENTER/vertex from `active_handle`'s bit-packed
    // value. Replaces the legacy `active_handle_pos` cache (which was
    // only valid after DrawHandles had run).
    glm::vec3 GetActiveSubHandleWorldPos() const;

    [[nodiscard]] std::vector<handles::Descriptor> GetHandles(
        handles::ViewMode mode, handles::Tool tool,
        const handles::ViewParams& view = {}) const override;
    std::unique_ptr<handles::DragSession> CreateDragSession(
        const std::string& modelName,
        const handles::Id& id,
        const handles::WorldRay& startRay) override;
    [[nodiscard]] std::unique_ptr<handles::DragSession> BeginCreate(
        const std::string& modelName,
        const handles::WorldRay& clickRay,
        handles::ViewMode mode) override;
    // polyline extension. Each click after the first
    // appends a vertex (`AddHandle`) and starts a fresh drag on
    // that new vertex. Caller is responsible for the AddHandle
    // call; this just opens a drag session on the named index.
    [[nodiscard]] std::unique_ptr<handles::DragSession> BeginExtend(
        const std::string& modelName,
        const handles::WorldRay& clickRay,
        handles::ViewMode mode,
        int vertexIndex);
    void SetDataFromString(const std::string& point_data);
    void SetCurveDataFromString(const std::string& cpoint_data);
    std::string GetPointDataAsString() const;
    std::string GetCurveDataAsString() const;

protected:
    struct xlPolyPoint {
        float x;
        float y;
        float z;
        float length;
        mutable xlPoint cp0;
        mutable xlPoint cp1;
        mutable bool has_curve;
        mutable BezierCurveCubic3D* curve;
        mutable glm::mat4* matrix;
        mutable glm::mat4* mod_matrix;
        mutable float seg_scale;

        glm::vec3 AsVector() const { return glm::vec3(x, y, z); }
    };
    mutable std::vector<xlPolyPoint> mPos;
    int num_points = 0;
    std::optional<handles::Id> selected_handle;
    mutable std::mutex _mutex;
    mutable float minX = 0.0f;
    mutable float minY = 0.0f;
    mutable float maxX = 0.0f;
    mutable float maxY = 0.0f;
    mutable float minZ = 0.0f;
    mutable float maxZ = 0.0f;
    mutable int selected_segment;
    mutable std::vector<glm::vec3> seg_aabb_min;
    mutable std::vector<glm::vec3> seg_aabb_max;
    mutable glm::mat4 main_matrix;
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center;
    mutable glm::vec3 rotate_pt;
    float saved_angle = 0.0f;
    void FixCurveHandles();
    void AdjustAllHandles(glm::mat4& mat);
};

