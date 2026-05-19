#include "ai/ollama.h"
#include "ai/ServiceManager.h"

#include "utils/CurlManager.h"
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"

#include <nlohmann/json.hpp>

#include <log.h>

#include <string>
#include <vector>

bool ollama::IsAvailable() const {
    return !host.empty() && !_enabledTypes.empty();
}

void ollama::SaveSettings() const {
    _sm->setServiceSetting("ollamaHost", host);
    _sm->setServiceSetting("ollamaPort", port_num);
    _sm->setServiceSetting("ollamaModel", model);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("ollamaEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
}

void ollama::LoadSettings() {
    host = _sm->getServiceSetting("ollamaHost", host);
    model = _sm->getServiceSetting("ollamaModel", model);
    port_num = _sm->getServiceSetting("ollamaPort", port_num);
    bool oldEnabled = _sm->getServiceSetting("ollamaEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("ollamaEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

std::vector<ServiceProperty> ollama::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "ollama", "ollama", {}, {}, {} });
    for (auto t : GetTypes()) {
        props.push_back({
            ServiceProperty::Kind::Bool,
            std::string("ollama.Enable_") + aiType::TypeSettingsSuffix(t),
            std::string("Enable ") + aiType::TypeName(t),
            "ollama",
            {},
            {},
            IsEnabledForType(t)
        });
    }
    props.push_back({ ServiceProperty::Kind::String, "ollama.Host", "Host", "ollama", {}, {}, host });
    props.push_back({ ServiceProperty::Kind::Int, "ollama.Port", "Port", "ollama", {}, {}, port_num });
    props.push_back({ ServiceProperty::Kind::Bool, "ollama.Https", "Https", "ollama", {}, {}, https });
    props.push_back({ ServiceProperty::Kind::String, "ollama.Model", "Model", "ollama", {}, {}, model });
    return props;
}

void ollama::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("ollama.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
    if (id == "ollama.Https") {
        https = value;
    }
}

void ollama::SetProperty(const std::string& id, int value) {
    if (id == "ollama.Port") {
        port_num = value;
    }
}

void ollama::SetProperty(const std::string& id, const std::string& value) {
    if (id == "ollama.Host") {
        host = value;
    } else if (id == "ollama.Model") {
        model = value;
    }
}

std::pair<std::string, bool> ollama::CallLLM(const std::string& prompt) const {

	if (host.empty()) {
		spdlog::error("ollama: host is empty");
		return {"ollama: host is empty", false};
    }
    std::string const reqUrl = (https ? "https://" : "http://") + host + ":" + std::to_string(port_num) + api;

	// remove all \t, \r and \n as ollama does not like it
    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));
    Replace(p, std::string("\n"), std::string("\\n"));
    std::string const request = "{ \"model\": \"" + model + "\", \"prompt\": \"" + JSONSafe(p) + "\",\"stream\": false }";

    spdlog::debug("ollama: {}", request);
    int responseCode { 0 };
	std::string response = CurlManager::HTTPSPost(reqUrl, request, "", "", "JSON", 60 * 10, {}, &responseCode);

    spdlog::debug("ollama Response {}: {}", responseCode, response);

	if (responseCode != 200) {
        return { response , false};
    }
    try {
        // Check if the response is valid JSON
        nlohmann::json const resp_json = nlohmann::json::parse(response);
        if (resp_json.contains("response")) {
            response = resp_json.at("response").get<std::string>();
        }
    } catch (const std::exception& ex) {
        spdlog::error("ollama: Invalid JSON response: {}", ex.what());
        return { "ollama: Invalid JSON response", false };
    }

	spdlog::debug("ollama: {}", response);

    return { response, true};
}

aiBase::AIColorPalette ollama::GenerateColorPalette(const std::string& prompt) const {

    if (host.empty()) {
        spdlog::error("ollama: host is empty");
        return {};
    }
    std::string const reqUrl = (https ? "https://" : "http://") + host + ":" + std::to_string(port_num) + api;

    auto fullprompt = "xlights color palettes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors. Include the hex_code and usage_notes.";

    nlohmann::json request_payload;
    request_payload["model"] = model;
    request_payload["temperature"] = 0;
    request_payload["prompt"] = fullprompt;
    request_payload["stream"] = false;

    // Include the structured output format (JSON schema)

    std::string schema = R"(
    {
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
    )";

    nlohmann::json format_schema = nlohmann::json::parse(schema);

    request_payload["format"] = format_schema;

    auto json_payload_str = request_payload.dump(3);

    spdlog::debug("ollama: {}", json_payload_str);
    int responseCode{ 0 };
    std::string const response = CurlManager::HTTPSPost(reqUrl, json_payload_str, "", "", "JSON", 60 * 10, {}, &responseCode);

    spdlog::debug("ollama Response {}: {}", responseCode, response);

    if (responseCode != 200) {
        return {};
    }

    try {
        nlohmann::json root = nlohmann::json::parse(response);

        if (root.contains("response") && root["response"].is_string()) {
            auto const color_responce = root["response"].get<std::string>();

            spdlog::debug("ollama Response {}", color_responce);
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
                spdlog::error(ex.what());
            }
            spdlog::error("Response does not contain 'colors' array or is not in expected format.");
        } else {
            spdlog::error("Invalid response from Ollama API.");
        }
    } catch (const std::exception& e) {
        spdlog::error(e.what());
    }
    return {};
}
