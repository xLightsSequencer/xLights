/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoDecodeSizeRegistry.h"

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace {
std::mutex g_mutex;
std::unordered_map<std::string, std::pair<int, int>> g_sizes;
} // namespace

namespace VideoDecodeSizeRegistry {

void Clear() {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_sizes.clear();
}

void SetMaxDecodeSize(const std::string& resolvedPath, int w, int h) {
    if (resolvedPath.empty() || w <= 0 || h <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lk(g_mutex);
    auto& e = g_sizes[resolvedPath];
    e.first = std::max(e.first, w);
    e.second = std::max(e.second, h);
}

bool GetMaxDecodeSize(const std::string& resolvedPath, int& w, int& h) {
    std::lock_guard<std::mutex> lk(g_mutex);
    auto it = g_sizes.find(resolvedPath);
    if (it == g_sizes.end()) {
        return false;
    }
    w = it->second.first;
    h = it->second.second;
    return true;
}

bool Empty() {
    std::lock_guard<std::mutex> lk(g_mutex);
    return g_sizes.empty();
}

} // namespace VideoDecodeSizeRegistry
