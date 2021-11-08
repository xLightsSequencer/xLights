#pragma once


#include "osxUtils/wxMetalCanvas.hpp"
#include "../xlGraphicsContext.h"

class xlGLCanvas;

class xlMetalCanvas : public wxMetalCanvas {
public:
    xlMetalCanvas(wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = "",
                  bool only2d = true);
    xlMetalCanvas(wxWindow *parent, const wxString& name) : xlMetalCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, name) {}

    virtual ~xlMetalCanvas();


    int getWidth() const { return mWindowWidth; }
    int getHeight() const { return mWindowHeight; }

    void Resized(wxSizeEvent& evt);
    void OnEraseBackGround(wxEraseEvent& event) {};

    virtual xlColor ClearBackgroundColor() const;

    double translateToBacking(double x) const;
    double mapLogicalToAbsolute(double x) const;

protected:
    DECLARE_EVENT_TABLE()

    virtual void PrepareCanvas();
    virtual xlGraphicsContext* PrepareContextForDrawing();
    virtual void FinishDrawing(xlGraphicsContext *ctx);

    virtual bool drawingUsingLogicalSize() const;


    size_t mWindowWidth;
    size_t mWindowHeight;
    int mWindowResized;
    bool mIsInitialized = false;
    wxString mName;
    bool is3d;
    
    xlGLCanvas *fallback;

};

