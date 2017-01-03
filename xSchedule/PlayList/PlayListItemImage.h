#ifndef PLAYLISTITEMIMAGE_H
#define PLAYLISTITEMIMAGE_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemImage : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _ImageFile;
	wxPoint _origin;
	wxSize _size;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemImage(wxXmlNode* node);
    PlayListItemImage();
    virtual ~PlayListItemImage() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetName() const override;
    void SetLocation(wxPoint pt, wxSize size) { _origin = pt; _size = size; }
    void SetImageFile(const std::string& ImageFile) { _ImageFile = ImageFile; }
    std::string GetImageFile() const { return _ImageFile; }
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