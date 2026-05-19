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
#include "handles/Handles.h"
#include "handles/DragSession.h"
#include <memory>
#include <string>
#include <vector>

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}

    virtual void Init() override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;

    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;

    [[nodiscard]] std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& id) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;

    // Descriptor pipeline. See `plans/handle-system-refactor.md`.
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

    virtual CursorType InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual void UpdateBoundingBox(float width, float height, float depth) override;

    virtual std::string GetDimension(float factor = 1.0) const override;
    virtual bool IsCenterBased() const override {return true;};

    virtual glm::vec2 GetScreenOffset(IModelPreview* preview) const override;

    virtual float GetHcenterPos() const override {
        return (float)worldPos_x;
    }
    virtual float GetVcenterPos() const override {
        return (float)worldPos_y;
    }
    virtual float GetDcenterPos() const override {
        return (float)worldPos_z;
    }
    virtual void SetHcenterPos(float f) override {
		worldPos_x = f;
    }
    virtual void SetVcenterPos(float f) override {
		worldPos_y = f;
    }
    virtual void SetDcenterPos(float f) override {
		worldPos_z = f;
    }

    virtual void SetPosition(float posx, float posy) override {
		worldPos_x = posx;
		worldPos_y = posy;
    }
    void SetScale(float x, float y) {
        scalex = x;
        scaley = y;
    }
    void SetScaleZ(float z) {
        scalez = z;
    }
    float GetScaleX() const {
        return scalex;
    }
    void SetScaleX(float x) {
        scalex = x;
    }
    float GetScaleY() const {
        return scaley;
    }
    void SetScaleY(float y) {
        scaley = y;
    }
    float GetScaleZ() const {
        return scalez;
    }

    void SetRotation(int r) {
        // Delegate to the base vec3 overload so rotate_quat is
        // rebuilt by the single canonical path. Boxed's 2D rotate
        // handle is the original caller.
        ModelScreenLocation::SetRotation(glm::vec3(rotatex, rotatey, (float)r));
    }
    void SetPerspective2D(float p) {
        perspective = p;
    }
    virtual float GetRealWidth() const override;
    virtual float GetRealHeight() const override;
    virtual float GetRealDepth() const override;

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
    virtual float GetRestorableMWidth() const override;
    virtual float GetRestorableMHeight() const override;
    virtual float GetRestorableMDepth() const override;

    int GetRotation() const {
        return rotatez;
    }

    bool GetSupportsZScaling() const { return supportsZScaling; }
    void SetSupportsZScaling(bool b) { supportsZScaling = b; }

    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_SCALE; }
    float GetCentreX() const { return centerx; }
    float GetCentreY() const { return centery; }
    float GetCentreZ() const { return centerz; }
    void SetCentreX(float x) { centerx = x; worldPos_x = x; }
    void SetCentreY(float y) { centery = y; worldPos_y = y; }
    void SetCentreZ(float z) { centerz = z; worldPos_z = z; }

protected:
    BoxedScreenLocation(int points);
    float perspective;

    mutable float centerx;
    mutable float centery;
    mutable float centerz;
};
