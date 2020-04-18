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
#include <string>
#include "../Blend.h"

class wxXmlNode;
class OutputManager;

class PlayListItemColourOrgan : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _mode;
    wxColor _colour;
    size_t _sc;
    std::string _startChannel;
    OutputManager* _outputManager;
    size_t _pixels;
    size_t _duration;
    APPLYMETHOD _blendMode;
    int _startNote = 1;
    int _endNote = 127;
    int _fadeFrames = 0;
    float _lastValue = 0;
    float _fadePerFrame = 1.0;
    int _threshold = 80;
    #pragma endregion Member Variables

    void SetPixel(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, APPLYMETHOD blendMode);

public:

    #pragma region Constructors and Destructors
    PlayListItemColourOrgan(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemColourOrgan(OutputManager* outputManager);
    virtual ~PlayListItemColourOrgan();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _duration; }
    void SetDuration(size_t duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    void SetColour(wxColor colour) { if (_colour != colour) { _colour = colour; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    void SetMode(const std::string& mode) { if (_mode != mode) { _mode = mode; _changeCount++; } };
    void SetStartChannel(const std::string startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _sc = 0; _changeCount++; } };
    void SetPixels(const size_t pixels) { if (_pixels != pixels) { _pixels = pixels; _changeCount++; } };
    void SetBlendMode(const std::string& blendMode) { if (_blendMode != EncodeBlendMode(blendMode)) { _blendMode = EncodeBlendMode(blendMode); _changeCount++; } }
    int GetBlendMode() const { return _blendMode; }
    wxColour GetColour() const { return _colour; }
    std::string GetStartChannel() const { return _startChannel; }
    size_t GetStartChannelAsNumber();
    size_t GetPixels() const { return _pixels; }
    std::string GetMode() const { return _mode; }
    int GetStartNote() const { return _startNote; }
    void SetStartNote(int startNote) { if (_startNote != startNote) { _startNote = startNote; _changeCount++; } };
    int GetEndNote() const { return _endNote; }
    void SetEndNote(int endNote) { if (_endNote != endNote) { _endNote = endNote; _changeCount++; } };
    int GetFadeFrames() const { return _fadeFrames; }
    void SetFadeFrames(int fadeFrames) { if (_fadeFrames != fadeFrames) { _fadeFrames = fadeFrames; _changeCount++; } };
    int GetThreshold() const { return _threshold; }
    void SetThreshold(int threshold) { if (_threshold != threshold) { _threshold = threshold; _changeCount++; } };
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
