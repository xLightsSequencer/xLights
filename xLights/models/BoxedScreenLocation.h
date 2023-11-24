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

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}

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

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual int MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;

    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual void UpdateBoundingBox(float width, float height, float depth) override;

    virtual std::string GetDimension(float factor = 1.0) const override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool IsCenterBased() const override {return true;};

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;

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
    float GetScaleZ() const {
        return scalez;
    }

    void SetRotation(int r) {
        rotatez = r;
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

    float GetScaleX() { return scalex; }
    float GetScaleY() { return scaley; }

    virtual int GetDefaultHandle() const override { return CENTER_HANDLE; }
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
