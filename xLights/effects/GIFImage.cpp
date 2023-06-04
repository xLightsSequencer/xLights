/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "GIFImage.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>

//#define DEBUG_GIF

bool GIFImage::IsGIF(const std::string& filename)
{
	wxFileName fn(filename);
	return (fn.GetExt().Lower() == "gif");
}

GIFImage::~GIFImage()
{
    wxLogNull logNo;  // suppress popups from gif images.
    _gifDecoder.Destroy();
}

GIFImage::GIFImage(const std::string& filename, bool suppressBackground)
{
    _ok = false;
    _totalTime = 0;
    _suppressBackground = suppressBackground;
    _backgroundColour = *wxBLACK;
    DoCreate(filename);
}

int GIFImage::GetMSUntilNextFrame(int msec, bool loop)
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

    return -1; // we shouldn't get here
}

int GIFImage::CalcFrameForTime(int msec, bool loop)
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

	return frame-1; // we shouldn't get here
}

void GIFImage::ReadFrameProperties()
{
    wxLogNull logNo;  // suppress popups from gif images.
    _totalTime = 0;
	for (size_t i = 0; i < _gifDecoder.GetFrameCount(); ++i) {
		long frametime = _gifDecoder.GetDelay(i);
		_frameTimes.push_back(frametime);
		_totalTime += frametime;
        _frameSizes.push_back(_gifDecoder.GetFrameSize(i));
        _frameOffsets.push_back(_gifDecoder.GetFramePosition(i));
	}
    if (_totalTime == 0) {
        _frameTimes.clear();
        _totalTime = 100 * _gifDecoder.GetFrameCount();
        for (unsigned int x = 0; x < _gifDecoder.GetFrameCount(); x++) {
            _frameTimes.push_back(100);
        }
    }
}

void GIFImage::DoCreate(const std::string& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxLogNull logNo;  // suppress popups from gif images.

#ifdef DEBUG_GIF
    logger_base.debug("Loading gif %s.", (const char*)filename.c_str());
#endif
    _filename = filename;
    _ok = false;

	wxFileInputStream stream(filename);
	if (stream.IsOk()) {
		if (_gifDecoder.LoadGIF(stream) == wxGIF_OK) {
            _frameImages.resize(_gifDecoder.GetFrameCount());
            _frameDispose.resize(_gifDecoder.GetFrameCount());
            _ok = true;

            _backgroundColour = _gifDecoder.GetBackgroundColour();

            ReadFrameProperties();
            auto its = _frameSizes.begin();
            auto ito = _frameOffsets.begin();
            _gifSize = wxSize(0, 0);

            while (its != _frameSizes.end()) {
                if (its->GetWidth() + ito->x > _gifSize.GetWidth() ||
                    its->GetHeight() + ito->y > _gifSize.GetHeight()) {
                    _gifSize = wxSize((std::max)((int)_gifSize.GetWidth(), (int)(its->GetWidth() + ito->x)),
                                      (std::max)((int)_gifSize.GetHeight(), (int)(its->GetHeight() + ito->y)));
                }
                ++its;
                ++ito;
            }
#ifdef DEBUG_GIF
            logger_base.debug("    GIF size (%d,%d)", _gifSize.GetWidth(), _gifSize.GetHeight());
            logger_base.debug("    Frames %d", _gifDecoder.GetFrameCount());
            logger_base.debug("    Background colour %s", (const char*)_backgroundColour.GetAsString().c_str());
#endif
        } else {
			logger_base.warn("Error interpreting GIF file %s.", (const char *)filename.c_str());
			_gifDecoder.Destroy();
            _ok = false;
		}
    } else {
		logger_base.warn("Error opening GIF file %s.", (const char *)filename.c_str());
	}
}

const wxImage& GIFImage::GetFrameForTime(int msec, bool loop)
{
	int frame = CalcFrameForTime(msec, loop);
    if (frame == -1) {
        _invalidImage = wxImage(_gifSize);
        return _invalidImage;
    }
	return GetFrame(frame);
}

void GIFImage::CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay, bool dontaddtransparency)
{
    if (from.GetWidth() != to.GetWidth() || from.GetHeight() != to.GetHeight() || overlay) {
        int tox = (std::min)((int)from.GetWidth(), (int)(to.GetWidth() - offset.x));
        int toy = (std::min)((int)from.GetHeight(), (int)(to.GetHeight() - offset.y));

        for (int y = 0; y < toy; y++) {
            for (int x = 0; x < tox; x++) {
                if (!from.IsTransparent(x, y)) {
                    to.SetAlpha(x + offset.x, y + offset.y, 255);
                    to.SetRGB(x + offset.x, y + offset.y, from.GetRed(x, y), from.GetGreen(x, y), from.GetBlue(x, y));
                } else {
                    if (!dontaddtransparency) {
                        to.SetAlpha(x + offset.x, y + offset.y, 0);
                    }
                }
            }
        }
    } else {
        to = from;
    }
}

wxString DecodeDispose(int dispose)
{
    switch (dispose) {
    case 0:
        return "DONOTREMOVE";
    case 1: 
        return "TOBACKGROUND";
    case 2: 
        return "TOPREVIOUS";
    default: 
        return "UNSPECIFIED";
    }
}

wxPoint GIFImage::LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal)
{
    wxLogNull logNo;  // suppress popups from gif images.

	#ifdef DEBUG_GIF
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Frame %d loaded actual image size (%d,%d)", frame, image.GetWidth(), image.GetHeight());
	#endif

    wxSize size = _gifDecoder.GetFrameSize(frame);
	#ifdef DEBUG_GIF
		logger_base.debug("    frame size (%d,%d)", size.GetWidth(), size.GetHeight());
	#endif
    image.Resize(size, wxPoint(0, 0));
    _gifDecoder.ConvertToImage(frame, &image);
    disposal = _gifDecoder.GetDisposalMethod(frame);
#ifdef DEBUG_GIF
    wxColor color  = _gifDecoder.GetTransparentColour(frame);
    logger_base.debug("    transparent colour %s", (const char*)color.GetAsString().c_str());
    logger_base.debug("    disposal %d %s", disposal, (const char *)DecodeDispose(disposal).c_str());
    long frameduration = _gifDecoder.GetDelay(frame);
    logger_base.debug("    delay %ldms", frameduration);
#endif
    wxPoint offset = _gifDecoder.GetFramePosition(frame);
#ifdef DEBUG_GIF
    logger_base.debug("    frame offset (%d,%d)", offset.x, offset.y);
#endif

    return offset;
}

const wxImage& GIFImage::GetFrame(int frame)
{
#ifdef DEBUG_GIF
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#endif
    if (!_frameImages[frame].IsOk()) {
        //find last completed frame before this one
        wxAnimationDisposal lastDispose = wxAnimationDisposal::wxANIM_TOBACKGROUND;

        int startframe = frame;
        while (startframe >= 0 && !_frameImages[startframe].IsOk()) {
            --startframe;
        }
        wxImage image;
        if (startframe >= 0) {
            image = _frameImages[startframe];
            lastDispose = _frameDispose[startframe];
        } else {
            image = wxImage(_gifSize);
            image.InitAlpha();
        }
        startframe++;

        for (size_t i = startframe; i <= frame; i++) {
            wxImage newframe(image.GetWidth(), image.GetHeight());
            wxAnimationDisposal dispose = wxANIM_TOBACKGROUND;
            wxPoint offset = LoadRawImageFrame(newframe, i, dispose);
#ifdef DEBUG_GIF
            logger_base.debug("    Frame %d loaded offset (%d,%d) frame size (%d,%d) dispose %d %s actual image size (%d,%d)", i, offset.x, offset.y, newframe.GetWidth(), newframe.GetHeight(), dispose, (const char *)DecodeDispose(dispose).c_str(), image.GetWidth(), image.GetHeight());
            logger_base.debug("    Applying dispose from last frame %s", (const char *)DecodeDispose(_lastDispose).c_str());
#endif
            
            if (_suppressBackground  && (i == 0 || lastDispose == wxANIM_TOBACKGROUND)) {
#ifdef DEBUG_GIF
                logger_base.debug("    Replacing gif image this frame");
#endif
                image.UnShare(); // wxWidgets bug https://github.com/wxWidgets/wxWidgets/issues/23553
                image.Clear();
                CopyImageToImage(image, newframe, offset, true);
            } else if (i == 0 || lastDispose == wxANIM_TOBACKGROUND) {
#ifdef DEBUG_GIF
                logger_base.debug("    Replacing gif image this after drawing background colour");
#endif
                unsigned char red = _backgroundColour.Red();
                unsigned char green = _backgroundColour.Green();
                unsigned char blue = _backgroundColour.Blue();
                for (int y = 0; y < image.GetHeight(); y++) {
                    for (int x = 0; x < image.GetWidth(); x++) {
                        image.SetRGB(x, y, red, green, blue);
                        image.SetAlpha(x, y, 255);
                    }
                }
                CopyImageToImage(image, newframe, offset, true);
            } else if (lastDispose == wxANIM_DONOTREMOVE) {
#ifdef DEBUG_GIF
                logger_base.debug("    Updating gif image this frame");
#endif
                CopyImageToImage(image, newframe, offset, true, true);
            } else {
#ifdef DEBUG_GIF
                logger_base.debug("    Updating gif image this frame");
#endif
                CopyImageToImage(image, newframe, offset, true, true);
            }
            _frameImages[i] = image;
            _frameDispose[i] = dispose;
        }
    }
    return _frameImages[frame];
}
