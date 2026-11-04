// The real macOS implementations live in xLights-Apple-core (Swift). The unit
// tests don't link it, so provide minimal stand-ins for core objects that
// reference these hooks. Other platforms use the inline fallbacks in
// ExternalHooks.h and don't need this file (and CMake only globs test_*.cpp).
#if defined(__APPLE__)

#include "osxUtils/ExternalHooksApple.h"

#include <filesystem>

bool ObtainAccessToURL(const std::string&, bool) { return true; }

bool FileExists(const std::string& s, bool) {
    std::error_code ec;
    return std::filesystem::exists(std::filesystem::path(s), ec);
}

void MarkNewFileRevision(const std::string&, int) {}
std::list<std::string> GetFileRevisions(const std::string&) { return {}; }
std::string GetURLForRevision(const std::string& path, const std::string&) { return path; }
void EnableSleepModes() {}
void DisableSleepModes() {}
bool IsFromAppStore() { return false; }
std::string DescribeCurrentAppleException() { return {}; }
void SetThreadQOS(int) {}

#endif
