/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef HAVE_VULKAN

#include "xlVulkanGraphicsContext.h"

#include <cstring>
#include <functional>

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "../xlMesh.h"
#include "VulkanMeshFrameData.h"
#include "../../effects/vulkan/VulkanComputeUtilities.h"

// ─── one-shot upload helper ─────────────────────────────────────────────────
// Texture uploads happen mid-frame (createTexture is called while drawing) but
// cannot be recorded inside the frame's active render pass, so they run on
// their own transient command buffer submitted (and waited) immediately —
// the Vulkan analogue of Metal's synchronous replaceRegion:.
static bool oneShotGraphicsSubmit(const std::function<void(VkCommandBuffer)>& record) {
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
    VkCommandPoolCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pci.queueFamilyIndex = vk.graphicsQueueFamilyIndex;
    VkCommandPool pool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(vk.device, &pci, nullptr, &pool) != VK_SUCCESS) {
        return false;
    }
    VkCommandBufferAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = pool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = 1;
    VkCommandBuffer cb = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(vk.device, &ai, &cb);

    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cb, &bi);
    record(cb);
    vkEndCommandBuffer(cb);

    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence = VK_NULL_HANDLE;
    vkCreateFence(vk.device, &fci, nullptr, &fence);

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cb;
    bool ok;
    {
        std::lock_guard<std::mutex> lock(vk.graphicsSubmitMutex());
        ok = vkQueueSubmit(vk.graphicsQueue, 1, &si, fence) == VK_SUCCESS;
    }
    if (ok) {
        vkWaitForFences(vk.device, 1, &fence, VK_TRUE, UINT64_MAX);
    }
    vkDestroyFence(vk.device, fence, nullptr);
    vkDestroyCommandPool(vk.device, pool, nullptr);
    return ok;
}

static bool createHostBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation, void*& mapped) {
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo aci = {};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo info = {};
    if (vmaCreateBuffer(VulkanComputeUtilities::INSTANCE.allocator, &bci, &aci, &buffer, &allocation, &info) != VK_SUCCESS) {
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        mapped = nullptr;
        return false;
    }
    mapped = info.pMappedData;
    return true;
}

// ─── accumulators ───────────────────────────────────────────────────────────
// CPU-side vectors, mirroring the Metal accumulators.  Non-finalized data is
// copied into the context's per-frame arena at bind time (the analogue of
// Metal's setVertexBytes); Finalize creates a persistently-mapped VkBuffer
// that SetVertex/SetColor write directly (the analogue of buffer.contents).

struct xlVulkanBufferHolder {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint8_t* mapped = nullptr;
    VkDeviceSize capacity = 0;

    ~xlVulkanBufferHolder() {
        VulkanPipelineCache::INSTANCE.deferDestroyBuffer(buffer, allocation);
    }
    bool ensure(VkDeviceSize size, VkBufferUsageFlags usage) {
        if (size <= capacity) {
            return buffer != VK_NULL_HANDLE;
        }
        VulkanPipelineCache::INSTANCE.deferDestroyBuffer(buffer, allocation);
        void* m = nullptr;
        if (!createHostBuffer(size, usage, buffer, allocation, m)) {
            capacity = 0;
            return false;
        }
        mapped = (uint8_t*)m;
        capacity = size;
        return true;
    }
};

class xlVulkanVertexAccumulator : public xlVertexAccumulator {
public:
    struct V3 {
        float x, y, z;
    };
    std::vector<V3> vertices;
    bool finalized = false;
    xlVulkanBufferHolder vbuf;

    virtual void Reset() override {
        if (!finalized) {
            vertices.clear();
        }
    }
    virtual void PreAlloc(unsigned int i) override { vertices.reserve(vertices.size() + i); }
    virtual void AddVertex(float x, float y, float z) override {
        if (!finalized) {
            vertices.push_back({ x, y, z });
        }
    }
    virtual uint32_t getCount() override { return (uint32_t)vertices.size(); }

    virtual void Finalize(bool mayChange) override {
        finalized = true;
        VkDeviceSize sz = sizeof(V3) * vertices.size();
        if (sz > 0 && vbuf.ensure(sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(vbuf.mapped, vertices.data(), sz);
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < vertices.size()) {
            vertices[vertex] = { x, y, z };
            if (finalized && vbuf.mapped != nullptr) {
                memcpy(vbuf.mapped + vertex * sizeof(V3), &vertices[vertex], sizeof(V3));
            }
        }
    }

    bool bind(xlVulkanGraphicsContext* ctx, VkCommandBuffer cmd) {
        if (vertices.empty()) {
            return false;
        }
        if (finalized && vbuf.buffer != VK_NULL_HANDLE) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf.buffer, &offset);
            return true;
        }
        VkBuffer buf;
        VkDeviceSize offset;
        void* ptr;
        if (!ctx->arenaAlloc(sizeof(V3) * vertices.size(), buf, offset, ptr)) {
            return false;
        }
        memcpy(ptr, vertices.data(), sizeof(V3) * vertices.size());
        vkCmdBindVertexBuffers(cmd, 0, 1, &buf, &offset);
        return true;
    }
};

class xlVulkanVertexColorAccumulator : public xlVertexColorAccumulator {
public:
    struct V3 {
        float x, y, z;
    };
    struct C4 {
        uint8_t r, g, b, a;
    };
    std::vector<V3> vertices;
    std::vector<C4> colors;
    bool finalized = false;
    xlVulkanBufferHolder vbuf;
    xlVulkanBufferHolder cbuf;

    virtual void Reset() override {
        if (!finalized) {
            vertices.clear();
            colors.clear();
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(vertices.size() + i);
        colors.reserve(colors.size() + i);
    }
    virtual void AddVertex(float x, float y, float z, const xlColor& c) override {
        if (!finalized) {
            vertices.push_back({ x, y, z });
            colors.push_back({ c.red, c.green, c.blue, c.alpha });
        }
    }
    virtual uint32_t getCount() override { return (uint32_t)vertices.size(); }

    virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) override {
        finalized = true;
        VkDeviceSize vs = sizeof(V3) * vertices.size();
        if (vs > 0 && vbuf.ensure(vs, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(vbuf.mapped, vertices.data(), vs);
        }
        VkDeviceSize cs = sizeof(C4) * colors.size();
        if (cs > 0 && cbuf.ensure(cs, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(cbuf.mapped, colors.data(), cs);
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor& c) override {
        SetVertex(vertex, x, y, z);
        SetVertex(vertex, c);
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < vertices.size()) {
            vertices[vertex] = { x, y, z };
            if (finalized && vbuf.mapped != nullptr) {
                memcpy(vbuf.mapped + vertex * sizeof(V3), &vertices[vertex], sizeof(V3));
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, const xlColor& c) override {
        if (vertex < colors.size()) {
            colors[vertex] = { c.red, c.green, c.blue, c.alpha };
            if (finalized && cbuf.mapped != nullptr) {
                memcpy(cbuf.mapped + vertex * sizeof(C4), &colors[vertex], sizeof(C4));
            }
        }
    }

    bool bind(xlVulkanGraphicsContext* ctx, VkCommandBuffer cmd) {
        if (vertices.empty()) {
            return false;
        }
        if (finalized && vbuf.buffer != VK_NULL_HANDLE && cbuf.buffer != VK_NULL_HANDLE) {
            VkBuffer bufs[2] = { vbuf.buffer, cbuf.buffer };
            VkDeviceSize offsets[2] = { 0, 0 };
            vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);
            return true;
        }
        VkBuffer vb, cb;
        VkDeviceSize vo, co;
        void* vp;
        void* cp;
        if (!ctx->arenaAlloc(sizeof(V3) * vertices.size(), vb, vo, vp) ||
            !ctx->arenaAlloc(sizeof(C4) * colors.size(), cb, co, cp)) {
            return false;
        }
        memcpy(vp, vertices.data(), sizeof(V3) * vertices.size());
        memcpy(cp, colors.data(), sizeof(C4) * colors.size());
        VkBuffer bufs[2] = { vb, cb };
        VkDeviceSize offsets[2] = { vo, co };
        vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);
        return true;
    }
};

class xlVulkanVertexTextureAccumulator : public xlVertexTextureAccumulator {
public:
    struct V3 {
        float x, y, z;
    };
    struct T2 {
        float tx, ty;
    };
    std::vector<V3> vertices;
    std::vector<T2> tvertices;
    bool finalized = false;
    xlVulkanBufferHolder vbuf;
    xlVulkanBufferHolder tbuf;

    virtual void Reset() override {
        if (!finalized) {
            vertices.clear();
            tvertices.clear();
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(vertices.size() + i);
        tvertices.reserve(tvertices.size() + i);
    }
    virtual void AddVertex(float x, float y, float z, float tx, float ty) override {
        if (!finalized) {
            vertices.push_back({ x, y, z });
            tvertices.push_back({ tx, ty });
        }
    }
    virtual uint32_t getCount() override { return (uint32_t)vertices.size(); }

    virtual void Finalize(bool mayChangeV, bool mayChangeT) override {
        finalized = true;
        VkDeviceSize vs = sizeof(V3) * vertices.size();
        if (vs > 0 && vbuf.ensure(vs, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(vbuf.mapped, vertices.data(), vs);
        }
        VkDeviceSize ts = sizeof(T2) * tvertices.size();
        if (ts > 0 && tbuf.ensure(ts, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(tbuf.mapped, tvertices.data(), ts);
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, float tx, float ty) override {
        if (vertex < vertices.size()) {
            vertices[vertex] = { x, y, z };
            tvertices[vertex] = { tx, ty };
            if (finalized) {
                if (vbuf.mapped != nullptr) {
                    memcpy(vbuf.mapped + vertex * sizeof(V3), &vertices[vertex], sizeof(V3));
                }
                if (tbuf.mapped != nullptr) {
                    memcpy(tbuf.mapped + vertex * sizeof(T2), &tvertices[vertex], sizeof(T2));
                }
            }
        }
    }

    bool bind(xlVulkanGraphicsContext* ctx, VkCommandBuffer cmd) {
        if (vertices.empty()) {
            return false;
        }
        if (finalized && vbuf.buffer != VK_NULL_HANDLE && tbuf.buffer != VK_NULL_HANDLE) {
            VkBuffer bufs[2] = { vbuf.buffer, tbuf.buffer };
            VkDeviceSize offsets[2] = { 0, 0 };
            vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);
            return true;
        }
        VkBuffer vb, tb;
        VkDeviceSize vo, to;
        void* vp;
        void* tp;
        if (!ctx->arenaAlloc(sizeof(V3) * vertices.size(), vb, vo, vp) ||
            !ctx->arenaAlloc(sizeof(T2) * tvertices.size(), tb, to, tp)) {
            return false;
        }
        memcpy(vp, vertices.data(), sizeof(V3) * vertices.size());
        memcpy(tp, tvertices.data(), sizeof(T2) * tvertices.size());
        VkBuffer bufs[2] = { vb, tb };
        VkDeviceSize offsets[2] = { vo, to };
        vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);
        return true;
    }
};

class xlVulkanVertexIndexedColorAccumulator : public xlVertexIndexedColorAccumulator {
public:
    struct V {
        float x, y, z;
        uint32_t cIdx;
    };
    std::vector<V> vertices;
    std::vector<uint32_t> colors; // packed RGBA8, matching the SSBO
    bool finalized = false;
    xlVulkanBufferHolder vbuf;
    xlVulkanBufferHolder cbuf;
    VkDescriptorSet colorTableSet = VK_NULL_HANDLE;

    virtual ~xlVulkanVertexIndexedColorAccumulator() {
        // deferDestroyImage handles descriptor sets on the deferred path
        VulkanPipelineCache::INSTANCE.deferDestroyImage(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, colorTableSet);
    }

    static uint32_t pack(const xlColor& c) {
        return ((uint32_t)c.red) | ((uint32_t)c.green << 8) | ((uint32_t)c.blue << 16) | ((uint32_t)c.alpha << 24);
    }

    virtual void Reset() override {
        if (!finalized) {
            vertices.clear();
        }
    }
    virtual void PreAlloc(unsigned int i) override { vertices.reserve(vertices.size() + i); }
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override {
        if (!finalized) {
            vertices.push_back({ x, y, z, cIdx });
        }
    }
    virtual uint32_t getCount() override { return (uint32_t)vertices.size(); }

    virtual void SetColorCount(int c) override {
        colors.resize((size_t)std::max(0, c), 0xFF000000u);
        if (finalized) {
            ensureColorTable();
        }
    }
    virtual uint32_t GetColorCount() override { return (uint32_t)colors.size(); }
    virtual void SetColor(uint32_t idx, const xlColor& c) override {
        if (idx < colors.size()) {
            colors[idx] = pack(c);
            if (finalized && cbuf.mapped != nullptr && (idx + 1) * sizeof(uint32_t) <= cbuf.capacity) {
                memcpy(cbuf.mapped + idx * sizeof(uint32_t), &colors[idx], sizeof(uint32_t));
            }
        }
    }

    void ensureColorTable() {
        VkDeviceSize cs = sizeof(uint32_t) * colors.size();
        if (cs == 0) {
            return;
        }
        bool grew = cs > cbuf.capacity;
        if (cbuf.ensure(cs, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
            memcpy(cbuf.mapped, colors.data(), cs);
            if (colorTableSet == VK_NULL_HANDLE) {
                colorTableSet = VulkanPipelineCache::INSTANCE.allocateColorTableSet(cbuf.buffer, cbuf.capacity);
            } else if (grew) {
                VulkanPipelineCache::INSTANCE.updateColorTableSet(colorTableSet, cbuf.buffer, cbuf.capacity);
            }
        }
    }

    virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) override {
        finalized = true;
        VkDeviceSize vs = sizeof(V) * vertices.size();
        if (vs > 0 && vbuf.ensure(vs, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
            memcpy(vbuf.mapped, vertices.data(), vs);
        }
        ensureColorTable();
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx) override {
        if (vertex < vertices.size()) {
            vertices[vertex] = { x, y, z, cIdx };
            if (finalized && vbuf.mapped != nullptr) {
                memcpy(vbuf.mapped + vertex * sizeof(V), &vertices[vertex], sizeof(V));
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < vertices.size()) {
            vertices[vertex].x = x;
            vertices[vertex].y = y;
            vertices[vertex].z = z;
            if (finalized && vbuf.mapped != nullptr) {
                memcpy(vbuf.mapped + vertex * sizeof(V), &vertices[vertex], sizeof(V));
            }
        }
    }
    virtual void SetVertex(uint32_t vertex, uint32_t cIdx) override {
        if (vertex < vertices.size()) {
            vertices[vertex].cIdx = cIdx;
            if (finalized && vbuf.mapped != nullptr) {
                memcpy(vbuf.mapped + vertex * sizeof(V), &vertices[vertex], sizeof(V));
            }
        }
    }

    // Finalized path: bind the interleaved vertex buffer + color-table set.
    bool bindIndexed(VkCommandBuffer cmd, VkPipelineLayout layout) {
        if (vertices.empty() || !finalized || vbuf.buffer == VK_NULL_HANDLE || colorTableSet == VK_NULL_HANDLE) {
            return false;
        }
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf.buffer, &offset);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &colorTableSet, 0, nullptr);
        return true;
    }

    // Non-finalized fallback: expand to position + resolved-color streams in
    // the frame arena and draw with the multi-color pipeline (no per-draw
    // descriptor churn for transient data).
    bool bindExpanded(xlVulkanGraphicsContext* ctx, VkCommandBuffer cmd) {
        if (vertices.empty()) {
            return false;
        }
        VkBuffer vb, cb;
        VkDeviceSize vo, co;
        void* vp;
        void* cp;
        if (!ctx->arenaAlloc(12ull * vertices.size(), vb, vo, vp) ||
            !ctx->arenaAlloc(4ull * vertices.size(), cb, co, cp)) {
            return false;
        }
        float* verts = (float*)vp;
        uint32_t* cols = (uint32_t*)cp;
        for (size_t i = 0; i < vertices.size(); i++) {
            verts[i * 3] = vertices[i].x;
            verts[i * 3 + 1] = vertices[i].y;
            verts[i * 3 + 2] = vertices[i].z;
            cols[i] = vertices[i].cIdx < colors.size() ? colors[vertices[i].cIdx] : 0xFF000000u;
        }
        VkBuffer bufs[2] = { vb, cb };
        VkDeviceSize offsets[2] = { vo, co };
        vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);
        return true;
    }
};

// ─── texture ────────────────────────────────────────────────────────────────

class xlVulkanTexture : public xlTexture {
public:
    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    int width = 0;
    int height = 0;
    bool initialized = false; // has left VK_IMAGE_LAYOUT_UNDEFINED
    std::vector<uint8_t> cpuPixels;
    bool dirty = false;

    virtual ~xlVulkanTexture() {
        VulkanPipelineCache::INSTANCE.deferDestroyImage(image, allocation, view, descriptorSet);
    }

    bool create(int w, int h, VkFormat fmt) {
        width = w;
        height = h;
        format = fmt;
        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = fmt;
        ici.extent = { (uint32_t)w, (uint32_t)h, 1 };
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocationCreateInfo aci = {};
        aci.usage = VMA_MEMORY_USAGE_AUTO;
        if (vmaCreateImage(VulkanComputeUtilities::INSTANCE.allocator, &ici, &aci, &image, &allocation, nullptr) != VK_SUCCESS) {
            return false;
        }
        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = image;
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = fmt;
        vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkCreateImageView(VulkanComputeUtilities::INSTANCE.device, &vci, nullptr, &view) != VK_SUCCESS) {
            return false;
        }
        descriptorSet = VulkanPipelineCache::INSTANCE.allocateTextureSet(view);
        return descriptorSet != VK_NULL_HANDLE;
    }

    // Upload RGBA/BGRA rows (4 bytes/pixel, matching `format`) to the image.
    bool upload(const uint8_t* pixels) {
        VkDeviceSize size = (VkDeviceSize)width * height * 4;
        VkBuffer staging = VK_NULL_HANDLE;
        VmaAllocation stagingAlloc = VK_NULL_HANDLE;
        void* mapped = nullptr;
        if (!createHostBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging, stagingAlloc, mapped)) {
            return false;
        }
        memcpy(mapped, pixels, size);

        bool wasInitialized = initialized;
        bool ok = oneShotGraphicsSubmit([&](VkCommandBuffer cb) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = wasInitialized ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            barrier.srcAccessMask = wasInitialized ? VK_ACCESS_SHADER_READ_BIT : 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            vkCmdPipelineBarrier(cb, wasInitialized ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkBufferImageCopy region = {};
            region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
            vkCmdCopyBufferToImage(cb, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);
        });
        vmaDestroyBuffer(VulkanComputeUtilities::INSTANCE.allocator, staging, stagingAlloc);
        initialized = initialized || ok;
        return ok;
    }

    // Textures created empty must still be in a legal layout before sampling.
    bool initializeEmpty() {
        cpuPixels.assign((size_t)width * height * 4, 0);
        return upload(cpuPixels.data());
    }

    void flushDirty() {
        if (dirty && !cpuPixels.empty()) {
            upload(cpuPixels.data());
            dirty = false;
        }
    }

    virtual void UpdatePixel(int x, int y, const xlColor& c, bool copyAlpha) override {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return;
        }
        if (cpuPixels.empty()) {
            cpuPixels.assign((size_t)width * height * 4, 0);
        }
        size_t idx = ((size_t)y * width + x) * 4;
        if (format == VK_FORMAT_B8G8R8A8_UNORM) {
            cpuPixels[idx] = c.blue;
            cpuPixels[idx + 1] = c.green;
            cpuPixels[idx + 2] = c.red;
        } else {
            cpuPixels[idx] = c.red;
            cpuPixels[idx + 1] = c.green;
            cpuPixels[idx + 2] = c.blue;
        }
        cpuPixels[idx + 3] = copyAlpha ? c.alpha : 255;
        dirty = true;
    }

    virtual void UpdateData(uint8_t* data, bool bgr, bool alpha) override {
        if (cpuPixels.empty()) {
            cpuPixels.assign((size_t)width * height * 4, 0);
        }
        // Source is tightly packed rows; expand to the image's 4-byte format.
        const bool formatIsBgr = format == VK_FORMAT_B8G8R8A8_UNORM;
        const bool swap = bgr != formatIsBgr;
        size_t px = (size_t)width * height;
        const int srcStride = alpha ? 4 : 3;
        for (size_t i = 0; i < px; i++) {
            const uint8_t* s = data + i * srcStride;
            uint8_t* d = &cpuPixels[i * 4];
            if (swap) {
                d[0] = s[2];
                d[1] = s[1];
                d[2] = s[0];
            } else {
                d[0] = s[0];
                d[1] = s[1];
                d[2] = s[2];
            }
            d[3] = alpha ? s[3] : 255;
        }
        dirty = true;
    }
};

// ─── context ────────────────────────────────────────────────────────────────

xlVulkanGraphicsContext::xlVulkanGraphicsContext(IVulkanCanvas* c) : canvas(c) {
    cache = c->getPipelineCache();
    cmd = c->getFrameCommandBuffer();

    frameData.MVP = glm::mat4(1.0f);
    frameData.fragmentColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    frameData.pointSize = 0;
    frameData.pointSmoothMin = 0.25f;
    frameData.pointSmoothMax = 0.5f;
    frameData.brightness = 1.0f;
    frameData.renderType = VkRenderTypeNormal;
    frameDataChanged = true;

    // Dynamic viewport/scissor must be set before the first draw even if the
    // consumer never calls SetViewport.
    VkExtent2D extent = c->getFrameExtent();
    VkViewport vp = { 0.0f, (float)extent.height, (float)extent.width, -(float)extent.height, 0.0f, 1.0f };
    vkCmdSetViewport(cmd, 0, 1, &vp);
    VkRect2D scissor = { { 0, 0 }, extent };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

xlVulkanGraphicsContext::~xlVulkanGraphicsContext() {
    for (auto& chunk : arena) {
        VulkanPipelineCache::INSTANCE.deferDestroyBuffer(chunk.buffer, chunk.allocation);
    }
}

bool xlVulkanGraphicsContext::arenaAlloc(VkDeviceSize size, VkBuffer& outBuffer, VkDeviceSize& outOffset, void*& outPtr) {
    constexpr VkDeviceSize ALIGN = 16;
    constexpr VkDeviceSize CHUNK = 1024 * 1024;
    if (!arena.empty()) {
        ArenaChunk& c = arena.back();
        VkDeviceSize offset = (c.used + ALIGN - 1) & ~(ALIGN - 1);
        if (offset + size <= c.size) {
            c.used = offset + size;
            outBuffer = c.buffer;
            outOffset = offset;
            outPtr = c.mapped + offset;
            return true;
        }
    }
    ArenaChunk c;
    c.size = std::max(CHUNK, size);
    void* mapped = nullptr;
    if (!createHostBuffer(c.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, c.buffer, c.allocation, mapped)) {
        return false;
    }
    c.mapped = (uint8_t*)mapped;
    c.used = size;
    arena.push_back(c);
    outBuffer = c.buffer;
    outOffset = 0;
    outPtr = c.mapped;
    return true;
}

bool xlVulkanGraphicsContext::setPipeline(VulkanPipelineCache::ShaderSet set, VkPrimitiveTopology topology, bool smoothPoints) {
    VkPipeline p = cache->getPipeline(set, topology, blending, smoothPoints, is3D);
    if (p == VK_NULL_HANDLE) {
        return false;
    }
    if (p != lastPipeline) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
        lastPipeline = p;
    }
    return true;
}

void xlVulkanGraphicsContext::pushFrameData() {
    if (frameDataChanged) {
        vkCmdPushConstants(cmd, cache->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(VulkanFrameData), &frameData);
        frameDataChanged = false;
    }
}

void xlVulkanGraphicsContext::setFragmentColor(const xlColor& c) {
    glm::vec4 fc = { c.red / 255.0f, c.green / 255.0f, c.blue / 255.0f, c.alpha / 255.0f };
    if (fc != frameData.fragmentColor) {
        frameData.fragmentColor = fc;
        frameDataChanged = true;
    }
}

void xlVulkanGraphicsContext::setPointSize(float ps, bool smoothPoints) {
    ps += 1.0f;
    int32_t nrt = smoothPoints ? VkRenderTypePointsSmooth : VkRenderTypePoints;
    if (frameData.renderType != nrt) {
        frameData.renderType = nrt;
        frameDataChanged = true;
    }
    if (frameData.pointSize != ps) {
        frameData.pointSize = ps;
        float delta = 1.0f / ps;
        float mid = 0.35f + 0.15f * ((ps - 2.0f) / 25.0f);
        if (mid > 0.5f) {
            mid = 0.5f;
        }
        frameData.pointSmoothMin = std::max(0.0f, mid - delta);
        frameData.pointSmoothMax = std::min(1.0f, mid + delta);
        frameDataChanged = true;
    }
}

xlGraphicsContext* xlVulkanGraphicsContext::SetViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, bool is3DViewport) {
    is3D = is3DViewport;
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    double x = topleft_x;
    double y = std::min(bottomright_y, topleft_y);
    double w = std::abs((double)bottomright_x - topleft_x);
    double h = std::abs((double)bottomright_y - topleft_y);
    if (canvas->drawingUsingLogicalSize()) {
        x = canvas->translateToBacking(x);
        y = canvas->translateToBacking(y);
        w = canvas->translateToBacking(w);
        h = canvas->translateToBacking(h);
    }
    // Negative-height viewport (core in Vulkan 1.1) flips NDC +Y to up, so the
    // Metal/GL projection matrices carry over unchanged.
    VkViewport vp = { (float)x, (float)(y + h), (float)w, (float)-h, 0.0f, 1.0f };
    vkCmdSetViewport(cmd, 0, 1, &vp);
    VkExtent2D extent = canvas->getFrameExtent();
    int32_t sx = std::max(0, (int32_t)x);
    int32_t sy = std::max(0, (int32_t)y);
    VkRect2D scissor = { { sx, sy },
                         { (uint32_t)std::max(0.0, std::min((double)extent.width - sx, w)),
                           (uint32_t)std::max(0.0, std::min((double)extent.height - sy, h)) } };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    if (is3D) {
        frameData.MVP = glm::perspective(glm::radians(45.0f), (float)(bottomright_x - topleft_x) / (float)(topleft_y - bottomright_y), 1.0f, 200000.0f);
    } else {
        frameData.MVP = glm::orthoLH_ZO((float)topleft_x, (float)bottomright_x, (float)bottomright_y, (float)topleft_y, 1.0f, 0.0f);
    }
    frameDataChanged = true;
    return this;
}

// ─── creators ───────────────────────────────────────────────────────────────

xlVertexAccumulator* xlVulkanGraphicsContext::createVertexAccumulator() {
    return new xlVulkanVertexAccumulator();
}
xlVertexColorAccumulator* xlVulkanGraphicsContext::createVertexColorAccumulator() {
    return new xlVulkanVertexColorAccumulator();
}
xlVertexTextureAccumulator* xlVulkanGraphicsContext::createVertexTextureAccumulator() {
    return new xlVulkanVertexTextureAccumulator();
}
xlVertexIndexedColorAccumulator* xlVulkanGraphicsContext::createVertexIndexedColorAccumulator() {
    return new xlVulkanVertexIndexedColorAccumulator();
}

xlTexture* xlVulkanGraphicsContext::createTexture(const xlImage& image, const std::string& name, bool finalize) {
    if (!image.IsOk()) {
        return nullptr;
    }
    xlVulkanTexture* t = new xlVulkanTexture();
    t->SetName(name);
    if (!t->create(image.GetWidth(), image.GetHeight(), VK_FORMAT_R8G8B8A8_UNORM) || !t->upload(image.GetData())) {
        delete t;
        return nullptr;
    }
    return t;
}

xlTexture* xlVulkanGraphicsContext::createTextureMipMaps(const std::vector<xlImage>& images, const std::string& name) {
    // The graphics samplers use mip_filter none (matching Metal), so only the
    // base level is ever sampled — upload just that.
    if (images.empty()) {
        return nullptr;
    }
    return createTexture(images[0], name, true);
}

xlTexture* xlVulkanGraphicsContext::createTexture(int w, int h, bool bgr, bool alpha) {
    xlVulkanTexture* t = new xlVulkanTexture();
    if (!t->create(w, h, bgr ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM) || !t->initializeEmpty()) {
        delete t;
        return nullptr;
    }
    return t;
}

xlTexture* xlVulkanGraphicsContext::createTextureForFont(const xlFontInfo& font) {
    return createTexture(font.getImage(), "Font-" + std::to_string(font.getSize()), true);
}

xlGraphicsProgram* xlVulkanGraphicsContext::createGraphicsProgram() {
    return new xlGraphicsProgram(createVertexColorAccumulator());
}

// ─── matrices ───────────────────────────────────────────────────────────────

xlGraphicsContext* xlVulkanGraphicsContext::PushMatrix() {
    matrixStack.push(frameData.MVP);
    modelStack.push(modelMatrix);
    viewStack.push(viewMatrix);
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::PopMatrix() {
    if (!matrixStack.empty()) {
        if (frameData.MVP != matrixStack.top()) {
            frameData.MVP = matrixStack.top();
            frameDataChanged = true;
        }
        matrixStack.pop();
        modelMatrix = modelStack.top();
        modelStack.pop();
        viewMatrix = viewStack.top();
        viewStack.pop();
    }
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::Translate(float x, float y, float z) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    frameData.MVP = frameData.MVP * m;
    modelMatrix = modelMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::Rotate(float angle, float x, float y, float z) {
    glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(x, y, z));
    frameData.MVP = frameData.MVP * m;
    modelMatrix = modelMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::ApplyMatrix(const glm::mat4& m) {
    frameData.MVP = frameData.MVP * m;
    modelMatrix = modelMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::Scale(float w, float h, float z) {
    glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, z));
    frameData.MVP = frameData.MVP * m;
    modelMatrix = modelMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::SetCamera(const glm::mat4& m) {
    frameData.MVP = frameData.MVP * m;
    viewMatrix = viewMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::SetModelMatrix(const glm::mat4& m) {
    frameData.MVP = frameData.MVP * m;
    modelMatrix = m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::ScaleViewMatrix(float w, float h, float z) {
    glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, z));
    frameData.MVP = frameData.MVP * m;
    viewMatrix = viewMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::TranslateViewMatrix(float x, float y, float z) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    frameData.MVP = frameData.MVP * m;
    viewMatrix = viewMatrix * m;
    frameDataChanged = true;
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::enableBlending(bool e) {
    blending = e;
    return this;
}

// ─── draws ──────────────────────────────────────────────────────────────────

static int drawCount(uint32_t total, int start, int count) {
    int c = count;
    if (c < 0) {
        c = (int)total - start;
    }
    return c;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawPrimitive(VkPrimitiveTopology topology, xlVertexAccumulator* vac, const xlColor& c, int start, int count) {
    if (vac == nullptr || vac->getCount() == 0) {
        return this;
    }
    bool smooth = topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST && frameData.renderType == VkRenderTypePointsSmooth;
    if (!setPipeline(VulkanPipelineCache::ShaderSet::SingleColor, topology, smooth)) {
        return this;
    }
    if (!((xlVulkanVertexAccumulator*)vac)->bind(this, cmd)) {
        return this;
    }
    setFragmentColor(c);
    pushFrameData();
    int ct = drawCount(vac->getCount(), start, count);
    if (ct > 0) {
        vkCmdDraw(cmd, ct, 1, start, 0);
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawPrimitive(VkPrimitiveTopology topology, xlVertexColorAccumulator* vac, int start, int count) {
    if (vac == nullptr || vac->getCount() == 0) {
        return this;
    }
    bool smooth = topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST && frameData.renderType == VkRenderTypePointsSmooth;
    if (!setPipeline(VulkanPipelineCache::ShaderSet::MultiColor, topology, smooth)) {
        return this;
    }
    if (!((xlVulkanVertexColorAccumulator*)vac)->bind(this, cmd)) {
        return this;
    }
    pushFrameData();
    int ct = drawCount(vac->getCount(), start, count);
    if (ct > 0) {
        vkCmdDraw(cmd, ct, 1, start, 0);
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawPrimitive(VkPrimitiveTopology topology, xlVertexIndexedColorAccumulator* vac, int start, int count) {
    if (vac == nullptr || vac->getCount() == 0) {
        return this;
    }
    xlVulkanVertexIndexedColorAccumulator* va = (xlVulkanVertexIndexedColorAccumulator*)vac;
    bool smooth = topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST && frameData.renderType == VkRenderTypePointsSmooth;
    if (va->finalized) {
        if (!setPipeline(VulkanPipelineCache::ShaderSet::IndexedColor, topology, smooth)) {
            return this;
        }
        if (!va->bindIndexed(cmd, cache->getPipelineLayout())) {
            return this;
        }
    } else {
        if (!setPipeline(VulkanPipelineCache::ShaderSet::MultiColor, topology, smooth)) {
            return this;
        }
        if (!va->bindExpanded(this, cmd)) {
            return this;
        }
    }
    pushFrameData();
    int ct = drawCount(vac->getCount(), start, count);
    if (ct > 0) {
        vkCmdDraw(cmd, ct, 1, start, 0);
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexAccumulator* vac, const xlColor& c, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vac, c, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexAccumulator* vac, const xlColor& c, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, vac, c, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexAccumulator* vac, const xlColor& c, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vac, c, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexAccumulator* vac, const xlColor& c, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, vac, c, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexAccumulator* vac, const xlColor& c, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, vac, c, start, count);
}

xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexColorAccumulator* vac, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, vac, start, count);
}

xlGraphicsContext* xlVulkanGraphicsContext::drawLines(xlVertexIndexedColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawLineStrip(xlVertexIndexedColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangles(xlVertexIndexedColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawTriangleStrip(xlVertexIndexedColorAccumulator* vac, int start, int count) {
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, vac, start, count);
}
xlGraphicsContext* xlVulkanGraphicsContext::drawPoints(xlVertexIndexedColorAccumulator* vac, float pointSize, bool smoothPoints, int start, int count) {
    setPointSize(pointSize, smoothPoints);
    return drawPrimitive(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, vac, start, count);
}

// ─── textures ───────────────────────────────────────────────────────────────

xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlTexture* texture,
                                                        float x, float y, float x2, float y2,
                                                        float tx, float ty, float tx2, float ty2,
                                                        bool smoothScale,
                                                        int brightness, int alpha) {
    xlVulkanTexture* txt = (xlVulkanTexture*)texture;
    if (txt == nullptr || txt->image == VK_NULL_HANDLE) {
        return this;
    }
    txt->flushDirty();
    if (!setPipeline(smoothScale ? VulkanPipelineCache::ShaderSet::Texture : VulkanPipelineCache::ShaderSet::TextureNearest,
                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)) {
        return this;
    }

    float verts[18] = {
        x, y, 0, x2, y, 0, x2, y2, 0,
        x, y, 0, x, y2, 0, x2, y2, 0
    };
    float tcoords[12] = {
        tx, ty, tx2, ty, tx2, ty2,
        tx, ty, tx, ty2, tx2, ty2
    };
    VkBuffer vb, tb;
    VkDeviceSize vo, to;
    void* vp;
    void* tp;
    if (!arenaAlloc(sizeof(verts), vb, vo, vp) || !arenaAlloc(sizeof(tcoords), tb, to, tp)) {
        return this;
    }
    memcpy(vp, verts, sizeof(verts));
    memcpy(tp, tcoords, sizeof(tcoords));
    VkBuffer bufs[2] = { vb, tb };
    VkDeviceSize offsets[2] = { vo, to };
    vkCmdBindVertexBuffers(cmd, 0, 2, bufs, offsets);

    float b = brightness / 100.0f;
    float a = alpha / 255.0f;
    glm::vec4 fc = { b, b, b, a };
    if (fc != frameData.fragmentColor) {
        frameData.fragmentColor = fc;
        frameDataChanged = true;
    }
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    pushFrameData();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cache->getPipelineLayout(), 0, 1, &txt->descriptorSet, 0, nullptr);
    vkCmdDraw(cmd, 6, 1, 0, 0);
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, int brightness, uint8_t alpha, int start, int count) {
    xlVulkanTexture* txt = (xlVulkanTexture*)texture;
    if (vac == nullptr || vac->getCount() == 0 || txt == nullptr || txt->image == VK_NULL_HANDLE) {
        return this;
    }
    txt->flushDirty();
    if (!setPipeline(VulkanPipelineCache::ShaderSet::Texture, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)) {
        return this;
    }
    if (!((xlVulkanVertexTextureAccumulator*)vac)->bind(this, cmd)) {
        return this;
    }
    float b = brightness / 100.0f;
    float a = alpha / 255.0f;
    glm::vec4 fc = { b, b, b, a };
    if (fc != frameData.fragmentColor) {
        frameData.fragmentColor = fc;
        frameDataChanged = true;
    }
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    pushFrameData();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cache->getPipelineLayout(), 0, 1, &txt->descriptorSet, 0, nullptr);
    vkCmdDraw(cmd, vac->getCount(), 1, 0, 0);
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, const xlColor& c, int start, int count) {
    xlVulkanTexture* txt = (xlVulkanTexture*)texture;
    if (vac == nullptr || vac->getCount() == 0 || txt == nullptr || txt->image == VK_NULL_HANDLE) {
        return this;
    }
    txt->flushDirty();
    if (!setPipeline(VulkanPipelineCache::ShaderSet::TextureColor, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)) {
        return this;
    }
    if (!((xlVulkanVertexTextureAccumulator*)vac)->bind(this, cmd)) {
        return this;
    }
    setFragmentColor(c);
    if (frameData.renderType != VkRenderTypeNormal) {
        frameData.renderType = VkRenderTypeNormal;
        frameDataChanged = true;
    }
    pushFrameData();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cache->getPipelineLayout(), 0, 1, &txt->descriptorSet, 0, nullptr);
    vkCmdDraw(cmd, vac->getCount(), 1, 0, 0);
    return this;
}

// ─── mesh ───────────────────────────────────────────────────────────────────
// Mirrors xlMetalMesh: interleaved pos/normal/texcoord vertices in a single
// VkBuffer, uint32 indices (triangles, then appended lines and wireframe) in
// another.  Submeshes group index ranges by material + primitive type.

struct xlVulkanSubMesh {
    std::string name;
    uint32_t startIndex = 0;
    uint32_t count = 0;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    int material = 0;
};

class xlVulkanMesh : public xlMesh {
public:
    struct MeshVertex {
        float px, py, pz, nx, ny, nz, tx, ty;
    };
    struct IndexKey {
        int v, n, t;
        bool operator<(const IndexKey& o) const {
            if (v != o.v) return v < o.v;
            if (n != o.n) return n < o.n;
            return t < o.t;
        }
    };

    xlVulkanMesh(xlGraphicsContext* ctx, const std::string& file) : xlMesh(ctx, file) {}
    virtual ~xlVulkanMesh() {
        for (auto* s : subMeshes) {
            delete s;
        }
        VulkanPipelineCache::INSTANCE.deferDestroyBuffer(vbuffer.buffer, vbuffer.allocation);
        VulkanPipelineCache::INSTANCE.deferDestroyBuffer(ibuffer.buffer, ibuffer.allocation);
    }

    uint32_t getOrAddIndex(std::map<IndexKey, uint32_t>& indexMap, std::vector<MeshVertex>& input, const tinyobj::index_t& idx) {
        IndexKey key{ idx.vertex_index, idx.normal_index, idx.texcoord_index };
        auto it = indexMap.find(key);
        if (it != indexMap.end()) {
            return it->second;
        }
        uint32_t ret = (uint32_t)input.size();
        MeshVertex mv = {};
        const auto& attrib = objects.GetAttrib();
        if (idx.vertex_index >= 0 && (size_t)(idx.vertex_index * 3 + 2) < attrib.vertices.size()) {
            mv.px = attrib.vertices[idx.vertex_index * 3];
            mv.py = attrib.vertices[idx.vertex_index * 3 + 1];
            mv.pz = attrib.vertices[idx.vertex_index * 3 + 2];
        }
        if (idx.normal_index >= 0 && (size_t)(idx.normal_index * 3 + 2) < attrib.normals.size()) {
            mv.nx = attrib.normals[idx.normal_index * 3];
            mv.ny = attrib.normals[idx.normal_index * 3 + 1];
            mv.nz = attrib.normals[idx.normal_index * 3 + 2];
        }
        if (idx.texcoord_index >= 0 && (size_t)(idx.texcoord_index * 2 + 1) < attrib.texcoords.size()) {
            mv.tx = attrib.texcoords[idx.texcoord_index * 2];
            mv.ty = attrib.texcoords[idx.texcoord_index * 2 + 1];
        }
        indexMap[key] = ret;
        input.push_back(mv);
        return ret;
    }

    void LoadBuffers() {
        loaded = true;
        if (!HasGeometry()) {
            return;
        }
        std::map<IndexKey, uint32_t> indexMap;
        std::vector<MeshVertex> input;
        input.resize(1); // index 0 is the "unset" sentinel, mirroring xlMetalMesh
        std::vector<uint32_t> indexes;
        std::vector<uint32_t> lines;
        std::vector<uint32_t> wireFrame;

        xlVulkanSubMesh* lastMesh = nullptr;
        for (auto& s : objects.GetShapes()) {
            if (!s.mesh.indices.empty()) {
                xlVulkanSubMesh* sm;
                if (lastMesh && s.mesh.material_ids[0] == lastMesh->material) {
                    sm = lastMesh;
                } else {
                    sm = new xlVulkanSubMesh();
                    sm->name = s.name;
                    sm->startIndex = (uint32_t)indexes.size();
                    sm->material = s.mesh.material_ids[0];
                    subMeshes.push_back(sm);
                    lastMesh = sm;
                }
                for (size_t idx = 0; idx < s.mesh.material_ids.size(); idx++) {
                    if (sm->material != s.mesh.material_ids[idx]) {
                        sm->count = (uint32_t)indexes.size() - sm->startIndex;
                        sm = new xlVulkanSubMesh();
                        sm->name = s.name;
                        sm->startIndex = (uint32_t)indexes.size();
                        sm->material = s.mesh.material_ids[idx];
                        lastMesh = sm;
                        subMeshes.push_back(sm);
                    }
                    uint32_t v1 = getOrAddIndex(indexMap, input, s.mesh.indices[idx * 3]);
                    uint32_t v2 = getOrAddIndex(indexMap, input, s.mesh.indices[idx * 3 + 1]);
                    uint32_t v3 = getOrAddIndex(indexMap, input, s.mesh.indices[idx * 3 + 2]);
                    indexes.push_back(v1);
                    indexes.push_back(v2);
                    indexes.push_back(v3);
                    wireFrame.push_back(v1);
                    wireFrame.push_back(v2);
                    wireFrame.push_back(v2);
                    wireFrame.push_back(v3);
                    wireFrame.push_back(v3);
                    wireFrame.push_back(v1);
                }
                sm->count = (uint32_t)indexes.size() - sm->startIndex;
            }
            if (!s.lines.indices.empty()) {
                for (auto& idx : s.lines.indices) {
                    lines.push_back(getOrAddIndex(indexMap, input, idx));
                }
            }
            if (!s.points.indices.empty()) {
                xlVulkanSubMesh* sm = new xlVulkanSubMesh();
                sm->name = s.name;
                sm->topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                sm->startIndex = (uint32_t)indexes.size();
                for (auto& idx : s.points.indices) {
                    indexes.push_back(getOrAddIndex(indexMap, input, idx));
                }
                sm->count = (uint32_t)indexes.size() - sm->startIndex;
                subMeshes.push_back(sm);
            }
        }
        if (!lines.empty()) {
            linesStart = (uint32_t)indexes.size();
            linesCount = (uint32_t)lines.size();
            indexes.insert(indexes.end(), lines.begin(), lines.end());
        }
        if (!wireFrame.empty()) {
            wireframeStart = (uint32_t)indexes.size();
            wireframeCount = (uint32_t)wireFrame.size();
            indexes.insert(indexes.end(), wireFrame.begin(), wireFrame.end());
        }

        VkDeviceSize vsz = sizeof(MeshVertex) * input.size();
        void* vp = nullptr;
        if (createHostBuffer(vsz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vbuffer.buffer, vbuffer.allocation, vp)) {
            memcpy(vp, input.data(), vsz);
        }
        if (!indexes.empty()) {
            VkDeviceSize isz = sizeof(uint32_t) * indexes.size();
            void* ip = nullptr;
            if (createHostBuffer(isz, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ibuffer.buffer, ibuffer.allocation, ip)) {
                memcpy(ip, indexes.data(), isz);
            }
        }
    }

    struct RawBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
    };
    bool loaded = false;
    RawBuffer vbuffer;
    RawBuffer ibuffer;
    std::vector<xlVulkanSubMesh*> subMeshes;
    uint32_t linesStart = 0, linesCount = 0;
    uint32_t wireframeStart = 0, wireframeCount = 0;
};

std::unique_ptr<xlMesh> xlVulkanGraphicsContext::loadMeshFromObjFile(const std::string& file) {
    return std::make_unique<xlVulkanMesh>(this, file);
}

bool xlVulkanGraphicsContext::setMeshPipeline(VulkanPipelineCache::ShaderSet set, VkPrimitiveTopology topology, bool blend) {
    VkPipeline p = cache->getPipeline(set, topology, blend, false, true);
    if (p == VK_NULL_HANDLE) {
        return false;
    }
    if (p != lastPipeline) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
        lastPipeline = p;
    }
    return true;
}

void xlVulkanGraphicsContext::pushMeshFrameData(const xlColor& c, int brightness, bool useViewMatrix) {
    VulkanMeshFrameData md;
    md.MVP = frameData.MVP;
    md.model = modelMatrix;
    md.view = viewMatrix;
    md.fragmentColor = { c.red / 255.0f, c.green / 255.0f, c.blue / 255.0f, c.alpha / 255.0f };
    md.brightness = brightness / 100.0f;
    md.useViewMatrix = useViewMatrix ? 1 : 0;
    vkCmdPushConstants(cmd, cache->getMeshPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(VulkanMeshFrameData), &md);
}

xlGraphicsContext* xlVulkanGraphicsContext::drawMeshSolids(xlMesh* mesh, int brightness, bool useViewMatrix) {
    xlVulkanMesh* m = (xlVulkanMesh*)mesh;
    if (!m->loaded) {
        m->LoadBuffers();
    }
    if (m->vbuffer.buffer == VK_NULL_HANDLE || m->ibuffer.buffer == VK_NULL_HANDLE || cache->getMeshPipelineLayout() == VK_NULL_HANDLE) {
        return this;
    }
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m->vbuffer.buffer, &offset);
    vkCmdBindIndexBuffer(cmd, m->ibuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    lastPipeline = VK_NULL_HANDLE;

    for (auto* sm : m->subMeshes) {
        int mid = sm->material;
        if (mid >= 0 && m->GetMaterial(mid).color.alpha != 255) {
            continue; // opaque pass only
        }
        bool textured = mid >= 0 && m->GetMaterial(mid).texture && !m->GetMaterial(mid).forceColor;
        if (textured) {
            if (!setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshTexture, sm->topology, blending)) {
                continue;
            }
            xlVulkanTexture* t = (xlVulkanTexture*)m->GetMaterial(mid).texture;
            t->flushDirty();
            pushMeshFrameData(xlWHITE, brightness, useViewMatrix);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cache->getMeshPipelineLayout(), 0, 1, &t->descriptorSet, 0, nullptr);
        } else {
            if (!setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshSolid, sm->topology, blending)) {
                continue;
            }
            xlColor color = mid < 0 ? xlWHITE : m->GetMaterial(mid).color;
            if (sm->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST) {
                color = xlBLACK;
            }
            pushMeshFrameData(color, brightness, useViewMatrix);
        }
        vkCmdDrawIndexed(cmd, sm->count, 1, sm->startIndex, 0, 0);
    }
    if (m->linesCount) {
        if (setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshSolid, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, blending)) {
            pushMeshFrameData(xlBLACK, brightness, useViewMatrix);
            vkCmdDrawIndexed(cmd, m->linesCount, 1, m->linesStart, 0, 0);
        }
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawMeshTransparents(xlMesh* mesh, int brightness) {
    xlVulkanMesh* m = (xlVulkanMesh*)mesh;
    if (!m->loaded) {
        m->LoadBuffers();
    }
    if (m->vbuffer.buffer == VK_NULL_HANDLE || m->ibuffer.buffer == VK_NULL_HANDLE || cache->getMeshPipelineLayout() == VK_NULL_HANDLE) {
        return this;
    }
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m->vbuffer.buffer, &offset);
    vkCmdBindIndexBuffer(cmd, m->ibuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    lastPipeline = VK_NULL_HANDLE;

    for (auto* sm : m->subMeshes) {
        int mid = sm->material;
        if (mid < 0 || m->GetMaterial(mid).color.alpha == 255) {
            continue; // transparent pass only
        }
        bool textured = m->GetMaterial(mid).texture != nullptr;
        if (textured) {
            if (!setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshTexture, sm->topology, blending)) {
                continue;
            }
            xlVulkanTexture* t = (xlVulkanTexture*)m->GetMaterial(mid).texture;
            t->flushDirty();
            pushMeshFrameData(xlWHITE, brightness, false);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cache->getMeshPipelineLayout(), 0, 1, &t->descriptorSet, 0, nullptr);
        } else {
            if (!setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshSolid, sm->topology, blending)) {
                continue;
            }
            xlColor color = m->GetMaterial(mid).color;
            if (sm->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST) {
                color = xlBLACK;
            }
            pushMeshFrameData(color, brightness, false);
        }
        vkCmdDrawIndexed(cmd, sm->count, 1, sm->startIndex, 0, 0);
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::drawMeshWireframe(xlMesh* mesh, int brightness) {
    xlVulkanMesh* m = (xlVulkanMesh*)mesh;
    if (!m->loaded) {
        m->LoadBuffers();
    }
    if (m->vbuffer.buffer == VK_NULL_HANDLE || m->ibuffer.buffer == VK_NULL_HANDLE || m->wireframeCount == 0 ||
        cache->getMeshPipelineLayout() == VK_NULL_HANDLE) {
        return this;
    }
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m->vbuffer.buffer, &offset);
    vkCmdBindIndexBuffer(cmd, m->ibuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    lastPipeline = VK_NULL_HANDLE;
    if (setMeshPipeline(VulkanPipelineCache::ShaderSet::MeshWireframe, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, blending)) {
        pushMeshFrameData(xlColor(0, 255, 0), brightness, false);
        vkCmdDrawIndexed(cmd, m->wireframeCount, 1, m->wireframeStart, 0, 0);
    }
    return this;
}

xlGraphicsContext* xlVulkanGraphicsContext::pushDebugContext(const std::string& label) {
    if (vkCmdBeginDebugUtilsLabelEXT != nullptr) {
        VkDebugUtilsLabelEXT l = {};
        l.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        l.pLabelName = label.c_str();
        vkCmdBeginDebugUtilsLabelEXT(cmd, &l);
    }
    return this;
}
xlGraphicsContext* xlVulkanGraphicsContext::popDebugContext() {
    if (vkCmdEndDebugUtilsLabelEXT != nullptr) {
        vkCmdEndDebugUtilsLabelEXT(cmd);
    }
    return this;
}

#endif
