#pragma once

// A7: SNClassifySoundRequest wrapper. Classifies audio into sound
// types (drums, vocals, guitar, applause, …) by running Apple's
// built-in classifier over the AudioManager's raw signal. Returns a
// per-second confidence curve for each detected class.
//
// Apple-only. The implementation lives in `SoundClassifier.mm`, which
// is compiled only on Apple targets (macOS + iOS). Non-Apple callers
// should guard with `#ifdef __APPLE__` — there is no stub here.

#include <string>
#include <vector>

class AudioManager;

struct SoundClassResult {
    std::string name;
    // One confidence value per `timeStepSeconds` of audio. Values are
    // Apple's [0, 1] classifier confidences for this class on each
    // window.
    std::vector<float> confidence;
    // Helper — mean of `confidence`. Classes are returned sorted by
    // this value descending.
    float averageConfidence = 0.0f;
};

struct SoundClassification {
    std::vector<SoundClassResult> classes;
    // Window-size (seconds) used by the analyzer. Usually 1.0 s for
    // the built-in classifier.
    float timeStepSeconds = 1.0f;
    long lengthMS = 0;
};

struct SoundClassifierOptions {
    // Classes whose averageConfidence falls below this are dropped
    // from the result before `maxClasses` is applied.
    float minAverageConfidence = 0.05f;
    // Upper bound on returned classes (for UI sanity).
    int maxClasses = 12;
    // Window in seconds. Larger = smoother / faster; smaller = more
    // reactive / slower. Apple defaults to 1.0.
    float windowSeconds = 1.0f;
};

// Runs synchronously. Apple's classifier is real-time on M-series
// Macs and ~real-time on modern iPads; a 4-minute track returns in
// well under 10 seconds. Returns an empty `SoundClassification` on
// failure (no audio loaded, framework unavailable, analyzer error).
SoundClassification ClassifySound(AudioManager* audio,
                                   const SoundClassifierOptions& opts = SoundClassifierOptions{});
