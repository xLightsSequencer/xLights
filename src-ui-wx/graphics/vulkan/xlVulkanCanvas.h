#pragma once

#include <wx/window.h>
#include "VulkanBackend.h"
#include "xlVulkanGraphicsContext.h"
#include "Color.h"
#include <wx/glcanvas.h>
#include <mutex>
#include <unordered_map>
#include <functional>

class xlVulkanCanvas : public wxWindow {
public:
    xlVulkanCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0);
    // Overload matching xlGLCanvas signature (name + only2d)
    xlVulkanCanvas(wxWindow* parent, wxWindowID id,
                   const wxPoint& pos,
                   const wxSize& size,
                   long style,
                   const wxString &name,
                   bool only2d = true);
    virtual ~xlVulkanCanvas();

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt) {}

    // Compatibility with xlGLCanvas API
    virtual void PrepareCanvas();
    virtual xlGraphicsContext* PrepareContextForDrawing();
    virtual xlGraphicsContext* PrepareContextForDrawing(const xlColor &bg);
    virtual void FinishDrawing(xlGraphicsContext* ctx, bool display = true);
    virtual void Resized(wxSizeEvent& evt);
    virtual bool RequiresDepthBuffer() const { return false; }

    // xlGLCanvas-compatible helpers
    void SetCurrentGLContext();
    const std::string &getName() const { return _name; }
    int getWidth() const { return static_cast<int>(mWindowWidth); }
    int getHeight() const { return static_cast<int>(mWindowHeight); }
    double translateToBacking(double x) const { return x; }
    double mapLogicalToAbsolute(double x) const { return x; }
    void DisplayWarning(const wxString &msg) { (void)msg; }
    wxImage *GrabImage( wxSize size = wxSize(0,0) );
    // Async grab: starts a readback and returns a request id, or -1 on failure.
    int StartGrabImageAsync(wxSize size = wxDefaultSize);
    // Retrieve the wxImage* for a completed async grab. Caller takes ownership and must delete.
    wxImage* GetAsyncGrabbedImage(int requestId);
    // Start async grab and register a callback invoked on the main thread with the wxImage*.
    // The callback receives ownership of the wxImage* and is responsible for deleting it.
    int StartGrabImageAsyncWithCallback(std::function<void(wxImage*)> cb, wxSize size = wxDefaultSize);
    // Event handler invoked on the main thread when a grab completes
    void OnGrabComplete(wxCommandEvent &evt);
    void captureNextFrame(int w, int h) { (void)w; (void)h; }
    bool getFrameForExport(int w, int h, void* /*AVFrame*/ , uint8_t *buffer, int bufferSize) { (void)w; (void)h; (void)buffer; (void)bufferSize; return false; }
    virtual void render() {}
    int GetZDepth() const { return 0; }
    bool IsCoreProfile() const { return false; }
    bool bindVertexArrayID(unsigned int pid) { (void)pid; return false; }
    static wxGLContext *GetSharedContext() { return nullptr; }

    virtual xlColor ClearBackgroundColor() const { return xlBLACK; }
    virtual bool drawingUsingLogicalSize() const { return true; }

    // Canvas state accessible by callers (kept similar to xlGLCanvas)
    size_t mWindowWidth = 0;
    size_t mWindowHeight = 0;
    int mWindowResized = 0;
    bool mIsInitialized = false;
    // Compatibility flag used throughout the codebase
    bool is3d = false;
    // static event table declaration for wxWidgets
    // no static event table; events are bound at runtime in the constructor

private:
    std::unique_ptr<xlVulkanGraphicsContext> m_ctx;
    std::string _name;
    // Expose backend pointer for the graphics context
    gfx::VulkanBackend* backend_ptr() { return &backend_; }

private:
    gfx::VulkanBackend backend_;
    void* native_display_ = nullptr; // on X11
    // GL compatibility members
    wxGLContext* m_context = nullptr;
    int  m_zDepth = 0;
    bool isCoreProfile = false;
    static wxGLContext *m_sharedContext;
    // async grab results (store raw pixel data; wxImage will be created on main thread)
    struct AsyncGrabData { std::unique_ptr<std::vector<uint8_t>> pixels; int width = 0; int height = 0; wxSize requestedSize; };
    std::mutex async_mutex_;
    std::unordered_map<int, AsyncGrabData> async_images_;
    // Callbacks stored for async grabs
    std::unordered_map<int, std::function<void(wxImage*)>> async_callbacks_;

};
