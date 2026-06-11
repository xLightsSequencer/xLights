#ifdef HAVE_OPENVINO_GENAI

#ifdef _WIN32
#  define NOMINMAX
#endif

#include "ai/OpenVINOService.h"
#include "ai/OpenVINOImageGenerator.h"
#include "ai/ServiceManager.h"
#include "ai/ServiceProperty.h"
#include "media/FFmpegAudioDecoder.h"

#include <openvino/genai/image_generation/text2image_pipeline.hpp>
#include <openvino/genai/whisper_pipeline.hpp>
#include <openvino/genai/whisper_generation_config.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

OpenVINOService::OpenVINOService(ServiceManager* sm)
    : aiBase(sm) {}

bool OpenVINOService::IsAvailable() const {
    if (_enabledTypes.empty()) return false;
    for (auto t : _enabledTypes) {
        if (t == aiType::TYPE::IMAGES     && !_imageModelDir.empty())   return true;
        if (t == aiType::TYPE::SPEECH2TEXT && !_whisperModelDir.empty()) return true;
    }
    return false;
}

std::list<aiType::TYPE> OpenVINOService::GetTypes() const {
    return std::list({ aiType::TYPE::IMAGES, aiType::TYPE::SPEECH2TEXT });
}

std::pair<std::string, bool> OpenVINOService::CallLLM(const std::string& /*prompt*/) const {
    return { "OpenVINO (local) does not support text generation", false };
}

// ─── Settings ────────────────────────────────────────────────────────────────

void OpenVINOService::SaveSettings() const {
    _sm->setServiceSetting("OpenVINODevice",         _device);
    _sm->setServiceSetting("OpenVINOImageModelDir",  _imageModelDir);
    _sm->setServiceSetting("OpenVINOWhisperModelDir",_whisperModelDir);
    _sm->setServiceSetting("OpenVINOImageWidth",     _imageWidth);
    _sm->setServiceSetting("OpenVINOImageHeight",    _imageHeight);
    _sm->setServiceSetting("OpenVINOImageSteps",     _imageSteps);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("OpenVINOEnable_") + aiType::TypeSettingsSuffix(t),
                               IsEnabledForType(t));
    }
    spdlog::info("OpenVINO service settings saved");
}

void OpenVINOService::LoadSettings() {
    _device          = _sm->getServiceSetting("OpenVINODevice",          _device);
    _imageModelDir   = _sm->getServiceSetting("OpenVINOImageModelDir",   _imageModelDir);
    _whisperModelDir = _sm->getServiceSetting("OpenVINOWhisperModelDir", _whisperModelDir);
    _imageWidth      = _sm->getServiceSetting("OpenVINOImageWidth",      _imageWidth);
    _imageHeight     = _sm->getServiceSetting("OpenVINOImageHeight",     _imageHeight);
    _imageSteps      = _sm->getServiceSetting("OpenVINOImageSteps",      _imageSteps);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(
            std::string("OpenVINOEnable_") + aiType::TypeSettingsSuffix(t), false);
        SetEnabledForType(t, enabled);
    }
}

std::vector<ServiceProperty> OpenVINOService::GetProperties() const {
    std::vector<ServiceProperty> props;

    props.push_back({ ServiceProperty::Kind::Category, {}, "OpenVINO (local)", "OpenVINO (local)", {}, {}, {} });

    // Per-type enable toggles
    for (auto t : GetTypes()) {
        std::string id = std::string("OpenVINO.Enable_") + aiType::TypeSettingsSuffix(t);
        props.push_back({ ServiceProperty::Kind::Bool, id,
                          std::string("Enable ") + aiType::TypeName(t),
                          "OpenVINO (local)", {}, {}, IsEnabledForType(t) });
    }

    // Inference device
    props.push_back({ ServiceProperty::Kind::Choice, "OpenVINO.Device", "Inference Device",
                      "OpenVINO (local)",
                      "Hardware target for inference (AUTO lets OpenVINO pick the fastest available)",
                      { "AUTO", "CPU", "GPU", "NPU" }, _device });

    // Image generation settings
    props.push_back({ ServiceProperty::Kind::String, "OpenVINO.ImageModelDir",
                      "SD Model Directory", "OpenVINO (local)",
                      "Path to a Stable Diffusion model exported to OpenVINO IR format.\n"
                      "Export with: optimum-cli export openvino --model stabilityai/stable-diffusion-v1-5 <dir>",
                      {}, _imageModelDir });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINO.ImageWidth",  "Image Width (px)",
                      "OpenVINO (local)", "Width of generated images", {}, _imageWidth });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINO.ImageHeight", "Image Height (px)",
                      "OpenVINO (local)", "Height of generated images", {}, _imageHeight });
    props.push_back({ ServiceProperty::Kind::Int, "OpenVINO.ImageSteps",  "Diffusion Steps",
                      "OpenVINO (local)", "Denoising steps (20 is a good default; more = slower but higher quality)",
                      {}, _imageSteps });

    // Whisper settings
    props.push_back({ ServiceProperty::Kind::String, "OpenVINO.WhisperModelDir",
                      "Whisper Model Directory", "OpenVINO (local)",
                      "Path to a Whisper model exported to OpenVINO IR format.\n"
                      "Export with: optimum-cli export openvino --model openai/whisper-base <dir>",
                      {}, _whisperModelDir });

    return props;
}

void OpenVINOService::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("OpenVINO.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
}

void OpenVINOService::SetProperty(const std::string& id, int value) {
    if      (id == "OpenVINO.ImageWidth")  _imageWidth  = std::max(64, value);
    else if (id == "OpenVINO.ImageHeight") _imageHeight = std::max(64, value);
    else if (id == "OpenVINO.ImageSteps")  _imageSteps  = std::max(1,  value);
}

void OpenVINOService::SetProperty(const std::string& id, const std::string& value) {
    if      (id == "OpenVINO.Device")         _device          = value;
    else if (id == "OpenVINO.ImageModelDir")  _imageModelDir   = value;
    else if (id == "OpenVINO.WhisperModelDir")_whisperModelDir = value;
}

// ─── Image generation ────────────────────────────────────────────────────────

aiBase::AIImageGenerator* OpenVINOService::createAIImageGenerator() const {
    return new OpenVINOImageGenerator(_imageModelDir, _device,
                                       _imageWidth, _imageHeight, _imageSteps);
}

// ─── Speech-to-text with word-level alignment ─────────────────────────────────

static std::vector<float> DecodeAudioTo16kHz(const std::string& audioPath) {
    FFmpegAudioDecoder decoder;
    DecodedAudioInfo info;
    uint8_t* pcmData  = nullptr;
    long     pcmSize  = 0;
    float*   leftData = nullptr;
    float*   rightData= nullptr;
    long     trackSize= 0;

    constexpr long kTargetRate = 16000;

    if (!decoder.DecodeFile(audioPath, kTargetRate, 0, info, pcmData, pcmSize,
                            leftData, rightData, trackSize)) {
        spdlog::error("OpenVINOService: failed to decode audio '{}'", audioPath);
        return {};
    }

    std::vector<float> mono(static_cast<size_t>(trackSize));
    if (rightData && rightData != leftData) {
        // Average stereo to mono
        for (long i = 0; i < trackSize; ++i)
            mono[i] = (leftData[i] + rightData[i]) * 0.5f;
        std::free(rightData);
    } else {
        for (long i = 0; i < trackSize; ++i)
            mono[i] = leftData[i];
    }
    std::free(leftData);
    std::free(pcmData);

    return mono;
}

aiBase::AILyricTrack OpenVINOService::GenerateLyricTrack(const std::string& audioPath) const {
    AILyricTrack result;

    if (_whisperModelDir.empty()) {
        result.error = "OpenVINO Whisper model directory not configured";
        return result;
    }

    std::vector<float> speech = DecodeAudioTo16kHz(audioPath);
    if (speech.empty()) {
        result.error = "Failed to decode audio file to 16 kHz PCM";
        return result;
    }

    try {
        ov::genai::WhisperPipeline pipeline(_whisperModelDir, _device);

        ov::genai::WhisperGenerationConfig config;
        config.return_timestamps = true;

        ov::genai::RawSpeechInput rawInput(speech.begin(), speech.end());
        auto decoded = pipeline.generate(rawInput, config);

        if (!decoded.chunks.has_value() || decoded.chunks->empty()) {
            // Fall back to full-text with single segment spanning the whole track
            spdlog::warn("OpenVINOService: Whisper returned no timed chunks; "
                          "timestamps will be approximate");
            std::string full;
            for (const auto& t : decoded.texts) full += t;

            std::istringstream iss(full);
            std::string word;
            float dur = static_cast<float>(speech.size()) / 16000.0f;
            std::vector<std::string> words;
            while (iss >> word) words.push_back(word);

            float wordDur = words.empty() ? 0.0f : dur / static_cast<float>(words.size());
            for (size_t i = 0; i < words.size(); ++i) {
                AILyric lyric;
                lyric.word     = words[i];
                lyric.startMS  = static_cast<int>(i * wordDur * 1000.0f);
                lyric.endMS    = static_cast<int>((i + 1) * wordDur * 1000.0f);
                result.lyrics.push_back(lyric);
            }
            return result;
        }

        for (const auto& chunk : *decoded.chunks) {
            // Trim leading/trailing whitespace from the chunk text
            std::string text = chunk.text;
            auto first = text.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            text = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);

            // Split into words and distribute timestamps proportionally by character count
            std::vector<std::string> words;
            {
                std::istringstream iss(text);
                std::string w;
                while (iss >> w) words.push_back(w);
            }
            if (words.empty()) continue;

            float chunkStart = chunk.start_ts;
            float chunkEnd   = chunk.end_ts;
            float chunkDur   = chunkEnd - chunkStart;

            // Compute total character length for proportional distribution
            size_t totalChars = 0;
            for (const auto& w : words) totalChars += w.size();
            if (totalChars == 0) totalChars = words.size(); // equal distribution fallback

            float t = chunkStart;
            for (size_t i = 0; i < words.size(); ++i) {
                float fraction = static_cast<float>(words[i].size()) / static_cast<float>(totalChars);
                float wordDur  = chunkDur * fraction;

                AILyric lyric;
                lyric.word    = words[i];
                lyric.startMS = static_cast<int>(t * 1000.0f);
                lyric.endMS   = static_cast<int>((t + wordDur) * 1000.0f);
                result.lyrics.push_back(lyric);
                t += wordDur;
            }
        }

        spdlog::info("OpenVINOService: transcribed {} words in {} chunks",
                     result.lyrics.size(), decoded.chunks->size());

    } catch (const std::exception& e) {
        spdlog::error("OpenVINOService: Whisper inference failed: {}", e.what());
        result.error = std::string("OpenVINO Whisper error: ") + e.what();
    }

    return result;
}
#else
std::string OpenVINOServiceData;
#endif // HAVE_OPENVINO_GENAI
