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

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;
class AudioManager;

//*.mid; excluded as ffmpeg wont read them
#define AUDIOFILES "Audio files|*.mp3;*.ogg;*.m4p;*.mp4;*.avi;*.wma;*.au;*.wav;*.m4a;*.mkv;*.mov;*.mpg;*.asf;*.flv;*.mpeg;*.wmv;*.flac|All Files|*.*"

class PlayListItemAudio : public PlayListItem {
protected:
#pragma region Member Variables
    std::string _audioFile = "";
    mutable std::string _lastNameFile = "";
    mutable std::string _lastName = "";
    AudioManager* _audioManager = nullptr;
    size_t _durationMS = 0;
    bool _controlsTimingCache = false;
    bool _fastStartAudio = false;
    std::string _audioDevice = "";
#pragma endregion Member Variables

    void LoadFiles();
    void CloseFiles();
    void FastSetDuration();

public:
#pragma region Constructors and Destructors
    PlayListItemAudio(wxXmlNode* node);
    PlayListItemAudio();
    virtual ~PlayListItemAudio();
    virtual PlayListItem* Copy(const bool isClone) const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    AudioManager* GetAudioManager() const {
        return _audioManager;
    }
    virtual size_t GetDurationMS() const override {
        return _delay + _durationMS;
    }
    virtual std::string GetNameNoTime() const override;
    std::string GetAudioFile() const {
        return _audioFile;
    }
    std::string GetAudioDevice() const {
        return _audioDevice;
    }
    void SetAudioFile(const std::string& audioFile);
    void SetAudioDevice(const std::string& audioDevice);
    virtual bool ControlsTiming() const override {
        return _controlsTimingCache || _audioManager != nullptr;
    }
    virtual size_t GetPositionMS() const override;
    virtual bool Done() const override {
        return GetPositionMS() >= GetDurationMS() - GetFrameMS();
    }
    virtual std::string GetSyncItemMedia() override {
        return GetAudioFile();
    }
    static bool IsAudio(const std::string& ext);
    virtual std::string GetTitle() const override;
    bool GetFastStartAudio() const {
        return _fastStartAudio;
    }
    void SetFastStartAudio(bool fastStartAudio);
    virtual std::list<std::string> GetMissingFiles() override;
    virtual bool SetPosition(size_t frame, size_t ms) override;
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
