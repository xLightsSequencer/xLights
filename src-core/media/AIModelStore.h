#pragma once

// A8: helpers for locating and installing downloadable AI model
// files under user-controlled roots (the show folder + any
// configured media folders). Models live in `<root>/ai-models/`.
// The store itself doesn't do the HTTP download — desktop uses
// wxWidgets / libcurl through `CurlManager`, iPad uses
// NSURLSession. This file just exposes the constants, path
// discovery, and a couple of filesystem utilities.

#include <string>
#include <vector>

namespace AIModelStore {

// HTDemucs 4-stem separation (john-rocky/CoreML-Models).
// The release asset is a zip that expands to a `.mlpackage` directory.
constexpr const char* kDemucsModelName = "HTDemucs_SourceSeparation_F32.mlpackage";
constexpr const char* kDemucsDownloadURL =
    "https://github.com/john-rocky/CoreML-Models/releases/download/demucs-v1/"
    "HTDemucs_SourceSeparation_F32.mlpackage.zip";

// HTDemucs ONNX model for OpenVINO inference.
constexpr const char* kDemucsOnnxModelName = "htdemucs.onnx";
constexpr const char* kDemucsOnnxDownloadURL = "https://huggingface.co/smank/htdemucs-onnx/resolve/main/htdemucs.onnx";

// Subdirectory under each root where models are stored.
constexpr const char* kModelsSubdir = "ai-models";

// Given a list of configured roots (show folder + media folders in
// preference order), returns the full `<root>/ai-models/` path for
// each. Paths are returned whether they exist or not — callers use
// `FindModel` to check for presence.
std::vector<std::string> CandidateModelDirs(const std::vector<std::string>& roots);

// Scan the provided model directories for a subdirectory matching
// `modelName`. Returns the first hit's absolute path, or empty on
// miss. `modelName` is typically a `.mlpackage` directory name.
std::string FindModel(const std::string& modelName,
                       const std::vector<std::string>& modelDirs);

// `mkdir -p` equivalent. Returns true if the directory exists or was
// created successfully.
bool EnsureDirectory(const std::string& path);

} // namespace AIModelStore
