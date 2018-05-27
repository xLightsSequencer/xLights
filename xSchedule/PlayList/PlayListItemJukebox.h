#ifndef PLAYLISTITEMJUKEBOX_H
#define PLAYLISTITEMJUKEBOX_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

typedef enum
{
    ENUM_STATE_BEGIN,
    ENUM_STATE_XLIGHTS,
    ENUM_STATE_NOTEBOOK,
    ENUM_STATE_SEQUENCERTAB,
    ENUM_STATE_JUKEBOX,
    ENUM_STATE_DONE
}
ENUMJUKEBOX;

class PlayListItemJukebox : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _jukeboxButton;
    std::string _sequence;
    bool _started;
    ENUMJUKEBOX _state;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemJukebox(wxXmlNode* node);
    PlayListItemJukebox();
    virtual ~PlayListItemJukebox() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetButton(int button) {
        if (_jukeboxButton != button) { _jukeboxButton = button; _changeCount++; }
    }
    void SetSequence(const std::string& sequence) {
        if (_sequence != sequence) { _sequence = sequence; _changeCount++; }
    }
    int GetButton() const { return _jukeboxButton; }
    void SetEnumState(ENUMJUKEBOX state) { _state = state; }
    ENUMJUKEBOX GetEnumState() const { return _state; }
    std::string GetSequence() const { return _sequence; }
    virtual std::string GetTitle() const override;
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif