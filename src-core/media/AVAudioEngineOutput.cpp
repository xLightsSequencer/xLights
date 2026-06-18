/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only `IAudioOutput` shell. Each method forwards to
// `AppleAudioOutputBridge` in `macOS/src-apple-core/media/`. The
// `AVAudioEngineManager` class is pure C++ (just owns a map of output
// shells) so its impl stays right here.

#ifdef __APPLE__

#include "AVAudioEngineOutput.h"
#include "media/AVAudioEngineOutputBridge.h"

#include <map>
#include <memory>

// ---------- AVAudioEngineOutput ----------

AVAudioEngineOutput::AVAudioEngineOutput(const std::string& device) {
    _bridge = AppleAudioOutputBridge::CreateOutput(device);
}

AVAudioEngineOutput::~AVAudioEngineOutput() {
    AppleAudioOutputBridge::DestroyOutput(_bridge);
}

bool AVAudioEngineOutput::OpenDevice() {
    return AppleAudioOutputBridge::OpenDevice(_bridge);
}

int AVAudioEngineOutput::AddAudio(long len, uint8_t* buffer, int volume, int rate, long tracksize, long lengthMS) {
    return AppleAudioOutputBridge::AddAudio(_bridge, len, buffer, volume, rate, tracksize, lengthMS);
}

void AVAudioEngineOutput::RemoveAudio(int id) {
    AppleAudioOutputBridge::RemoveAudio(_bridge, id);
}

bool AVAudioEngineOutput::HasAudio(int id) const {
    return AppleAudioOutputBridge::HasAudio(_bridge, id);
}

void AVAudioEngineOutput::Play() {
    AppleAudioOutputBridge::Play(_bridge);
}

void AVAudioEngineOutput::Stop() {
    AppleAudioOutputBridge::Stop(_bridge);
}

void AVAudioEngineOutput::Pause(int id, bool pause) {
    AppleAudioOutputBridge::PauseTrack(_bridge, id, pause);
}

void AVAudioEngineOutput::Pause() {
    AppleAudioOutputBridge::Pause(_bridge);
}

void AVAudioEngineOutput::Unpause() {
    AppleAudioOutputBridge::Unpause(_bridge);
}

long AVAudioEngineOutput::Tell(int id) {
    return AppleAudioOutputBridge::Tell(_bridge, id);
}

void AVAudioEngineOutput::Seek(int id, long pos) {
    AppleAudioOutputBridge::Seek(_bridge, id, pos);
}

void AVAudioEngineOutput::SeekAndLimitPlayLength(int id, long pos, long len) {
    AppleAudioOutputBridge::SeekAndLimitPlayLength(_bridge, id, pos, len);
}

void AVAudioEngineOutput::SetVolume(int id, int volume) {
    AppleAudioOutputBridge::SetVolume(_bridge, id, volume);
}

int AVAudioEngineOutput::GetVolume(int id) {
    return AppleAudioOutputBridge::GetVolume(_bridge, id);
}

void AVAudioEngineOutput::SetGlobalVolume(int volume) {
    AppleAudioOutputBridge::SetGlobalVolume(_bridge, volume);
}

void AVAudioEngineOutput::SetRate(float rate) {
    AppleAudioOutputBridge::SetRate(_bridge, rate);
}

void AVAudioEngineOutput::Reopen() {
    AppleAudioOutputBridge::Reopen(_bridge);
}

// ---------- AVAudioEngineManager ----------

struct AVAudioEngineManagerImpl {
    std::map<std::string, std::unique_ptr<AVAudioEngineOutput>> outputs;
    int globalVolume = 100;
    bool initialized = true; // AVAudioEngine is always available on Apple
    std::string defaultOutput;
};

AVAudioEngineManager::AVAudioEngineManager() {
    _impl = new AVAudioEngineManagerImpl();
}

AVAudioEngineManager::~AVAudioEngineManager() {
    delete _impl;
}

IAudioOutput* AVAudioEngineManager::GetOutput(const std::string& device) {
    auto d = device;
    if (d.empty() || d == "(Default)")
        d = _impl->defaultOutput;

    auto it = _impl->outputs.find(d);
    if (it != _impl->outputs.end())
        return it->second.get();

    _impl->outputs[d] = std::make_unique<AVAudioEngineOutput>(d);
    // Outputs default to globalVolume=100; seed from the manager so a newly
    // created output (e.g. after opening a new sequence) honours the volume
    // the user picked from the Audio menu.
    _impl->outputs[d]->SetGlobalVolume(_impl->globalVolume);
    return _impl->outputs[d].get();
}

IAudioInput* AVAudioEngineManager::GetInput(const std::string& /*device*/) {
    return nullptr;
}

void AVAudioEngineManager::SetGlobalVolume(int volume) {
    _impl->globalVolume = volume;
    for (auto& [name, output] : _impl->outputs) {
        output->SetGlobalVolume(volume);
    }
}

int AVAudioEngineManager::GetGlobalVolume() const {
    return _impl->globalVolume;
}

bool AVAudioEngineManager::IsNoAudio() const {
    return !_impl->initialized;
}

void AVAudioEngineManager::SetRate(float rate) {
    for (auto& [name, output] : _impl->outputs) {
        output->SetRate(rate);
    }
}

void AVAudioEngineManager::SetDefaultInput(const std::string& /*input*/) {
}

void AVAudioEngineManager::SetDefaultOutput(const std::string& output) {
    if (output == "(Default)")
        _impl->defaultOutput = "";
    else
        _impl->defaultOutput = output;
}

std::list<std::string> AVAudioEngineManager::GetOutputDevices() {
    return {};
}

std::list<std::string> AVAudioEngineManager::GetInputDevices() {
    return {};
}

#endif // __APPLE__
