#ifndef GIFIMAGE_H
#define GIFIMAGE_H

#include <string>
#include <list>
#include <wx/gifdecod.h>
#include <wx/wfstream.h>
#include <wx/image.h>

class GIFImage
{
	wxSize _desiredSize;
	wxGIFDecoder _gifDecoder;
	wxImage _lastImage;
	std::list<long> _frameTimes();
	long _totalTime;
	
	void ReadFrameTimes();
	int CalcFrameForTime(int msec, bool loop)
	
	public:
		GIFImage(const std::string& filename);
		GIFImage(const std::string& filename, wxSize desiredSize);
		virtual ~GIFImage();
		wxImage GetFrame(int frame);
		wxImage GetFrameForTime(int msec, bool loop);
		
		static bool IsGIF(const std::string& filename);
}

#endif
