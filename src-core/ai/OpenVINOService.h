#pragma once

// OpenVINO GenAI AI service: on-device image generation (Stable Diffusion)
// and speech-to-text with word-level alignment (Whisper). Enabled when the
// build is configured with cmake -DXLIGHTS_USE_OPENVINO_GENAI=ON.

#ifdef HAVE_OPENVINO_GENAI

#include "aiBase.h"
#include "aiType.h"

#include <list>
#include <string>
#include <vector>

class OpenVINOService : public aiBase {
public:
    explicit OpenVINOService(ServiceManager* sm);
    ~OpenVINOService() override = default;

    [[nodiscard]] std::string GetLLMName() const override { return "OpenVINO (local)"; }
    [[nodiscard]] bool IsAvailable() const override;
    [[nodiscard]] std::list<aiType::TYPE> GetTypes() const override;

    // Text generation not supported for on-device OpenVINO.
    [[nodiscard]] std::pair<std::string, bool> CallLLM(const std::string& prompt) const override;

    [[nodiscard]] std::vector<ServiceProperty> GetProperties() const override;
    void SetProperty(const std::string& id, bool value) override;
    void SetProperty(const std::string& id, int value) override;
    void SetProperty(const std::string& id, const std::string& value) override;

    void SaveSettings() const override;
    void LoadSettings() override;

    [[nodiscard]] AIImageGenerator* createAIImageGenerator() const override;
    [[nodiscard]] AILyricTrack GenerateLyricTrack(const std::string& audioPath) const override;

private:
    // "AUTO", "CPU", "GPU", or "NPU"
    std::string _device { "AUTO" };
    // Directory of an SD model exported to OpenVINO IR format.
    // Export with: optimum-cli export openvino --model stabilityai/stable-diffusion-v1-5 <dir>
    std::string _imageModelDir;
    // Directory of a Whisper model exported to OpenVINO IR format.
    // Export with: optimum-cli export openvino --model openai/whisper-base <dir>
    std::string _whisperModelDir;
    // Image generation parameters
    int _imageWidth { 512 };
    int _imageHeight { 512 };
    int _imageSteps { 20 };
};

#endif // HAVE_OPENVINO_GENAI
