#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H

#include <memory>

#include <wx/wx.h>

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"
#include "xlGLCanvas.h"

class Model;
class PreviewPane;
class LayoutGroup;
class xLightsFrame;

class ModelPreview : public xlGLCanvas
{

public:
    ModelPreview(wxPanel* parent);
	ModelPreview(wxPanel* parent,
                 xLightsFrame* xlights_,
                 std::vector<Model*> &models,
                 std::vector<LayoutGroup *> &groups,
                 bool allowSelected,
                 int style = 0,
                 bool allowPreviewChange = false);
	virtual ~ModelPreview();

    // Public Methods
	void InitializePreview(wxString img,int brightness);
    bool StartDrawing(wxDouble pointSize);
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
	void SetBackgroundBrightness(int brightness);
    int GetBackgroundBrightness() const { return mBackgroundBrightness;}
    void SetScaleBackgroundImage(bool b);
    bool GetScaleBackgroundImage() const { return scaleImage; }

    void Render();
    void Render(const unsigned char *data, bool swapBuffers=true);

    double calcPixelSize(double i);
    void SetModels(std::vector<Model*> &models) {
        previewModels.clear();
        previewModels = models;
        PreviewModels = &previewModels;
    }
    void SetModel(Model* model) { _model = model; }
    std::vector<Model*> &GetModels() {
        if (PreviewModels == nullptr) {
            return previewModels;
        }
        return *PreviewModels;
    }

    void SetPreviewPane(PreviewPane* pane) {mPreviewPane = pane;}
    void SetActive(bool show);
    bool GetActive();

	 virtual void render(const wxSize& size=wxSize(0,0)) override;

    DrawGLUtils::xlAccumulator &GetAccumulator() {return accumulator;}
protected:
    virtual void InitializeGLCanvas() override;
    virtual bool UsesVertexTextureAccumulator() override {return true;}
    virtual bool UsesVertexColorAccumulator() override {return false;}
    virtual bool UsesVertexAccumulator() override {return false;}
    virtual bool UsesAddVertex() override {return true;}

private:
	void render(wxPaintEvent& event);
	void SetOrigin();
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftUp(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	//void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
    void OnPopup(wxCommandEvent& event);

    bool mIsDrawing = false;
    bool mBackgroundImageExists = false;
    wxString mBackgroundImage;
    int  mBackgroundBrightness=100;
    wxDouble mPointSize = 2.0;
    int virtualWidth, virtualHeight;

    Image* image = nullptr;
    bool scaleImage = false;
    xLightsDrawable* sprite;
    std::vector<Model*> previewModels;
    std::vector<Model*> *PreviewModels;
    std::vector<Model*> *HouseModels;
    std::vector<LayoutGroup *> *LayoutGroups;
    bool allowSelected;
    bool allowPreviewChange;
    PreviewPane* mPreviewPane;
    DrawGLUtils::xlAccumulator accumulator;
    Model* _model;
    xLightsFrame* xlights;

    double currentPixelScaleFactor = 1.0;

    int maxVertexCount;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
