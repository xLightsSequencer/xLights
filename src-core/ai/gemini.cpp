#include "ai/gemini.h"
#include "ai/ServiceManager.h"

#include "utils/Base64.h"
#include "utils/CurlManager.h"
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>

#include <log.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <vector>

bool gemini::IsAvailable() const {
    return !api_key.empty() && !_enabledTypes.empty();
}

void gemini::SaveSettings() const {

    _sm->setServiceSetting("GeminiModel", model);
    _sm->setSecretServiceToken("GeminiApiKey", api_key);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("GeminiEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
    spdlog::info("Gemini settings saved successfully");
}

void gemini::LoadSettings() {
    model = _sm->getServiceSetting("GeminiModel", model);
    api_key = _sm->getSecretServiceToken("GeminiApiKey");
    bool oldEnabled = _sm->getServiceSetting("GeminiEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("GeminiEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

std::vector<ServiceProperty> gemini::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "Gemini", "Gemini", {}, {}, {} });
    for (auto t : GetTypes()) {
        props.push_back({
            ServiceProperty::Kind::Bool,
            std::string("Gemini.Enable_") + aiType::TypeSettingsSuffix(t),
            std::string("Enable ") + aiType::TypeName(t),
            "Gemini",
            {},
            {},
            IsEnabledForType(t)
        });
    }
    props.push_back({ ServiceProperty::Kind::Secret, "Gemini.Key", "API Key", "Gemini",
                     "Your Google Gemini API key (masked for security)", {}, api_key });
    props.push_back({ ServiceProperty::Kind::String, "Gemini.Model", "Model", "Gemini", {}, {}, model });
    return props;
}

void gemini::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("Gemini.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
}

void gemini::SetProperty(const std::string& id, const std::string& value) {
    if (id == "Gemini.Key") {
        api_key = value;
    } else if (id == "Gemini.Model") {
        model = value;
    }
}

std::pair<std::string, bool> gemini::CallLLM(const std::string& prompt) const {

    std::string apiKey = api_key;

    if (apiKey.empty()) {
        apiKey = _sm->getSecretServiceToken("GeminiApiKey");
    }

    if (api_key.empty() && apiKey.empty()) {
        return { "Gemini: API Key is empty", false };
    }

    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] }";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + apiKey }
    };

    spdlog::debug("Gemini: {}", request);

    int responseCode = 0;
    std::string response = CurlManager::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    spdlog::debug("Gemini Response {}: {}", responseCode, response);

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        spdlog::error("Gemini: Invalid JSON response: {}", response);
        return { "Gemini: Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        spdlog::error("Gemini: No choices in response");
        return { "Gemini: No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        spdlog::error("Gemini: No text in response");
        return { "Gemini: No text in response", false };
    }

    response = text.get<std::string>();
    spdlog::debug("Gemini: {}", response);

    return { response, true };
}

namespace {

constexpr const char* kGeminiStyleId = "Gemini.Style";
constexpr const char* kGeminiStyleCategory = "Gemini Image";

class GeminiImageGenerator : public aiBase::AIImageGenerator {
private:
    std::string api_key;
    std::string model;
    std::string style = "cartoon";

public:
    GeminiImageGenerator(std::string apiKey, std::string modelName)
        : api_key(std::move(apiKey)), model(std::move(modelName)) {}

    ~GeminiImageGenerator() override = default;

    std::vector<ServiceProperty> GetProperties() const override {
        ServiceProperty p;
        p.kind = ServiceProperty::Kind::Choice;
        p.id = kGeminiStyleId;
        p.label = "Style";
        p.category = kGeminiStyleCategory;
        p.choices = { "cartoon", "sticker", "outline", "line art / neon glow" };
        p.value = style;
        return { p };
    }

    void SetProperty(const std::string& id, const std::string& value) override {
        if (id == kGeminiStyleId) {
            style = Lower(value);
        }
    }

    void generateImage(const std::string& prompt,
                       std::function<void(aiBase::AIImageResult)> cb) override {

        aiBase::AIImageResult result;

        if (prompt.empty()) {
            result.error = "Prompt cannot be empty";
            cb(std::move(result));
            return;
        }

        if (api_key.empty()) {
            result.error = "Gemini API key not configured";
            cb(std::move(result));
            return;
        }

        std::string styleDesc;
        if (style == "cartoon") {
            styleDesc = "in vibrant cartoon style, clean lines, cel-shaded, fun and exaggerated features";
        } else if (style == "sticker") {
            styleDesc = "as a cute sticker, bold flat colors, thick black outline, simple background";
        } else if (style == "outline") {
            styleDesc = "clean thick vector outline art, minimal details, high contrast lines only";
        } else if (style == "line art / neon glow") {
            styleDesc = "neon glow line art, cyberpunk aesthetic, glowing colored lines on dark background";
        } else {
            styleDesc = "in detailed illustrative style";
        }

        std::string fullPrompt =
            "Background: Pure Black background (#000000) with no watermarks or border. Design: 2D Vector Style. "
            "No black outlines; use soft, colored outlines that match the object's palette. "
            "Style: A minimalist, flat 2D pixel art illustration using a 8-bit look and minimalist facial features. "
            "Non-anthropomorphic (no faces or eyes). "
            "Details: Simple cel-shading, limited vibrant color palette, clean edges, and no gradients. "
            "Ensure the lines are perfectly horizontal and vertical. Center the subject. " + styleDesc +
            " Subject: " + prompt;

        Replace(fullPrompt, std::string("\t"), std::string(" "));
        Replace(fullPrompt, std::string("\r"), std::string(""));

        std::string imgModel = "gemini-2.5-flash-image";  // Correct model name
        std::string endpoint = "https://generativelanguage.googleapis.com/v1beta/models/" + imgModel + ":generateContent";

        nlohmann::json req;
        req["contents"][0]["parts"][0]["text"] = fullPrompt;
        req["generationConfig"]["responseModalities"] = nlohmann::json::array({"IMAGE"});

        std::string jsonBody = req.dump();

        std::vector<std::pair<std::string, std::string>> headers = {
            {"x-goog-api-key", api_key},
            {"Content-Type", "application/json"}
        };

        spdlog::debug("Gemini image request: {}", jsonBody);

        int httpCode = 0;
        std::string response = CurlManager::HTTPSPost(endpoint, jsonBody, "", "", "JSON", 120, headers, &httpCode);

        spdlog::debug("Gemini image response code: {}, body length: {}", httpCode, response.size());

        if (httpCode != 200) {
            result.error = "Gemini API error " + std::to_string(httpCode) + ": " + response.substr(0, 300);
            cb(std::move(result));
            return;
        }

        nlohmann::json root;
        try {
            root = nlohmann::json::parse(response);
        } catch (const std::exception& e) {
            spdlog::error("Gemini image JSON parse failed: {}", e.what());
            result.error = "Invalid response from Gemini";
            cb(std::move(result));
            return;
        }

        std::string base64Data;
        try {
            if (root.contains("candidates") && root["candidates"].is_array() && !root["candidates"].empty()) {
                auto parts = root["candidates"][0]["content"]["parts"];
                for (const auto& part : parts) {
                    if (part.contains("inlineData") && part["inlineData"].contains("data")) {
                        base64Data = part["inlineData"]["data"].get<std::string>();
                        break;
                    }
                }
            }
        } catch (...) {
            std::string errMsg = root.contains("error") ? root["error"]["message"].get<std::string>() : "No image data found";
            spdlog::error("Gemini image: {}", errMsg);
            result.error = "Gemini: " + errMsg;
            cb(std::move(result));
            return;
        }

        if (base64Data.empty()) {
            result.error = "No image data found in response";
            cb(std::move(result));
            return;
        }

        // Remove whitespace from base64 data
        base64Data.erase(std::remove_if(base64Data.begin(), base64Data.end(),
                                        [](unsigned char c) { return std::isspace(c); }),
                         base64Data.end());

        result.pngBytes = Base64::Decode(base64Data);
        if (result.pngBytes.empty()) {
            result.error = "Base64 decode failed";
        }
        cb(std::move(result));
    }
};

} // namespace

aiBase::AIImageGenerator* gemini::createAIImageGenerator() const {
    return new GeminiImageGenerator(api_key, model);
}
