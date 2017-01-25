#ifndef PLAYLISTITEMVIRTUALMATRIX_H
#define PLAYLISTITEMVIRTUALMATRIX_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class PlayerWindow;

class PlayListItemVirtualMatrix : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _matrixWidth;
    int _matrixHeight;
    size_t _startChannel;
	wxPoint _origin;
	wxSize _size;
    wxImage* _image;
    PlayerWindow* _window;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemVirtualMatrix(wxXmlNode* node);
    PlayListItemVirtualMatrix();
    virtual ~PlayListItemVirtualMatrix();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual std::string GetNameNoTime() const override;
    void SetLocation(wxPoint pt, wxSize size) { if (_origin != pt || _size != size) { _origin = pt; _size = size; _changeCount++; } }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    virtual std::string GetTitle() const override;
    int GetWidth() const { return _matrixWidth; }
    std::string GetRawName() const { return _name; }
    int GetHeight() const { return _matrixHeight; }
    size_t GetStartChannel() const { return _startChannel; }
    void SetWidth(int width) { if (_matrixWidth != width) { _matrixWidth = width; _changeCount++; } }
    void SetHeight(int height) { if (_matrixHeight != height) { _matrixHeight = height; _changeCount++; } }
    void SetStartChannel(size_t startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _changeCount++; } }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif