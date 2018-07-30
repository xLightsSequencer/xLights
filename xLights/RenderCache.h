#ifndef RENDERCACHE_H
#define RENDERCACHE_H

#include <string>
#include <list>
#include <map>

class Effect;
class RenderCache;
class SequenceElements;
class RenderBuffer;

class RenderCacheItem
{
		std::string _cacheFile;
		std::map<std::string, std::string> _properties;
        std::map<int, unsigned char *> _frames;
        bool _purged;
        long _frameSize;

        void PurgeFrames();
        void Save();

	public:
		RenderCacheItem(const std::string& file);
		RenderCacheItem(RenderCache* renderCache, Effect* effect, RenderBuffer* buffer);
		virtual ~RenderCacheItem();
		void GetFrame(RenderBuffer* buffer);
		void AddFrame(RenderBuffer* buffer);
        bool IsMatch(Effect* effect, RenderBuffer* buffer);
        void Delete();
        bool IsDone(RenderBuffer* buffer) const;
};

class RenderCache
{
	std::string _cacheFolder;
	std::list<RenderCacheItem*> _cache;
    bool _enabled;
	
    void Close();

    public:
		RenderCache();
		virtual ~RenderCache();
		void SetSequence(const std::string& sequenceFile);
		RenderCacheItem* GetItem(Effect* effect, RenderBuffer* buffer);
        std::string GetCacheFolder() const { return _cacheFolder; }
        void CleanupCache(SequenceElements* sequenceElements);
        void Purge(bool dodelete);
        void Enable(bool enabled) { _enabled = enabled; }
};

#endif // RENDERCACHE_H
