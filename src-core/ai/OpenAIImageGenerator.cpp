#include "ai/OpenAIImageGenerator.h"

#include "utils/Base64.h"
#include "utils/CurlManager.h"
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>

#include <log.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>

constexpr const char* img_gen_api = "/images/generations";

constexpr const char* kImageModelId = "OpenAIImage.Model";
constexpr const char* kImageSizeId = "OpenAIImage.Size";
constexpr const char* kImageStepsId = "OpenAIImage.Steps";
constexpr const char* kImageCategory = "Image";

std::vector<ServiceProperty> OpenAIImageGenerator::GetProperties() const {
    std::vector<ServiceProperty> props;

    // Model picker.
    ServiceProperty model;
    model.id = kImageModelId;
    model.label = "Model";
    model.category = kImageCategory;
    model.value = image_model;
    if (available_models.empty()) {
        // No model list available — let the user type one.
        model.kind = ServiceProperty::Kind::String;
    } else {
        model.kind = ServiceProperty::Kind::Choice;
        model.choices = available_models;
        // Keep the current model selectable even if the server didn't list it.
        if (!image_model.empty() && std::find(model.choices.begin(), model.choices.end(), image_model) == model.choices.end()) {
            model.choices.insert(model.choices.begin(), image_model);
        }
    }
    props.push_back(model);

    // Output size. "auto" is omitted from the request (server default). The
    // smaller squares (512/768) are common Stable Diffusion sizes (the Generic
    // client defaults to SD-Turbo); the larger options match gpt-image-1.
    ServiceProperty size;
    size.kind = ServiceProperty::Kind::Choice;
    size.id = kImageSizeId;
    size.label = "Size";
    size.category = kImageCategory;
    size.choices = { "auto", "512x512", "768x768", "1024x1024", "1536x1024", "1024x1536" };
    if (!image_size.empty() && std::find(size.choices.begin(), size.choices.end(), image_size) == size.choices.end()) {
        size.choices.insert(size.choices.begin(), image_size);
    }
    size.value = image_size;
    props.push_back(size);

    // Diffusion steps. "auto" is omitted from the request (server default).
    // SD-Turbo needs very few (1-4); regular Stable Diffusion uses 20-50.
    ServiceProperty steps;
    steps.kind = ServiceProperty::Kind::Choice;
    steps.id = kImageStepsId;
    steps.label = "Steps";
    steps.category = kImageCategory;
    steps.choices = { "auto", "1", "2", "4", "8", "20", "30", "50" };
    if (!image_steps.empty() && std::find(steps.choices.begin(), steps.choices.end(), image_steps) == steps.choices.end()) {
        steps.choices.insert(steps.choices.begin(), image_steps);
    }
    steps.value = image_steps;
    props.push_back(steps);

    return props;
}

void OpenAIImageGenerator::SetProperty(const std::string& id, const std::string& value) {
    if (id == kImageModelId) {
        image_model = value;
    } else if (id == kImageSizeId) {
        image_size = value;
    } else if (id == kImageStepsId) {
        image_steps = value;
    }
}

void OpenAIImageGenerator::generateImage(const std::string& prompt,
                               std::function<void(aiBase::AIImageResult)> cb)
{
    aiBase::AIImageResult result;
    if (prompt.empty()) {
        result.error = "Prompt cannot be empty";
        cb(std::move(result));
        return;
    }

    if (token.empty()) {
        result.error = "OpenAI token key not configured";
        cb(std::move(result));
        return;
    }

    std::string fullPrompt = prompt;

    Replace(fullPrompt, std::string("\t"), std::string(" "));
    Replace(fullPrompt, std::string("\r"), std::string(""));
    std::string const endpoint = base_url + img_gen_api;

    nlohmann::json req;
    req["model"] = image_model;
    req["prompt"] = fullPrompt;
    req["n"] = 1;
    req["response_format"] = "b64_json";
    if (!image_size.empty() && image_size != "auto") {
        req["size"] = image_size;
    }
    if (!image_steps.empty() && image_steps != "auto") {
        // strtol (not std::stoi) — codebase avoids throwing conversions.
        long const steps = std::strtol(image_steps.c_str(), nullptr, 10);
        if (steps > 0) {
            req["steps"] = static_cast<int>(steps);
        }
    }

    std::string jsonBody = req.dump();

    std::vector<std::pair<std::string, std::string>> headers = {
        { "Authorization", "Bearer " + token },
        { "Content-Type", "application/json" }
    };

    spdlog::debug("OpenAI image request: {}", jsonBody);

    int httpCode { 0 };
    std::string const response = CurlManager::HTTPSPost(endpoint, jsonBody, "", "", "JSON", 120, headers, &httpCode);

    spdlog::debug("OpenAI image response code: {}, body length: {}", httpCode, response.size());

    if (httpCode != 200) {
        result.error = "OpenAI API error " + std::to_string(httpCode) + ": " + response.substr(0, 300);
        cb(std::move(result));
        return;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception& e) {
        spdlog::error("OpenAI image JSON parse failed: {}", e.what());
        result.error = "Invalid response from OpenAI";
        cb(std::move(result));
        return;
    }

    std::string base64Data;
    try {
        if (root.contains("data") && root["data"].is_array() && !root["data"].empty()) {
            if (root["data"][0].contains("b64_json") && root["data"][0]["b64_json"].is_string()) {
                base64Data = root["data"][0]["b64_json"].get<std::string>();
            }
        }
    } catch (...) {
        std::string errMsg = "No image data found";
        spdlog::error("OpenAI image: {}", errMsg);
        result.error = "OpenAI: " + errMsg;
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
