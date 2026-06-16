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

#include <string>
#include <map>
#include <vector>
#include <memory>

class EffectSymbol;
class Effect;
namespace pugi { class xml_node; }

class IEffectSymbolListener
{
public:
    virtual ~IEffectSymbolListener() = default;
    virtual void OnSymbolChanged(const std::string& symbolId) = 0;
    virtual void OnSymbolDeleted(const std::string& symbolId) = 0;
    virtual void OnSymbolCreated(const std::string& symbolId) = 0;
    virtual void OnSymbolRenamed(const std::string& symbolId, const std::string& oldName, const std::string& newName) = 0;
};

class EffectSymbolManager
{
public:
    EffectSymbolManager();
    ~EffectSymbolManager();

    void Clear();

    EffectSymbol* CreateSymbol(const std::string& name, const Effect* sourceEffect);
    EffectSymbol* CreateEmptySymbol(const std::string& name, const std::string& effectType, int effectIndex);
    EffectSymbol* GetSymbol(const std::string& id) const;
    EffectSymbol* GetSymbolByName(const std::string& name) const;
    std::vector<EffectSymbol*> GetAllSymbols() const;
    size_t GetSymbolCount() const { return _symbols.size(); }
    bool DeleteSymbol(const std::string& id);
    bool RenameSymbol(const std::string& id, const std::string& newName);
    bool SymbolExists(const std::string& id) const;
    bool SymbolNameExists(const std::string& name) const;
    std::string GetUniqueSymbolName(const std::string& baseName) const;

    void RegisterLinkedEffect(Effect* effect, const std::string& symbolId);
    void UnregisterLinkedEffect(Effect* effect);
    std::vector<Effect*> GetLinkedEffects(const std::string& symbolId) const;
    size_t GetLinkedEffectCount(const std::string& symbolId) const;

    void UpdateSymbolSettings(const std::string& id, const std::string& settings);
    void UpdateSymbolPalette(const std::string& id, const std::string& palette);
    void NotifySymbolChanged(const std::string& symbolId);

    // Pugi-based XML round-trip. SaveToXml appends an <EffectSymbols> child to the
    // supplied parent (typically the xsequence root). LoadFromXml expects the
    // <EffectSymbols> node itself.
    void LoadFromXml(const pugi::xml_node& symbolsNode);
    void SaveToXml(pugi::xml_node& parent) const;

    void AddListener(IEffectSymbolListener* listener);
    void RemoveListener(IEffectSymbolListener* listener);

private:
    std::string GenerateUniqueId();

    std::map<std::string, std::unique_ptr<EffectSymbol>> _symbols;
    std::multimap<std::string, Effect*> _linkedEffects;
    std::vector<IEffectSymbolListener*> _listeners;
    int _nextSymbolId;
};
