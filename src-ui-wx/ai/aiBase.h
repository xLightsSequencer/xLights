#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "aiType.h"

#include <functional>
#include <utility>
#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

class ServiceManager;
class wxPropertyGrid;
class wxVariant;
class wxSizer;
class wxDialog;
class wxBitmap;

class aiBase {  
protected:
    ServiceManager* _sm = nullptr;
    std::set<aiType::TYPE> _enabledTypes;
public:
    explicit aiBase(ServiceManager* sm);
    virtual ~aiBase() {}

    virtual void SaveSettings() const = 0;
    virtual void LoadSettings() = 0;

    [[nodiscard]] virtual std::string GetLLMName() const = 0;
    // Returns true if any capability is enabled
    [[nodiscard]] virtual bool IsEnabled() const { return !_enabledTypes.empty(); }
    // Returns true if the specific capability is enabled
    [[nodiscard]] bool IsEnabledForType(aiType::TYPE type) const { return _enabledTypes.count(type) > 0; }
    void SetEnabledForType(aiType::TYPE type, bool enabled) {
        if (enabled) _enabledTypes.insert(type);
        else _enabledTypes.erase(type);
    }
    [[nodiscard]] virtual bool IsAvailable() const = 0;

    // Returns all capabilities this service supports (regardless of enabled state)
    [[nodiscard]] virtual std::list<aiType::TYPE> GetTypes() const = 0;
    
    [[nodiscard]] virtual std::pair<std::string, bool> CallLLM(const std::string& prompt) const = 0;
    [[nodiscard]] virtual std::pair<std::string, bool> TestLLM() const;
    
    virtual void PopulateLLMSettings(wxPropertyGrid* page) = 0;  
    virtual void SetSetting(const std::string& key, const wxVariant& value) = 0;
    
    
    // For ColorPalette generation:
    class AIColor {
    public:
        std::string hexValue;
        std::string name;
        std::string description;        
    };
    class AIColorPalette {
    public:
        std::string description;
        std::vector<AIColor> colors;

        std::string error;
    };

    
    virtual AIColorPalette GenerateColorPalette(const std::string &prompt) const {
        return AIColorPalette();
    }

    class AIImageGenerator {
    public:
        virtual ~AIImageGenerator() {}
        
        virtual void generateImage(const std::string &prompt,
                                   const std::function<void(const wxBitmap &, const std::string &err)> &callback) = 0;
        virtual void addControls(wxDialog *parent, wxSizer *sizer) {}
    };
    
    virtual AIImageGenerator *createAIImageGenerator() const {
        return nullptr;
    }

    // For model mapping (import sequence → current layout):
    struct AIModelMappingResult {
        std::map<std::string, std::string> mappings; // targetModel → sourceModel
        std::string error;
    };

    struct MappingModelInfo {
        std::string name;
        std::string type;
        std::string modelClass;
        int effectCount = 0;
        std::string groupModels;
        bool isSubModel = false;
        bool isStrand = false;
        bool isNode = false;
        int nodeCount = 0;
        int strandCount = 0;
        int width = 0;
        int height = 0;
        std::vector<std::string> aliases;
        std::vector<std::string> subModelNames;
    };

    virtual AIModelMappingResult GenerateModelMapping(
        const std::vector<MappingModelInfo>& sourceModels,
        const std::vector<MappingModelInfo>& targetModels,
        const std::map<std::string, std::string>& existingMappings) const {
        return AIModelMappingResult();
    }

    struct AILyric 
    {
        std::string word;
        int startMS{ 0 };
        int endMS{ 0 };
    };

    struct AILyricTrack {
        std::vector<AILyric> lyrics;
        std::string error;
    };


    virtual AILyricTrack GenerateLyricTrack(const std::string& audioPath) const {
        return AILyricTrack();
    }
};
