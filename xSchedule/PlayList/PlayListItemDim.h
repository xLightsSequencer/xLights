#ifndef PLAYLISTITEMDIM_H
#define PLAYLISTITEMDIM_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class OutputManager;

class PlayListItemDim : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _name;
    int _dim;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _channels;
    size_t _duration;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemDim(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemDim(OutputManager* outputManager);
    virtual ~PlayListItemDim();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _duration; }
    void SetDuration(size_t duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    void SetDim(const int dim) { if (_dim != dim) { _dim = dim; _changeCount++; } };
    void SetStartChannel(const std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0; _changeCount++; } };
    void SetChannels(const size_t channels) { if (_channels != channels) { _channels = channels; _changeCount++; } };

    int GetDim() const { return _dim; }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    size_t GetChannels() const { return _channels; }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start(long stepLengthMS) override;
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif