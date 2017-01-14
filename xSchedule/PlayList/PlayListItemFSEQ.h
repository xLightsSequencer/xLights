#ifndef PLAYLISTITEMFSEQ_H
#define PLAYLISTITEMFSEQ_H

#include "PlayListItem.h"
#include "../FSEQFile.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;

class PlayListItemFSEQ : public PlayListItem
{
protected:

    #pragma region Member Variables
    APPLYMETHOD _applyMethod;
    std::string _fseqFileName;
    std::string _audioFile;
    bool _overrideAudio;
    FSEQFile* _fseqFile;
    AudioManager* _audioManager;
    size_t _durationMS;
    #pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles();
    std::string GetAudioFilename() const;
    void FastSetDuration();

public:

    #pragma region Constructors and Destructors
    PlayListItemFSEQ(wxXmlNode* node);
    PlayListItemFSEQ();
    virtual ~PlayListItemFSEQ() { CloseFiles(); };
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(int blendMode) { _applyMethod = (APPLYMETHOD)blendMode; _changeCount++; }
    virtual size_t GetDurationMS() const override { return _delay + _durationMS; }
    virtual std::string GetNameNoTime() const override;
    std::string GetFSEQFileName() const { return _fseqFileName; }
    std::string GetAudioFile() const { return _audioFile; }
    bool GetOverrideAudio() const { return _overrideAudio; }
    void SetFSEQFileName(const std::string& fseqFileName);
    void SetAudioFile(const std::string& audioFile) { _audioFile = audioFile; _changeCount++; LoadFiles(); }
    void SetOverrideAudio(bool overrideAudio) { _overrideAudio = overrideAudio; _changeCount++; LoadFiles(); }
    virtual bool ControlsTiming() const override { return _audioManager != nullptr; }
    virtual size_t GetPositionMS() const override;
    virtual size_t GetFrameMS() const override { return _msPerFrame; }
    virtual bool Done() const override { return GetPositionMS() >= GetDurationMS(); }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) override;
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Restart() override;
    virtual void Pause(bool pause) override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif