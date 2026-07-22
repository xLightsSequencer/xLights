#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Vulkan implementation of xlGraphicsContext, mirroring
// xlMetalGraphicsContext method-for-method.  One context is created per frame
// by xlVulkanCanvas::PrepareContextForDrawing and destroyed in FinishDrawing;
// draw calls record into the canvas's frame command buffer (inside an active
// render pass on the acquired swapchain image).

#ifdef HAVE_VULKAN

#include <stack>

#include <glm/glm.hpp>

#include "../xlGraphicsContext.h"
#include "IVulkanCanvas.h"
#include "VulkanFrameData.h"
#include "VulkanPipelineCache.h"

#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif
#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

class xlVulkanGraphicsContext : public xlGraphicsContext {
public:
    xlVulkanGraphicsContext(IVulkanCanvas* canvas);
    virtual ~xlVulkanGraphicsContext();

    virtual xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    virtual xlVertexAccumulator* createVertexAccumulator() override;
    virtual xlVertexColorAccumulator* createVertexColorAccumulator() override;
    virtual xlVertexTextureAccumulator* createVertexTextureAccumulator() override;
    virtual xlVertexIndexedColorAccumulator* createVertexIndexedColorAccumulator() override;
    virtual xlTexture* createTextureMipMaps(const std::vector<xlImage>& images, const std::string& name) override;
    virtual xlTexture* createTexture(const xlImage& image, const std::string& name, bool finalize = false) override;
    virtual xlTexture* createTexture(int w, int h, bool bgr, bool alpha) override;
    virtual xlTexture* createTextureForFont(const xlFontInfo& font) override;
    virtual xlGraphicsProgram* createGraphicsProgram() override;
    virtual std::unique_ptr<xlMesh> loadMeshFromObjFile(const std::string& file) override;

    virtual xlGraphicsContext* PushMatrix() override;
    virtual xlGraphicsContext* PopMatrix() override;
    virtual xlGraphicsContext* Translate(float x, float y, float z) override;
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) override;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4& m) override;
    virtual xlGraphicsContext* Scale(float w, float h, float z) override;
    virtual xlGraphicsContext* SetCamera(const glm::mat4& m) override;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4& m) override;
    virtual xlGraphicsContext* ScaleViewMatrix(float w, float h, float z) override;
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z) override;

    virtual xlGraphicsContext* enableBlending(bool e = true) override;

    virtual xlGraphicsContext* drawLines(xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexAccumulator* vac, const xlColor& c, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    virtual xlGraphicsContext* drawLines(xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexColorAccumulator* vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    virtual xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator* vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    virtual xlGraphicsContext* drawTexture(xlTexture* texture,
                                           float x, float y, float x2, float y2,
                                           float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                                           bool smoothScale = true,
                                           int brightness = 100, int alpha = 255) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, int brightness, uint8_t alpha, int start, int count) override;

    // Mesh drawing lands with the milestone-2 depth/mesh pipelines.
    virtual xlGraphicsContext* drawMeshSolids(xlMesh* mesh, int brightness, bool useViewMatrix) override;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh* mesh, int brightness) override;
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh* mesh, int brightness) override;

    virtual xlGraphicsContext* pushDebugContext(const std::string& label) override;
    virtual xlGraphicsContext* popDebugContext() override;

    // Transient per-frame vertex data (bump-allocated, retired after the
    // frame's fence).  Used for non-finalized accumulator uploads.
    bool arenaAlloc(VkDeviceSize size, VkBuffer& outBuffer, VkDeviceSize& outOffset, void*& outPtr);

private:
    bool setPipeline(VulkanPipelineCache::ShaderSet set, VkPrimitiveTopology topology, bool smoothPoints = false);
    // Builds/pushes VulkanMeshFrameData and binds the mesh pipeline; returns
    // false if the mesh pipeline layout is unavailable (push-constant limit).
    bool setMeshPipeline(VulkanPipelineCache::ShaderSet set, VkPrimitiveTopology topology, bool blending);
    void pushMeshFrameData(const xlColor& c, int brightness, bool useViewMatrix);
    void pushFrameData();
    void setPointSize(float ps, bool smoothPoints);
    void setFragmentColor(const xlColor& c);
    xlGraphicsContext* drawPrimitive(VkPrimitiveTopology topology, xlVertexAccumulator* vac, const xlColor& c, int start, int count);
    xlGraphicsContext* drawPrimitive(VkPrimitiveTopology topology, xlVertexColorAccumulator* vac, int start, int count);
    xlGraphicsContext* drawPrimitive(VkPrimitiveTopology topology, xlVertexIndexedColorAccumulator* vac, int start, int count);

    IVulkanCanvas* canvas = nullptr;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VulkanPipelineCache* cache = nullptr;

    VulkanFrameData frameData;
    bool frameDataChanged = true;
    bool blending = false;
    bool is3D = false;             // depth-test 2D draws inside a 3D viewport
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    std::stack<glm::mat4> matrixStack;
    std::stack<glm::mat4> modelStack;
    std::stack<glm::mat4> viewStack;
    VkPipeline lastPipeline = VK_NULL_HANDLE;

    struct ArenaChunk {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        uint8_t* mapped = nullptr;
        VkDeviceSize used = 0;
        VkDeviceSize size = 0;
    };
    std::vector<ArenaChunk> arena;
};

#endif
