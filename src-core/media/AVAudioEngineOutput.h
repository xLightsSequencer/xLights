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

// AVAudioEngine-based audio output for Apple platforms (macOS + iPad).
// Replaces SDL2 for audio playback on Apple. The actual AVAudioEngine
// machinery lives behind `AppleAudioOutputBridge` in
// `macOS/src-apple-core/media/AVAudioEngineOutputBridge.{h,mm}`; this
// class is a thin `IAudioOutput` shell that holds an opaque bridge
// handle.

#include "IAudioOutput.h"

namespace AppleAudioOutputBridge {
    struct OutputHandle;
}

// The manager is pure C++ — its impl just owns a map of output shells.
struct AVAudioEngineManagerImpl;

class AVAudioEngineOutput : public IAudioOutput {
public:
    AVAudioEngineOutput(const std::string& device);
    ~AVAudioEngineOutput() override;

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

    void SetGlobalVolume(int volume);

private:
    AppleAudioOutputBridge::OutputHandle* _bridge;
};

class AVAudioEngineManager : public IAudioOutputManager {
public:
    AVAudioEngineManager();
    ~AVAudioEngineManager() override;

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

private:
    AVAudioEngineManagerImpl* _impl;
};
