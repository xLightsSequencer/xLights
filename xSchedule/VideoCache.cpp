#include "VideoCache.h"
#include "../xLights/VideoReader.h"
#include <log4cpp/Category.hh>

#include "../xLights/JobPool.h"

//#define VIDEO_EXTRALOGGING

VideoCache* VideoCache::_videoCache = nullptr;

class VideoCacheItem
{
public:

    std::string Key;
    long Timestamp;
    wxMilliClock_t LastAccessed;
    wxImage Image;
    bool Purged;

    VideoCacheItem(const std::string& key, long timestamp, const wxImage& image)
    {
        Key = key;
        Timestamp = timestamp;
        Image = image;
        Purged = false;
        LastAccessed = wxGetLocalTimeMillis();
    }

    wxSize GetSize() const
    {
        return Image.GetSize();
    }

    void Touch()
    {
        LastAccessed = wxGetLocalTimeMillis();
    }
};

class VideoReadJob : Job
{
private:
    VideoCache* _videoCache;
    std::string _videoFile;
    long _start;
    long _end;
    int _frameMS;
    wxSize _size;
    bool _keepAspectRatio;

public:
    VideoReadJob(VideoCache* videoCache, const std::string& videoFile, long millisecond, long duration, int framems, const wxSize& size, bool keepAspectRatio)
    {
        _videoCache = videoCache;
        _videoFile = videoFile;
        _start = millisecond;
        _end = _start + duration;
        _frameMS = framems;
        _size = size;
        _keepAspectRatio = keepAspectRatio;
    }
    virtual ~VideoReadJob() {};
    // set the running indicator … exit if it was already set.
    // read the frames … and then clear the reader flag. All access to member variables must be guarded.
    virtual void Process() override
    {
        xxx;
    }
    virtual std::string GetStatus() override { return ""; }
    virtual bool DeleteWhenComplete() override { return true; }
    virtual const std::string GetName() const override { return "VideoRead"; }
};

VideoCache* VideoCache::GetVideoCache()
{
    if (_videoCache == nullptr)
    {
        _videoCache = new VideoCache(10 * 60 * 30); // 10 minutes of video
    }

    return _videoCache;
}

VideoCache::VideoCache(int maxItems)
{
    _maxItems = maxItems;
}

std::string VideoCache::CreateKey(const std::string& videoFile, const wxSize& size) const
{
    return wxString::Format("%s_%d_%d", (const char *)videoFile.c_str(), size.GetX(), size.GetY()).ToStdString();
}

VideoCacheItem* VideoCache::GetVideoCacheItem(const std::string& key, long millisecond)
{
    // assume this thread already has the lock
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if ((*it)->Key == key && (*it)->Timestamp == millisecond)
        {
            return *it;
        }
    }

    return nullptr;
}

VideoCache::~VideoCache()
{
    std::unique_lock<std::mutex> locker(_cacheAccess);

    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        delete *it;
    }
    _cache.clear();

    for (auto it = _readers.begin(); it != _readers.end(); ++it)
    {
        delete it->second;
    }
    _readers.clear();
}

wxImage VideoCache::GetImage(const std::string& videoFile, long millisecond, int frameTime, const wxSize& size)
{
    std::string key = CreateKey(videoFile, size);
    {
        std::unique_lock<std::mutex> locker(_cacheAccess);
        VideoCacheItem* item = GetVideoCacheItem(key, millisecond);
        if (item != nullptr)
        {
            item->Touch();
            return item->Image;
        }
    }

    if (!EnsureCached(videoFile, millisecond, 60000, frameTime, size, false))
    {
        return wxImage(size, true);
    }

    return GetImage(videoFile, millisecond, frameTime, size);
}

bool VideoCache::EnsureCached(const std::string& videoFile, long millisecond, long duration, int frameTime, const wxSize& size, bool keepAspectRatio)
{
    std::string key = CreateKey(videoFile, size);
    {
        // check if we already have a thread reading the video
        std::unique_lock<std::mutex> locker(_cacheAccess);
        if (_readerInUse[key])
        {
            VideoCacheItem* item = GetVideoCacheItem(key, millisecond);
            return (item != nullptr);
        }
    }

    // check if all the required frames are in the cache already
    long firstFrameNotThere = millisecond;
    bool allThere = true;
    {
        {
            std::unique_lock<std::mutex> locker(_cacheAccess);
            for (long ms = millisecond; ms <= millisecond + duration; ms += frameTime)
            {
                if (GetVideoCacheItem(key, ms) == nullptr)
                {
                    firstFrameNotThere = ms;
                    allThere = false;
                    break;
                }
            }
        }
    }

    if (allThere)
    {
        return true;
    }

    // so not all there ... we need to spin up a thread to read the missing frames
    Job* job = (Job*)new VideoReadJob(this, videoFile, millisecond, duration, frameTime, size, keepAspectRatio);
    _jobPool.PushJob(job);

    if (firstFrameNotThere > millisecond)
    {
        // at least the first frame is there so we can exit now
        return true;
    }

    // so we have to wait for the first frame to be there
    // wait up to half a frame time
    wxMilliClock_t expire = wxGetLocalTimeMillis() + frameTime / 2;
    while (wxGetLocalTimeMillis() < expire)
    {
        {
            std::unique_lock<std::mutex> locker(_cacheAccess);
            VideoCacheItem* vci = GetVideoCacheItem(key, millisecond);
            if (vci != nullptr) return true;
        }
        wxMilliSleep(2);
    }

    return false;
}

void VideoCache::PurgeVideo(const std::string& videoFile, const wxSize& size)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    
    std::string key = CreateKey(videoFile, size);
    
    auto it = _readers.find(key);
    if (it != _readers.end())
    {
        delete it->second;
        _readers.erase(it);
    }

    auto it2 = _readerInUse.find(key);
    if (it2 != _readerInUse.end())
    {
        _readerInUse.erase(it2);
    }

    for (auto it3 = _cache.begin(); it3 != _cache.end(); ++it3)
    {
        if ((*it3)->Key == key)
        {
            (*it3)->Purged = true;
        }
    }
}

long VideoCache::GetLengthMS(const std::string& videoFile, const wxSize &size)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    std::string key = CreateKey(videoFile, size);
    if (_readers.find(key) == _readers.end())
    {
        return 0;
    }
    else
    {
        return _readers[key]->GetLengthMS();
    }
}

void VideoCache::FreeSpace(int frames)
{
    if (_cache.size() < _maxItems - frames)
    {
        // already enough space
        return;
    }

    std::unique_lock<std::mutex> locker(_cacheAccess);
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if ((*it)->Purged)
        {
            auto itprior = _cache.end();
            if (it == _cache.begin())
            {
            }
            else
            {
                itprior = it;
                --itprior;
            }

            delete *it;
            _cache.erase(it);

            if (_cache.size() < _maxItems - frames)
            {
                // already enough space
                return;
            }

            if (itprior == _cache.end())
            {
                it = _cache.begin();
            }
            else
            {
                it = itprior;
            }
        }
    }

    // not enough space ... so now we need to purge old stuff
    // remove anything not touched for 2 minutes
    wxMilliClock_t maxage = wxGetLocalTimeMillis() - 2 * 60000;
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if ((*it)->LastAccessed  < maxage)
        {
            auto itprior = _cache.end();
            if (it == _cache.begin())
            {
            }
            else
            {
                itprior = it;
                --itprior;
            }

            delete *it;
            _cache.erase(it);

            if (_cache.size() < _maxItems - frames)
            {
                // already enough space
                return;
            }

            if (itprior == _cache.end())
            {
                it = _cache.begin();
            }
            else
            {
                it = itprior;
            }
        }
    }

    // at this point we failed to free enough space
    // so we are going to grow the buffer
    _maxItems = _cache.size() + frames;
}

wxImage VideoCache::CreateImageFromFrame(AVFrame* frame, const wxSize& size) const
{
    if (frame != nullptr)
    {
        wxImage img(frame->width, frame->height, (unsigned char *)frame->data[0], true);
        img.SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
        return img;
    }
    else
    {
        wxImage img(size.x, size.y, true);
        return img;
    }
}
