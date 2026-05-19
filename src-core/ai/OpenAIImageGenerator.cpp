#include "ai/OpenAIImageGenerator.h"

#include "utils/Base64.h"
#include "utils/CurlManager.h"
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>

#include <log.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

constexpr const char* img_gen_api = "/images/generations";

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
