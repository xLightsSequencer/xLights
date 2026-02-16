#include "gemini.h"
#include "AIImageDialog.h"
#include <nlohmann/json.hpp>
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>
#include <wx/mstream.h>
#include <wx/base64.h>

#include <vector>
#include <string>

#include <log4cpp/Category.hh>

//#define TEST_WITH_LOCAL_IMAGE

bool gemini::IsAvailable() const {
    return !api_key.empty() && _enabled;
}

void gemini::SaveSettings() const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _sm->setServiceSetting("GeminiModel", model);
    _sm->setServiceSetting("GeminiEnable", _enabled);
    _sm->setSecretServiceToken("GeminiApiKey", api_key);
    logger_base.info("Gemini settings saved successfully");
}

void gemini::LoadSettings() {
    model = _sm->getServiceSetting("GeminiModel", model);
    _enabled = _sm->getServiceSetting("GeminiEnable", _enabled);
    api_key = _sm->getSecretServiceToken("GeminiApiKey");
}

void gemini::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Gemini"));
    auto p = page->Append(new wxBoolProperty("Enabled", "Gemini.Enabled", _enabled));
    p->SetEditor("CheckBox");
    page->Append(new wxStringProperty("API Key", "Gemini.Key", api_key));
    page->Append(new wxStringProperty("Model", "Gemini.Model", model));
}

void gemini::SetSetting(const std::string& key, const wxVariant& value) {
    if (key == "Gemini.Enabled") {
        _enabled = value.GetBool();
    } else if (key == "Gemini.Key") {
        api_key = value.GetString();
    } else if (key == "Gemini.Model") {
        model = value.GetString();
    }
}

std::pair<std::string, bool> gemini::CallLLM(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string apiKey = api_key;

    if (apiKey.empty()) {
        apiKey = _sm->getSecretServiceToken("GeminiApiKey");
    }

    if (api_key.empty() && apiKey.empty()) {
        wxMessageBox("You must set a Gemini API Key in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { "Gemini: API Key is empty", false };
    }

    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] }";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + apiKey }
    };

    logger_base.debug("Gemini: %s", request.c_str());

    int responseCode = 0;	
    std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("Gemini Response %d: %s", responseCode, response.c_str());

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        logger_base.error("Gemini: Invalid JSON response: %s", response.c_str());
        return { "Gemini: Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        logger_base.error("Gemini: No choices in response");
        return { "Gemini: No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        logger_base.error("Gemini: No text in response");
        return { "Gemini: No text in response", false };
    }

    response = text.get<std::string>();
    logger_base.debug("Gemini: %s", response.c_str());

    return { response, true };
}

class GeminiImageGenerator : public aiBase::AIImageGenerator {
private:
    std::string api_key;
    std::string model;
    std::string style = "cartoon";

public:
    GeminiImageGenerator(const std::string& apiKey, const std::string& modelName)
        : api_key(apiKey), model(modelName) {}

    virtual ~GeminiImageGenerator() {}

    virtual void generateImage(const std::string &prompt,
        const std::function<void(const wxBitmap &, const std::string &)> &cb) override {

        if (prompt.empty()) {
            cb(wxBitmap(), "Prompt cannot be empty");
            return;
        }

        if (api_key.empty()) {
            cb(wxBitmap(), "Gemini API key not configured");
            return;
        }

        std::string styleDesc;
        if (style == "cartoon") {
            styleDesc = "in vibrant cartoon style, clean lines, cel-shaded, fun and exaggerated features";
        } else if (style == "sticker") {
            styleDesc = "as a cute sticker, bold flat colors, thick black outline, simple background";
        } else if (style == "outline") {
            styleDesc = "clean thick vector outline art, minimal details, high contrast lines only";
        } else if (style == "line art / neon glow") {
            styleDesc = "neon glow line art, cyberpunk aesthetic, glowing colored lines on dark background";
        } else {
            styleDesc = "in detailed illustrative style";
        }

        std::string fullPrompt = 
            "Background: Pure Black background (#000000) with no watermarks or border. Design: 2D Vector Style. "
            "No black outlines; use soft, colored outlines that match the object's palette. "
            "Style: A minimalist, flat 2D pixel art illustration using a 8-bit look and minimalist facial features. "
            "Non-anthropomorphic (no faces or eyes). "
            "Details: Simple cel-shading, limited vibrant color palette, clean edges, and no gradients. "
            "Ensure the lines are perfectly horizontal and vertical. Center the subject. " + styleDesc + 
            " Subject: " + prompt;

        Replace(fullPrompt, std::string("\t"), std::string(" "));
        Replace(fullPrompt, std::string("\r"), std::string(""));

        std::string model = "gemini-2.5-flash-image";  // Correct model name
        std::string endpoint = "https://generativelanguage.googleapis.com/v1beta/models/" + model + ":generateContent";

        nlohmann::json req;
        req["contents"][0]["parts"][0]["text"] = fullPrompt;
        req["generationConfig"]["responseModalities"] = nlohmann::json::array({"IMAGE"});

        std::string jsonBody = req.dump();

        std::vector<std::pair<std::string, std::string>> headers = {
            {"x-goog-api-key", api_key},
            {"Content-Type", "application/json"}
        };

        static log4cpp::Category& logger = log4cpp::Category::getInstance("log_base");
        logger.debug("Gemini image request: %s", jsonBody.c_str());

#ifdef TEST_WITH_LOCAL_IMAGE
        // Load test image from file
        wxImage wxImg("F:\\ShowFolderQA\\Gemini_Generated_Image.png", wxBITMAP_TYPE_PNG);
        if (!wxImg.IsOk()) {
            logger.error("Failed to load test image");
            cb(wxBitmap(), "Failed to load test image.");
            return;
        }

        logger.debug("Test image loaded with image size: %dx%d", wxImg.GetWidth(), wxImg.GetHeight());
        cb(wxImg, jsonBody.c_str());
#else
        int httpCode = 0;
        std::string response = Curl::HTTPSPost(endpoint, jsonBody, "", "", "JSON", 120, headers, &httpCode);

        logger.debug("Gemini image response code: %d, body length: %zu", httpCode, response.size());

        if (httpCode != 200) {
            cb(wxBitmap(), "Gemini API error " + std::to_string(httpCode) + ": " + response.substr(0, 300));
            return;
        }

        nlohmann::json root;
        try {
            root = nlohmann::json::parse(response);
        } catch (const std::exception& e) {
            logger.error("Gemini image JSON parse failed: %s", e.what());
            cb(wxBitmap(), "Invalid response from Gemini");
            return;
        }

        std::string base64Data;
        try {
            // Iterate through parts to find the image data
            if (root.contains("candidates") && root["candidates"].is_array() && !root["candidates"].empty()) {
                auto parts = root["candidates"][0]["content"]["parts"];
                for (const auto& part : parts) {
                    if (part.contains("inlineData") && part["inlineData"].contains("data")) {
                        base64Data = part["inlineData"]["data"].get<std::string>();
                        break;
                    }
                }
            }
        } catch (...) {
            std::string errMsg = root.contains("error") ? root["error"]["message"].get<std::string>() : "No image data found";
            logger.error("Gemini image: %s", errMsg.c_str());
            cb(wxBitmap(), "Gemini: " + errMsg);
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

        logger.debug("Generated image size: %dx%d", wxImg.GetWidth(), wxImg.GetHeight());
        wxBitmap bmp(wxImg);
        cb(bmp, "");
#endif
    }
    virtual void addControls(wxDialog *parent, wxSizer *sizer) override {
        wxStaticText *StaticText2 = new wxStaticText(parent, wxNewId(), _T("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
        sizer->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxChoice *Choice1 = new wxChoice(parent, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
        Choice1->Append(_T("Cartoon"));
        Choice1->Append(_T("Sticker"));
        Choice1->Append(_T("Outline"));
        Choice1->Append(_T("Line art / Neon glow"));
        Choice1->SetSelection(0);
        sizer->Add(Choice1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

        Choice1->Bind(wxEVT_COMMAND_CHOICE_SELECTED, [this](wxCommandEvent& event) {
            style = Lower(event.GetString().ToStdString());
            });
    }

};

aiBase::AIImageGenerator *gemini::createAIImageGenerator() const {
    return new GeminiImageGenerator(api_key, model);
}
