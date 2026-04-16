#include "ServiceManager.h"
#include "UtilFunctions.h"
#include "OpenAIAPI.h"
#include "utils/CurlManager.h"
#include <nlohmann/json.hpp>

#include "OpenAIImageGenerator.h"

#include <string>
#include <vector>

#include <log.h>
#include <wx/msgdlg.h>

constexpr const char* completion_url = "/chat/completions";

constexpr const char* transcriptions_url = "/audio/transcriptions";

std::pair<std::string, bool> OpenAIAPI::CallLLM(const std::string& prompt) const {
    std::string bearerToken = token;

    if (bearerToken.empty()) {
        wxMessageBox("You must set a " + GetLLMName() + " Bearer Token in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { GetLLMName() + ": Bearer Token is empty", false };
    }

    // remove all \t, \r and \n as chatGPT does not like it
    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));
    Replace(p, std::string("\n"), std::string("\\n"));

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] }";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + bearerToken }
    };

    spdlog::debug("{}: {}", GetLLMName(), request);

    int responseCode = 0;
    std::string response = CurlManager::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    spdlog::debug("{} Response {}: {}", GetLLMName(), responseCode, response);

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        spdlog::error("{}: Invalid JSON response: {}", GetLLMName(), response);
        return { GetLLMName() +  ": Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        spdlog::error("{}: No choices in response", GetLLMName());
        return { GetLLMName() + ": No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        spdlog::error("{}: No text in response", GetLLMName());
        return { GetLLMName() + ": No text in response", false };
    }

    response = text.get<std::string>();
    spdlog::debug("{}: {}", GetLLMName(), response);

    return { response, true };
}

aiBase::AIColorPalette OpenAIAPI::GenerateColorPalette(const std::string& prompt) const {
    aiBase::AIColorPalette ret;
    if (token.empty()) {
        ret.error = "You must set a " + GetLLMName() + " Bearer Token in the Preferences on the Services Panel";
        return ret;
    }

    std::string fullprompt = "xlights color palettes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors.";

    std::string schema = R"(
    "response_format" : {
        "type": "json_schema",
        "json_schema": {
            "name": "color_palette",
            "schema": {
                "type": "object",
                "properties":  {
                    "description": {
                        "type": "string"
                    },
                    "colors": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "hex_code": {
                                    "type": "string"
                                },
                                "name": {
                                    "type": "string"
                                },
                                "usage_notes": {
                                    "type": "string"
                                }
                            },
                            "required": [
                                "hex_code", "name", "usage_notes"
                            ]
                        }
                    }
                },
                "required": [
                    "description", "colors"
                ]
            }
        }
    }
    )";

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(fullprompt) + "\" } ]," + schema + "}";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + token }
    };

    spdlog::debug("{}: {}", GetLLMName(), request);

    int responseCode = 0;
    std::string const response = CurlManager::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    spdlog::debug("{} Response {}: {}", GetLLMName(), responseCode, response);
    if (responseCode != 200) {
        ret.error = response;
    }
    spdlog::debug("{} Response: {}", GetLLMName(), response);
    try {
        nlohmann::json root = nlohmann::json::parse(response);

        if (root.contains("choices") && root["choices"].is_array() && root["choices"].size() > 0 && root["choices"][0].contains("message")) {
            auto const color_responce = root["choices"][0]["message"]["content"].get<std::string>();

            spdlog::debug("{} Content {}", GetLLMName(), color_responce);
            try {
                // Check if the response is valid JSON
                nlohmann::json const color_root = nlohmann::json::parse(color_responce);
                if (color_root.contains("colors") && color_root["colors"].is_array()) {
                    aiBase::AIColorPalette ret;
                    ret.description = prompt;
                    if (color_root.contains("description")) {
                        ret.description = color_root["description"].get<std::string>();
                    }
                    for (size_t x = 0; x < color_root["colors"].size(); x++) {
                        auto& color = color_root["colors"][x];
                        ret.colors.push_back(aiBase::AIColor());
                        ret.colors.back().hexValue = color["hex_code"].get<std::string>();
                        ret.colors.back().description = color["usage_notes"].get<std::string>();
                        ret.colors.back().name = color["name"].get<std::string>();
                    }
                    return ret;
                }
            } catch (const std::exception& ex) {
                spdlog::error("{}", ex.what());
            }
            spdlog::error("Response does not contain 'colors' array or is not in expected format.");
            ret.error = "Response does not contain 'colors' array or is not in expected format.";
        } else {
            spdlog::error("Invalid response from {} API.", GetLLMName());
            ret.error = "Invalid response from " + GetLLMName() + " API.";
        }
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
    }

    return ret;
}

aiBase::AIImageGenerator* OpenAIAPI::createAIImageGenerator() const {
    return new OpenAIImageGenerator(base_url, token, image_model);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//https://api.openai.com/v1/audio/transcriptions
aiBase::AILyricTrack OpenAIAPI::GenerateLyricTrack(const std::string& audioPath) const {
    AILyricTrack ret;
    if (token.empty()) {
        ret.error = "You must set a " + GetLLMName() + " Bearer Token in the Preferences on the Services Panel";
        return ret;
    }

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_mime* form = curl_mime_init(curl);

        // 1. Add file
        curl_mimepart* field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, audioPath.c_str());

        // 2. Specify model
        field = curl_mime_addpart(form);
        curl_mime_name(field, "model");
        curl_mime_data(field, transcribe_model.c_str(), CURL_ZERO_TERMINATED);

        // 3. Set format to verbose_json to get timestamps
        field = curl_mime_addpart(form);
        curl_mime_name(field, "response_format");
        curl_mime_data(field, "verbose_json", CURL_ZERO_TERMINATED);

        // 4. Request word-level timestamps
        field = curl_mime_addpart(form);
        curl_mime_name(field, "timestamp_granularities[]");
        curl_mime_data(field, "word", CURL_ZERO_TERMINATED);

        struct curl_slist* headerlist = NULL;
        std::string authHeader = "Authorization: Bearer " + token;
        headerlist = curl_slist_append(headerlist, authHeader.c_str());

        std::string full_url = base_url + transcriptions_url;

        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            spdlog::debug("OpenAI transcription response: {}", readBuffer);
            try {
                nlohmann::json root = nlohmann::json::parse(readBuffer);
                if (root.contains("error")) {
                    ret.error = root["error"]["message"].get<std::string>();
                } else if (root.contains("words") && root["words"].is_array()) {
                    // top-level words array (gpt-4o-transcribe with word granularity)
                    for (const auto& word : root["words"]) {
                        AILyric lyric;
                        lyric.word = word["word"].get<std::string>();
                        lyric.startMS = static_cast<int>(word["start"].get<double>() * 1000);
                        lyric.endMS = static_cast<int>(word["end"].get<double>() * 1000);
                        ret.lyrics.push_back(lyric);
                    }
                } else if (root.contains("segments") && root["segments"].is_array()) {
                    for (const auto& segment : root["segments"]) {
                        if (segment.contains("words") && segment["words"].is_array()) {
                            for (const auto& word : segment["words"]) {
                                AILyric lyric;
                                lyric.word = word["word"].get<std::string>();
                                lyric.startMS = static_cast<int>(word["start"].get<double>() * 1000);
                                lyric.endMS = static_cast<int>(word["end"].get<double>() * 1000);
                                ret.lyrics.push_back(lyric);
                            }
                        }
                    }
                } else {
                    ret.error = "Response does not contain 'words' or 'segments' array.";
                }
            } catch (const nlohmann::json::parse_error& e) {
                ret.error = e.what();
            } catch (const std::exception& e) {
                ret.error = e.what();
            }
        } else {
            ret.error = "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        curl_mime_free(form);
        curl_slist_free_all(headerlist);
    }
    curl_global_cleanup();

    return ret;
}