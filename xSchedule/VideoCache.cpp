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
    wxImage Image;
    bool Purged;

    VideoCacheItem(const std::string& key, long timestamp, const wxImage& image)
    {
        Key = key;
        Timestamp = timestamp;
        Image = image.Copy();
        Purged = false;
    }

    wxSize GetSize() const
    {
        return Image.GetSize();
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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("Caching video %s (%dx%d)", (const char *)_videoFile.c_str(), _size.GetWidth(), _size.GetHeight());

        _videoCache->Start(_videoFile, _size);
        VideoReader videoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), _keepAspectRatio);

        if (videoReader.IsValid())
        {
            _videoCache->SetLengthMS(_videoFile, videoReader.GetLengthMS());
            logger_base.debug("Video length %ld.", videoReader.GetLengthMS());
            long end = std::min(_end, (long)videoReader.GetLengthMS());
            logger_base.debug("Caching %ld - %ld.", _start, end);
            for (long i = _start; i < end; i+= _frameMS)
            {
                _videoCache->CacheImage(_videoFile, _size, i, VideoCache::CreateImageFromFrame(videoReader.GetNextFrame(i), _size));
#ifdef VIDEO_EXTRALOGGING
                logger_base.debug("Cached frame %ld for %s (%ldx%ld).", i, _videoFile.c_str(), _size.GetWidth(), _size.GetHeight());
#endif
            }
        }
        else
        {
            logger_base.error("Error opening video file to cache the images. %s", (const char *)_videoFile.c_str());
            _videoCache->SetLengthMS(_videoFile, 0);
        }

        _videoCache->Done(_videoFile, _size);
        logger_base.debug("Caching video %s (%dx%d) ... DONE", (const char *)_videoFile.c_str(), _size.GetWidth(), _size.GetHeight());
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
}

#define TIMEOUT(a) a / 2

wxImage VideoCache::GetImage(const std::string& videoFile, long millisecond, int frameTime, const wxSize& size)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string key = CreateKey(videoFile, size);

    millisecond = millisecond / frameTime * frameTime;

    long lengthMS = 0;
    {
        std::unique_lock<std::mutex> locker(_cacheAccess);
        lengthMS = _lengthMS[videoFile];

        if (millisecond > lengthMS)
        {
            // we are past the end of the video
            return wxImage(size, true);
        }
    }

    wxMilliClock_t expire = wxGetLocalTimeMillis() + TIMEOUT(frameTime);
    while (wxGetLocalTimeMillis() < expire)
    {
        {
            std::unique_lock<std::mutex> locker(_cacheAccess);
            VideoCacheItem* vci = GetVideoCacheItem(key, millisecond);
            if (vci != nullptr)
            {
#ifdef VIDEO_EXTRALOGGING
                logger_base.debug("Found image in cache %s frame %ld", (const char *)key.c_str(), millisecond);
#endif
                wxASSERT(vci->Image.IsOk());
                return vci->Image.Copy();
            }
        }
        wxMilliSleep(2);
    }

    logger_base.warn("Timeout waiting for image %s frame %ld", (const char *)key.c_str(), millisecond);
    return wxImage(size, true);
}

bool VideoCache::Cache(const std::string& videoFile, long millisecond, long duration, int frameTime, const wxSize& size, bool keepAspectRatio)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string key = CreateKey(videoFile, size);
    {
        // check if we already have a thread reading the video
        std::unique_lock<std::mutex> locker(_cacheAccess);
        if (std::find(_reading.begin(), _reading.end(), key) != _reading.end())
        {
            logger_base.debug("Asked to cache %s (%ldx%ld) but we are in the middle of caching it.", (const char *)key.c_str(), size.GetWidth(), size.GetHeight());
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
                VideoCacheItem* vci = GetVideoCacheItem(key, ms);
                if (vci == nullptr)
                {
                    firstFrameNotThere = ms;
                    allThere = false;
                    break;
                }
                else
                {
                    vci->Purged = false;
                }
            }
        }
    }

    if (allThere)
    {
        logger_base.debug("%s (%ldx%ld) was already in the cache ... nothing to do.", (const char *)key.c_str(), size.GetWidth(), size.GetHeight());
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
    wxMilliClock_t expire = wxGetLocalTimeMillis() + TIMEOUT(frameTime);
    while (wxGetLocalTimeMillis() < expire)
    {
        {
            std::unique_lock<std::mutex> locker(_cacheAccess);
            VideoCacheItem* vci = GetVideoCacheItem(key, millisecond);
            if (vci != nullptr) return true;
        }
        wxMilliSleep(2);
    }

    logger_base.warn("%s (%ldx%ld) failed to get the first frame ... timed out.", (const char *)key.c_str(), size.GetWidth(), size.GetHeight());
    return false;
}

void VideoCache::PurgeVideo(const std::string& videoFile, const wxSize& size)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    
    std::string key = CreateKey(videoFile, size);
    if (std::find(_reading.begin(), _reading.end(), key) != _reading.end())
    {
        // not good we are still reading and now trying to delete ... not much i can do about that
    }

    for (auto it3 = _cache.begin(); it3 != _cache.end(); ++it3)
    {
        if ((*it3)->Key == key)
        {
            (*it3)->Purged = true;
        }
    }
}

long VideoCache::GetLengthMS(const std::string& videoFile)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    if (_lengthMS.find(videoFile) == _lengthMS.end())
    {
        return 0;
    }
    else
    {
        return _lengthMS[videoFile];
    }
}

void VideoCache::Start(const std::string& videoFile, const wxSize& size)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    std::string key = CreateKey(videoFile, size);
    _reading.push_back(key);
}

void VideoCache::Done(const std::string& videoFile, const wxSize& size)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    std::string key = CreateKey(videoFile, size);
    auto it = std::find(_reading.begin(), _reading.end(), key);
    if (it != _reading.end())
    {
        _reading.remove(key);
    }
}

void VideoCache::CacheImage(const std::string& videoFile, const wxSize& size, long millisecond, const wxImage& image)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    std::string key = CreateKey(videoFile, size);

    std::unique_lock<std::mutex> locker(_cacheAccess);
    VideoCacheItem* vci = GetVideoCacheItem(key, millisecond);
    if (vci == nullptr)
    {
        _cache.push_back(new VideoCacheItem(key, millisecond, image));
    }
    else
    {
        logger_base.debug("Cache already had the image.");
    }
}

void VideoCache::SetLengthMS(const std::string& videoFile, long lengthMS)
{
    std::unique_lock<std::mutex> locker(_cacheAccess);
    _lengthMS[videoFile] = lengthMS;
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

    // at this point we failed to free enough space
    // so we are going to grow the buffer
    _maxItems = _cache.size() + frames;
}

wxImage VideoCache::CreateImageFromFrame(AVFrame* frame, const wxSize& size)
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
