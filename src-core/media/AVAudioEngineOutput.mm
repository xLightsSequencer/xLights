/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AVAudioEngineOutput.h"

#import <AVFAudio/AVFAudio.h>

#include <map>
#include <mutex>
#include <string>

#include <log.h>

// ---------- Per-track audio state ----------

struct AudioTrack {
    int trackId;
    uint8_t* rawBuffer;       // owned by AudioManager, not us
    long rawBufferLen;
    long trackSize;           // samples per channel
    long lengthMS;
    int rate;
    int volume = 100;         // 0-100
    bool paused = false;

    // Playback state
    AVAudioPlayerNode* playerNode = nil;
    bool playing = false;

    // Position tracking
    long seekOffsetFrames = 0;      // frame offset from last schedule
    long scheduledFrameCount = 0;   // how many frames were scheduled
};

// ---------- Output implementation ----------

struct AVAudioEngineOutputImpl {
    AVAudioEngine* engine = nil;
    AVAudioUnitTimePitch* timePitch = nil;
    AVAudioMixerNode* mixer = nil;
    float playbackRate = 1.0f;
    bool usingTimePitch = false;  // only insert timePitch when rate != 1.0
    std::string device;
    bool initialized = false;
    int globalVolume = 100;  // 0-100, combined with per-track volume

    std::map<int, AudioTrack*> tracks;
    std::mutex trackLock;
    int nextId = 0;

    ~AVAudioEngineOutputImpl() {
        if (engine && engine.isRunning) {
            [engine stop];
        }
        for (auto& [id, track] : tracks) {
            if (track->playerNode) {
                [track->playerNode stop];
            }
            delete track;
        }
        tracks.clear();
    }

    // The node that player nodes connect TO (either timePitch or mixer directly)
    AVAudioNode* playerTarget() {
        return usingTimePitch ? (AVAudioNode*)timePitch : (AVAudioNode*)mixer;
    }

    bool init() {
        if (initialized) return true;

        engine = [[AVAudioEngine alloc] init];
        timePitch = [[AVAudioUnitTimePitch alloc] init];
        timePitch.rate = playbackRate;
        mixer = engine.mainMixerNode;

        // Only insert timePitch into the chain when rate != 1.0
        // At rate 1.0, playerNode -> mixer directly (lowest latency)
        // At other rates, playerNode -> timePitch -> mixer
        if (playbackRate != 1.0f) {
            AVAudioFormat* fmt = [engine.outputNode inputFormatForBus:0];
            [engine attachNode:timePitch];
            [engine connect:timePitch to:mixer format:fmt];
            usingTimePitch = true;
        }

        NSError* error = nil;
        [engine startAndReturnError:&error];
        if (error) {
            spdlog::error("AVAudioEngine start failed: {}", error.localizedDescription.UTF8String);
            return false;
        }

        initialized = true;
        AVAudioFormat* fmt = [engine.outputNode inputFormatForBus:0];
        spdlog::debug("AVAudioEngine initialized for device '{}', format: {}Hz {}ch, timePitch: {}",
                       device, fmt.sampleRate, (int)fmt.channelCount,
                       usingTimePitch ? "active" : "bypassed");
        return true;
    }

    void ensureTimePitch(bool need) {
        if (need == usingTimePitch) return;
        if (!engine) return;

        bool wasRunning = engine.isRunning;
        if (wasRunning) [engine stop];

        // Disconnect all player nodes first
        for (auto& [id, track] : tracks) {
            if (track->playerNode) {
                [engine disconnectNodeOutput:track->playerNode];
            }
        }

        if (need && !usingTimePitch) {
            // Insert timePitch: disconnect mixer input, add timePitch in between
            AVAudioFormat* fmt = [engine.outputNode inputFormatForBus:0];
            [engine attachNode:timePitch];
            [engine connect:timePitch to:mixer format:fmt];
            usingTimePitch = true;
        } else if (!need && usingTimePitch) {
            // Remove timePitch
            [engine disconnectNodeOutput:timePitch];
            [engine detachNode:timePitch];
            usingTimePitch = false;
            // Re-create timePitch for potential future use
            timePitch = [[AVAudioUnitTimePitch alloc] init];
            timePitch.rate = playbackRate;
        }

        // Reconnect all player nodes to the new target
        for (auto& [id, track] : tracks) {
            if (track->playerNode) {
                AVAudioFormat* nodeFormat = [[AVAudioFormat alloc]
                    initWithCommonFormat:AVAudioPCMFormatFloat32
                    sampleRate:track->rate
                    channels:2
                    interleaved:NO];
                [engine connect:track->playerNode to:playerTarget() format:nodeFormat];
            }
        }

        if (wasRunning) {
            NSError* error = nil;
            [engine startAndReturnError:&error];
        }
    }

    float effectiveVolume(int trackVolume) {
        int v = (std::max(0, std::min(100, trackVolume)) * std::max(0, std::min(100, globalVolume))) / 100;
        return v / 100.0f;
    }

    void updateAllTrackVolumes() {
        for (auto& [id, track] : tracks) {
            if (track->playerNode) {
                track->playerNode.volume = effectiveVolume(track->volume);
            }
        }
    }

    AVAudioPCMBuffer* createFloat32Buffer(AudioTrack* track, long frameOffset, long frameCount) {
        // Source: 16-bit stereo interleaved PCM
        long totalFrames = track->rawBufferLen / (2 * sizeof(int16_t)); // stereo 16-bit
        if (frameCount <= 0) {
            frameCount = totalFrames - frameOffset;
        }
        if (frameOffset + frameCount > totalFrames) {
            frameCount = totalFrames - frameOffset;
        }
        if (frameCount <= 0) return nil;

        // Create Float32 non-interleaved stereo buffer at the track's sample rate
        AVAudioFormat* trackFormat = [[AVAudioFormat alloc]
            initWithCommonFormat:AVAudioPCMFormatFloat32
            sampleRate:track->rate
            channels:2
            interleaved:NO];

        AVAudioPCMBuffer* pcmBuf = [[AVAudioPCMBuffer alloc] initWithPCMFormat:trackFormat
                                                                 frameCapacity:(AVAudioFrameCount)frameCount];
        pcmBuf.frameLength = (AVAudioFrameCount)frameCount;

        // Convert Int16 interleaved -> Float32 non-interleaved
        int16_t* src = (int16_t*)(track->rawBuffer) + frameOffset * 2; // stereo interleaved
        float* leftDst = pcmBuf.floatChannelData[0];
        float* rightDst = pcmBuf.floatChannelData[1];

        for (long i = 0; i < frameCount; i++) {
            leftDst[i] = (float)src[i * 2] / 32768.0f;
            rightDst[i] = (float)src[i * 2 + 1] / 32768.0f;
        }

        return pcmBuf;
    }

    void scheduleTrack(AudioTrack* track, long frameOffset, long frameCount) {
        if (!engine || !track->playerNode) return;

        AVAudioPCMBuffer* pcmBuf = createFloat32Buffer(track, frameOffset, frameCount);
        if (!pcmBuf) return;

        track->seekOffsetFrames = frameOffset;
        track->scheduledFrameCount = pcmBuf.frameLength;

        [track->playerNode scheduleBuffer:pcmBuf completionHandler:nil];
    }
};

// ---------- AVAudioEngineOutput ----------

AVAudioEngineOutput::AVAudioEngineOutput(const std::string& device) {
    _impl = new AVAudioEngineOutputImpl();
    _impl->device = device;
}

AVAudioEngineOutput::~AVAudioEngineOutput() {
    delete _impl;
}

bool AVAudioEngineOutput::OpenDevice() {
    return _impl->init();
}

int AVAudioEngineOutput::AddAudio(long len, uint8_t* buffer, int volume, int rate, long tracksize, long lengthMS) {
    if (!_impl->init()) return -1;

    std::lock_guard<std::mutex> lock(_impl->trackLock);

    auto* track = new AudioTrack();
    track->trackId = _impl->nextId++;
    track->rawBuffer = buffer;
    track->rawBufferLen = len;
    track->trackSize = tracksize;
    track->lengthMS = lengthMS;
    track->rate = rate;
    track->volume = volume;

    // Create player node
    track->playerNode = [[AVAudioPlayerNode alloc] init];
    [_impl->engine attachNode:track->playerNode];

    // Connect playerNode -> target (mixer directly, or timePitch if rate != 1.0)
    AVAudioFormat* nodeFormat = [[AVAudioFormat alloc]
        initWithCommonFormat:AVAudioPCMFormatFloat32
        sampleRate:rate
        channels:2
        interleaved:NO];
    [_impl->engine connect:track->playerNode to:_impl->playerTarget() format:nodeFormat];

    track->playerNode.volume = _impl->effectiveVolume(volume);

    _impl->tracks[track->trackId] = track;

    spdlog::debug("AVAudioEngine: AddAudio id={}, rate={}, len={}, lengthMS={}", track->trackId, rate, len, lengthMS);
    return track->trackId;
}

void AVAudioEngineOutput::RemoveAudio(int id) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return;

    auto* track = it->second;
    [track->playerNode stop];
    [_impl->engine detachNode:track->playerNode];
    _impl->tracks.erase(it);
    delete track;
    spdlog::debug("AVAudioEngine: RemoveAudio id={}", id);
}

bool AVAudioEngineOutput::HasAudio(int id) const {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    return _impl->tracks.count(id) > 0;
}

void AVAudioEngineOutput::Play() {
    if (!_impl->engine) return;

    if (!_impl->engine.isRunning) {
        NSError* error = nil;
        [_impl->engine startAndReturnError:&error];
        if (error) {
            spdlog::error("AVAudioEngine: Failed to start: {}", error.localizedDescription.UTF8String);
            return;
        }
    }

    std::lock_guard<std::mutex> lock(_impl->trackLock);
    for (auto& [id, track] : _impl->tracks) {
        if (!track->paused && track->playerNode) {
            [track->playerNode play];
            track->playing = true;
        }
    }
}

void AVAudioEngineOutput::Stop() {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    for (auto& [id, track] : _impl->tracks) {
        if (track->playerNode) {
            [track->playerNode stop];
            track->playing = false;
        }
    }
}

void AVAudioEngineOutput::Pause(int id, bool pause) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return;
    it->second->paused = pause;
    if (pause && it->second->playerNode) {
        [it->second->playerNode pause];
    } else if (!pause && it->second->playerNode) {
        [it->second->playerNode play];
    }
}

void AVAudioEngineOutput::Pause() {
    if (_impl->engine) {
        [_impl->engine pause];
    }
}

void AVAudioEngineOutput::Unpause() {
    if (_impl->engine && !_impl->engine.isRunning) {
        NSError* error = nil;
        [_impl->engine startAndReturnError:&error];
    }
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    for (auto& [id, track] : _impl->tracks) {
        if (!track->paused && track->playerNode) {
            [track->playerNode play];
        }
    }
}

long AVAudioEngineOutput::Tell(int id) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return 0;
    auto* track = it->second;

    if (!track->playerNode || !track->playing) return 0;

    AVAudioTime* nodeTime = track->playerNode.lastRenderTime;
    if (!nodeTime || !nodeTime.isSampleTimeValid) return 0;

    AVAudioTime* playerTime = [track->playerNode playerTimeForNodeTime:nodeTime];
    if (!playerTime) return 0;

    long framePlayed = (long)playerTime.sampleTime + track->seekOffsetFrames;
    if (framePlayed < 0) framePlayed = 0;
    if (track->trackSize <= 0) return 0;

    // Convert frame position to milliseconds
    long pos = (framePlayed * track->lengthMS) / track->trackSize;
    return pos;
}

void AVAudioEngineOutput::Seek(int id, long pos) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return;
    auto* track = it->second;

    bool wasPlaying = track->playing;
    if (track->playerNode) {
        [track->playerNode stop];
    }

    // Convert ms to frame offset
    long frameOffset = (pos * track->rate) / 1000;
    _impl->scheduleTrack(track, frameOffset, -1);

    if (wasPlaying && !track->paused) {
        [track->playerNode play];
        track->playing = true;
    }
}

void AVAudioEngineOutput::SeekAndLimitPlayLength(int id, long pos, long len) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return;
    auto* track = it->second;

    if (track->playerNode) {
        [track->playerNode stop];
    }

    long frameOffset = (pos * track->rate) / 1000;
    long frameCount = (len * track->rate) / 1000;
    _impl->scheduleTrack(track, frameOffset, frameCount);
}

void AVAudioEngineOutput::SetVolume(int id, int volume) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return;
    it->second->volume = volume;
    if (it->second->playerNode) {
        it->second->playerNode.volume = _impl->effectiveVolume(volume);
    }
}

int AVAudioEngineOutput::GetVolume(int id) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    auto it = _impl->tracks.find(id);
    if (it == _impl->tracks.end()) return 0;
    return it->second->volume;
}

void AVAudioEngineOutput::SetGlobalVolume(int volume) {
    std::lock_guard<std::mutex> lock(_impl->trackLock);
    _impl->globalVolume = volume;
    _impl->updateAllTrackVolumes();
}

void AVAudioEngineOutput::SetRate(float rate) {
    _impl->playbackRate = rate;
    _impl->timePitch.rate = rate;
    // Insert or remove timePitch from the chain based on whether rate != 1.0
    _impl->ensureTimePitch(rate != 1.0f);
}

void AVAudioEngineOutput::Reopen() {
    // AVAudioEngine handles device changes automatically
    spdlog::debug("AVAudioEngine: Reopen (no-op, engine handles device changes)");
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
    return _impl->outputs[d].get();
}

IAudioInput* AVAudioEngineManager::GetInput(const std::string& /*device*/) {
    // Audio input not implemented for AVAudioEngine yet
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
