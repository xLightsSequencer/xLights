#include "ollama.h"
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonwriter.h"

#include <vector>
#include <string>

#include <log4cpp/Category.hh>
#include <Color.h>

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

aiBase::AIColorPalette ollama::GenerateColorPalette(const std::string& prompt) const {
    
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    if (host.empty()) {
        logger_base.error("ollama: host is empty");
        return {};
    }
    std::string const url = (https ? "https://" : "http://") + host + ":" + std::to_string(port_num) + api;

    auto fullprompt = "xlights color paletes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors. Return As Hex Values.";
    
    wxJSONValue request_payload;
    request_payload["model"] = model;
    request_payload["temperature"] = 0;
    request_payload["prompt"] = fullprompt;
    request_payload["stream"] = false;

    // Include the structured output format (JSON schema)
    wxJSONValue format_schema;
    format_schema["type"] = wxString("object");
    format_schema["properties"]["colors"]["type"] = wxString("array");
    format_schema["properties"]["colors"]["items"]["type"] = wxString("string");
    format_schema["properties"]["colors"]["description"] = wxString("A list of colors mentioned in the text.");
    //format_schema["required"] = wxArrayString({ "colors" });
    wxJSONValue requiredArray(wxJSONTYPE_ARRAY);
    requiredArray.Append(wxString("colors"));
    format_schema["required"] = requiredArray;

    request_payload["format"] = format_schema;

     wxJSONWriter writer;

    // Create a wxString to store the JSON text
     wxString json_payload_str;

    // Write the wxJSONValue to the wxString
    writer.Write(request_payload, json_payload_str);

    logger_base.debug("ollama: %s", json_payload_str.c_str());
    int responseCode{ 0 };
    std::string const response = Curl::HTTPSPost(url, json_payload_str, "", "", "JSON", 60 * 10, {}, &responseCode);

    logger_base.debug("ollama Response %d: %s", responseCode, response.c_str());

    if (responseCode != 200) {
        return {};
    }

    try {
        wxJSONValue root;
        wxJSONReader reader;
        reader.Parse(response, &root);

        if (root.HasMember("response") && root["response"].IsString()) {
            wxJSONValue color_root;
            auto const color_responce = root["response"].AsString();
            bool const worked = reader.Parse(color_responce, &color_root);
            logger_base.debug("ollama Response %s", color_responce.c_str());
            if (color_root.HasMember("colors") && color_root["colors"].IsArray()) {
                aiBase::AIColorPalette ret;
                ret.description = prompt;
                
                for (int x = 0; x < color_root["colors"].Size(); x++) {
                    auto const& color = color_root["colors"][x];
                    if (color.IsArray()) {
                        for (int y = 0; y < color.Size(); y++) {
                            auto const& col = color.ItemAt(y);
                            if (col.IsString()) {
                                auto colorl = col.AsString();
                                ret.colors.push_back(aiBase::AIColor());
                                ret.colors.back().hexValue = colorl;
                                wxColor xc(colorl);
                                ret.colors.back().name = GetColourName(xc);
                            }
                        }
                    }
                    if (color.IsString()) {
                        auto cc = color.AsString();
                        ret.colors.push_back(aiBase::AIColor());
                        ret.colors.back().hexValue = cc;
                        wxColor xc(cc);
                        ret.colors.back().name = GetColourName(xc);
                    }
                }
                return ret;
            }
            logger_base.error("Response does not contain 'colors' array or is not in expected format.");
        } else {
            logger_base.error("Invalid response from Ollama API.");
        }
    } catch (const std::exception& e) {
        logger_base.error(e.what());
    }
    return {};
}

