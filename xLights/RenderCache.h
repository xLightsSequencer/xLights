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

class RenderCacheItem
{
        RenderCache* _renderCache;
		std::string _cacheFile;
		std::map<std::string, std::string> _properties;
        std::vector<unsigned char *> _frames;
        bool _purged;
        long _frameSize;
        bool _dirty;


	public:
		RenderCacheItem(RenderCache* renderCache, const std::string& file);
		RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer);
		virtual ~RenderCacheItem();
		bool GetFrame(RenderBuffer* buffer);
		void AddFrame(RenderBuffer* buffer);
        void PurgeFrames();

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
    bool _enabled;
	
    void Close();

    public:
		RenderCache();
		virtual ~RenderCache();
		void SetSequence(const std::string& path, const std::string& sequenceFile);
		RenderCacheItem* GetItem(Effect* effect, RenderBuffer* buffer);
        void RemoveItem(RenderCacheItem *item);
        std::string GetCacheFolder() const { return _cacheFolder; }
        void CleanupCache(SequenceElements* sequenceElements);
        void Purge(SequenceElements* sequenceElements, bool dodelete);
        void Enable(bool enabled) { _enabled = enabled; }
};

#endif // RENDERCACHE_H
