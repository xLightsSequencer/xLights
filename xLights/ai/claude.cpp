#include "claude.h"
#include <nlohmann/json.hpp>
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>
#include <wx/msgdlg.h>

#include <set>
#include <vector>
#include <string>

#include <log4cpp/Category.hh>

bool claude::IsAvailable() const {
    return !api_key.empty() && !_enabledTypes.empty();
}

void claude::SaveSettings() const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _sm->setServiceSetting("ClaudeModel", claudeModel);
    _sm->setSecretServiceToken("ClaudeApiKey", api_key);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("ClaudeEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
    logger_base.info("Claude settings saved successfully");
}

void claude::LoadSettings() {
    claudeModel = _sm->getServiceSetting("ClaudeModel", claudeModel);
    api_key = _sm->getSecretServiceToken("ClaudeApiKey");
    bool oldEnabled = _sm->getServiceSetting("ClaudeEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("ClaudeEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

void claude::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Claude"));
    for (auto t : GetTypes()) {
        auto p = page->Append(new wxBoolProperty(wxString("Enable ") + aiType::TypeName(t),
                                                  wxString("Claude.Enable_") + aiType::TypeSettingsSuffix(t),
                                                  IsEnabledForType(t)));
        p->SetEditor("CheckBox");
    }
    auto* apiKeyProp = page->Append(new wxStringProperty("API Key", "Claude.Key", api_key));
    apiKeyProp->SetAttribute(wxPG_STRING_PASSWORD, true);
    apiKeyProp->SetHelpString("Your Anthropic API key (masked for security)");
    page->Append(new wxStringProperty("Model", "Claude.Model", claudeModel));
}

void claude::SetSetting(const std::string& key, const wxVariant& value) {
    for (auto t : GetTypes()) {
        if (key == std::string("Claude.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value.GetBool());
            return;
        }
    }
    if (key == "Claude.Key") {
        api_key = value.GetString().ToStdString();
    } else if (key == "Claude.Model") {
        claudeModel = value.GetString().ToStdString();
    }
}

static std::pair<std::string, bool> CallAnthropicAPI(const std::string& base_url, const std::string& api_key, const std::string& model,
                                    const nlohmann::json& messages, int max_tokens, const std::string& serviceName,
                                    const std::string& system_prompt = "") {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    nlohmann::json requestJson;
    requestJson["model"] = model;
    requestJson["max_tokens"] = max_tokens;
    if (!system_prompt.empty()) {
        requestJson["system"] = system_prompt;
    }
    requestJson["messages"] = messages;

    std::string request = requestJson.dump();

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "x-api-key", api_key },
        { "anthropic-version", "2023-06-01" }
    };

    logger_base.debug("%s: %s", serviceName.c_str(), request.c_str());

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(base_url + "/messages", request, "", "", "JSON", 120, customHeaders, &responseCode);

    logger_base.debug("%s Response %d: %s", serviceName.c_str(), responseCode, response.c_str());

    if (responseCode != 200) {
        // Try to extract error message from response
        std::string errorMsg = serviceName + ": HTTP " + std::to_string(responseCode);
        try {
            auto errJson = nlohmann::json::parse(response);
            if (errJson.contains("error") && errJson["error"].contains("message")) {
                errorMsg += " - " + errJson["error"]["message"].get<std::string>();
            }
        } catch (...) {
            if (!response.empty()) {
                errorMsg += " - " + response.substr(0, 200);
            }
        }
        logger_base.error("%s", errorMsg.c_str());
        return { errorMsg, false };
    }

    return { response, true };
}

// Strip markdown code fences (```json ... ```) and extract the JSON object/array
static std::string StripCodeFences(const std::string& text) {
    std::string result = text;
    if (result.find("```") != std::string::npos) {
        auto start = result.find('{');
        auto startArr = result.find('[');
        // Use whichever comes first: { or [
        if (startArr != std::string::npos && (start == std::string::npos || startArr < start)) {
            start = startArr;
            auto end = result.rfind(']');
            if (start != std::string::npos && end != std::string::npos) {
                result = result.substr(start, end - start + 1);
            }
        } else if (start != std::string::npos) {
            auto end = result.rfind('}');
            if (end != std::string::npos) {
                result = result.substr(start, end - start + 1);
            }
        }
    }
    return result;
}

static std::string ExtractTextContent(const std::string& response, const std::string& serviceName) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        logger_base.error("%s: Invalid JSON response: %s", serviceName.c_str(), response.c_str());
        return "";
    }

    auto content = root["content"];
    if (content.is_null() || !content.is_array() || content.empty()) {
        logger_base.error("%s: No content in response", serviceName.c_str());
        return "";
    }

    for (const auto& block : content) {
        if (block.contains("type") && block["type"] == "text" && block.contains("text")) {
            return block["text"].get<std::string>();
        }
    }

    logger_base.error("%s: No text block in response content", serviceName.c_str());
    return "";
}

std::pair<std::string, bool> claude::CallLLM(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (api_key.empty()) {
        wxMessageBox("You must set a Claude API Key in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { "Claude: API Key is empty", false };
    }

    nlohmann::json messages = nlohmann::json::array();
    nlohmann::json userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    messages.push_back(userMsg);

    auto [response, success] = CallAnthropicAPI(base_url, api_key, claudeModel, messages, 4096, "Claude");
    if (!success) {
        return { response, false };
    }

    std::string text = ExtractTextContent(response, "Claude");
    if (text.empty()) {
        return { "Claude: No text in response", false };
    }

    logger_base.debug("Claude: %s", text.c_str());
    return { text, true };
}

aiBase::AIColorPalette claude::GenerateColorPalette(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    aiBase::AIColorPalette ret;

    if (api_key.empty()) {
        ret.error = "You must set a Claude API Key in the Preferences on the Services Panel";
        return ret;
    }

    std::string fullprompt = "xlights color palettes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors.\n\nRespond with ONLY valid JSON in this exact format:\n{\"description\": \"palette description\", \"colors\": [{\"hex_code\": \"#RRGGBB\", \"name\": \"color name\", \"usage_notes\": \"how to use\"}]}";

    nlohmann::json messages = nlohmann::json::array();
    nlohmann::json userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = fullprompt;
    messages.push_back(userMsg);

    auto [response, success] = CallAnthropicAPI(base_url, api_key, claudeModel, messages, 2048, "Claude");
    if (!success) {
        ret.error = response;
        return ret;
    }

    std::string text = ExtractTextContent(response, "Claude");
    if (text.empty()) {
        ret.error = "No text in Claude response";
        return ret;
    }

    text = StripCodeFences(text);

    try {
        nlohmann::json color_root = nlohmann::json::parse(text);
        if (color_root.contains("colors") && color_root["colors"].is_array()) {
            ret.description = prompt;
            if (color_root.contains("description")) {
                ret.description = color_root["description"].get<std::string>();
            }
            for (const auto& color : color_root["colors"]) {
                ret.colors.push_back(aiBase::AIColor());
                ret.colors.back().hexValue = color["hex_code"].get<std::string>();
                ret.colors.back().description = color["usage_notes"].get<std::string>();
                ret.colors.back().name = color["name"].get<std::string>();
            }
            return ret;
        }
        ret.error = "Response does not contain 'colors' array.";
    } catch (const std::exception& ex) {
        logger_base.error("Claude color palette parse error: %s", ex.what());
        ret.error = "Failed to parse color palette response.";
    }

    return ret;
}

aiBase::AIModelMappingResult claude::GenerateModelMapping(
    const std::vector<MappingModelInfo>& sourceModels,
    const std::vector<MappingModelInfo>& targetModels,
    const std::map<std::string, std::string>& existingMappings) const {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    AIModelMappingResult result;

    if (api_key.empty()) {
        result.error = "You must set a Claude API Key in the Preferences on the Services Panel";
        return result;
    }

    // Build a structured prompt with all the model information
    std::string system = "You are an expert at mapping xLights lighting models between sequences. "
        "xLights is a lighting sequencer for holiday displays. Models represent physical lighting elements like trees, arches, snowflakes, candy canes, matrices, DMX fixtures, etc.";

    std::string prompt = "Your task: Map each target model to the best matching source model. The goal is to transfer lighting effects from the source sequence to the target layout.\n\n";

    // Helper to format structural details for a model
    auto formatModelDetails = [](const MappingModelInfo& m, bool includeEffects) -> std::string {
        std::string s = "- name=\"" + m.name + "\" type=\"" + m.type + "\" class=\"" + m.modelClass + "\"";
        if (m.nodeCount > 0) s += " nodes=" + std::to_string(m.nodeCount);
        if (m.strandCount > 1) s += " strands=" + std::to_string(m.strandCount);
        if (m.width > 0 && m.height > 0) s += " size=" + std::to_string(m.width) + "x" + std::to_string(m.height);
        if (includeEffects && m.effectCount > 0) s += " effects=" + std::to_string(m.effectCount);
        if (!m.groupModels.empty()) {
            s += " groupModels=\"" + m.groupModels + "\"";
        }
        if (m.isSubModel) s += " [submodel]";
        if (m.isStrand) s += " [strand]";
        if (m.isNode) s += " [node]";
        if (!m.aliases.empty()) {
            s += " aliases=\"";
            for (size_t i = 0; i < m.aliases.size(); ++i) {
                if (i > 0) s += ",";
                s += m.aliases[i];
            }
            s += "\"";
        }
        if (!m.subModelNames.empty()) {
            s += " submodels=\"";
            for (size_t i = 0; i < m.subModelNames.size(); ++i) {
                if (i > 0) s += ",";
                s += m.subModelNames[i];
            }
            s += "\"";
        }
        return s + "\n";
    };

    prompt += "## Source Models (from imported sequence):\n";
    for (const auto& m : sourceModels) {
        prompt += formatModelDetails(m, true);
    }

    prompt += "\n## Target Models (current layout, need mappings):\n";
    for (const auto& m : targetModels) {
        prompt += formatModelDetails(m, false);
    }

    if (!existingMappings.empty()) {
        prompt += "\n## Already Mapped (for context, do NOT remap these):\n";
        for (const auto& [target, source] : existingMappings) {
            prompt += "- \"" + target + "\" -> \"" + source + "\"\n";
        }
    }

    prompt += "\n## Mapping Rules (in priority order):\n";
    prompt += "1. Models with exactly the same name MUST be mapped to each other first.\n";
    prompt += "2. Prefer models with more effects (higher effectCount) as source.\n";
    prompt += "3. Match by model type and class (e.g., Tree->Tree, Arch->Arch, Matrix->Matrix).\n";
    prompt += "4. When multiple candidates match by type, prefer models with similar structure: similar node counts, strand counts, and dimensions (size=WxH). A 12-strand tree maps better to a 10-strand tree than a 4-strand tree.\n";
    prompt += "5. Submodel names (submodels=\"...\") reveal what a Custom model actually IS. Use them to identify the model's real purpose:\n";
    prompt += "   - Spinner/InnerRing/OuterRing/Bow → wreath. Match to other wreaths.\n";
    prompt += "   - Mouth/LeftEye/RightEye/Outline → singing face. Match to other singing faces.\n";
    prompt += "   - Snowflake-like submodels (Arms, Points, Center) → snowflake. Match to other snowflakes.\n";
    prompt += "   - Two Custom models with similar submodel compositions are likely the same kind of decoration and should map to each other.\n";
    prompt += "6. Match by similar names: 'Snowflake' matches 'flake', 'Canes' matches 'Candy Canes', 'CC' means Candy Cane, 'MH' means Moving Head, 'SF' means Snow Flake.\n";
    prompt += "7. If names contain numbers, try to match the numbers (e.g., 'Tree-3' -> 'Mini Tree 3').\n";
    prompt += "8. SubModels are named 'ModelName/SubModelName' - match submodels to similar submodels or models with similar names.\n";
    prompt += "9. ModelGroups (type=\"ModelGroup\") contain other models listed in their groupModels attribute. When you map a source ModelGroup to a target ModelGroup, do NOT also map that same source to the individual member models within the target group. The group mapping already covers them. Only map to individual models if there is no suitable group-level mapping.\n";
    prompt += "10. ModelGroups should map to other ModelGroups with similar member types and composition. Compare the groupModels lists to find the best match.\n";
    prompt += "11. A source model can be reused for multiple targets. Try to map as many targets as possible.\n";
    prompt += "12. Ignore brand names in model names: GE, EFL, Boscoyo, Chroma, Twinkly, IMPRESSION, Daycor, Living Light Shows, LLS, GRP, SS, Showstopper, PPD, PixelTrim, PixNode, xTreme.\n";
    prompt += "13. Focus on the decorative TYPE when parsing names (e.g., 'LeftArch' -> focus on 'Arch', 'HouseFlake' -> focus on 'Flake').\n";
    prompt += "14. Model aliases (aliases=\"...\") are alternative names for a model or submodel. They often contain the name from a previous show or a descriptive name. Treat aliases as equally valid names for matching — a source model with aliases that match a target name (or vice versa) is a strong match. Aliases prefixed with 'oldname:' indicate the model's previous name and are especially useful for matching across different shows.\n";
    prompt += "15. Not every target must be mapped, but map as many as reasonably possible.\n";

    prompt += "\nRespond with ONLY valid JSON in this exact format:\n";
    prompt += "{\"mappings\": [{\"targetModel\": \"target name\", \"sourceModel\": \"source name\"}, ...]}\n";
    prompt += "Only include targets that have a reasonable match. Do not explain your reasoning.";

    logger_base.debug("Claude mapping prompt size: %zu chars", prompt.size());

    nlohmann::json messages = nlohmann::json::array();
    nlohmann::json userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    messages.push_back(userMsg);

    auto [response, success] = CallAnthropicAPI(base_url, api_key, claudeModel, messages, 8192, "Claude", system);
    if (!success) {
        result.error = response;
        return result;
    }

    std::string text = ExtractTextContent(response, "Claude");
    if (text.empty()) {
        result.error = "No text in Claude response";
        return result;
    }

    logger_base.debug("Claude mapping response: %s", text.c_str());

    text = StripCodeFences(text);

    // Build a set of valid source names for validation
    std::set<std::string> validSources;
    for (const auto& m : sourceModels) {
        validSources.insert(m.name);
    }
    std::set<std::string> validTargets;
    for (const auto& m : targetModels) {
        validTargets.insert(m.name);
    }

    try {
        nlohmann::json root = nlohmann::json::parse(text);
        auto mappingsArr = root["mappings"];
        if (mappingsArr.is_null() || !mappingsArr.is_array()) {
            result.error = "No mappings array in response";
            return result;
        }

        for (const auto& mapping : mappingsArr) {
            std::string targetModel = mapping["targetModel"].get<std::string>();
            std::string sourceModel = mapping["sourceModel"].get<std::string>();

            // Validate both sides exist
            if (validSources.count(sourceModel) > 0 && validTargets.count(targetModel) > 0) {
                // Only map if not already mapped
                if (existingMappings.count(targetModel) == 0 && result.mappings.count(targetModel) == 0) {
                    result.mappings[targetModel] = sourceModel;
                }
            } else {
                logger_base.debug("Claude mapping skipped invalid: '%s' -> '%s'", targetModel.c_str(), sourceModel.c_str());
            }
        }
    } catch (const std::exception& e) {
        logger_base.error("Claude mapping parse error: %s", e.what());
        result.error = "Failed to parse mapping response: " + std::string(e.what());
    }

    return result;
}
