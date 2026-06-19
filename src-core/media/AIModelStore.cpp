#include "AIModelStore.h"

#include <filesystem>
#include <system_error>

namespace AIModelStore {

std::vector<std::string> CandidateModelDirs(const std::vector<std::string>& roots) {
    std::vector<std::string> out;
    out.reserve(roots.size());
    for (const auto& r : roots) {
        if (r.empty()) continue;
        std::filesystem::path p(r);
        p /= kModelsSubdir;
        out.push_back(p.string());
    }
    return out;
}

std::string FindModel(const std::string& modelName,
                       const std::vector<std::string>& modelDirs) {
    for (const auto& d : modelDirs) {
        if (d.empty()) continue;
        std::filesystem::path candidate = std::filesystem::path(d) / modelName;
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate.string();
        }
    }
    return {};
}

bool EnsureDirectory(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    std::filesystem::path p(path);
    if (std::filesystem::exists(p, ec)) {
        return std::filesystem::is_directory(p, ec);
    }
    return std::filesystem::create_directories(p, ec);
}

} // namespace AIModelStore
