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

#include "PlayListItem.h"

class wxXmlNode;
class wxWindow;
class OutputManager;

typedef enum
{
	FADE_IN,
	FADE_OUT
} FADEDIRECTION;

class PlayListItemFade : public PlayListItem
{
protected:

#pragma region Member Variables
    long _duration;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _channels;
	FADEDIRECTION _fadeDirection;
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    PlayListItemFade(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemFade(OutputManager* outputManager);
    virtual ~PlayListItemFade() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { if (_restOfStep) return _stepLengthMS - _delay; else return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    void SetStartChannel(std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0;  _changeCount++; } }
    long GetChannels() const { return _channels; }
    void SetChannels(long channels) { if (_channels != channels) { _channels = channels; _changeCount++; } }
    FADEDIRECTION GetFadeDirection() const { return _fadeDirection; }
    void SetFadeDirection(FADEDIRECTION fadeDirection) { if (_fadeDirection != fadeDirection) { _fadeDirection = fadeDirection; _changeCount++; } }
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

