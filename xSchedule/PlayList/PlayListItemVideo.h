#ifndef PLAYLISTITEMVIDEO_H
#define PLAYLISTITEMVIDEO_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemVideo : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _videoFile;
	wxPoint _origin;
	wxSize _size;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemVideo(wxXmlNode* node);
    PlayListItemVideo();
    virtual ~PlayListItemVideo() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetName() const override;
    void SetLocation(wxPoint pt, wxSize size) { _origin = pt; _size = size; }
    void SetVideoFile(const std::string& videoFile) { _videoFile = videoFile; }
    std::string GetVideoFile() const { return _videoFile; }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Play() override {};
    virtual void Stop() override {};
    virtual void PlayFrame(long frame) override {};
    virtual wxByte* GetFrameData(long frame) override { return nullptr; }
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif