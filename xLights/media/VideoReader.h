#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include "VideoFrame.h"

class VideoReaderImpl;

class VideoReader
{
public:
    static bool IsVideoFile(const std::string &filename);
    static long GetVideoLength(const std::string& filename);
	VideoReader(const std::string& filename, int width, int height, bool keepaspectratio, bool usenativeresolution = false,
                bool wantAlpha = false, bool bgr = false, bool wantsHardwareDecoderType = false);
	~VideoReader();
	int GetLengthMS() const;
	void Seek(int timestampMS, bool readFrame = true);
	VideoFrame* GetNextFrame(int timestampMS, int gracetime = 0);
	bool IsValid() const;
	int GetWidth() const;
	int GetHeight() const;
	bool AtEnd() const;
    int GetPos();
    std::string GetFilename() const;
    int GetPixelChannels() const;
    static void SetHardwareAcceleratedVideo(bool accel);
    static void SetHardwareRenderType(int type);
    static bool IsHardwareAcceleratedVideo();
    static int GetHardwareRenderType();
    static void InitHWAcceleration();
private:
    VideoReaderImpl* _impl = nullptr;
};
