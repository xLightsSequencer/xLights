#ifndef PLAYLISTITEMDELAY_H
#define PLAYLISTITEMDELAY_H

#include "PlayListItem.h"

class wxXmlNode;
class wxWindow;

class PlayListItemDelay : public PlayListItem
{
protected:

    #pragma region Member Variables
    long _duration;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemDelay(wxXmlNode* node);
    PlayListItemDelay();
    virtual ~PlayListItemDelay() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    virtual std::string GetTitle() const override;
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

#pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
#pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif