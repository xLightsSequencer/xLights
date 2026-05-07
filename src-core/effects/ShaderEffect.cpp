/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/shader_64.xpm"
#include "../../include/shader_48.xpm"
#include "../../include/shader_32.xpm"
#include "../../include/shader_24.xpm"
#include "../../include/shader_16.xpm"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <spdlog/fmt/fmt.h>
#include <semaphore>
#include <sstream>

#ifndef __APPLE__
    #ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #endif
    #include <GL/gl.h>
    #include <GL/glext.h>

    #ifdef _WIN32
        extern PFNGLACTIVETEXTUREPROC glActiveTexture;
    #endif
    extern PFNGLGENBUFFERSPROC glGenBuffers;
    extern PFNGLBINDBUFFERPROC glBindBuffer;
    extern PFNGLBUFFERDATAPROC glBufferData;
    extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
    extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
    extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
    extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
    extern PFNGLUSEPROGRAMPROC glUseProgram;
    extern PFNGLISPROGRAMPROC glIsProgram;
    extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix4fv;
    extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
    extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
    extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
    extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
    extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
    extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    extern PFNGLUNIFORM1IPROC glUniform1i;
    extern PFNGLUNIFORM1FPROC glUniform1f;
    extern PFNGLUNIFORM2FPROC glUniform2f;
    extern PFNGLUNIFORM4FPROC glUniform4f;
#else
    #ifdef USE_GLES
        // ANGLE provides OpenGL ES 3.0 on top of Metal
        #define GL_GLES_PROTOTYPES 1
        #define EGL_EGL_PROTOTYPES 1
        #include <EGL/egl.h>
        #include <EGL/eglext.h>
        #include <EGL/eglext_angle.h>
        #include <GLES3/gl3.h>
    #elif !TARGET_OS_IPHONE
        #include "OpenGL/gl3.h"
        #define __gl_h_
        #include <OpenGL/OpenGL.h>

        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #endif

#endif

#include "ShaderEffect.h"
#include "media/AudioManager.h"
#include "../graphics/GLContextManager.h"
#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h"
#include "../render/SequenceMedia.h"
#include "../models/Model.h"
#include "UtilClasses.h"
#include "../render/RenderContext.h"
#include "OpenGLShaders.h"
#include "UtilFunctions.h"
#include "utils/ExternalHooks.h"
#include "../utils/FileUtils.h"
#include <nlohmann/json.hpp>

#include <regex>

#include <log.h>

#include <fstream>
#include <map>
#include <set>
#include <mutex>
#include <string>



namespace
{
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

    GLuint RenderBufferTexture(int w, int h)
    {
        GLuint texId = 0;

        LOG_GL_ERRORV(glGenTextures(1, &texId));
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, texId));

        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));

        return texId;
    }

    GLuint FFTAudioTexture()
    {
        GLuint texId = 0;

        LOG_GL_ERRORV(glGenTextures(1, &texId));
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, texId));

#ifdef USE_GLES
        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 128, 1, 0, GL_RED, GL_FLOAT, nullptr));
#else
        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 1, 0, GL_RED, GL_FLOAT, nullptr));
#endif

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));

        return texId;
    }

    bool createOpenGLRenderBuffer(int width, int height, GLuint* rbID, GLuint* fbID)
    {
        LOG_GL_ERRORV(glGenRenderbuffers(1, rbID));
        LOG_GL_ERRORV(glBindRenderbuffer(GL_RENDERBUFFER, *rbID));
#ifdef USE_GLES
        LOG_GL_ERRORV(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height));
#else
        LOG_GL_ERRORV(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height));
#endif

        LOG_GL_ERRORV(glGenFramebuffers(1, fbID));
        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, *fbID));
        glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);

        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        return *rbID != 0 && *fbID != 0;
    }

    const char* vsSrc =
#ifdef USE_GLES
        "#version 300 es\n"
        "precision highp float;\n"
#else
        "#version 330 core\n"
#endif
        "uniform vec2 RENDERSIZE;\n"
        "uniform vec2 XL_OFFSET;\n"
        "uniform float XL_ZOOM;\n"
        "uniform float XL_DURATION;\n"
        "in vec2 vpos;\n"
        "in vec2 tpos;\n"
        "out vec2 texCoord;\n"
        "out vec2 orig_FragNormCoord;\n"
        "out vec2 orig_FragCoord;\n"
        "out vec2 xl_FragNormCoord;\n"
        "out vec2 xl_FragCoord;\n"
        "vec2 XL_ZOOM_OFFSET(vec2 coord) {\n  return ((coord.xy - (XL_OFFSET - 0.5) - 0.5) / XL_ZOOM) + 0.5;\n}\n\n"
        "void isf_vertShaderInit(void)\n"
        "{\n"
        //"   gl_Position = ftransform();\n"
        "   gl_Position = vec4(vpos,0,1);\n"
        "   texCoord = tpos;\n"
        "   orig_FragNormCoord = vec2(tpos.x, tpos.y);\n"
        "   xl_FragNormCoord = XL_ZOOM_OFFSET(vec2(tpos.x, tpos.y));\n"
        "   orig_FragCoord = orig_FragNormCoord * RENDERSIZE;\n"
        "   xl_FragCoord = xl_FragNormCoord * RENDERSIZE;\n"
        "}\n"
        "void main(){\n"
        "    isf_vertShaderInit();"
        "}\n";

    void setRenderBufferAll(RenderBuffer& buffer, const xlColor& colour) {
        buffer.Fill(colour);
    }
}

void ShaderEffect::SetBackgroundRender(bool b) {
    GLContextManager::Instance().SetBackgroundRenderEnabled(b);
}

bool ShaderEffect::IsBackgroundRender() {
    return GLContextManager::Instance().IsBackgroundRenderEnabled();
}

bool ShaderEffect::IsShaderFile(std::string filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "fs") {
        return true;
    }

    return false;
}

std::list<std::string> ShaderEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    std::string ifsFilename = settings.Get("E_0FILEPICKERCTRL_IFS", "");

    if (ifsFilename.empty()) {
        res.push_back(fmt::format("    ERR: Shader effect cant find file '{}'. Model '{}', Start {}", ifsFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    } else {
        auto& mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        auto entry = mm.GetShader(ifsFilename);
        entry->MarkIsUsed();

        if (entry->GetShaderSource().empty()) {
            res.push_back(fmt::format("    ERR: Shader effect cant find file '{}'. Model '{}', Start {}", ifsFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else if (!entry->IsEmbedded()) {
            if (!FileUtils::IsFileInShowDir(std::string(), ifsFilename)) {
                res.push_back(fmt::format("    WARN: Shader effect file '{}' not under show directory. Model '{}', Start {}", ifsFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            }
        }
    }

    return res;
}

std::list<std::string> ShaderEffect::GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_0FILEPICKERCTRL_IFS"] != "") {
        res.push_back(SettingsMap["E_0FILEPICKERCTRL_IFS"]);
    }
    return res;
}

bool ShaderEffect::CleanupFileLocations(RenderContext* ctx, SettingsMap& SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_0FILEPICKERCTRL_IFS"];
    if (FileExists(file))
    {
        if (!ctx->IsInShowFolder(file))
        {
            SettingsMap["E_0FILEPICKERCTRL_IFS"] = ctx->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Shaders", true);
            rc = true;
        }
    }

    return rc;
}

ShaderConfig* ShaderEffect::ParseShaderFromSource(const std::string& filename, const std::string& source, SequenceElements* sequenceElements) {
    if (source.empty()) {
        return nullptr;
    }

    std::string code = source;
    if (code[0] == '{' && code[1] == '"') {
        nlohmann::json root = nlohmann::json::parse(code);
        if (root.contains("rawFragmentSource")) {
            code = root["rawFragmentSource"].get<std::string>();
            if (code.empty()) {
                return nullptr;
            }
        }
    }

    std::smatch match;
    std::regex re("\\/\\*([\\s\\S]*?)\\*\\/", std::regex_constants::ECMAScript);
    if (!std::regex_search(code, match, re)) {
        return nullptr;
    }
    return new ShaderConfig(filename, code, match[1].str(), sequenceElements);
}

ShaderConfig* ShaderEffect::ParseShader(const std::string& filename, SequenceElements* sequenceElements) {
    auto shader = sequenceElements->GetSequenceMedia().GetShader(filename);
    if (!shader) {
        return nullptr;
    }
    std::string code = shader->GetShaderSource();
    return ParseShaderFromSource(filename, code, sequenceElements);
}

bool ShaderEffect::needToAdjustSettings(const std::string& version)
{
    return true;
}

void ShaderEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap& settings = effect->GetSettings();

    // The way we used to do names allowed for potential settings name clashes ... this should minimise them
    std::list<std::pair<std::string, std::string>> renames;
    for (auto& it : settings.keys()) {
        if (it != "E_VALUECURVE_Shader_Zoom" &&
            it != "E_VALUECURVE_Shader_Offset_Y" &&
            it != "E_VALUECURVE_Shader_Speed" &&
            it != "E_TEXTCTRL_Shader_LeadIn" &&
            it != "E_0FILEPICKERCTRL_IFS" &&
            it != "E_SLIDER_Shader_Speed" &&
            it != "E_TEXTCTRL_Shader_Offset_X" &&
            it != "E_TEXTCTRL_Shader_Offset_Y" &&
            it != "E_TEXTCTRL_Shader_Zoom" &&
            it != "E_VALUECURVE_Shader_Offset_X"
           ) {
            if (StartsWith(it, "E_") && !Contains(it, "SHADERXYZZY")) {
                std::string undecorated = AfterFirst(it, '_');
                std::string name = AfterFirst(undecorated, '_');
                std::string prefix = it.substr(0, it.size() - name.size());
                renames.push_back({ it, prefix + "SHADERXYZZY_" + name });
            }
        }
    }
    for (const auto& it : renames) {
        settings[it.second] = settings[it.first];
        settings.erase(it.first);
    }

    // Resolve broken paths first, then convert to relative for portability
    std::string file = settings["E_0FILEPICKERCTRL_IFS"];
    if (!file.empty() && !FileExists(file)) {
        std::string fixed = FileUtils::FixFile("", file);
        if (!fixed.empty() && fixed != file) {
            settings["E_0FILEPICKERCTRL_IFS"] = fixed;
            file = fixed;
        }
    }
    if (!file.empty()) {
        if (std::filesystem::path(file).is_absolute()) {
            if (!FileExists(file, false)) {
                std::string fixed = FileUtils::FixFile("", file);
                std::string rel = FileUtils::MakeRelativeFile(fixed);
                settings["E_0FILEPICKERCTRL_IFS"] = rel.empty() ? fixed : rel;
            } else {
                std::string rel = FileUtils::MakeRelativeFile(file);
                if (!rel.empty())
                    settings["E_0FILEPICKERCTRL_IFS"] = rel;
            }
        }
        // Register with SequenceMedia so it appears in the Media tab
        auto& media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        media.GetShader(settings["E_0FILEPICKERCTRL_IFS"]);
    }
}

// Platform-specific GL context management moved to graphics/GLContextManager.cpp

#if defined(__APPLE__)
constexpr int COMPILED_PROGRAM_RETAIN_COUNT = 24;
#else
constexpr int COMPILED_PROGRAM_RETAIN_COUNT = 10;
#endif

class ShaderRenderCache : public EffectRenderCache {
public:
    class ShaderInfo {
        std::map<std::string, GLint> uniforms;

    public:
        std::list<unsigned> programIds;

        ShaderInfo(GLint pid) {
            LoadUniforms(pid);
        }

        inline bool SetUniformInt(const std::string &name, int v) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) { glUniform1i(loc, v); return true; }
            return false;
        }
        inline bool SetUniform1f(const std::string &name, float v) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) { glUniform1f(loc, v); return true; }
            return false;
        }
        inline bool SetUniform2f(const std::string &name, float v1, float v2) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) { glUniform2f(loc, v1, v2); return true; }
            return false;
        }
        inline bool SetUniform4f(const std::string &name, float v1, float v2, float v3, float v4) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) { glUniform4f(loc, v1, v2, v3, v4); return true; }
            return false;
        }
        inline bool HasUniform(const std::string &name) {
            return FindUniformLocation(name) != -1;
        }
    private:
        GLint FindUniformLocation(const std::string &name) const {
            const auto &a = uniforms.find(name);
            if (a != uniforms.end()) return a->second;
            return -1;
        }
        void LoadUniforms(GLint program) {
            int uniformCount = 0;
            glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
            char buf[256];
            for (int i = 0; i < uniformCount; i++) {
                int len = 0;
                GLint size;
                GLenum type;
                glGetActiveUniform(program, i, sizeof(buf), &len, &size, &type, buf);
                uniforms[buf] = glGetUniformLocation(program, buf);
            }
        }
    };

    static std::map<std::string, ShaderInfo*> shaderMap;
    static std::set<std::string> failedShaders;
    static std::mutex shaderMapMutex;

    ShaderRenderCache() { _shaderConfig = nullptr; }
    virtual ~ShaderRenderCache()
    {
        if (s_programId != 0 && _shaderConfig != nullptr) {
            std::unique_lock<std::mutex> lock(shaderMapMutex);
            shaderMap[s_code]->programIds.push_back(s_programId);
            s_programId = 0;
        }
        if (_shaderConfig != nullptr) delete _shaderConfig;

        if (contextHandle) {
            auto& mgr = GLContextManager::Instance();
            mgr.ExecuteOnGLThread([&]() {
                if (mgr.MakeCurrent(contextHandle)) {
                    DestroyResources();
                    mgr.DoneCurrent(contextHandle);
                }
                mgr.ReleaseContext(contextHandle);
                contextHandle = nullptr;
            });
        }
    }

    void SetProgramId(unsigned programId, ShaderInfo *si) {
        if (s_programId && s_shaderInfo) StoreProgramId();
        s_programId = programId;
        s_shaderInfo = si;
        if (_shaderConfig) s_code = _shaderConfig->GetCode();
    }
    unsigned RefreshProgramId() {
        if (s_shaderInfo) {
            std::unique_lock<std::mutex> lock(shaderMapMutex);
            if (s_shaderInfo->programIds.empty()) {
                lock.unlock();
                s_programId = ShaderEffect::programIdForShaderCode(_shaderConfig, this);
            } else {
                s_programId = s_shaderInfo->programIds.back();
                s_shaderInfo->programIds.pop_back();
            }
        }
        return s_programId;
    }
    void StoreProgramId() {
        if (s_programId && s_shaderInfo) {
            std::unique_lock<std::mutex> lock(shaderMapMutex);
            if (s_shaderInfo->programIds.size() > COMPILED_PROGRAM_RETAIN_COUNT) {
                glDeleteProgram(s_programId);
            } else {
                s_shaderInfo->programIds.push_back(s_programId);
            }
            s_programId = 0;
        }
    }

    ShaderConfig* _shaderConfig = nullptr;
    bool s_shadersInit = false;
    unsigned s_vertexArrayId = 0;
    unsigned s_vertexBufferId = 0;
    unsigned s_fbId = 0;
    unsigned s_rbId = 0;
    unsigned s_rbTex = 0;
    unsigned s_audioTex = 0;
    unsigned s_programId = 0;
    ShaderInfo *s_shaderInfo = nullptr;
    std::string s_code;
    int s_rbWidth = 0;
    int s_rbHeight = 0;
    long _timeMS = 0;
    GLContextManager::ContextHandle contextHandle = nullptr;

    void InitialiseShaderConfig(const std::string& filename, SequenceElements* sequenceElements) {
        if (_shaderConfig != nullptr) delete _shaderConfig;
        _shaderConfig = ShaderEffect::ParseShader(filename, sequenceElements);
        s_shaderInfo = nullptr;
    }

    void DestroyResources() {
        DestroyResources(s_vertexArrayId, s_vertexBufferId, s_fbId, s_rbId, s_rbTex, s_audioTex);
        s_programId = 0; s_vertexArrayId = 0; s_vertexBufferId = 0;
        s_fbId = 0; s_rbId = 0; s_rbTex = 0; s_audioTex = 0;
    }
    static void DestroyResources(unsigned vaId, unsigned vbId, unsigned fb, unsigned rb, unsigned rbTx, unsigned audTx) {
        if (vaId) { LOG_GL_ERRORV(glDeleteVertexArrays(1, &vaId)); }
        if (vbId) { LOG_GL_ERRORV(glDeleteBuffers(1, &vbId)); }
        if (fb) { LOG_GL_ERRORV(glDeleteFramebuffers(1, &fb)); }
        if (rb) { LOG_GL_ERRORV(glDeleteRenderbuffers(1, &rb)); }
        if (rbTx) { LOG_GL_ERRORV(glDeleteTextures(1, &rbTx)); }
        if (audTx) { LOG_GL_ERRORV(glDeleteTextures(1, &audTx)); }
    }
};
std::map<std::string, ShaderRenderCache::ShaderInfo*> ShaderRenderCache::shaderMap;
std::set<std::string> ShaderRenderCache::failedShaders;
std::mutex ShaderRenderCache::shaderMapMutex;

ShaderEffect::ShaderEffect(int i) : RenderableEffect(i, "Shader", shader_16_xpm, shader_24_xpm, shader_32_xpm, shader_48_xpm, shader_64_xpm)
{
}

ShaderEffect::~ShaderEffect()
{
}

bool ShaderEffect::SetGLContext(ShaderRenderCache *cache) {
    auto& mgr = GLContextManager::Instance();
    if (!cache->contextHandle) {
        cache->contextHandle = mgr.AcquireContext();
        if (!cache->contextHandle) return false;
    }
    if (!mgr.MakeCurrent(cache->contextHandle)) {
        mgr.ReleaseContext(cache->contextHandle);
        cache->contextHandle = nullptr;
        return false;
    }
    return true;
}

void ShaderEffect::UnsetGLContext(ShaderRenderCache* cache) {
    auto& mgr = GLContextManager::Instance();
    mgr.DoneCurrent(cache->contextHandle);
#if !defined(_WIN32)
    // macOS/Linux: return context to pool after each frame so other threads
    // can acquire it.  The per-thread context cache (contextHandle) is
    // re-acquired at the start of the next frame; pool contexts share no
    // state so GL resources (FBOs, textures) owned by the cache remain valid.
    mgr.ReleaseContext(cache->contextHandle);
    cache->contextHandle = nullptr;
#endif
    // Windows: keep context cached in ShaderRenderCache for reuse (pool grows on demand)
}

void ShaderEffect::Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    // All GL work runs through the GL thread.  On Windows that's a
    // dedicated worker thread internal to GLContextManager (so flaky
    // drivers see a stable single-thread caller); on other platforms
    // the lambda runs directly on this thread.  ExecuteOnGLThread
    // blocks until the lambda returns, so captures by reference are safe.
    GLContextManager::Instance().ExecuteOnGLThread([&]() {
    // Bail out right away if we don't have the necessary OpenGL support
    if (!OpenGLShaders::HasFramebufferObjects() || !OpenGLShaders::HasShaderSupport()) {
        setRenderBufferAll(buffer, xlCYAN);
        spdlog::error("ShaderEffect::Render() - missing OpenGL support!!");
        return;
    }

    // No shader file configured - render red just like video/pictures effect
    if (SettingsMap.Get("0FILEPICKERCTRL_IFS", "").empty()) {
        setRenderBufferAll(buffer, xlRED);
        return;
    }

    ShaderRenderCache* cache = (ShaderRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new ShaderRenderCache();
        buffer.infoCache[id] = cache;
    }

    // This object has all the data from the json in the .fs file
    ShaderConfig*& _shaderConfig = cache->_shaderConfig;
    bool& s_shadersInit = cache->s_shadersInit;
    unsigned& s_vertexArrayId = cache->s_vertexArrayId;
    unsigned& s_vertexBufferId = cache->s_vertexBufferId;
    unsigned& s_fbId = cache->s_fbId;
    unsigned& s_rbId = cache->s_rbId;
    unsigned& s_rbTex = cache->s_rbTex;
    unsigned& s_audioTex = cache->s_audioTex;
    int& s_rbWidth = cache->s_rbWidth;
    int& s_rbHeight = cache->s_rbHeight;
    long& _timeMS = cache->_timeMS;

    bool contextSet = SetGLContext(cache);

    float oset = buffer.GetEffectTimeIntervalPosition();
    double timeRate = GetValueCurveDouble("Shader_Speed", 100, SettingsMap, oset, SHADER_SPEED_MIN, SHADER_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;

    double offsetX = GetValueCurveInt("Shader_Offset_X", 0, SettingsMap, oset, SHADER_OFFSET_X_MIN, SHADER_OFFSET_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
    // -100 - 100 -> 0-1
    offsetX /= 200.0;
    offsetX += 0.5;
    double offsetY = GetValueCurveInt("Shader_Offset_Y", 0, SettingsMap, oset, SHADER_OFFSET_Y_MIN, SHADER_OFFSET_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
    offsetY /= 200.0;
    offsetY += 0.5;
    double zoom = GetValueCurveInt("Shader_Zoom", 0, SettingsMap, oset, SHADER_ZOOM_MIN, SHADER_ZOOM_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
    if (zoom < 0) {
        zoom = 1.0 - abs(zoom) / 100.0;
    }
    else if (zoom > 0) {
        zoom = 1.0 + (zoom * 9.0) / 100.0;
    }
    else     {
        zoom = 1.0;
    }

    unsigned programId = 0u;

    if (buffer.needToInit) {
        buffer.needToInit = false;
        _timeMS = SettingsMap.GetInt("TEXTCTRL_Shader_LeadIn", 0) * buffer.frameTimeInMs;
        if (contextSet) {
            cache->InitialiseShaderConfig(SettingsMap.Get("0FILEPICKERCTRL_IFS", ""), GetSequenceElements(buffer));
            if (_shaderConfig != nullptr) {
                programId = programIdForShaderCode(_shaderConfig, cache);
            }
        } else {
            spdlog::warn("Could not create/set OpenGL Context for ShaderEffect.  ShaderEffect disabled.");
        }
    } else {
        if (!contextSet) {
            setRenderBufferAll(buffer, xlYELLOW);
            return;
        }
        if (_shaderConfig != nullptr) {
            programId = cache->RefreshProgramId();
        }
        _timeMS += buffer.frameTimeInMs * timeRate;
    }

    ShaderRenderCache::ShaderInfo *si = cache->s_shaderInfo;
    // if there is no config then we should paint it red ... just like the video effect
    if (_shaderConfig == nullptr) {
        setRenderBufferAll(buffer, xlRED);
        UnsetGLContext(cache);
        return;
    } else if (programId == 0u || si == nullptr) {
        setRenderBufferAll(buffer, xlYELLOW);
        UnsetGLContext(cache);
        return;
    }

    // ***********************************************************************************************************
    // todo is there more of this code we could add to the needtoinit case as this only happens on the first frame
    // ***********************************************************************************************************

    // We re-use the same framebuffer for rendering all the shader effects
    sizeForRenderBuffer(buffer, s_shadersInit, s_vertexArrayId, s_vertexBufferId, s_rbId, s_fbId, s_rbTex, s_rbWidth, s_rbHeight);

    preparePixelTextures(buffer, s_shadersInit, s_fbId);

    LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, s_fbId));
    LOG_GL_ERRORV(glViewport(0, 0, buffer.BufferWi, buffer.BufferHt));

    LOG_GL_ERRORV(glClearColor(0.f, 0.f, 0.f, 0.f));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));

    if (_shaderConfig->IsAudioFFTShader() || _shaderConfig->IsAudioIntensityShader()) {
        if (s_audioTex == 0)
            s_audioTex = FFTAudioTexture();

        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0));
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, s_audioTex));

        AudioManager* audioManager = buffer.GetMedia();
        if (audioManager != nullptr) {
            auto fftData = audioManager->GetFrameData(buffer.curPeriod, "");
            if (fftData) {
                std::vector<float> fft128;
                if ( _shaderConfig->IsAudioFFTShader() )
                    fft128.insert( fft128.begin(), fftData->vu.cbegin(), fftData->vu.cend()  );
                else
                    fft128.insert( fft128.begin(), 127, fftData->max );
                fft128.push_back( 0.f );

                LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, fft128.size(),1, GL_RED, GL_FLOAT, fft128.data()));
            }
        }
    } else {
        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0));
        copyPixelDataToTexture(buffer, s_rbTex);
    }

    LOG_GL_ERRORV(glBindVertexArray(s_vertexArrayId));
    LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, s_vertexBufferId));
    LOG_GL_ERRORV(glUseProgram(programId));
    
    int colourIndex = 0;
    if (!si->SetUniform2f("RENDERSIZE", buffer.BufferWi, buffer.BufferHt)) {
        if (buffer.curPeriod == buffer.curEffStartPer && _shaderConfig->HasRendersize()) {
            spdlog::warn("Unable to bind to RENDERSIZE\n{}", (const char*)_shaderConfig->GetCode().c_str());
        }
    }
    if (!si->SetUniform2f("XL_OFFSET", offsetX, offsetY)) {
        spdlog::warn("Unable to bind to XL_OFFSET");
    }
    if (!si->SetUniform1f("XL_ZOOM", zoom)) {
        spdlog::warn("Unable to bind to XL_ZOOM");
    }
    if (!si->SetUniform1f("XL_DURATION", (GLfloat)((buffer.GetEndTimeMS() - buffer.GetStartTimeMS()) / 1000.0))) {
        // This may just have been optimized out of the shader program.  If it cannot be set, it is not worth logging.
        //spdlog::warn("Unable to bind to XL_DURATION");
    }
    if (!si->SetUniform1f("TIME", (GLfloat)(_timeMS) / 1000.0)) {
        if (buffer.curPeriod == buffer.curEffStartPer && _shaderConfig->HasTime()) {
            spdlog::warn("Unable to bind to TIME\n{}", (const char*)_shaderConfig->GetCode().c_str());
        }
    }
    si->SetUniform1f("TIMEDELTA", (GLfloat)(buffer.frameTimeInMs /1000.f));

    if (si->HasUniform("DATE")) {
        auto now = std::chrono::system_clock::now();
        std::time_t nowt = std::chrono::system_clock::to_time_t(now);
        std::tm tmbuf;
#ifdef _MSC_VER
        localtime_s(&tmbuf, &nowt);
#else
        localtime_r(&nowt, &tmbuf);
#endif
        si->SetUniform4f("DATE", tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday, tmbuf.tm_hour * 3600 + tmbuf.tm_min * 60 + tmbuf.tm_sec);
    }
    si->SetUniformInt("NUMCOLORS", buffer.GetColorCount());
    si->SetUniformInt("PASSINDEX", 0);
    si->SetUniformInt("FRAMEINDEX", _timeMS / buffer.frameTimeInMs);
    si->SetUniform1f("clearBuffer", SettingsMap.GetBool("CHECKBOX_OverlayBkg", false) ? 1.0 : 0.0);
    si->SetUniform1f("resetNow", (buffer.curPeriod == buffer.curEffStartPer) ? 1.0 : 0.0);
    si->SetUniformInt("texSampler", 0);

    for (const auto& it : _shaderConfig->GetParms())
    {
        if (si->HasUniform(it._name)) {
            switch (it._type)
            {
            case ShaderParmType::SHADER_PARM_FLOAT:
            {
                double f = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default * 100.0, SettingsMap, oset, it._min * 100.0, it._max * 100.0, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
                si->SetUniform1f(it._name, f);
                break;
            }
            case ShaderParmType::SHADER_PARM_POINT2D:
            {
                double x = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "X", it._defaultPt.x * 100, SettingsMap, oset, it._minPt.x * 100, it._maxPt.x * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
                double y = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "Y", it._defaultPt.y * 100, SettingsMap, oset, it._minPt.y * 100, it._maxPt.y * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
                si->SetUniform2f(it._name, x, y);
                break;
            }
            case ShaderParmType::SHADER_PARM_BOOL:
            {
                bool b = SettingsMap.GetBool(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHECKBOX));
                si->SetUniform1f(it._name, b);
                break;
            }
            case ShaderParmType::SHADER_PARM_EVENT:
            {
                auto timingtrack = SettingsMap.Get(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TIMING), "");

                EffectLayer* el = GetTiming(timingtrack);

                bool b = false;
                if (el != nullptr) {
                    int ms = buffer.curPeriod * buffer.frameTimeInMs;
                    for (int j = 0; j < el->GetEffectCount(); j++) {
                        int ems = el->GetEffect(j)->GetStartTimeMS();
                        if (ems == ms) {
                            b = true;
                            break;
                        }
                        else if (ems > ms) break;
                    }
                }

                si->SetUniform1f(it._name, b);
                break;
            }
            case ShaderParmType::SHADER_PARM_LONGCHOICE:
            {
                long l = it.EncodeChoice(SettingsMap[it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHOICE)]);
                si->SetUniformInt(it._name, l);
                break;
            }
            case ShaderParmType::SHADER_PARM_LONG:
            {
                long l = GetValueCurveInt(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default, SettingsMap, oset, it._min, it._max,
                    buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
                si->SetUniformInt(it._name, l);
                break;
            }
            case ShaderParmType::SHADER_PARM_COLOUR:
            {
                xlColor c = buffer.palette.GetColor(colourIndex);
                colourIndex++;
                if (colourIndex > (int)buffer.GetColorCount()) colourIndex = 0;
                si->SetUniform4f(it._name, (double)c.red / 255.0, (double)c.green / 255.0, (double)c.blue / 255.0, 1.0);
                break;
            }
            default:
                spdlog::warn("No binding supported for {} ... we have more work to do.", (const char*)it._name.c_str());
                break;
            }
        } else {
            if (buffer.curPeriod == buffer.curEffStartPer)
                spdlog::warn("Unable to bind to {}", (const char*)it._name.c_str());
        }
    }

    LOG_GL_ERRORV(GLuint vattrib = glGetAttribLocation(programId, "vpos"));
    LOG_GL_ERRORV(glVertexAttribPointer(vattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void*>(offsetof(VertexTex, v))));
    LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));

    LOG_GL_ERRORV(GLuint tattrib = glGetAttribLocation(programId, "tpos"));
    LOG_GL_ERRORV(glVertexAttribPointer(tattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void*>(offsetof(VertexTex, t))));
    LOG_GL_ERRORV(glEnableVertexAttribArray(tattrib));

    LOG_GL_ERRORV(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
    LOG_GL_ERRORV(glDisableVertexAttribArray(tattrib));

    LOG_GL_ERRORV(glBindVertexArray(0));
    LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));

    copyPixelDataFromTexture(buffer);

    LOG_GL_ERRORV(glUseProgram(0));
    cache->StoreProgramId();
    UnsetGLContext(cache);
    });
}

void ShaderEffect::preparePixelTextures(RenderBuffer& buffer, bool shadersInit, unsigned fbId) {
}

void ShaderEffect::copyPixelDataToTexture(RenderBuffer& buffer, unsigned rbTex) {
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, rbTex));
    LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, buffer.GetPixels()));
}

void ShaderEffect::copyPixelDataFromTexture(RenderBuffer& buffer) {
    LOG_GL_ERRORV(glReadPixels(0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, buffer.GetPixels()));
}



void ShaderEffect::sizeForRenderBuffer(const RenderBuffer& rb,
    bool& s_shadersInit,
    unsigned& s_vertexArrayId, unsigned& s_vertexBufferId, unsigned& s_rbId, unsigned& s_fbId,
    unsigned& s_rbTex, int& s_rbWidth, int& s_rbHeight)
{
    

    if (!s_shadersInit) {
        VertexTex vt[4] =
        {
           { {  1.f, -1.f }, { 1.f, 0.f } },
           { { -1.f, -1.f }, { 0.f, 0.f } },
           { {  1.f,  1.f }, { 1.f, 1.f } },
           { { -1.f,  1.f }, { 0.f, 1.f } }
        };
        LOG_GL_ERRORV(glGenVertexArrays(1, &s_vertexArrayId));
        LOG_GL_ERRORV(glGenBuffers(1, &s_vertexBufferId));

        LOG_GL_ERRORV(glBindVertexArray(s_vertexArrayId));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, s_vertexBufferId));
        LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTex[4]), vt, GL_STATIC_DRAW));

        LOG_GL_ERRORV(glBindVertexArray(0));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
           spdlog::error( "ShaderEffect::sizeForRenderBuffer() - Error with vertex array - {}", err );
        }

        createOpenGLRenderBuffer(rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId);
        if ((err = glGetError()) != GL_NO_ERROR) {
           spdlog::error( "ShaderEffect::sizeForRenderBuffer() - Error creating framebuffer - {}", err );
        }

        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);
        if ((err = glGetError()) != GL_NO_ERROR) {
           spdlog::error( "ShaderEffect::sizeForRenderBuffer() - Error creating renderbuffer texture - {}", err );
        }

        s_rbWidth = rb.BufferWi;
        s_rbHeight = rb.BufferHt;
        s_shadersInit = true;
    } else if (rb.BufferWi > s_rbWidth || rb.BufferHt > s_rbHeight) {
        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        if (s_fbId) {
            LOG_GL_ERRORV(glDeleteFramebuffers(1, &s_fbId));
        }
        if (s_rbId) {
            LOG_GL_ERRORV(glBindRenderbuffer(GL_RENDERBUFFER, 0));
            LOG_GL_ERRORV(glDeleteRenderbuffers(1, &s_rbId));
        }
        if (s_rbTex) {
            LOG_GL_ERRORV(glDeleteTextures(1, &s_rbTex));
        }
        createOpenGLRenderBuffer(rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
           spdlog::error( "ShaderEffect::sizeForRenderBuffer() - Error recreating framebuffer - {}", err );
        }
        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);;
        if ((err = glGetError()) != GL_NO_ERROR) {
           spdlog::error( "ShaderEffect::sizeForRenderBuffer() - Error recreating renderbuffer texture - {}", err );
        }

        s_rbWidth = rb.BufferWi;
        s_rbHeight = rb.BufferHt;
    }
}

unsigned ShaderEffect::programIdForShaderCode(ShaderConfig* cfg, ShaderRenderCache *cache)
{
    

    if (cfg == nullptr) {
        spdlog::error("ShaderEffect::programIdForShaderCode() - NULL ShaderConfig!");
        return 0u;
    }

    std::unique_lock<std::mutex> lock(ShaderRenderCache::shaderMapMutex);
    std::string fragmentShaderSrc(cfg->GetCode());
    if (ShaderRenderCache::failedShaders.find(fragmentShaderSrc) != ShaderRenderCache::failedShaders.end()) {
        //previously failed to compile, don't try again
        return 0u;
    }

    ShaderRenderCache::ShaderInfo *shaderInfo = nullptr;
    auto iter = ShaderRenderCache::shaderMap.find(fragmentShaderSrc);
    if (iter != ShaderRenderCache::shaderMap.cend()) {
        shaderInfo = (*iter).second;
        while (!shaderInfo->programIds.empty()) {
            unsigned programId = shaderInfo->programIds.front();
            shaderInfo->programIds.pop_front();
            if (!glIsProgram(programId)) {
                spdlog::error("ShaderEffect::programIdForShaderCode() - program id {} is not a shader program!", programId);
            } else {
                //spdlog::debug("ShaderEffect::programIdForShaderCode() - shader program {} unchanged -- id {}", (const char*)cfg->GetFilename().c_str(), programId);
                cache->SetProgramId(programId, shaderInfo);
                return programId;
            }
        }
    }

    lock.unlock();
    unsigned programId = OpenGLShaders::compile(vsSrc, fragmentShaderSrc, cfg->GetFilename());
    if (programId == 0u) {
        lock.lock();
        spdlog::error("ShaderEffect::programIdForShaderCode() - failed to compile shader program {}", (const char *)cfg->GetFilename().c_str());
        ShaderRenderCache::failedShaders.emplace(fragmentShaderSrc);
        lock.unlock();
    } else {
        spdlog::debug("ShaderEffect::programIdForShaderCode() - fragment shader {} compiled successfully", (const char*)cfg->GetFilename().c_str());
        if (shaderInfo == nullptr) {
            lock.lock();
            shaderInfo = ShaderRenderCache::shaderMap[fragmentShaderSrc];
            if (shaderInfo  == nullptr) {
                shaderInfo = new ShaderRenderCache::ShaderInfo(programId);
                ShaderRenderCache::shaderMap[fragmentShaderSrc] = shaderInfo;
            }
            lock.unlock();
        }
        cache->SetProgramId(programId, shaderInfo);
    }
    return programId;
}

std::string SafeFloat(const std::string& s)
{
    if (!s.empty() && s[0] == '.') {
        return "0" + s;
    } else if (s.find('.') == std::string::npos) {
        return s + ".0";
    }
    return s;
}

std::string SafeValueOption(std::string value)
{
    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
    return value;
}

ShaderConfig::ShaderConfig(const std::string& filename, const std::string& code, const std::string& json, SequenceElements* sequenceElements) :
    _filename(filename) {
    
    std::string canvasImgName;
    std::string audioFFTName;

    auto getNumberProperty = [](nlohmann::json const& item, std::string const& name, double defaultVal) {
        if (!item.contains(name) || item.at(name).is_null()) {
            return defaultVal;
        }
        if (item.at(name).is_number()) {
            return item.at(name).get<double>();
        }
        if (item.at(name).is_boolean()) {
            return static_cast<double>(item.at(name).get<bool>());
        }
        if (item.at(name).is_string()) {
            const auto& s = item.at(name).get<std::string>();
            char* end;
            double val = std::strtod(s.c_str(), &end);
            if (end != s.c_str()) {
                return val;
            }
            spdlog::warn("Error parsing shader Property : {} (not a number).", name);
        }
        return defaultVal;
    };

    auto getStringProperty = [](nlohmann::json const& item, std::string const& name, const std::string& defaultVal = "") {
        if (!item.contains(name) || item.at(name).is_null()) {
            return defaultVal;
        }
        if (item.at(name).is_string()) {
            return item.at(name).get<std::string>();
        }
        return defaultVal;
    };

    auto getPointProperty = [](nlohmann::json const& item, std::string const& name, double defaultX, double defaultY) {
        if (!item.contains(name) || item.at(name).is_null() || item.at(name).empty()) {
            return xlPointD(defaultX, defaultY);
        }
        const auto& arr = item.at(name);
        auto getComponent = [](const nlohmann::json& v, double def) {
            if (v.is_number()) {
                return v.get<double>();
            }
            if (v.is_string()) {
                char* end;
                double val = std::strtod(v.get<std::string>().c_str(), &end);
                if (end != v.get<std::string>().c_str()) {
                    return val;
                }
            }
            return def;
        };
        if (arr.size() >= 2) {
            defaultX = getComponent(arr[0], defaultX);
            defaultY = getComponent(arr[1], defaultY);
        }
        return xlPointD(defaultX, defaultY);
    };

    try {
        nlohmann::json root = nlohmann::json::parse(json,
                                                    nullptr,
                                                    true,    // allow_exceptions
                                                    true,    // ignore_comments
                                                    true,    // ignore_trailing_commas
                                                    true);   // ignore_missing_values
        if (root.contains("DESCRIPTION")) {
            _description = getStringProperty(root, "DESCRIPTION");
        }
        if (_description == "xLights AudioFFT") {
            _audioFFTMode = true;
        } else if (_description == "xLights Audio2") {
            _audioIntensityMode = true;
        }
        if (root.contains("INPUTS")) {
            const auto& inputs = root["INPUTS"];

            for (const auto& input : inputs) {
                std::string const name = getStringProperty(input, "NAME");

                // we ignore these as xlights provides these settings
                if (name == "XL_OFFSET" || name == "XL_DURATION" || name == "XL_ZOOM") {
                    continue;
                }

                std::string const type = getStringProperty(input, "TYPE");
                if (type == "float") {
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_FLOAT,
                        getNumberProperty(input, "MIN", 0.0),
                        getNumberProperty(input, "MAX", 1.0),
                        getNumberProperty(input, "DEFAULT", 0.0));
                } else if (type == "long") {
                    if (input.contains("MIN")) {
                        _parms.emplace_back(
                            name,
                            getStringProperty(input, "LABEL"),
                            ShaderParmType::SHADER_PARM_LONG,
                            getNumberProperty(input, "MIN", 0.0),
                            getNumberProperty(input, "MAX", 1.0),
                            getNumberProperty(input, "DEFAULT", 0.0));
                    } else if (input.contains("LABELS") && input.contains("VALUES")) {
                        _parms.emplace_back(
                            name,
                            getStringProperty(input, "LABEL"),
                            ShaderParmType::SHADER_PARM_LONGCHOICE,
                            0.0,
                            0.0,
                            getNumberProperty(input, "DEFAULT", 0.0));
                        const auto& ls = input["LABELS"];
                        const auto& vs = input["VALUES"];
                        int const no = std::min(ls.size(), vs.size());
                        for (int i = 0; i < static_cast<int>(no); i++) {
                            _parms.back()._valueOptions[vs[i].get<int>()] = SafeValueOption(ls[i].get<std::string>());
                        }
                    } else {
                        assert(false);
                    }
                } else if (type == "color") {
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_COLOUR);
                } else if (type == "audio") {
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_AUDIO);
                } else if (type == "bool") {
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_BOOL,
                        0.0,
                        0.0,
                        getNumberProperty(input, "DEFAULT", 0.0));
                } else if (type == "point2D") {
                    xlPointD const minPt = getPointProperty(input, "MIN", 0.0, 0.0);
                    xlPointD const maxPt = getPointProperty(input, "MAX", 1.0, 1.0);
                    xlPointD const defPt = getPointProperty(input, "DEFAULT", 0.0, 0.0);
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_POINT2D,
                        minPt,
                        maxPt,
                        defPt);
                } else if (type == "image") {
                    // ignore these as we will use the existing buffer content
                    if (!name.empty()) {
                        canvasImgName = name;
                    }
                } else if (type == "audioFFT") {
                    if (!name.empty()) {
                        audioFFTName = name;
                        spdlog::info("ShaderEffect - found audioFFT shader with name '{}'", audioFFTName.c_str());
                    }
                } else if (type == "text") {
                    // ignore these
                    if (!name.empty()) {
                        spdlog::warn("ShaderEffect - found text property with name '{}' ... ignored", name.c_str());
                    }
                } else if (type == "event") {
                    _parms.emplace_back(
                        name,
                        getStringProperty(input, "LABEL"),
                        ShaderParmType::SHADER_PARM_EVENT,
                        0.0,
                        0.0,
                        0.0);

                    // Add timing tracks
                    if (sequenceElements != nullptr) {
                        int tt = 0;
                        for (int i = 0; i < static_cast<int>(sequenceElements->GetElementCount()); i++) {
                            Element* e = sequenceElements->GetElement(i);
                            if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                                _parms.back()._valueOptions[tt++] = e->GetName();
                            }
                        }
                    }
                } else {
                    spdlog::warn("Unknown type parsing shader JSON : {}.", type.c_str());
                    assert(false);
                }
            }
            if (root.contains("PASSES")) {
                const auto& inputs2 = root["INPUTS"];
                const auto& passes = root["PASSES"];
                for (int i = 0; i < static_cast<int>(passes.size()); i++) {
                    _passes.push_back({ (i < static_cast<int>(inputs2.size()) && inputs2[i].contains("TARGET")) ? inputs2[i]["TARGET"].get<std::string>() : "",
                                        passes[i].contains("PERSISTENT") ? getStringProperty(passes[i], "PERSISTENT") == "true" : false });
                }
            }
        }
    } catch (const nlohmann::json::exception& e) {
        spdlog::warn("Error parsing shader JSON :  {} {}.", filename.c_str(), e.what());
    } catch (std::exception& ex) {
        spdlog::warn("Error parsing shader JSON :  {} {}.", filename.c_str(), ex.what());
    }

    // The shader code needs declarations for the uniforms that we silently set with each call to Render()
    // and the uniforms that correspond to user-visible settings
    std::string prependText =
        "uniform float TIME;\n"
        "uniform float TIMEDELTA;\n"
        "uniform vec2 RENDERSIZE;\n"
        "uniform bool clearBuffer;\n"
        "uniform bool resetNow;\n"
        "uniform int NUMCOLORS;\n"
        "uniform int PASSINDEX;\n"
        "uniform int FRAMEINDEX;\n"
        "uniform vec2 XL_OFFSET;\n"
        "uniform float XL_ZOOM;\n"
        "uniform float XL_DURATION;\n"
        "uniform sampler2D texSampler;\n\n"
        "// THESE ARE THE PRE ZOOM AND OFFSET COORDS\n"
        "in vec2 orig_FragNormCoord;\n"
        "in vec2 orig_FragCoord;\n"
        "// THESE ARE THE POST ZOOM AND OFFSET COORDS\n"
        "in vec2 xl_FragNormCoord;\n"
        "in vec2 xl_FragCoord;\n"
        "out vec4 fragmentColor;\n"
        "uniform vec4 DATE;\n\n"
        "// USE THIS IN PUBLIC SHADERS FOR CODE WHICH ONLY RUNS IN XLIGHTS\n"
        "#define XL_SHADER\n\n";

    for (const auto& p : _parms) {
        const std::string& name = p._name;
        switch (p._type) {
        case ShaderParmType::SHADER_PARM_FLOAT: {
            prependText += fmt::format("uniform float {};\n", name);
            break;
        }
        case ShaderParmType::SHADER_PARM_BOOL:
        case ShaderParmType::SHADER_PARM_EVENT: {
            prependText += fmt::format("uniform bool {};\n", name);
            break;
        }
        case ShaderParmType::SHADER_PARM_LONG:
        case ShaderParmType::SHADER_PARM_LONGCHOICE: {
            prependText += fmt::format("uniform int {};\n", name);
            break;
        }
        case ShaderParmType::SHADER_PARM_POINT2D: {
            prependText += fmt::format("uniform vec2 {};\n", name);
            break;
        }
        case ShaderParmType::SHADER_PARM_COLOUR: {
            prependText += fmt::format("uniform vec4 {};\n", name);
            break;
        }
        default: {
            // rest of these are un-implemented currently
        }
        }
    }

    prependText += "vec4 IMG_NORM_PIXEL_2D(sampler2D sampler, vec2 pct, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord* pct);\n}\n\n";
    prependText += "vec4 IMG_NORM_PIXEL(sampler2D sampler, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord);\n}\n\n";
    prependText += "vec4 IMG_PIXEL_2D(sampler2D sampler, vec2 pct, vec2 loc) {\n   return IMG_NORM_PIXEL_2D(sampler, pct, loc / RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_PIXEL(sampler2D sampler, vec2 loc) {\n   return texture(sampler, loc / RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_THIS_PIXEL(sampler2D sampler) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord);\n}\n\n";
    prependText += "vec4 IMG_THIS_NORM_PIXEL_2D(sampler2D sampler, vec2 pct) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord * pct);\n}\n\n";
    prependText += "vec4 IMG_THIS_NORM_PIXEL(sampler2D sampler) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord);\n}\n\n";
    prependText += "vec4 IMG_THIS_PIXEL_2D(sampler2D sampler, vec2 pct) {\n   return IMG_THIS_NORM_PIXEL_2D(sampler, pct);\n}\n\n";
#ifdef USE_GLES
    // OpenGL ES has no sampler2DRect; provide sampler2D equivalents so shaders
    // written for desktop GL still compile (they sample via normalized coords).
    prependText += "vec4 IMG_NORM_PIXEL_RECT(sampler2D sampler, vec2 pct, vec2 normLoc) {\n   return texture(sampler, normLoc);\n}\n\n";
    prependText += "vec4 IMG_PIXEL_RECT(sampler2D sampler, vec2 pct, vec2 loc) {\n   return texture(sampler, loc / RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_THIS_NORM_PIXEL_RECT(sampler2D sampler, vec2 pct) {\n   return texture(sampler, xl_FragNormCoord);\n}\n\n";
    prependText += "vec4 IMG_THIS_PIXEL_RECT(sampler2D sampler, vec2 pct) {\n   return texture(sampler, xl_FragNormCoord);\n}\n\n";
#else
    prependText += "vec4 IMG_NORM_PIXEL_RECT(sampler2DRect sampler, vec2 pct, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord * RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_PIXEL_RECT(sampler2DRect sampler, vec2 pct, vec2 loc) {\n   return IMG_NORM_PIXEL_RECT(sampler, pct, loc / RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_THIS_NORM_PIXEL_RECT(sampler2DRect sampler, vec2 pct) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord * RENDERSIZE);\n}\n\n";
    prependText += "vec4 IMG_THIS_PIXEL_RECT(sampler2DRect sampler, vec2 pct) {\n   return IMG_THIS_NORM_PIXEL_RECT(sampler, pct);\n}\n\n";
#endif
    prependText += "ivec2 IMG_SIZE(sampler2D sampler) {\n   return textureSize(sampler, 0);\n}\n\n";

#ifdef __DEBUG
    int i = 0;
    for (auto c : code) {
        if ((int)c < 32 || (int)c > 127) {
            if (c != 13 && c != 10 && c != 9)
                spdlog::debug("{} {:X} {}", i, (int)c, c);
            assert(false);
        }
        i++;
    }
#endif

    std::string shaderCode = code;
    for (char& c : shaderCode) {
        if ((unsigned char)c == 133) {
            c = '.';
        }
    }
    auto pos = shaderCode.find("*/");
    if (pos != std::string::npos && pos > 0) {
        shaderCode = shaderCode.substr(pos + 2);
    }
    Replace(shaderCode, "gl_FragColor", "fragmentColor");
    Replace(shaderCode, "vv_FragNormCoord", "xl_FragNormCoord");
    Replace(shaderCode, "isf_FragNormCoord", "xl_FragNormCoord");
    Replace(shaderCode, "isf_FragCoord", "xl_FragCoord");
    Replace(shaderCode, "gl_FragCoord", "xl_FragCoord");
    Replace(shaderCode, "gl_FragNormCoord", "xl_FragNormCoord");
    Replace(shaderCode, "varying ", "uniform ");
    Replace(shaderCode, "texture2D(", "texture(");
    Replace(shaderCode, "texture2D (", "texture(");
    if (!audioFFTName.empty()) {
        Replace(shaderCode, audioFFTName, "texSampler");
        _audioFFTMode = true;
    } else if (!canvasImgName.empty()) {
        Replace(shaderCode, canvasImgName, "texSampler");
        _canvasMode = true;
    }

    _hasRendersize = Contains(shaderCode, "RENDERSIZE");
    _hasTime = Contains(shaderCode, "TIME");
    _hasCoord = Contains(shaderCode, "xl_FragCoord");

#ifdef USE_GLES
    // GLSL ES 3.00 requires global variable initializers to be constant expressions.
    // Desktop GLSL 330 allows non-constant initializers (e.g. "float T = TIME*rate;").
    // Fix: split such declarations and defer their initialization into main().
    {
        std::set<std::string> uniformNames = {
            "TIME", "TIMEDELTA", "RENDERSIZE", "PASSINDEX", "FRAMEINDEX",
            "DATE", "NUMCOLORS", "clearBuffer", "resetNow", "texSampler",
            "XL_OFFSET", "XL_ZOOM", "XL_DURATION",
            "xl_FragNormCoord", "xl_FragCoord", "orig_FragNormCoord", "orig_FragCoord"
        };
        for (const auto& p : _parms) {
            uniformNames.insert(p._name);
        }

        // Join split declarations where the type is alone on one line:
        //   vec2\n    center = 0.5*RENDERSIZE.xy;  -->  vec2 center = 0.5*RENDERSIZE.xy;
        static const std::regex splitDeclRe(
            R"(\n(\s*)(float|int|bool|u?vec[234]|mat[234](?:x[234])?|ivec[234]|uvec[234])\s*\n(\s*\w+\s*=))",
            std::regex_constants::ECMAScript);
        shaderCode = std::regex_replace(shaderCode, splitDeclRe, "\n$1$2 $3");

        // Match global-scope type declarations with initializers:
        //   float foo = expr;  vec2 bar = expr;  etc.
        // Also handles comma-separated multi-variable declarations:
        //   float T = TIME * rate, moy = 0.0;
        // but NOT: const float foo = 1.0;  uniform float foo;  void main()  #define  //comment
        static const std::regex declRe(
            R"(^(\s*)(float|int|bool|u?vec[234]|mat[234](?:x[234])?|ivec[234]|uvec[234])\s+(.+?)\s*;\s*(?://.*)?\s*$)",
            std::regex_constants::ECMAScript);

        std::istringstream stream(shaderCode);
        std::string line;
        std::string newCode;
        std::string deferredInits;
        int braceDepth = 0;

        while (std::getline(stream, line)) {
            // Count braces to track scope (outside of strings/comments — good enough for most shaders)
            for (char c : line) {
                if (c == '{') braceDepth++;
                else if (c == '}') braceDepth--;
            }

            bool replaced = false;
            if (braceDepth == 0) {
                // At global scope — check for non-const variable decl with uniform-dependent initializer
                std::string trimmed = line;
                auto ws = trimmed.find_first_not_of(" \t");
                if (ws != std::string::npos) trimmed = trimmed.substr(ws);

                // Skip lines that can't be variable declarations
                if (!trimmed.empty() && trimmed[0] != '#' && trimmed[0] != '/' &&
                    trimmed.rfind("const ", 0) != 0 && trimmed.rfind("uniform ", 0) != 0 &&
                    trimmed.rfind("in ", 0) != 0 && trimmed.rfind("out ", 0) != 0 &&
                    trimmed.rfind("void ", 0) != 0 && trimmed.rfind("struct ", 0) != 0) {

                    std::smatch m;
                    if (std::regex_match(line, m, declRe)) {
                        std::string varList = m[3].str();
                        std::string indent = m[1].str();
                        std::string typeName = m[2].str();

                        // Split comma-separated variables at top-level commas only
                        // (skip commas inside parentheses from function calls)
                        std::vector<std::string> vars;
                        int parenDepth = 0;
                        size_t start = 0;
                        for (size_t i = 0; i <= varList.size(); ++i) {
                            if (i < varList.size()) {
                                if (varList[i] == '(') parenDepth++;
                                else if (varList[i] == ')') parenDepth--;
                                else if (varList[i] == ',' && parenDepth == 0) {
                                    vars.push_back(varList.substr(start, i - start));
                                    start = i + 1;
                                    continue;
                                }
                            } else {
                                vars.push_back(varList.substr(start));
                            }
                        }

                        // Check each variable — only defer those whose initializer references uniforms
                        bool anyDeferred = false;
                        std::string declLine;
                        for (auto& var : vars) {
                            // Trim whitespace
                            auto s = var.find_first_not_of(" \t");
                            if (s != std::string::npos) var = var.substr(s);
                            auto e = var.find_last_not_of(" \t");
                            if (e != std::string::npos) var = var.substr(0, e + 1);

                            auto eqPos = var.find('=');
                            if (eqPos == std::string::npos) {
                                // No initializer — keep as-is
                                if (!declLine.empty()) declLine += ", ";
                                declLine += var;
                                continue;
                            }

                            std::string varName = var.substr(0, eqPos);
                            auto ne = varName.find_last_not_of(" \t");
                            if (ne != std::string::npos) varName = varName.substr(0, ne + 1);

                            std::string initExpr = var.substr(eqPos + 1);
                            auto ns = initExpr.find_first_not_of(" \t");
                            if (ns != std::string::npos) initExpr = initExpr.substr(ns);

                            bool hasUniformRef = false;
                            for (const auto& u : uniformNames) {
                                size_t pos = 0;
                                while ((pos = initExpr.find(u, pos)) != std::string::npos) {
                                    bool leftOk = (pos == 0 || (!std::isalnum(initExpr[pos - 1]) && initExpr[pos - 1] != '_'));
                                    size_t uend = pos + u.size();
                                    bool rightOk = (uend >= initExpr.size() || (!std::isalnum(initExpr[uend]) && initExpr[uend] != '_'));
                                    if (leftOk && rightOk) { hasUniformRef = true; break; }
                                    pos = uend;
                                }
                                if (hasUniformRef) break;
                            }

                            if (hasUniformRef) {
                                // Emit declaration without initializer
                                if (!declLine.empty()) declLine += ", ";
                                declLine += varName;
                                deferredInits += "    " + varName + " = " + initExpr + ";\n";
                                anyDeferred = true;
                            } else {
                                // Keep initializer (it's constant)
                                if (!declLine.empty()) declLine += ", ";
                                declLine += var;
                            }
                        }

                        if (anyDeferred) {
                            newCode += indent + typeName + " " + declLine + ";\n";
                            replaced = true;
                        }
                    }
                }
            }

            if (!replaced) {
                newCode += line + "\n";
            }
        }

        // Inject deferred initializations at the start of main()
        if (!deferredInits.empty()) {
            // Find "void main()" and inject after the opening brace
            auto mainPos = newCode.find("void main()");
            if (mainPos == std::string::npos) mainPos = newCode.find("void main(void)");
            if (mainPos == std::string::npos) mainPos = newCode.find("void main ()");
            if (mainPos != std::string::npos) {
                auto bracePos = newCode.find('{', mainPos);
                if (bracePos != std::string::npos) {
                    newCode.insert(bracePos + 1, "\n    // [ES compat] deferred global initializers\n" + deferredInits);
                }
            }
        }

        shaderCode = newCode;
    }

    _code = "#version 300 es\nprecision highp float;\nprecision mediump sampler2D;\n\n";
#else
    _code = "#version 330\n\n";
#endif
    size_t idx = shaderCode.find("#extension");
    if (idx != std::string::npos) {
        size_t nidx = shaderCode.find("\n", idx);
        _code += shaderCode.substr(0, nidx);
        _code += "\n";
        shaderCode = shaderCode.substr(nidx);
    }
    _code += prependText;
    _code += shaderCode;
    assert(_code != "");
#if 0
    std::ofstream s("C:\\Temp\\shader.txt");
    if (s.good())
    {
        s << _code;
        s.close();
    }
#endif
}

bool ShaderConfig::UsesEvents() const
{
    return std::any_of(_parms.begin(), _parms.end(),
                       [](const ShaderParm& p) { return p._type == ShaderParmType::SHADER_PARM_EVENT; });
}

nlohmann::json ShaderConfig::GetDynamicPropertiesJson() const
{
    // Panel-builder settingPrefix decides which control owns the serialized
    // value. Shader floats/longs historically stored the primary under
    // E_SLIDER_SHADERXYZZY_<name> with 100× scaling for floats; keeping that
    // exact key is what makes old .xsq sequences still load. Choices serialize
    // as label strings under E_CHOICE_; bools under E_CHECKBOX_. Point2d
    // expands into two sibling slider rows suffixed X / Y, matching the
    // legacy ShaderPanel layout and the render path in
    // ShaderEffect::Render (which appends "X"/"Y" to the VC-undecorated id).
    nlohmann::json out = nlohmann::json::array();

    for (const auto& p : _parms) {
        if (!p.ShowParm())
            continue;

        // Namespace the id to match the legacy SHADERXYZZY_<name> setting-map
        // keys that the render code reads via GetUndecoratedId.
        const std::string id = "SHADERXYZZY_" + p._name;

        nlohmann::json entry;
        entry["id"] = id;
        entry["label"] = p.GetLabel();

        switch (p._type) {
        case ShaderParmType::SHADER_PARM_FLOAT:
            entry["type"] = "float";
            entry["controlType"] = "slider";
            entry["settingPrefix"] = "SLIDER";
            entry["divisor"] = 100;
            entry["min"] = static_cast<int>(p._min * 100.0);
            entry["max"] = static_cast<int>(p._max * 100.0);
            entry["default"] = p._default;
            entry["valueCurve"] = true;
            break;

        case ShaderParmType::SHADER_PARM_LONG:
            entry["type"] = "int";
            entry["controlType"] = "slider";
            entry["min"] = static_cast<int>(p._min);
            entry["max"] = static_cast<int>(p._max);
            entry["default"] = static_cast<int>(p._default);
            entry["valueCurve"] = true;
            break;

        case ShaderParmType::SHADER_PARM_LONGCHOICE: {
            entry["type"] = "enum";
            entry["controlType"] = "choice";
            nlohmann::json options = nlohmann::json::array();
            std::string defaultLabel;
            const int defaultIdx = static_cast<int>(p._default);
            for (const auto& [idx, label] : p._valueOptions) {
                options.push_back(label);
                if (idx == defaultIdx)
                    defaultLabel = label;
            }
            entry["options"] = std::move(options);
            if (!defaultLabel.empty())
                entry["default"] = defaultLabel;
            break;
        }

        case ShaderParmType::SHADER_PARM_EVENT: {
            // Event params are populated with timing-track names at parse time
            // (see ShaderConfig ctor's SHADER_PARM_EVENT branch). Emit the
            // current list as a plain choice — render reads it as a string
            // name and matches against timing tracks.
            entry["type"] = "enum";
            entry["controlType"] = "choice";
            nlohmann::json options = nlohmann::json::array();
            std::string defaultLabel;
            const int defaultIdx = static_cast<int>(p._default);
            for (const auto& [idx, label] : p._valueOptions) {
                options.push_back(label);
                if (idx == defaultIdx)
                    defaultLabel = label;
            }
            entry["options"] = std::move(options);
            if (!defaultLabel.empty())
                entry["default"] = defaultLabel;
            break;
        }

        case ShaderParmType::SHADER_PARM_BOOL:
            entry["type"] = "bool";
            entry["controlType"] = "checkbox";
            entry["default"] = (p._default != 0.0);
            break;

        case ShaderParmType::SHADER_PARM_POINT2D:
            entry["type"] = "float";
            entry["controlType"] = "point2d";
            entry["settingPrefix"] = "SLIDER";
            entry["divisor"] = 100;
            entry["minX"] = static_cast<int>(p._minPt.x * 100.0);
            entry["maxX"] = static_cast<int>(p._maxPt.x * 100.0);
            entry["defaultX"] = p._defaultPt.x;
            entry["minY"] = static_cast<int>(p._minPt.y * 100.0);
            entry["maxY"] = static_cast<int>(p._maxPt.y * 100.0);
            entry["defaultY"] = p._defaultPt.y;
            entry["valueCurve"] = true;
            break;

        default:
            continue;
        }

        out.push_back(std::move(entry));
    }

    return out;
}

#if defined(__APPLE__) && !defined(USE_GLES)
#pragma clang diagnostic pop
#endif
