#include "xlVulkanGraphicsContext.h"
#include <wx/window.h>
#include <wx/image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include "../xlMesh.h"


// Do not include <windows.h> here to avoid winsock header conflicts; treat native handle as void*

xlVulkanGraphicsContext::xlVulkanGraphicsContext(wxWindow* w, gfx::VulkanBackend* backend)
    : xlGraphicsContext(w), backend_(backend)
{
    // We assume the backend is already initialized by the canvas/window.
    (void)w;
    matrixStack_ = std::stack<glm::mat4>();
    projection_ = glm::mat4(1.0f);
    view_ = glm::mat4(1.0f);
    model_ = glm::mat4(1.0f);
}

xlVulkanGraphicsContext::~xlVulkanGraphicsContext() {
    // Do not shutdown backend_ here; the canvas owns it.
}

xlGraphicsContext* xlVulkanGraphicsContext::SetViewport(int x1, int y1, int x2, int y2, bool is3D) {
    // set projection matrix to orthographic matching window coords
    // use glm::ortho from the glm/gtc/matrix_transform.hpp header
    projection_ = glm::ortho<float>(0.0f, (float)x2, (float)y2, 0.0f, -1.0f, 1.0f);
    return this;
}

// Accumulators: provide simple CPU-side accumulators that the user code expects
class VulkanVertexAccumulator : public xlVertexAccumulator {
public:
    std::vector<float> data;
    virtual void Reset() override { data.clear(); }
    virtual void PreAlloc(unsigned int i) override { data.reserve(i*3); }
    virtual void AddVertex(float x, float y, float z) override { data.push_back(x); data.push_back(y); data.push_back(z); }
    virtual uint32_t getCount() override { return static_cast<uint32_t>(data.size()/3); }
};

class VulkanVertexColorAccumulator : public xlVertexColorAccumulator {
public:
    struct V { float x,y,z; xlColor c; };
    std::vector<V> data;
    virtual void Reset() override { data.clear(); }
    virtual void PreAlloc(unsigned int i) override { data.reserve(i); }
    virtual void AddVertex(float x, float y, float z, const xlColor &c) override { data.push_back({x,y,z,c}); }
    virtual uint32_t getCount() override { return static_cast<uint32_t>(data.size()); }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor &c) override { if (vertex < data.size()) data[vertex] = {x,y,z,c}; }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override { if (vertex < data.size()) { data[vertex].x=x; data[vertex].y=y; data[vertex].z=z; } }
    virtual void SetVertex(uint32_t vertex, const xlColor &c) override { if (vertex < data.size()) data[vertex].c = c; }
};

class VulkanVertexTextureAccumulator : public xlVertexTextureAccumulator {
public:
    struct V { float x,y,z,tx,ty; };
    std::vector<V> data;
    virtual void Reset() override { data.clear(); }
    virtual void PreAlloc(unsigned int i) override { data.reserve(i); }
    virtual void AddVertex(float x, float y, float z, float tx, float ty) override { data.push_back({x,y,z,tx,ty}); }
    virtual uint32_t getCount() override { return static_cast<uint32_t>(data.size()); }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, float tx, float ty) override { if (vertex < data.size()) data[vertex] = {x,y,z,tx,ty}; }
};

class VulkanTexture : public xlTexture {
public:
    int w,h;
    std::vector<uint8_t> pixels;
    int backendId = -1;
    VulkanTexture(int ww, int hh) : w(ww), h(hh) {}
    virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) override {
        if (x<0||y<0||x>=w||y>=h) return;
        int idx = (y*w + x)*4;
        // xlColor uses fields named red/green/blue/alpha
        pixels[idx+0] = c.red;
        pixels[idx+1] = c.green;
        pixels[idx+2] = c.blue;
        pixels[idx+3] = c.alpha;
    }
    virtual void UpdateData(uint8_t *data, bool bgr, bool alpha) override {
        pixels.assign(data, data + (size_t)w*h*(alpha?4:3));
    }
};

xlVertexAccumulator *xlVulkanGraphicsContext::createVertexAccumulator() { return new VulkanVertexAccumulator(); }
xlVertexColorAccumulator *xlVulkanGraphicsContext::createVertexColorAccumulator() { return new VulkanVertexColorAccumulator(); }
xlVertexTextureAccumulator *xlVulkanGraphicsContext::createVertexTextureAccumulator() { return new VulkanVertexTextureAccumulator(); }

// GPU-backed indexed color accumulator: uploads vertex/color buffers to Vulkan (host-visible coherent memory for now)
class VulkanVertexIndexedColorAccumulatorGPU : public xlVertexIndexedColorAccumulator {
public:
    struct V { float x,y,z; uint32_t cIdx; };
    std::vector<V> vertices;
    std::vector<xlColor> colors;

    gfx::VulkanBackend* backend = nullptr;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    VkDeviceSize vertexBufferCapacity = 0;

    VkBuffer colorBuffer = VK_NULL_HANDLE;
    VkDeviceMemory colorMemory = VK_NULL_HANDLE;
    VkDeviceSize colorBufferCapacity = 0;

    bool vertexDirty = false;
    bool colorDirty = false;

    VulkanVertexIndexedColorAccumulatorGPU(gfx::VulkanBackend* b) : backend(b) {}
    virtual ~VulkanVertexIndexedColorAccumulatorGPU() {
        if (backend) {
            backend->DestroyBuffer(vertexBuffer, vertexMemory);
            backend->DestroyBuffer(colorBuffer, colorMemory);
        }
    }

    virtual void Reset() override { vertices.clear(); colors.clear(); vertexDirty = colorDirty = false; }
    virtual void PreAlloc(unsigned int i) override { vertices.reserve(i); }
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override { vertices.push_back({x,y,z,cIdx}); vertexDirty = true; }
    virtual uint32_t getCount() override { return static_cast<uint32_t>(vertices.size()); }

    virtual void SetColorCount(int c) override { colors.assign((size_t)std::max(0, c), xlColor()); colorDirty = true; }
    virtual uint32_t GetColorCount() override { return static_cast<uint32_t>(colors.size()); }
    virtual void SetColor(uint32_t idx, const xlColor &c) override { if (idx < colors.size()) { colors[idx] = c; colorDirty = true; } }

    virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) override {
        (void)mayChangeVertices; (void)mayChangeColors;
        if (!backend) return;
        // ensure vertex buffer capacity
        VkDeviceSize vbSize = sizeof(V) * vertices.size();
        if (vbSize > vertexBufferCapacity) {
            // destroy old
            backend->DestroyBuffer(vertexBuffer, vertexMemory);
            vertexBuffer = VK_NULL_HANDLE; vertexMemory = VK_NULL_HANDLE; vertexBufferCapacity = 0;
            if (vbSize > 0) {
                if (!backend->CreateBuffer(vbSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexMemory)) {
                    // allocation failed; leave buffers null but keep CPU data
                    vertexBuffer = VK_NULL_HANDLE; vertexMemory = VK_NULL_HANDLE; vertexBufferCapacity = 0;
                } else {
                    vertexBufferCapacity = vbSize;
                }
            }
        }

        VkDeviceSize cbSize = sizeof(xlColor) * colors.size();
        if (cbSize > colorBufferCapacity) {
            backend->DestroyBuffer(colorBuffer, colorMemory);
            colorBuffer = VK_NULL_HANDLE; colorMemory = VK_NULL_HANDLE; colorBufferCapacity = 0;
            if (cbSize > 0) {
                if (!backend->CreateBuffer(cbSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, colorBuffer, colorMemory)) {
                    colorBuffer = VK_NULL_HANDLE; colorMemory = VK_NULL_HANDLE; colorBufferCapacity = 0;
                } else {
                    colorBufferCapacity = cbSize;
                }
            }
        }

        // upload data if buffers exist
        if (vertexBuffer != VK_NULL_HANDLE && vertexDirty && !vertices.empty()) {
            backend->UpdateBufferData(vertexMemory, 0, vertices.data(), sizeof(V) * vertices.size());
            vertexDirty = false;
        }
        if (colorBuffer != VK_NULL_HANDLE && colorDirty && !colors.empty()) {
            backend->UpdateBufferData(colorMemory, 0, colors.data(), sizeof(xlColor) * colors.size());
            colorDirty = false;
        }
    }

    virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx) override { if (vertex < vertices.size()) { vertices[vertex] = {x,y,z,cIdx}; vertexDirty = true; } }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override { if (vertex < vertices.size()) { vertices[vertex].x = x; vertices[vertex].y = y; vertices[vertex].z = z; vertexDirty = true; } }
    virtual void SetVertex(uint32_t vertex, uint32_t cIdx) override { if (vertex < vertices.size()) { vertices[vertex].cIdx = cIdx; vertexDirty = true; } }
    virtual void FlushRange(uint32_t start, uint32_t len) override { (void)start; (void)len; /* Full buffer upload on Finalize for now */ }
    virtual void FlushColors(uint32_t start, uint32_t len) override { (void)start; (void)len; /* Full color upload on Finalize for now */ }
};

xlVertexIndexedColorAccumulator *xlVulkanGraphicsContext::createVertexIndexedColorAccumulator() { return new VulkanVertexIndexedColorAccumulatorGPU(backend_); }

xlTexture *xlVulkanGraphicsContext::createTextureMipMaps(const std::vector<wxBitmap> &bitmaps, const std::string &name) { return nullptr; }
xlTexture *xlVulkanGraphicsContext::createTextureMipMaps(const std::vector<wxImage> &images, const std::string &name) { return nullptr; }
xlTexture *xlVulkanGraphicsContext::createTexture(const wxImage &image, const std::string &name, bool finalize) {
    int w = image.GetWidth();
    int h = image.GetHeight();
    VulkanTexture *t = new VulkanTexture(w,h);
    t->pixels.resize((size_t)w*h*4);
    unsigned char *srcData = image.GetData();
    for (int y=0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w + x);
            int src = idx * 3;
            t->pixels[idx * 4 + 0] = srcData[src + 0];
            t->pixels[idx * 4 + 1] = srcData[src + 1];
            t->pixels[idx * 4 + 2] = srcData[src + 2];
            t->pixels[idx * 4 + 3] = 255;
        }
    }
    // register GPU texture
    int id = backend_->CreateTextureFromPixels(t->pixels.data(), w, h);
    t->backendId = id;
    return t;
}
xlTexture *xlVulkanGraphicsContext::createTextureMipMaps(const std::vector<xlImage> &images, const std::string &name) { return nullptr; }
xlTexture *xlVulkanGraphicsContext::createTexture(const xlImage &image, const std::string &name, bool finalize) { return nullptr; }
xlTexture *xlVulkanGraphicsContext::createTexture(int w, int h, bool bgr, bool alpha) { VulkanTexture *t = new VulkanTexture(w,h); t->pixels.resize((size_t)w*h*4); return t; }
xlTexture *xlVulkanGraphicsContext::createTextureForFont(const xlFontInfo &font) { (void)font; return nullptr; }
xlGraphicsProgram *xlVulkanGraphicsContext::createGraphicsProgram() { 
    // Provide a basic program backed by a CPU-side color accumulator so higher-level code
    // can push draw steps into the program even if GPU pipeline is not implemented yet.
    return new xlGraphicsProgram(createVertexColorAccumulator()); 
}
std::unique_ptr<xlMesh> xlVulkanGraphicsContext::loadMeshFromObjFile(const std::string &file) {
    // Create an xlMesh that will parse the OBJ and load any referenced textures via this graphics context.
    try {
        return std::make_unique<xlMesh>(this, file);
    } catch (...) {
        // In case of any unexpected exception, return nullptr so callers can handle failure
        return nullptr;
    }
}

// matrix ops
xlGraphicsContext* xlVulkanGraphicsContext::PushMatrix() { matrixStack_.push(model_); return this; }
xlGraphicsContext* xlVulkanGraphicsContext::PopMatrix() { if (!matrixStack_.empty()) { model_ = matrixStack_.top(); matrixStack_.pop(); } return this; }
xlGraphicsContext* xlVulkanGraphicsContext::Translate(float x, float y, float z) { model_ = glm::translate(model_, glm::vec3(x,y,z)); return this; }
xlGraphicsContext* xlVulkanGraphicsContext::Rotate(float angle, float x, float y, float z) { model_ = glm::rotate(model_, angle, glm::vec3(x,y,z)); return this; }
xlGraphicsContext* xlVulkanGraphicsContext::ApplyMatrix(const glm::mat4 &m) { model_ = m * model_; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::Scale(float w, float h, float z) { model_ = glm::scale(model_, glm::vec3(w,h,z)); return this; }
xlGraphicsContext* xlVulkanGraphicsContext::SetCamera(const glm::mat4 &m) { view_ = m; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::SetModelMatrix(const glm::mat4 &m) { model_ = m; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::ScaleViewMatrix(float w, float h, float z) { view_ = glm::scale(view_, glm::vec3(w,h,z)); return this; }
xlGraphicsContext* xlVulkanGraphicsContext::TranslateViewMatrix(float x, float y, float z) { view_ = glm::translate(view_, glm::vec3(x,y,z)); return this; }

xlGraphicsContext* xlVulkanGraphicsContext::enableBlending(bool e) { blendingEnabled_ = e; return this; }

// Drawing methods: minimal implementations that do not yet upload to GPU.
xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    if (!vac || count <= 0 || start < 0) {
        return this;
    }
    
    VulkanVertexAccumulator* vulkanVac = dynamic_cast<VulkanVertexAccumulator*>(vac);
    if (!vulkanVac) {
        return this;
    }
    
    uint32_t totalVertices = vulkanVac->getCount();
    if (static_cast<uint32_t>(start + count) > totalVertices) {
        return this;
    }
    
    // TODO: Implement Vulkan line rendering pipeline
    (void)c;
    
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) { return drawLines(vac,c,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start, int count) { (void)vac; (void)c; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) { return drawTriangles(vac,c,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start, int count) { (void)vac; (void)c; (void)pointSize; (void)smoothPoints; (void)start; (void)count; return this; }

xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexColorAccumulator *vac, int start, int count) { (void)vac; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexColorAccumulator *vac, int start, int count) { return drawLines(vac,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexColorAccumulator *vac, int start, int count) { (void)vac; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac, int start, int count) { return drawTriangles(vac,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) { (void)vac; (void)pointSize; (void)smoothPoints; (void)start; (void)count; return this; }

xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexIndexedColorAccumulator *vac, int start, int count) { (void)vac; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) { return drawLines(vac,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexIndexedColorAccumulator *vac, int start, int count) { (void)vac; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) { return drawTriangles(vac,start,count); }
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) {
    if (!vac) return this;
    
    VulkanVertexIndexedColorAccumulatorGPU* gpuVac = dynamic_cast<VulkanVertexIndexedColorAccumulatorGPU*>(vac);
    if (!gpuVac) return this;
    
    if (gpuVac->vertexBuffer == VK_NULL_HANDLE || gpuVac->colorBuffer == VK_NULL_HANDLE) return this;
    
    (void)pointSize; (void)smoothPoints; (void)start; (void)count;
    
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlTexture *texture, float x, float y, float x2, float y2, float tx, float ty, float tx2, float ty2, bool smoothScale, int brightness, int alpha) {
    // For now, we just clear using backend; full textured quad rendering requires pipeline and descriptor sets.
    (void)tx; (void)ty; (void)tx2; (void)ty2; (void)smoothScale; (void)brightness; (void)alpha;
    VulkanTexture *vt = dynamic_cast<VulkanTexture*>(texture);
    if (!vt) return this;
    if (vt->backendId < 0) return this;
    // queue copy to swapchain - convert coords to integers
    uint32_t dstX = static_cast<uint32_t>(x);
    uint32_t dstY = static_cast<uint32_t>(y);
    uint32_t dstW = static_cast<uint32_t>(x2 - x);
    uint32_t dstH = static_cast<uint32_t>(y2 - y);
    backend_->QueueCopyTextureToSwapchain(vt->backendId, dstX, dstY, dstW, dstH);
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int start, int count) { (void)vac; (void)texture; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start, int count) { (void)vac; (void)texture; (void)c; (void)start; (void)count; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) { (void)vac; (void)texture; (void)brightness; (void)alpha; (void)start; (void)count; return this; }

xlGraphicsContext* xlVulkanGraphicsContext::drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) { (void)mesh; (void)brightness; (void)useViewMatrix; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawMeshTransparents(xlMesh *mesh, int brightness) { (void)mesh; (void)brightness; return this; }
xlGraphicsContext* xlVulkanGraphicsContext::drawMeshWireframe(xlMesh *mesh, int brightness) { (void)mesh; (void)brightness; return this; }
