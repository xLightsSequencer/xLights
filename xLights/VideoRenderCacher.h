#ifndef VIDEORENDERCACHER_H
#define VIDEORENDERCACHER_H

#include <string>
#include <list>
#include <wx/file.h>
#include "RenderBuffer.h"

// Valid property tags
#define VRC_FILENAME "Filename"
#define VRC_STARTTIME "StartTime"
#define VRC_ENDTIME "EndTime"
#define VRC_FRAMEMS "FrameMS"
#define VRC_CROPLEFT "CropLeft"
#define VRC_CROPRIGHT "CropRight"
#define VRC_CROPTOP "CropTop"
#define VRC_CROPBOTTOM "CropBottom"
#define VRC_KEEPASPECTRATIO "KeepAR"
#define VRC_ADJUSTEDFRAMETIME "AdjustedFrameMS"
#define VRC_MODELWIDTH "ModelWidth"
#define VRC_MODELHEIGHT "ModelHeight"
#define VRC_HEADEREND "HEADEREND"

class VideoRenderCacheItem
{
		std::string _cacheFile;
		std::map<std::string, std::string> _properties;
		wxFile _file;
		unsigned long _fileSize;
		bool _created;
        std::map<int, unsigned char *> _frames;
        long _frameSize;
        long _firstFrameOffset;

        void Save();

	public:
		VideoRenderCacheItem(const std::string& file);
		VideoRenderCacheItem(const std::string& cacheFolder, const std::string& fileName);
		virtual ~VideoRenderCacheItem();
		void SetProperty(const std::string& propertyName, long value);
		void SetProperty(const std::string& propertyName, const std::string& value);
		bool IsPropertyMatch(std::string propertyName, long value);
		bool IsPropertyMatch(std::string propertyName, std::string value);
		void GetFrame(int frame, RenderBuffer &buffer);
		void AddFrame(int frame, RenderBuffer &buffer);
		void Finished();
		bool WasCreated() const { return _created; }
};

class VideoRenderCacher
{
	std::string _cacheFolder;
	std::list<VideoRenderCacheItem*> _cache;
	
    void PurgeCache();

	public:
		VideoRenderCacher();
		virtual ~VideoRenderCacher();
		void SetShowFolder(const std::string& showFolder);
		VideoRenderCacheItem* Get(const std::string& videoFile, int adjustedFrameTime, int sequenceFrameTime,
                 int cropLeft, int cropRight, int cropTop, int cropBottom,
                long startMS, long endMS, bool keepAspectRatio, int modelWidth, int modelHeight);
};

#endif // VIDEORENDERCACHE_H
