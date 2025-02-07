#include "chatGPT.h"
#include "xLightsMain.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <vector>
#include <string>

#include <log4cpp/Category.hh>

// https://platform.openai.com/docs/api-reference/introduction

#define CHATGPT_API_URL "https://api.openai.com/v1/chat/completions"

// we may want to make these user controlled in the future or even feature controlled
#define CHATGPT_MODEL "gpt-4o-mini"
#define TEMPERATURE "0.0"

// to get a list of models curl https://api.openai.com/v1/models -H "Authorization: Bearer YOUR_API_KEY"

bool IsChatGPTAvailable(xLightsFrame* frame, const std::string& token) {
    return token != "" || frame->GetServiceSetting("ChatGPTBearerToken") != "";
}

std::string CallChatGPT(xLightsFrame* frame, const std::string& prompt, const std::string& token) {

	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::string bearerToken = token;
    
	if (bearerToken == "") bearerToken = frame->GetServiceSetting("ChatGPTBearerToken");

	if (token == "" && bearerToken.empty()) {
		wxMessageBox("You must set a ChatGPT Bearer Token in the Preferences on the Services Panel", "Error", wxICON_ERROR);
		return "";
	}

	// remove all \t, \r and \n as chatGPT does not like it
    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));
    Replace(p, std::string("\n"), std::string("\\n"));

	std::string request = "{ \"model\": \"" + std::string(CHATGPT_MODEL) + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] , \"temperature\": " + std::string(TEMPERATURE) + " }";

	std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + bearerToken }
    };

    logger_base.debug("ChatGPT: %s", request.c_str());

	int responseCode = 0;	
	std::string response = Curl::HTTPSPost(CHATGPT_API_URL, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("ChatGPT Response %d: %s", responseCode, response.c_str());

	if (responseCode != 200) {
        return "";
    }

	wxJSONReader reader;
	wxJSONValue root;
    if (reader.Parse(response, &root) > 0)
	{
		logger_base.error("ChatGPT: Failed to parse response");
		return "";
	}

	wxJSONValue choices = root["choices"];
	if (choices.IsNull() || choices.Size() == 0) {
		logger_base.error("ChatGPT: No choices in response");
		return "";
		}

	wxJSONValue choice = choices[0];
	wxJSONValue text = choice["message"]["content"];
	if (text.IsNull()) {
		logger_base.error("ChatGPT: No text in response");
		return "";
		}

	response = text.AsString();
	logger_base.debug("ChatGPT: %s", response.c_str());

    return response;
}

bool TestChatGPT(xLightsFrame* frame, const std::string& token) {
	std::string response = CallChatGPT(frame, "Hello", token);
	if (response.empty()) {
		return false;
	}
	return true;
}