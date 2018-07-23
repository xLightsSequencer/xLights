#ifndef GIFIMAGE_H
#define GIFIMAGE_H

#include <string>
#include <list>
#include <wx/gifdecod.h>
#include <wx/wfstream.h>
#include <wx/image.h>

class GIFImage
{
	wxGIFDecoder _gifDecoder;
	wxImage _lastImage;
	std::list<long> _frameTimes;
    std::list<wxSize> _frameSizes;
    std::list<wxPoint> _frameOffsets;
    wxColour _backgroundColour;
    wxSize _gifSize;
	long _totalTime;
    int _lastFrame;
    bool _suppressBackground;
    std::string _filename;
    wxAnimationDisposal _lastDispose;
    bool _ok;
	
	void ReadFrameProperties();
    int CalcFrameForTime(int msec, bool loop);
    wxPoint LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal);
    void CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay);
    void DoCreate(const std::string& filename);

	public:
		GIFImage(const std::string& filename, bool suppressBackground = true);
		virtual ~GIFImage();
		wxImage GetFrame(int frame);
		wxImage GetFrameForTime(int msec, bool loop);
        int GetMSUntilNextFrame(int msec, bool loop);
        std::string GetFilename() const { return _filename; }
        bool IsOk() const { return _ok; }

		static bool IsGIF(const std::string& filename);
};

#endif
