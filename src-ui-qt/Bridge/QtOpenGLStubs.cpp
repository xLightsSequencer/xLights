// Stubs for OpenGLShaders and GLContextManager.
// The actual OpenGL extension function pointer DEFINITIONS live in
// QtGLPointerDefs.cpp (compiled as a separate non-Qt object library so it
// can include <GL/glext.h> without conflicting with Qt's GL abstractions).

#include "../../src-core/effects/OpenGLShaders.h"
#include "../../src-core/graphics/GLContextManager.h"

#include <functional>

// ── OpenGLShaders ─────────────────────────────────────────────────────────────
// Always report no shader support so ShaderEffect::Render() exits immediately.

bool        OpenGLShaders::HasShaderSupport()      { return false; }
bool        OpenGLShaders::HasFramebufferObjects()  { return false; }
unsigned    OpenGLShaders::compile(const std::string&, const std::string&, const std::string&) { return 0; }
std::string OpenGLShaders::PrepareShaderCodeForLogging(const std::string& c) { return c; }
void        OpenGLShaders::LogGLError(const char*, int, const char*)   {}
void        OpenGLShaders::DoLogGLError(const char*, int, const char*) {}
void        OpenGLShaders::SetupDebugLogging() {}

// ── GLContextManager ──────────────────────────────────────────────────────────
// ShaderEffect's destructor calls GLContextManager even though Render() is
// a no-op. Stubs make all these calls safe no-ops.

// GLContextManager constructor is private — use aligned storage so we can
// return a reference without calling the constructor.
GLContextManager& GLContextManager::Instance() {
    alignas(GLContextManager) static char storage[sizeof(GLContextManager)] = {};
    return *reinterpret_cast<GLContextManager*>(storage);
}

// Private destructor: provide the definition so the linker is satisfied.
// The storage-based singleton above is never destructed (char array, not GLContextManager).
GLContextManager::~GLContextManager() {}

void* GLContextManager::AcquireContext()               { return nullptr; }
void  GLContextManager::MakeCurrent(void*)             {}
void  GLContextManager::DoneCurrent(void*)             {}
void  GLContextManager::ReleaseContext(void*)          {}
void  GLContextManager::ExecuteOnGLThread(std::function<void()> fn) { fn(); }
