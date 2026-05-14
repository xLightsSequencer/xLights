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

#include "TwoPointScreenLocation.h"

class ThreePointScreenLocation : public TwoPointScreenLocation {
public:
    ThreePointScreenLocation();
    virtual ~ThreePointScreenLocation();
    virtual void Init() override;
    virtual CursorType InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, IModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual bool IsContained(IModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const override;

    [[nodiscard]] std::unique_ptr<handles::SpaceMouseSession>
    BeginSpaceMouseSession(const std::optional<handles::Id>& id) override;
    // SpaceMouse Shear-handle handler (public so the session class
    // in the .cpp can call it; called only for Shear-role ids).
    void ApplySpaceMouseShearHandle(float scale, const glm::vec3& rot, const glm::vec3& mov);
    virtual float GetVScaleFactor() const override;

    void SetYShear( float shear_ ) { shear = shear_; }
    virtual float GetYShear() const override { return supportsShear ? shear : 0.0f; }

    virtual void SetMWidth(float w) override;
    virtual float GetMWidth() const override;

    virtual void SetMHeight(float h) override { height = h; }
    virtual float GetMHeight() const override { return height; }

    void SetModelHandleHeight(bool b) { modelHandleHeight = b; }
    void SetSupportsAngle(bool b) { supportsAngle = b; }
    void SetSupportsShear(bool b) { supportsShear = b; }

    void SetAngle(int a) { angle = a; }
    int GetAngle() const { return angle; }

    virtual float GetRealWidth() const override;
    virtual float GetRealHeight() const override;

    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    // ThreePoint inherits TwoPoint's SetActiveHandleToCentre override.
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;
    virtual bool IsXYTransHandle() const override { return IsRole(active_handle, handles::Role::Shear); }

    float GetHeight() const { return height; }
    void SetHeight(float h) { height = h; }
    float GetShear() const { return shear; }
    void SetShear(float s) { shear = s; }
    bool GetSupportsShear() const { return supportsShear; }
    bool GetSupportsAngle() const { return supportsAngle; }

    [[nodiscard]] glm::vec3 GetShearHandleWorldPosition() const;

    [[nodiscard]] std::vector<handles::Descriptor> GetHandles(
        handles::ViewMode mode, handles::Tool tool,
        const handles::ViewParams& view = {}) const override;
    std::unique_ptr<handles::DragSession> CreateDragSession(
        const std::string& modelName,
        const handles::Id& id,
        const handles::WorldRay& startRay) override;

private:
    bool modelHandleHeight = false;
    bool supportsAngle = false;
    bool supportsShear = false;
    mutable glm::mat4 shearMatrix;
    float height = 1.0f;
    int angle = 0;
    float shear = 0.0f;
};


