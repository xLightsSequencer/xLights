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
#include "ServiceProperty.h"

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ServiceManager;

class aiBase {
protected:
    ServiceManager* _sm = nullptr;
    std::set<aiType::TYPE> _enabledTypes;
public:
    // Inline so plugin DLLs need no import linkage back to xLights.exe
    explicit aiBase(ServiceManager* sm) : _sm(sm) {}
    virtual ~aiBase() = default;

    aiBase(const aiBase&) = delete;
    aiBase& operator=(const aiBase&) = delete;
    aiBase(aiBase&&) = delete;
    aiBase& operator=(aiBase&&) = delete;

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
    [[nodiscard]] virtual std::pair<std::string, bool> TestLLM() const { return CallLLM("Hello"); }

    // Declarative list of editable settings for this service. The UI
    // (wxPropertyGrid on desktop, SwiftUI Form on iPad) builds its controls
    // from this vector. See ServiceProperty.h.
    [[nodiscard]] virtual std::vector<ServiceProperty> GetProperties() const = 0;

    // Property setters. Overloads cover the three value types that properties
    // can carry — the UI calls the overload matching the property's Kind.
    virtual void SetProperty(const std::string& id, bool value) {}
    virtual void SetProperty(const std::string& id, int value) {}
    virtual void SetProperty(const std::string& id, const std::string& value) {}


    // For ColorPalette generation:
    struct AIColor {
        std::string hexValue;
        std::string name;
        std::string description;
    };
    struct AIColorPalette {
        std::string description;
        std::vector<AIColor> colors;

        std::string error;
    };


    [[nodiscard]] virtual AIColorPalette GenerateColorPalette(const std::string& prompt) const {
        return {};
    }

    // Result of an image generation call. On success pngBytes holds encoded
    // PNG data (host decodes into wxBitmap / UIImage / NSImage as needed).
    // On failure pngBytes is empty and error is non-empty.
    struct AIImageResult {
        std::vector<uint8_t> pngBytes;
        std::string error;
    };

    class AIImageGenerator {
    public:
        AIImageGenerator() = default;
        virtual ~AIImageGenerator() = default;

        AIImageGenerator(const AIImageGenerator&) = delete;
        AIImageGenerator& operator=(const AIImageGenerator&) = delete;
        AIImageGenerator(AIImageGenerator&&) = delete;
        AIImageGenerator& operator=(AIImageGenerator&&) = delete;

        // Generate an image from a prompt. The callback is invoked once with
        // the result; on success `pngBytes` is non-empty, on failure `error`
        // is non-empty. Hosts must marshal back to the UI thread themselves
        // if the callback fires off-thread.
        virtual void generateImage(const std::string& prompt,
                                   std::function<void(AIImageResult)> callback) = 0;

        // Per-generator editable properties (e.g. an image-style picker).
        // Same pattern as aiBase::GetProperties — rendered by the host UI.
        [[nodiscard]] virtual std::vector<ServiceProperty> GetProperties() const { return {}; }
        virtual void SetProperty(const std::string& id, bool value) {}
        virtual void SetProperty(const std::string& id, int value) {}
        virtual void SetProperty(const std::string& id, const std::string& value) {}
    };

    [[nodiscard]] virtual AIImageGenerator* createAIImageGenerator() const {
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

    [[nodiscard]] virtual AIModelMappingResult GenerateModelMapping(
        const std::vector<MappingModelInfo>& sourceModels,
        const std::vector<MappingModelInfo>& targetModels,
        const std::map<std::string, std::string>& existingMappings) const {
        return {};
    }

    struct AILyric {
        std::string word;
        int startMS{ 0 };
        int endMS{ 0 };
    };

    struct AILyricTrack {
        std::vector<AILyric> lyrics;
        std::string error;
    };


    [[nodiscard]] virtual AILyricTrack GenerateLyricTrack(const std::string& audioPath) const {
        return {};
    }
};
