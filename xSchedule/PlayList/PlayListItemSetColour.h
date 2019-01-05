#ifndef PLAYLISTITEMSETCOLOUR_H
#define PLAYLISTITEMSETCOLOUR_H

#include "PlayListItem.h"
#include "../Blend.h"

class wxXmlNode;
class wxWindow;
class OutputManager;

class PlayListItemSetColour : public PlayListItem
{
protected:

#pragma region Member Variables
    long _duration;
    wxColor _value;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _nodes;
    APPLYMETHOD _applyMethod;
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    PlayListItemSetColour(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemSetColour(OutputManager* outputManager);
    virtual ~PlayListItemSetColour() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    void SetStartChannel(std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0;  _changeCount++; } }
    long GetNodes() const { return _nodes; }
    void SetNodes(long nodes) { if (_nodes != nodes) { _nodes = nodes; _changeCount++; } }
    wxColor GetValue() const { return _value; }
    void SetValue(wxColor value) { if (_value != value) { _value = value; _changeCount++; } }
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
