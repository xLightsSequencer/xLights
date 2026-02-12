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

#include <string>
#include <list>
#include <wx/gifdecod.h>
#include <wx/wfstream.h>
#include <wx/image.h>
#include "../Color.h"

class GIFImage
{
	wxGIFDecoder _gifDecoder;
	wxImage _invalidImage;
	std::vector<long> _frameTimes;
    std::vector<wxSize> _frameSizes;
    std::vector<wxPoint> _frameOffsets;
    std::vector<wxImage> _frameImages;
    std::vector<wxAnimationDisposal> _frameDispose;
    xlColor _backgroundColor;
    wxSize _gifSize;
	long _totalTime = 0;
    int _lastFrame = 0;
    bool _suppressBackground = false;
    std::string _filename;
    bool _ok = false;
    int numFrames = 0;
	
	void ReadFrameProperties();
    int CalcFrameForTime(int msec, bool loop);
    wxPoint LoadRawImageFrame(wxImage& image, int frame, wxAnimationDisposal& disposal);
    void CopyImageToImage(wxImage& to, wxImage& from, wxPoint offset, bool overlay, bool dontaddtransparency = false);
    void DoCreate(const std::string& filename);
    void DoCreate(wxInputStream &inStream, const std::string &fname);

public:
    GIFImage(const std::string& filename, bool suppressBackground = true);
    GIFImage(const std::string& name, wxInputStream& ins, bool suppressBackground = true);
    virtual ~GIFImage();
    const wxImage &GetFrame(int frame);
    const wxImage &GetFrameForTime(int msec, bool loop);
    int GetMSUntilNextFrame(int msec, bool loop);
    std::string GetFilename() const { return _filename; }
    bool IsOk() const { return _ok; }

    int GetNumFrames() const { return _frameImages.size(); }
    int GetFrameTime(int x) const { return _frameTimes[x]; }
    int GetTotalTime() const { return _totalTime; }
    wxSize GetImageSize() const { return _gifSize; }
    
    void ResetSuppressBackground(bool suppressBackground);
    
    static bool IsGIF(const std::string& filename);
};
