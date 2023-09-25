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

#include "ModelScreenLocation.h"

//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation();

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    
    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual int MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;

    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;
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

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;

    glm::vec3 GetPoint1() const { return origin; }
    glm::vec3 GetPoint2() const { return point2; }

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

    wxXmlNode *old = nullptr;
    mutable glm::mat4 matrix;
};
