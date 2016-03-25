#ifndef MODELSCREENLOCATION_H
#define MODELSCREENLOCATION_H


#define OVER_NO_HANDLE              -1
#define OVER_L_TOP_HANDLE           1
#define OVER_R_TOP_HANDLE           2
#define OVER_L_BOTTOM_HANDLE        3
#define OVER_R_BOTTOM_HANDLE        4
#define OVER_ROTATE_HANDLE          5

class wxXmlNode;
class ModelPreview;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxCursor;

#include <vector>
#include "Node.h"

class ModelScreenLocation
{
public:
    ModelScreenLocation();
    virtual ~ModelScreenLocation() {};
    
    virtual void Read(wxXmlNode *node) = 0;
    virtual void Write(wxXmlNode *node) = 0;
    
    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) = 0;
    virtual void PrepareToDraw() const = 0;
    virtual void TranslatePoint(double &x, double &y) const = 0;

    virtual bool IsContained(int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(int x,int y) const = 0;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const = 0;
    virtual void DrawHandles() const = 0;
    virtual void MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) = 0;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) = 0;
 
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const = 0;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) = 0;
    

    virtual void SetOffset(double xPct, double yPct) = 0;
    virtual void AddOffset(double xPct, double yPct) = 0;
    
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
    
    
    void SetRenderSize(int NewWi, int NewHt) {
        RenderHt=NewHt;
        RenderWi=NewWi;
    }
    int RenderHt,RenderWi;  // size of the rendered output
    int previewW,previewH;
};

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}
    
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    
    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw() const override;
    virtual void TranslatePoint(double &x, double &y) const override;
    
    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles() const override;
    virtual void MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) override;
    
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual float GetHcenterOffset() const override {
        return offsetXpct;
    }
    virtual float GetVcenterOffset() const override {
        return offsetYpct;
    }
    virtual void SetHcenterOffset(float f) override {
        offsetXpct = f;
    }
    virtual void SetVcenterOffset(float f) override {
        offsetYpct = f;
    }

    virtual void SetOffset(double xPct, double yPct) override {
        offsetXpct=xPct;
        offsetYpct=yPct;
    }
    virtual void AddOffset(double xPct, double yPct) override {
        offsetXpct+=xPct;
        offsetYpct+=yPct;
    }
    void SetScale(double x, double y) {
        PreviewScaleX = x;
        PreviewScaleY = y;
        singleScale = false;
    }
    void SetRotation(int r) {
        PreviewRotation = r;
    }
    
    virtual int GetTop() const override {
        return mMaxScreenY;
    }
    virtual int GetLeft() const override {
        return mMinScreenX;
    }
    virtual int GetRight() const override {
        return mMaxScreenX;
    }
    virtual int GetBottom() const override {
        return mMinScreenY;
    }
    virtual void SetTop(int i) override;
    virtual void SetLeft(int i) override;
    virtual void SetRight(int i) override;
    virtual void SetBottom(int i) override;
    
private:
    double offsetXpct,offsetYpct;
    bool singleScale;
    double PreviewScaleX, PreviewScaleY;
    int PreviewRotation;
    
    mutable double radians;
    mutable double scalex;
    mutable double scaley;
    mutable double centerx;
    mutable double centery;
    
    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
    
    struct xlPoint {
        int x;
        int y;
    };
    mutable xlPoint mHandlePosition[5];
};


//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation() {}
    
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    
    virtual void SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) override;
    virtual void PrepareToDraw() const override;
    virtual void TranslatePoint(double &x, double &y) const override;
    
    virtual bool IsContained(int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(int x,int y) const override;
    virtual wxCursor CheckIfOverHandles(int &handle, int x, int y) const override;
    virtual void DrawHandles() const override;
    virtual void MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) override;

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    
    virtual float GetHcenterOffset() const override;
    virtual float GetVcenterOffset() const override;
    virtual void SetHcenterOffset(float f) override;
    virtual void SetVcenterOffset(float f) override;
    
    virtual void SetOffset(double xPct, double yPct) override;
    virtual void AddOffset(double xPct, double yPct) override;
    virtual int GetTop() const override;
    virtual int GetLeft() const override;
    virtual int GetRight() const override;
    virtual int GetBottom() const override;
    virtual void SetTop(int i) override;
    virtual void SetLeft(int i) override;
    virtual void SetRight(int i) override;
    virtual void SetBottom(int i) override;
    
    void FlipCoords();
private:
    float x1, y1;
    float x2, y2;
    wxXmlNode *old;
    
    struct xlPoint {
        int x;
        int y;
    };
    mutable xlPoint mHandlePosition[2];
};

#endif // MODELSCREENLOCATION_H
