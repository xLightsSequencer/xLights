#ifndef PLAYLISTITEMAUDIO_H
#define PLAYLISTITEMAUDIO_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;

class PlayListItemFSEQ : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _audioFile;
    AudioManager* _audioManager;
    size_t _durationMS;
    #pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles();
    void FastSetDuration();

public:

    #pragma region Constructors and Destructors
    PlayListItemAudio(wxXmlNode* node);
    PlayListItemAudio();
    virtual ~PlayListItemAudio() { CloseFiles(); };
    virtual PlayListAudio* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const override { return _delay + _durationMS; }
    virtual std::string GetNameNoTime() const override;
    std::string GetAudioFile() const { return _audioFile; }
    void SetAudioFile(const std::string& audioFile);
    virtual bool ControlsTiming() const override { return _audioManager != nullptr; }
    virtual size_t GetPositionMS() const override;
    virtual bool Done() const override { return GetPositionMS() >= GetDurationMS(); }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) override {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Restart() override;
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif
