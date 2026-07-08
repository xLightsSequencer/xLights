/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MetalShaderTranslator.h"

#import <Metal/Metal.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <spirv-tools/optimizer.hpp>
#include <spirv_cross/spirv_msl.hpp>

#include <cctype>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace {

// glslang requires one process-wide InitializeProcess(); its parser also uses
// thread-local pools. The render engine calls translation from many worker
// threads, but only ever once per unique shader (results are cached upstream),
// so serializing the whole GLSL->MSL path behind one mutex is both correct and
// free of any real throughput cost.
std::mutex sTranslateMutex;

void ensureGlslangInit() {
    static const bool inited = [] {
        glslang::InitializeProcess();
        return true;
    }();
    (void)inited;
}

bool glslToSpirv(const std::string& src, EShLanguage stage, std::vector<uint32_t>& out, std::string& err) {
    glslang::TShader shader(stage);
    const char* s = src.c_str();
    shader.setStrings(&s, 1);
    // OpenGL semantics (matches `glslangValidator -G`) with auto-mapped uniform
    // locations/bindings (--aml/--amb) — xLights emits loose default-block
    // uniforms, which the SPIR-V path otherwise rejects.
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientOpenGL, 330);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
    shader.setAutoMapLocations(true);
    shader.setAutoMapBindings(true);
    const TBuiltInResource* res = GetDefaultResources();
    if (!shader.parse(res, 330, false, EShMsgDefault)) {
        err = shader.getInfoLog();
        return false;
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault) || !program.mapIO()) {
        err = program.getInfoLog();
        return false;
    }
    glslang::GlslangToSpv(*program.getIntermediate(stage), out);
    return !out.empty();
}

// Inline every helper function into the entry point. This removes the MSL
// address-space reference mismatches spirv-cross would otherwise emit (a
// `constant` uniform passed to a `thread const&` helper parameter). merge-return
// first, because the exhaustive inliner skips functions whose `return` is not at
// the end (early-return helpers) — that was the last few percent in the spike.
bool inlineAll(const std::vector<uint32_t>& in, std::vector<uint32_t>& out, std::string& err) {
    spvtools::Optimizer opt(SPV_ENV_UNIVERSAL_1_3);
    opt.SetMessageConsumer([&](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
        if (m) err += m;
    });
    opt.RegisterPass(spvtools::CreateMergeReturnPass());
    opt.RegisterPass(spvtools::CreateInlineExhaustivePass());
    return opt.Run(in.data(), in.size(), &out);
}

// GL drivers commonly zero-init shader locals, and real-world ISF shaders rely
// on that luck (e.g. `float i,g,d=1.;` then reading i/g). Metal gives garbage
// instead -> NaN -> black. Emit explicit zero initializers so the accidental GL
// behavior becomes deterministic here (and on any future backend).
static void setCommonOptions(spirv_cross::CompilerGLSL& comp) {
    spirv_cross::CompilerGLSL::Options g = comp.get_common_options();
    g.force_zero_initialized_variables = true;
    comp.set_common_options(g);
}

bool spirvToMsl(const std::vector<uint32_t>& spirv, bool forIOS, std::string& msl, std::string& err) {
    try {
        spirv_cross::CompilerMSL comp(spirv);
        spirv_cross::CompilerMSL::Options o;
        o.platform = forIOS ? spirv_cross::CompilerMSL::Options::iOS
                            : spirv_cross::CompilerMSL::Options::macOS;
        comp.set_msl_options(o);
        setCommonOptions(comp);
        msl = comp.compile();
        return !msl.empty();
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
}

} // namespace

namespace ShaderTranslate {

std::string ToMSL(const std::string& glsl, Stage stage, bool forIOS, std::string& error) {
    std::lock_guard<std::mutex> lock(sTranslateMutex);
    ensureGlslangInit();

    const EShLanguage esStage = (stage == Stage::Vertex) ? EShLangVertex : EShLangFragment;
    std::vector<uint32_t> spirv, inlined;
    if (!glslToSpirv(glsl, esStage, spirv, error)) {
        return {};
    }
    if (!inlineAll(spirv, inlined, error)) {
        return {};
    }
    std::string msl;
    if (!spirvToMsl(inlined, forIOS, msl, error)) {
        return {};
    }
    return msl;
}

// Pin the fragment stage's varying inputs to the same explicit locations the
// native vertex shader declares, so the separately-compiled MSL stages agree on
// the interface (spirv-cross renumbers per-stage otherwise, since the fragment
// drops unused varyings, which mis-wires the coordinates -> black output).
static std::string pinFragmentVaryings(std::string fs) {
    // Enable explicit varying locations in the (330) fragment shader.
    size_t nl = fs.find('\n');
    if (fs.rfind("#version", 0) == 0 && nl != std::string::npos) {
        fs.insert(nl + 1, "#extension GL_ARB_separate_shader_objects : enable\n");
    }
    static const char* const names[] = { "orig_FragNormCoord", "orig_FragCoord",
                                         "xl_FragNormCoord", "xl_FragCoord", "texCoord" };
    static const int locs[] = { 1, 2, 3, 4, 0 };
    for (int i = 0; i < 5; i++) {
        std::string pat = std::string("in vec2 ") + names[i] + ";";
        std::string rep = "layout(location=" + std::to_string(locs[i]) + ") in vec2 " + names[i] + ";";
        size_t p = fs.find(pat);
        if (p != std::string::npos) fs.replace(p, pat.size(), rep);
    }
    return fs;
}

// Compile a vertex+fragment GLSL pair as ONE linked glslang program so their
// varying locations match across stages, returning per-stage SPIR-V.
static bool glslProgramToSpirv(const std::string& vsrc, const std::string& fsrc,
                               std::vector<uint32_t>& vspv, std::vector<uint32_t>& fspv, std::string& err) {
    glslang::TShader vert(EShLangVertex), frag(EShLangFragment);
    const char* vs = vsrc.c_str(); vert.setStrings(&vs, 1);
    const char* fs = fsrc.c_str(); frag.setStrings(&fs, 1);
    for (glslang::TShader* sh : { &vert, &frag }) {
        const EShLanguage st = (sh == &vert) ? EShLangVertex : EShLangFragment;
        sh->setEnvInput(glslang::EShSourceGlsl, st, glslang::EShClientOpenGL, 330);
        sh->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        sh->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
        sh->setAutoMapLocations(true);
        sh->setAutoMapBindings(true);
    }
    const TBuiltInResource* res = GetDefaultResources();
    if (!vert.parse(res, 330, false, EShMsgDefault)) { err = std::string("vertex: ") + vert.getInfoLog(); return false; }
    if (!frag.parse(res, 330, false, EShMsgDefault)) { err = std::string("fragment: ") + frag.getInfoLog(); return false; }
    glslang::TProgram program;
    program.addShader(&vert);
    program.addShader(&frag);
    if (!program.link(EShMsgDefault) || !program.mapIO()) { err = program.getInfoLog(); return false; }
    glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), vspv);
    glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), fspv);
    return !vspv.empty() && !fspv.empty();
}

// `origNames` maps SPIR-V variable id -> the shader's ORIGINAL uniform name,
// captured before compile(): spirv-cross renames identifiers that collide with
// MSL keywords (e.g. `level` -> `level0`) during compilation, and the host binds
// by the ISF name.
static ShaderStageInfo reflectStage(spirv_cross::CompilerMSL& comp, bool isVertex,
                                    const std::unordered_map<uint32_t, std::string>& origNames) {
    ShaderStageInfo info;
    spirv_cross::ShaderResources res = comp.get_shader_resources();
    for (auto& u : res.gl_plain_uniforms) {
        uint32_t idx = comp.get_automatic_msl_resource_binding(u.id);
        if (idx == uint32_t(-1)) continue;
        const spirv_cross::SPIRType& t = comp.get_type(u.base_type_id);
        ShaderBinding b;
        auto it = origNames.find(u.id);
        b.name = (it != origNames.end()) ? it->second : u.name;
        b.index = idx;
        b.vecSize = (uint8_t)t.vecsize;
        b.isFloat = (t.basetype == spirv_cross::SPIRType::Float || t.basetype == spirv_cross::SPIRType::Half);
        info.uniforms.push_back(b);
    }
    for (auto& s : res.sampled_images) {
        uint32_t idx = comp.get_automatic_msl_resource_binding(s.id);
        if (idx != uint32_t(-1)) info.samplerTexture = int(idx);
    }
    if (isVertex) {
        for (auto& in : res.stage_inputs) {
            uint32_t loc = comp.get_decoration(in.id, spv::DecorationLocation);
            if (in.name == "vpos") info.attrVpos = int(loc);
            else if (in.name == "tpos") info.attrTpos = int(loc);
        }
    }
    return info;
}

TranslatedProgram TranslateProgram(const std::string& vertexGLSL, const std::string& fragmentGLSL, bool forIOS) {
    std::lock_guard<std::mutex> lock(sTranslateMutex);
    ensureGlslangInit();
    TranslatedProgram out;
    std::vector<uint32_t> vspv, fspv, vopt, fopt;
    if (!glslProgramToSpirv(vertexGLSL, pinFragmentVaryings(fragmentGLSL), vspv, fspv, out.error)) return out;
    if (!inlineAll(vspv, vopt, out.error) || !inlineAll(fspv, fopt, out.error)) return out;
    try {
        spirv_cross::CompilerMSL vc(vopt), fc(fopt);
        spirv_cross::CompilerMSL::Options o;
        o.platform = forIOS ? spirv_cross::CompilerMSL::Options::iOS : spirv_cross::CompilerMSL::Options::macOS;
        vc.set_msl_options(o);
        fc.set_msl_options(o);
        setCommonOptions(vc);
        setCommonOptions(fc);
        // Capture original uniform names before compile() renames MSL-keyword
        // collisions (level -> level0, ...).
        auto captureNames = [](spirv_cross::CompilerMSL& c) {
            std::unordered_map<uint32_t, std::string> names;
            for (auto& u : c.get_shader_resources().gl_plain_uniforms) {
                names[u.id] = u.name;
            }
            return names;
        };
        auto vNames = captureNames(vc);
        auto fNames = captureNames(fc);
        out.vertexMSL = vc.compile();
        out.fragmentMSL = fc.compile();
        out.vertex = reflectStage(vc, true, vNames);
        out.fragment = reflectStage(fc, false, fNames);
        out.ok = !out.vertexMSL.empty() && !out.fragmentMSL.empty();
    } catch (const std::exception& e) {
        out.error = e.what();
    }
    return out;
}

bool ValidateRenderPipeline(const std::string& vertexGLSL, const std::string& fragmentGLSL, std::string& error) {
    TranslatedProgram p = TranslateProgram(vertexGLSL, fragmentGLSL, false);
    if (!p.ok) { error = p.error; return false; }
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) { error = "no Metal device"; return false; }
        NSError* e = nil;
        id<MTLLibrary> vlib = [device newLibraryWithSource:[NSString stringWithUTF8String:p.vertexMSL.c_str()] options:nil error:&e];
        if (vlib == nil) { error = std::string("vertex lib: ") + (e ? e.localizedDescription.UTF8String : "?"); return false; }
        id<MTLLibrary> flib = [device newLibraryWithSource:[NSString stringWithUTF8String:p.fragmentMSL.c_str()] options:nil error:&e];
        if (flib == nil) { error = std::string("fragment lib: ") + (e ? e.localizedDescription.UTF8String : "?"); return false; }
        id<MTLFunction> vfn = [vlib newFunctionWithName:@"main0"];
        id<MTLFunction> ffn = [flib newFunctionWithName:@"main0"];
        if (vfn == nil || ffn == nil) { error = "missing main0 entry point"; return false; }

        MTLRenderPipelineDescriptor* rpd = [[MTLRenderPipelineDescriptor alloc] init];
        rpd.vertexFunction = vfn;
        rpd.fragmentFunction = ffn;
        rpd.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;

        // Vertex data comes from a buffer at a high slot so it can't collide with
        // the low uniform [[buffer(N)]] indices spirv-cross assigns.
        const NSUInteger kVertexBufferSlot = 30;
        MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
        if (p.vertex.attrVpos >= 0) {
            vd.attributes[p.vertex.attrVpos].format = MTLVertexFormatFloat2;
            vd.attributes[p.vertex.attrVpos].offset = 0;
            vd.attributes[p.vertex.attrVpos].bufferIndex = kVertexBufferSlot;
        }
        if (p.vertex.attrTpos >= 0) {
            vd.attributes[p.vertex.attrTpos].format = MTLVertexFormatFloat2;
            vd.attributes[p.vertex.attrTpos].offset = 8;
            vd.attributes[p.vertex.attrTpos].bufferIndex = kVertexBufferSlot;
        }
        vd.layouts[kVertexBufferSlot].stride = 16;
        vd.layouts[kVertexBufferSlot].stepFunction = MTLVertexStepFunctionPerVertex;
        rpd.vertexDescriptor = vd;

        id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:rpd error:&e];
        if (pso == nil) { error = std::string("pipeline: ") + (e ? e.localizedDescription.UTF8String : "?"); return false; }
    }
    return true;
}

bool TranslateAndCompile(const std::string& glslFragment, std::string& error) {
    const std::string msl = ToMSL(glslFragment, Stage::Fragment, false, error);
    if (msl.empty()) {
        return false;
    }
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) {
            error = "no Metal device";
            return false;
        }
        NSError* nsErr = nil;
        id<MTLLibrary> lib = [device newLibraryWithSource:[NSString stringWithUTF8String:msl.c_str()]
                                                  options:nil
                                                    error:&nsErr];
        if (lib == nil) {
            error = nsErr ? nsErr.localizedDescription.UTF8String : "MSL compile failed";
            return false;
        }
    }
    return true;
}

} // namespace ShaderTranslate
