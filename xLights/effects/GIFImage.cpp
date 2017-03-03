#include "GIFImage.h"

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
	GIFImage(filename, wxSize(-1, -1));
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

	if (msec = _totalTime) return -1;

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
	
	return -1; // we shouldn't get here
}

void GIFImage::ReadFrameTimes()
{
	_totalTime = 0;
	for (int i = 0; i < _gifDecoder.GetFrameCount(); ++i)
	{
		long frametime = _gifDecoder.GetDelay(i);
		_frameTimes.push_back(frametime);
		_totalTime += frametime;
	}
}

GIFImage::GIFImage(const std::string& filename, wxSize desiredSize)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	_desiredSize = desiredSize;
	
	wxFileInputStream stream(filename);
	if (stream.IsOk())
	{
		if (_gifDecoder.LoadGIF(stream) == wxGIF_OK)
		{
			// loaded successfully
			if (_desiredSize.x == -1)
			{
				_desiredSize = _gifDecoder.GetFrameSize(0); // how do i know this is right?
				ReadFrameTimes();
			}
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

void CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay)
{
    if (from.GetWidth() != to.GetWidth() || from.GetHeight() != to.GetHeight() || overlay)
    {
        int tox = std::min(from.GetWidth(), to.GetWidth() - offset.x);
        int toy = std::min(from.GetHeight(), to.GetHeight() - offset.y);

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

wxPoint LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal)
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

void LoadImageFrame(wxGIFDecoder& GIFdecoder, wxImage& image, int frame, int& lastframe, wxImage& lastimage, wxAnimationDisposal& lastdispose)
{
    int startframe = 0;
    if (lastframe < 0)
    {
        // we start at 0
    }
    else if (lastframe == frame)
    {
        image = lastimage;
        return;
    }
    else if (lastframe < frame)
    {
        image = lastimage;
        startframe = lastframe+1;
    }
    else
    {
        //lastframe is greater than the frame we want so we have to start from zero again
    }

    for (size_t i = startframe; i <= frame; i++)
    {
        wxImage newframe(image.GetWidth(), image.GetHeight());
        wxAnimationDisposal dispose = wxANIM_TOBACKGROUND;
        wxPoint offset = LoadRawImageFrame(GIFdecoder, newframe, i, dispose);
        
#ifdef DEBUG_GIF
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Frame %d loaded offset (%d,%d) size (%d,%d) dispose %d actual image size (%d,%d)", i, offset.x, offset.y, newframe.GetWidth(), newframe.GetHeight(), dispose, image.GetWidth(), image.GetHeight());
#endif

        if (i == 0 || lastdispose == wxANIM_TOBACKGROUND || lastdispose == wxANIM_UNSPECIFIED)
        {
            image.Clear();
            CopyImageToImage(image, newframe, offset, true);
        }
        else
        {
            CopyImageToImage(image, newframe, offset, true);
        }

        if (dispose == wxANIM_DONOTREMOVE)
        {
            lastimage = image;
        }

        lastdispose = dispose;
    }    

    lastframe = frame;

    return;
}

wxImage GIFImage::GetFrame(int frame)
{
#pragma todo
}
