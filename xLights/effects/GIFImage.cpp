#include "GIFImage.h"
#include <wx/filename.h>
#include <log4cpp/Category.hh>

//#define DEBUG_GIF

bool GIFImage::IsGIF(const std::string& filename)
{
	wxFileName fn(filename);
	return (fn.GetExt().Lower() == "gif");
}

GIFImage::~GIFImage()
{
	_gifDecoder.Destroy();
}

GIFImage::GIFImage(const std::string& filename)
{
    _ok = false;
    _totalTime = 0;
    _lastFrame = -1;
    _lastDispose = wxAnimationDisposal::wxANIM_UNSPECIFIED;
    DoCreate(filename, wxSize(-1, -1));
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

void GIFImage::ReadFrameTimes()
{
	_totalTime = 0;
	for (size_t i = 0; i < _gifDecoder.GetFrameCount(); ++i)
	{
		long frametime = _gifDecoder.GetDelay(i);
		_frameTimes.push_back(frametime);
		_totalTime += frametime;
	}
    if (_totalTime == 0) {
        _frameTimes.clear();
        _totalTime = 100 * _gifDecoder.GetFrameCount();
        for (int x = 0; x < _gifDecoder.GetFrameCount(); x++) {
            _frameTimes.push_back(100);
        }
    }
}

GIFImage::GIFImage(const std::string& filename, wxSize desiredSize)
{
    DoCreate(filename, desiredSize);
}

void GIFImage::DoCreate(const std::string& filename, wxSize desiredSize)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _filename = filename;
    _desiredSize = desiredSize;
    _lastFrame = -1;
    _lastDispose = wxAnimationDisposal::wxANIM_UNSPECIFIED;
    _ok = false;

	wxFileInputStream stream(filename);
	if (stream.IsOk())
	{
		if (_gifDecoder.LoadGIF(stream) == wxGIF_OK)
		{
            _ok = true;

			// loaded successfully
			if (_desiredSize.x == -1)
			{
				_desiredSize = _gifDecoder.GetFrameSize(0); // how do i know this is right?
			}
            ReadFrameTimes();
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
		return wxImage(_desiredSize);
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

        for (size_t y = 0; y < toy; y++)
        {
            for (size_t x = 0; x < tox; x++)
            {
                if (!from.IsTransparent(x, y))
                {
                    to.SetRGB(x + offset.x, y + offset.y, from.GetRed(x, y), from.GetGreen(x, y), from.GetBlue(x, y));
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
	#ifdef DEBUG_GIF
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Frame %d loaded actual image size (%d,%d)", frame, image.GetWidth(), image.GetHeight());
	#endif

    wxSize size = _gifDecoder.GetFrameSize(frame);
	#ifdef DEBUG_GIF
		logger_base.debug("    size (%d,%d)", size.GetWidth(), size.GetHeight());
	#endif
    image.Resize(size, wxPoint(0, 0));
    _gifDecoder.ConvertToImage(frame, &image);
    disposal = _gifDecoder.GetDisposalMethod(frame);
#ifdef DEBUG_GIF
    logger_base.debug("    disposal %d", disposal);
    long frameduration = _gifDecoder.GetDelay(frame);
    logger_base.debug("    delay %ldms", frameduration);
#endif
    wxPoint offset = _gifDecoder.GetFramePosition(frame);
#ifdef DEBUG_GIF
    logger_base.debug("    offset (%d,%d)", offset.x, offset.y);
#endif

    // handle first frame with an offset
    if (frame == 0 && (offset.x > 0 || offset.y > 0))
    {
        image.Resize(wxSize(size.GetWidth() + offset.x, size.GetHeight() + offset.y), offset);
        offset.x = 0;
        offset.y = 0;
        #ifdef DEBUG_GIF
        logger_base.debug("    Frame 0 had non zero offset so image size now (%d,%d)", image.GetWidth(), image.GetHeight());
        #endif
    }

    return offset;
}

wxImage GIFImage::GetFrame(int frame)
{
    wxImage image(_desiredSize);

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
        logger_base.debug("Frame %d loaded offset (%d,%d) size (%d,%d) dispose %d actual image size (%d,%d)", i, offset.x, offset.y, newframe.GetWidth(), newframe.GetHeight(), dispose, image.GetWidth(), image.GetHeight());
#endif

        if (i == 0 || _lastDispose == wxANIM_TOBACKGROUND || _lastDispose == wxANIM_UNSPECIFIED)
        {
            image.Clear();
            CopyImageToImage(image, newframe, offset, true);
        }
        else
        {
            CopyImageToImage(image, newframe, offset, true);
        }

        _lastImage = image;
        _lastDispose = dispose;
    }

    _lastFrame = frame;

    return image;
}
