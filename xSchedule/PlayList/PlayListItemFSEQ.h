#ifndef PLAYLISTITEMFSEQ_H
#define PLAYLISTITEMFSEQ_H

#include "PlayListItem.h"
#include "../FSEQFile.h"
#include "../Blend.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;

#define FSEQFILES "FSEQ files|*.fseq|All files (*.*)|*.*"

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
    bool _controlsTimingCache;
    size_t _startChannel;
    size_t _channels;
    bool _fastStartAudio;
    std::string _cachedAudioFilename;
    #pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles();
    void FastSetDuration();
    void LoadAudio();

public:

    #pragma region Constructors and Destructors
    PlayListItemFSEQ(wxXmlNode* node);
    PlayListItemFSEQ();
    virtual ~PlayListItemFSEQ();
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    AudioManager* GetAudioManager() const { return _audioManager; }
    std::string GetAudioFilename();
    int GetBlendMode() const { return _applyMethod; }
    void SetBlendMode(const std::string blendMode) { if (_applyMethod != EncodeBlendMode(blendMode)) { _applyMethod = EncodeBlendMode(blendMode); _changeCount++; } }
    virtual size_t GetDurationMS() const override { return _delay + _durationMS; }
    virtual std::string GetNameNoTime() const override;
    std::string GetFSEQFileName() const { return _fseqFileName; }
    std::string GetAudioFile() const { return _audioFile; }
    bool GetOverrideAudio() const { return _overrideAudio; }
    bool GetFastStartAudio() const { return _fastStartAudio; }
    void SetFSEQFileName(const std::string& fseqFileName);
    void SetAudioFile(const std::string& audioFile);
    void SetOverrideAudio(bool overrideAudio);
    void SetFastStartAudio(bool fastStartAudio);
    virtual bool ControlsTiming() const override { return _controlsTimingCache || _audioManager != nullptr; }
    virtual size_t GetPositionMS() const override;
    virtual size_t GetFrameMS() const override { return _msPerFrame; }
    virtual bool Done() const override { return GetPositionMS() >= GetDurationMS() - GetFrameMS(); }
    virtual std::string GetSyncItemFSEQ() const override { return GetFSEQFileName(); }
    virtual std::string GetSyncItemMedia() override { return GetAudioFilename(); }
    virtual std::string GetTitle() const override;
    long GetStartChannel() const { return _startChannel; }
    void SetStartChannel(long startChannel) { if (_startChannel != startChannel) { _startChannel = startChannel; _changeCount++; } }
    long GetChannels() const { return _channels; }
    void SetChannels(long channels) { if (_channels != channels) { _channels = channels; _changeCount++; } }
    virtual std::list<std::string> GetMissingFiles() override;
    bool SetPosition(size_t frame, size_t ms);
    virtual long GetFSEQChannels() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Restart() override;
    virtual void Pause(bool pause) override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif