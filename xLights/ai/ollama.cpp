#include "ollama.h"
#include "ServiceManager.h"
#include "utils/Curl.h"


#include <wx/propgrid/propgrid.h>
#include <wx/colour.h>

#include <vector>
#include <string>

#include "./utils/spdlog_macros.h"

#include <nlohmann/json.hpp>

#include "UtilFunctions.h"

bool ollama::IsAvailable() const {
    return !host.empty() && _enabled;
}

void ollama::SaveSettings() const {
    _sm->setServiceSetting("ollamaEnable", _enabled);
    _sm->setServiceSetting("ollamaHost", host);
    _sm->setServiceSetting("ollamaPort", port_num);
    _sm->setServiceSetting("ollamaModel", model);
}

void ollama::LoadSettings() {
    _enabled = _sm->getServiceSetting("ollamaEnable", _enabled);
    host = _sm->getServiceSetting("ollamaHost", host);
    model = _sm->getServiceSetting("ollamaModel", model);
    port_num = _sm->getServiceSetting("ollamaPort", port_num);
}

void ollama::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("ollama"));
    auto p = page->Append(new wxBoolProperty("Enabled", "ollama.Enabled", _enabled));
    p->SetEditor("CheckBox");
    page->Append(new wxStringProperty("Host", "ollama.Host", host));
    page->Append(new wxIntProperty("Port", "ollama.Port", port_num));
    auto pp = page->Append(new wxBoolProperty("Https", "ollama.Https", https));
    pp->SetEditor("CheckBox");
    page->Append(new wxStringProperty("Model", "ollama.Model", model));
}

void ollama::SetSetting(const std::string& key, const wxVariant& value) {
	if (key == "ollama.Enabled") {
		_enabled = value.GetBool();
	} else if (key == "ollama.Host") {
		host = value.GetString();
	} else if (key == "ollama.Port") {
		port_num = value.GetLong();
	} else if (key == "ollama.Https") {
		https = value.GetBool();
	} else if (key == "ollama.Model") {
		model = value.GetString();
    }
}

std::pair<std::string, bool> ollama::CallLLM(const std::string& prompt) const {

	if (host.empty()) {
        spdlog::error("ollama: host is empty");
		return {"ollama: host is empty", false};
    }
    std::string const url = (https ? "https://" : "http://") + host + ":" + std::to_string(port_num) + api;

	// remove all \t, \r and \n as ollama does not like it
    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));
    Replace(p, std::string("\n"), std::string("\\n"));
    /*
	{
      "model": "llama3.2",
      "prompt":"Why is the sky blue?"
    }
	*/
    std::string const request = "{ \"model\": \"" + model + "\", \"prompt\": \"" + JSONSafe(p) + "\",\"stream\": false }";

    spdlog::debug("ollama: {}", request);
    int responseCode { 0 };	
	std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60 * 10, {}, &responseCode);

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
    } catch (const std::exception&) {
        spdlog::error("ollama: Invalid JSON response: {}", response);
        return { "ollama: Invalid JSON response", false };
    }
    spdlog::debug("ollama: {}", response);

    return { response, true};
}

std::vector<wxColour> ollama::CallLLMForColors(const std::string& prompt) const {
    
    if (host.empty()) {
        spdlog::error("ollama: host is empty");
        return std::vector<wxColour>();
    }
    std::string const url = (https ? "https://" : "http://") + host + ":" + std::to_string(port_num) + api;

    /*
    "model": "llama3.1",
  "messages": [{"role": "user", "content": "Tell me about Canada."}],
  "stream": false,
  "format": {
    "type": "object",
    "properties": {
      "name": {
        "type": "string"
      },
      "capital": {
        "type": "string"
      },
      "languages": {
        "type": "array",
        "items": {
          "type": "string"
        }
      }
    },
    "required": [
      "name",
      "capital", 
      "languages"
    ]
  }
    */

    /*
    "type": "array",
        "items": {
          "type": "string"
        }
    */
    nlohmann::json input;
    input["model"] = model;
    input["stream"] = false;
    input["format"]["type"] = "array";
    input["format"]["items"]["type"] = "string";
    input["format"]["items"]["description"] = "A color in hex format, e.g. #FF0000 or rgb(255,0,0) or rgba(255,0,0,1)";
    input["format"]["items"]["example"] = "#FF0000, #00FF00, #0000FF";
    
    nlohmann::json message;
    message["role"] = "user";
    message["content"] = "Generate a list of colors in hex format based on the following song: " + prompt;
    std::vector<nlohmann::ordered_json> messages;
    messages.push_back(message);
    input["messages"] = messages;


    std::string str = input.dump(3);
    //std::string const request = "{ \"model\": \"" + model + "\", \"prompt\": \"" + JSONSafe(p) + "\",\"stream\": false }";
    spdlog::debug("ollama: {}", str);
    int responseCode{ 0 };
    std::string response = Curl::HTTPSPost(url, str, "", "", "JSON", 60 * 10, {}, &responseCode);
    spdlog::debug("ollama Response {}: {}", responseCode, response);
    if (responseCode != 200) {
        return {};
    }
    try {
        // Check if the response is valid JSON
        nlohmann::json const resp_json = nlohmann::json::parse(response);
        if (resp_json.contains("response")) {
            response = resp_json.at("response").get<std::string>();
        }
    } catch (const std::exception&) {
    }

    spdlog::debug("ollama: {}", response);
    std::vector<wxColour> colors;
    auto colorStrings = Split(response, { ',', ';' });
    for (const auto& colorString : colorStrings) {
        wxColour const color(colorString);
        if (color.IsOk()) {
            colors.push_back(color);
        } else {
            spdlog::error("ollama: Invalid color '{}'", colorString);
        }
    }
    return colors;
}

