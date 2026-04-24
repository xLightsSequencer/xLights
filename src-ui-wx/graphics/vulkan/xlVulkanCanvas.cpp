#include "xlVulkanCanvas.h"
#include <wx/dcclient.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <wx/image.h>
#include <wx/event.h>

// Custom event type for async grab completion
wxDEFINE_EVENT(EVT_VULKAN_GRAB_COMPLETE, wxCommandEvent);

struct VulkanGrabEvent : public wxCommandEvent {
    VulkanGrabEvent(int id = 0) : wxCommandEvent(EVT_VULKAN_GRAB_COMPLETE, id) {}
    wxEvent* Clone() const override { return new VulkanGrabEvent(*this); }
};

#if defined(__WXGTK__)
#  include <X11/Xlib.h>
#endif

wxGLContext* xlVulkanCanvas::m_sharedContext = nullptr;

xlVulkanCanvas::xlVulkanCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxWindow(parent, id, pos, size, style) {
    // Try to get native handles
    void* native_window = GetHandle();
    void* native_display = nullptr;

#if defined(_WIN32)
    // On Windows GetHandle() returns HWND
    native_display = nullptr;
#elif defined(__WXGTK__)
    // On X11, try to open a display (simplified)
    native_display = XOpenDisplay(nullptr);
    if (!native_display) {
        std::cerr << "wxVulkanCanvas: failed to open X display" << std::endl;
    }

wxImage *xlVulkanCanvas::GrabImage( wxSize size ) {
    // Blocking synchronous grab implemented on main thread using async readback API
    if (!backend_.IsInitialized()) return nullptr;

    int req = backend_.StartReadback(UINT32_MAX);
    if (req < 0) return nullptr;

    const int timeoutMs = 5000; // 5s timeout
    const int pollMs = 5;
    int waited = 0;
    std::vector<uint8_t> pixels;
    while (waited < timeoutMs) {
        if (backend_.TryGetReadback(req, pixels)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
        waited += pollMs;
    }
    if (pixels.empty()) return nullptr;

    uint32_t w = static_cast<uint32_t>(mWindowWidth);
    uint32_t h = static_cast<uint32_t>(mWindowHeight);
    if (w == 0 || h == 0) return nullptr;

    wxImage *img = new wxImage((int)w, (int)h);
    img->InitAlpha();
    unsigned char *data = img->GetData();
    unsigned char *alpha = img->GetAlpha();
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = ((size_t)y * w + x) * 4;
            size_t dst = ((size_t)y * w + x) * 3;
            data[dst + 0] = pixels[idx + 0];
            data[dst + 1] = pixels[idx + 1];
            data[dst + 2] = pixels[idx + 2];
            alpha[(size_t)y * w + x] = pixels[idx + 3];
        }
    }

    if (size.GetWidth() > 0 && size.GetHeight() > 0 && (size.GetWidth() != (int)w || size.GetHeight() != (int)h)) {
        wxImage *res = new wxImage(img->Rescale(size.GetWidth(), size.GetHeight()));
        delete img;
        img = res;
    }

    return img;
}
#endif

    if (!backend_.Init(native_window, native_display, 2)) {
        std::cerr << "wxVulkanCanvas: Vulkan backend Init failed" << std::endl;
    }
    native_display_ = native_display;
    mWindowWidth = size.GetWidth();
    mWindowHeight = size.GetHeight();
    _name = "";

    // Bind events at runtime
    Bind(wxEVT_PAINT, &xlVulkanCanvas::OnPaint, this);
    Bind(wxEVT_SIZE, &xlVulkanCanvas::OnSize, this);
    Bind(wxEVT_ERASE_BACKGROUND, &xlVulkanCanvas::OnEraseBackground, this);
    Bind(EVT_VULKAN_GRAB_COMPLETE, &xlVulkanCanvas::OnGrabComplete, this);
}

// Overload matching xlGLCanvas signature (name + only2d)
xlVulkanCanvas::xlVulkanCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString &name, bool only2d)
    : xlVulkanCanvas(parent, id, pos, size, style) {
    _name = std::string(name.mb_str());
    (void)only2d;
}

xlVulkanCanvas::~xlVulkanCanvas() {
#if defined(__WXGTK__)
    if (native_display_) XCloseDisplay(static_cast<Display*>(native_display_));
#endif
    backend_.Shutdown();
}

void xlVulkanCanvas::PrepareCanvas() {
    if (mIsInitialized) return;
    m_ctx = std::make_unique<xlVulkanGraphicsContext>(this, backend_ptr());
    mIsInitialized = true;
}

xlGraphicsContext* xlVulkanCanvas::PrepareContextForDrawing() {
    if (!mIsInitialized) PrepareCanvas();
    return m_ctx.get();
}

xlGraphicsContext* xlVulkanCanvas::PrepareContextForDrawing(const xlColor &bg) {
    (void)bg;
    return PrepareContextForDrawing();
}

void xlVulkanCanvas::FinishDrawing(xlGraphicsContext* ctx, bool display) {
    (void)ctx; (void)display;
}

void xlVulkanCanvas::Resized(wxSizeEvent& evt) {
    wxSize s = evt.GetSize();
    mWindowWidth = s.GetWidth();
    mWindowHeight = s.GetHeight();
    mWindowResized = 1;
}

void xlVulkanCanvas::OnSize(wxSizeEvent& evt) {
    wxSize s = evt.GetSize();
    backend_.Resize(s.GetWidth(), s.GetHeight());
    evt.Skip();
}

void xlVulkanCanvas::OnPaint(wxPaintEvent& WXUNUSED(evt)) {
    wxPaintDC dc(this);
    if (!backend_.IsInitialized()) return;
    if (!backend_.BeginFrame()) {
        // possibly handle recreate
    }
    backend_.EndFrame();
}

int xlVulkanCanvas::StartGrabImageAsync(wxSize size) {
    if (!backend_.IsInitialized()) return -1;
    int req = backend_.StartReadback(UINT32_MAX);
    if (req < 0) return -1;

    std::thread([this, req, size]() {
        std::vector<uint8_t> pixels;
        const int timeoutMs = 5000;
        const int pollMs = 5;
        int waited = 0;
        while (waited < timeoutMs) {
            if (backend_.TryGetReadback(req, pixels)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
            waited += pollMs;
        }
        if (pixels.empty()) return;

        uint32_t w = static_cast<uint32_t>(mWindowWidth);
        uint32_t h = static_cast<uint32_t>(mWindowHeight);
        if (w == 0 || h == 0) return;

        auto pixelBuf = std::make_unique<std::vector<uint8_t>>(std::move(pixels));
        {
            std::lock_guard<std::mutex> lk(async_mutex_);
            xlVulkanCanvas::AsyncGrabData d;
            d.pixels = std::move(pixelBuf);
            d.width = (int)w;
            d.height = (int)h;
            d.requestedSize = size;
            async_images_[req] = std::move(d);
        }
        VulkanGrabEvent ev(req);
        wxQueueEvent(this, ev.Clone());
    }).detach();

    return req;
}

int xlVulkanCanvas::StartGrabImageAsyncWithCallback(std::function<void(wxImage*)> cb, wxSize size) {
    int req = StartGrabImageAsync(size);
    if (req < 0) return -1;
    std::lock_guard<std::mutex> lk(async_mutex_);
    async_callbacks_[req] = std::move(cb);
    return req;
}

wxImage* xlVulkanCanvas::GetAsyncGrabbedImage(int requestId) {
    std::lock_guard<std::mutex> lk(async_mutex_);
    auto it = async_images_.find(requestId);
    if (it == async_images_.end()) return nullptr;
    auto &d = it->second;
    if (!d.pixels) {
        async_images_.erase(it);
        return nullptr;
    }
    int w = d.width;
    int h = d.height;
    wxImage* img = new wxImage(w, h);
    img->InitAlpha();
    unsigned char* data = img->GetData();
    unsigned char* alpha = img->GetAlpha();
    auto &pix = *d.pixels;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t idx = ((size_t)y * w + x) * 4;
            size_t dst = ((size_t)y * w + x) * 3;
            data[dst + 0] = pix[idx + 0];
            data[dst + 1] = pix[idx + 1];
            data[dst + 2] = pix[idx + 2];
            alpha[(size_t)y * w + x] = pix[idx + 3];
        }
    }
    if (d.requestedSize.GetWidth() > 0 && d.requestedSize.GetHeight() > 0 && (d.requestedSize.GetWidth() != w || d.requestedSize.GetHeight() != h)) {
        wxImage* res = new wxImage(img->Rescale(d.requestedSize.GetWidth(), d.requestedSize.GetHeight()));
        delete img;
        img = res;
    }
    async_images_.erase(it);
    return img;
}

void xlVulkanCanvas::OnGrabComplete(wxCommandEvent &evt) {
    int id = evt.GetId();
    std::function<void(wxImage*)> cb;
    {
        std::lock_guard<std::mutex> lk(async_mutex_);
        auto it = async_callbacks_.find(id);
        if (it != async_callbacks_.end()) {
            cb = it->second;
            async_callbacks_.erase(it);
        }
    }
    if (cb) {
        wxImage* img = GetAsyncGrabbedImage(id);
        cb(img);
    }
}

wxImage *xlVulkanCanvas::GrabImage( wxSize size ) {
    if (!backend_.IsInitialized()) return nullptr;

    int req = backend_.StartReadback(UINT32_MAX);
    if (req < 0) return nullptr;

    const int timeoutMs = 5000;
    const int pollMs = 5;
    int waited = 0;
    std::vector<uint8_t> pixels;
    while (waited < timeoutMs) {
        if (backend_.TryGetReadback(req, pixels)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
        waited += pollMs;
    }
    if (pixels.empty()) return nullptr;

    uint32_t w = static_cast<uint32_t>(mWindowWidth);
    uint32_t h = static_cast<uint32_t>(mWindowHeight);
    if (w == 0 || h == 0) return nullptr;

    wxImage *img = new wxImage((int)w, (int)h);
    img->InitAlpha();
    unsigned char *data = img->GetData();
    unsigned char *alpha = img->GetAlpha();
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = ((size_t)y * w + x) * 4;
            size_t dst = ((size_t)y * w + x) * 3;
            data[dst + 0] = pixels[idx + 0];
            data[dst + 1] = pixels[idx + 1];
            data[dst + 2] = pixels[idx + 2];
            alpha[(size_t)y * w + x] = pixels[idx + 3];
        }
    }

    if (size.GetWidth() > 0 && size.GetHeight() > 0 && (size.GetWidth() != (int)w || size.GetHeight() != (int)h)) {
        wxImage *res = new wxImage(img->Rescale(size.GetWidth(), size.GetHeight()));
        delete img;
        img = res;
    }

    return img;
}
