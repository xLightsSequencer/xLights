#include "GIFImage.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>

#undef max

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
    _lastFrame = -1;
    _suppressBackground = suppressBackground;
    _lastDispose = wxAnimationDisposal::wxANIM_TOBACKGROUND;
    _backgroundColour = *wxBLACK;
    DoCreate(filename);
}

int GIFImage::GetMSUntilNextFrame(int msec, bool loop)
{
    if (loop)
    {
        while (msec >= _totalTime)
        {
            msec -= _totalTime;
        }
    }

    if (msec > _totalTime) return -1;

    for (auto it = _frameTimes.begin(); it != _frameTimes.end(); ++it)
    {
        if (msec < *it)
        {
            return *it - msec;
        }
        msec -= *it;
    }

    return -1; // we shouldn't get here
}

int GIFImage::CalcFrameForTime(int msec, bool loop)
{
	if (loop)
	{
		while (msec >= _totalTime)
		{
			msec -= _totalTime;
		}
	}

	if (msec > _totalTime) return -1;

	int frame = 0;
	for (auto it = _frameTimes.begin(); it != _frameTimes.end(); ++it)
	{
		if (msec < *it)
		{
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
	for (size_t i = 0; i < _gifDecoder.GetFrameCount(); ++i)
	{
		long frametime = _gifDecoder.GetDelay(i);
		_frameTimes.push_back(frametime);
		_totalTime += frametime;
        _frameSizes.push_back(_gifDecoder.GetFrameSize(i));
        _frameOffsets.push_back(_gifDecoder.GetFramePosition(i));
	}
    if (_totalTime == 0) {
        _frameTimes.clear();
        _totalTime = 100 * _gifDecoder.GetFrameCount();
        for (int x = 0; x < _gifDecoder.GetFrameCount(); x++) {
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
    _lastFrame = -1;
    _lastDispose = wxAnimationDisposal::wxANIM_UNSPECIFIED;
    _ok = false;

	wxFileInputStream stream(filename);
	if (stream.IsOk())
	{
		if (_gifDecoder.LoadGIF(stream) == wxGIF_OK)
		{
            _ok = true;

            _backgroundColour = _gifDecoder.GetBackgroundColour();

            ReadFrameProperties();
            auto its = _frameSizes.begin();
            auto ito = _frameOffsets.begin();
            _gifSize = wxSize(0, 0);

            while (its != _frameSizes.end())
            {
                if (its->GetWidth() + ito->x > _gifSize.GetWidth() ||
                    its->GetHeight() + ito->y > _gifSize.GetHeight())
                {
                    _gifSize = wxSize(std::max(_gifSize.GetWidth(), its->GetWidth() + ito->x),
                                      std::max(_gifSize.GetHeight(), its->GetHeight() + ito->y));
                }
                ++its;
                ++ito;
            }
#ifdef DEBUG_GIF
            logger_base.debug("    GIF size (%d,%d)", _gifSize.GetWidth(), _gifSize.GetHeight());
#endif
        }
		else
		{
			logger_base.warn("Error interpreting GIF file %s.", (const char *)filename.c_str());
			_gifDecoder.Destroy();
		}
	}
	else
	{
		logger_base.warn("Error opening GIF file %s.", (const char *)filename.c_str());
	}
}

wxImage GIFImage::GetFrameForTime(int msec, bool loop)
{
	int frame = CalcFrameForTime(msec, loop);

	if (frame == -1)
	{
		return wxImage(_gifSize);
	}

	return GetFrame(frame);
}

void GIFImage::CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay)
{
    if (from.GetWidth() != to.GetWidth() || from.GetHeight() != to.GetHeight() || overlay)
    {
        #ifdef _MSC_VER
        int tox = min(from.GetWidth(), to.GetWidth() - offset.x);
        int toy = min(from.GetHeight(), to.GetHeight() - offset.y);
        #else
        int tox = std::min(from.GetWidth(), to.GetWidth() - offset.x);
        int toy = std::min(from.GetHeight(), to.GetHeight() - offset.y);
        #endif

        for (int y = 0; y < toy; y++)
        {
            for (int x = 0; x < tox; x++)
            {
                if (!from.IsTransparent(x, y))
                {
                    to.SetAlpha(x + offset.x, y + offset.y, 255);
                    to.SetRGB(x + offset.x, y + offset.y, from.GetRed(x, y), from.GetGreen(x, y), from.GetBlue(x, y));
                }
                else
                {
                    to.SetAlpha(x + offset.x, y + offset.y, 0);
                }
            }
        }
    }
    else
    {
        to = from;
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
    logger_base.debug("    disposal %d %s", disposal, (disposal == 0 ? "DONOTREMOVE" : (disposal == 1 ? "TOBACKGROUND" : (disposal == 2 ? "TOPREVIOUS" : "UNSPECIFIED"))));
    long frameduration = _gifDecoder.GetDelay(frame);
    logger_base.debug("    delay %ldms", frameduration);
#endif
    wxPoint offset = _gifDecoder.GetFramePosition(frame);
#ifdef DEBUG_GIF
    logger_base.debug("    frame offset (%d,%d)", offset.x, offset.y);
#endif

    return offset;
}

wxImage GIFImage::GetFrame(int frame)
{
    wxImage image(_gifSize);
    image.InitAlpha();

    int startframe = 0;
    if (_lastFrame < 0)
    {
        // we start at 0
    }
    else if (_lastFrame == frame)
    {
        return _lastImage;
    }
    else if (_lastFrame < frame)
    {
        if (_lastImage.IsOk())
        {
            image = _lastImage;
        }
        else
        {
            wxASSERT(false);
        }
        startframe = _lastFrame + 1;
    }
    else
    {
        //lastframe is greater than the frame we want so we have to start from zero again
    }

    for (size_t i = startframe; i <= frame; i++)
    {
        wxImage newframe(image.GetWidth(), image.GetHeight());
        wxAnimationDisposal dispose = wxANIM_TOBACKGROUND;
        wxPoint offset = LoadRawImageFrame(newframe, i, dispose);

#ifdef DEBUG_GIF
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Frame %d loaded offset (%d,%d) frame size (%d,%d) dispose %d actual image size (%d,%d)", i, offset.x, offset.y, newframe.GetWidth(), newframe.GetHeight(), dispose, image.GetWidth(), image.GetHeight());
#endif

        if (_suppressBackground  && (i == 0 || _lastDispose == wxANIM_TOBACKGROUND || _lastDispose == wxANIM_UNSPECIFIED))
        {
#ifdef DEBUG_GIF
            logger_base.debug("    Replacing gif image this frame : Applying dispose %d", _lastDispose);
#endif
            image.Clear();
            CopyImageToImage(image, newframe, offset, true);
        }
        else if (i == 0 || _lastDispose == wxANIM_TOBACKGROUND || _lastDispose == wxANIM_UNSPECIFIED)
        {
#ifdef DEBUG_GIF
            logger_base.debug("    Replacing gif image this after drawing background colour : Applying dispose %d", _lastDispose);
#endif
            unsigned char red = _backgroundColour.Red();
            unsigned char green = _backgroundColour.Green();
            unsigned char blue = _backgroundColour.Blue();
            for (int y = 0; y < image.GetHeight(); y++)
            {
                for (int x = 0; x < image.GetWidth(); x++)
                {
                    image.SetRGB(x, y, red, green, blue);
                }
            }
            CopyImageToImage(image, newframe, offset, true);
        }
        else
        {
#ifdef DEBUG_GIF
            logger_base.debug("    Updating gif image this frame : Applying dispose %d", _lastDispose);
#endif
            CopyImageToImage(image, newframe, offset, true);
        }

        _lastImage = image;
        _lastDispose = dispose;
    }

    _lastFrame = frame;

    return image;
}
