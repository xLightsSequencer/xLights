#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H

#include <memory>

#include <wx/wx.h>

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"
#include "xlGLCanvas.h"
#include "ViewpointMgr.h"

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;
class PreviewPane;
class LayoutGroup;
class xLightsFrame;
class xlVertex3Accumulator;

class ModelPreview : public xlGLCanvas
{

public:
    ModelPreview(wxPanel* parent, xLightsFrame* xlights = nullptr);
	ModelPreview(wxPanel* parent,
                 xLightsFrame* xlights,
                 bool allowSelected,
                 int style = 0,
                 bool allowPreviewChange = false);
	virtual ~ModelPreview();

    // Public Methods
	void InitializePreview(wxString img,int brightness,int alpha);
    bool StartDrawing(wxDouble pointSize, bool fromPaint = false);
    void SetPointSize(wxDouble pointSize);
    void EndDrawing(bool swapBuffers=true);
	void SetCanvasSize(int width,int height);
    void SetVirtualCanvasSize(int width, int height);
    void GetVirtualCanvasSize(int &w, int& h) const
    {
        w = virtualWidth; h = virtualHeight;
    }
    int GetVirtualCanvasHeight() const
    {
        return virtualHeight;
    }
    int GetVirtualCanvasWidth() const
    {
        return virtualWidth;
    }
	void SetbackgroundImage(wxString image);
    const wxString &GetBackgroundImage() const { return mBackgroundImage;}
	void SetBackgroundBrightness(int brightness, int alpha);
/*
    int GetBackgroundBrightness() const { return mBackgroundBrightness;}
     */
    void SetScaleBackgroundImage(bool b);
    bool GetScaleBackgroundImage() const { return scaleImage; }

	void SetCameraView(int camerax, int cameray, bool latch, bool reset = false);
	void SetCameraPos(int camerax, int cameray, bool latch, bool reset = false);
    void SetZoomDelta(float delta);

    void Render();
    void Render(const unsigned char *data, bool swapBuffers=true);

    double calcPixelSize(double i);

    void SetModel(const Model* model);
    void SetActiveLayoutGroup(const std::string &grp = "Default") {
        currentLayoutGroup = grp;
        Refresh();
    }
    const std::vector<Model*> &GetModels();
    void SetAdditionalModel(Model *m) {
        additionalModel = m;
        Refresh();
    }


    void SetPreviewPane(PreviewPane* pane) {mPreviewPane = pane;}
    void SetActive(bool show);
    bool GetActive();
    float GetZoom() { return (is_3d ? camera3d->GetZoom() : camera2d->GetZoom()); }
    float GetCameraRotation() { return (is_3d ? camera3d->GetAngleY() : camera2d->GetAngleY()); }
    void SetPan(float deltax, float deltay, float deltaz);
    void Set3D(bool value) { is_3d = value; }
    bool Is3D() { return is_3d; }
    glm::mat4& GetProjViewMatrix() { return ProjViewMatrix; }
    glm::mat4& GetProjMatrix() { return ProjMatrix; }

	virtual void render(const wxSize& size=wxSize(0,0)) override;

    DrawGLUtils::xlAccumulator &GetAccumulator() { return accumulator; }
    DrawGLUtils::xl3Accumulator &GetAccumulator3d() { return accumulator3d; }
    void SaveCurrentCameraPosition();
    void SetCamera2D(int i);
    void SetCamera3D(int i);

protected:
    virtual void InitializeGLCanvas() override;
    virtual bool UsesVertexTextureAccumulator() override {return true;}
    virtual bool UsesVertexColorAccumulator() override {return false;}
    virtual bool UsesVertexAccumulator() override {return false;}
    virtual bool UsesAddVertex() override {return true;}
    virtual bool UsesVertex3Accumulator() override {return true;}
    virtual bool UsesVertex3TextureAccumulator() override { return true; }
    virtual bool UsesVertex3ColorAccumulator() override {return true;}

private:
    void setupCameras();
	void render(wxPaintEvent& event);
	void SetOrigin();
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftUp(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
    void mouseMiddleDown(wxMouseEvent& event);
    void mouseMiddleUp(wxMouseEvent& event);
    //void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
    void OnPopup(wxCommandEvent& event);
	void drawGrid(float size, float step);
    void OnSysColourChanged(wxSysColourChangedEvent& event);
    bool mIsDrawing = false;
    bool mBackgroundImageExists = false;
    wxString mBackgroundImage;
    int mBackgroundBrightness=100;
    int mBackgroundAlpha=100;
    wxDouble mPointSize = 2.0;
    int virtualWidth, virtualHeight;

    Image* image;
    bool scaleImage = false;
    xLightsDrawable* sprite;
    bool allowSelected;
    bool allowPreviewChange;
    PreviewPane* mPreviewPane;
    DrawGLUtils::xlAccumulator accumulator;

    xLightsFrame* xlights;
    std::string currentModel;
    std::string currentLayoutGroup;
    std::vector<Model*> tmpModelList;
    Model *additionalModel;

	DrawGLUtils::xl3Accumulator view_object_accumulator;
    DrawGLUtils::xl3Accumulator accumulator3d;
    bool is_3d;
    bool m_mouse_down;
    bool m_wheel_down;
    int m_last_mouse_x, m_last_mouse_y;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjMatrix;
    glm::mat4 ProjViewMatrix;

    PreviewCamera* camera3d;
    PreviewCamera* camera2d;
    static const long ID_VIEWPOINT2D;
    static const long ID_VIEWPOINT3D;

    double currentPixelScaleFactor = 1.0;

    int maxVertexCount;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
