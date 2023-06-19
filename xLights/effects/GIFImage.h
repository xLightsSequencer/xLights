#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <list>
#include <wx/gifdecod.h>
#include <wx/wfstream.h>
#include <wx/image.h>

class GIFImage
{
	wxGIFDecoder _gifDecoder;
	wxImage _invalidImage;
	std::list<long> _frameTimes;
    std::list<wxSize> _frameSizes;
    std::list<wxPoint> _frameOffsets;
    std::vector<wxImage> _frameImages;
    std::vector<wxAnimationDisposal> _frameDispose;
    wxColour _backgroundColour;
    wxSize _gifSize;
	long _totalTime = 0;
    int _lastFrame = 0;
    bool _suppressBackground = false;
    std::string _filename;
    bool _ok = false;
	
	void ReadFrameProperties();
    int CalcFrameForTime(int msec, bool loop);
    wxPoint LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal);
    void CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay, bool dontaddtransparency = false);
    void DoCreate(const std::string& filename);

	public:
		GIFImage(const std::string& filename, bool suppressBackground = true);
		virtual ~GIFImage();
		const wxImage &GetFrame(int frame);
		const wxImage &GetFrameForTime(int msec, bool loop);
        int GetMSUntilNextFrame(int msec, bool loop);
        std::string GetFilename() const { return _filename; }
        bool IsOk() const { return _ok; }

		static bool IsGIF(const std::string& filename);
};
