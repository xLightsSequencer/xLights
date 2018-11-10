#ifndef PLAYLISTITEMALLOFF_H
#define PLAYLISTITEMALLOFF_H

#include "PlayListItem.h"
#include "../Blend.h"

class wxXmlNode;
class wxWindow;
class OutputManager;

class PlayListItemAllOff : public PlayListItem
{
protected:

#pragma region Member Variables
    long _duration;
    uint8_t _value;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _channels;
    APPLYMETHOD _applyMethod;
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    PlayListItemAllOff(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemAllOff(OutputManager* outputManager);
    virtual ~PlayListItemAllOff() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    void SetStartChannel(std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0;  _changeCount++; } }
    long GetChannels() const { return _channels; }
    void SetChannels(long channels) { if (_channels != channels) { _channels = channels; _changeCount++; } }
    uint8_t GetValue() const { return _value; }
    void SetValue(uint8_t value) { if (_value != value) { _value = value; _changeCount++; } }
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(const std::string& blendMode) { if (_applyMethod != EncodeBlendMode(blendMode)) { _applyMethod = EncodeBlendMode(blendMode); _changeCount++; } }
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
