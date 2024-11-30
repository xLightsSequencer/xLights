#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>

#include "PlayListItem.h"
#include "../Blend.h"

class wxXmlNode;
class wxWindow;
class AudioManager;
class PlayerWindow;
class PlayerFrame;
class VideoReader;
class CachedVideoReader;
class OutputManager;
class FSEQFile;
class ScheduleOptions;

class PlayListItemFSEQVideo : public PlayListItem {
protected:
#pragma region Member Variables
    APPLYMETHOD _applyMethod;
    std::string _fseqFileName;
    std::string _audioFile;
    bool _overrideAudio = false;
    bool _topMost = false;
    bool _suppressVirtualMatrix = false;
    FSEQFile* _fseqFile = nullptr;
    AudioManager* _audioManager = nullptr;
    size_t _durationMS = 0;
    size_t _videoLength = 0;
    bool _controlsTimingCache = false;
    size_t _sc = 0;
    std::string _startChannel;
    OutputManager* _outputManager = nullptr;
    size_t _channels = 0;
    bool _fastStartAudio = false;
    bool _cacheVideo = false;
    VideoReader* _videoReader = nullptr;
    CachedVideoReader* _cachedVideoReader = nullptr;
    std::string _cachedAudioFilename;
    long _fadeInMS = 0;
    long _fadeOutMS = 0;
    bool _loopVideo = false;
    bool _useMediaPlayer = false;
    std::string _videoFile;
    wxPoint _origin;
    wxSize _size;
    PlayerWindow* _window = nullptr;
    PlayerFrame* _frame = nullptr;
    std::string _audioDevice = "";
#pragma endregion

    void LoadFiles(bool doCache);
    void CloseFiles();
    void FastSetDuration();
    void LoadAudio();

public:
#pragma region Constructors and Destructors
    PlayListItemFSEQVideo(OutputManager* outputManager, wxXmlNode* node);
    PlayListItemFSEQVideo(OutputManager* outputManager, ScheduleOptions* options);
    virtual ~PlayListItemFSEQVideo();
    virtual PlayListItem* Copy(const bool isClone) const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    AudioManager* GetAudioManager() const {
        return _audioManager;
    }
    bool GetTopMost() const {
        return _topMost;
    }
    void SetTopmost(bool topmost) {
        if (_topMost != topmost) {
            _topMost = topmost;
            _changeCount++;
        }
    }
    bool GetSuppressVirtualMatrix() const {
        return _suppressVirtualMatrix;
    }
    void SetSuppressVirtualMatrix(bool suppressVirtualMatrix) {
        if (_suppressVirtualMatrix != suppressVirtualMatrix) {
            _suppressVirtualMatrix = suppressVirtualMatrix;
            _changeCount++;
        }
    }
    std::string GetAudioFilename();
    int GetBlendMode() const {
        return _applyMethod;
    }
    void SetBlendMode(const std::string& blendMode) {
        if (_applyMethod != EncodeBlendMode(blendMode)) {
            _applyMethod = EncodeBlendMode(blendMode);
            _changeCount++;
        }
    }
    int GetFadeInMS() const {
        return _fadeInMS;
    }
    void SetFadeInMS(const int fadeInMS) {
        if (_fadeInMS != fadeInMS) {
            _fadeInMS = fadeInMS;
            _changeCount++;
        }
    }
    int GetFadeOutMS() const {
        return _fadeOutMS;
    }
    void SetFadeOutMS(const int fadeOutMS) {
        if (_fadeOutMS != fadeOutMS) {
            _fadeOutMS = fadeOutMS;
            _changeCount++;
        }
    }
    virtual size_t GetDurationMS() const override {
        return _delay + _durationMS;
    }
    virtual std::string GetNameNoTime() const override;
    std::string GetFSEQFileName() const {
        return _fseqFileName;
    }
    std::string GetAudioFile() const {
        return _audioFile;
    }
    std::string GetAudioDevice() const {
        return _audioDevice;
    }
    bool GetOverrideAudio() const {
        return _overrideAudio;
    }
    bool GetFastStartAudio() const {
        return _fastStartAudio;
    }
    bool GetCacheVideo() const {
        return _cacheVideo;
    }
    bool GetLoopVideo() const {
        return _loopVideo;
    }
    void SetFSEQFileName(const std::string& fseqFileName);
    void SetAudioFile(const std::string& audioFile);
    void SetAudioDevice(const std::string& audioDevice);
    void SetOverrideAudio(bool overrideAudio);
    void SetFastStartAudio(bool fastStartAudio);
    virtual bool ControlsTiming() const override {
        return _controlsTimingCache || _audioManager != nullptr;
    }
    virtual size_t GetPositionMS() const override;
    virtual size_t GetFrameMS() const override {
        return _msPerFrame;
    }
    virtual bool Done() const override {
        return GetPositionMS() >= GetDurationMS() - GetFrameMS();
    }
    virtual std::string GetSyncItemFSEQ() const override {
        return GetFSEQFileName();
    }
    virtual std::string GetSyncItemMedia() override {
        return GetAudioFilename();
    }
    virtual std::string GetTitle() const override;
    std::string GetStartChannel() const {
        return _startChannel;
    }
    size_t GetStartChannelAsNumber();
    void SetStartChannel(std::string startChannel) {
        if (_startChannel != startChannel) {
            _startChannel = startChannel;
            _sc = 0;
            _changeCount++;
        }
    }
    long GetChannels() const {
        return _channels;
    }
    void SetChannels(long channels) {
        if (_channels != channels) {
            _channels = channels;
            _changeCount++;
        }
    }
    void SetCacheVideo(bool cacheVideo) {
        if (_cacheVideo != cacheVideo) {
            _cacheVideo = cacheVideo;
            _changeCount++;
        }
    }
    void SetLoopVideo(bool loopVideo) {
        if (_loopVideo != loopVideo) {
            _loopVideo = loopVideo;
            _changeCount++;
        }
    }
    virtual bool SetPosition(size_t frame, size_t ms) override;
    void SetLocation(wxPoint pt, wxSize size) {
        if (_origin != pt || _size != size) {
            _origin = pt;
            _size = size;
            _changeCount++;
        }
    }
    void SetVideoFile(const std::string& videoFile);
    std::string GetVideoFile() const {
        return _videoFile;
    }
    wxPoint GetPosition() const {
        return _origin;
    }
    wxSize GetSize() const {
        return _size;
    }
    virtual std::list<std::string> GetMissingFiles() override;
    virtual long GetFSEQChannels() const override;
    bool GetUseMediaPlayer() const {
        return _useMediaPlayer;
    }
    void SetUseMediaPlayer(bool useMediaPlayer) {
        if (_useMediaPlayer != useMediaPlayer) {
            _useMediaPlayer = useMediaPlayer;
            _changeCount++;
        }
    }
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

#pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Restart() override;
    virtual void Pause(bool pause) override;
    virtual void Suspend(bool suspend) override;
    virtual bool Advance(int seconds) override;
#pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
