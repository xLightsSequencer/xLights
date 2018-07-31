#include "VideoRenderCacher.h"
#include <log4cpp/Category.hh>
#include <wx/filename.h>
#include <wx/dir.h>

VideoRenderCacher::VideoRenderCacher()
{
    _cacheFolder = "";
}

VideoRenderCacher::~VideoRenderCacher()
{
    PurgeCache();
}

void VideoRenderCacheItem::Save()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    char zero = 0x00;
    wxASSERT(_frameSize >= 0);

    if (_file.IsOpened())
    {
        _file.Close();
    }

    if (_file.Create(_cacheFile, true))
    {
        logger_base.debug("Saving video cache %s.", (const char*)_cacheFile.c_str());

        // write the header fields
        for (auto it = _properties.begin(); it != _properties.end(); ++it)
        {
            _file.Write(it->first);
            _file.Write(&zero, 1);
            _file.Write(it->second);
            _file.Write(&zero, 1);
        }

        _file.Write(VRC_HEADEREND);
        _file.Write(&zero, 1);

        // write the frames
        for (auto it = _frames.begin(); it != _frames.end(); ++it)
        {
            _file.Write(it->second, _frameSize * 3);
        }
        _file.Close();

        wxFileName fn(_cacheFile);
        _fileSize = fn.GetSize().ToULong();
    }
}

VideoRenderCacheItem::VideoRenderCacheItem(const std::string& file)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _cacheFile = file;
    wxFileName fn(_cacheFile);
    _created = false;
    _fileSize = fn.GetSize().ToULong();

    if (_file.Open(_cacheFile))
    {
        logger_base.debug("Reading video cache %s.", (const char*)_cacheFile.c_str());

        char headerBuffer[4096];
        memset(headerBuffer, 0x00, sizeof(headerBuffer));
        _file.Read(headerBuffer, sizeof(headerBuffer));

        char* ps = headerBuffer;

        while (strcmp(ps, VRC_HEADEREND) != 0)
        {
            std::string key(ps);
            ps += strlen(ps) + 1;
            std::string value(ps);
            ps += strlen(ps) + 1;

            SetProperty(key, value);
        }
        ps += strlen(ps) + 1;

        _frameSize = wxAtoi(_properties[VRC_MODELWIDTH]) * wxAtoi(_properties[VRC_MODELHEIGHT]);
        _firstFrameOffset = ps - headerBuffer;

        _file.Close();
    }
}

VideoRenderCacheItem::VideoRenderCacheItem(const std::string& cacheFolder, const std::string& fileName)
{
    wxFileName fn(fileName);
    int i = 1;
    do
    {
        _cacheFile = cacheFolder + wxFileName::GetPathSeparator() + fn.GetName() + "_" + wxString::Format("%d", i) + ".vcf";
        i++;
    }
    while (wxFile::Exists(_cacheFile));
    _fileSize = fn.GetSize().ToULong();
    _created = true;
    _frameSize = -1;
    _firstFrameOffset = -1;
}

VideoRenderCacheItem::~VideoRenderCacheItem()
{
    for (auto it = _frames.begin(); it != _frames.end(); ++it)
    {
        free(it->second);
    }

    if (_file.IsOpened())
    {
        _file.Close();
    }
}

void VideoRenderCacheItem::SetProperty(const std::string& propertyName, long value)
{
    _properties[propertyName] = wxString::Format("%ld", value).ToStdString();
}

void VideoRenderCacheItem::SetProperty(const std::string& propertyName, const std::string& value)
{
    _properties[propertyName] = value;
}

bool VideoRenderCacheItem::IsPropertyMatch(std::string propertyName, long value) 
{
    if (_properties.find(propertyName) != _properties.end())
    {
        return (_properties[propertyName] == wxString::Format("%ld", value).ToStdString());
    }
    return false;
}

bool VideoRenderCacheItem::IsPropertyMatch(std::string propertyName, std::string value) 
{
    if (_properties.find(propertyName) != _properties.end())
    {
        return (_properties[propertyName] == value);
    }
    return false;
}

void VideoRenderCacheItem::GetFrame(int frame, RenderBuffer& buffer, bool transparentBlack, int transparentBlackLevel)
{
    wxASSERT(_frameSize == buffer.BufferWi * buffer.BufferHt);

    if (_frames.find(frame) != _frames.end())
    {
        // its in memory ... read it from there
        unsigned char* pc = _frames.find(frame)->second;

        for (int y = 0; y < buffer.BufferHt; y++)
        {
            for (int x = 0; x < buffer.BufferWi; x++)
            {
                xlColor c(*pc, *(pc + 1), *(pc + 2));
                pc += 3;

                if (transparentBlack)
                {
                    int level = c.Red() + c.Green() + c.Blue();
                    if (level > transparentBlackLevel)
                    {
                        buffer.SetPixel(x, y, c);
                    }
                }
                else
                {
                    buffer.SetPixel(x, y, c);
                }
            }
        }
    }
    else
    {
        wxASSERT(_firstFrameOffset > 0);

        if (!_file.IsOpened())
        {
            _file.Open(_cacheFile);
        }

        long pos = _firstFrameOffset + _frameSize * 3 * frame;

        if (pos == _file.Seek(wxFileOffset(pos)))
        {
            unsigned char* frameData = (unsigned char*)malloc(_frameSize * 3);

            _file.Read(frameData, _frameSize * 3);
            unsigned char* pc = frameData;

            for (int y = 0; y < buffer.BufferHt; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    xlColor c(*pc, *(pc + 1), *(pc + 2));
                    pc += 3;
                    buffer.SetPixel(x, y, c);
                }
            }

            free(frameData);
        }

        if (frame >= (wxAtoi(_properties[VRC_ENDTIME]) - wxAtoi(_properties[VRC_STARTTIME]))/wxAtoi(_properties[VRC_FRAMEMS]))
        {
            _file.Close();
        }
    }
}

void VideoRenderCacheItem::AddFrame(int frame, RenderBuffer& buffer)
{
    if (_frameSize == -1)
    {
        _frameSize = buffer.BufferWi * buffer.BufferHt;
        SetProperty(VRC_MODELWIDTH, buffer.BufferWi);
        SetProperty(VRC_MODELHEIGHT, buffer.BufferHt);
    }
    else
    {
        wxASSERT(_frameSize == buffer.BufferWi * buffer.BufferHt);
    }

    unsigned char* frameBuffer = (unsigned char *)malloc(_frameSize * 3);
    unsigned char* pc = frameBuffer;

    for (int y = 0; y < buffer.BufferHt; y++)
    {
        for (int x = 0; x < buffer.BufferWi; x++)
        {
            xlColor c = buffer.GetPixel(x, y);
            *pc = c.red;
            ++pc;
            *pc = c.green;
            ++pc;
            *pc = c.blue;
            ++pc;
        }
    }

    _frames[frame] = frameBuffer;
}

void VideoRenderCacheItem::Finished()
{
    _created = false;
    Save();
}

void VideoRenderCacher::PurgeCache()
{
    // delete all the cached items
    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        delete *it;
    }
    _cache.clear();
}

void VideoRenderCacher::SetShowFolder(const std::string& showFolder)
{

    if (_cacheFolder == showFolder + wxFileName::GetPathSeparator() + "VideoRenderCache")
    {
        return;
    }

    _cacheFolder = showFolder + wxFileName::GetPathSeparator() + "VideoRenderCache";

    if (!wxDir::Exists(_cacheFolder))
    {
        wxDir::Make(_cacheFolder);
    }

    PurgeCache();

    wxDir dir(_cacheFolder);
    wxArrayString files;
    dir.GetAllFiles(_cacheFolder, &files, "*.vcf");

    for (auto it = files.begin(); it != files.end(); ++it)
    {
        _cache.push_back(new VideoRenderCacheItem(*it));
    }
}

VideoRenderCacheItem* VideoRenderCacher::Get(const std::string& videoFile, int adjustedFrameTime, int sequenceFrameTime, int cropLeft, int cropRight, int cropTop, int cropBottom, long startMS, long endMS, bool keepAspectRatio, int modelWidth, int modelHeight)
{
    wxASSERT(_cacheFolder != "");
    if (_cacheFolder == "") return nullptr;

    for (auto it = _cache.begin(); it != _cache.end(); ++it)
    {
        if ((*it)->IsPropertyMatch(VRC_FILENAME, videoFile) &&
            (*it)->IsPropertyMatch(VRC_FRAMEMS, sequenceFrameTime) &&
            (*it)->IsPropertyMatch(VRC_ADJUSTEDFRAMETIME, adjustedFrameTime) &&
            (*it)->IsPropertyMatch(VRC_STARTTIME, startMS) &&
            (*it)->IsPropertyMatch(VRC_ENDTIME, endMS) &&
            (*it)->IsPropertyMatch(VRC_KEEPASPECTRATIO, keepAspectRatio) &&
            (*it)->IsPropertyMatch(VRC_MODELWIDTH, modelWidth) &&
            (*it)->IsPropertyMatch(VRC_MODELHEIGHT, modelHeight) &&
            (*it)->IsPropertyMatch(VRC_CROPLEFT, cropLeft) &&
            (*it)->IsPropertyMatch(VRC_CROPRIGHT, cropRight) &&
            (*it)->IsPropertyMatch(VRC_CROPTOP, cropTop) &&
            (*it)->IsPropertyMatch(VRC_CROPBOTTOM, cropBottom)
            )
        {
            return *it;
        }
    }

    VideoRenderCacheItem* item = new VideoRenderCacheItem(_cacheFolder, videoFile);
    item->SetProperty(VRC_FILENAME, videoFile);
    item->SetProperty(VRC_FRAMEMS, sequenceFrameTime);
    item->SetProperty(VRC_ADJUSTEDFRAMETIME, adjustedFrameTime);
    item->SetProperty(VRC_STARTTIME, startMS);
    item->SetProperty(VRC_ENDTIME, endMS);
    item->SetProperty(VRC_KEEPASPECTRATIO, keepAspectRatio);
    item->SetProperty(VRC_CROPLEFT, cropLeft);
    item->SetProperty(VRC_CROPRIGHT, cropRight);
    item->SetProperty(VRC_CROPTOP, cropTop);
    item->SetProperty(VRC_CROPBOTTOM, cropBottom);
    item->SetProperty(VRC_MODELWIDTH, modelWidth);
    item->SetProperty(VRC_MODELHEIGHT, modelHeight);
    _cache.push_back(item);
    return item;
}