#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <string>
#include <wx/wx.h>

class wxXmlNode;
class wxNotebook;

class PlayListItem
{
protected:

    #pragma region Member Variables
    bool _dirty;
    std::string _name;
    long _frames;
    int _msPerFrame;
    long _delay;
    #pragma endregion Member Variables

    void Save(wxXmlNode* node);

public:

    #pragma region Constructors and Destructors
    PlayListItem(wxXmlNode* node);
    PlayListItem();
    virtual ~PlayListItem() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }
    virtual std::string GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    virtual long GetLength() { return _frames; }
    long GetDelay() const { return _delay; }
    void SetDelay(long delay) { _delay = delay; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() = 0;
    virtual void Load(wxXmlNode* node);

    #pragma region Playing
    virtual void Play() = 0;
    virtual void PlayFrame(long frame) = 0;
    virtual wxByte* GetFrameData(long frame) = 0;
    virtual void Stop() = 0;
    #pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) = 0;
    #pragma endregion UI
};
#endif