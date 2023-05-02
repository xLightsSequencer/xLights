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

#include <memory>

#include <wx/wx.h>

#include "graphics/xlGraphicsBase.h"
#include "graphics/xlGraphicsAccumulators.h"

#include "Color.h"
#include "ViewpointMgr.h"
#include "models/ModelManager.h"

#include "Mouse3DManager.h"

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// This does not actually fix the pixel size on dialog problem but consolidating this value into one location makes sense
// This is the default size pixels render in things like submodel, faces etc dialogs
#define PIXEL_SIZE_ON_DIALOGS 2

class Model;
class ModelGroup;
class PreviewPane;
class LayoutGroup;
class xLightsFrame;
class xlVertex3Accumulator;

class ModelPreview : public GRAPHICS_BASE_CLASS
{
    int _cameraView_last_offsetx = 0;
    int _cameraView_last_offsety = 0;
    int _cameraView_latched_x = 0;
    int _cameraView_latched_y = 0;

    int _cameraPos_last_offsetx = 0;
    int _cameraPos_last_offsety = 0;
    int _cameraPos_latched_x = 0;
    int _cameraPos_latched_y = 0;
    bool _showFirstPixel = false;

public:
    ModelPreview(wxPanel* parent, xLightsFrame* xlights = nullptr);
	ModelPreview(wxPanel* parent,
                 xLightsFrame* xlights,
                 bool allowSelected,
                 int style = 0,
                 bool allowPreviewChange = false,
                 bool showFirstPixel = false);
	virtual ~ModelPreview();

    // Public Methods
	void InitializePreview(wxString img,int brightness,int alpha, bool center2d0);
    bool StartDrawing(wxDouble pointSize, bool fromPaint = false);
    void SetPointSize(wxDouble pointSize);
    void Reset();
    void EndDrawing(bool swapBuffers=true);
	void SetCanvasSize(int width,int height);
    void SetVirtualCanvasSize(int width, int height);
    void GetVirtualCanvasSize(int &w, int& h) const { w = virtualWidth; h = virtualHeight; }
    int GetVirtualCanvasHeight() const  { return virtualHeight; }
    int GetVirtualCanvasWidth() const { return virtualWidth; }
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
    float GetCameraZoomForHandles() const;
    int GetHandleScale() const;

    void DrawGroupCentre(float x, float y);
    void Render();
    void Render(uint32_t frameTime, const unsigned char *data, bool swapBuffers=true);
    void RenderModels(const std::vector<Model*>& models, bool selected, bool showFirstPixel);
    void RenderModel(Model* m, bool wiring = false, bool highlightFirst = false, int highlightpixel = 0);

    double calcPixelSize(double i);

    void SetModel(const Model* model, bool wiring = false, bool highlightFirst = false);
    void SetActiveLayoutGroup(const std::string &grp = "Default") {
        currentLayoutGroup = grp;
        Refresh();
    }
    const std::vector<Model*> &GetModels();
    ModelGroup* GetSelectedModelGroup();
    bool ValidateModels(const ModelManager& mm);
    bool ValidateModels(const std::vector<Model*>models, const ModelManager& mm);
    void SetAdditionalModel(Model *m) {
        additionalModel = m;
        Refresh();
    }
    Model* GetAdditionalModel() const { return additionalModel; }

    void SetPreviewPane(PreviewPane* pane) {mPreviewPane = pane;}
    void SetActive(bool show);
    bool GetActive() const;
    float GetZoom() const { return (is3d ? camera3d->GetZoom() : camera2d->GetZoom()); }
    double GetCurrentScaleFactor() const { return std::max((float)virtualWidth / (float)mWindowWidth, (float)virtualHeight / (float)mWindowHeight); }
    float GetCameraRotationY() const { return (is3d ? camera3d->GetAngleY() : camera2d->GetAngleY()); }
    float GetCameraRotationX() const { return (is3d ? camera3d->GetAngleX() : camera2d->GetAngleX()); }
    void SetPan(float deltax, float deltay, float deltaz);
    void Set3D(bool value) { is3d = value; }
    bool Is3D() const { return is3d; }
    glm::mat4& GetProjViewMatrix() { return ProjViewMatrix; }
    glm::mat4& GetProjMatrix() { return ProjMatrix; }


    void SaveCurrentCameraPosition();
    void SaveDefaultCameraPosition();
    void RestoreDefaultCameraPosition();
    void SetCamera2D(int i);
    void SetCamera3D(int i);
    void SetDisplay2DBoundingBox(bool bb) {
        _display2DBox = bb; 
    }
    void SetDisplay2DGrid(bool grid, long gridSpacing) { _displayGrid = grid; _displayGridSpacing = gridSpacing; grid2dValid=false; }
    void SetDisplay2DCenter0(bool bb) { _center2D0 = bb; grid2dValid=false; }

    bool IsNoCurrentModel() { return currentModel == "&---none---&"; }

    void AddBoundingBoxToAccumulator(int x1, int y1, int x2, int y2);


    void render() override;
    
    bool drawingUsingLogicalSize() const override { return true; }
    virtual bool RequiresDepthBuffer() const override { return true; }

    void setCurrentFrameTime(uint32_t ft) { currentFrameTime = ft; }
    uint32_t getCurrentFrameTime() const { return currentFrameTime; }
    xlGraphicsContext *getCurrentGraphicsContext() { return currentContext; }
    xlGraphicsProgram *getCurrentSolidProgram() { return solidProgram; }
    xlGraphicsProgram *getCurrentTransparentProgram() { return transparentProgram; }
    
    
    void OnMotion3DEvent(Motion3DEvent &event);
    void OnMotion3DButtonEvent(wxCommandEvent &event);

protected:
    void AddGridToAccumulator(const glm::mat4& ViewScale);

private:
    void setupCameras();
	void Paint(wxPaintEvent& event);
	void SetOrigin();
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftDClick(wxMouseEvent& event);
	void mouseLeftUp(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
    void mouseMiddleDown(wxMouseEvent& event);
    void mouseMiddleUp(wxMouseEvent& event);
    //void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);

    void OnZoomGesture(wxZoomGestureEvent& event);

    void OnPopup(wxCommandEvent& event);

    bool mIsDrawing = false;
    bool mBackgroundImageExists = false;
    wxString mBackgroundImage;
    int mBackgroundBrightness = 100;
    int mBackgroundAlpha = 100;
    wxDouble mPointSize = 2.0;
    int virtualWidth = 0;
    int virtualHeight = 0;

    bool _display2DBox =  false;
    bool _displayGrid = false;
    long _displayGridSpacing = 100;
    bool _center2D0 = false;
    bool scaleImage = false;
    bool allowSelected;
    bool allowPreviewChange;
    PreviewPane* mPreviewPane;

    xLightsFrame* xlights = nullptr;
    std::string currentModel;
    std::string currentLayoutGroup;
    std::vector<Model*> tmpModelList;
    Model *additionalModel = nullptr;
    uint32_t currentFrameTime = 0;

    xlGraphicsProgram *solidProgram = nullptr;
    xlGraphicsProgram *transparentProgram = nullptr;
    xlGraphicsProgram *solidViewObjectProgram = nullptr;
    xlGraphicsProgram *transparentViewObjectProgram = nullptr;
    xlTexture *background = nullptr;
    wxSize backgroundSize;
    
    xlVertexAccumulator *grid2d = nullptr;
    bool grid2dValid = false;

    bool _wiring = false;
    bool _highlightFirst = false;
    bool m_mouse_down = false;
    bool m_wheel_down = false;
    int m_last_mouse_x = 0;
    int m_last_mouse_y = 0;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjMatrix;
    glm::mat4 ProjViewMatrix;

    PreviewCamera* camera3d;
    PreviewCamera* camera2d;
    static const long ID_VIEWPOINT2D;
    static const long ID_VIEWPOINT3D;
    static const long ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE;
    double currentPixelScaleFactor = 1.0;


    xlGraphicsContext *currentContext = nullptr;
    std::list<xlTexture *> texturesToDelete;


	DECLARE_EVENT_TABLE()
};
