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
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>


class Effect;
class RenderCache;
class SequenceElements;
class RenderBuffer;
class SettingsMap;

// Render-cache mode.  Replaces the old free-form std::string _enabled so the
// hot-path eligibility check compares an enum instead of doing string compares.
enum class RenderCacheMode { Disabled, LockedOnly, Enabled };

class RenderCacheItem
{
    RenderCache* _renderCache = nullptr;
    std::string _cacheFile;
    std::string _effectName;
    std::map<std::string, std::string> _properties;
    std::map<std::string, std::vector<uint8_t *>> _frames;
    std::map<std::string, long> _frameSize;
    bool _purged = false;
    bool _dirty = false;
    bool _sawEndFrame = false;
    static std::string GetModelName(RenderBuffer* buffer);

    
    uint8_t *_mmap = nullptr;
    size_t _mmapSize = 0;
    size_t _firstFrameOffset = 0;

    void unmmap();
    void remmap();

public:
    RenderCacheItem(RenderCache* renderCache, const std::string& file);
    RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer);
    virtual ~RenderCacheItem();
    bool GetFrame(RenderBuffer* buffer);
    void AddFrame(RenderBuffer* buffer);
    void PurgeFrames();
    bool IsPurged() const { return _purged; }
    bool IsMatch(Effect* effect, RenderBuffer* buffer);
    void Delete();
    void Save();
    void Touch() const;
    bool IsDone(RenderBuffer* buffer) const;
    const std::string& Description() const { return _cacheFile; }
    const std::string& EffectName() const { return _effectName; }
};

class RenderCache
{
    class PerEffectCache {
    public:
        PerEffectCache() {}
        ~PerEffectCache() {}
        std::list<RenderCacheItem*> cache;
        std::shared_mutex lock;
    };
    
    std::recursive_mutex  _cacheLock;
	std::string _cacheFolder;
	std::map<std::string, PerEffectCache*> _cache;
    RenderCacheMode _mode = RenderCacheMode::Enabled;
    std::mutex _loadMutex;
    std::thread _loadThread;
    size_t _maximumSizeMB = 0;
    std::string _baseCache = "";

    void Close();
    void LoadCache();
    
    PerEffectCache* GetPerEffectCache(const std::string &s);
    void EnforceMaximumSize();

    public:
		RenderCache();
		virtual ~RenderCache();
        inline bool IsEnabled() const { return _mode != RenderCacheMode::Disabled; }
        void SetRenderCacheFolder(const std::string& path);
        void SetSequence(const std::string& path, const std::string& sequenceFile);
		RenderCacheItem* GetItem(Effect* effect, const SettingsMap& settings, RenderBuffer* buffer);
        void RemoveItem(RenderCacheItem *item);
        std::string GetCacheFolder() const { return _cacheFolder; }
        void CleanupCache(SequenceElements* sequenceElements);
        void Purge(SequenceElements* sequenceElements, bool dodelete);
        // Accepts the preference string ("Disabled" | "Locked Only" |
        // "Enabled"; "Locked Effects Only" is the legacy spelling).  Anything
        // unrecognised maps to Enabled, matching the prior default.
        void Enable(const std::string& enabled) {
            if (enabled == "Disabled") {
                _mode = RenderCacheMode::Disabled;
            } else if (enabled == "Locked Only" || enabled == "Locked Effects Only") {
                _mode = RenderCacheMode::LockedOnly;
            } else {
                _mode = RenderCacheMode::Enabled;
            }
        }
        std::mutex& GetLoadMutex() { return _loadMutex; }
        void AddCacheItem(RenderCacheItem* rci);
        // Thread-safe: reads the per-render SettingsMap copy (prefix-stripped
        // keys), not the live Effect, so it can run without the effect's
        // settingsLock.
        bool IsEffectOkForCaching(const SettingsMap& settings) const;
        bool UseMMap() const;
        void SetMaximumSizeMB(size_t mb);
};
