#include "ollama.h"
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>

#include <vector>
#include <string>

#include <log4cpp/Category.hh>


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

	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	if (host.empty()) {
		logger_base.error("ollama: host is empty");
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

    logger_base.debug("ollama: %s", request.c_str());
    int responseCode { 0 };	
	std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60 * 10, {}, &responseCode);

    logger_base.debug("ollama Response %d: %s", responseCode, response.c_str());

	if (responseCode != 200) {
        return { response , false};
    }

	wxJSONReader reader;
	wxJSONValue root;
    if (reader.Parse(response, &root) > 0)
	{
		logger_base.error("ollama: Failed to parse response");
        return { "ollama: Failed to parse response", false };
	}
    wxJSONValue text = root["response"];
	if (text.IsNull()) {
		logger_base.error("ollama: No text in response");
        return { "ollama: No text in response", false };
		}

	response = text.AsString();
	logger_base.debug("ollama: %s", response.c_str());

    return { response, true};
}

