/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/shader_64.xpm"
#include "../../include/shader_48.xpm"
#include "../../include/shader_32.xpm"
#include "../../include/shader_24.xpm"
#include "../../include/shader_16.xpm"
#include <wx/wx.h>
#include <wx/config.h>

#ifndef __WXOSX__
    #include <GL/gl.h>
    #ifdef _MSC_VER
        #include "graphics\opengl\GL\glext.h"
    #else
        #include <GL/glext.h>
    #endif

    #ifdef __WXMSW__
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
    #include "OpenGL/gl3.h"
    #define __gl_h_
    #include <OpenGL/OpenGL.h>
#endif

#include "ShaderEffect.h"
#include "ShaderPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../TimingPanel.h"
#include "OpenGLShaders.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"
#include "../../xSchedule/wxJSON/jsonreader.h"
#include "graphics/opengl/DrawGLUtils.h"

#include <wx/regex.h>

#include <log4cpp/Category.hh>

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

        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 1, 0, GL_RED, GL_FLOAT, nullptr));

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
        LOG_GL_ERRORV(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height));

        LOG_GL_ERRORV(glGenFramebuffers(1, fbID));
        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, *fbID));
        glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);

        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        return *rbID != 0 && *fbID != 0;
    }

    const char* vsSrc =
        "#version 330 core\n"
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

bool ShaderEffect::useBackgroundRender = false;

ShaderEffect::ShaderEffect(int i) : RenderableEffect(i, "Shader", shader_16_xpm, shader_24_xpm, shader_32_xpm, shader_48_xpm, shader_64_xpm)
{

}

ShaderEffect::~ShaderEffect()
{

}

bool ShaderEffect::IsShaderFile(std::string filename)
{
    wxFileName fn(filename);
    auto ext = fn.GetExt().Lower().ToStdString();

    if (ext == "fs") {
        return true;
    }

    return false;
}

xlEffectPanel *ShaderEffect::CreatePanel(wxWindow *parent)
{
    return new ShaderPanel(parent);
}

std::list<std::string> ShaderEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString ifsFilename = settings.Get("E_0FILEPICKERCTRL_IFS", "");

    if (ifsFilename == "" || !FileExists(ifsFilename))
    {
        res.push_back(wxString::Format("    ERR: Shader effect cant find file '%s'. Model '%s', Start %s", ifsFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (!IsFileInShowDir(xLightsFrame::CurrentDir, ifsFilename.ToStdString()))
    {
        res.push_back(wxString::Format("    WARN: Shader effect file '%s' not under show directory. Model '%s', Start %s", ifsFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
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

bool ShaderEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap)
{
    bool rc = false;
    wxString file = SettingsMap["E_0FILEPICKERCTRL_IFS"];
    if (FileExists(file))
    {
        if (!frame->IsInShowFolder(file))
        {
            SettingsMap["E_0FILEPICKERCTRL_IFS"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()) + "Shaders");
            rc = true;
        }
    }

    return rc;
}

ShaderConfig* ShaderEffect::ParseShader(const std::string& filename, SequenceElements* sequenceElements)
{
    if (!FileExists(filename)) return nullptr;

    wxFile f(filename);
    if (!f.IsOpened()) return nullptr;

    wxString code;
    f.ReadAll(&code);
    f.Close();

    if (code == "") return nullptr;
    
    if (code[0] == '{' && code[1] == '"') {
        wxJSONReader reader;
        wxJSONValue root;
        reader.Parse(code, &root);
        if (root.HasMember("rawFragmentSource")) {
            code = root["rawFragmentSource"].AsString();
            if (code == "") return nullptr;
        }
    }

    wxRegEx re("\\/\\*(.*?)\\*\\/", wxRE_ADVANCED);
    if (!re.Matches(code)) return nullptr;
    return new ShaderConfig(filename, code, re.GetMatch(code, 1), sequenceElements);
}

void ShaderEffect::SetDefaultParameters()
{
    ShaderPanel* fp = (ShaderPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Shader_Speed->SetActive(false);
    fp->BitmapButton_Shader_Offset_X->SetActive(false);
    fp->BitmapButton_Shader_Offset_Y->SetActive(false);
    fp->BitmapButton_Shader_Zoom->SetActive(false);

    SetSliderValue(fp->Slider_Shader_LeadIn, 0);
    SetSliderValue(fp->Slider_Shader_Speed, 100);
    fp->FilePickerCtrl1->SetFileName(wxFileName());
    SetSliderValue(fp->Slider_Shader_Offset_X, 0);
    SetSliderValue(fp->Slider_Shader_Offset_Y, 0);
    SetSliderValue(fp->Slider_Shader_Zoom, 0);
    
    if (fp->_shaderConfig != nullptr) {
        for (const auto& it : fp->_shaderConfig->GetParms()) {
            if (it.ShowParm()) {
                if (it._type == ShaderParmType::SHADER_PARM_POINT2D) {
                    auto id = it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "X";
                    wxWindow *c = fp->FindWindow(id);
                    if (c != nullptr) {
                        BulkEditValueCurveButton *vcb = dynamic_cast<BulkEditValueCurveButton*>(c);
                        vcb->SetActive(false);
                    }
                    id = it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "Y";
                    c = fp->FindWindow(id);
                    if (c != nullptr) {
                        BulkEditValueCurveButton *vcb = dynamic_cast<BulkEditValueCurveButton*>(c);
                        vcb->SetActive(false);
                    }
                } else {
                    auto id = it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE);
                    wxWindow *c = fp->FindWindow(id);
                    if (c != nullptr) {
                        BulkEditValueCurveButton *vcb = dynamic_cast<BulkEditValueCurveButton*>(c);
                        vcb->SetActive(false);
                    }
                }
            }
        }
    }
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

    std::string file = settings["E_0FILEPICKERCTRL_IFS"];
    if (file != "") {
        if (!FileExists(file)) {
            settings["E_0FILEPICKERCTRL_IFS"] = FixFile("", file);
        }
    }

    // The way we used to do names allowed for potential settings name clashes ... this should minimise them
    std::list<std::pair<std::string, std::string>> renames;
    for (auto& it : settings) {
        if (it.first != "E_VALUECURVE_Shader_Zoom" &&
            it.first != "E_VALUECURVE_Shader_Offset_Y" &&
            it.first != "E_VALUECURVE_Shader_Speed" &&
            it.first != "E_TEXTCTRL_Shader_LeadIn" &&
            it.first != "E_0FILEPICKERCTRL_IFS" &&
            it.first != "E_SLIDER_Shader_Speed" &&
            it.first != "E_TEXTCTRL_Shader_Offset_X" &&
            it.first != "E_TEXTCTRL_Shader_Offset_Y" &&
            it.first != "E_TEXTCTRL_Shader_Zoom" &&
            it.first != "E_VALUECURVE_Shader_Offset_X"
           ) {
            if (StartsWith(it.first, "E_") && !Contains(it.first, "SHADERXYZZY")) {
                std::string undecorated = AfterFirst(it.first, '_');
                std::string name = AfterFirst(undecorated, '_');
                std::string prefix = it.first.substr(0, it.first.size() - name.size());
                renames.push_back({ it.first, prefix + "SHADERXYZZY_" + name });
            }
        }
    }
    for (const auto& it : renames) {
        settings[it.second] = settings[it.first];
        settings.erase(it.first);
    }
}

void ShaderEffect::RemoveDefaults(const std::string &version, Effect *effect)
{
    RenderableEffect::RemoveDefaults(version, effect);
}

#ifdef __WXMSW__
typedef HGLRC(WINAPI * wglCreateContextAttribsARB_t)
(HDC hDC, HGLRC hShareContext, const int *attribList);
template <typename T>
inline T wxWGLProcCast(PROC proc)
{
    return reinterpret_cast<T>(proc);
}
#define wxDEFINE_WGL_FUNC(name) \
name##_t name = wxWGLProcCast<name##_t>(wglGetProcAddress(#name))

static wxGLAttributes GetShaderAttributes() {
    wxGLAttributes atts;
    atts.Reset();
    // we don't need a depth buffer or double buffering
    atts.PlatformDefaults();
    atts.RGBA()
        .EndList();
    return atts;
}



class ShaderGLCanvas : public xlGLCanvas {
public:
    ShaderGLCanvas(wxWindow *parent)
    : xlGLCanvas(parent, GetShaderAttributes(), "ShaderEffects") {}
    virtual ~ShaderGLCanvas() {}

    virtual void InitializeGLContext() {}
};

class GLContextInfo {
public:
    GLContextInfo(xlGLCanvas* win) : _canvas(nullptr), _context(nullptr) {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        //we need a valid context to get the ARB
        win->SetCurrentGLContext();
        wxDEFINE_WGL_FUNC(wglCreateContextAttribsARB);
        wglMakeCurrent(win->GetHDC(), nullptr);

        wxGLContext *sharedContext = win->GetSharedContext();
        HGLRC shared = sharedContext ? sharedContext->GetGLRC() : 0;

        // create a new window and new HDC specifically for this context, we
        // won't display this anywhere, but it's required to get the hardware accelerated
        // contexts and such.
        _canvas = new ShaderGLCanvas(win->GetParent());
        _hdc = _canvas->GetHDC();

        // we *should* need core profile/3.3... but seems to work withe 3.1 for Windows??
        wxGLContextAttrs cxtAttrs;
        cxtAttrs.PlatformDefaults().OGLVersion(3, 3).CoreProfile().EndList();
        _context = wglCreateContextAttribsARB(_hdc, shared, cxtAttrs.GetGLAttrs());
        if (_context == NULL) {
           wxGLContextAttrs newAttrs;
           newAttrs.PlatformDefaults().OGLVersion(3, 1).CoreProfile().EndList();
           _context = wglCreateContextAttribsARB(_hdc, shared, newAttrs.GetGLAttrs());
        }
        logger_opengl.debug("ShaderEffect Thread %d created open gl context 0x%llx.", wxThread::GetCurrentId(), (uint64_t)_context);

        //now unset this as current on the main thread
        UnsetCurrent();
    }
    ~GLContextInfo() {
        wglDeleteContext(_context);
        // by the time we get here it seems to be destroyed ... possibly because its parent has been destroyed
        //delete _canvas;
    }
    void SetCurrent() {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        for (int x = 0; x < 10; x++) {
            if (wglMakeCurrent(_hdc, _context)) {
                logger_opengl.debug("ShaderEffect Thread %d given open gl context 0x%llx.", wxThread::GetCurrentId(), (uint64_t)_context);
                return;
            }
            wxMilliSleep(1);
        }
        wxASSERT(false);
        logger_opengl.error("ShaderEffect unable to give thread %d open gl context 0x%llx.", wxThread::GetCurrentId(), (uint64_t)_context);
    }
    void UnsetCurrent() {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        for (int x = 0; x < 10; x++) {
            if (wglMakeCurrent(_hdc, nullptr))
            {
                logger_opengl.debug("ShaderEffect Thread %d has no current GL Context.", wxThread::GetCurrentId());
                return;
            }
            wxMilliSleep(1);
        }
        wxASSERT(false);
        logger_opengl.error("ShaderEffect Thread %d tried to set no current GL Context but failed.", wxThread::GetCurrentId());
    }

    HGLRC _context;
    HDC _hdc;
    xlGLCanvas *_canvas;
};

class GLContextPool {
public:

    GLContextPool() {
    }
    ~GLContextPool() {
        while (!contexts.empty()) {
            GLContextInfo *ret = contexts.front();
            delete ret;
            contexts.pop();
        }
    }

    GLContextInfo *GetContext(xlGLCanvas *parent) {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        // This seems odd but manually releasing the lock causes hard crashes on Visual Studio
        bool contextsEmpty = false;
        {
            std::unique_lock<std::mutex> locker(lock);
            contextsEmpty = contexts.empty();
        }

        if (contextsEmpty) {
            return create(parent);
        }

        {
            std::unique_lock<std::mutex> locker(lock);
            GLContextInfo *ret = contexts.front();
            contexts.pop();
            logger_opengl.debug("Shader opengl context taken from pool 0x%llx", (uint64_t)ret);
            return ret;
        }
    }
    void ReleaseContext(GLContextInfo *pctx) {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        std::unique_lock<std::mutex> locker(lock);
        contexts.push(pctx);
        logger_opengl.debug("Shader opengl context released 0x%llx", (uint64_t)pctx);
    }

    GLContextInfo *create(xlGLCanvas *canv) {
        std::mutex mtx;
        std::condition_variable signal;
        std::unique_lock<std::mutex> lck(mtx);
        GLContextInfo *tdc = nullptr;
        if (ShaderEffect::IsBackgroundRender()) {
            canv->CallAfter([&mtx, &signal, &tdc, canv]() {
                std::unique_lock<std::mutex> lck(mtx);
                tdc = new GLContextInfo(canv);
                lck.unlock();
                signal.notify_all();
            });
            signal.wait(lck, [&tdc] {return tdc != nullptr;});
        } else {
            tdc = new GLContextInfo(canv);
        }
        return tdc;
    }
private:
    std::mutex lock;
    std::queue<GLContextInfo*> contexts;
} GL_CONTEXT_POOL;
#endif /* __WXMSW__*/


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
            if (loc != -1) {
                glUniform1i(loc, v);
                return true;
            }
            return false;
        }
        inline bool SetUniform1f(const std::string &name, float v) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) {
                glUniform1f(loc, v);
                return true;
            }
            return false;
        }
        inline bool SetUniform2f(const std::string &name, float v1, float v2) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) {
                glUniform2f(loc, v1, v2);
                return true;
            }
            return false;
        }
        inline bool SetUniform4f(const std::string &name, float v1, float v2, float v3, float v4) const {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) {
                glUniform4f(loc, v1, v2, v3, v4);
                return true;
            }
            return false;
        }
        inline bool HasUniform(const std::string &name) {
            GLint loc = FindUniformLocation(name);
            if (loc != -1) {
                return true;
            }
            return false;
        }
    private:
        GLint FindUniformLocation(const std::string &name) const {
            const auto &a = uniforms.find(name);
            if (a != uniforms.end()) {
                return a->second;
            }
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

                int location = glGetUniformLocation(program, buf);
                uniforms[buf] = location;
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
#if defined(__WXOSX__)
        if (s_glContext) {
            WXGLSetCurrentContext(s_glContext);
            DestroyResources();
            WXGLUnsetCurrentContext();
            WXGLDestroyContext(s_glContext);
        }
#elif defined(__WXMSW__)
        if (glContextInfo) {
            glContextInfo->SetCurrent();
            DestroyResources();
            glContextInfo->UnsetCurrent();
            if (ShaderEffect::IsBackgroundRender()) {
                GL_CONTEXT_POOL.ReleaseContext(glContextInfo);
            }
        }
#else
        if (preview) {
            unsigned vertexArrayId = s_vertexArrayId;
            unsigned vertexBufferId = s_vertexBufferId;
            unsigned fbId = s_fbId;
            unsigned rbId = s_rbId;
            unsigned rbTex = s_rbTex;
            unsigned audioTex = s_audioTex;
            xlGLCanvas *preview = this->preview;

            preview->CallAfter([preview,
                                vertexArrayId,
                                vertexBufferId,
                                fbId,
                                rbId,
                                rbTex,
                                audioTex] {
                preview->SetCurrentGLContext();
                DestroyResources(vertexArrayId,
                                 vertexBufferId,
                                 fbId,
                                 rbId,
                                 rbTex,
                                 audioTex);
            });

            s_programId = 0;
            s_vertexArrayId = 0;
            s_vertexBufferId = 0;
            s_fbId = 0;
            s_rbId = 0;
            s_rbTex = 0;
        }
#endif
    }

    void SetProgramId(unsigned programId, ShaderInfo *si) {
        if (s_programId && s_shaderInfo) {
            std::unique_lock<std::mutex> lock(shaderMapMutex);
            // we'll keep 10 of them around.  We can always re-compile if we need more later
            // but this keeps object retention count a bit bounded
            if (s_shaderInfo->programIds.size() > 10) {
                glDeleteProgram(s_programId);
            } else {
                s_shaderInfo->programIds.push_back(s_programId);
            }
            s_programId = 0;
        }
        s_programId = programId;
        s_shaderInfo = si;
        if (_shaderConfig) {
            s_code = _shaderConfig->GetCode();
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

    void InitialiseShaderConfig(const wxString& filename, SequenceElements* sequenceElements) {
        if (_shaderConfig != nullptr) delete _shaderConfig;
        _shaderConfig = ShaderEffect::ParseShader(filename, sequenceElements);
        s_shaderInfo = nullptr;
    }

    void DestroyResources() {
        DestroyResources(s_vertexArrayId,
                         s_vertexBufferId,
                         s_fbId,
                         s_rbId,
                         s_rbTex,
                         s_audioTex);
        s_programId = 0;
        s_vertexArrayId = 0;
        s_vertexBufferId = 0;
        s_fbId = 0;
        s_rbId = 0;
        s_rbTex = 0;
        s_audioTex = 0;
    }
    static void DestroyResources(unsigned s_vertexArrayId,
                                 unsigned s_vertexBufferId,
                                 unsigned s_fbId,
                                 unsigned s_rbId,
                                 unsigned s_rbTex,
                                 unsigned s_audioTex) {
        if (s_vertexArrayId) {
            LOG_GL_ERRORV(glDeleteVertexArrays(1, &s_vertexArrayId));
        }
        if (s_vertexBufferId) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &s_vertexBufferId));
        }
        if (s_fbId) {
            LOG_GL_ERRORV(glDeleteFramebuffers(1, &s_fbId));
        }
        if (s_rbId) {
            LOG_GL_ERRORV(glDeleteRenderbuffers(1, &s_rbId));
        }
        if (s_rbTex) {
            LOG_GL_ERRORV(glDeleteTextures(1, &s_rbTex));
        }
        if (s_audioTex) {
            LOG_GL_ERRORV(glDeleteTextures(1, &s_audioTex));
        }
    }

#if defined(__WXOSX__)
    WXGLContext s_glContext = nullptr;
#elif defined(__WXMSW__)
    GLContextInfo *glContextInfo = nullptr;
#else
    xlGLCanvas *preview;
#endif
};
std::map<std::string, ShaderRenderCache::ShaderInfo*> ShaderRenderCache::shaderMap;
std::set<std::string> ShaderRenderCache::failedShaders;
std::mutex ShaderRenderCache::shaderMapMutex;

bool ShaderEffect::CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
{

#if defined(__WXOSX__)
    // if we create a specific OpenGL context for this thread and not try to share contexts between threads,
    // the OSX GL engine is thread safe.
    //
    // on windows, we need to create the GL contexts on the main thread, but then can use them
    // on the background thread.  Similar to the Path and text drawing contexts
    return true;
#elif defined(__WXMSW__)
    return useBackgroundRender;
#else
    return false;
#endif
}

void ShaderEffect::UnsetGLContext(ShaderRenderCache* cache) {
#if defined(__WXOSX__)
    WXGLUnsetCurrentContext();
#elif defined(__WXMSW__)
    if (cache->glContextInfo != nullptr) {
        // release it from the thread every time so we never find ourselves in a situation where it has not been released by a thread
        cache->glContextInfo->UnsetCurrent();
    }
#endif
}

bool ShaderEffect::SetGLContext(ShaderRenderCache *cache) {
#if defined(__WXOSX__)
    if (cache->s_glContext == nullptr) {
        wxGLAttributes attributes;
        attributes.AddAttribute(51 /* NSOpenGLPFAMinimumPolicy */ );
        attributes.AddAttribute(96 /* NSOpenGLPFAAcceleratedCompute */ );
        attributes.AddAttribute(97 /* NSOpenGLPFAAllowOfflineRenderers */ );
        attributes.MinRGBA(8, 8, 8, 8).EndList();
        wxGLContextAttrs cxtAttrs;
        cxtAttrs.CoreProfile().EndList();

        WXGLPixelFormat pixelFormat = WXGLChoosePixelFormat(attributes.GetGLAttrs(),
                                                            attributes.GetSize(),
                                                            cxtAttrs.GetGLAttrs(),
                                                            cxtAttrs.GetSize());
        if (!pixelFormat) {
            // couldn't get an AcceleratedCompute format, let's at least try defaults
            wxGLAttributes attributes2;
            attributes2.PlatformDefaults().MinRGBA(8, 8, 8, 8).EndList();
            pixelFormat = WXGLChoosePixelFormat(attributes2.GetGLAttrs(),
                                                attributes2.GetSize(),
                                                cxtAttrs.GetGLAttrs(),
                                                cxtAttrs.GetSize());
        }
        if (pixelFormat) {
            static std::mutex sharedContextLock;
            static WXGLContext sharedContext = 0;

            std::unique_lock<std::mutex> lock(sharedContextLock);
            if (sharedContext == 0) {
                sharedContext = WXGLCreateContext(pixelFormat, 0);
            }
            lock.unlock();
            cache->s_glContext = WXGLCreateContext(pixelFormat, sharedContext);

            WXGLDestroyPixelFormat(pixelFormat);
            if (!cache->s_glContext) {
                return false;
            }

            struct GLRendererInfo {
              GLint rendererID;       // RendererID number
              GLint accelerated;      // Whether Hardware accelerated
              GLint online;           // Whether renderer (/GPU) is onilne. Second GPU on Mac Pro is offline
              GLint virtualScreen;    // Virtual screen number
              GLint videoMemoryMB;
              GLint textureMemoryMB;
              GLint eGpu;
              const GLubyte *vendor;
            };
            WXGLSetCurrentContext(cache->s_glContext);

            // Grab the GLFW context and pixel format for future calls
            CGLContextObj contextObject = CGLGetCurrentContext();
            CGLPixelFormatObj pixel_format = CGLGetPixelFormat(contextObject);

            // Number of renderers
            CGLRendererInfoObj rend;
            GLint nRenderers = 0;
            CGLQueryRendererInfo(0xffffffff, &rend, &nRenderers);

            // Number of virtual screens
            GLint nVirtualScreens = 0;
            CGLDescribePixelFormat(pixel_format, 0, kCGLPFAVirtualScreenCount, &nVirtualScreens);

            int maxMem = 0;

            // Get renderer information
            std::vector<GLRendererInfo> Renderers(nRenderers);
            for (GLint i = 0; i < nRenderers; ++i) {
                CGLDescribeRenderer(rend, i, kCGLRPOnline, &(Renderers[i].online));
                CGLDescribeRenderer(rend, i, kCGLRPAcceleratedCompute, &(Renderers[i].accelerated));
                CGLDescribeRenderer(rend, i, kCGLRPRendererID,  &(Renderers[i].rendererID));
                CGLDescribeRenderer(rend, i, kCGLRPVideoMemoryMegabytes, &(Renderers[i].videoMemoryMB));
                CGLDescribeRenderer(rend, i, kCGLRPTextureMemoryMegabytes, &(Renderers[i].textureMemoryMB));
                CGLDescribeRenderer(rend, i, (CGLRendererProperty)142, &(Renderers[i].eGpu));
            }

            // Get corresponding virtual screen
            for (GLint i = 0; i != nVirtualScreens; ++i) {
                CGLSetVirtualScreen(contextObject, i);
                GLint r;
                CGLGetParameter(contextObject, kCGLCPCurrentRendererID, &r);

                for (GLint j = 0; j < nRenderers; ++j) {
                    if (Renderers[j].rendererID == r) {
                        Renderers[j].virtualScreen = i;
                        Renderers[j].vendor = glGetString(GL_VENDOR);
                    }
                }
            }

            // Print out information of renderers
            bool found = false;
            //printf("No. renderers: %d\n", nRenderers);
            //printf(" No. virtual screens: %d\n", nVirtualScreens);
            for (GLint i = 0; i < nRenderers; ++i) {
                /*
                printf("Renderer: %d\n", i);
                printf(" Virtual Screen: %d\n", Renderers[i].virtualScreen);
                printf(" Renderer ID: %d\n", Renderers[i].rendererID);
                printf(" Vendor: %s\n", Renderers[i].vendor);
                printf(" Accelerated: %d\n", Renderers[i].accelerated);
                printf(" Online: %d\n", Renderers[i].online);
                printf(" Video Memory MB: %d\n", Renderers[i].videoMemoryMB);
                printf(" Texture Memory MB: %d\n", Renderers[i].textureMemoryMB);
                printf(" eGpu: %d\n", Renderers[i].eGpu);
                */
                if (Renderers[i].eGpu) {
                    //prefer an eGPU if we find one
                    CGLSetVirtualScreen(contextObject, Renderers[i].virtualScreen);
                    found = true;
                } else {
                    // otherwise, use the one with the most video memory
                    // as we'll assume that's the best option
                    maxMem = std::max(maxMem, Renderers[i].videoMemoryMB);
                }
            }

            // Set the context to our desired virtual screen (and therefore OpenGL renderer)
            if (!found) {
                for (GLint i = 0; i < nRenderers; ++i) {
                    if (maxMem == Renderers[i].videoMemoryMB) {
                        CGLSetVirtualScreen(contextObject, Renderers[i].virtualScreen);
                    }
                }
            }
            
            const GLubyte* str = glGetString(GL_VERSION);
            const GLubyte* rendn = glGetString(GL_RENDERER);
            const GLubyte* vend = glGetString(GL_VENDOR);
            wxString configs = wxString::Format("ShaderEffect - glVer:  %s  (%s)(%s)",
                                                (const char *)str,
                                                (const char *)rendn,
                                                (const char *)vend);

            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info(configs);
            //printf("%s\n", (const char *)configs.c_str());
        } else {
            return false;
        }
    }
    WXGLSetCurrentContext(cache->s_glContext);
    return true;
#elif defined(__WXMSW__)
    ShaderPanel *p = (ShaderPanel *)panel;
    if (!ShaderEffect::IsBackgroundRender()) {
        p->_preview->SetCurrentGLContext();
    } else if (cache->glContextInfo == nullptr) {
        // we grab it here and release it when the cache is deleted
        cache->glContextInfo = GL_CONTEXT_POOL.GetContext(p->_preview);

        cache->glContextInfo->SetCurrent();
        const GLubyte* str = glGetString(GL_VERSION);
        const GLubyte* rend = glGetString(GL_RENDERER);
        const GLubyte* vend = glGetString(GL_VENDOR);
        wxString configs = wxString::Format("ShaderEffect - glVer:  %s  (%s)(%s)",
                                            (const char *)str,
                                            (const char *)rend,
                                            (const char *)vend);

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info(configs);
    } else {
        // we still need to grab the gl context to this thread
        cache->glContextInfo->SetCurrent();
    }
    return true;
#else
    ShaderPanel *p = (ShaderPanel *)panel;
    cache->preview = p->_preview;
    p->_preview->SetCurrentGLContext();
    return true;
#endif
}


void ShaderEffect::Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // Bail out right away if we don't have the necessary OpenGL support
    if (!OpenGLShaders::HasFramebufferObjects() || !OpenGLShaders::HasShaderSupport()) {
        setRenderBufferAll(buffer, xlCYAN);
        logger_base.error("ShaderEffect::Render() - missing OpenGL support!!");
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
            cache->InitialiseShaderConfig(SettingsMap.Get("0FILEPICKERCTRL_IFS", ""), mSequenceElements);
            programId = programIdForShaderCode(_shaderConfig, cache);
        } else {
            logger_base.warn("Could not create/set OpenGL Context for ShaderEffect.  ShaderEffect disabled.");
        }
    } else {
        if (!contextSet) {
            setRenderBufferAll(buffer, xlYELLOW);
            return;
        }
        if (_shaderConfig != nullptr) {
            programId = cache->s_programId;
        } else if (programId == 0) {
            programId = programIdForShaderCode(_shaderConfig, cache);
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

    LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, s_fbId));
    LOG_GL_ERRORV(glViewport(0, 0, buffer.BufferWi, buffer.BufferHt));

    LOG_GL_ERRORV(glClearColor(0.f, 0.f, 0.f, 0.f));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));

    if (_shaderConfig->IsAudioFFTShader() || _shaderConfig->IsAudioIntensityShader()) {
        if (s_audioTex == 0)
            s_audioTex = FFTAudioTexture();

        AudioManager* audioManager = buffer.GetMedia();
        if (audioManager != nullptr) {
            FRAMEDATATYPE datatype = ( _shaderConfig->IsAudioFFTShader() ) ? FRAMEDATA_VU : FRAMEDATA_HIGH;
            auto fftData = audioManager->GetFrameData(buffer.curPeriod, datatype, "");

            std::vector<float> fft128;
            if ( _shaderConfig->IsAudioFFTShader() )
               fft128.insert( fft128.begin(), fftData->cbegin(), fftData->cend()  );
            else
               fft128.insert( fft128.begin(), 127, *(fftData->cbegin()) );
            fft128.push_back( 0.f );

            LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0));
            LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, s_audioTex));
            LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, fft128.size(),1, GL_RED, GL_FLOAT, fft128.data()));
        }
    } else {
        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0));
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, s_rbTex));
        LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, buffer.GetPixels()));
    }

    LOG_GL_ERRORV(glBindVertexArray(s_vertexArrayId));
    LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, s_vertexBufferId));


    GLint current = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (current != programId) {
        LOG_GL_ERRORV(glUseProgram(programId));
    }

    int colourIndex = 0;
    if (!si->SetUniform2f("RENDERSIZE", buffer.BufferWi, buffer.BufferHt)) {
        if (buffer.curPeriod == buffer.curEffStartPer && _shaderConfig->HasRendersize()) {
            logger_base.warn("Unable to bind to RENDERSIZE\n%s", (const char*)_shaderConfig->GetCode().c_str());
        }
    }
    if (!si->SetUniform2f("XL_OFFSET", offsetX, offsetY)) {
        logger_base.warn("Unable to bind to XL_OFFSET");
    }
    if (!si->SetUniform1f("XL_ZOOM", zoom)) {
        logger_base.warn("Unable to bind to XL_ZOOM");
    }
    if (!si->SetUniform1f("XL_DURATION", (GLfloat)((buffer.GetEndTimeMS() - buffer.GetStartTimeMS()) / 1000.0))) {
            logger_base.warn("Unable to bind to XL_DURATION");
    }
    if (!si->SetUniform1f("TIME", (GLfloat)(_timeMS) / 1000.0)) {
        if (buffer.curPeriod == buffer.curEffStartPer && _shaderConfig->HasTime()) {
            logger_base.warn("Unable to bind to TIME\n%s", (const char*)_shaderConfig->GetCode().c_str());
        }
    }
    si->SetUniform1f("TIMEDELTA", (GLfloat)(buffer.frameTimeInMs /1000.f));

    if (si->HasUniform("DATE")) {
        wxDateTime dt = wxDateTime::Now();
        si->SetUniform4f("DATE", dt.GetYear(), dt.GetMonth() + 1, dt.GetDay(), dt.GetHour() * 3600 + dt.GetMinute() * 60 + dt.GetSecond());
    }
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
                if (colourIndex > buffer.GetColorCount()) colourIndex = 0;
                si->SetUniform4f(it._name, (double)c.red / 255.0, (double)c.green / 255.0, (double)c.blue / 255.0, 1.0);
                break;
            }
            default:
                logger_base.warn("No binding supported for %s ... we have more work to do.", (const char*)it._name.c_str());
                break;
            }
        } else {
            if (buffer.curPeriod == buffer.curEffStartPer)
                logger_base.warn("Unable to bind to %s", (const char*)it._name.c_str());
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

    LOG_GL_ERRORV(glReadPixels(0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, buffer.GetPixels()));
    if (!CanRenderOnBackgroundThread(eff, SettingsMap, buffer)) {
        LOG_GL_ERRORV(glUseProgram(0));
    }

    UnsetGLContext(cache);
}

void ShaderEffect::sizeForRenderBuffer(const RenderBuffer& rb,
    bool& s_shadersInit,
    unsigned& s_vertexArrayId, unsigned& s_vertexBufferId, unsigned& s_rbId, unsigned& s_fbId,
    unsigned& s_rbTex, int& s_rbWidth, int& s_rbHeight)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
           logger_base.error( "ShaderEffect::sizeForRenderBuffer() - Error with vertex array - %d", err );
        }

        createOpenGLRenderBuffer(rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId);
        if ((err = glGetError()) != GL_NO_ERROR) {
           logger_base.error( "ShaderEffect::sizeForRenderBuffer() - Error creating framebuffer - %d", err );
        }

        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);
        if ((err = glGetError()) != GL_NO_ERROR) {
           logger_base.error( "ShaderEffect::sizeForRenderBuffer() - Error creating renderbuffer texture - %d", err );
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
           logger_base.error( "ShaderEffect::sizeForRenderBuffer() - Error recreating framebuffer - %d", err );
        }
        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);;
        if ((err = glGetError()) != GL_NO_ERROR) {
           logger_base.error( "ShaderEffect::sizeForRenderBuffer() - Error recreating renderbuffer texture - %d", err );
        }

        s_rbWidth = rb.BufferWi;
        s_rbHeight = rb.BufferHt;
    }
}

unsigned ShaderEffect::programIdForShaderCode(ShaderConfig* cfg, ShaderRenderCache *cache)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (cfg == nullptr) {
        logger_base.error("ShaderEffect::programIdForShaderCode() - NULL ShaderConfig!");
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
                logger_base.error("ShaderEffect::programIdForShaderCode() - program id %u is not a shader program!", programId);
            } else {
                //logger_base.debug("ShaderEffect::programIdForShaderCode() - shader program %s unchanged -- id %u", (const char*)cfg->GetFilename().c_str(), programId);
                cache->SetProgramId(programId, shaderInfo);
                return programId;
            }
        }
    }

    lock.unlock();
    unsigned programId = OpenGLShaders::compile(vsSrc, fragmentShaderSrc, cfg->GetFilename());
    if (programId == 0u) {
        lock.lock();
        logger_base.error("ShaderEffect::programIdForShaderCode() - failed to compile shader program %s", (const char *)cfg->GetFilename().c_str());
        ShaderRenderCache::failedShaders.emplace(fragmentShaderSrc);
        lock.unlock();
    } else {
        logger_base.debug("ShaderEffect::programIdForShaderCode() - fragment shader %s compiled successfully", (const char*)cfg->GetFilename().c_str());
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

wxString SafeFloat(const wxString& s)
{
    if (s.StartsWith(".")) {
        return "0" + s;
    } else if (!s.Contains(".")) {
        return s + ".0";
    }
    return s;
}

wxString SafeValueOption(wxString value)
{
    wxString res = value;
    res.Replace(",", "");
    return res;
}

ShaderConfig::ShaderConfig(const wxString& filename, const wxString& code, const wxString& json, SequenceElements* sequenceElements) : _filename(filename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxJSONReader reader;
    wxJSONValue root;
    reader.Parse(json, &root);
    _description = root["DESCRIPTION"].AsString();
    if ( _description == "xLights AudioFFT" )
        _audioFFTMode = true;
    else if ( _description == "xLights Audio2" )
       _audioIntensityMode = true;
    wxJSONValue inputs = root["INPUTS"];
    wxString canvasImgName, audioFFTName;
    for (int i = 0; i < inputs.Size(); i++) {

        wxString name = inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "";

        // we ignore these as xlights provides these settings
        if (name == "XL_OFFSET") continue;
        if (name == "XL_DURATION") continue;
        if (name == "XL_ZOOM") continue;

        wxString type = inputs[i]["TYPE"].AsString();
        if (type == "float") {
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_FLOAT,
                (double)(inputs[i].HasMember("MIN") ? wxAtof(SafeFloat(inputs[i]["MIN"].AsString())) : 0.0),
                (double)(inputs[i].HasMember("MAX") ? wxAtof(SafeFloat(inputs[i]["MAX"].AsString())) : 1.0),
                (double)(inputs[i].HasMember("DEFAULT") ? wxAtof(SafeFloat(inputs[i]["DEFAULT"].AsString())) : 0.0)
            ));
        }
        else if (type == "long")
        {
            if (inputs[i].HasMember("MIN"))
            {
                _parms.push_back(ShaderParm(
                    inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                    inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                    ShaderParmType::SHADER_PARM_LONG,
                    (double)(inputs[i].HasMember("MIN") ? wxAtol(inputs[i]["MIN"].AsString()) : 0.0),
                    (double)(inputs[i].HasMember("MAX") ? wxAtol(inputs[i]["MAX"].AsString()) : 1.0),
                    (double)(inputs[i].HasMember("DEFAULT") ? wxAtol(inputs[i]["DEFAULT"].AsString()) : 0.0)
                ));
            }
            else if (inputs[i].HasMember("LABELS") && inputs[i].HasMember("VALUES"))
            {
                _parms.push_back(ShaderParm(
                    inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                    inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                    ShaderParmType::SHADER_PARM_LONGCHOICE,
                    0.0f,
                    0.0f,
                    (double)(inputs[i].HasMember("DEFAULT") ? wxAtol(inputs[i]["DEFAULT"].AsString()) : 0.0)
                ));
                auto ls = inputs[i]["LABELS"];
                auto vs = inputs[i]["VALUES"];
                int no = std::min(ls.Size(), vs.Size());
                for (int i = 0; i < no; i++)
                {
                    _parms.back()._valueOptions[vs[i].AsInt()] = SafeValueOption(ls[i].AsString());
                }
            }
            else
            {
                wxASSERT(false);
            }
        }
        else if (type == "color")
        {
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_COLOUR
            ));
        }
        else if (type == "audio")
        {
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_AUDIO
            ));
        }
        else if (type == "bool")
        {
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_BOOL,
                0.0f,
                0.0f,
                (double)(inputs[i].HasMember("DEFAULT") ? wxAtof(SafeFloat(inputs[i]["DEFAULT"].AsString())) : 0.0f)
            ));
        }
        else if (type == "point2D")
        {
            wxRealPoint minPt = wxRealPoint(
                inputs[i].HasMember("MIN") ? (inputs[i]["MIN"][0].IsDouble() ? inputs[i]["MIN"][0].AsDouble() : inputs[i]["MIN"][0].AsInt()) : 0.0f,
                inputs[i].HasMember("MIN") ? (inputs[i]["MIN"][1].IsDouble() ? inputs[i]["MIN"][1].AsDouble() : inputs[i]["MIN"][1].AsInt()) : 0.0f
            );
            wxRealPoint maxPt = wxRealPoint(
                inputs[i].HasMember("MAX") ? (inputs[i]["MAX"][0].IsDouble() ? inputs[i]["MAX"][0].AsDouble() : inputs[i]["MAX"][0].AsInt()) : 1.0f,
                inputs[i].HasMember("MAX") ? (inputs[i]["MAX"][1].IsDouble() ? inputs[i]["MAX"][1].AsDouble() : inputs[i]["MAX"][1].AsInt()) : 1.0f
            );
            wxRealPoint defPt = wxRealPoint(
                inputs[i].HasMember("DEFAULT") ? (inputs[i]["DEFAULT"][0].IsDouble() ? inputs[i]["DEFAULT"][0].AsDouble() : inputs[i]["DEFAULT"][0].AsInt()) : 0.0f,
                inputs[i].HasMember("DEFAULT") ? (inputs[i]["DEFAULT"][1].IsDouble() ? inputs[i]["DEFAULT"][1].AsDouble() : inputs[i]["DEFAULT"][1].AsInt()) : 0.0f
            );
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_POINT2D,
                minPt,
                maxPt,
                defPt
            ));
        }
        else if (type == "image")
        {
            // ignore these as we will use the existing buffer content
            //_parms.push_back({
            //        inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
            //        inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
            //        ShaderParmType::SHADER_PARM_IMAGE,
            //        0.0f,
            //        0.0f,
            //        0.0f
            //    });
            if (inputs[i].HasMember("NAME"))
            {
                canvasImgName = inputs[i]["NAME"].AsString();
                //_canvasMode = true;
            }
        }
        else if (type == "audioFFT")
        {
            if (inputs[i].HasMember("NAME"))
            {
                audioFFTName = inputs[i]["NAME"].AsString();
                logger_base.info("ShaderEffect - found audioFFT shader with name '%s'", static_cast<const char *>(audioFFTName.c_str()));
            }
        }
        else if (type == "text") {
            // ignore these
            if (inputs[i].HasMember("NAME")) {
                logger_base.warn("ShaderEffect - found text property with name '%s' ... ignored", static_cast<const char*>(inputs[i]["NAME"].AsString().c_str()));
            }
        }
        else if (type == "event")
        {
            // ignore these
            _parms.push_back(ShaderParm(
                inputs[i].HasMember("NAME") ? inputs[i]["NAME"].AsString() : "",
                inputs[i].HasMember("LABEL") ? inputs[i]["LABEL"].AsString() : "",
                ShaderParmType::SHADER_PARM_EVENT,
                0.0f,
                0.0f,
                0.0f)
            );

            // Add timing tracks
            if (sequenceElements != nullptr) {
                int tt = 0;
                for (int i = 0; i < sequenceElements->GetElementCount(); i++) {
                    Element* e = sequenceElements->GetElement(i);
                    if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                        _parms.back()._valueOptions[tt++] = e->GetName();
                    }
                }
            }
        }
        else
        {
            logger_base.warn("Unknown type parsing shader JSON : %s.", (const char*)type.c_str());
            wxASSERT(false);
        }
    }
    wxJSONValue passes = root["PASSES"];
    for (int i = 0; i < passes.Size(); i++)
    {
        _passes.push_back({
            inputs[i].HasMember("TARGET") ? inputs[i]["TARGET"].AsString() : "",
            passes[i].HasMember("PERSISTENT") ? passes[i]["PERSISTENT"].AsString() == "true" : false
            });
    }

    // The shader code needs declarations for the uniforms that we silently set with each call to Render()
    // and the uniforms that correspond to user-visible settings
    wxString prependText = _(
    "uniform float TIME;\n"
    "uniform float TIMEDELTA;\n"
    "uniform vec2 RENDERSIZE;\n"
    "uniform bool clearBuffer;\n"
    "uniform bool resetNow;\n"
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
    "#define XL_SHADER\n\n"
    );

    for (const auto& p : _parms) {
        wxString name(p._name);
        wxString str;
        switch (p._type) {
        case ShaderParmType::SHADER_PARM_FLOAT:
        {
            str = wxString::Format("uniform float %s;\n", name);
            prependText += str;
            break;
        }
        case ShaderParmType::SHADER_PARM_BOOL:
        case ShaderParmType::SHADER_PARM_EVENT:
        {
            str = wxString::Format("uniform bool %s;\n", name);
            prependText += str;
            break;
        }
        case ShaderParmType::SHADER_PARM_LONG:
        case ShaderParmType::SHADER_PARM_LONGCHOICE:
        {
            str = wxString::Format("uniform int %s;\n", name);
            prependText += str;
            break;
        }
        case ShaderParmType::SHADER_PARM_POINT2D:
        {
            str = wxString::Format("uniform vec2 %s;\n", name);
            prependText += str;
            break;
        }
        case ShaderParmType::SHADER_PARM_COLOUR:
        {
            str = wxString::Format("uniform vec4 %s;\n", name);
            prependText += str;
            break;
        }
        default:
        {
            // rest of these are un-implemented currently
        }
        }
    }

    prependText += _("vec4 IMG_NORM_PIXEL_2D(sampler2D sampler, vec2 pct, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord* pct);\n}\n\n");
    prependText += _("vec4 IMG_NORM_PIXEL(sampler2D sampler, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord);\n}\n\n");
    prependText += _("vec4 IMG_PIXEL_2D(sampler2D sampler, vec2 pct, vec2 loc) {\n   return IMG_NORM_PIXEL_2D(sampler, pct, loc / RENDERSIZE);\n}\n\n");
    prependText += _("vec4 IMG_PIXEL(sampler2D sampler, vec2 loc) {\n   return texture(sampler, loc / RENDERSIZE);\n}\n\n");
    prependText += _("vec4 IMG_THIS_PIXEL(sampler2D sampler) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord);\n}\n\n");
    prependText += _("vec4 IMG_THIS_NORM_PIXEL_2D(sampler2D sampler, vec2 pct) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord * pct);\n}\n\n");
    prependText += _("vec4 IMG_THIS_NORM_PIXEL(sampler2D sampler) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord);\n}\n\n");
    prependText += _("vec4 IMG_THIS_PIXEL_2D(sampler2D sampler, vec2 pct) {\n   return IMG_THIS_NORM_PIXEL_2D(sampler, pct);\n}\n\n");
    prependText += _("vec4 IMG_NORM_PIXEL_RECT(sampler2DRect sampler, vec2 pct, vec2 normLoc) {\n   vec2 coord = normLoc;\n   return texture(sampler, coord * RENDERSIZE);\n}\n\n");
    prependText += _("vec4 IMG_PIXEL_RECT(sampler2DRect sampler, vec2 pct, vec2 loc) {\n   return IMG_NORM_PIXEL_RECT(sampler, pct, loc / RENDERSIZE);\n}\n\n");
    prependText += _("vec4 IMG_THIS_NORM_PIXEL_RECT(sampler2DRect sampler, vec2 pct) {\n   vec2 coord = xl_FragNormCoord;\n   return texture(sampler, coord * RENDERSIZE);\n}\n\n");
    prependText += _("vec4 IMG_THIS_PIXEL_RECT(sampler2DRect sampler, vec2 pct) {\n   return IMG_THIS_NORM_PIXEL_RECT(sampler, pct);\n}\n\n");
    prependText += _("ivec2 IMG_SIZE(sampler2D sampler) {\n   return textureSize(sampler, 0);\n}\n\n");

#ifdef __DEBUG
    int i = 0;
    for (auto c : code)
    {
       if ((int)c < 32 || (int)c > 127)
        {
            if (c != 13 && c != 10 && c!= 9)
            logger_base.debug("%d 0x%x %c", i, (int)c, c);
            wxASSERT(false);
        }
        i++;
    }
#endif

    wxUniChar ch133(133);
    wxString shaderCode = wxString(code.mb_str(wxConvUTF8));
    for (int x = 0; x < shaderCode.size(); x++) {
        if (shaderCode[x] == ch133) {
            shaderCode[x] = '.';
        }
    }
    int pos = shaderCode.Find("*/");
    if (pos > 0)
    {
        shaderCode = shaderCode.substr(pos + 2);
    }
    shaderCode.Replace("gl_FragColor", "fragmentColor");
    shaderCode.Replace("vv_FragNormCoord", "xl_FragNormCoord");
    shaderCode.Replace("isf_FragNormCoord", "xl_FragNormCoord");
    shaderCode.Replace("isf_FragCoord", "xl_FragCoord");
    shaderCode.Replace("gl_FragCoord", "xl_FragCoord");
    shaderCode.Replace("gl_FragNormCoord", "xl_FragNormCoord");
    shaderCode.Replace("varying ", "uniform ");
    shaderCode.Replace("texture2D(", "texture(");
    shaderCode.Replace("texture2D (", "texture(");
    if (!audioFFTName.empty())
    {
        shaderCode.Replace(audioFFTName, "texSampler");
        _audioFFTMode = true;
    }
    else if (!canvasImgName.empty())
    {
        shaderCode.Replace(canvasImgName, "texSampler");
        _canvasMode = true;
    }

    _hasRendersize = Contains(shaderCode, "RENDERSIZE");
    _hasTime = Contains(shaderCode, "TIME");
    _hasCoord = Contains(shaderCode, "xl_FragCoord");

    _code = "#version 330\n\n";
    size_t idx = shaderCode.find("#extension");
    if (idx != std::string::npos) {
        size_t nidx = shaderCode.find("\n", idx);
        _code += shaderCode.substr(0, nidx);
        _code += "\n";
        shaderCode = shaderCode.substr(nidx);
    }
    _code += prependText.ToStdString();
    _code += shaderCode.ToStdString();
    wxASSERT(_code != "");
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
    for (const auto& p : _parms) {
        if (p._type == ShaderParmType::SHADER_PARM_EVENT) return true;
    }
    return false;
}
