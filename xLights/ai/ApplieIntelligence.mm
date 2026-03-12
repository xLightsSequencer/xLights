
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#include "ServiceManager.h"
#include <wx/propgrid/propgrid.h>
#include <wx/choice.h>

#include "xLights-Swift.h"
#include "AppleIntelligence.h"


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




class AppleIntelligenceImageGenerator : public aiBase::AIImageGenerator {
public:
    virtual ~AppleIntelligenceImageGenerator() {}
    
    virtual void generateImage(const std::string &prompt,
                                const std::function<void(const wxBitmap &, const std::string &)> &cb) override {
        callback = cb;
                
        std::string full = prompt + R"(
        MANDATORY OUTPUT REQUIREMENTS: Background: Black background (#000000) with no watermarks or border.
        The design features bold, clean outlines, simple cell-shading, and a limited vibrant color palette with clean edges and no gradients.
        )";


        NSString *p = @(prompt.c_str());
        ImagesAsyncCaller *caller = [[ImagesAsyncCaller alloc] init];
        
        [caller generateImagesWithPrompt:p fullInstructions:@(full.c_str()) style:@(style.c_str()) completionHandler:^(CGImage *result, NSString *errString) {
            std::string err = std::string([errString UTF8String]);
            if (!err.empty()) {
                callback(wxBitmap(), err);
            } else {                
                callback(wxBitmap(result), "");
            }
        }];
    }
    virtual void addControls(wxDialog *parent, wxSizer *sizer) override {
        wxStaticText *StaticText2 = new wxStaticText(parent, wxNewId(), _T("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
        sizer->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxChoice *Choice1 = new wxChoice(parent, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));

        Choice1->Append(_T("Animation"));
        Choice1->Append(_T("Illustration"));
        Choice1->Append(_T("Sketch"));
        Choice1->Append(_T("Emoji"));
        sizer->Add(Choice1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
        
        Choice1->Bind(wxEVT_COMMAND_CHOICE_SELECTED, [this](wxCommandEvent& event) {
            style = Lower(event.GetString().ToStdString());
        });

    }
    
    std::function<void(const wxBitmap &, const std::string &err)> callback;
    std::string style = "animation";
};

aiBase::AIImageGenerator *AppleIntelligence::createAIImageGenerator() const {
    return new AppleIntelligenceImageGenerator();
}
