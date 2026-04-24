/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// xlVkGraphicsContext implementation.
// Depends on: Vulkan SDK (vulkan.h), shaderc (libshaderc / shaderc_combined),
//             glm, tiny_obj_loader (via xlMesh).

// Undo the opaque forward-declarations from the header before pulling in real Vulkan types.
#undef VkInstance
#undef VkPhysicalDevice
#undef VkDevice
#undef VkQueue
#undef VkSurfaceKHR
#undef VkSwapchainKHR
#undef VkRenderPass
#undef VkFramebuffer
#undef VkCommandPool
#undef VkCommandBuffer
#undef VkSemaphore
#undef VkFence
#undef VkImage
#undef VkImageView
#undef VkDeviceMemory
#undef VkBuffer
#undef VkDescriptorPool
#undef VkDescriptorSetLayout
#undef VkPipelineLayout
#undef VkPipeline

#include <vulkan/vulkan.h>

#include <shaderc/shaderc.hpp>

#include <log.h>

#include "xlVkGraphicsContext.h"
#include "xlVkCanvas.h"

#include "graphics/xlMesh.h"
#include "graphics/xlFontInfo.h"

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------
std::map<VkPipelineKey, VkPipeline> xlVkGraphicsContext::s_pipelines;
VkRenderPass xlVkGraphicsContext::s_renderPassUsed = nullptr;

// ---------------------------------------------------------------------------
// GLSL shader source strings (Vulkan GLSL 4.50)
// All shaders share the same push_constant block layout.
// ---------------------------------------------------------------------------
static const char* kSingleColorVert = R"glsl(
#version 450
layout(location = 0) in vec3 inPosition;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(location = 0) out vec4 fragColor;
void main() {
    gl_Position = pc.MVP * vec4(inPosition, 1.0);
    gl_PointSize = 1.0;
    fragColor = pc.inColor;
}
)glsl";

static const char* kSingleColorFrag = R"glsl(
#version 450
layout(location = 0) in vec4 fragColor;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(location = 0) out vec4 outColor;
void main() {
    if (pc.renderType == 0) {
        outColor = fragColor;
    } else {
        float dist  = distance(gl_PointCoord, vec2(0.5));
        float alpha = 1.0 - smoothstep(pc.smoothMin, pc.smoothMax, dist);
        if (alpha == 0.0) discard;
        outColor = vec4(fragColor.rgb, alpha * fragColor.a);
    }
}
)glsl";

static const char* kPerVertexColorVert = R"glsl(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;     // overrides vertex color when renderType == -1 or -2
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(location = 0) out vec4 fragColor;
void main() {
    gl_Position  = pc.MVP * vec4(inPosition, 1.0);
    gl_PointSize = 1.0;
    if (pc.renderType == -2 || pc.renderType == -1) {
        fragColor = pc.inColor;
    } else {
        fragColor = inColor;
    }
}
)glsl";

// Fragment shader is identical to single-color for per-vertex pipelines.

static const char* kTextureVert = R"glsl(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 UV;
void main() {
    gl_Position = pc.MVP * vec4(inPosition, 1.0);
    fragColor   = pc.inColor;
    UV          = inUV;
}
)glsl";

static const char* kTextureFrag = R"glsl(
#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 UV;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(set = 0, binding = 0) uniform sampler2D tex;
layout(location = 0) out vec4 outColor;
void main() {
    vec4 c = texture(tex, UV);
    if (pc.renderType == 0) {
        // brightness / alpha mode: colour multiplies texture sample
        outColor = vec4(c.r * fragColor.r, c.g * fragColor.g, c.b * fragColor.b, c.a * fragColor.a);
    } else {
        // colour tint: use colour RGB, texture alpha
        outColor = vec4(fragColor.rgb, c.a * fragColor.a);
    }
}
)glsl";

static const char* kMeshSolidVert = R"glsl(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(set = 0, binding = 0) uniform MeshUBO { mat4 NM; } mesh;
layout(location = 0) out vec4 fragColor;
void main() {
    gl_Position = pc.MVP * vec4(inPosition, 1.0);
    vec3 n      = normalize((mesh.NM * vec4(inNormal, 0.0)).xyz);
    vec3 l      = normalize(vec3(0.1, 0.1, 1.0));
    float ct    = abs(clamp(dot(n, l), -1.0, 1.0));
    fragColor   = vec4(pc.inColor.rgb * 0.75 * ct + pc.inColor.rgb * 0.25, pc.inColor.a);
}
)glsl";

static const char* kMeshSolidFrag = R"glsl(
#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;
void main() { outColor = fragColor; }
)glsl";

static const char* kMeshTextureVert = R"glsl(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(set = 0, binding = 0) uniform MeshUBO { mat4 NM; } mesh;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 UV;
layout(location = 2) out float cosTheta;
void main() {
    gl_Position = pc.MVP * vec4(inPosition, 1.0);
    UV = inUV;
    if (pc.inColor.a == 1.0) {
        vec3 n  = normalize((mesh.NM * vec4(inNormal, 0.0)).xyz);
        vec3 l  = normalize(vec3(0.1, 0.1, 1.0));
        cosTheta = abs(clamp(dot(n, l), -1.0, 1.0));
        vec4 cv = vec4(cosTheta, cosTheta, cosTheta, 1.0);
        fragColor = (pc.inColor * cv) * 0.75 + pc.inColor * 0.75;
    } else {
        cosTheta  = 1.0;
        fragColor = pc.inColor;
    }
}
)glsl";

static const char* kMeshTextureFrag = R"glsl(
#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 UV;
layout(location = 2) in float cosTheta;
layout(push_constant) uniform PC {
    mat4  MVP;
    vec4  inColor;
    int   renderType;
    float smoothMin;
    float smoothMax;
    float brightness;
} pc;
layout(set = 0, binding = 0) uniform MeshUBO { mat4 NM; } mesh;
layout(set = 0, binding = 1) uniform sampler2D tex;
layout(location = 0) out vec4 outColor;
void main() {
    vec4 c = texture(tex, UV);
    if (cosTheta != 1.0) {
        vec3 c3 = cosTheta * c.rgb * 0.75 + c.rgb * 0.25;
        c = vec4(c3, c.a);
    }
    outColor = vec4(c.r * pc.brightness, c.g * pc.brightness, c.b * pc.brightness, c.a);
}
)glsl";

// ---------------------------------------------------------------------------
// SPIR-V compilation via shaderc
// ---------------------------------------------------------------------------
static std::vector<uint32_t> CompileGLSL(const char* source, shaderc_shader_kind kind,
                                          const char* debugName) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions opts;
    opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
    opts.SetOptimizationLevel(shaderc_optimization_level_performance);
    opts.SetSourceLanguage(shaderc_source_language_glsl);

    auto result = compiler.CompileGlslToSpv(source, kind, debugName, opts);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        spdlog::error("xlVkGraphicsContext: shader compilation error in {}: {}",
                      debugName, result.GetErrorMessage());
        return {};
    }
    return {result.cbegin(), result.cend()};
}

static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& spv) {
    if (spv.empty()) return VK_NULL_HANDLE;
    VkShaderModuleCreateInfo smci{};
    smci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = spv.size() * sizeof(uint32_t);
    smci.pCode    = spv.data();
    VkShaderModule mod;
    vkCreateShaderModule(device, &smci, nullptr, &mod);
    return mod;
}

// ---------------------------------------------------------------------------
// Pipeline creation helpers
// ---------------------------------------------------------------------------
static VkPrimitiveTopology ToVkTopology(VkTopology t) {
    switch (t) {
    case VkTopology::LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case VkTopology::LineStrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case VkTopology::TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case VkTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case VkTopology::PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

struct PipelineSpec {
    const char* vertSrc;
    const char* fragSrc;
    const char* debugVert;
    const char* debugFrag;
    VkPipelineLayout layout;
    // Vertex bindings/attributes
    std::vector<VkVertexInputBindingDescription>   bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
};

static VkPipeline BuildPipeline(VkDevice device, VkRenderPass renderPass,
                                 const PipelineSpec& spec,
                                 VkPrimitiveTopology topology, bool blend) {
    auto vertSpv = CompileGLSL(spec.vertSrc, shaderc_vertex_shader,   spec.debugVert);
    auto fragSpv = CompileGLSL(spec.fragSrc, shaderc_fragment_shader, spec.debugFrag);
    VkShaderModule vertMod = CreateShaderModule(device, vertSpv);
    VkShaderModule fragMod = CreateShaderModule(device, fragSpv);
    if (!vertMod || !fragMod) {
        if (vertMod) vkDestroyShaderModule(device, vertMod, nullptr);
        if (fragMod) vkDestroyShaderModule(device, fragMod, nullptr);
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertMod;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragMod;
    stages[1].pName  = "main";

    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount   = (uint32_t)spec.bindings.size();
    vi.pVertexBindingDescriptions      = spec.bindings.empty() ? nullptr : spec.bindings.data();
    vi.vertexAttributeDescriptionCount = (uint32_t)spec.attributes.size();
    vi.pVertexAttributeDescriptions    = spec.attributes.empty() ? nullptr : spec.attributes.data();

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology               = topology;
    ia.primitiveRestartEnable = VK_FALSE;

    // Dynamic viewport and scissor
    VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynState{};
    dynState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynState.dynamicStateCount = 2;
    dynState.pDynamicStates    = dynStates;

    VkPipelineViewportStateCreateInfo vps{};
    vps.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vps.viewportCount = 1;
    vps.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rast{};
    rast.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rast.polygonMode = VK_POLYGON_MODE_FILL;
    rast.cullMode    = VK_CULL_MODE_NONE;
    rast.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rast.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable  = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState cba{};
    cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (blend) {
        cba.blendEnable         = VK_TRUE;
        cba.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        cba.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        cba.colorBlendOp        = VK_BLEND_OP_ADD;
        cba.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        cba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        cba.alphaBlendOp        = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments    = &cba;

    VkGraphicsPipelineCreateInfo gpci{};
    gpci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount          = 2;
    gpci.pStages             = stages;
    gpci.pVertexInputState   = &vi;
    gpci.pInputAssemblyState = &ia;
    gpci.pViewportState      = &vps;
    gpci.pRasterizationState = &rast;
    gpci.pMultisampleState   = &ms;
    gpci.pDepthStencilState  = &ds;
    gpci.pColorBlendState    = &cb;
    gpci.pDynamicState       = &dynState;
    gpci.layout              = spec.layout;
    gpci.renderPass          = renderPass;
    gpci.subpass             = 0;

    VkPipeline pipeline;
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpci, nullptr, &pipeline);

    vkDestroyShaderModule(device, vertMod, nullptr);
    vkDestroyShaderModule(device, fragMod, nullptr);
    return pipeline;
}

// ---------------------------------------------------------------------------
// Pipeline initialization
// ---------------------------------------------------------------------------
bool xlVkGraphicsContext::InitializePipelines(VkRenderPass renderPass) {
    if (!s_pipelines.empty() && s_renderPassUsed == renderPass) return true;
    // If render pass changed (e.g. swapchain recreation), destroy old pipelines first.
    DestroyPipelines();
    s_renderPassUsed = renderPass;

    VkDevice device = xlVkCanvas::GetShared().device;
    auto& sh = xlVkCanvas::GetShared();

    // ------ Vertex input descriptions ------
    // Binding 0: position (vec3, binding=0)
    VkVertexInputBindingDescription posBinding{0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription posAttr{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

    // Binding 1: color (vec4 UNORM from uint32, binding=1)
    VkVertexInputBindingDescription colBinding{1, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription colAttr{1, 1, VK_FORMAT_R8G8B8A8_UNORM, 0};

    // Binding 1 (texture): UV (vec2, binding=1)
    VkVertexInputBindingDescription uvBinding{1, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription uvAttr{1, 1, VK_FORMAT_R32G32_SFLOAT, 0};

    // Binding 1 (mesh): normal (vec3, binding=1)
    VkVertexInputBindingDescription normalBinding{1, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription normalAttr{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0};

    // Binding 2 (mesh texture): UV (vec2, binding=2)
    VkVertexInputBindingDescription meshUvBinding{2, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription meshUvAttr{2, 2, VK_FORMAT_R32G32_SFLOAT, 0};

    // Topologies we support for single-color and per-vertex-color pipelines
    const VkTopology colorTopos[] = {
        VkTopology::LineList, VkTopology::LineStrip,
        VkTopology::TriangleList, VkTopology::TriangleStrip,
        VkTopology::PointList
    };

    for (bool blend : {false, true}) {
        // --- SingleColor ---
        PipelineSpec scSpec;
        scSpec.vertSrc   = kSingleColorVert;
        scSpec.fragSrc   = kSingleColorFrag;
        scSpec.debugVert = "singleColor.vert";
        scSpec.debugFrag = "singleColor.frag";
        scSpec.layout    = sh.basicLayout;
        scSpec.bindings  = {posBinding};
        scSpec.attributes= {posAttr};

        for (auto topo : colorTopos) {
            VkPipelineKey key{VkPipelineType::SingleColor, topo, blend};
            VkPipeline p = BuildPipeline(device, renderPass, scSpec, ToVkTopology(topo), blend);
            if (p) s_pipelines[key] = p;
        }

        // --- PerVertexColor ---
        PipelineSpec pvcSpec;
        pvcSpec.vertSrc   = kPerVertexColorVert;
        pvcSpec.fragSrc   = kSingleColorFrag;   // same fragment shader
        pvcSpec.debugVert = "perVertexColor.vert";
        pvcSpec.debugFrag = "perVertexColor.frag";
        pvcSpec.layout    = sh.basicLayout;
        pvcSpec.bindings  = {posBinding, colBinding};
        pvcSpec.attributes= {posAttr, colAttr};

        for (auto topo : colorTopos) {
            VkPipelineKey key{VkPipelineType::PerVertexColor, topo, blend};
            VkPipeline p = BuildPipeline(device, renderPass, pvcSpec, ToVkTopology(topo), blend);
            if (p) s_pipelines[key] = p;
        }

        // --- Texture (brightness/alpha mode) ---
        PipelineSpec txSpec;
        txSpec.vertSrc   = kTextureVert;
        txSpec.fragSrc   = kTextureFrag;
        txSpec.debugVert = "texture.vert";
        txSpec.debugFrag = "texture.frag";
        txSpec.layout    = sh.textureLayout;
        txSpec.bindings  = {posBinding, uvBinding};
        txSpec.attributes= {posAttr, uvAttr};
        {
            VkPipelineKey key{VkPipelineType::Texture, VkTopology::TriangleList, blend};
            VkPipeline p = BuildPipeline(device, renderPass, txSpec, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, blend);
            if (p) s_pipelines[key] = p;
        }

        // --- TextureColor (alpha-only tint mode, same shaders different renderType via push constants) ---
        {
            VkPipelineKey key{VkPipelineType::TextureColor, VkTopology::TriangleList, blend};
            // Same pipeline as Texture; renderType is toggled in push constants.
            VkPipeline p = BuildPipeline(device, renderPass, txSpec, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, blend);
            if (p) s_pipelines[key] = p;
        }

        // --- MeshSolid ---
        PipelineSpec msSpec;
        msSpec.vertSrc   = kMeshSolidVert;
        msSpec.fragSrc   = kMeshSolidFrag;
        msSpec.debugVert = "meshSolid.vert";
        msSpec.debugFrag = "meshSolid.frag";
        msSpec.layout    = sh.meshLayout;
        msSpec.bindings  = {posBinding, normalBinding};
        msSpec.attributes= {posAttr, normalAttr};
        {
            VkPipelineKey key{VkPipelineType::MeshSolid, VkTopology::TriangleList, blend};
            VkPipeline p = BuildPipeline(device, renderPass, msSpec, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, blend);
            if (p) s_pipelines[key] = p;
        }
        // Also need lines for wireframe mesh
        {
            VkPipelineKey key{VkPipelineType::MeshSolid, VkTopology::LineList, blend};
            VkPipeline p = BuildPipeline(device, renderPass, msSpec, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, blend);
            if (p) s_pipelines[key] = p;
        }

        // --- MeshTexture ---
        PipelineSpec mtSpec;
        mtSpec.vertSrc   = kMeshTextureVert;
        mtSpec.fragSrc   = kMeshTextureFrag;
        mtSpec.debugVert = "meshTexture.vert";
        mtSpec.debugFrag = "meshTexture.frag";
        mtSpec.layout    = sh.meshLayout;
        mtSpec.bindings  = {posBinding, normalBinding, meshUvBinding};
        mtSpec.attributes= {posAttr, normalAttr, meshUvAttr};
        {
            VkPipelineKey key{VkPipelineType::MeshTexture, VkTopology::TriangleList, blend};
            VkPipeline p = BuildPipeline(device, renderPass, mtSpec, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, blend);
            if (p) s_pipelines[key] = p;
        }
    }

    spdlog::debug("xlVkGraphicsContext: compiled {} pipeline variants", s_pipelines.size());
    return !s_pipelines.empty();
}

void xlVkGraphicsContext::DestroyPipelines() {
    VkDevice device = xlVkCanvas::GetShared().device;
    if (!device) { s_pipelines.clear(); return; }
    for (auto& [k, p] : s_pipelines) {
        if (p) vkDestroyPipeline(device, p, nullptr);
    }
    s_pipelines.clear();
    s_renderPassUsed = nullptr;
}

// ---------------------------------------------------------------------------
// Vulkan buffer/image helpers (used by accumulator / texture implementations)
// ---------------------------------------------------------------------------
static bool VkCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags props,
                            VkBuffer& buf, VkDeviceMemory& mem) {
    auto& sh = xlVkCanvas::GetShared();
    VkBufferCreateInfo bci{};
    bci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size        = size;
    bci.usage       = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(sh.device, &bci, nullptr, &buf) != VK_SUCCESS) return false;

    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(sh.device, buf, &mr);

    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(sh.physicalDevice, &mp);
    uint32_t mti = UINT32_MAX;
    for (uint32_t i = 0; i < mp.memoryTypeCount; ++i) {
        if ((mr.memoryTypeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props) {
            mti = i; break;
        }
    }
    if (mti == UINT32_MAX) { vkDestroyBuffer(sh.device, buf, nullptr); return false; }

    VkMemoryAllocateInfo mai{};
    mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize  = mr.size;
    mai.memoryTypeIndex = mti;
    if (vkAllocateMemory(sh.device, &mai, nullptr, &mem) != VK_SUCCESS) {
        vkDestroyBuffer(sh.device, buf, nullptr);
        return false;
    }
    vkBindBufferMemory(sh.device, buf, mem, 0);
    return true;
}

static void VkDestroyBuffer(VkBuffer& buf, VkDeviceMemory& mem) {
    auto& sh = xlVkCanvas::GetShared();
    if (!sh.device) return;
    if (buf) { vkDestroyBuffer(sh.device, buf, nullptr); buf = nullptr; }
    if (mem) { vkFreeMemory   (sh.device, mem, nullptr); mem = nullptr; }
}

// Upload data into a host-visible buffer (creates / resizes as needed).
static void VkUploadToHostBuffer(VkBuffer& buf, VkDeviceMemory& mem,
                                  VkDeviceSize& capacity,
                                  const void* data, VkDeviceSize size,
                                  VkBufferUsageFlags usage) {
    if (size == 0) return;
    if (size > capacity) {
        VkDestroyBuffer(buf, mem);
        VkCreateBuffer(size, usage,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       buf, mem);
        capacity = size;
    }
    auto& sh = xlVkCanvas::GetShared();
    void* mapped;
    vkMapMemory(sh.device, mem, 0, size, 0, &mapped);
    memcpy(mapped, data, (size_t)size);
    vkUnmapMemory(sh.device, mem);
}

// Upload via staging buffer to device-local memory.
static bool VkUploadToDeviceBuffer(VkBuffer dst, const void* data, VkDeviceSize size) {
    auto& sh = xlVkCanvas::GetShared();
    VkBuffer stageBuf; VkDeviceMemory stageMem;
    if (!VkCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stageBuf, stageMem)) return false;
    void* mapped;
    vkMapMemory(sh.device, stageMem, 0, size, 0, &mapped);
    memcpy(mapped, data, (size_t)size);
    vkUnmapMemory(sh.device, stageMem);

    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo ai{};
    ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool        = sh.commandPool;
    ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = 1;
    vkAllocateCommandBuffers(sh.device, &ai, &cmd);

    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);
    VkBufferCopy copy{0, 0, size};
    vkCmdCopyBuffer(cmd, stageBuf, dst, 1, &copy);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo si{};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd;
    vkQueueSubmit(sh.graphicsQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(sh.graphicsQueue);
    vkFreeCommandBuffers(sh.device, sh.commandPool, 1, &cmd);

    vkDestroyBuffer(sh.device, stageBuf, nullptr);
    vkFreeMemory   (sh.device, stageMem, nullptr);
    return true;
}

// ---------------------------------------------------------------------------
// Vertex accumulators
// ---------------------------------------------------------------------------
class xlVkVertexAccumulator : public xlVertexAccumulator {
public:
    std::vector<float> vertices;   // x,y,z triples
    uint32_t           count = 0;
    bool               finalized  = false;
    bool               mayChange  = false;
    bool               changed    = false;

    VkBuffer       vbuf     = VK_NULL_HANDLE;
    VkDeviceMemory vmem     = VK_NULL_HANDLE;
    VkDeviceSize   capacity = 0;

    virtual ~xlVkVertexAccumulator() { VkDestroyBuffer(vbuf, vmem); }

    virtual void Reset() override {
        if (!finalized) { count = 0; vertices.clear(); }
    }
    virtual void PreAlloc(unsigned int n) override { vertices.reserve(n * 3); }
    virtual void AddVertex(float x, float y, float z) override {
        if (finalized) return;
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        ++count; changed = true;
    }
    virtual uint32_t getCount() override { return count; }
    virtual void Finalize(bool mc) override { finalized = true; mayChange = mc; }
    virtual void SetVertex(uint32_t v, float x, float y, float z) override {
        if (v < count) {
            vertices[v*3]   = x; vertices[v*3+1] = y; vertices[v*3+2] = z;
            changed = true;
        }
    }
    virtual void FlushRange(uint32_t, uint32_t) override { EnsureUploaded(); }

    void EnsureUploaded() {
        if (!changed || count == 0) return;
        VkDeviceSize size = count * sizeof(float) * 3;
        VkUploadToHostBuffer(vbuf, vmem, capacity, vertices.data(), size,
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        changed = false;
    }
};

class xlVkVertexColorAccumulator : public xlVertexColorAccumulator {
public:
    std::vector<float>    vertices;  // x,y,z
    std::vector<uint32_t> colors;    // packed RGBA

    uint32_t count = 0;
    bool     finalized         = false;
    bool     mayChangeVertices = false;
    bool     mayChangeColors   = false;
    bool     vchanged = false, cchanged = false;

    VkBuffer       vbuf = VK_NULL_HANDLE; VkDeviceMemory vmem = VK_NULL_HANDLE; VkDeviceSize vcap = 0;
    VkBuffer       cbuf = VK_NULL_HANDLE; VkDeviceMemory cmem = VK_NULL_HANDLE; VkDeviceSize ccap = 0;

    virtual ~xlVkVertexColorAccumulator() {
        VkDestroyBuffer(vbuf, vmem);
        VkDestroyBuffer(cbuf, cmem);
    }

    virtual void Reset() override {
        if (!finalized) { count = 0; vertices.clear(); colors.clear(); }
    }
    virtual void PreAlloc(unsigned int n) override { vertices.reserve(n*3); colors.reserve(n); }
    virtual void AddVertex(float x, float y, float z, const xlColor& c) override {
        if (finalized) return;
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        colors.push_back(c.GetRGBA());
        ++count; vchanged = cchanged = true;
    }
    virtual uint32_t getCount() override { return count; }
    virtual void Finalize(bool mcv, bool mcc) override { finalized = true; mayChangeVertices = mcv; mayChangeColors = mcc; }
    virtual void SetVertex(uint32_t v, float x, float y, float z, const xlColor& c) override {
        if (v<count){ vertices[v*3]=x; vertices[v*3+1]=y; vertices[v*3+2]=z; colors[v]=c.GetRGBA(); vchanged=cchanged=true; }
    }
    virtual void SetVertex(uint32_t v, float x, float y, float z) override {
        if (v<count){ vertices[v*3]=x; vertices[v*3+1]=y; vertices[v*3+2]=z; vchanged=true; }
    }
    virtual void SetVertex(uint32_t v, const xlColor& c) override {
        if (v<count){ colors[v]=c.GetRGBA(); cchanged=true; }
    }
    virtual void FlushRange(uint32_t, uint32_t) override { EnsureUploaded(); }

    void EnsureUploaded() {
        if (vchanged && count>0) {
            VkDeviceSize sz = count * sizeof(float)*3;
            VkUploadToHostBuffer(vbuf, vmem, vcap, vertices.data(), sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            vchanged = false;
        }
        if (cchanged && count>0) {
            VkDeviceSize sz = count * sizeof(uint32_t);
            VkUploadToHostBuffer(cbuf, cmem, ccap, colors.data(), sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            cchanged = false;
        }
    }
};

class xlVkVertexTextureAccumulator : public xlVertexTextureAccumulator {
public:
    std::vector<float> vertices;   // x,y,z
    std::vector<float> tvertices;  // tx,ty
    uint32_t count = 0;
    bool finalized = false, mayChangeV = false, mayChangeT = false;
    bool vchanged = false, tchanged = false;

    VkBuffer       vbuf = VK_NULL_HANDLE; VkDeviceMemory vmem = VK_NULL_HANDLE; VkDeviceSize vcap = 0;
    VkBuffer       tbuf = VK_NULL_HANDLE; VkDeviceMemory tmem = VK_NULL_HANDLE; VkDeviceSize tcap = 0;

    virtual ~xlVkVertexTextureAccumulator() {
        VkDestroyBuffer(vbuf, vmem);
        VkDestroyBuffer(tbuf, tmem);
    }

    virtual void Reset() override {
        if (!finalized) { count = 0; vertices.clear(); tvertices.clear(); }
    }
    virtual void PreAlloc(unsigned int n) override { vertices.reserve(n*3); tvertices.reserve(n*2); }
    virtual void AddVertex(float x, float y, float z, float tx, float ty) override {
        if (finalized) return;
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        tvertices.push_back(tx); tvertices.push_back(ty);
        ++count; vchanged = tchanged = true;
    }
    virtual uint32_t getCount() override { return count; }
    virtual void Finalize(bool mv, bool mt) override { finalized=true; mayChangeV=mv; mayChangeT=mt; }
    virtual void SetVertex(uint32_t v, float x, float y, float z, float tx, float ty) override {
        if (v<count){ vertices[v*3]=x; vertices[v*3+1]=y; vertices[v*3+2]=z;
                      tvertices[v*2]=tx; tvertices[v*2+1]=ty; vchanged=tchanged=true; }
    }
    virtual void FlushRange(uint32_t, uint32_t) override { EnsureUploaded(); }

    void EnsureUploaded() {
        if (vchanged && count>0) {
            VkDeviceSize sz = count*sizeof(float)*3;
            VkUploadToHostBuffer(vbuf, vmem, vcap, vertices.data(), sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            vchanged=false;
        }
        if (tchanged && count>0) {
            VkDeviceSize sz = count*sizeof(float)*2;
            VkUploadToHostBuffer(tbuf, tmem, tcap, tvertices.data(), sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            tchanged=false;
        }
    }
};

// IndexedColor: same CPU-side expansion trick as OGL (expand at Finalize time).
class xlVkVertexIndexedColorAccumulator : public xlVertexIndexedColorAccumulator {
public:
    xlVkVertexColorAccumulator vca;
    std::vector<uint32_t>      colorIndexes;
    std::vector<xlColor>       colors;

    virtual void Reset() override { vca.Reset(); colorIndexes.clear(); }
    virtual void PreAlloc(unsigned int n) override { vca.PreAlloc(n); colorIndexes.reserve(n); }
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override {
        vca.AddVertex(x, y, z, xlBLACK);
        colorIndexes.push_back(cIdx);
    }
    virtual uint32_t getCount() override { return vca.getCount(); }
    virtual void SetColorCount(int c) override { colors.resize(c); }
    virtual uint32_t GetColorCount() override { return (uint32_t)colors.size(); }
    virtual void SetColor(uint32_t idx, const xlColor& c) override { if (idx < colors.size()) colors[idx] = c; }
    virtual void Finalize(bool mcv, bool mcc) override {
        for (size_t i = 0; i < colorIndexes.size(); ++i)
            vca.SetVertex((uint32_t)i, colors[colorIndexes[i]]);
        vca.Finalize(mcv, mcc);
    }
    virtual void SetVertex(uint32_t v, float x, float y, float z, uint32_t cIdx) override { vca.SetVertex(v,x,y,z); colorIndexes[v]=cIdx; }
    virtual void SetVertex(uint32_t v, float x, float y, float z) override { vca.SetVertex(v,x,y,z); }
    virtual void SetVertex(uint32_t v, uint32_t cIdx) override { colorIndexes[v]=cIdx; }
    virtual void FlushRange(uint32_t s, uint32_t l) override { vca.FlushRange(s,l); }
    virtual void FlushColors(uint32_t s, uint32_t l) override {
        for (size_t i = 0; i < colorIndexes.size(); ++i) {
            uint32_t idx = colorIndexes[i];
            if (idx >= s && idx < s+l) vca.SetVertex((uint32_t)i, colors[idx]);
        }
        vca.FlushRange(0, vca.getCount());
    }
};

// ---------------------------------------------------------------------------
// Texture
// ---------------------------------------------------------------------------
class xlVkTexture : public xlTexture {
public:
    VkImage        image    = VK_NULL_HANDLE;
    VkDeviceMemory memory   = VK_NULL_HANDLE;
    VkImageView    view     = VK_NULL_HANDLE;
    VkSampler      sampler  = VK_NULL_HANDLE;
    VkDescriptorSet dset    = VK_NULL_HANDLE;  // allocated from s_shared.descriptorPool

    int  width = 0, height = 0;
    bool bgr = false, hasAlpha = false;

    xlVkTexture() {}
    virtual ~xlVkTexture() { Destroy(); }

    void Destroy() {
        auto& sh = xlVkCanvas::GetShared();
        if (!sh.device) return;
        vkDeviceWaitIdle(sh.device);
        if (dset)    vkFreeDescriptorSets(sh.device, sh.descriptorPool, 1, &dset);
        if (sampler) vkDestroySampler (sh.device, sampler, nullptr);
        if (view)    vkDestroyImageView(sh.device, view,   nullptr);
        if (image)   vkDestroyImage   (sh.device, image,  nullptr);
        if (memory)  vkFreeMemory     (sh.device, memory, nullptr);
        dset = VK_NULL_HANDLE; sampler = VK_NULL_HANDLE;
        view = VK_NULL_HANDLE; image   = VK_NULL_HANDLE; memory = VK_NULL_HANDLE;
    }

    bool Init(int w, int h, bool isBgr, bool alpha, bool mutable_, bool nearest = false) {
        auto& sh = xlVkCanvas::GetShared();
        width = w; height = h; bgr = isBgr; hasAlpha = alpha;

        VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;

        VkImageCreateInfo ici{};
        ici.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.imageType     = VK_IMAGE_TYPE_2D;
        ici.format        = fmt;
        ici.extent        = {(uint32_t)w, (uint32_t)h, 1};
        ici.mipLevels     = 1;
        ici.arrayLayers   = 1;
        ici.samples       = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling        = mutable_ ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
        ici.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(sh.device, &ici, nullptr, &image) != VK_SUCCESS) return false;

        VkMemoryRequirements mr;
        vkGetImageMemoryRequirements(sh.device, image, &mr);

        VkPhysicalDeviceMemoryProperties mp;
        vkGetPhysicalDeviceMemoryProperties(sh.physicalDevice, &mp);
        VkMemoryPropertyFlags memProps = mutable_
            ? (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t mti = UINT32_MAX;
        for (uint32_t i = 0; i < mp.memoryTypeCount; ++i) {
            if ((mr.memoryTypeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & memProps) == memProps)
                { mti = i; break; }
        }
        if (mti == UINT32_MAX) return false;
        VkMemoryAllocateInfo mai{};
        mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mai.allocationSize  = mr.size;
        mai.memoryTypeIndex = mti;
        if (vkAllocateMemory(sh.device, &mai, nullptr, &memory) != VK_SUCCESS) return false;
        vkBindImageMemory(sh.device, image, memory, 0);

        VkImageViewCreateInfo ivci{};
        ivci.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image                       = image;
        ivci.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format                      = fmt;
        ivci.components                  = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.layerCount = 1;
        vkCreateImageView(sh.device, &ivci, nullptr, &view);

        VkSamplerCreateInfo sci{};
        sci.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.magFilter    = nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        sci.minFilter    = nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        vkCreateSampler(sh.device, &sci, nullptr, &sampler);

        // Allocate descriptor set
        VkDescriptorSetAllocateInfo dsai{};
        dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.descriptorPool     = sh.descriptorPool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts        = &sh.textureDSL;
        vkAllocateDescriptorSets(sh.device, &dsai, &dset);

        UpdateDescriptorSet();
        return true;
    }

    void UpdateDescriptorSet() {
        auto& sh = xlVkCanvas::GetShared();
        VkDescriptorImageInfo dii{};
        dii.sampler     = sampler;
        dii.imageView   = view;
        dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet wd{};
        wd.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wd.dstSet          = dset;
        wd.dstBinding      = 0;
        wd.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        wd.descriptorCount = 1;
        wd.pImageInfo      = &dii;
        vkUpdateDescriptorSets(sh.device, 1, &wd, 0, nullptr);
    }

    // Upload RGBA pixel data to the image.
    void UploadRGBA(const uint8_t* rgba, int w, int h) {
        auto& sh = xlVkCanvas::GetShared();
        VkDeviceSize size = w * h * 4;
        VkBuffer stageBuf; VkDeviceMemory stageMem;
        VkCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       stageBuf, stageMem);
        void* mapped;
        vkMapMemory(sh.device, stageMem, 0, size, 0, &mapped);
        memcpy(mapped, rgba, (size_t)size);
        vkUnmapMemory(sh.device, stageMem);

        // Transition → TRANSFER_DST
        VkCommandBuffer cmd;
        VkCommandBufferAllocateInfo ai{};
        ai.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool=sh.commandPool; ai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount=1;
        vkAllocateCommandBuffers(sh.device,&ai,&cmd);
        VkCommandBufferBeginInfo bi{}; bi.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd,&bi);

        VkImageMemoryBarrier barr{};
        barr.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; barr.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        barr.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barr.srcQueueFamilyIndex=barr.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barr.image=image; barr.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
        barr.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&barr);

        VkBufferImageCopy region{};
        region.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
        region.imageExtent={(uint32_t)w,(uint32_t)h,1};
        vkCmdCopyBufferToImage(cmd,stageBuf,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);

        barr.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barr.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barr.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT; barr.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&barr);

        vkEndCommandBuffer(cmd);
        VkSubmitInfo si{}; si.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount=1; si.pCommandBuffers=&cmd;
        vkQueueSubmit(sh.graphicsQueue,1,&si,VK_NULL_HANDLE);
        vkQueueWaitIdle(sh.graphicsQueue);
        vkFreeCommandBuffers(sh.device,sh.commandPool,1,&cmd);
        vkDestroyBuffer(sh.device,stageBuf,nullptr);
        vkFreeMemory   (sh.device,stageMem,nullptr);
    }

    virtual void UpdatePixel(int x, int y, const xlColor& c, bool copyAlpha) override {
        // Simple single-pixel update: rebuild entire row (acceptable for small textures)
        // For production, use vkCmdCopyBufferToImage with a 1x1 region.
        std::vector<uint8_t> pixel(4);
        pixel[0]=c.red; pixel[1]=c.green; pixel[2]=c.blue; pixel[3]=copyAlpha?c.alpha:255u;
        // Re-upload full texture is too slow; do targeted upload
        auto& sh = xlVkCanvas::GetShared();
        VkDeviceSize size = 4;
        VkBuffer stageBuf; VkDeviceMemory stageMem;
        VkCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       stageBuf, stageMem);
        void* mapped;
        vkMapMemory(sh.device, stageMem, 0, size, 0, &mapped);
        memcpy(mapped, pixel.data(), 4);
        vkUnmapMemory(sh.device, stageMem);

        VkCommandBuffer cmd;
        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,nullptr,sh.commandPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY,1};
        vkAllocateCommandBuffers(sh.device,&ai,&cmd);
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,nullptr,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
        vkBeginCommandBuffer(cmd,&bi);

        VkImageMemoryBarrier barr{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barr.oldLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barr.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barr.srcQueueFamilyIndex=barr.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barr.image=image; barr.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
        barr.srcAccessMask=VK_ACCESS_SHADER_READ_BIT; barr.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&barr);

        VkBufferImageCopy region{0,0,0,{VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},{x,y,0},{1,1,1}};
        vkCmdCopyBufferToImage(cmd,stageBuf,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);

        barr.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; barr.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barr.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT; barr.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&barr);

        vkEndCommandBuffer(cmd);
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO,nullptr,0,nullptr,nullptr,1,&cmd};
        vkQueueSubmit(sh.graphicsQueue,1,&si,VK_NULL_HANDLE);
        vkQueueWaitIdle(sh.graphicsQueue);
        vkFreeCommandBuffers(sh.device,sh.commandPool,1,&cmd);
        vkDestroyBuffer(sh.device,stageBuf,nullptr);
        vkFreeMemory   (sh.device,stageMem,nullptr);
    }

    virtual void UpdateData(uint8_t* data, bool bgrIn, bool alphaIn) override {
        // Convert to RGBA if needed, then upload
        std::vector<uint8_t> rgba((size_t)width * height * 4);
        const uint8_t* src = data;
        int srcChannels = alphaIn ? 4 : 3;
        for (int i = 0; i < width * height; ++i, src += srcChannels) {
            if (bgrIn) {
                rgba[i*4+0] = src[2]; rgba[i*4+1] = src[1]; rgba[i*4+2] = src[0];
            } else {
                rgba[i*4+0] = src[0]; rgba[i*4+1] = src[1]; rgba[i*4+2] = src[2];
            }
            rgba[i*4+3] = alphaIn ? src[3] : 255u;
        }
        UploadRGBA(rgba.data(), width, height);
    }
};

// ---------------------------------------------------------------------------
// Vulkan mesh (analogous to xlGLMesh)
// ---------------------------------------------------------------------------
class xlVkMesh : public xlMesh {
public:
    struct SubMesh {
        std::string name;
        int startIndex = 0;
        int count      = 0;
        int material   = -1;
        VkTopology topology = VkTopology::TriangleList;
    };

    xlVkMesh(const std::string& file, xlVkGraphicsContext* ctx) : xlMesh(ctx, file) {}

    virtual ~xlVkMesh() {
        VkDestroyBuffer(vbuf, vmem);
        VkDestroyBuffer(nbuf, nmem);
        VkDestroyBuffer(tbuf, tmem);
        VkDestroyBuffer(ibuf, imem);
        VkDestroyBuffer(wfbuf, wfmem);
        VkDestroyBuffer(lbuf,  lmem);
        // NM UBO
        VkDestroyBuffer(nmUBO, nmUBOMem);
        // Descriptor sets are freed with the pool (or individually in production)
        if (meshDS) {
            auto& sh = xlVkCanvas::GetShared();
            if (sh.device && sh.descriptorPool)
                vkFreeDescriptorSets(sh.device, sh.descriptorPool, 1, &meshDS);
        }
    }

    void LoadBuffers();

    VkBuffer       vbuf = VK_NULL_HANDLE; VkDeviceMemory vmem = VK_NULL_HANDLE;  // positions
    VkBuffer       nbuf = VK_NULL_HANDLE; VkDeviceMemory nmem = VK_NULL_HANDLE;  // normals
    VkBuffer       tbuf = VK_NULL_HANDLE; VkDeviceMemory tmem = VK_NULL_HANDLE;  // tex coords
    VkBuffer       ibuf = VK_NULL_HANDLE; VkDeviceMemory imem = VK_NULL_HANDLE;  // triangle indexes
    VkBuffer       wfbuf= VK_NULL_HANDLE; VkDeviceMemory wfmem= VK_NULL_HANDLE;  // wireframe indexes
    VkBuffer       lbuf = VK_NULL_HANDLE; VkDeviceMemory lmem = VK_NULL_HANDLE;  // line indexes
    uint32_t       indexCount = 0;
    uint32_t       wfCount    = 0;
    uint32_t       lineCount  = 0;

    // NM uniform buffer + descriptor set for mesh shaders
    VkBuffer         nmUBO    = VK_NULL_HANDLE;
    VkDeviceMemory   nmUBOMem = VK_NULL_HANDLE;
    VkDeviceSize     nmUBOCap = 0;
    VkDescriptorSet  meshDS   = VK_NULL_HANDLE;

    std::vector<SubMesh> subMeshes;
};

static uint32_t GetOrAddMeshVertex(
        std::map<std::tuple<int,int,int>, uint32_t>& idxMap,
        std::vector<float>& positions,
        std::vector<float>& normals,
        std::vector<float>& texcoords,
        const tinyobj::attrib_t& attrib,
        const tinyobj::index_t& idx) {
    auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);
    auto it = idxMap.find(key);
    if (it != idxMap.end()) return it->second;

    uint32_t newIdx = (uint32_t)(positions.size() / 3);
    idxMap[key] = newIdx;

    if (idx.vertex_index >= 0) {
        positions.push_back(attrib.vertices[3*idx.vertex_index+0]);
        positions.push_back(attrib.vertices[3*idx.vertex_index+1]);
        positions.push_back(attrib.vertices[3*idx.vertex_index+2]);
    } else { positions.push_back(0); positions.push_back(0); positions.push_back(0); }

    if (idx.normal_index >= 0) {
        normals.push_back(attrib.normals[3*idx.normal_index+0]);
        normals.push_back(attrib.normals[3*idx.normal_index+1]);
        normals.push_back(attrib.normals[3*idx.normal_index+2]);
    } else { normals.push_back(0); normals.push_back(1); normals.push_back(0); }

    if (idx.texcoord_index >= 0) {
        texcoords.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
        texcoords.push_back(1.0f - attrib.texcoords[2*idx.texcoord_index+1]);
    } else { texcoords.push_back(0); texcoords.push_back(0); }

    return newIdx;
}

void xlVkMesh::LoadBuffers() {
    auto& attrib  = objects.GetAttrib();
    auto& shapes  = objects.GetShapes();

    std::vector<float>    positions, normals, texcoords;
    std::vector<uint32_t> indexes, wireframe, lines;
    std::map<std::tuple<int,int,int>, uint32_t> idxMap;

    xlVkMesh::SubMesh* lastSM = nullptr;
    for (auto& s : shapes) {
        if (!s.mesh.indices.empty()) {
            for (size_t fi = 0; fi < s.mesh.material_ids.size(); ++fi) {
                int matId = s.mesh.material_ids[fi];
                if (!lastSM || matId != lastSM->material) {
                    subMeshes.push_back({});
                    lastSM = &subMeshes.back();
                    lastSM->name       = s.name;
                    lastSM->startIndex = (int)indexes.size();
                    lastSM->material   = matId;
                    lastSM->topology   = VkTopology::TriangleList;
                }
                uint32_t v0 = GetOrAddMeshVertex(idxMap, positions, normals, texcoords, attrib, s.mesh.indices[fi*3+0]);
                uint32_t v1 = GetOrAddMeshVertex(idxMap, positions, normals, texcoords, attrib, s.mesh.indices[fi*3+1]);
                uint32_t v2 = GetOrAddMeshVertex(idxMap, positions, normals, texcoords, attrib, s.mesh.indices[fi*3+2]);
                indexes.push_back(v0); indexes.push_back(v1); indexes.push_back(v2);
                wireframe.push_back(v0); wireframe.push_back(v1);
                wireframe.push_back(v1); wireframe.push_back(v2);
                wireframe.push_back(v2); wireframe.push_back(v0);
            }
            if (lastSM) lastSM->count = (int)indexes.size() - lastSM->startIndex;
        }
        if (!s.lines.indices.empty()) {
            for (auto& li : s.lines.indices) {
                lines.push_back(GetOrAddMeshVertex(idxMap, positions, normals, texcoords, attrib, li));
            }
        }
    }

    indexCount = (uint32_t)indexes.size();
    wfCount    = (uint32_t)wireframe.size();
    lineCount  = (uint32_t)lines.size();

    auto upload = [](VkBuffer& b, VkDeviceMemory& m, const void* d, VkDeviceSize sz, VkBufferUsageFlags usage) {
        VkCreateBuffer(sz, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, b, m);
        VkUploadToDeviceBuffer(b, d, sz);
    };

    if (!positions.empty())  upload(vbuf,  vmem,  positions.data(),  positions.size()*4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!normals.empty())    upload(nbuf,  nmem,  normals.data(),    normals.size()*4,   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!texcoords.empty())  upload(tbuf,  tmem,  texcoords.data(),  texcoords.size()*4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    if (!indexes.empty())    upload(ibuf,  imem,  indexes.data(),    indexes.size()*4,   VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (!wireframe.empty())  upload(wfbuf, wfmem, wireframe.data(),  wireframe.size()*4, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (!lines.empty())      upload(lbuf,  lmem,  lines.data(),      lines.size()*4,     VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // Create NM UBO + descriptor set
    auto& sh = xlVkCanvas::GetShared();
    glm::mat4 identity(1.0f);
    VkCreateBuffer(sizeof(glm::mat4),
                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   nmUBO, nmUBOMem);
    void* mapped;
    vkMapMemory(sh.device, nmUBOMem, 0, sizeof(glm::mat4), 0, &mapped);
    memcpy(mapped, &identity, sizeof(glm::mat4));
    vkUnmapMemory(sh.device, nmUBOMem);
    nmUBOCap = sizeof(glm::mat4);

    VkDescriptorSetAllocateInfo dsai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,nullptr,sh.descriptorPool,1,&sh.meshDSL};
    vkAllocateDescriptorSets(sh.device, &dsai, &meshDS);

    // Write NM UBO binding (binding=0)
    VkDescriptorBufferInfo dbi{nmUBO, 0, sizeof(glm::mat4)};
    VkWriteDescriptorSet wd{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,nullptr,meshDS,0,0,1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,nullptr,&dbi};
    vkUpdateDescriptorSets(sh.device, 1, &wd, 0, nullptr);
    // binding=1 (sampler) left null until a submesh uses a texture
}

// ---------------------------------------------------------------------------
// xlVkGraphicsContext
// ---------------------------------------------------------------------------
xlVkGraphicsContext::xlVkGraphicsContext(xlVkCanvas* canvas)
    : mCanvas(canvas)
    , mCmd(canvas->getCurrentCmdBuf()) {
    mFrameData.MVP              = glm::mat4(1.0f);
    mFrameData.modelMatrix      = glm::mat4(1.0f);
    mFrameData.viewMatrix       = glm::mat4(1.0f);
    mFrameData.perspectiveMatrix= glm::mat4(1.0f);
    mPC = {};
    mPC.MVP         = mFrameData.MVP;
    mPC.inColor     = glm::vec4(1.0f);
    mPC.renderType  = 0;
    mPC.smoothMin   = 0.4f;
    mPC.smoothMax   = 0.5f;
    mPC.brightness  = 1.0f;
}

xlVkGraphicsContext::~xlVkGraphicsContext() {}

void xlVkGraphicsContext::EndFrame() {
    // Nothing to do here; render pass is ended by the canvas.
}

// ---------------------------------------------------------------------------
// Viewport
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::SetViewport(int x1, int y1, int x2, int y2, bool is3D) {
    mFrameData.modelMatrix  = glm::mat4(1.0f);
    mFrameData.viewMatrix   = glm::mat4(1.0f);

    float w = (float)(x2 - x1);
    float h = (float)(y1 - y2);  // y1 > y2 in top-left coords

    if (is3D) {
        mFrameData.perspectiveMatrix = glm::perspective(glm::radians(45.0f), w / h, 1.0f, 200000.0f);
        mFrameData.MVP = mFrameData.perspectiveMatrix;
    } else {
        mFrameData.MVP = glm::ortho((float)x1, (float)x2, (float)y2, (float)y1);
        mFrameData.perspectiveMatrix = mFrameData.MVP;
    }
    mPC.MVP = mFrameData.MVP;

    // Update dynamic viewport/scissor to match sub-region
    double sf = mCanvas->contentScale();
    VkViewport vp{};
    vp.x        = (float)(std::min(x1,x2) * sf);
    vp.y        = (float)(std::min(y1,y2) * sf);
    vp.width    = (float)(std::abs(x2-x1) * sf);
    vp.height   = (float)(std::abs(y1-y2) * sf);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(mCmd, 0, 1, &vp);

    VkRect2D scissor{{(int32_t)vp.x, (int32_t)vp.y}, {(uint32_t)vp.width, (uint32_t)vp.height}};
    vkCmdSetScissor(mCmd, 0, 1, &scissor);

    return this;
}

// ---------------------------------------------------------------------------
// Blending
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::enableBlending(bool e) {
    mBlending = e;
    mLastKeyValid = false; // force pipeline rebind
    return this;
}

// ---------------------------------------------------------------------------
// Accumulator / texture factories
// ---------------------------------------------------------------------------
xlVertexAccumulator*             xlVkGraphicsContext::createVertexAccumulator()             { return new xlVkVertexAccumulator(); }
xlVertexColorAccumulator*        xlVkGraphicsContext::createVertexColorAccumulator()        { return new xlVkVertexColorAccumulator(); }
xlVertexTextureAccumulator*      xlVkGraphicsContext::createVertexTextureAccumulator()      { return new xlVkVertexTextureAccumulator(); }
xlVertexIndexedColorAccumulator* xlVkGraphicsContext::createVertexIndexedColorAccumulator() { return new xlVkVertexIndexedColorAccumulator(); }
xlGraphicsProgram*               xlVkGraphicsContext::createGraphicsProgram()               { return new xlGraphicsProgram(createVertexColorAccumulator()); }

xlTexture* xlVkGraphicsContext::createTextureInternal(const xlImage& image, bool /*mipMaps*/, const std::string& nm) {
    auto* t = new xlVkTexture();
    t->SetName(nm);
    if (!t->Init(image.GetWidth(), image.GetHeight(), false, true, false, true)) {
        delete t; return nullptr;
    }
    t->UploadRGBA(image.GetData(), image.GetWidth(), image.GetHeight());
    return t;
}

xlTexture* xlVkGraphicsContext::createTextureMipMaps(const std::vector<xlImage>& images, const std::string& nm) {
    if (images.empty()) return nullptr;
    return createTextureInternal(images[0], true, nm);
}
xlTexture* xlVkGraphicsContext::createTexture(const xlImage& image, const std::string& nm, bool finalize) {
    return createTextureInternal(image, false, nm);
}
xlTexture* xlVkGraphicsContext::createTexture(int w, int h, bool bgr, bool alpha) {
    auto* t = new xlVkTexture();
    t->Init(w, h, bgr, alpha, true, true);
    return t;
}
xlTexture* xlVkGraphicsContext::createTextureForFont(const xlFontInfo& font) {
    return createTexture(font.getImage(), "Font-" + std::to_string(font.getSize()));
}
std::unique_ptr<xlMesh> xlVkGraphicsContext::loadMeshFromObjFile(const std::string& file) {
    return std::make_unique<xlVkMesh>(file, this);
}

// ---------------------------------------------------------------------------
// Pipeline binding
// ---------------------------------------------------------------------------
void xlVkGraphicsContext::bindPipeline(const VkPipelineKey& key) {
    if (mLastKeyValid && mLastKey.type==key.type && mLastKey.topology==key.topology && mLastKey.blend==key.blend)
        return;
    auto it = s_pipelines.find(key);
    if (it == s_pipelines.end()) {
        spdlog::warn("xlVkGraphicsContext: pipeline not found for key ({},{},{})",
                     (int)key.type, (int)key.topology, key.blend);
        return;
    }
    vkCmdBindPipeline(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, it->second);
    mLastKey = key; mLastKeyValid = true;
}

void xlVkGraphicsContext::pushConstants() {
    mPC.MVP = mFrameData.MVP;
    // Choose the right layout based on current pipeline type
    VkPipelineLayout layout = xlVkCanvas::GetShared().basicLayout;
    if (mLastKeyValid) {
        switch (mLastKey.type) {
        case VkPipelineType::Texture:
        case VkPipelineType::TextureColor:
            layout = xlVkCanvas::GetShared().textureLayout; break;
        case VkPipelineType::MeshSolid:
        case VkPipelineType::MeshTexture:
            layout = xlVkCanvas::GetShared().meshLayout; break;
        default: break;
        }
    }
    vkCmdPushConstants(mCmd, layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(VkPushConstants), &mPC);
}

void xlVkGraphicsContext::setPointSmoothParams(float ps) {
    float delta = 1.0f / (ps + 1.0f);
    float mid   = 0.35f + 0.15f * ((ps - 1.0f) / 25.0f);
    mid = std::min(mid, 0.5f);
    mPC.smoothMin = std::max(0.0f, mid - delta);
    mPC.smoothMax = std::min(1.0f, mid + delta);
}

// ---------------------------------------------------------------------------
// Draw helpers
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::drawPrimitive(VkPipelineType type, VkTopology topo,
                                                       xlVertexAccumulator* vac,
                                                       const xlColor& c, int start, int count) {
    auto* v = static_cast<xlVkVertexAccumulator*>(vac);
    if (v->count == 0) return this;
    int cnt = (count < 0) ? (int)v->count - start : count;
    if (cnt <= 0) return this;

    v->EnsureUploaded();

    VkPipelineKey key{type, topo, mBlending};
    bindPipeline(key);

    mPC.inColor = glm::vec4(c.red/255.0f, c.green/255.0f, c.blue/255.0f, c.alpha/255.0f);
    pushConstants();

    VkDeviceSize offset = (VkDeviceSize)(start * sizeof(float) * 3);
    vkCmdBindVertexBuffers(mCmd, 0, 1, &v->vbuf, &offset);
    vkCmdDraw(mCmd, (uint32_t)cnt, 1, 0, 0);
    return this;
}

xlGraphicsContext* xlVkGraphicsContext::drawPrimitive(VkPipelineType type, VkTopology topo,
                                                       xlVertexColorAccumulator* vac,
                                                       int start, int count) {
    auto* v = static_cast<xlVkVertexColorAccumulator*>(vac);
    if (v->count == 0) return this;
    int cnt = (count < 0) ? (int)v->count - start : count;
    if (cnt <= 0) return this;

    v->EnsureUploaded();

    VkPipelineKey key{type, topo, mBlending};
    bindPipeline(key);
    pushConstants();

    VkBuffer     bufs[2] = {v->vbuf, v->cbuf};
    VkDeviceSize offs[2] = {(VkDeviceSize)(start * sizeof(float)*3), (VkDeviceSize)(start * sizeof(uint32_t))};
    vkCmdBindVertexBuffers(mCmd, 0, 2, bufs, offs);
    vkCmdDraw(mCmd, (uint32_t)cnt, 1, 0, 0);
    return this;
}

// ---------------------------------------------------------------------------
// Single-color draw calls
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::drawLines        (xlVertexAccumulator* v, const xlColor& c, int s, int n) { return drawPrimitive(VkPipelineType::SingleColor, VkTopology::LineList,      v,c,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawLineStrip    (xlVertexAccumulator* v, const xlColor& c, int s, int n) { return drawPrimitive(VkPipelineType::SingleColor, VkTopology::LineStrip,     v,c,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangles    (xlVertexAccumulator* v, const xlColor& c, int s, int n) { return drawPrimitive(VkPipelineType::SingleColor, VkTopology::TriangleList,  v,c,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangleStrip(xlVertexAccumulator* v, const xlColor& c, int s, int n) { return drawPrimitive(VkPipelineType::SingleColor, VkTopology::TriangleStrip, v,c,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawPoints       (xlVertexAccumulator* v, const xlColor& c, float ps, bool smooth, int s, int n) {
    if (smooth) { setPointSmoothParams(ps); mPC.renderType = 1; }
    auto* r = drawPrimitive(VkPipelineType::SingleColor, VkTopology::PointList, v, c, s, n);
    mPC.renderType = 0;
    return r;
}

// ---------------------------------------------------------------------------
// Per-vertex color draw calls
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::drawLines        (xlVertexColorAccumulator* v, int s, int n) { return drawPrimitive(VkPipelineType::PerVertexColor, VkTopology::LineList,      v,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawLineStrip    (xlVertexColorAccumulator* v, int s, int n) { return drawPrimitive(VkPipelineType::PerVertexColor, VkTopology::LineStrip,     v,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangles    (xlVertexColorAccumulator* v, int s, int n) { return drawPrimitive(VkPipelineType::PerVertexColor, VkTopology::TriangleList,  v,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangleStrip(xlVertexColorAccumulator* v, int s, int n) { return drawPrimitive(VkPipelineType::PerVertexColor, VkTopology::TriangleStrip, v,s,n); }
xlGraphicsContext* xlVkGraphicsContext::drawPoints       (xlVertexColorAccumulator* v, float ps, bool smooth, int s, int n) {
    if (smooth) { setPointSmoothParams(ps); mPC.renderType = 1; }
    auto* r = drawPrimitive(VkPipelineType::PerVertexColor, VkTopology::PointList, v, s, n);
    mPC.renderType = 0;
    return r;
}

// ---------------------------------------------------------------------------
// Indexed-color draw calls (delegate to the inner xlVkVertexColorAccumulator)
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::drawLines        (xlVertexIndexedColorAccumulator* v, int s, int n) { return drawLines        (&static_cast<xlVkVertexIndexedColorAccumulator*>(v)->vca, s, n); }
xlGraphicsContext* xlVkGraphicsContext::drawLineStrip    (xlVertexIndexedColorAccumulator* v, int s, int n) { return drawLineStrip    (&static_cast<xlVkVertexIndexedColorAccumulator*>(v)->vca, s, n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangles    (xlVertexIndexedColorAccumulator* v, int s, int n) { return drawTriangles    (&static_cast<xlVkVertexIndexedColorAccumulator*>(v)->vca, s, n); }
xlGraphicsContext* xlVkGraphicsContext::drawTriangleStrip(xlVertexIndexedColorAccumulator* v, int s, int n) { return drawTriangleStrip(&static_cast<xlVkVertexIndexedColorAccumulator*>(v)->vca, s, n); }
xlGraphicsContext* xlVkGraphicsContext::drawPoints       (xlVertexIndexedColorAccumulator* v, float ps, bool smooth, int s, int n) { return drawPoints(&static_cast<xlVkVertexIndexedColorAccumulator*>(v)->vca, ps, smooth, s, n); }

// ---------------------------------------------------------------------------
// Texture draw calls
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::drawTexture(xlTexture* texture,
                                                     float x, float y, float x2, float y2,
                                                     float tx, float ty, float tx2, float ty2,
                                                     bool /*smooth*/, int brightness, int alpha) {
    xlVkVertexTextureAccumulator va;
    va.PreAlloc(6);
    va.AddVertex(x,  y,  0, tx,  ty);
    va.AddVertex(x,  y2, 0, tx,  ty2);
    va.AddVertex(x2, y2, 0, tx2, ty2);
    va.AddVertex(x2, y2, 0, tx2, ty2);
    va.AddVertex(x2, y,  0, tx2, ty);
    va.AddVertex(x,  y,  0, tx,  ty);
    float b = brightness / 100.0f;
    mPC.inColor    = glm::vec4(b, b, b, alpha / 255.0f);
    mPC.renderType = 0;
    return drawTexture(&va, texture, brightness, (uint8_t)alpha, 0, 6);
}

xlGraphicsContext* xlVkGraphicsContext::drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture,
                                                     int brightness, uint8_t alpha, int start, int count) {
    auto* va = static_cast<xlVkVertexTextureAccumulator*>(vac);
    auto* t  = static_cast<xlVkTexture*>(texture);
    if (va->count == 0 || !t) return this;
    int cnt = (count < 0) ? (int)va->count - start : count;
    if (cnt <= 0) return this;

    va->EnsureUploaded();

    VkPipelineKey key{VkPipelineType::Texture, VkTopology::TriangleList, mBlending};
    bindPipeline(key);

    float b = brightness / 100.0f;
    mPC.inColor    = glm::vec4(b, b, b, alpha / 255.0f);
    mPC.renderType = 0;
    pushConstants();

    vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            xlVkCanvas::GetShared().textureLayout,
                            0, 1, &t->dset, 0, nullptr);

    VkBuffer     bufs[2] = {va->vbuf, va->tbuf};
    VkDeviceSize offs[2] = {(VkDeviceSize)(start*sizeof(float)*3), (VkDeviceSize)(start*sizeof(float)*2)};
    vkCmdBindVertexBuffers(mCmd, 0, 2, bufs, offs);
    vkCmdDraw(mCmd, (uint32_t)cnt, 1, 0, 0);
    return this;
}

xlGraphicsContext* xlVkGraphicsContext::drawTexture(xlVertexTextureAccumulator* vac, xlTexture* texture,
                                                     const xlColor& c, int start, int count) {
    auto* va = static_cast<xlVkVertexTextureAccumulator*>(vac);
    auto* t  = static_cast<xlVkTexture*>(texture);
    if (va->count == 0 || !t) return this;
    int cnt = (count < 0) ? (int)va->count - start : count;
    if (cnt <= 0) return this;

    va->EnsureUploaded();

    VkPipelineKey key{VkPipelineType::TextureColor, VkTopology::TriangleList, mBlending};
    bindPipeline(key);

    mPC.inColor    = glm::vec4(c.red/255.0f, c.green/255.0f, c.blue/255.0f, c.alpha/255.0f);
    mPC.renderType = 1;  // alpha-only tint mode
    pushConstants();

    vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            xlVkCanvas::GetShared().textureLayout,
                            0, 1, &t->dset, 0, nullptr);

    VkBuffer     bufs[2] = {va->vbuf, va->tbuf};
    VkDeviceSize offs[2] = {(VkDeviceSize)(start*sizeof(float)*3), (VkDeviceSize)(start*sizeof(float)*2)};
    vkCmdBindVertexBuffers(mCmd, 0, 2, bufs, offs);
    vkCmdDraw(mCmd, (uint32_t)cnt, 1, 0, 0);
    mPC.renderType = 0;
    return this;
}

// ---------------------------------------------------------------------------
// Mesh draw
// ---------------------------------------------------------------------------
void xlVkGraphicsContext::drawMesh(xlMesh* mesh, int brightness, bool useViewMatrix, bool transparents) {
    auto* m = static_cast<xlVkMesh*>(mesh);
    if (!m->vbuf) m->LoadBuffers();
    if (m->indexCount == 0) return;

    float b = brightness / 100.0f;
    glm::mat4 nm = useViewMatrix ? mFrameData.viewMatrix * mFrameData.modelMatrix : mFrameData.modelMatrix;

    // Update NM UBO
    auto& sh = xlVkCanvas::GetShared();
    void* mapped;
    vkMapMemory(sh.device, m->nmUBOMem, 0, sizeof(glm::mat4), 0, &mapped);
    memcpy(mapped, &nm, sizeof(glm::mat4));
    vkUnmapMemory(sh.device, m->nmUBOMem);

    vkCmdBindIndexBuffer(mCmd, m->ibuf, 0, VK_INDEX_TYPE_UINT32);

    xlTexture* lastTexture = nullptr;
    bool lastIsSolid = true;

    for (auto& sm : m->subMeshes) {
        int mid = sm.material;
        bool output = (mid < 0 || mesh->GetMaterial(mid).color.alpha == 255);
        if (transparents) output = (mid >= 0 && mesh->GetMaterial(mid).color.alpha != 255);
        if (!output) continue;

        bool hasTexture = (mid >= 0 && mesh->GetMaterial(mid).texture && !mesh->GetMaterial(mid).forceColor);
        if (!hasTexture) {
            // Solid color submesh
            VkPipelineKey key{VkPipelineType::MeshSolid, VkTopology::TriangleList, mBlending};
            bindPipeline(key);

            xlColor col = (mid < 0) ? xlWHITE : mesh->GetMaterial(mid).color;
            mPC.inColor = glm::vec4(col.red/255.0f*b, col.green/255.0f*b, col.blue/255.0f*b, col.alpha/255.0f);
            vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    sh.meshLayout, 0, 1, &m->meshDS, 0, nullptr);
            pushConstants();
            lastIsSolid = true;
        } else {
            // Textured submesh
            VkPipelineKey key{VkPipelineType::MeshTexture, VkTopology::TriangleList, mBlending};
            bindPipeline(key);

            auto* tx = static_cast<xlVkTexture*>(mesh->GetMaterial(mid).texture);
            mPC.inColor     = glm::vec4(0,0,0,1);
            mPC.brightness  = b;

            // Update binding=1 (sampler) of the mesh descriptor set if texture changed
            if (tx && tx != lastTexture) {
                VkDescriptorImageInfo dii{tx->sampler, tx->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                VkWriteDescriptorSet wd{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,nullptr,m->meshDS,1,0,1,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&dii};
                vkUpdateDescriptorSets(sh.device, 1, &wd, 0, nullptr);
                lastTexture = tx;
            }
            vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    sh.meshLayout, 0, 1, &m->meshDS, 0, nullptr);
            pushConstants();
            lastIsSolid = false;
        }

        VkBuffer     vbufs[3] = {m->vbuf, m->nbuf, m->tbuf};
        VkDeviceSize voffs[3] = {0, 0, 0};
        int numBufs = hasTexture ? 3 : 2;
        vkCmdBindVertexBuffers(mCmd, 0, numBufs, vbufs, voffs);
        vkCmdDrawIndexed(mCmd, (uint32_t)sm.count, 1, (uint32_t)sm.startIndex, 0, 0);
    }

    mPC.brightness = 1.0f;

    // Line elements (not in transparent pass)
    if (!transparents && m->lineCount > 0) {
        VkPipelineKey key{VkPipelineType::MeshSolid, VkTopology::LineList, mBlending};
        bindPipeline(key);
        mPC.inColor = glm::vec4(0,0,0,1);
        vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                sh.meshLayout, 0, 1, &m->meshDS, 0, nullptr);
        pushConstants();
        VkBuffer     vbufs[2] = {m->vbuf, m->nbuf};
        VkDeviceSize voffs[2] = {0, 0};
        vkCmdBindVertexBuffers(mCmd, 0, 2, vbufs, voffs);
        vkCmdBindIndexBuffer(mCmd, m->lbuf, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(mCmd, m->lineCount, 1, 0, 0, 0);
    }
}

xlGraphicsContext* xlVkGraphicsContext::drawMeshSolids      (xlMesh* m, int b, bool uv) { drawMesh(m,b,uv,false); return this; }
xlGraphicsContext* xlVkGraphicsContext::drawMeshTransparents(xlMesh* m, int b)           { drawMesh(m,b,false,true); return this; }
xlGraphicsContext* xlVkGraphicsContext::drawMeshWireframe   (xlMesh* mesh, int brightness) {
    auto* m = static_cast<xlVkMesh*>(mesh);
    if (!m->vbuf) m->LoadBuffers();
    if (m->wfCount == 0) return this;

    VkPipelineKey key{VkPipelineType::MeshSolid, VkTopology::LineList, mBlending};
    bindPipeline(key);
    float b = brightness / 100.0f;
    mPC.inColor = glm::vec4(0, b, 0, 1);
    vkCmdBindDescriptorSets(mCmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            xlVkCanvas::GetShared().meshLayout, 0, 1, &m->meshDS, 0, nullptr);
    pushConstants();
    VkBuffer     bufs[2] = {m->vbuf, m->nbuf};
    VkDeviceSize offs[2] = {0, 0};
    vkCmdBindVertexBuffers(mCmd, 0, 2, bufs, offs);
    vkCmdBindIndexBuffer(mCmd, m->wfbuf, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(mCmd, m->wfCount, 1, 0, 0, 0);
    return this;
}

// ---------------------------------------------------------------------------
// Matrix operations
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::PushMatrix() {
    mMatrixStack.push({mFrameData.MVP, mFrameData.viewMatrix, mFrameData.modelMatrix});
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::PopMatrix() {
    if (!mMatrixStack.empty()) {
        auto [mvp, view, model] = mMatrixStack.top();
        mMatrixStack.pop();
        mFrameData.MVP         = mvp;
        mFrameData.viewMatrix  = view;
        mFrameData.modelMatrix = model;
        mPC.MVP = mFrameData.MVP;
    }
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::Translate(float x, float y, float z) {
    auto t = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
    mFrameData.MVP         = mFrameData.MVP * t;
    mFrameData.modelMatrix = mFrameData.modelMatrix * t;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::Rotate(float angle, float x, float y, float z) {
    angle = angle * 3.14159f / 180.0f;
    auto r = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(x,y,z));
    mFrameData.MVP         = mFrameData.MVP * r;
    mFrameData.modelMatrix = mFrameData.modelMatrix * r;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::Scale(float w, float h, float z) {
    auto s = glm::scale(glm::mat4(1.0f), glm::vec3(w,h,z));
    mFrameData.MVP         = mFrameData.MVP * s;
    mFrameData.modelMatrix = mFrameData.modelMatrix * s;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::ApplyMatrix(const glm::mat4& m) {
    mFrameData.MVP         = mFrameData.MVP * m;
    mFrameData.modelMatrix = mFrameData.modelMatrix * m;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::SetCamera(const glm::mat4& m) {
    mFrameData.MVP        = mFrameData.MVP * m;
    mFrameData.viewMatrix = mFrameData.viewMatrix * m;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::SetModelMatrix(const glm::mat4& m) {
    mFrameData.MVP         = mFrameData.MVP * m;
    mFrameData.modelMatrix = mFrameData.modelMatrix * m;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::ScaleViewMatrix(float w, float h, float z) {
    auto s = glm::scale(glm::mat4(1.0f), glm::vec3(w,h,z));
    mFrameData.MVP        = mFrameData.MVP * s;
    mFrameData.viewMatrix = mFrameData.viewMatrix * s;
    mPC.MVP = mFrameData.MVP;
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::TranslateViewMatrix(float x, float y, float z) {
    auto t = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
    mFrameData.MVP        = mFrameData.MVP * t;
    mFrameData.viewMatrix = mFrameData.viewMatrix * t;
    mPC.MVP = mFrameData.MVP;
    return this;
}

// ---------------------------------------------------------------------------
// Debug markers (via VK_EXT_debug_utils if present, else no-op)
// ---------------------------------------------------------------------------
xlGraphicsContext* xlVkGraphicsContext::pushDebugContext(const std::string& label) {
    // vkCmdBeginDebugUtilsLabelEXT is available via VK_EXT_debug_utils.
    // Use a function pointer loaded at runtime to keep this optional.
    static auto fn = (PFN_vkCmdBeginDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(xlVkCanvas::GetShared().instance, "vkCmdBeginDebugUtilsLabelEXT");
    if (fn) {
        VkDebugUtilsLabelEXT lbl{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        lbl.pLabelName = label.c_str();
        fn(mCmd, &lbl);
    }
    return this;
}
xlGraphicsContext* xlVkGraphicsContext::popDebugContext() {
    static auto fn = (PFN_vkCmdEndDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(xlVkCanvas::GetShared().instance, "vkCmdEndDebugUtilsLabelEXT");
    if (fn) fn(mCmd);
    return this;
}
