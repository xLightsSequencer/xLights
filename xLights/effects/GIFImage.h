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
	std::list<long> _frameTimes;
	long _totalTime;
    int _lastFrame;
    std::string _filename;
    wxAnimationDisposal _lastDispose;
    bool _ok;
	
	void ReadFrameTimes();
    int CalcFrameForTime(int msec, bool loop);
    wxPoint LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal);
    void CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay);

	public:
		GIFImage(const std::string& filename);
		GIFImage(const std::string& filename, wxSize desiredSize);
		virtual ~GIFImage();
		wxImage GetFrame(int frame);
		wxImage GetFrameForTime(int msec, bool loop);
        int GetMSUntilNextFrame(int msec, bool loop);
        std::string GetFilename() const { return _filename; }
        bool IsOk() const { return _ok; }

		static bool IsGIF(const std::string& filename);
};

#endif
