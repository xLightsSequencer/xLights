#include "OpenAIImageGenerator.h"

#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <nlohmann/json.hpp>

#include <log.h>

#include <wx/mstream.h>
#include <wx/base64.h>

#include <vector>
#include <string>

constexpr const char* img_gen_api = "/images/generations";

void OpenAIImageGenerator::generateImage(const std::string& prompt,
                               const std::function<void(const wxBitmap&, const std::string&)>& cb) 
{
    if (prompt.empty()) {
        cb(wxBitmap(), "Prompt cannot be empty");
        return;
    }

    if (token.empty()) {
        cb(wxBitmap(), "OpenAI token key not configured");
        return;
    }

    std::string fullPrompt = prompt;

    Replace(fullPrompt, std::string("\t"), std::string(" "));
    Replace(fullPrompt, std::string("\r"), std::string(""));
    //https://api.openai.com/v1/images/generations
    std::string const endpoint = base_url + img_gen_api ;

    nlohmann::json req;
    req["model"] = image_model;
    req["prompt"] = fullPrompt;
    req["n"] = 1;
    //req["size"] = "256x256";
    req["response_format"] = "b64_json";
 
    std::string jsonBody = req.dump();

    std::vector<std::pair<std::string, std::string>> headers = {
        { "Authorization", "Bearer " + token },
        { "Content-Type", "application/json" }
    };

    spdlog::debug("OpenAI image request: {}", jsonBody.c_str());

    int httpCode {0};
    std::string const response = Curl::HTTPSPost(endpoint, jsonBody, "", "", "JSON", 120, headers, &httpCode);

    spdlog::debug("OpenAI image response code: {}, body length: {}", httpCode, response.size());

    if (httpCode != 200) {
        cb(wxBitmap(), "OpenAI API error " + std::to_string(httpCode) + ": " + response.substr(0, 300));
        return;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception& e) {
        spdlog::error("OpenAI image JSON parse failed: {}", e.what());
        cb(wxBitmap(), "Invalid response from OpenAI");
        return;
    }

    std::string base64Data;
    try {
        //std::string base64_image = j["data"][0]["b64_json"];
        if (root.contains("data") && root["data"].is_array() && !root["data"].empty()) {
            if (root["data"][0].contains("b64_json") && root["data"][0]["b64_json"].is_string()) {
                base64Data = root["data"][0]["b64_json"].get<std::string>();                               
            }
        }
    } catch (...) {
        std::string errMsg = "No image data found";
        spdlog::error("OpenAI image: {}", errMsg.c_str());
        cb(wxBitmap(), "OpenAI: " + errMsg);
        return;
    }

    if (base64Data.empty()) {
        cb(wxBitmap(), "No image data found in response");
        return;
    }

    // Remove whitespace from base64 data
    base64Data.erase(std::remove_if(base64Data.begin(), base64Data.end(), ::isspace), base64Data.end());

    wxMemoryBuffer decoded = wxBase64Decode(base64Data.c_str(), base64Data.length());
    if (decoded.GetDataLen() == 0) {
        cb(wxBitmap(), "Base64 decode failed");
        return;
    }

    wxMemoryInputStream memStream(decoded.GetData(), decoded.GetDataLen());
    wxImage wxImg(memStream, wxBITMAP_TYPE_PNG);

    if (!wxImg.IsOk()) {
        cb(wxBitmap(), "Failed to load generated image");
        return;
    }

    spdlog::debug("Generated image size: {}x{}", wxImg.GetWidth(), wxImg.GetHeight());
    wxBitmap bmp(wxImg);
    cb(bmp, "");
}
