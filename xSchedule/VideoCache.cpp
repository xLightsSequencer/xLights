/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoCache.h"
#include "../xLights/utils/UtilFunctions.h"
#include "../xLights/ui/wxUtilities.h"
#include "../xLights/render/VideoReader.h"

#include <log.h>

// #define VIDEO_EXTRALOGGING

class CVRThread : public wxThread {
    std::string _videoFile;
    long _currentStart;
    int _maxFrames;
    std::mutex _access;
    wxSize _size;
    bool _stop;
    bool _running;
    VideoReader* _videoReader;
    int _frameMS;
    CachedVideoReader* _cvr = nullptr;
    std::condition_variable _signal;

    long GetCurrentStart() {
        std::unique_lock<std::mutex> locker(_access);
        return _currentStart;
    }

public:
    CVRThread(CachedVideoReader* cvr, int maxFrames, const std::string& videoFile, long startMillisecond, int frameMS, const wxSize& size, bool keepAspectRatio) {
        _cvr = cvr;
        _maxFrames = maxFrames;
        _videoFile = videoFile;
        _currentStart = startMillisecond;
        _size = size;
        _frameMS = frameMS;
        _stop = false;
        _running = false;
        _videoReader = new VideoReader(_videoFile, size.GetWidth(), size.GetHeight(), keepAspectRatio);

        if (_videoReader != nullptr && _videoReader->IsValid()) {
            _cvr->SetLengthMS(_videoReader->GetLengthMS());
            if (Run() != wxTHREAD_NO_ERROR) {
                spdlog::error("Failed to start video reading thread for {} ({}x{})", _videoFile, size.GetWidth(), size.GetHeight());
                delete _videoReader;
                _videoReader = nullptr;
            }
        } else {
            spdlog::error("Video reading thread not started for {} ({}x{}) because video could not be opened.", _videoFile, size.GetWidth(), size.GetHeight());

            if (_videoReader != nullptr) {
                delete _videoReader;
                _videoReader = nullptr;
            }
        }
    }
    virtual ~CVRThread() {

        if (_running && !_stop) {
            spdlog::debug("Asking video reading thread {} ({}x{}) to stop", _videoFile, _size.GetWidth(), _size.GetHeight());
            _stop = true;
            std::unique_lock<std::mutex> mutLock(_access);
            _signal.notify_all();
        }

        // now wait for exit
        while (_running) {
            wxMilliSleep(1);
        }

        spdlog::debug("Video reading thread {} ({}x{}) has stopped.", _videoFile, _size.GetWidth(), _size.GetHeight());

        if (_videoReader != nullptr) {
            delete _videoReader;
            _videoReader = nullptr;
        }
    }
    bool IsOk() {
        return _videoReader != nullptr;
    }
    void Stop() {

        spdlog::debug("Asking video reading thread {} ({}x{}) to stop", _videoFile, _size.GetWidth(), _size.GetHeight());
        _stop = true;
        std::unique_lock<std::mutex> mutLock(_access);
        _signal.notify_all();
    }

    virtual void* Entry() override {
        _running = true;


        spdlog::debug("Video reading thread {} ({}x{}) started", _videoFile, _size.GetWidth(), _size.GetHeight());

        int lastStart = -1;

        while (!_stop) {
            long currentStart = GetCurrentStart();
            if (lastStart != currentStart) {
                _cvr->PurgeCachePriorTo(currentStart);

                lastStart = currentStart;
#ifdef VIDEO_EXTRALOGGING
                long end = std::min((long)currentStart + _maxFrames * _frameMS, (long)_videoReader->GetLengthMS());
                spdlog::debug("Video reading thread {} ({}x{}) filling cache {}-{}", _videoFile, _size.GetWidth(), _size.GetHeight(), currentStart, end);
#endif

                // we need to refill the cache
                for (long i = currentStart; i < currentStart + _maxFrames * _frameMS && !_stop; i += _frameMS) {
                    if (!_cvr->HasFrame(i)) {
                        wxStopWatch sw;

                        //_videoReader->Seek(i);
                        _cvr->CacheImage(i, CachedVideoReader::CreateImageFromFrame(_videoReader->GetNextFrame(i), _size));

                        if (sw.Time() > _frameMS) {
                            spdlog::warn("Video reading thread {} ({}x{}) took more than {}ms to decode frame {}ms.", _videoFile, _size.GetWidth(), _size.GetHeight(), _frameMS, i);
                        }

                        // if we have fallen behind ... jump ahead - this will cause blank frames
                        long s = GetCurrentStart();
                        if (s > i + _frameMS) {
                            spdlog::warn("Video reading thread {} ({}x{}) has fallen behind ... jumping ahead ... video will go black.", _videoFile, _size.GetWidth(), _size.GetHeight());
                            i = s + 3 * _frameMS;
                        }
                    }
                }
            } else {
#ifdef VIDEO_EXTRALOGGING
                spdlog::debug("Video reading thread {} ({}x{}) waiting for start time to change.", _videoFile, _size.GetWidth(), _size.GetHeight());
#endif
                std::unique_lock<std::mutex> accessLock(_access);
                _signal.wait(accessLock);
#ifdef VIDEO_EXTRALOGGING
                spdlog::debug("Video reading thread {} ({}x{}) start time changed.", _videoFile, _size.GetWidth(), _size.GetHeight());
#endif
            }
        }

        if (_videoReader != nullptr) {
            delete _videoReader;
            _videoReader = nullptr;
        }

        _cvr->Done();
        _running = false;

        spdlog::debug("Video reading thread {} ({}x{}) stopped", _videoFile, _size.GetWidth(), _size.GetHeight());

        return nullptr;
    }
    void SetNewStart(int newStart) {
        std::unique_lock<std::mutex> mutLock(_access);
        _currentStart = newStart;
        _signal.notify_all();
    }
};

void CachedVideoReader::Done() {
    std::unique_lock<std::mutex> locker(_cacheAccess);
    _done = true;
}

CachedVideoReader::~CachedVideoReader() {
     

    if (_thread != nullptr) {
#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("Cached Video Reader destructor asking thread to stop. {}", _videoFile);
#endif
        _thread->Stop();
        // threads delete themselves when stopped

        bool done = false;
        {
            std::unique_lock<std::mutex> locker(_cacheAccess);
            done = _done;
        }

#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("Cached Video Reader destructor initial check shows thread is done. {}", done ? "TRUE" : "FALSE");
#endif

        // we will wait up to 2 seconds for this to happen because this can go bad
        int i = 0;
        while (!done && ++i < 2000) {
            wxMilliSleep(1);

            std::unique_lock<std::mutex> locker(_cacheAccess);
            done = _done;
        }

        if (!done) {
            spdlog::warn("Cached Video Reader destructor ... thread never finished");
        } else {
#ifdef VIDEO_EXTRALOGGING
            spdlog::debug("Cached Video Reader destructor thread seems to be done.");
#endif
        }

        // delete _thread;
        //_thread = nullptr;
    }

    {
        std::unique_lock<std::mutex> locker(_cacheAccess);

#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("Cached Video Reader destructor clearing cache.");
#endif

        _cache.clear();
    }
}

#define TIMEOUT(a) a / 2

void CachedVideoReader::CacheImage(long millisecond, const wxImage& image) {
#ifdef VIDEO_EXTRALOGGING
    static 
#endif

    std::unique_lock<std::mutex> locker(_cacheAccess);
    auto it = _cache.find(millisecond);
    if (it == _cache.end()) {
#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("Cached image for time {}.", millisecond);
#endif
        _cache[millisecond] = image.Copy();
    } else {
#ifdef VIDEO_EXTRALOGGING
        spdlog::debug("Cache already had the image.");
#endif
    }
}

void CachedVideoReader::SetLengthMS(long lengthMS) {
    std::unique_lock<std::mutex> locker(_cacheAccess);
    _lengthMS = lengthMS;
}

wxImage CachedVideoReader::GetNextFrame(long ms) {
 

    if (_thread == nullptr || ms > _lengthMS) {
        return wxImage(_size);
    }

    // round ms to frame boundary
    ms = ms / _frameTime * _frameTime;

    _thread->SetNewStart(ms);

    {
        std::unique_lock<std::mutex> locker(_cacheAccess);
        auto it = _cache.find(ms);
        if (it != _cache.end()) {
            return it->second.Copy();
        }
    }

    {
        spdlog::debug("Video {} ({}x{}) tried to get frame {} from cache but it wasnt there ... give it a bit of time.", _videoFile, _size.GetWidth(), _size.GetHeight(), ms);

        // give it a bit of time ... say half a frame
        int i = 0;
        while (i < _frameTime / 2) {
            wxMilliSleep(2);
            i += 2;

            {
                std::unique_lock<std::mutex> locker(_cacheAccess);
                auto it = _cache.find(ms);
                if (it != _cache.end()) {
                    return it->second.Copy();
                }
            }
        }
    }

    spdlog::debug("Video {} ({}x{}) tried to get frame {} from cache but it wasnt there :(", _videoFile, _size.GetWidth(), _size.GetHeight(), ms);
    return wxImage(_size);
}

wxImage CachedVideoReader::CreateImageFromFrame(AVFrame* frame, const wxSize& size) {
    if (frame != nullptr) {
        wxImage img(frame->width, frame->height, (unsigned char*)frame->data[0], true);
        img.SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
        return img;
    } else {
        wxImage img(size.x, size.y, true);
        return img;
    }
}

wxImage CachedVideoReader::FadeImage(const wxImage& image, int brightness) {
    unsigned char btable[256];

    wxImage faded;

    if (!image.IsOk() || brightness == 0) {
        // create a blank image if it is invalid or faded to black
        faded.Create(image.GetWidth(), image.GetHeight());
    } else {
        faded = image.Copy();
        if (brightness != 100) {
            for (int i = 0; i < 256; i++) {
                btable[i] = i * brightness / 100;
            }

            unsigned char* pdata = faded.GetData();

            for (int i = 0; i < faded.GetWidth() * faded.GetHeight() * 3; i++) {
                *(pdata + i) = btable[*(pdata + i)];
            }
        }
    }

    return faded;
}

bool CachedVideoReader::HasFrame(long millisecond) {
    std::unique_lock<std::mutex> locker(_cacheAccess);
    auto it = _cache.find(millisecond);
    return (it != _cache.end());
}

#define CALCCACHESIZE(a) 5 * 1000 / a
void CachedVideoReader::PurgeCachePriorTo(long start) {
    std::unique_lock<std::mutex> locker(_cacheAccess);

    auto it = _cache.begin();
    while (it != _cache.end()) {
        if (it->first < start) {
            auto next = it;
            ++next;
            _cache.erase(it);
            it = next;
        } else {
            ++it;
        }
    }
}

CachedVideoReader::CachedVideoReader(const std::string& videoFile, long startMillisecond, int frameTime, const wxSize& size, bool keepAspectRatio) {
    _done = false;
    _maxItems = CALCCACHESIZE(frameTime);
    _frameTime = frameTime;
    _videoFile = FixFile("", videoFile);
    _size = size;
    _lengthMS = 0;
    _thread = new CVRThread(this, _maxItems, _videoFile, startMillisecond, _frameTime, _size, keepAspectRatio);
    if (!_thread->IsOk()) {
        delete _thread;
        _thread = nullptr;
    }
}
