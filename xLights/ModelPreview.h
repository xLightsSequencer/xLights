#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H

#include <memory>

#include "wx/wx.h"

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"
#include "xlGLCanvas.h"

class Model;
class PreviewPane;

class ModelPreview : public xlGLCanvas
{

public:
    ModelPreview(wxPanel* parent);
	ModelPreview(wxPanel* parent,
                 std::vector<Model*> &models,
                 bool allowSelected,
                 int style = 0);
	virtual ~ModelPreview();

    // Public Methods
	void InitializePreview(wxString img,int brightness);
    bool StartDrawing(wxDouble pointSize);
    void SetPointSize(wxDouble pointSize);
    void EndDrawing();
	void SetCanvasSize(int width,int height);
    void SetVirtualCanvasSize(int width, int height);
    void GetVirtualCanvasSize(int &w, int& h) {
        w = virtualWidth; h = virtualHeight;
    }
    int GetVirtualCanvasHeight() {
        return virtualHeight;
    }
    int GetVirtualCanvasWidth() {
        return virtualWidth;
    }
	void SetbackgroundImage(wxString image);
    const wxString &GetBackgroundImage() const { return mBackgroundImage;}
	void SetBackgroundBrightness(int brightness);
    int GetBackgroundBrightness() const { return mBackgroundBrightness;}
    void SetScaleBackgroundImage(bool b);
    bool GetScaleBackgroundImage() const { return scaleImage; }

    void Render();
    void Render(const unsigned char *data);

    double calcPixelSize(double i);
    void SetModels(std::vector<Model*> &models) {
        previewModels.clear();
        previewModels = models;
        PreviewModels = &previewModels;
    }
    std::vector<Model*> &GetModels() {
        if (PreviewModels == nullptr) {
            return previewModels;
        }
        return *PreviewModels;
    }

    void SetPreviewPane(PreviewPane* pane) {mPreviewPane = pane;}
    void SetActive(bool show);
    bool GetActive();

protected:
    virtual void InitializeGLCanvas();
    virtual bool UsesVertexTextureAccumulator() {return true;}
    virtual bool UsesVertexColorAccumulator() {return false;}
    virtual bool UsesVertexAccumulator() {return false;}
    virtual bool UsesAddVertex() {return true;}

private:
	void render(wxPaintEvent& event);
	void SetOrigin();
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftUp(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);

    bool mIsDrawing = false;
    bool mBackgroundImageExists = false;
    wxString mBackgroundImage;
    int  mBackgroundBrightness=100;
    wxDouble mPointSize = 2.0;
    int virtualWidth, virtualHeight;

    Image* image = NULL;
    bool scaleImage = false;
    xLightsDrawable* sprite;
    std::vector<Model*> previewModels;
    std::vector<Model*> *PreviewModels;
    bool allowSelected;
    PreviewPane* mPreviewPane;

    double currentPixelScaleFactor = 1.0;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
