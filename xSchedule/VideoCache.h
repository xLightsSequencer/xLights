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
class AVFrame;

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
    // bool is true if a read thread is running … we should only have one per video reader
    std::map<std::string, VideoReader*> _readers;
    std::map<std::string, bool> _readerInUse;

    // purges enough images from the cache to create this much room … if there isn’t already this much room
    void FreeSpace(int frames);
    wxImage CreateImageFromFrame(AVFrame* frame, const wxSize& size) const;
    std::string CreateKey(const std::string& videoFile, const wxSize& size) const;
    VideoCacheItem* GetVideoCacheItem(const std::string& key, long millisecond);

public:

    static VideoCache* GetVideoCache();
    VideoCache(int maxItems);
    virtual ~VideoCache();
    // get an image from the cache … if not there read it and add it to the cache
    // if we don’t have a reader … create one
    wxImage GetImage(const std::string& videoFile, long millisecond, int frameTime, const wxSize& size);
    // check cache contains all frames from the frame before the start until the end … read them if necessary from disk
    // if we don’t have a reader … create one
    // spawn a thread to do this work … only return when the first frame is in the cache
    // if there is already a thread running skip it … unless the first frame is not available … if that is the case wait until thread finishes and if the frame still isn’t there start the thread and wait for that frame … this is undesirable but it stops lots of pausing for frames … it works well if the thread reading the video can get ahead … if it cant worst case we revert to inline which is slow
    bool EnsureCached(const std::string& videoFile, long startMillisecond, long durationMilliseconds, int frameTime, const wxSize& size, bool keepAspectRatio);
    // tags frames as purged … purged frames will be the first ones discarded
    // also should delete the video reader as it is not going to be requested again
    void PurgeVideo(const std::string& videoFile, const wxSize& size);
    long GetLengthMS(const std::string& videoFile, const wxSize& size);
};

#endif // VIDEOCACHE_H
