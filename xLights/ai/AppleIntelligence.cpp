#include "AppleIntelligence.h"
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"
#include <nlohmann/json.hpp>

#include <wx/propgrid/propgrid.h>

#include <vector>
#include <string>

#include <log4cpp/Category.hh>

#include "xLights-Swift.h"


bool AppleIntelligence::IsAvailable() const {
    return _enabled;
}

void AppleIntelligence::SaveSettings() const {
    _sm->setServiceSetting("appleAIEnable", _enabled);
}

void AppleIntelligence::LoadSettings() {
    _enabled = _sm->getServiceSetting("appleAIEnable", _enabled);
}

void AppleIntelligence::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Apple Intelligence"));
    auto p = page->Append(new wxBoolProperty("Enabled", "AppleIntelligence.Enabled", _enabled));
    p->SetEditor("CheckBox");
}

void AppleIntelligence::SetSetting(const std::string& key, const wxVariant& value) {
	if (key == "AppleIntelligence.Enabled") {
		_enabled = value.GetBool();
	}
}

std::pair<std::string, bool> AppleIntelligence::CallLLM(const std::string& prompt) const {
    std::string s = xLights::RunAppleIntelligencePrompt(prompt);

    return {s, !s.empty()};
}

aiBase::AIColorPalette AppleIntelligence::GenerateColorPalette(const std::string &prompt) const {
    aiBase::AIColorPalette ret;
    
    std::string res = xLights::RunAppleIntelligenceGeneratePalette(prompt);
    if (!res.empty()) {
        
        try {
            // Check if the response is valid JSON
            nlohmann::json const root = nlohmann::json::parse(res);
            if (root.contains("error")) {
                ret.error = root["error"].get<std::string>();
            } else {
                ret.description = root["Description"].get<std::string>();
                for (int x = 0; x < root["Colors"].size(); x++) {
                    ret.colors.push_back(aiBase::AIColor());
                    ret.colors.back().description = root["Colors"][x]["Description"].get<std::string>();
                    ret.colors.back().name = root["Colors"][x]["Name"].get<std::string>();
                    ret.colors.back().hexValue = root["Colors"][x]["Hex Value"].get<std::string>();
                    if (!ret.colors.back().hexValue.empty() &&  ret.colors.back().hexValue[0] != '#') {
                        ret.colors.back().hexValue = "#" + ret.colors.back().hexValue;
                    }
                }
            }
        } catch (const std::exception& ex) {
            
        }
    }
    return ret;
}
