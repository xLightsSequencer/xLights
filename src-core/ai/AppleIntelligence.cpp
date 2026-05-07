/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only aiBase subclass. The actual platform calls (FoundationModels
// LLM, ImagePlayground image generator, soon SFSpeechRecognizer) live
// behind the `AppleAIBridge` namespace declared in
// `macOS/src-apple-core/ai/AppleIntelligenceBridge.h`. That keeps Swift
// + CoreGraphics types out of `src-core/`, which must build wx- and
// platform-detail-free for non-Apple toolchains.
//
// Mirrors the StemSeparator split: this file in src-core/, the
// implementation behind it in src-apple-core/. Non-Apple Xcode builds
// don't reach this code because the whole TU is `#ifdef __APPLE__`.
// Linux / Windows build systems don't compile it either (they only
// pull in files explicitly listed in their .cbp / .vcxproj).

#ifdef __APPLE__

#include "ai/AppleIntelligence.h"
#include "ai/AppleIntelligenceBridge.h"
#include "ai/ServiceManager.h"
#include "ai/ServiceProperty.h"

#include "utils/string_utils.h"

#include <nlohmann/json.hpp>

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <utility>
#include <vector>


std::list<aiType::TYPE> AppleIntelligence::GetTypes() const {
    // PROMPT is intentionally excluded — the on-device session size
    // limit is too small for the long prompts model-mapping needs.
    // FoundationModels.LanguageModelSession requires macOS 26+ / iOS 26+;
    // ImagePlayground.ImageCreator requires macOS 15.4+ / iOS 18.4+.
    std::list<aiType::TYPE> types;
    if (__builtin_available(macOS 26.0, iOS 26.0, *)) {
        types.push_back(aiType::TYPE::COLORPALETTES);
    }
    if (__builtin_available(macOS 15.4, iOS 18.4, *)) {
        types.push_back(aiType::TYPE::IMAGES);
    }
    // SFSpeechRecognizer's on-device path goes back to macOS 10.15 /
    // iOS 13, so the OS gate is effectively just "Apple platform".
    // Vocals isolation via stem separation gives much better results
    // than running the recognizer on the full mix; the call site is
    // responsible for picking which file to hand us.
    types.push_back(aiType::TYPE::SPEECH2TEXT);
    return types;
}

bool AppleIntelligence::IsAvailable() const {
    return !_enabledTypes.empty();
}

void AppleIntelligence::SaveSettings() const {
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("appleAIEnable_") + aiType::TypeSettingsSuffix(t),
                                IsEnabledForType(t));
    }
}

void AppleIntelligence::LoadSettings() {
    bool oldEnabled = _sm->getServiceSetting("appleAIEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("appleAIEnable_") + aiType::TypeSettingsSuffix(t),
                                              oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

std::vector<ServiceProperty> AppleIntelligence::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "Apple Intelligence", "AppleIntelligence", {}, {}, {} });
    for (auto t : GetTypes()) {
        props.push_back({
            ServiceProperty::Kind::Bool,
            std::string("AppleIntelligence.Enable_") + aiType::TypeSettingsSuffix(t),
            std::string("Enable ") + aiType::TypeName(t),
            "AppleIntelligence",
            {},
            {},
            IsEnabledForType(t)
        });
    }
    return props;
}

void AppleIntelligence::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("AppleIntelligence.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
}

std::pair<std::string, bool> AppleIntelligence::CallLLM(const std::string& prompt) const {
    std::string s = AppleAIBridge::CallLLM(prompt);
    return { s, !s.empty() };
}

aiBase::AIColorPalette AppleIntelligence::GenerateColorPalette(const std::string& prompt) const {
    aiBase::AIColorPalette ret;

    std::string res = AppleAIBridge::GenerateColorPaletteJSON(prompt);
    if (res.empty()) {
        return ret;
    }

    try {
        nlohmann::json const root = nlohmann::json::parse(res);
        if (root.contains("error")) {
            ret.error = root["error"].get<std::string>();
            return ret;
        }
        if (root.contains("Description")) {
            ret.description = root["Description"].get<std::string>();
        }
        if (root.contains("Colors") && root["Colors"].is_array()) {
            for (auto const& c : root["Colors"]) {
                aiBase::AIColor col;
                col.name        = c.value("Name", std::string());
                col.description = c.value("Description", std::string());
                col.hexValue    = c.value("Hex Value", std::string());
                if (!col.hexValue.empty() && col.hexValue.front() != '#') {
                    col.hexValue = "#" + col.hexValue;
                }
                ret.colors.push_back(std::move(col));
            }
        }
    } catch (const std::exception&) {
        // Malformed JSON from FoundationModels. Leave ret empty so the
        // caller surfaces "no colors generated".
    }

    return ret;
}

namespace {

constexpr const char* kAppleStyleId       = "AppleIntelligence.Style";
constexpr const char* kAppleStyleCategory = "Apple Intelligence Image";

class AppleIntelligenceImageGenerator : public aiBase::AIImageGenerator {
public:
    ~AppleIntelligenceImageGenerator() override = default;

    std::vector<ServiceProperty> GetProperties() const override {
        ServiceProperty p;
        p.kind     = ServiceProperty::Kind::Choice;
        p.id       = kAppleStyleId;
        p.label    = "Style";
        p.category = kAppleStyleCategory;
        p.choices  = { "animation", "illustration", "sketch", "emoji" };
        p.value    = style;
        return { p };
    }

    void SetProperty(const std::string& id, const std::string& value) override {
        if (id == kAppleStyleId) {
            style = Lower(value);
        }
    }

    void generateImage(const std::string& prompt,
                       std::function<void(aiBase::AIImageResult)> cb) override {
        // Same "MANDATORY OUTPUT" instructions desktop has shipped with —
        // pushes the model toward black-background, flat-shaded designs
        // that fit the typical xLights pixel-grid use case better than
        // photoreal output.
        std::string full = prompt + R"(
        MANDATORY OUTPUT REQUIREMENTS: Background: Black background (#000000) with no watermarks or border.
        The design features bold, clean outlines, simple cell-shading, and a limited vibrant color palette with clean edges and no gradients.
        )";

        auto cbCopy = std::move(cb);
        AppleAIBridge::GenerateImage(prompt, full, style,
            [cbCopy = std::move(cbCopy)](AppleAIBridge::ImageResult r) {
                aiBase::AIImageResult res;
                res.pngBytes = std::move(r.png);
                res.error    = std::move(r.error);
                if (cbCopy) cbCopy(std::move(res));
            });
    }

    std::string style = "animation";
};

} // namespace

aiBase::AIImageGenerator* AppleIntelligence::createAIImageGenerator() const {
    return new AppleIntelligenceImageGenerator();
}

aiBase::AILyricTrack AppleIntelligence::GenerateLyricTrack(const std::string& audioPath) const {
    aiBase::AILyricTrack ret;

    if (audioPath.empty()) {
        ret.error = "No audio path provided";
        return ret;
    }

    // SFSpeechRecognizer is callback-based; the bridge marshals the
    // result back through `callback`. Block synchronously on a
    // condition variable so the iBase contract (synchronous return
    // from GenerateLyricTrack) is preserved. The caller is expected
    // to dispatch this method off the main thread — long audio means
    // many seconds of recognition.
    std::mutex m;
    std::condition_variable cv;
    bool done = false;
    AppleAIBridge::LyricResult bridgeResult;

    AppleAIBridge::GenerateLyricTrack(audioPath,
        [&](AppleAIBridge::LyricResult r) {
            {
                std::lock_guard<std::mutex> lk(m);
                bridgeResult = std::move(r);
                done = true;
            }
            cv.notify_one();
        });

    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{ return done; });
    }

    if (!bridgeResult.error.empty()) {
        ret.error = bridgeResult.error;
        return ret;
    }
    ret.lyrics.reserve(bridgeResult.lyrics.size());
    for (auto& s : bridgeResult.lyrics) {
        aiBase::AILyric l;
        l.word    = std::move(s.word);
        l.startMS = s.startMS;
        l.endMS   = s.endMS;
        ret.lyrics.push_back(std::move(l));
    }
    return ret;
}

#endif // __APPLE__
