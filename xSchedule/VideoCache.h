#ifndef VIDEOCACHE_H
#define VIDEOCACHE_H

#include <mutex>

#include <wx/wx.h>

#include <string>
#include <list>
#include <map>
#include "../xLights/JobPool.h"

class VideoReader;
class VideoCacheItem;
struct AVFrame;

// basically a class needing to read a cached video uses this class to access a video reader and as much
// as possible don’t use the underlying video reader
class VideoCache
{
    static VideoCache* _videoCache;
    std::mutex _cacheAccess;
    int _maxItems;
    JobPool _jobPool;

    // cross thread variables … all access to these must be guarded
    std::list<VideoCacheItem*> _cache;
    std::map<std::string, int> _lengthMS;
    std::list<std::string> _reading;

    // purges enough images from the cache to create this much room … if there isn’t already this much room
    void FreeSpace(int frames);
    std::string CreateKey(const std::string& videoFile, const wxSize& size) const;
    VideoCacheItem* GetVideoCacheItem(const std::string& key, long millisecond);

public:

    static VideoCache* GetVideoCache();
    static wxImage CreateImageFromFrame(AVFrame* frame, const wxSize& size);
    VideoCache(int maxItems);
    virtual ~VideoCache();
    wxImage GetImage(const std::string& videoFile, long millisecond, int frameTime, const wxSize& size);
    bool Cache(const std::string& videoFile, long startMillisecond, long durationMilliseconds, int frameTime, const wxSize& size, bool keepAspectRatio);
    void PurgeVideo(const std::string& videoFile, const wxSize& size);
    long GetLengthMS(const std::string& videoFile);

    void Start(const std::string& videoFile, const wxSize& size);
    void Done(const std::string& videoFile, const wxSize& size);
    void CacheImage(const std::string& videoFile, const wxSize& size, long millisecond, const wxImage& image);
    void SetLengthMS(const std::string& videoFile, long lengthMS);
};

#endif // VIDEOCACHE_H
