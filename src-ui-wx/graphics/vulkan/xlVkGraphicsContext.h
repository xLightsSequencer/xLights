/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

#include <stack>
#include <string>
#include <map>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/xlGraphicsContext.h"
#include "xlVkCanvas.h"

// Push constants block shared by all Vulkan shaders (96 bytes ≤ 128-byte minimum).
struct VkPushConstants {
    glm::mat4 MVP;           // 64 bytes @ offset 0
    glm::vec4 inColor;       // 16 bytes @ offset 64
    int       renderType;    //  4 bytes @ offset 80
    float     smoothMin;     //  4 bytes @ offset 84
    float     smoothMax;     //  4 bytes @ offset 88
    float     brightness;    //  4 bytes @ offset 92
};                           // total = 96 bytes

// Frame data mirrors OGLFrameData / MTLFrameData for matrix bookkeeping.
struct VkFrameData {
    glm::mat4 MVP;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 perspectiveMatrix;
};

// Identifies a compiled pipeline variant.
enum class VkPipelineType : uint8_t {
    SingleColor     = 0,  // solid color, vertex positions only
    PerVertexColor  = 1,  // per-vertex RGBA colors
    Texture         = 2,  // texture + brightness/alpha
    TextureColor    = 3,  // texture + color tint (alpha-only from tex)
    MeshSolid       = 4,  // mesh geometry + solid color + basic lighting
    MeshTexture     = 5,  // mesh geometry + texture + basic lighting
};

enum class VkTopology : uint8_t {
    LineList      = 0,
    LineStrip     = 1,
    TriangleList  = 2,
    TriangleStrip = 3,
    PointList     = 4,
};

struct VkPipelineKey {
    VkPipelineType type;
    VkTopology     topology;
    bool           blend;

    bool operator<(const VkPipelineKey& o) const {
        if (type     != o.type)     return type     < o.type;
        if (topology != o.topology) return topology < o.topology;
        return blend < o.blend;
    }
};


class xlVkGraphicsContext : public xlGraphicsContext {
public:
    xlVkGraphicsContext(xlVkCanvas* canvas);
    virtual ~xlVkGraphicsContext();

    // Called by xlVkCanvas::FinishDrawing before deleting the context.
    void EndFrame();

    //-----------------------------------------------------------------------
    // xlGraphicsContext overrides
    //-----------------------------------------------------------------------
    virtual xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    virtual xlVertexAccumulator*             createVertexAccumulator()              override;
    virtual xlVertexColorAccumulator*        createVertexColorAccumulator()         override;
    virtual xlVertexTextureAccumulator*      createVertexTextureAccumulator()       override;
    virtual xlVertexIndexedColorAccumulator* createVertexIndexedColorAccumulator()  override;
    virtual xlTexture* createTextureMipMaps(const std::vector<xlImage>& images, const std::string& name)          override;
    virtual xlTexture* createTexture(const xlImage& image, const std::string& name, bool finalize = false)        override;
    virtual xlTexture* createTexture(int w, int h, bool bgr, bool alpha)            override;
    virtual xlTexture* createTextureForFont(const xlFontInfo& font)                 override;
    virtual xlGraphicsProgram* createGraphicsProgram()                              override;
    virtual std::unique_ptr<xlMesh> loadMeshFromObjFile(const std::string& file)   override;

    virtual xlGraphicsContext* enableBlending(bool e = true) override;

    // Single-color draw
    virtual xlGraphicsContext* drawLines        (xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip    (xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles    (xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator* vac, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints       (xlVertexAccumulator* vac, const xlColor& c, float pointSize, bool smooth, int start = 0, int count = -1) override;

    // Per-vertex color draw
    virtual xlGraphicsContext* drawLines        (xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip    (xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles    (xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints       (xlVertexColorAccumulator* vac, float pointSize, bool smooth, int start = 0, int count = -1) override;

    // Indexed-color draw
    virtual xlGraphicsContext* drawLines        (xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip    (xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles    (xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator* vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints       (xlVertexIndexedColorAccumulator* vac, float pointSize, bool smooth, int start = 0, int count = -1) override;

    // Texture draw
    virtual xlGraphicsContext* drawTexture(xlTexture* texture,
                                           float x, float y, float x2, float y2,
                                           float tx = 0.0f, float ty = 0.0f, float tx2 = 1.0f, float ty2 = 1.0f,
                                           bool smoothScale = true,
                                           int brightness = 100, int alpha = 255) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, const xlColor& c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture, int brightness, uint8_t alpha, int start, int count) override;

    // Mesh draw
    virtual xlGraphicsContext* drawMeshSolids      (xlMesh* mesh, int brightness, bool useViewMatrix) override;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh* mesh, int brightness)                     override;
    virtual xlGraphicsContext* drawMeshWireframe   (xlMesh* mesh, int brightness)                     override;

    // Matrix operations
    virtual xlGraphicsContext* PushMatrix() override;
    virtual xlGraphicsContext* PopMatrix()  override;
    virtual xlGraphicsContext* Translate(float x, float y, float z)                    override;
    virtual xlGraphicsContext* Rotate   (float angle, float x, float y, float z)       override;
    virtual xlGraphicsContext* Scale    (float w, float h, float z)                    override;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4& m)                         override;
    virtual xlGraphicsContext* SetCamera    (const glm::mat4& m)                       override;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4& m)                      override;
    virtual xlGraphicsContext* ScaleViewMatrix    (float w, float h, float z)          override;
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z)          override;

    virtual xlGraphicsContext* pushDebugContext(const std::string& label) override;
    virtual xlGraphicsContext* popDebugContext()                           override;

    // --- Static pipeline management (shared across all contexts/canvases) ---
    static bool InitializePipelines(VkRenderPass renderPass);
    static void DestroyPipelines();

private:
    xlVkCanvas*    mCanvas;
    VkCommandBuffer mCmd;        // borrowed from canvas, valid for one frame

    VkFrameData    mFrameData;
    VkPushConstants mPC;
    std::stack<std::tuple<glm::mat4, glm::mat4, glm::mat4>> mMatrixStack;

    bool mBlending = false;
    float mPointSize = 1.0f;

    // Helpers
    xlGraphicsContext* drawPrimitive(VkPipelineType type, VkTopology topo,
                                     xlVertexAccumulator* vac,
                                     const xlColor& c, int start, int count);
    xlGraphicsContext* drawPrimitive(VkPipelineType type, VkTopology topo,
                                     xlVertexColorAccumulator* vac,
                                     int start, int count);
    void bindPipeline(const VkPipelineKey& key);
    void pushConstants();
    void setPointSmoothParams(float ps);

    xlTexture* createTextureInternal(const xlImage& image, bool mipMaps, const std::string& nm);
    void drawMesh(xlMesh* mesh, int brightness, bool useViewMatrix, bool transparents);

    static std::map<VkPipelineKey, VkPipeline> s_pipelines;
    static VkRenderPass s_renderPassUsed;

    VkPipelineKey mLastKey;
    bool mLastKeyValid = false;
};
