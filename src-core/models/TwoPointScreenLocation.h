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

//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation();

    virtual void Init() override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;

    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const;

    [[nodiscard]] std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& id) override;

    // SpaceMouse per-handle helpers. Public so the session class
    // in the .cpp can call them without needing friend status.
    void ApplySpaceMouseStartHandle(float scale, const glm::vec3& rot, const glm::vec3& mov);
    void ApplySpaceMouseEndHandle  (float scale, const glm::vec3& rot, const glm::vec3& mov);

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
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;

    virtual CursorType InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual glm::vec2 GetScreenOffset(IModelPreview* preview) const override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual float GetDcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual void SetDcenterPos(float f) override;
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
    virtual float GetMDepth() const override;
    virtual void SetMDepth(float d) override;
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle) override;
    void UpdateBoundingBox();

    virtual float GetYShear() const {return 0.0;}
    
    float GetX2() const {return x2;}
    float GetY2() const {return y2;}
    float GetZ2() const {return z2;}
    void SetX2(float val) {x2 = val;}
    void SetY2(float val) {y2 = val;}
    void SetZ2(float val) {z2 = val;}

    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    void SetActiveHandleToCentre() override;
    void SetActiveHandleToDefault() override {
        // END_HANDLE = Endpoint(2), the legacy GetDefaultHandle()
        // value for TwoPoint / ThreePoint.
        handles::Id id;
        id.role = handles::Role::Endpoint;
        id.index = END_HANDLE;
        SetActiveHandle(std::optional<handles::Id>(id));
    }
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;

    glm::vec3 GetPoint1() const { return origin; }
    glm::vec3 GetPoint2() const { return point2; }
    glm::mat4 GetModelMatrix() const { return matrix; }

protected:
    float x2 = 0.0f;
    float y2 = 0.0f;
    float z2 = 0.0f;
    mutable glm::vec3 origin;
    mutable glm::vec3 point2 = glm::vec3(0.0f);
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center = glm::vec3(0.0f);
    mutable float length = 0;
    float saved_angle = 0.0f;
    bool minMaxSet = false;

    mutable glm::mat4 matrix;
};
