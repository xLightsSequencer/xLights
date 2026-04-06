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

#include <cstdint>
#include <list>
#include <string>
#include <vector>

// Abstract audio output interface.  The core AudioManager uses this
// to play/stop/seek audio without depending on a concrete backend
// (SDL2, AVFoundation, etc.).

class IAudioOutput {
public:
    virtual ~IAudioOutput() = default;

    virtual bool OpenDevice() = 0;
    virtual int AddAudio(long len, uint8_t* buffer, int volume, int rate, long tracksize, long lengthMS) = 0;
    virtual void RemoveAudio(int id) = 0;
    virtual bool HasAudio(int id) const = 0;

    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual void Pause(int id, bool pause) = 0;
    virtual void Pause() = 0;
    virtual void Unpause() = 0;

    virtual long Tell(int id) = 0;
    virtual void Seek(int id, long pos) = 0;
    virtual void SeekAndLimitPlayLength(int id, long pos, long len) = 0;

    virtual void SetVolume(int id, int volume) = 0;
    virtual int GetVolume(int id) = 0;
    virtual void SetRate(float rate) = 0;

    virtual void Reopen() = 0;
};

class IAudioInput {
public:
    virtual ~IAudioInput() = default;

    virtual bool IsListening() const = 0;
    virtual void StartListening() = 0;
    virtual void StopListening() = 0;
    virtual void PurgeAllButInputAudio(int ms) const = 0;
    virtual int GetAudio(uint8_t* buffer, int bufsize) = 0;
    virtual int GetMax(int ms) const = 0;
    virtual std::vector<float> GetSpectrum(int ms) const = 0;
    virtual void PurgeInput() = 0;
};

class IAudioOutputManager {
public:
    virtual ~IAudioOutputManager() = default;

    virtual IAudioOutput* GetOutput(const std::string& device) = 0;
    virtual IAudioInput* GetInput(const std::string& device) = 0;

    virtual void SetGlobalVolume(int volume) = 0;
    virtual int GetGlobalVolume() const = 0;
    virtual bool IsNoAudio() const = 0;
    virtual void SetRate(float rate) = 0;

    virtual void SetDefaultInput(const std::string& input) = 0;
    virtual void SetDefaultOutput(const std::string& output) = 0;

    virtual std::list<std::string> GetOutputDevices() = 0;
    virtual std::list<std::string> GetInputDevices() = 0;
};
