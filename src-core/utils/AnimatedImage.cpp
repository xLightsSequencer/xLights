/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AnimatedImage.h"

#include <algorithm>
#include <filesystem>

#include <log.h>

bool AnimatedImage::IsGIF(const std::string& filename)
{
	auto ext = std::filesystem::path(filename).extension().string();
	if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return (ext == "gif");
}

AnimatedImage::~AnimatedImage()
{
}

AnimatedImage::AnimatedImage(const std::string& filename, const AnimatedImageData& data, bool suppressBackground)
    : _backgroundColor(data.backgroundColor)
    , _imageWidth(data.width)
    , _imageHeight(data.height)
    , _suppressBackground(suppressBackground)
    , _filename(filename)
{
    if (data.frames.empty()) {
        _ok = false;
        return;
    }

    _ok = true;
    _totalTime = 0;

    for (size_t i = 0; i < data.frames.size(); i++) {
        _frameImages.push_back(data.frames[i]);
        _frameTimes.push_back(data.frameTimes[i]);
        _totalTime += data.frameTimes[i];
    }

    if (_totalTime == 0) {
        _frameTimes.clear();
        _totalTime = 100 * (int)_frameImages.size();
        for (size_t x = 0; x < _frameImages.size(); x++) {
            _frameTimes.push_back(100);
        }
    }

    // Use pre-composited noBG frames if available
    if (!data.framesNoBG.empty()) {
        for (size_t i = 0; i < data.framesNoBG.size(); i++) {
            _frameImagesNoBG.push_back(data.framesNoBG[i]);
        }
    } else if (_suppressBackground) {
        BuildNoBGFrames();
    } else {
        _frameImagesNoBG = _frameImages;
    }
}

void AnimatedImage::BuildNoBGFrames() {
    _frameImagesNoBG.clear();
    _frameImagesNoBG.reserve(_frameImages.size());
    for (const auto& img : _frameImages) {
        xlImage noBg(img.GetWidth(), img.GetHeight());
        for (int y = 0; y < img.GetHeight(); y++) {
            for (int x = 0; x < img.GetWidth(); x++) {
                uint8_t r = img.GetRed(x, y);
                uint8_t g = img.GetGreen(x, y);
                uint8_t b = img.GetBlue(x, y);
                uint8_t a = img.GetAlpha(x, y);
                if (r == _backgroundColor.red && g == _backgroundColor.green &&
                    b == _backgroundColor.blue && a > 0) {
                    noBg.SetRGBA(x, y, 0, 0, 0, 0);
                } else {
                    noBg.SetRGBA(x, y, r, g, b, a);
                }
            }
        }
        _frameImagesNoBG.push_back(std::move(noBg));
    }
}

int AnimatedImage::GetMSUntilNextFrame(int msec, bool loop) const
{
    if (loop) {
        while (msec >= _totalTime) {
            msec -= _totalTime;
        }
    }

    if (msec > _totalTime) return -1;

    for (auto it = _frameTimes.begin(); it != _frameTimes.end(); ++it) {
        if (msec < *it) {
            return *it - msec;
        }
        msec -= *it;
    }

    return -1;
}

int AnimatedImage::CalcFrameForTime(int msec, bool loop) const
{
    if (_totalTime == 0) return 0;

	if (loop) {
		while (msec >= _totalTime) {
			msec -= _totalTime;
		}
	}

	if (msec > _totalTime) return -1;

	int frame = 0;
	for (auto it = _frameTimes.begin(); it != _frameTimes.end(); ++it) {
		if (msec < *it) {
			return frame;
		}
		msec -= *it;
		frame++;
	}

	return frame - 1;
}

const xlImage& AnimatedImage::GetFrameForTime(int msec, bool loop) const
{
	int frame = CalcFrameForTime(msec, loop);
    if (frame < 0 || frame >= (int)_frameImages.size()) {
        static xlImage invalid;
        return invalid;
    }
	return GetFrame(frame);
}

void AnimatedImage::ResetSuppressBackground(bool suppressBackground) {
    _suppressBackground = suppressBackground;
    if (_suppressBackground) {
        BuildNoBGFrames();
    } else {
        _frameImagesNoBG = _frameImages;
    }
}

const xlImage& AnimatedImage::GetFrame(int frame) const
{
    if (_suppressBackground && frame < (int)_frameImagesNoBG.size()) {
        return _frameImagesNoBG[frame];
    }
    if (frame < (int)_frameImages.size()) {
        return _frameImages[frame];
    }
    static xlImage invalid;
    return invalid;
}
