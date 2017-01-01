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
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemAllOff(wxXmlNode* node);
    PlayListItemAllOff();
    virtual ~PlayListItemAllOff() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { _duration = duration; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

#pragma region Playing
    virtual void Play() override {};
    virtual void Stop() override {};
    virtual void PlayFrame(long frame) override {};
    virtual wxByte* GetFrameData(long frame) override { return nullptr; };
#pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
    #pragma endregion UI
};
#endif