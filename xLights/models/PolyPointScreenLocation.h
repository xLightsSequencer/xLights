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

//Location that uses multiple points
class PolyPointScreenLocation : public ModelScreenLocation {
public:
    PolyPointScreenLocation();
    virtual ~PolyPointScreenLocation();

    virtual void Read(wxXmlNode* node) override;
    virtual void Write(wxXmlNode* node) override;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode* node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float& x, float& y, float& z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int& handle, int x, int y) const override;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const override;

    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const override;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual int MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;
    virtual void SelectHandle(int handle) override;
    virtual int GetSelectedHandle() const override { return selected_handle; }
    virtual int GetNumHandles() const override { return num_points; }
    virtual void SelectSegment(int segment) override;
    virtual int GetSelectedSegment() const override { return selected_segment; }
    virtual bool SupportsCurves() const override {return true;}
    virtual bool HasCurve(int segment) const override { return mPos[segment].has_curve; }
    virtual void SetCurve(int seg_num, bool create = true) override;
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override;
    virtual void InsertHandle(int after_handle, float zoom, int scale) override;
    virtual void DeleteHandle(int handle) override;
    virtual wxCursor InitializeLocation(int& handle, int x, int y, const std::vector<NodeBaseClassPtr>& Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr>& Node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;
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

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_XY_TRANS; }
    virtual float GetYShear() const { return 0.0; }
    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;

protected:
    struct xlPolyPoint {
        float x;
        float y;
        float z;
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
    int selected_handle = 0;
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

