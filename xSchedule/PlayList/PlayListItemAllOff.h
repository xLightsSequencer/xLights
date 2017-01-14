#ifndef PLAYLISTITEMALLOFF_H
#define PLAYLISTITEMALLOFF_H

#include "PlayListItem.h"
#include "../FSEQFile.h"

class wxXmlNode;
class wxWindow;

class PlayListItemAllOff : public PlayListItem
{
protected:

#pragma region Member Variables
    long _duration;
    wxByte _value;
    APPLYMETHOD _applyMethod;
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
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    wxByte GetValue() const { return _value; }
    void SetValue(wxByte value) { if (_value != value) { _value = value; _changeCount++; } }
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(int blendMode) { if (_applyMethod != (APPLYMETHOD)blendMode) { _applyMethod = (APPLYMETHOD)blendMode; _changeCount++; } }
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