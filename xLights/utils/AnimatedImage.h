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

#include <functional>
#include <string>
#include <vector>
#include "xlImage.h"
#include "../Color.h"

class AnimatedImage
{
    std::vector<long> _frameTimes;
    std::vector<xlImage> _frameImages;
    std::vector<xlImage> _frameImagesNoBG;
    xlColor _backgroundColor;
    int _imageWidth = 0;
    int _imageHeight = 0;
	long _totalTime = 0;
    bool _suppressBackground = false;
    std::string _filename;
    bool _ok = false;

    int CalcFrameForTime(int msec, bool loop) const;
    void BuildNoBGFrames();

public:
    // Construct from pre-decoded animation data
    AnimatedImage(const std::string& filename, const AnimatedImageData& data, bool suppressBackground = true);
    virtual ~AnimatedImage();
    const xlImage &GetFrame(int frame) const;
    const xlImage &GetFrameForTime(int msec, bool loop) const;
    int GetMSUntilNextFrame(int msec, bool loop) const;
    std::string GetFilename() const { return _filename; }
    bool IsOk() const { return _ok; }

    int GetNumFrames() const { return (int)_frameImages.size(); }
    int GetFrameTime(int x) const { return _frameTimes[x]; }
    int GetTotalTime() const { return _totalTime; }
    int GetImageWidth() const { return _imageWidth; }
    int GetImageHeight() const { return _imageHeight; }

    void ResetSuppressBackground(bool suppressBackground);

    static bool IsGIF(const std::string& filename);
};
