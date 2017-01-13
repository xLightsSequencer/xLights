#ifndef PLAYLISTITEMIMAGE_H
#define PLAYLISTITEMIMAGE_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class PlayerWindow;

class PlayListItemImage : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _ImageFile;
	wxPoint _origin;
	wxSize _size;
    wxImage _image;
    PlayerWindow* _window;
    bool _done;
    long _duration;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemImage(wxXmlNode* node);
    PlayListItemImage();
    virtual ~PlayListItemImage();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetNameNoTime() const override;
    void SetLocation(wxPoint pt, wxSize size) { _origin = pt; _size = size; _changeCount++; }
    void SetImageFile(const std::string& ImageFile) { _ImageFile = ImageFile; _changeCount++; }
    std::string GetImageFile() const { return _ImageFile; }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { _duration = duration; _changeCount++; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) override;
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif