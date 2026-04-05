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

// SDL2-based audio output/input implementation.
// Desktop (macOS/Windows/Linux) uses this.  iPad uses AVFoundation instead.

#include "IAudioOutput.h"

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

extern "C" {
#define SDL_MAIN_HANDLED
#include "sdl/include/SDL.h"  // resolved via dependencies/ include path
}

class AudioData {
public:
    static int __nextId;
    int _id;
    long _audio_len;
    Uint8* _audio_pos;
    Uint8* _original_pos;
    int _volume = SDL_MIX_MAXVOLUME;
    int _rate;
    long _original_len;
    long _savedpos;
    long _trackSize;
    long _lengthMS;
    bool _paused;
    AudioData();
    ~AudioData() {
    }
    long Tell() const;
    void Seek(long ms);
    void SavePos();
    void RestorePos();
    void SeekAndLimitPlayLength(long pos, long len);
    void Pause(bool pause) {
        _paused = pause;
    }
};

class BaseSDL {
protected:
    typedef enum SDLSTATE {
        SDLPLAYING,
        SDLNOTPLAYING
    } SDLSTATE;

    SDL_AudioDeviceID _dev = 0;
    SDLSTATE _state = SDLSTATE::SDLNOTPLAYING;
    std::string _device;
    SDL_AudioSpec _wanted_spec = { 0 };
    bool _input = false;

public:
    BaseSDL(const std::string& device) :
        _device(device) {
    }

    virtual ~BaseSDL();
    bool CloseDevice();
    void DumpState(std::string device, int devid, SDL_AudioSpec* wanted, SDL_AudioSpec* actual) const;
    bool OpenDevice(bool input, int rate);
};

class InputSDL : public BaseSDL, public IAudioInput {
    int _listeners = 0;

    bool OpenDeviceInternal();

public:
    InputSDL(const std::string& device) :
        BaseSDL(device) {
    }

    virtual ~InputSDL() {
    }
    static std::list<std::string> GetAudioDevices();

    // IAudioInput
    bool IsListening() const override;
    void StopListening() override;
    void StartListening() override;
    void PurgeAllButInputAudio(int ms) const override;
    int GetAudio(uint8_t* buffer, int bufsize) override;
    int GetMax(int ms) const override;
    std::vector<float> GetSpectrum(int ms) const override;
    void PurgeInput() override;
};

class OutputSDL : public BaseSDL, public IAudioOutput {
    float _playbackrate = 1.0f;
    std::list<AudioData*> _audioData;
    std::mutex _audio_Lock;
    int _initialisedRate = 44100;

public:
    OutputSDL(const std::string& device);
    virtual ~OutputSDL();
    static std::list<std::string> GetOutputDevices();

    // IAudioOutput
    bool OpenDevice() override;
    int AddAudio(long len, uint8_t* buffer, int volume, int rate, long tracksize, long lengthMS) override;
    void RemoveAudio(int id) override;
    bool HasAudio(int id) const override;
    void Play() override;
    void Stop() override;
    void Pause(int id, bool pause) override;
    void Pause() override;
    void Unpause() override;
    long Tell(int id) override;
    void Seek(int id, long pos) override;
    void SeekAndLimitPlayLength(int id, long pos, long len) override;
    void SetVolume(int id, int volume) override;
    int GetVolume(int id) override;
    void SetRate(float rate) override;
    void Reopen() override;

    // SDL-specific (used by fill_audio callback)
    std::list<AudioData*> GetAudio() const;
    [[nodiscard]] std::mutex* GetAudioLock();
    AudioData* GetData(int id) const;

    // SDL-specific (used by Waveform)
    std::vector<float> GetSpectrum(int ms) const;
    void TogglePause();
};

class SDLManager : public IAudioOutputManager {
    std::map<std::string, std::unique_ptr<InputSDL>> _inputs;
    std::map<std::string, std::unique_ptr<OutputSDL>> _outputs;
    int _globalVolume = 100;
    bool _initialised = false;
    std::string _defaultInput = "";
    std::string _defaultOutput = "";

public:
    SDLManager();
    virtual ~SDLManager();

    // IAudioOutputManager
    IAudioOutput* GetOutput(const std::string& device) override;
    IAudioInput* GetInput(const std::string& device) override;
    void SetGlobalVolume(int volume) override;
    int GetGlobalVolume() const override;
    bool IsNoAudio() const override;
    void SetRate(float rate) override;
    void SetDefaultInput(const std::string& input) override;
    void SetDefaultOutput(const std::string& output) override;
    std::list<std::string> GetOutputDevices() override;
    std::list<std::string> GetInputDevices() override;

    // SDL-specific: typed access for code that needs SDL-specific features
    InputSDL* GetInputSDL(const std::string& device);
    OutputSDL* GetOutputSDL(const std::string& device);
};
