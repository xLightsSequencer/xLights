#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <mutex>
#include <wx/wx.h>
#include <string>
#include <map>
#include "../xLights/JobPool.h"

class VideoReader;
class VideoCacheItem;
class CVRThread;
struct AVFrame;

class CachedVideoReader
{
    std::map<long, wxImage> _cache;
    std::mutex _cacheAccess;
    int _maxItems;
    CVRThread* _thread;
    int _frameTime;
    std::string _videoFile;
    wxSize _size;
    long _lengthMS;
    bool _done;

public:
    CachedVideoReader(const std::string& videoFile, long startMillisecond, int frameTime, const wxSize& size, bool keepAspectRatio);
    virtual ~CachedVideoReader();

    static wxImage CreateImageFromFrame(AVFrame* frame, const wxSize& size);
    static wxImage FadeImage(const wxImage& image, int brightness);

    bool HasFrame(long millisecond);
    void CacheImage(long millisecond, const wxImage& image);
    void SetLengthMS(long lengthMS);
    void Done();
    void PurgeCachePriorTo(long start);

    long GetLengthMS() const { return _lengthMS; };
    wxImage GetNextFrame(long ms);
};

