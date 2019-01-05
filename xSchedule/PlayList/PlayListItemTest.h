#ifndef PLAYLISTITEMTEST_H
#define PLAYLISTITEMTEST_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class OutputManager;

class PlayListItemTest : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _name;
    std::string _mode;
    uint8_t _value1;
    uint8_t _value2;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _channels;
    size_t _duration;
    int _state;
    size_t _frameDuration;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemTest(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemTest(OutputManager* outputManager);
    virtual ~PlayListItemTest();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _duration; }
    void SetDuration(size_t duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    void SetFrameDuration(size_t duration) { if (_frameDuration != duration) { _frameDuration = duration; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    void SetMode(const std::string& mode) { if (_mode != mode) { _mode = mode; _changeCount++; } };
    void SetValue1(const size_t value1) { if (_value1 != value1) { _value1 = value1; _changeCount++; } };
    void SetValue2(const size_t value2) { if (_value2 != value2) { _value2 = value2; _changeCount++; } };
    void SetStartChannel(const std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0; _changeCount++; } };
    void SetChannels(const size_t channels) { if (_channels != channels) { _channels = channels; _changeCount++; } };

    size_t GetFrameDuration() const { return _frameDuration; }
    size_t GetValue1() const { return _value1; }
    size_t GetValue2() const { return _value2; }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    size_t GetChannels() const { return _channels; }
    std::string GetMode() const { return _mode; }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start(long stepLengthMS) override;
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif