#ifndef RENDERCACHE_H
#define RENDERCACHE_H

#include <string>
#include <list>
#include <map>
#include <vector>
#include <mutex>

class Effect;
class RenderCache;
class SequenceElements;
class RenderBuffer;
class RenderCacheLoadThread;

class RenderCacheItem
{
    RenderCache* _renderCache;
    std::string _cacheFile;
    std::map<std::string, std::string> _properties;
    std::map<std::string, std::vector<unsigned char *>> _frames;
    std::map<std::string, long> _frameSize;
    bool _purged;
    bool _dirty;
    std::string GetModelName(RenderBuffer* buffer) const;

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
    bool IsDone(RenderBuffer* buffer) const;
};

class RenderCache
{
    std::recursive_mutex  _cacheLock;
	std::string _cacheFolder;
	std::list<RenderCacheItem*> _cache;
    std::string _enabled; // Disabled | Locked Only | Enabled
    std::mutex _loadMutex;

    void Close();
    void LoadCache();

    public:
		RenderCache();
		virtual ~RenderCache();
        inline bool IsEnabled() const { return _enabled != "Disabled"; }
        void SetSequence(const std::string& path, const std::string& sequenceFile);
		RenderCacheItem* GetItem(Effect* effect, RenderBuffer* buffer);
        void RemoveItem(RenderCacheItem *item);
        std::string GetCacheFolder() const { return _cacheFolder; }
        void CleanupCache(SequenceElements* sequenceElements);
        void Purge(SequenceElements* sequenceElements, bool dodelete);
        void ForgetCache(SequenceElements* sequenceElements);
        void Enable(std::string enabled) { _enabled = enabled; }
        std::mutex& GetLoadMutex() { return _loadMutex; }
        void AddCacheItem(RenderCacheItem* rci);
        bool IsEffectOkForCaching(Effect* effect);
};

#endif // RENDERCACHE_H
