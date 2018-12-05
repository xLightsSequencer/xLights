#ifndef BOXEDSCREENLOCATION3D_H
#define BOXEDSCREENLOCATION3D_H

#include "ModelScreenLocation.h"

class wxXmlNode;
class ModelPreview;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxCursor;

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation3D : public ModelScreenLocation {
public:
    BoxedScreenLocation3D();
    virtual ~BoxedScreenLocation3D() {}

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw() const override;
    virtual void TranslatePoint(float &x, float &y) const override;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles(DrawGLUtils::xlAccumulator &va) const override;
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
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
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool IsCenterBased() const override {return true;};

    virtual float GetHcenterOffset() const override {
        return (float)offsetXpct;
    }
    virtual float GetVcenterOffset() const override {
        return (float)offsetYpct;
    }
    virtual void SetHcenterOffset(float f) override {
        offsetXpct = f;
    }
    virtual void SetVcenterOffset(float f) override {
        offsetYpct = f;
    }

    virtual void SetOffset(float xPct, float yPct) override {
        offsetXpct=xPct;
        offsetYpct=yPct;
    }
    virtual void AddOffset(float xPct, float yPct) override {

        if (_locked) return;

        offsetXpct+=xPct;
        offsetYpct+=yPct;
    }
    void SetScale(float x, float y) {
        PreviewScaleX = x;
        PreviewScaleY = y;
        singleScale = false;
    }
    void SetRotation(int r) {
        PreviewRotation = r;
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

    virtual float GetScaleX() const {
        return PreviewScaleX;
    }

    virtual float GetScaleY() const {
        return PreviewScaleY;
    }

    int GetRotation() const {
        return PreviewRotation;
    }

    void SetScaleX(float scaleX)
    {
        PreviewScaleX = scaleX;
    }

    void SetScaleY(float scaleY)
    {
        PreviewScaleY = scaleY;
    }

private:
    float offsetXpct,offsetYpct;
    bool singleScale;
    float PreviewScaleX, PreviewScaleY;
    int PreviewRotation;

    mutable float radians;
    mutable float scalex;
    mutable float scaley;
    mutable float centerx;
    mutable float centery;

    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
};

#endif // BOXEDSCREENLOCATION3D_H
