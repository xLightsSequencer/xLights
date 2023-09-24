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

#include "TwoPointScreenLocation.h"

class ThreePointScreenLocation : public TwoPointScreenLocation {
public:
    ThreePointScreenLocation();
    virtual ~ThreePointScreenLocation();
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const override;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual int MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) override;
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
    virtual float GetRealWidth() const override;
    virtual float GetRealHeight() const override;

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;
    virtual bool IsXYTransHandle() const override { return active_handle == SHEAR_HANDLE; }

private:
    bool modelHandlesHeight = false;
    bool supportsAngle = false;
    bool supportsShear = false;
    mutable glm::mat4 shearMatrix;
    float height = 1.0f;
    int angle = 0;
    float shear = 0.0f;
};


