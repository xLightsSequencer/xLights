#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>

// Scratch directory under the system temp dir, removed on destruction.
struct TempDir {
    std::filesystem::path path;

    TempDir() {
        static int counter = 0;
        auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        // ctest runs many test processes at once, so the clock alone can collide
        std::random_device rd;
        path = std::filesystem::temp_directory_path() / ("xl_unit_" + std::to_string(stamp) + "_" + std::to_string(rd()) + "_" + std::to_string(counter++));
        std::filesystem::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    std::string File(const std::string& name) const { return (path / name).string(); }

    std::string Write(const std::string& name, const std::string& body) const {
        std::ofstream(path / name, std::ios::binary) << body;
        return File(name);
    }
};
