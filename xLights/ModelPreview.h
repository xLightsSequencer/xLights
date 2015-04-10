#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H

#include <memory>

#include "wx/wx.h"

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"
#include "xlGLCanvas.h"

class ModelClass;
typedef std::unique_ptr<ModelClass> ModelClassPtr;

class ModelPreview : public xlGLCanvas
{

public:
    ModelPreview(wxPanel* parent);
	ModelPreview(wxPanel* parent,
                 std::vector<ModelClassPtr> &previewModels,
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
	void SetBackgroundBrightness(int brightness);
    
    void Render();
    void Render(const unsigned char *data);
    
    double calcPixelSize(double i);

protected:
    virtual void InitializeGLCanvas();

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
    xLightsDrawable* sprite;
    std::vector<ModelClassPtr> *PreviewModels;
    bool allowSelected;
    
    double currentPixelScaleFactor = 1.0;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
