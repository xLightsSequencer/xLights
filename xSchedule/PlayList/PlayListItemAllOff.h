#ifndef PLAYLISTITEMALLOFF_H
#define PLAYLISTITEMALLOFF_H

#include "PlayListItem.h"

class wxXmlNode;
class wxWindow;

class PlayListItemAllOff : public PlayListItem
{
protected:

#pragma region Member Variables
    long _duration;
    wxByte _value;
    int _blendMode;
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    PlayListItemAllOff(wxXmlNode* node);
    PlayListItemAllOff();
    virtual ~PlayListItemAllOff() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { _duration = duration; _dirty = true; }
    wxByte GetValue() const { return _value; }
    void SetValue(wxByte value) { _value = value; _dirty = true; }
    int GetBlendMode() const { return _blendMode; }
    void SetBlendMode(int blendMode) { _blendMode = blendMode; _dirty = true; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

#pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) override;
#pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
    #pragma endregion UI
};
#endif