/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Native Vulkan Shader (ISF) effect render path.  Parallels
// effects/metal/MetalShaderEffect.mm: it reuses ShaderConfig (ISF parse + GLSL
// assembly) unchanged, rewrites the assembled desktop-GLSL fragment into Vulkan
// dialect (all non-sampler uniforms folded into one anonymous-instance std140
// UBO block bound at set0/binding0, texSampler at set0/binding1, explicit
// varying/out locations), generates a matching attribute-less fullscreen-triangle
// vertex shader, translates both to SPIR-V, builds a graphics pipeline, packs the
// per-frame uniform values into a host-visible UBO, and renders into the
// RenderBuffer.  Generative shaders work end-to-end today; the input image/audio
// texture is bound as the foundation's 1x1 dummy for now (texSampler stays
// declared so image shaders still compile).  Compiled only under HAVE_VULKAN.
#ifdef HAVE_VULKAN_SHADER

#include "VulkanShaderEffect.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>
#include <regex>

#include "VulkanGraphicsUtilities.h"
#include "VulkanComputeUtilities.h"
#include "VulkanShaderTranslate.h"

#include "../ShaderEffect.h"
#include "../../render/RenderBuffer.h"

namespace {

static const bool sDbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;

// One member of the generated std140 UBO block.  vecSize/isFloat drive both the
// std140 layout and the float-bits-vs-int-bits encoding at pack time (spec §5).
struct UBOMember {
    std::string glslType;   // "float", "vec2", "vec4", "int", "bool", ...
    std::string name;
    uint8_t vecSize = 1;
    bool isFloat = true;
    uint32_t align = 4;
    uint32_t size = 4;
    uint32_t offset = 0;    // std140 byte offset, filled by computeStd140
};

// Map a GLSL scalar/vector type to (vecSize, isFloat, std140 align, std140 size).
// Returns false for types we don't fold into the UBO (samplers, matrices, ...).
static bool classifyType(const std::string& t, uint8_t& vecSize, bool& isFloat, uint32_t& align, uint32_t& size) {
    auto set = [&](uint8_t vs, bool fl, uint32_t al, uint32_t sz) {
        vecSize = vs; isFloat = fl; align = al; size = sz; return true;
    };
    if (t == "float") return set(1, true, 4, 4);
    if (t == "int" || t == "uint" || t == "bool") return set(1, false, 4, 4);
    if (t == "vec2") return set(2, true, 8, 8);
    if (t == "vec3") return set(3, true, 16, 12);
    if (t == "vec4") return set(4, true, 16, 16);
    if (t == "ivec2" || t == "uvec2" || t == "bvec2") return set(2, false, 8, 8);
    if (t == "ivec3" || t == "uvec3" || t == "bvec3") return set(3, false, 16, 12);
    if (t == "ivec4" || t == "uvec4" || t == "bvec4") return set(4, false, 16, 16);
    return false;
}

static void computeStd140(std::vector<UBOMember>& members, uint32_t& uboSize) {
    uint32_t off = 0;
    for (auto& m : members) {
        off = (off + m.align - 1) & ~(m.align - 1);
        m.offset = off;
        off += m.size;
    }
    // std140 struct alignment is 16; never emit a zero-length buffer.
    uboSize = (off + 15u) & ~15u;
    if (uboSize == 0) uboSize = 16;
}

static std::string ltrim(const std::string& s) {
    size_t p = s.find_first_not_of(" \t\r");
    return (p == std::string::npos) ? std::string() : s.substr(p);
}
static std::string rtrim(std::string s) {
    size_t p = s.find_last_not_of(" \t\r");
    if (p == std::string::npos) return std::string();
    s.erase(p + 1);
    return s;
}
static bool startsWith(const std::string& s, const char* pre) {
    return s.rfind(pre, 0) == 0;
}

static std::string emitUBOBlock(const std::vector<UBOMember>& members) {
    // Named block with the INSTANCE name omitted so user/vertex code keeps bare
    // member access (TIME, RENDERSIZE, ...).  A truly-nameless block is a GLSL
    // syntax error, hence the "_XLUniforms" block name.  Must be byte-identical
    // in both stages for cross-stage interface matching.
    std::string b = "layout(std140, set=0, binding=0) uniform _XLUniforms {\n";
    for (const auto& m : members) {
        b += "    " + m.glslType + " " + m.name + ";\n";
    }
    b += "};\n";
    return b;
}

// Rewrite the ShaderConfig-assembled fragment GLSL (desktop 330 / ES 300, loose
// uniforms) into Vulkan-dialect GLSL, collecting the UBO member list in
// declaration order.  Returns false only if no member list could be built.
static bool assembleVulkanGLSL(const std::string& code,
                               std::vector<UBOMember>& members,
                               std::string& fragOut, std::string& vertOut) {
    std::string extensions;
    std::string body;
    int brace = 0;
    bool versionSeen = false;

    // Explicit locations for the coordinate varyings, matching the generated
    // vertex stage's outputs (texCoord loc0 is not declared by the fragment).
    static const std::pair<const char*, int> kVaryings[] = {
        { "in vec2 orig_FragNormCoord;", 1 },
        { "in vec2 orig_FragCoord;", 2 },
        { "in vec2 xl_FragNormCoord;", 3 },
        { "in vec2 xl_FragCoord;", 4 },
    };

    std::istringstream ss(code);
    std::string line;
    while (std::getline(ss, line)) {
        const std::string trimmed = ltrim(line);

        if (!versionSeen && startsWith(trimmed, "#version")) {
            versionSeen = true;
            continue; // drop; we emit "#version 450"
        }
        if (startsWith(trimmed, "#extension")) {
            extensions += line + "\n";
            continue;
        }
        if (startsWith(trimmed, "precision ")) {
            continue;  // GLES precision statements — Vulkan uses default precision
        }

        if (brace == 0 && startsWith(trimmed, "uniform ")) {
            std::string rest = ltrim(trimmed.substr(8)); // after "uniform "
            size_t sp = rest.find_first_of(" \t");
            if (sp != std::string::npos) {
                std::string type = rest.substr(0, sp);
                if (startsWith(type, "sampler")) {
                    // Keep the sampler as an opaque binding at set0/binding1
                    // (foundation's shaderSetLayout).  Only texSampler is
                    // expected for our shaders.
                    body += "layout(set=0, binding=1) " + trimmed + "\n";
                    continue;
                }
                std::string after = ltrim(rest.substr(sp));
                size_t semi = after.find(';');
                std::string name = rtrim(after.substr(0, (semi == std::string::npos) ? after.size() : semi));
                UBOMember m;
                if (!name.empty() && classifyType(type, m.vecSize, m.isFloat, m.align, m.size)) {
                    m.glslType = type;
                    m.name = name;
                    members.push_back(std::move(m));
                    continue; // remove loose uniform; folded into the UBO
                }
                // Unrecognized uniform type: fall through and keep verbatim
                // (translation will surface the problem and we fall back to GL).
            }
        }

        if (brace == 0) {
            bool handled = false;
            for (const auto& v : kVaryings) {
                if (trimmed == v.first) {
                    body += "layout(location=" + std::to_string(v.second) + ") " + trimmed + "\n";
                    handled = true;
                    break;
                }
            }
            if (handled) continue;
            if (trimmed == "out vec4 fragmentColor;") {
                body += "layout(location=0) out vec4 fragmentColor;\n";
                continue;
            }
        }

        body += line + "\n";
        for (char c : line) {
            if (c == '{') brace++;
            else if (c == '}') brace--;
        }
    }

    // "sampler" is a reserved type keyword in Vulkan GLSL (the separate-sampler
    // type), but the ISF IMG_* helper functions declare parameters named
    // `sampler` (legal in desktop/GLES GLSL) — a syntax error under
    // EShClientVulkan.  Rename the standalone identifier so glslang accepts it.
    // \bsampler\b never matches sampler2D / texSampler / isampler etc.
    body = std::regex_replace(body, std::regex("\\bsampler\\b"), "xlsampler");
    // Strip precision qualifiers (highp/mediump/lowp) — Vulkan doesn't need them
    // and mismatched qualifiers on overloaded ISF helpers are a hard error there.
    body = std::regex_replace(body, std::regex("\\b(highp|mediump|lowp)\\s+"), "");

    if (members.empty()) {
        return false;
    }

    const std::string block = emitUBOBlock(members);

    fragOut = "#version 450\n";
    fragOut += extensions;
    fragOut += block;
    fragOut += body;

    // Attribute-less fullscreen-triangle vertex stage: reproduces the same
    // varyings and XL_ZOOM_OFFSET math from gl_VertexIndex.  The UBO block is
    // declared identically (same name + members + order) for interface match.
    vertOut = "#version 450\n";
    vertOut += block;
    vertOut +=
        "layout(location=1) out vec2 orig_FragNormCoord;\n"
        "layout(location=2) out vec2 orig_FragCoord;\n"
        "layout(location=3) out vec2 xl_FragNormCoord;\n"
        "layout(location=4) out vec2 xl_FragCoord;\n"
        "vec2 XL_ZOOM_OFFSET(vec2 c){ return ((c - (XL_OFFSET - 0.5) - 0.5) / XL_ZOOM) + 0.5; }\n"
        "void main(){\n"
        "    vec2 p = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));\n"
        "    vec2 tpos = p;\n"
        "    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);\n"
        "    orig_FragNormCoord = tpos;\n"
        "    xl_FragNormCoord = XL_ZOOM_OFFSET(tpos);\n"
        "    orig_FragCoord = orig_FragNormCoord * RENDERSIZE;\n"
        "    xl_FragCoord = xl_FragNormCoord * RENDERSIZE;\n"
        "}\n";
    return true;
}

// Process-wide translated-program cache keyed by the assembled fragment source
// (config->GetCode()).  Translation + pipeline build is expensive; mirror the
// GL path's shaderMap and the Metal path's programCache.  Failures are cached
// too (ok=false) so a bad shader doesn't re-translate per buffer.
struct CachedProgram {
    bool ok = false;
    VkPipeline pipeline = VK_NULL_HANDLE;
    std::vector<UBOMember> members;
    uint32_t uboSize = 16;
};
static std::mutex sProgramCacheMutex;
static std::unordered_map<std::string, CachedProgram>& programCache() {
    static std::unordered_map<std::string, CachedProgram> cache;
    return cache;
}

static CachedProgram buildVulkanProgram(const std::string& code, const std::string& label) {
    CachedProgram out;
    std::vector<UBOMember> members;
    std::string fragGLSL, vertGLSL;
    if (!assembleVulkanGLSL(code, members, fragGLSL, vertGLSL)) {
        if (sDbg) fprintf(stderr, "VULKAN shader assemble-fail %s: no uniforms\n", label.c_str());
        return out;
    }
    computeStd140(members, out.uboSize);

    std::vector<uint32_t> vspv, fspv;
    std::string err;
    if (!VulkanShaderTranslate::ToSpirv(vertGLSL, VulkanShaderTranslate::Stage::Vertex, vspv, err)) {
        if (sDbg) fprintf(stderr, "VULKAN vtx xlate-fail %s: %s\n", label.c_str(), err.substr(0, 400).c_str());
        return out;
    }
    if (!VulkanShaderTranslate::ToSpirv(fragGLSL, VulkanShaderTranslate::Stage::Fragment, fspv, err)) {
        if (sDbg) fprintf(stderr, "VULKAN frag xlate-fail %s: %s\n", label.c_str(), err.substr(0, 400).c_str());
        return out;
    }

    VkPipelineLayout layout = VulkanGraphicsUtilities::INSTANCE.shaderPipelineLayout();
    if (layout == VK_NULL_HANDLE) {
        if (sDbg) fprintf(stderr, "VULKAN no shader pipeline layout %s\n", label.c_str());
        return out;
    }
    VkPipeline pipe = VulkanGraphicsUtilities::INSTANCE.buildPipeline(
        vspv.data(), vspv.size() * sizeof(uint32_t),
        fspv.data(), fspv.size() * sizeof(uint32_t), layout);
    if (pipe == VK_NULL_HANDLE) {
        if (sDbg) fprintf(stderr, "VULKAN pipeline build-fail %s\n", label.c_str());
        return out;
    }
    out.pipeline = pipe;
    out.members = std::move(members);
    out.ok = true;
    return out;
}

// Vulkan-specific per-buffer state; lifecycle/config/timeMS live in the
// shared SPIRVShaderEffect::CacheBase.
class VulkanShaderNativeCache : public SPIRVShaderEffect::CacheBase {
public:
    VulkanShaderNativeCache() {}
    virtual ~VulkanShaderNativeCache() {
        if (ubo) VulkanComputeUtilities::INSTANCE.destroyBuffer(ubo);
    }

    VkPipeline pipeline = VK_NULL_HANDLE;
    std::vector<UBOMember> members;
    uint32_t uboSize = 16;
    VulkanBuffer ubo{};

    virtual void platformReset() override {
        pipeline = VK_NULL_HANDLE; // owned by the process-wide programCache
        members.clear();
        uboSize = 16;
        if (ubo) VulkanComputeUtilities::INSTANCE.destroyBuffer(ubo);
        ubo = VulkanBuffer{};
    }

    bool build(RenderBuffer& buffer) {
        CachedProgram prog;
        {
            std::lock_guard<std::mutex> lock(sProgramCacheMutex);
            auto& cache = programCache();
            auto it = cache.find(transformedSource);
            if (it == cache.end()) {
                it = cache.emplace(transformedSource,
                                   buildVulkanProgram(transformedSource, config->GetFilename())).first;
            }
            prog = it->second;
        }
        if (!prog.ok) return false;
        pipeline = prog.pipeline;
        members = prog.members;
        uboSize = prog.uboSize;

        if (!ubo || ubo.size < uboSize) {
            if (ubo) VulkanComputeUtilities::INSTANCE.destroyBuffer(ubo);
            ubo = VulkanBuffer{};
            if (!VulkanComputeUtilities::INSTANCE.createSharedBuffer(ubo, uboSize, "ShaderUBO")) {
                return false;
            }
        }
        return true;
    }

    // Pack the computed uniform values into the mapped UBO per std140.  Float
    // members take float bits; int/bool members are lround'ed to int bits.
    // Members with no computed value stay zero.
    void packUniforms(const SPIRVShaderEffect::UniformValues& vals) {
        uint8_t* base = (uint8_t*)ubo.mapped;
        std::memset(base, 0, uboSize);
        for (const auto& m : members) {
            auto it = vals.find(m.name);
            if (it == vals.end()) continue;
            uint32_t tmp[4] = { 0, 0, 0, 0 };
            for (int i = 0; i < m.vecSize && i < 4; i++) {
                if (m.isFloat) {
                    float f = it->second[i];
                    std::memcpy(&tmp[i], &f, 4);
                } else {
                    int iv = (int)std::lround(it->second[i]);
                    std::memcpy(&tmp[i], &iv, 4);
                }
            }
            std::memcpy(base + m.offset, tmp, (size_t)m.vecSize * 4);
        }
    }
};

} // namespace

VulkanShaderEffect::VulkanShaderEffect(int i) :
    SPIRVShaderEffect(i) {
}

VulkanShaderEffect::~VulkanShaderEffect() {
}

bool VulkanShaderEffect::nativeAvailable() const {
    return VulkanGraphicsUtilities::INSTANCE.available();
}

SPIRVShaderEffect::CacheBase* VulkanShaderEffect::newCache() const {
    return new VulkanShaderNativeCache();
}

bool VulkanShaderEffect::nativeBuild(CacheBase* cache, RenderBuffer& buffer) {
    return static_cast<VulkanShaderNativeCache*>(cache)->build(buffer);
}

bool VulkanShaderEffect::nativeEncode(CacheBase* base, RenderBuffer& buffer,
                                      const UniformValues& vals, InputKind kind,
                                      const float* audio128) {
    auto* cache = static_cast<VulkanShaderNativeCache*>(base);
    cache->packUniforms(vals);

    // Input texture (set0/binding1): audio shaders sample the 128x1 float
    // FFT/intensity texture; everything else samples the buffer's own pixels
    // (previous frame's output / canvas), matching the GL and Metal paths.
    // On upload failure the foundation's 1x1 dummy stays bound.
    VkImageView inputView = VK_NULL_HANDLE;
    if (kind == InputKind::Audio && audio128 != nullptr) {
        inputView = VulkanGraphicsUtilities::INSTANCE.prepareInputImage(128, 1, VK_FORMAT_R32_SFLOAT, audio128, 128 * sizeof(float));
    } else {
        inputView = VulkanGraphicsUtilities::INSTANCE.prepareInputImage(
            (uint32_t)buffer.BufferWi, (uint32_t)buffer.BufferHt, VK_FORMAT_R8G8B8A8_UNORM,
            buffer.GetPixels(), (size_t)buffer.BufferWi * buffer.BufferHt * 4);
    }

    if (!VulkanGraphicsUtilities::INSTANCE.renderShader(buffer, cache->pipeline, cache->ubo.buffer, inputView)) {
        return false; // transient — the shared layer fills this frame yellow
    }

    if (sDbg && (buffer.curPeriod % 100) == 0) {
        spdlog::debug("VULKAN shader f={} {} {}x{} members={} ubo={}",
                      buffer.curPeriod, cache->shaderFile, buffer.BufferWi, buffer.BufferHt,
                      cache->members.size(), cache->uboSize);
    }
    return true;
}

#endif // HAVE_VULKAN
